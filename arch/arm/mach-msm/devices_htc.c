/* linux/arch/arm/mach-msm/devices.c
 *
 * Copyright (C) 2008 Google, Inc.
 * Copyright (C) 2007-2009 HTC Corporation.
 * Author: Thomas Tsai <thomas_tsai@htc.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/platform_device.h>

#include <linux/dma-mapping.h>
#include <mach/msm_iomap.h>
#include <mach/dma.h>
#include "devices.h"
#include <mach/board.h>
#include <mach/board_htc.h>
#include <mach/msm_hsusb.h>
#include <linux/usb/android_composite.h>

#include <asm/mach/flash.h>
#include <asm/setup.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/delay.h>
#include <linux/android_pmem.h>
#include <mach/msm_rpcrouter.h>
#include <mach/msm_iomap.h>
#include <asm/mach/mmc.h>
#include <linux/msm_kgsl.h>
#include <mach/dal_axi.h>
#include "proc_comm.h"

static char *df_serialno = "000000000000";
static char *board_sn;

#define MFG_GPIO_TABLE_MAX_SIZE        0x400
static unsigned char mfg_gpio_table[MFG_GPIO_TABLE_MAX_SIZE];

#define PM_LIBPROG      0x30000061
#if (CONFIG_MSM_AMSS_VERSION == 6220) || (CONFIG_MSM_AMSS_VERSION == 6225)
#define PM_LIBVERS      0xfb837d0b
#else
#define PM_LIBVERS      0x10001
#endif

/*#if 1
static struct platform_device *msm_serial_devices[] __initdata = {
#ifndef CONFIG_ARCH_MSM8X60
	&msm_device_uart1,
	&msm_device_uart2,
	&msm_device_uart3,
	#ifdef CONFIG_SERIAL_MSM_HS
	&msm_device_uart_dm1,
	&msm_device_uart_dm2,
	#endif
#endif
};*/

#ifdef CONFIG_ARCH_MSM8X60
static struct msm_mem_settings mem_settings[] = {
	{
		.mem_size_mb = 1024,
		.mem_info = {
			.nr_banks = 2,
			.bank = {
				[0] = {
					.start = 0x40400000,
					.size = 0x42E00000 - 0x40400000,
				},
				[1] = {
					.start = 0x48000000,
					.size = 0x80000000 - 0x48000000,
				}
			}
		}
	}
};
#endif

/*int __init msm_add_serial_devices(unsigned num)
{
	if (num > MSM_SERIAL_NUM)
		return -EINVAL;
	if (num)
		return platform_device_register(msm_serial_devices[num]);
	else
		return -EINVAL;
}
#endif*/

#define ATAG_SMI 0x4d534D71
/* setup calls mach->fixup, then parse_tags, parse_cmdline
 * We need to setup meminfo in mach->fixup, so this function
 * will need to traverse each tag to find smi tag.
 */
int __init parse_tag_smi(const struct tag *tags)
{
	int smi_sz = 0, find = 0;
	struct tag *t = (struct tag *)tags;

	for (; t->hdr.size; t = tag_next(t)) {
		if (t->hdr.tag == ATAG_SMI) {
			printk(KERN_DEBUG "find the smi tag\n");
			find = 1;
			break;
		}
	}
	if (!find)
		return -1;

	printk(KERN_DEBUG "parse_tag_smi: smi size = %d\n", t->u.mem.size);
	smi_sz = t->u.mem.size;
	return smi_sz;
}
__tagtable(ATAG_SMI, parse_tag_smi);


#define ATAG_HWID 0x4d534D72
int __init parse_tag_hwid(const struct tag *tags)
{
	int hwid = 0, find = 0;
	struct tag *t = (struct tag *)tags;

	for (; t->hdr.size; t = tag_next(t)) {
		if (t->hdr.tag == ATAG_HWID) {
			printk(KERN_DEBUG "find the hwid tag\n");
			find = 1;
			break;
		}
	}

	if (find)
		hwid = t->u.revision.rev;
	printk(KERN_DEBUG "parse_tag_hwid: hwid = 0x%x\n", hwid);
	return hwid;
}
__tagtable(ATAG_HWID, parse_tag_hwid);

static char *keycap_tag = NULL;
static int __init board_keycaps_tag(char *get_keypads)
{
	if(strlen(get_keypads))
		keycap_tag = get_keypads;
	else
		keycap_tag = NULL;
	return 1;
}
__setup("androidboot.keycaps=", board_keycaps_tag);

void board_get_keycaps_tag(char **ret_data)
{
	*ret_data = keycap_tag;
}
EXPORT_SYMBOL(board_get_keycaps_tag);

static char *cid_tag = NULL;
static int __init board_set_cid_tag(char *get_hboot_cid)
{
	if(strlen(get_hboot_cid))
		cid_tag = get_hboot_cid;
	else
		cid_tag = NULL;
	return 1;
}
__setup("androidboot.cid=", board_set_cid_tag);

void board_get_cid_tag(char **ret_data)
{
	*ret_data = cid_tag;
}
EXPORT_SYMBOL(board_get_cid_tag);

static char *carrier_tag = NULL;
static int __init board_set_carrier_tag(char *get_hboot_carrier)
{
	if(strlen(get_hboot_carrier))
		carrier_tag = get_hboot_carrier;
	else
		carrier_tag = NULL;
	return 1;
}
__setup("androidboot.carrier=", board_set_carrier_tag);

void board_get_carrier_tag(char **ret_data)
{
	*ret_data = carrier_tag;
}
EXPORT_SYMBOL(board_get_carrier_tag);

/* G-Sensor calibration value */
#define ATAG_GS         0x5441001d

unsigned int gs_kvalue;
EXPORT_SYMBOL(gs_kvalue);

static int __init parse_tag_gs_calibration(const struct tag *tag)
{
	gs_kvalue = tag->u.revision.rev;
	printk(KERN_DEBUG "%s: gs_kvalue = 0x%x\n", __func__, gs_kvalue);
	return 0;
}

__tagtable(ATAG_GS, parse_tag_gs_calibration);

/* Proximity sensor calibration values */
#define ATAG_PS         0x5441001c

unsigned int ps_kparam1;
EXPORT_SYMBOL(ps_kparam1);

unsigned int ps_kparam2;
EXPORT_SYMBOL(ps_kparam2);

static int __init parse_tag_ps_calibration(const struct tag *tag)
{
	ps_kparam1 = tag->u.serialnr.low;
	ps_kparam2 = tag->u.serialnr.high;

	printk(KERN_INFO "%s: ps_kparam1 = 0x%x, ps_kparam2 = 0x%x\n",
		__func__, ps_kparam1, ps_kparam2);

	return 0;
}

__tagtable(ATAG_PS, parse_tag_ps_calibration);

unsigned int als_kadc;
EXPORT_SYMBOL(als_kadc);

static int __init parse_tag_als_calibration(const struct tag *tag)
{
	als_kadc = tag->u.als_kadc.kadc;

	return 0;
}

__tagtable(ATAG_ALS, parse_tag_als_calibration);

/* CSA sensor calibration values */
#define ATAG_CSA	0x5441001f

unsigned int csa_kvalue1;
EXPORT_SYMBOL(csa_kvalue1);

unsigned int csa_kvalue2;
EXPORT_SYMBOL(csa_kvalue2);

unsigned int csa_kvalue3;
EXPORT_SYMBOL(csa_kvalue3);

static int __init parse_tag_csa_calibration(const struct tag *tag)
{
	unsigned int *ptr = (unsigned int *)&tag->u;
	csa_kvalue1 = ptr[0];
	csa_kvalue2 = ptr[1];
	csa_kvalue3 = ptr[2];

	printk(KERN_DEBUG "csa_kvalue1 = 0x%x, csa_kvalue2 = 0x%x, "
	"csa_kvalue3 = 0x%x\n", csa_kvalue1, csa_kvalue2, csa_kvalue3);

	return 0;
}
__tagtable(ATAG_CSA, parse_tag_csa_calibration);

/* Gyro/G-senosr calibration values */
#define ATAG_GRYO_GSENSOR	0x54410020
unsigned char gyro_gsensor_kvalue[37];
EXPORT_SYMBOL(gyro_gsensor_kvalue);

/* camera values */
#define ATAG_CAM	0x54410021

int __init parse_tag_cam(const struct tag *tags)
{
int mem_size = 0, find = 0;
struct tag *t = (struct tag *)tags;

for (; t->hdr.size; t = tag_next(t)) {
	if (t->hdr.tag == ATAG_CAM) {
		printk(KERN_DEBUG "find the memsize tag\n");
		find = 1;
		break;
	}
}

if (find) {
	mem_size = t->u.revision.rev;
}
printk(KERN_DEBUG "parse_tag_memsize: %d\n", mem_size);
return mem_size;
}
__tagtable(ATAG_CAM, parse_tag_cam);


static int __init parse_tag_gyro_gsensor_calibration(const struct tag *tag)
{
	int i;
	unsigned char *ptr = (unsigned char *)&tag->u;
	memcpy(&gyro_gsensor_kvalue[0], ptr, sizeof(gyro_gsensor_kvalue));

	printk(KERN_DEBUG "gyro_gs data\n");
	for (i = 0; i < sizeof(gyro_gsensor_kvalue); i++)
		printk(KERN_DEBUG "[%d]:0x%x", i, gyro_gsensor_kvalue[i]);

	return 0;
}
__tagtable(ATAG_GRYO_GSENSOR, parse_tag_gyro_gsensor_calibration);

static int mfg_mode;
int __init board_mfg_mode_init(char *s)
{
	if (!strcmp(s, "normal"))
		mfg_mode = 0;
	else if (!strcmp(s, "factory2"))
		mfg_mode = 1;
	else if (!strcmp(s, "recovery"))
		mfg_mode = 2;
	else if (!strcmp(s, "charge"))
		mfg_mode = 3;
	else if (!strcmp(s, "power_test"))
		mfg_mode = 4;
	else if (!strcmp(s, "offmode_charging"))
		mfg_mode = 5;
	else if (!strcmp(s, "mfgkernel:diag58"))
		mfg_mode = 6;
	else if (!strcmp(s, "gift_mode"))
		mfg_mode = 7;

	return 1;
}

int board_mfg_mode(void)
{
	return mfg_mode;
}

EXPORT_SYMBOL(board_mfg_mode);

__setup("androidboot.mode=", board_mfg_mode_init);

static int build_flag;

static int __init board_bootloader_setup(char *str)
{
	char temp[strlen(str) + 1];
	char *p = NULL;
	char *build = NULL;
	char *args = temp;

	printk(KERN_INFO "%s: %s\n", __func__, str);

	strcpy(temp, str);

	/*parse the last parameter*/
	while ((p = strsep(&args, ".")) != NULL) build = p;

	/* Sometime hboot version would change from .X000 to .X001, .X002,...
	 * So compare the first character to avoid unnecessary error.
	 */
	if (build) {
		if (build[0] == '0') {
			printk(KERN_INFO "%s: SHIP BUILD\n", __func__);
			build_flag = SHIP_BUILD;
		} else if (build[0] == '2') {
			printk(KERN_INFO "%s: ENG BUILD\n", __func__);
			build_flag = ENG_BUILD;
		} else if (build[0] == '1') {
			printk(KERN_INFO "%s: MFG BUILD\n", __func__);
			build_flag = MFG_BUILD;
		} else {
			printk(KERN_INFO "%s: default ENG BUILD\n", __func__);
			build_flag = ENG_BUILD;
		}
	}
	return 1;
}
__setup("androidboot.bootloader=", board_bootloader_setup);

int board_build_flag(void)
{
	return build_flag;
}

EXPORT_SYMBOL(board_build_flag);

static int __init board_serialno_setup(char *serialno)
{
	char *str;

	/* use default serial number when mode is factory2 */
	if (board_mfg_mode() == 1 || !strlen(serialno))
		str = df_serialno;
	else
		str = serialno;
#ifdef CONFIG_USB_FUNCTION
	msm_hsusb_pdata.serial_number = str;
#endif
	board_sn = str;
	return 1;
}
__setup("androidboot.serialno=", board_serialno_setup);

char *board_serialno(void)
{
	return board_sn;
}

#define ATAG_SKUID 0x4d534D73
int __init parse_tag_skuid(const struct tag *tags)
{
	int skuid = 0, find = 0;
	struct tag *t = (struct tag *)tags;

	for (; t->hdr.size; t = tag_next(t)) {
		if (t->hdr.tag == ATAG_SKUID) {
			printk(KERN_DEBUG "find the skuid tag\n");
			find = 1;
			break;
		}
	}

	if (find)
		skuid = t->u.revision.rev;
	printk(KERN_DEBUG "parse_tag_skuid: hwid = 0x%x\n", skuid);
	return skuid;
}
__tagtable(ATAG_SKUID, parse_tag_skuid);

#define ATAG_HERO_PANEL_TYPE 0x4d534D74
int panel_type;
int __init tag_panel_parsing(const struct tag *tags)
{
	panel_type = tags->u.revision.rev;

	printk(KERN_DEBUG "%s: panel type = %d\n", __func__,
		panel_type);

	return panel_type;
}
__tagtable(ATAG_HERO_PANEL_TYPE, tag_panel_parsing);

/* ISL29028 ID values */
#define ATAG_PS_TYPE 0x4d534D77
int ps_type;
EXPORT_SYMBOL(ps_type);
int __init tag_ps_parsing(const struct tag *tags)
{
	ps_type = tags->u.revision.rev;

	printk(KERN_DEBUG "%s: PS type = 0x%x\n", __func__,
		ps_type);

	return ps_type;
}
__tagtable(ATAG_PS_TYPE, tag_ps_parsing);

#define ATAG_ENGINEERID 0x4d534D75
unsigned engineer_id;
EXPORT_SYMBOL(engineer_id);
int __init parse_tag_engineerid(const struct tag *tags)
{
	int engineerid = 0, find = 0;
	struct tag *t = (struct tag *)tags;

	for (; t->hdr.size; t = tag_next(t)) {
		if (t->hdr.tag == ATAG_ENGINEERID) {
			printk(KERN_DEBUG "find the engineer tag\n");
			find = 1;
			break;
		}
	}

	if (find) {
		engineer_id = t->u.revision.rev;
		engineerid = t->u.revision.rev;
	}
	printk(KERN_DEBUG "parse_tag_engineerid: 0x%x\n", engineerid);
	return engineerid;
}
__tagtable(ATAG_ENGINEERID, parse_tag_engineerid);

#define ATAG_MFG_GPIO_TABLE 0x59504551
int __init parse_tag_mfg_gpio_table(const struct tag *tags)
{
       unsigned char *dptr = (unsigned char *)(&tags->u);
       __u32 size;

       size = min((__u32)(tags->hdr.size - 2) * sizeof(__u32), (__u32)MFG_GPIO_TABLE_MAX_SIZE);
       memcpy(mfg_gpio_table, dptr, size);
       return 0;
}
__tagtable(ATAG_MFG_GPIO_TABLE, parse_tag_mfg_gpio_table);

char * board_get_mfg_sleep_gpio_table(void)
{
        return mfg_gpio_table;
}
EXPORT_SYMBOL(board_get_mfg_sleep_gpio_table);

static char *emmc_tag;
static int __init board_set_emmc_tag(char *get_hboot_emmc)
{
	if (strlen(get_hboot_emmc))
		emmc_tag = get_hboot_emmc;
	else
		emmc_tag = NULL;
	return 1;
}
__setup("androidboot.emmc=", board_set_emmc_tag);

int board_emmc_boot(void)
{
	if (emmc_tag) {
		if (!strcmp(emmc_tag, "true"))
			return 1;
	}

	return 0;
}

#define ATAG_MEMSIZE 0x5441001e
unsigned memory_size;
int __init parse_tag_memsize(const struct tag *tags)
{
	int mem_size = 0, find = 0;
	struct tag *t = (struct tag *)tags;

	for (; t->hdr.size; t = tag_next(t)) {
		if (t->hdr.tag == ATAG_MEMSIZE) {
			printk(KERN_DEBUG "find the memsize tag\n");
			find = 1;
			break;
		}
	}

	if (find) {
		memory_size = t->u.revision.rev;
		mem_size = t->u.revision.rev;
	}
	printk(KERN_DEBUG "parse_tag_memsize: %d\n", memory_size);
	return mem_size;
}
__tagtable(ATAG_MEMSIZE, parse_tag_memsize);

int __init parse_tag_extdiag(const struct tag *tags)
{
	const struct tag *t = tags;

	for (; t->hdr.size; t = tag_next(t)) {
		if (t->hdr.tag == 0x54410022)
			return t->u.revision.rev;
	}
	return 0;
}

#if defined(CONFIG_ARCH_MSM8X60)
static struct msm_mem_settings *board_find_mem_settings(unsigned mem_size_mb)
{
	int index;
	for (index = 0; index < sizeof(mem_settings) / sizeof(mem_settings[0]); index++) {
		if (mem_settings[index].mem_size_mb == mem_size_mb) {
			pr_info("%s: %d MB settings is found.\n", __func__, mem_size_mb);
			return &mem_settings[index];
		}
	}
	pr_info("%s: use default mem bank settigs.\n", __func__);
	return &mem_settings[0];
}

int msm_fixup(struct tag *tags, struct meminfo *mi)
{
	unsigned mem_size_mb = parse_tag_memsize((const struct tag *)tags);
	struct msm_mem_settings *settings = board_find_mem_settings(mem_size_mb);
	int index = 0;

	pr_info("%s: mem size = %d\n", __func__, mem_size_mb);

	mi->nr_banks = settings->mem_info.nr_banks;
	for (index = 0; index < settings->mem_info.nr_banks; index++) {
		mi->bank[index].start = settings->mem_info.bank[index].start;
		mi->bank[index].size = settings->mem_info.bank[index].size;
	}
	return 0;
}
#endif

static unsigned int radio_flag = 0;
int __init radio_flag_init(char *s)
{
	radio_flag = simple_strtoul(s, 0, 16);
	return 1;
}
__setup("radioflag=", radio_flag_init);

unsigned int get_radio_flag(void)
{
	return radio_flag;
}

static unsigned long kernel_flag;
int __init kernel_flag_init(char *s)
{
	int ret;
	ret = strict_strtoul(s, 16, &kernel_flag);
	return 1;
}
__setup("kernelflag=", kernel_flag_init);

unsigned long get_kernel_flag(void)
{
	return kernel_flag;
}

BLOCKING_NOTIFIER_HEAD(psensor_notifier_list);

int register_notifier_by_psensor(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&psensor_notifier_list, nb);
}

int unregister_notifier_by_psensor(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&psensor_notifier_list, nb);
}

static char *sku_color_tag = NULL;
static int __init board_set_qwerty_color_tag(char *get_sku_color)
{
	if (strlen(get_sku_color))
		sku_color_tag = get_sku_color;
	else
		sku_color_tag = NULL;
	return 1;
}
__setup("androidboot.qwerty_color=", board_set_qwerty_color_tag);

void board_get_sku_color_tag(char **ret_data)
{
	*ret_data = sku_color_tag;
}
EXPORT_SYMBOL(board_get_sku_color_tag);
