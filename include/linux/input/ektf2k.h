#ifndef _LINUX_ELAN_KTF2K_H
#define _LINUX_ELAN_KTF2K_H

/* [Arima5908][35614][JessicaTseng] [All][Main][TP][DMS]Add 8926DS definition 20140402 start */
/*[Arima5908][27559][bozhi_lin] porting elan-ektf3135 touch driver 20131203 begin*/
#if ((CONFIG_BSP_HW_V_CURRENT >= CONFIG_BSP_HW_V_8226DS_PDP1) && defined(CONFIG_BSP_HW_SKU_8226DS) \
 ||  (CONFIG_BSP_HW_V_CURRENT >= CONFIG_BSP_HW_V_8226SS_PDP1) && defined(CONFIG_BSP_HW_SKU_8226SS) \
 ||  (CONFIG_BSP_HW_V_CURRENT >= CONFIG_BSP_HW_V_8926SS_PDP1) && defined(CONFIG_BSP_HW_SKU_8926SS) \
 ||  (CONFIG_BSP_HW_V_CURRENT >= CONFIG_BSP_HW_V_8926DS_PDP1) && defined(CONFIG_BSP_HW_SKU_8926DS) )
#define ELAN_X_MAX      768
#define ELAN_Y_MAX      1344
#else
#define ELAN_X_MAX      1280
#define ELAN_Y_MAX      2112
#endif
/*[Arima5908][27559][bozhi_lin] 20131203 end  */
/* [Arima5908][35614][JessicaTseng] [All][Main][TP][DMS]Add 8926DS definition 20140402 end */

#define ELAN_KTF2K_NAME "elan-ktf2k"

struct elan_ktf2k_i2c_platform_data {
	uint16_t version;
	int abs_x_min;
	int abs_x_max;
	int abs_y_min;
	int abs_y_max;
	bool i2c_pull_up;
	int irq_gpio;
	u32 irq_flags;
	u32 reset_flags;
	int reset_gpio;
// [All][Main][TP][32783]20140106,Eric, Add the vender and version informations.
	int hw_det_gpio;
	u32 hw_det_gpio_flags;
// [All][Main][TP][32783]end
	int mode_check_gpio;
	int (*power)(int on);
};

#endif /* _LINUX_ELAN_KTF2K_H */
