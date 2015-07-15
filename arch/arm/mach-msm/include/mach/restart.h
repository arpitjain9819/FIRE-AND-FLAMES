/* Copyright (c) 2011, The Linux Foundation. All rights reserved.
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

#ifndef _ASM_ARCH_MSM_RESTART_H_
#define _ASM_ARCH_MSM_RESTART_H_

#define RESTART_NORMAL 0x0
#define RESTART_DLOAD  0x1

// [All][Main][Ramdump][DMS][33536][akenhsu] Modify Qualcomm's ramdump mechanism for SONY 20140130 BEGIN
// Define for all Arima E2 SKU
#ifndef IS_ARIMA_E2_SKU_ALL
#define IS_ARIMA_E2_SKU_ALL \
( (CONFIG_BSP_HW_V_CURRENT >= CONFIG_BSP_HW_V_8226DS_PDP1) && defined(CONFIG_BSP_HW_SKU_8226DS) || \
  (CONFIG_BSP_HW_V_CURRENT >= CONFIG_BSP_HW_V_8226SS_PDP1) && defined(CONFIG_BSP_HW_SKU_8226SS) || \
  (CONFIG_BSP_HW_V_CURRENT >= CONFIG_BSP_HW_V_8926DS_PDP1) && defined(CONFIG_BSP_HW_SKU_8926DS) || \
  (CONFIG_BSP_HW_V_CURRENT >= CONFIG_BSP_HW_V_8926SS_PDP1) && defined(CONFIG_BSP_HW_SKU_8926SS) )
#endif // #ifndef IS_ARIMA_E2_SKU_ALL
// [All][Main][Ramdump][DMS][33536][akenhsu] 20140130 END

#if defined(CONFIG_MSM_NATIVE_RESTART)
void msm_set_restart_mode(int mode);
void msm_restart(char mode, const char *cmd);
#elif defined(CONFIG_ARCH_FSM9XXX)
void fsm_restart(char mode, const char *cmd);
#else
#define msm_set_restart_mode(mode)
#endif

extern int pmic_reset_irq;

// [All][Main][Ramdump][DMS][33828][akenhsu] Add restart_parameters for Linux and SBL1 20140214 BEGIN
#if IS_ARIMA_E2_SKU_ALL
extern void msm_set_crash_status(int status);
extern int msm_get_crash_status(void);
#endif // IS_ARIMA_E2_SKU_ALL
// [All][Main][Ramdump][DMS][33828][akenhsu] 20140214 END

#endif

