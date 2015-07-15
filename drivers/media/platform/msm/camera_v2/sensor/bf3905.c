/* Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "msm_sensor.h"
#include "msm_cci.h"
#include "msm_camera_io_util.h"
#include "msm_camera_i2c_mux.h"
#include <mach/gpiomux.h>

#define BF3905_SENSOR_NAME "bf3905"
DEFINE_MSM_MUTEX(bf3905_mut);

static struct msm_sensor_ctrl_t bf3905_s_ctrl;

static struct msm_sensor_power_setting bf3905_power_setting[] = {
	{	.seq_type = SENSOR_VREG,  ///only USE for i2c pull high		
		.seq_val = CAM_VIO,
		.config_val = 0,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_LOW,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_HIGH,
		.delay = 30,
	},
	{
		.seq_type = SENSOR_CLK,
		.seq_val = SENSOR_CAM_MCLK,
		.config_val = 24000000,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_I2C_MUX,
		.seq_val = 0,
		.config_val = 0,
		.delay = 0,
	},
};

static struct msm_camera_i2c_reg_conf bf3905_start_settings[] = {
	{0xf5, 0x80},
};

static struct msm_camera_i2c_reg_conf bf3905_stop_settings[] = {
	{0xf5, 0x00},
};

static struct msm_camera_i2c_reg_conf bf3905_recommend0_settings[] = {
};
static struct msm_camera_i2c_reg_conf bf3905_recommend1_settings[] = {
		{0x15,0x12},
		{0x09,0x01},
		{0x12,0x00},
		{0x3a,0x20},
		{0x1e,0x40},
		{0x1b,0x0e},
		{0x2a,0x00},
		{0x2b,0x10},
		{0x92,0x09},
		{0x93,0x00},
		{0x8a,0x9c},
		{0x8b,0x82},
		{0x13,0x00},
		{0x01,0x15},
		{0x02,0x23},
		{0x9d,0x20},
		{0x8c,0x02},
		{0x8d,0xee},
		{0x13,0x07},
		{0x5d,0xb3},
		{0xbf,0x08},
		{0xc3,0x08},
		{0xca,0x10},
		{0x62,0x00},
		{0x63,0x00},
		{0xb9,0x00},
		{0x64,0x00},
		{0xbb,0x10},
		{0x08,0x02},
		{0x20,0x09},
		{0x21,0x4f},
		{0x3e,0x83},
		{0x2f,0x84},
		{0x16,0xa1},
		{0x71,0x0f},
		{0x7e,0x84},
		{0x7f,0x3c},
		{0x60,0xe5},
		{0x61,0xf2},
		{0x6d,0xc0},
		{0x1e,0x40},
		{0xd9,0x25},
		{0xdf,0x26},
		{0x17,0x00},
		{0x18,0xa0},
		{0x19,0x00},
		{0x1a,0x78},
		{0x03,0xa0},
		{0x4a,0x0c},
		{0xda,0x00},
		{0xdb,0xa2},
		{0xdc,0x00},
		{0xdd,0x7a},
		{0xde,0x00},
		{0x34,0x1d},
		{0x36,0x45},
		{0x6e,0x20},
		{0xbc,0x0d},
		{0x35,0x30},
		{0x65,0x24},
		{0x66,0x19},
		{0xbd,0xf4},
		{0xbe,0x44},
		{0x9b,0xf4},
		{0x9c,0x44},
		{0x37,0xf4},
		{0x38,0x44},
		{0xf1,0x00},
		{0x70,0x0b},
		{0x73,0x27},
		{0x79,0x24},
		{0x7a,0x12},
		{0x75,0xca},
		{0x76,0x98},
		{0x77,0x2a},
		{0x7b,0x58},
		{0x7d,0x00},
		{0x13,0x07},
		{0x24,0x48},
		{0x25,0xff},
		{0x97,0x3c},
		{0x98,0x8a},
		{0x80,0xD0},
		{0x81,0x00},
		{0x82,0x2a},
		{0x83,0x54},
		{0x84,0x39},
		{0x85,0x5d},
		{0x86,0x88},
		{0x89,0x63},
		{0x94,0x22},
		{0x96,0xb3},
		{0x9a,0x50},
		{0x99,0x10},
		{0x9f,0x64},
		{0x39,0x90},
		{0x3f,0x90},
		{0x90,0x20},
		{0x91,0xd0},		
		{0x40,0x3b},
		{0x41,0x36},
		{0x42,0x2b},
		{0x43,0x1d},
		{0x44,0x1a},
		{0x45,0x14},
		{0x46,0x11},
		{0x47,0x0f},
		{0x48,0x0e},
		{0x49,0x0d},
		{0x4b,0x0c},
		{0x4c,0x0b},
		{0x4e,0x0a},
		{0x4f,0x09},
		{0x50,0x09},
		{0x5a,0x56},
		{0x51,0x12},
		{0x52,0x0d},
		{0x53,0x92},
		{0x54,0x7d},
		{0x57,0x97},
		{0x58,0x43},
		{0x5a,0xd6},
		{0x51,0x39},
		{0x52,0x0f},
		{0x53,0x3b},
		{0x54,0x55},
		{0x57,0x7e},
		{0x58,0x05},
		{0x5c,0x26},
		{0x6a,0xe1},
		{0x23,0x55},
		{0xa1,0x31},
		{0xa2,0x0d},
		{0xa3,0x25},
		{0xa4,0x0b},
		{0xa5,0x23},
		{0xa6,0x04},
		{0xa7,0x17},
		{0xa8,0x1a},
		{0xa9,0x20},
		{0xaa,0x20},
		{0xab,0x20},
		{0xac,0x3c},
		{0xad,0xf0},
		{0xae,0x7b},
		{0xd0,0xa4},
		{0xd1,0x00},
		{0xd2,0x58},
		{0xc5,0xaa},
		{0xc6,0xca},
		{0xc7,0x30},
		{0xc8,0x0d},
		{0xc9,0x10},
		{0xd3,0x09},
		{0xd4,0x24},
		{0xee,0x30},
		{0xb0,0xe0},
		{0xb3,0x48},
		{0xb4,0xe3},
		{0xb1,0xff},
		{0xb2,0xa0},
		{0xb4,0x63},
		{0xb1,0xb3},
		{0xb2,0xa0},
		{0x55,0x00},
		{0x56,0x40},
};
static struct msm_camera_i2c_reg_conf bf3905_snap_settings[] = {
		{0x89,0x23},
		{0x80,0xd0},
		{0x8b,0x82},
};

static struct v4l2_subdev_info bf3905_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_YUYV8_2X8,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
};

static int32_t msm_bf3905_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	return msm_sensor_i2c_probe(client, id, &bf3905_s_ctrl);
}

static const struct i2c_device_id bf3905_i2c_id[] = {
	{BF3905_SENSOR_NAME, (kernel_ulong_t)&bf3905_s_ctrl},
	{ }
};

static struct i2c_driver bf3905_i2c_driver = {
	.id_table = bf3905_i2c_id,
	.probe  = msm_bf3905_i2c_probe,
	.driver = {
		.name = BF3905_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client bf3905_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
};
static int32_t bf3905_enable_i2c_mux(struct msm_camera_i2c_conf *i2c_conf)
{
	struct v4l2_subdev *i2c_mux_sd =
		dev_get_drvdata(&i2c_conf->mux_dev->dev);
	v4l2_subdev_call(i2c_mux_sd, core, ioctl,
		VIDIOC_MSM_I2C_MUX_INIT, NULL);
	v4l2_subdev_call(i2c_mux_sd, core, ioctl,
		VIDIOC_MSM_I2C_MUX_CFG, (void *)&i2c_conf->i2c_mux_mode);
	return 0;
}
static int32_t bf3905_disable_i2c_mux(struct msm_camera_i2c_conf *i2c_conf)
{
	struct v4l2_subdev *i2c_mux_sd =
		dev_get_drvdata(&i2c_conf->mux_dev->dev);
	v4l2_subdev_call(i2c_mux_sd, core, ioctl,
				VIDIOC_MSM_I2C_MUX_RELEASE, NULL);
	return 0;
}
int32_t bf3905_sensor_config(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp)
{
	struct sensorb_cfg_data *cdata = (struct sensorb_cfg_data *)argp;
	long rc = 0;
	int32_t i = 0;
	mutex_lock(s_ctrl->msm_sensor_mutex);
	pr_info("%s:%d %s cfgtype = %d\n", __func__, __LINE__,	s_ctrl->sensordata->sensor_name, cdata->cfgtype);
	switch (cdata->cfgtype) {
	case CFG_GET_SENSOR_INFO:
		pr_info("%s:%d CFG_GET_SENSOR_INFO\n", __func__, __LINE__);		
		memcpy(cdata->cfg.sensor_info.sensor_name,
			s_ctrl->sensordata->sensor_name,
			sizeof(cdata->cfg.sensor_info.sensor_name));
		cdata->cfg.sensor_info.session_id =
			s_ctrl->sensordata->sensor_info->session_id;
		for (i = 0; i < SUB_MODULE_MAX; i++)
			cdata->cfg.sensor_info.subdev_id[i] =
				s_ctrl->sensordata->sensor_info->subdev_id[i];
		pr_info("%s:%d sensor name %s\n", __func__, __LINE__,
			cdata->cfg.sensor_info.sensor_name);
		pr_info("%s:%d session id %d\n", __func__, __LINE__,
			cdata->cfg.sensor_info.session_id);
		for (i = 0; i < SUB_MODULE_MAX; i++)
			pr_info("%s:%d subdev_id[%d] %d\n", __func__, __LINE__, i,
				cdata->cfg.sensor_info.subdev_id[i]);

		break;
	case CFG_SET_INIT_SETTING:
		/* 1. Write Recommend settings */
		/* 2. Write change settings */
		pr_info("%s:%d CFG_SET_INIT_SETTING\n", __func__, __LINE__);		
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(
			s_ctrl->sensor_i2c_client, bf3905_recommend0_settings,
			ARRAY_SIZE(bf3905_recommend0_settings),
			MSM_CAMERA_I2C_BYTE_DATA);
		msleep(100);

		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
					i2c_write_conf_tbl(
					s_ctrl->sensor_i2c_client, bf3905_recommend1_settings,
					ARRAY_SIZE(bf3905_recommend1_settings),
					MSM_CAMERA_I2C_BYTE_DATA);
		break;
	case CFG_SET_RESOLUTION:
		pr_info("%s:%d CFG_SET_RESOLUTION\n", __func__, __LINE__);
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(
			s_ctrl->sensor_i2c_client, bf3905_snap_settings,
			ARRAY_SIZE(bf3905_snap_settings),
			MSM_CAMERA_I2C_BYTE_DATA);		
		break;
	case CFG_SET_STOP_STREAM:
		pr_info("%s:%d CFG_SET_STOP_STREAM\n", __func__, __LINE__);
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(
			s_ctrl->sensor_i2c_client, bf3905_stop_settings,
			ARRAY_SIZE(bf3905_stop_settings),
			MSM_CAMERA_I2C_BYTE_DATA);	
		break;
	case CFG_SET_START_STREAM:
		pr_info("%s:%d CFG_SET_START_STREAM\n", __func__, __LINE__);				
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(
			s_ctrl->sensor_i2c_client, bf3905_start_settings,
			ARRAY_SIZE(bf3905_start_settings),
			MSM_CAMERA_I2C_BYTE_DATA);	
		break;
	case CFG_GET_SENSOR_INIT_PARAMS:
		cdata->cfg.sensor_init_params =
			*s_ctrl->sensordata->sensor_init_params;
		pr_info("%s:%d init params mode %d pos %d mount %d\n", __func__,
			__LINE__,
			cdata->cfg.sensor_init_params.modes_supported,
			cdata->cfg.sensor_init_params.position,
			cdata->cfg.sensor_init_params.sensor_mount_angle);
		break;
	case CFG_SET_SLAVE_INFO: {
		struct msm_camera_sensor_slave_info sensor_slave_info;
		struct msm_sensor_power_setting_array *power_setting_array;
		int slave_index = 0;
		pr_info("%s:%d CFG_SET_SLAVE_INFO\n", __func__, __LINE__);		
		if (copy_from_user(&sensor_slave_info,
		    (void *)cdata->cfg.setting,
		    sizeof(struct msm_camera_sensor_slave_info))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		/* Update sensor slave address */
		if (sensor_slave_info.slave_addr) {
			s_ctrl->sensor_i2c_client->cci_client->sid =
				sensor_slave_info.slave_addr >> 1;
		}

		/* Update sensor address type */
		s_ctrl->sensor_i2c_client->addr_type =
			sensor_slave_info.addr_type;

		/* Update power up / down sequence */
		s_ctrl->power_setting_array =
			sensor_slave_info.power_setting_array;
		power_setting_array = &s_ctrl->power_setting_array;
		power_setting_array->power_setting = kzalloc(
			power_setting_array->size *
			sizeof(struct msm_sensor_power_setting), GFP_KERNEL);
		if (!power_setting_array->power_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(power_setting_array->power_setting,
		    (void *)sensor_slave_info.power_setting_array.power_setting,
		    power_setting_array->size *
		    sizeof(struct msm_sensor_power_setting))) {
			kfree(power_setting_array->power_setting);
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		s_ctrl->free_power_setting = true;
		pr_info("%s sensor id %x\n", __func__,
			sensor_slave_info.slave_addr);
		pr_info("%s sensor addr type %d\n", __func__,
			sensor_slave_info.addr_type);
		pr_info("%s sensor reg %x\n", __func__,
			sensor_slave_info.sensor_id_info.sensor_id_reg_addr);
		pr_info("%s sensor id %x\n", __func__,
			sensor_slave_info.sensor_id_info.sensor_id);
		for (slave_index = 0; slave_index <
			power_setting_array->size; slave_index++) {
			pr_info("%s i %d power setting %d %d %ld %d\n", __func__,
				slave_index,
				power_setting_array->power_setting[slave_index].
				seq_type,
				power_setting_array->power_setting[slave_index].
				seq_val,
				power_setting_array->power_setting[slave_index].
				config_val,
				power_setting_array->power_setting[slave_index].
				delay);
		}
		kfree(power_setting_array->power_setting);
		break;
	}
	case CFG_WRITE_I2C_ARRAY: {
		struct msm_camera_i2c_reg_setting conf_array;
		struct msm_camera_i2c_reg_array *reg_setting = NULL;

		pr_info("%s:%d CFG_WRITE_I2C_ARRAY\n", __func__, __LINE__);


		if (copy_from_user(&conf_array,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_i2c_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}

		conf_array.reg_setting = reg_setting;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write_table(
			s_ctrl->sensor_i2c_client, &conf_array);
		kfree(reg_setting);
		break;
	}
	case CFG_WRITE_I2C_SEQ_ARRAY: {
		struct msm_camera_i2c_seq_reg_setting conf_array;
		struct msm_camera_i2c_seq_reg_array *reg_setting = NULL;
		pr_info("%s:%d CFG_WRITE_I2C_SEQ_ARRAY\n", __func__, __LINE__);

		if (copy_from_user(&conf_array,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_i2c_seq_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_seq_reg_array)),
			GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_seq_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}

		conf_array.reg_setting = reg_setting;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_seq_table(s_ctrl->sensor_i2c_client,
			&conf_array);
		kfree(reg_setting);
		break;
	}

	case CFG_POWER_UP:
		pr_info("%s:%d CFG_POWER_UP\n", __func__, __LINE__);

		if (s_ctrl->func_tbl->sensor_power_up)
			rc = s_ctrl->func_tbl->sensor_power_up(s_ctrl);
		else
			rc = -EFAULT;
		break;

	case CFG_POWER_DOWN:
		pr_info("%s:%d CFG_POWER_DOWN\n", __func__, __LINE__);

		if (s_ctrl->func_tbl->sensor_power_down)
			rc = s_ctrl->func_tbl->sensor_power_down(
				s_ctrl);
		else
			rc = -EFAULT;
		break;

	case CFG_SET_STOP_STREAM_SETTING: {
		struct msm_camera_i2c_reg_setting *stop_setting =
			&s_ctrl->stop_setting;
		struct msm_camera_i2c_reg_array *reg_setting = NULL;
		pr_info("%s:%d CFG_SET_STOP_STREAM_SETTING\n", __func__, __LINE__);
		if (copy_from_user(stop_setting, (void *)cdata->cfg.setting,
		    sizeof(struct msm_camera_i2c_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = stop_setting->reg_setting;
		stop_setting->reg_setting = kzalloc(stop_setting->size *
			(sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
		if (!stop_setting->reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(stop_setting->reg_setting,
		    (void *)reg_setting, stop_setting->size *
		    sizeof(struct msm_camera_i2c_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(stop_setting->reg_setting);
			stop_setting->reg_setting = NULL;
			stop_setting->size = 0;
			rc = -EFAULT;
			break;
		}
		break;
	}
	default:
		rc = -EFAULT;
		break;
	}

	mutex_unlock(s_ctrl->msm_sensor_mutex);

	return rc;
}

int32_t bf3905_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{

		int32_t rc = 0, index = 0;
		struct msm_sensor_power_setting_array *power_setting_array = NULL;
		struct msm_sensor_power_setting *power_setting = NULL;
		struct msm_camera_sensor_board_info *data = s_ctrl->sensordata;
	
		pr_info("%s:%d\n", __func__, __LINE__);
		power_setting_array = &s_ctrl->power_setting_array;
	
		if (data->gpio_conf->cam_gpiomux_conf_tbl != NULL) {
			pr_err("%s:%d mux install\n", __func__, __LINE__);
			msm_gpiomux_install(
				(struct msm_gpiomux_config *)
				data->gpio_conf->cam_gpiomux_conf_tbl,
				data->gpio_conf->cam_gpiomux_conf_tbl_size);
		}
	
		rc = msm_camera_request_gpio_table(
			data->gpio_conf->cam_gpio_req_tbl,
			data->gpio_conf->cam_gpio_req_tbl_size, 1);
		if (rc < 0) {
			pr_err("%s: request gpio failed\n", __func__);
			return rc;
		}
		
		gpio_set_value_cansleep(
			12,GPIOF_OUT_INIT_HIGH);
		gpio_set_value_cansleep(
			13,GPIOF_OUT_INIT_HIGH);
		gpio_set_value_cansleep(
			23,GPIOF_OUT_INIT_HIGH);
	
		for (index = 0; index < power_setting_array->size; index++) {
			pr_info("%s index %d\n", __func__, index);
			power_setting = &power_setting_array->power_setting[index];
			pr_info("%s type %d\n", __func__, power_setting->seq_type);
			switch (power_setting->seq_type) {
			case SENSOR_CLK:
				if (power_setting->seq_val >= s_ctrl->clk_info_size) {
					pr_err("%s clk index %d >= max %d\n", __func__,
						power_setting->seq_val,
						s_ctrl->clk_info_size);
					goto power_up_failed;
				}
				if (power_setting->config_val)
					s_ctrl->clk_info[power_setting->seq_val].
						clk_rate = power_setting->config_val;
	
				rc = msm_cam_clk_enable(s_ctrl->dev,
					&s_ctrl->clk_info[0],
					(struct clk **)&power_setting->data[0],
					s_ctrl->clk_info_size,
					1);
				if (rc < 0) {
					pr_err("%s: clk enable failed\n",
						__func__);
					goto power_up_failed;
				}
				break;
			case SENSOR_GPIO:
				if (power_setting->seq_val >= SENSOR_GPIO_MAX ||
					!data->gpio_conf->gpio_num_info) {
					pr_err("%s gpio index %d >= max %d\n", __func__,
						power_setting->seq_val,
						SENSOR_GPIO_MAX);
					goto power_up_failed;
				}
				pr_debug("%s:%d gpio set val %d\n", __func__, __LINE__,
					data->gpio_conf->gpio_num_info->gpio_num
					[power_setting->seq_val]);
				gpio_set_value_cansleep(
					data->gpio_conf->gpio_num_info->gpio_num
					[power_setting->seq_val],
					power_setting->config_val);
				break;
			case SENSOR_VREG:
				if (power_setting->seq_val >= CAM_VREG_MAX) {
					pr_err("%s vreg index %d >= max %d\n", __func__,
						power_setting->seq_val,
						SENSOR_GPIO_MAX);
					goto power_up_failed;
				}
				msm_camera_config_single_vreg(s_ctrl->dev,
					&data->cam_vreg[power_setting->seq_val],
					(struct regulator **)&power_setting->data[0],
					1);
				break;
			case SENSOR_I2C_MUX:
				if (data->i2c_conf && data->i2c_conf->use_i2c_mux)
					bf3905_enable_i2c_mux(data->i2c_conf);
				break;
			default:
				pr_err("%s error power seq type %d\n", __func__,
					power_setting->seq_type);
				break;
			}
			if (power_setting->delay > 20) {
				msleep(power_setting->delay);
			} else if (power_setting->delay) {
				usleep_range(power_setting->delay * 1000,
					(power_setting->delay * 1000) + 1000);
			}
		}
	
		if (s_ctrl->sensor_device_type == MSM_CAMERA_PLATFORM_DEVICE) {
			rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_util(
				s_ctrl->sensor_i2c_client, MSM_CCI_INIT);
			if (rc < 0) {
				pr_err("%s cci_init failed\n", __func__);
				goto power_up_failed;
			}
		}
	
		if (s_ctrl->func_tbl->sensor_match_id)
			rc = s_ctrl->func_tbl->sensor_match_id(s_ctrl);
		else
			rc = msm_sensor_match_id(s_ctrl);
		if (rc < 0) {
			pr_err("%s:%d match id failed rc %d\n", __func__, __LINE__, rc);
			goto power_up_failed;
		}
	
		pr_info("%s exit\n", __func__);
		return 0;
	power_up_failed:
		pr_err("%s:%d failed\n", __func__, __LINE__);
		if (s_ctrl->sensor_device_type == MSM_CAMERA_PLATFORM_DEVICE) {
			s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_util(
				s_ctrl->sensor_i2c_client, MSM_CCI_RELEASE);
		}
	
		for (index--; index >= 0; index--) {
			pr_info("%s index %d\n", __func__, index);
			power_setting = &power_setting_array->power_setting[index];
			pr_info("%s type %d\n", __func__, power_setting->seq_type);
			switch (power_setting->seq_type) {
			case SENSOR_CLK:
				msm_cam_clk_enable(s_ctrl->dev,
					&s_ctrl->clk_info[0],
					(struct clk **)&power_setting->data[0],
					s_ctrl->clk_info_size,
					0);
				break;
			case SENSOR_GPIO:
				gpio_set_value_cansleep(
					data->gpio_conf->gpio_num_info->gpio_num
					[power_setting->seq_val], GPIOF_OUT_INIT_LOW);
				break;
			case SENSOR_VREG:
				msm_camera_config_single_vreg(s_ctrl->dev,
					&data->cam_vreg[power_setting->seq_val],
					(struct regulator **)&power_setting->data[0],
					0);
				break;
			case SENSOR_I2C_MUX:
				if (data->i2c_conf && data->i2c_conf->use_i2c_mux)
					bf3905_disable_i2c_mux(data->i2c_conf);
				break;
			default:
				pr_err("%s error power seq type %d\n", __func__,
					power_setting->seq_type);
				break;
			}
			if (power_setting->delay > 20) {
				msleep(power_setting->delay);
			} else if (power_setting->delay) {
				usleep_range(power_setting->delay * 1000,
					(power_setting->delay * 1000) + 1000);
			}
		}
		msm_camera_request_gpio_table(
			data->gpio_conf->cam_gpio_req_tbl,
			data->gpio_conf->cam_gpio_req_tbl_size, 0);
		return rc;

};
int32_t bf3905_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t index = 0;
	struct msm_sensor_power_setting_array *power_setting_array = NULL;
	struct msm_sensor_power_setting *power_setting = NULL;
	struct msm_camera_sensor_board_info *data = s_ctrl->sensordata;
	s_ctrl->stop_setting_valid = 0;

	pr_info("%s:%d\n", __func__, __LINE__);
	power_setting_array = &s_ctrl->power_setting_array;

	if (s_ctrl->sensor_device_type == MSM_CAMERA_PLATFORM_DEVICE) {
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_util(
			s_ctrl->sensor_i2c_client, MSM_CCI_RELEASE);
	}

	for (index = (power_setting_array->size - 1); index >= 0; index--) {
		pr_info("%s index %d\n", __func__, index);
		power_setting = &power_setting_array->power_setting[index];
		pr_info("%s type %d\n", __func__, power_setting->seq_type);
		switch (power_setting->seq_type) {
		case SENSOR_CLK:
			msm_cam_clk_enable(s_ctrl->dev,
				&s_ctrl->clk_info[0],
				(struct clk **)&power_setting->data[0],
				s_ctrl->clk_info_size,
				0);
			break;
		case SENSOR_GPIO:
			if (power_setting->seq_val >= SENSOR_GPIO_MAX ||
				!data->gpio_conf->gpio_num_info) {
				pr_err("%s gpio index %d >= max %d\n", __func__,
					power_setting->seq_val,
					SENSOR_GPIO_MAX);
				continue;
			}
			gpio_set_value_cansleep(
				data->gpio_conf->gpio_num_info->gpio_num
				[power_setting->seq_val], GPIOF_OUT_INIT_LOW);
			break;
		case SENSOR_VREG:
			if (power_setting->seq_val >= CAM_VREG_MAX) {
				pr_err("%s vreg index %d >= max %d\n", __func__,
					power_setting->seq_val,
					SENSOR_GPIO_MAX);
				continue;
			}
			msm_camera_config_single_vreg(s_ctrl->dev,
				&data->cam_vreg[power_setting->seq_val],
				(struct regulator **)&power_setting->data[0],
				0);
			break;
		case SENSOR_I2C_MUX:
			if (data->i2c_conf && data->i2c_conf->use_i2c_mux)
				bf3905_disable_i2c_mux(data->i2c_conf);
			break;
		default:
			pr_err("%s error power seq type %d\n", __func__,
				power_setting->seq_type);
			break;
		}
		if (power_setting->delay > 20) {
			msleep(power_setting->delay);
		} else if (power_setting->delay) {
			usleep_range(power_setting->delay * 1000,
				(power_setting->delay * 1000) + 1000);
		}
	}

	gpio_set_value_cansleep(
		12,GPIOF_OUT_INIT_HIGH);
	gpio_set_value_cansleep(
		13,GPIOF_OUT_INIT_HIGH);
	gpio_set_value_cansleep(
		23,GPIOF_OUT_INIT_HIGH);
	msm_camera_request_gpio_table(
		data->gpio_conf->cam_gpio_req_tbl,
		data->gpio_conf->cam_gpio_req_tbl_size, 0);
	pr_info("%s exit\n", __func__);
	return 0;

};

static struct msm_sensor_fn_t bf3905_sensor_func_tbl = {	
	.sensor_config = bf3905_sensor_config,	
	.sensor_power_up = bf3905_power_up,	
	.sensor_power_down = bf3905_power_down,	
	.sensor_match_id = msm_sensor_match_id,
};

static struct msm_sensor_ctrl_t bf3905_s_ctrl = {
	.sensor_i2c_client = &bf3905_sensor_i2c_client,
	.power_setting_array.power_setting = bf3905_power_setting,
	.power_setting_array.size = ARRAY_SIZE(bf3905_power_setting),
	.msm_sensor_mutex = &bf3905_mut,
	.sensor_v4l2_subdev_info = bf3905_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(bf3905_subdev_info),
	.func_tbl = &bf3905_sensor_func_tbl,
};

static const struct of_device_id bf3905_dt_match[] = {
	{.compatible = "qcom,bf3905", .data = &bf3905_s_ctrl},
	{}
};

MODULE_DEVICE_TABLE(of, bf3905_dt_match);

static struct platform_driver bf3905_platform_driver = {
	.driver = {
		.name = "qcom,bf3905",
		.owner = THIS_MODULE,
		.of_match_table = bf3905_dt_match,
	},
};

static int32_t bf3905_platform_probe(struct platform_device *pdev)
{
	int32_t rc = 0;
	const struct of_device_id *match;

	match = of_match_device(bf3905_dt_match, &pdev->dev);
	rc = msm_sensor_platform_probe(pdev, match->data);
	return rc;
}

static int __init bf3905_init_module(void)
{
	int32_t rc = 0;

	rc = platform_driver_probe(&bf3905_platform_driver,
		bf3905_platform_probe);
	if (!rc)
		return rc;
	return i2c_add_driver(&bf3905_i2c_driver);
}

static void __exit bf3905_exit_module(void)
{
	if (bf3905_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&bf3905_s_ctrl);
		platform_driver_unregister(&bf3905_platform_driver);
	} else
		i2c_del_driver(&bf3905_i2c_driver);
	return;
}

module_init(bf3905_init_module);
module_exit(bf3905_exit_module);
MODULE_DESCRIPTION("bf3905");
MODULE_LICENSE("GPL v2");
