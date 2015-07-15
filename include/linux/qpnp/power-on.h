/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef QPNP_PON_H
#define QPNP_PON_H

#include <linux/errno.h>

/* [Arima5908][35596][JessicaTseng] [All][Main][Key][DMS]Add definition 8926DS 20140401 start */
/* [Arima5908][33332][JessicaTseng] [All][Main][Key][DMS]S3 reset timer is only controlled in SBL 20140123 start */
/* [Arima5908][32719][JessicaTseng] [All][Main][KEY][DMS]Porting reset key 20140103 start */
#if ((CONFIG_BSP_HW_V_CURRENT >= CONFIG_BSP_HW_V_8226DS_PDP1) && defined(CONFIG_BSP_HW_SKU_8226DS) \
 ||  (CONFIG_BSP_HW_V_CURRENT >= CONFIG_BSP_HW_V_8226SS_PDP1) && defined(CONFIG_BSP_HW_SKU_8226SS) \
 ||  (CONFIG_BSP_HW_V_CURRENT >= CONFIG_BSP_HW_V_8926SS_PDP1) && defined(CONFIG_BSP_HW_SKU_8926SS) \
 ||  (CONFIG_BSP_HW_V_CURRENT >= CONFIG_BSP_HW_V_8926DS_PDP1) && defined(CONFIG_BSP_HW_SKU_8926DS) )
#define IS_KEEP_DISABLE_S2_RESET 0
#define IS_KEEP_DISABLE_S3_RESET 0
#else
#define IS_KEEP_DISABLE_S2_RESET 1
#define IS_KEEP_DISABLE_S3_RESET 1
#endif
/* [Arima5908][32719][JessicaTseng] [All][Main][KEY][DMS]Porting reset key 20140103 end */
/* [Arima5908][33332][JessicaTseng] [All][Main][Key][DMS]S3 reset timer is only controlled in SBL 20140123 end */
/* [Arima5908][35596][JessicaTseng] [All][Main][Key][DMS]Add definition 8926DS 20140401 end */

/**
 * enum pon_trigger_source: List of PON trigger sources
 * %PON_SMPL:		PON triggered by SMPL - Sudden Momentary Power Loss
 * %PON_RTC:		PON triggered by RTC alarm
 * %PON_DC_CHG:		PON triggered by insertion of DC charger
 * %PON_USB_CHG:	PON triggered by insertion of USB
 * %PON_PON1:		PON triggered by other PMIC (multi-PMIC option)
 * %PON_CBLPWR_N:	PON triggered by power-cable insertion
 * %PON_KPDPWR_N:	PON triggered by long press of the power-key
 */
enum pon_trigger_source {
	PON_SMPL = 1,
	PON_RTC,
	PON_DC_CHG,
	PON_USB_CHG,
	PON_PON1,
	PON_CBLPWR_N,
	PON_KPDPWR_N,
};

/**
 * enum pon_power_off_type: Possible power off actions to perform
 * %PON_POWER_OFF_WARM_RESET:	Reset the MSM but not all PMIC peripherals
 * %PON_POWER_OFF_SHUTDOWN:	Shutdown the MSM and PMIC completely
 * %PON_POWER_OFF_HARD_RESET:	Reset the MSM and all PMIC peripherals
};
 */
enum pon_power_off_type {
	PON_POWER_OFF_WARM_RESET	= 0x01,
	PON_POWER_OFF_SHUTDOWN		= 0x04,
	PON_POWER_OFF_HARD_RESET	= 0x07,
};

#ifdef CONFIG_QPNP_POWER_ON
int qpnp_pon_system_pwr_off(enum pon_power_off_type type);
int qpnp_pon_is_warm_reset(void);
int qpnp_pon_trigger_config(enum pon_trigger_source pon_src, bool enable);
int qpnp_pon_wd_config(bool enable);
#else
static int qpnp_pon_system_pwr_off(enum pon_power_off_type type)
{
	return -ENODEV;
}
static inline int qpnp_pon_is_warm_reset(void) { return -ENODEV; }
static inline int qpnp_pon_trigger_config(enum pon_trigger_source pon_src,
							bool enable)
{
	return -ENODEV;
}
int qpnp_pon_wd_config(bool enable)
{
	return -ENODEV;
}
#endif

#endif
