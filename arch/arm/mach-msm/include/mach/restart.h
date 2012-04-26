/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
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

#ifdef CONFIG_MSM_NATIVE_RESTART

/* This constant is used in bootloader to decide actions. */
#define RESTART_REASON_BOOT_BASE	0x77665500
#define RESTART_REASON_BOOTLOADER	(RESTART_REASON_BOOT_BASE | 0x00)
#define RESTART_REASON_REBOOT		(RESTART_REASON_BOOT_BASE | 0x01)
#define RESTART_REASON_RECOVERY		(RESTART_REASON_BOOT_BASE | 0x02)
#define RESTART_REASON_RAMDUMP		(RESTART_REASON_BOOT_BASE | 0xAA)
#define RESTART_REASON_POWEROFF		(RESTART_REASON_BOOT_BASE | 0xBB)
#define RESTART_REASON_ERASE_FLASH	(RESTART_REASON_BOOT_BASE | 0xEF)

/* This restart constant is used for oem commands.
   The actual value is parsed from reboot commands.
   RIL FATAL will use oem-99 to restart a device. */
#define RESTART_REASON_OEM_BASE		0x6f656d00
#define RESTART_REASON_RIL_FATAL	(RESTART_REASON_OEM_BASE | 0x99)

/* if arch_reset is called from userspace,
   restart mode will be set to 'h' equal to 104.
   As a result, we need MAX to know the mode is valid. */
enum RESTART_MODE {
	/* for legecy cmd restart */
	RESTART_MODE_LEGECY = 0,

	/* all other restart rised by kernel.
	   these modes will all enter ramdump. */
	RESTART_MODE_Q6_WATCHDOG_BITE,

	RESTART_MODE_MODEM_CRASH,
	RESTART_MODE_MODEM_USER_INVOKED,
	RESTART_MODE_MODEM_UNWEDGE_TIMEOUT,
	RESTART_MODE_MODEM_WATCHDOG_BITE,
	RESTART_MODE_MODEM_ERROR_FATAL,

	RESTART_MODE_MDM_DOG_BITE,
	RESTART_MODE_MDM_FATAL,

	RESTART_MODE_APP_WATCHDOG_BARK,
	/* This is pseudo enum to indicate the maximum,
	   add new  mode before this one. */
	RESTART_MODE_MAX
};

void set_ramdump_reason(const char *msg);
int check_in_panic(void);
void msm_set_restart_mode(int mode);
#else
#define msm_set_restart_mode(mode)
#endif

extern int pmic_reset_irq;
#endif /* _ASM_ARCH_MSM_RESTART_H_ */
