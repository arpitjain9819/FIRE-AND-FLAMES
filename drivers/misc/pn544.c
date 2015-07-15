/*
 * Copyright (C) 2010 Trusted Logic S.A.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

//#define DEBUG
//#define NFC_DEBUG

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/device.h>
#include <linux/pn544.h>
//<2014/7/14-jazzchiang-44831-B[All][Main][NFC][DMS05988255] Set wakelock when NFC are working, to prove all NFC scenario works well in sleep mode.
#include <linux/wakelock.h>
//>2014/7/14-jazzchiang-44831-B[All][Main][NFC][DMS05988255] Set wakelock when NFC are working, to prove all NFC scenario works well in sleep mode.

#define MAX_BUFFER_SIZE	512
#define ENABLE_DELAY 50
#define DISABLE_DELAY 70

#define VEN_ENABALE	1
#define VEN_DISABALE 0

struct pn544_dev	{
	wait_queue_head_t	read_wq;
	struct mutex		read_mutex;
	struct i2c_client	*client;
	struct miscdevice	pn544_device;
	//<2014/7/14-jazzchiang-44831-B[All][Main][NFC][DMS05988255] Set wakelock when NFC are working, to prove all NFC scenario works well in sleep mode.
	struct wake_lock wake_lock;
	//>2014/7/14-jazzchiang-44831-B[All][Main][NFC][DMS05988255] Set wakelock when NFC are working, to prove all NFC scenario works well in sleep mode.
	unsigned int 		ven_gpio;
	unsigned int 		firm_gpio;
	unsigned int		irq_gpio;
	bool			irq_enabled;
	spinlock_t		irq_enabled_lock;
};


static void pn544_disable_irq(struct pn544_dev *pn544_dev)
{
	unsigned long flags;

	pr_debug("%s, +++\n", __func__);

	spin_lock_irqsave(&pn544_dev->irq_enabled_lock, flags);
	if (pn544_dev->irq_enabled) {
		pr_debug("%s, if (pn544_dev->irq_enabled)\n", __func__);
		disable_irq_nosync(pn544_dev->client->irq);
		pn544_dev->irq_enabled = false;
	}
	spin_unlock_irqrestore(&pn544_dev->irq_enabled_lock, flags);
	pr_debug("%s, ---\n", __func__);
}

static irqreturn_t pn544_dev_irq_handler(int irq, void *dev_id)
{
	struct pn544_dev *pn544_dev = dev_id;

    pr_debug("%s, irq:%d \n", __func__, gpio_get_value(pn544_dev->irq_gpio));

	if(gpio_get_value(pn544_dev->irq_gpio)) {
		pn544_disable_irq(pn544_dev);
		/* Wake up waiting readers */
		//wake_up_interruptible_sync(&pn544_dev->read_wq);
		wake_up_interruptible(&pn544_dev->read_wq);
	} else {
    pr_debug("%s, no irq \n", __func__);
	}

	return IRQ_HANDLED;
}

static ssize_t pn544_dev_read(struct file *filp, char __user *buf,
		size_t count, loff_t *offset)
{
	struct pn544_dev *pn544_dev = filp->private_data;
	char tmp[MAX_BUFFER_SIZE];
	int ret = 0;
#ifdef NFC_DEBUG
	int i;
#endif /* NFC_DEBUG */	

	pr_debug("%s +++, irq:%d\n", __func__, gpio_get_value(pn544_dev->irq_gpio));

	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	pr_debug("%s : reading %zu bytes.\n", __func__, count);

	mutex_lock(&pn544_dev->read_mutex);

	if (!gpio_get_value(pn544_dev->irq_gpio)) {
		pr_debug("%s, no irq, irq_gpio:%d, irq_enabled:%d\n", __func__, 
			gpio_get_value(pn544_dev->irq_gpio), pn544_dev->irq_enabled);

		if (filp->f_flags & O_NONBLOCK) {
			ret = -EAGAIN;
			goto fail;
		}

		if(!(pn544_dev->irq_enabled)) {

			pr_debug("%s, enable_irq\n", __func__);

			pn544_dev->irq_enabled = true;
			enable_irq(pn544_dev->client->irq);
		}

		pr_debug("%s, start wait!\n", __func__);
		pr_debug("%s, no irq, irq_gpio:%d, irq_enabled:%d\n", __func__, gpio_get_value(pn544_dev->irq_gpio), pn544_dev->irq_enabled);

		ret = wait_event_interruptible(pn544_dev->read_wq,
				(gpio_get_value(pn544_dev->irq_gpio)) );

		//ret = wait_io_event_interruptible(pn544_dev->read_wq,
		//		(gpio_get_value(pn544_dev->irq_gpio)) );

		//ret = wait_event_interruptible_timeout(pn544_dev->read_wq,
		//		(gpio_get_value(pn544_dev->irq_gpio)),  msecs_to_jiffies(300));

		pr_debug("%s, resume to read, irq_gpio:%d\n", __func__, gpio_get_value(pn544_dev->irq_gpio));
		pr_debug("%s, ret:0x%x\n", __func__, ret);

		pn544_disable_irq(pn544_dev);

		if(!gpio_get_value(pn544_dev->irq_gpio)){
			ret = -EIO;
			goto fail;
		}
	}

	/* Read data */
	ret = i2c_master_recv(pn544_dev->client, tmp, count);

#ifdef NFC_DEBUG
	pr_info("%s: bytes[%d] ", __func__, count);
	for(i=0;i<count;i++) {
		pr_info("%02x ", tmp[i]);
	}
	pr_info(" \n");
#endif /* NFC_DEBUG */

	mutex_unlock(&pn544_dev->read_mutex);

	if (ret < 0) {
		pr_err("%s: i2c_master_recv returned %d\n", __func__, ret);
		return ret;
	}
	if (ret > count) {
		pr_err("%s: received too many bytes from i2c (%d)\n",
			__func__, ret);
		return -EIO;
	}
	if (copy_to_user(buf, tmp, ret)) {
		pr_warning("%s : failed to copy to user space\n", __func__);
		return -EFAULT;
	}

	pr_debug("%s ---, irq:%d\n", __func__, gpio_get_value(pn544_dev->irq_gpio));

	//<2014/7/14-jazzchiang-44831-B[All][Main][NFC][DMS05988255] Set wakelock when NFC are working, to prove all NFC scenario works well in sleep mode.
	wake_lock_timeout(&pn544_dev->wake_lock, msecs_to_jiffies(1500));
	//>2014/7/14-jazzchiang-44831-B[All][Main][NFC][DMS05988255] Set wakelock when NFC are working, to prove all NFC scenario works well in sleep mode.

	return ret;

fail:
	mutex_unlock(&pn544_dev->read_mutex);

	pr_debug("%s error ---, ret:0x%x\n", __func__, ret);

	return ret;
}

static ssize_t pn544_dev_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *offset)
{
	struct pn544_dev  *pn544_dev;
	char tmp[MAX_BUFFER_SIZE];
	int ret;
#ifdef NFC_DEBUG
	int i;
#endif /* NFC_DEBUG */

	pr_debug("%s +++\n", __func__);

	pn544_dev = filp->private_data;

	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	if (copy_from_user(tmp, buf, count)) {
		pr_err("%s : failed to copy from user space\n", __func__);
		return -EFAULT;
	}

	pr_debug("%s : writing %zu bytes.\n", __func__, count);

#ifdef NFC_DEBUG
	pr_info(" %s: bytes[%d] ", __func__, count);
	for(i=0;i<count;i++) {
		pr_info("%02x ", tmp[i]);
	}
	pr_info(" \n");
#endif /* NFC_DEBUG */

	/* Write data */
	ret = i2c_master_send(pn544_dev->client, tmp, count);

	if (ret != count) {
		pr_err("%s : i2c_master_send returned %d\n", __func__, ret);
		ret = -EIO;
	}

	pr_debug("%s ---\n", __func__);

	return ret;
}

static int pn544_dev_open(struct inode *inode, struct file *filp)
{
	struct pn544_dev *pn544_dev = container_of(filp->private_data,
						struct pn544_dev,
						pn544_device);

	filp->private_data = pn544_dev;

	pr_debug("%s : %d,%d\n", __func__, imajor(inode), iminor(inode));

	return 0;
}

static long pn544_dev_ioctl(struct file *filp,
			    unsigned int cmd, unsigned long arg)
{
	struct pn544_dev *pn544_dev = filp->private_data;

	switch (cmd) {
	case PN544_SET_PWR:
		if (arg == 2) {
			/* power on with firmware download (requires hw reset)
			 */
			pr_info("%s power on with firmware\n", __func__);
			gpio_direction_output(pn544_dev->ven_gpio, VEN_ENABALE);
			gpio_direction_output(pn544_dev->firm_gpio, 1);
			msleep(ENABLE_DELAY);
			gpio_direction_output(pn544_dev->ven_gpio, VEN_DISABALE);
			msleep(DISABLE_DELAY);
			gpio_direction_output(pn544_dev->ven_gpio, VEN_ENABALE);
			msleep(ENABLE_DELAY);
		} else if (arg == 1) {
			/* power on */
/* --- [ALL][Main][NFC][DMS][41958][LuboLu] Fix turn-off NFC can't suspend issue. 20140728 begin ---  */
			irq_set_irq_wake(pn544_dev->client->irq, 1);
/* --- [ALL][Main][NFC][DMS][41958][LuboLu] 20140728 end ---  */
			pr_info("%s power on\n", __func__);
			gpio_direction_output(pn544_dev->firm_gpio, 0);
			gpio_direction_output(pn544_dev->ven_gpio, VEN_ENABALE);
			msleep(ENABLE_DELAY);
		} else  if (arg == 0) {
			/* power off */
/* --- [ALL][Main][NFC][DMS][41958][LuboLu] Fix turn-off NFC can't suspend issue. 20140728 begin ---  */
			irq_set_irq_wake(pn544_dev->client->irq, 0);
/* --- [ALL][Main][NFC][DMS][41958][LuboLu] 20140728 end ---  */
			pr_info("%s power off\n", __func__);
			gpio_direction_output(pn544_dev->firm_gpio, 0);
			gpio_direction_output(pn544_dev->ven_gpio, VEN_DISABALE);
			msleep(DISABLE_DELAY);
		} else {
			pr_err("%s bad arg %lu\n", __func__, arg);
			return -EINVAL;
		}
		break;
	default:
		pr_err("%s bad ioctl %u\n", __func__, cmd);
		return -EINVAL;
	}
	return 0;
}

static const struct file_operations pn544_dev_fops = {
	.owner	= THIS_MODULE,
	.llseek	= no_llseek,
	.read	= pn544_dev_read,
	.write	= pn544_dev_write,
	.open	= pn544_dev_open,
	.unlocked_ioctl	= pn544_dev_ioctl,
};

static int pn544_parse_dt(struct device *dev, struct pn544_i2c_platform_data *pdata)
{
	int r = 0;
	struct device_node *np = dev->of_node;

	pr_debug("%s, get VEN gpio number\n", __func__);
	pdata->ven_gpio= of_get_named_gpio(np, "nxp,ven-gpio", 0);
	if ((!gpio_is_valid(pdata->ven_gpio)))
		return -EINVAL;
	pr_debug("%s, get IRQ gpio number\n", __func__);
	pdata->irq_gpio = of_get_named_gpio(np, "nxp,irq-gpio", 0);
	if ((!gpio_is_valid(pdata->irq_gpio)))
		return -EINVAL;
	pr_debug("%s, get Firm gpio number\n", __func__);
	pdata->firm_gpio= of_get_named_gpio(np, "nxp,firm-gpio", 0);
	if ((!gpio_is_valid(pdata->firm_gpio)))
		return -EINVAL;

	return r;
}

static int pn544_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int r = 0;
	int ret;
	struct pn544_i2c_platform_data *platform_data;
	struct pn544_dev *pn544_dev;

	if (client->dev.of_node) {
		platform_data = devm_kzalloc(&client->dev,
			sizeof(*platform_data), GFP_KERNEL);

		if (!platform_data) {
			dev_err(&client->dev,
			"pn544 probe: Failed to allocate memory\n");
			return -ENOMEM;
		}
		r = pn544_parse_dt(&client->dev, platform_data);
		if (r)
			return r;
	} else {
		platform_data = client->dev.platform_data;
	}

	if (!platform_data){
		pr_err("%s, Can not get platform_data\n", __func__);
		return -EINVAL;
		}

	dev_dbg(&client->dev,
		"pn544 probe: %s, inside pn544 flags = %x\n",
		__func__, client->flags);

	if (platform_data == NULL) {
		pr_err("%s : nfc probe fail\n", __func__);
		return  -ENODEV;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("%s : need I2C_FUNC_I2C\n", __func__);
		return  -ENODEV;
	}

	ret = gpio_request(platform_data->irq_gpio, "nfc_int");
	if (ret)
		return  -ENODEV;
	ret = gpio_request(platform_data->ven_gpio, "nfc_ven");
	if (ret)
		goto err_ven;
	ret = gpio_request(platform_data->firm_gpio, "nfc_firm");
	if (ret)
		goto err_firm;

	gpio_direction_input(platform_data->irq_gpio);
	gpio_direction_output(platform_data->ven_gpio, VEN_DISABALE);
	gpio_direction_output(platform_data->firm_gpio, 0);

	pn544_dev = kzalloc(sizeof(*pn544_dev), GFP_KERNEL);
	if (pn544_dev == NULL) {
		dev_err(&client->dev,
				"failed to allocate memory for module data\n");
		ret = -ENOMEM;
		goto err_exit;
	}

	pn544_dev->irq_gpio = platform_data->irq_gpio;
	pn544_dev->ven_gpio  = platform_data->ven_gpio;
	pn544_dev->firm_gpio  = platform_data->firm_gpio;
	pn544_dev->client   = client;

	/* init mutex and queues */
	init_waitqueue_head(&pn544_dev->read_wq);
	mutex_init(&pn544_dev->read_mutex);
	spin_lock_init(&pn544_dev->irq_enabled_lock);

	pn544_dev->pn544_device.minor = MISC_DYNAMIC_MINOR;
	pn544_dev->pn544_device.name = "pn544";
	pn544_dev->pn544_device.fops = &pn544_dev_fops;

	ret = misc_register(&pn544_dev->pn544_device);
	if (ret) {
		pr_err("%s : misc_register failed\n", __FILE__);
		goto err_misc_register;
	}

	/* request irq.  the irq is set whenever the chip has data available
	 * for reading.  it is cleared when all data has been read.
	 */
	client->irq = gpio_to_irq(platform_data->irq_gpio);
	pr_debug("%s : requesting IRQ %d\n", __func__, client->irq);
	pn544_dev->irq_enabled = true;
	ret = request_irq(client->irq, pn544_dev_irq_handler,
			  IRQF_TRIGGER_HIGH, client->name, pn544_dev);
	if (ret) {
		dev_err(&client->dev, "request_irq failed\n");
		goto err_request_irq_failed;
	}

	pn544_disable_irq(pn544_dev);
	i2c_set_clientdata(client, pn544_dev);

	//<2014/7/14-jazzchiang-44831-B[All][Main][NFC][DMS05988255] Set wakelock when NFC are working, to prove all NFC scenario works well in sleep mode.
	wake_lock_init(&pn544_dev->wake_lock, WAKE_LOCK_SUSPEND, "pn544_nfc_read");
	//>2014/7/14-jazzchiang-44831-B[All][Main][NFC][DMS05988255] Set wakelock when NFC are working, to prove all NFC scenario works well in sleep mode.

	return 0;

err_request_irq_failed:
	misc_deregister(&pn544_dev->pn544_device);
err_misc_register:
	mutex_destroy(&pn544_dev->read_mutex);
	kfree(pn544_dev);
err_exit:
	gpio_free(platform_data->firm_gpio);
err_firm:
	gpio_free(platform_data->ven_gpio);
err_ven:
	gpio_free(platform_data->irq_gpio);

	//<2014/7/14-jazzchiang-44831-B[All][Main][NFC][DMS05988255] Set wakelock when NFC are working, to prove all NFC scenario works well in sleep mode.
	wake_lock_destroy(&pn544_dev->wake_lock);
	//>2014/7/14-jazzchiang-44831-B[All][Main][NFC][DMS05988255] Set wakelock when NFC are working, to prove all NFC scenario works well in sleep mode.

	return ret;
}

static int pn544_remove(struct i2c_client *client)
{
	struct pn544_dev *pn544_dev;

	pn544_dev = i2c_get_clientdata(client);
	free_irq(client->irq, pn544_dev);
	misc_deregister(&pn544_dev->pn544_device);
	mutex_destroy(&pn544_dev->read_mutex);
	//<2014/7/14-jazzchiang-44831-B[All][Main][NFC][DMS05988255] Set wakelock when NFC are working, to prove all NFC scenario works well in sleep mode.
	wake_lock_destroy(&pn544_dev->wake_lock);
	//>2014/7/14-jazzchiang-44831-B[All][Main][NFC][DMS05988255] Set wakelock when NFC are working, to prove all NFC scenario works well in sleep mode.
	gpio_free(pn544_dev->irq_gpio);
	gpio_free(pn544_dev->ven_gpio);
	gpio_free(pn544_dev->firm_gpio);
	kfree(pn544_dev);

	return 0;
}

static const struct i2c_device_id pn544_id[] = {
	{"pn544", 0},
	{}
};

static struct of_device_id pn544_match_table[] = {
	{.compatible = "nxp,nfc-pn544"},
	{}
};

static struct i2c_driver pn544_driver = {
	.id_table	= pn544_id,
	.probe		= pn544_probe,
	.remove		= pn544_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "pn544",
		.of_match_table = pn544_match_table,		
	},
};

/*
 * module load/unload record keeping
 */

static int __init pn544_dev_init(void)
{
	pr_info("Loading pn544 driver\n");
	return i2c_add_driver(&pn544_driver);
}
module_init(pn544_dev_init);

static void __exit pn544_dev_exit(void)
{
	pr_info("Unloading pn544 driver\n");
	i2c_del_driver(&pn544_driver);
}
module_exit(pn544_dev_exit);

MODULE_AUTHOR("Sylvain Fonteneau");
MODULE_DESCRIPTION("NFC PN544 driver");
MODULE_LICENSE("GPL");
