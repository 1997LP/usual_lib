/*
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * To match the U-Boot user interface on ARM platforms to the U-Boot
 * standard (as on PPC platforms), some messages with debug character
 * are removed from the default U-Boot build.
 *
 * Define DEBUG here if you want additional info as shown below
 * printed upon startup:
 *
 * U-Boot code: 00F00000 -> 00F3C774  BSS: -> 00FC3274
 * IRQ Stack: 00ebff7c
 * FIQ Stack: 00ebef7c
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <stdio_dev.h>
#include <timestamp.h>
#include <version.h>
#include <net.h>
#include <serial.h>
#include <nand.h>
#include <onenand_uboot.h>
#include <mmc.h>

#include "../../../drivers/mtd/nand/t18_bb_ops.h"
#include "inno_system_pll.h"


#ifdef CONFIG_BITBANGMII
#include <miiphy.h>
#endif

#ifdef CONFIG_DRIVER_SMC91111
#include "../drivers/net/smc91111.h"
#endif
#ifdef CONFIG_DRIVER_LAN91C96
#include "../drivers/net/lan91c96.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

ulong monitor_flash_len;
#ifdef CONFIG_SPI_FLASH
#include <spi_flash.h>
#endif

#ifdef CONFIG_HAS_DATAFLASH
extern int  AT91F_DataflashInit(void);
extern void dataflash_print_info(void);
#endif

#ifndef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING ""
#endif

const char version_string[] =
	U_BOOT_VERSION" (" U_BOOT_DATE " - " U_BOOT_TIME ")"CONFIG_IDENT_STRING;

#ifdef CONFIG_DRIVER_RTL8019
extern void rtl8019_get_enetaddr (uchar * addr);
#endif

#if defined(CONFIG_HARD_I2C) || \
    defined(CONFIG_SOFT_I2C)
#include <i2c.h>
#endif

int enable_nor_flash = 0;




#define   PWM_APB_SYS_BASE     (0x6000017C)
#define   PWM_GPIO_BASE        (0x60000110)
#define   PWM_FAN_BASE         (0x602f0000)


/************************************************************************
 * Coloured LED functionality
 ************************************************************************
 * May be supplied by boards if desired
 */
void inline __coloured_LED_init (void) {}
void coloured_LED_init (void) __attribute__((weak, alias("__coloured_LED_init")));
void inline __red_LED_on (void) {}
void red_LED_on (void) __attribute__((weak, alias("__red_LED_on")));
void inline __red_LED_off(void) {}
void red_LED_off(void) __attribute__((weak, alias("__red_LED_off")));
void inline __green_LED_on(void) {}
void green_LED_on(void) __attribute__((weak, alias("__green_LED_on")));
void inline __green_LED_off(void) {}
void green_LED_off(void) __attribute__((weak, alias("__green_LED_off")));
void inline __yellow_LED_on(void) {}
void yellow_LED_on(void) __attribute__((weak, alias("__yellow_LED_on")));
void inline __yellow_LED_off(void) {}
void yellow_LED_off(void) __attribute__((weak, alias("__yellow_LED_off")));
void inline __blue_LED_on(void) {}
void blue_LED_on(void) __attribute__((weak, alias("__blue_LED_on")));
void inline __blue_LED_off(void) {}
void blue_LED_off(void) __attribute__((weak, alias("__blue_LED_off")));

/************************************************************************
 * Init Utilities							*
 ************************************************************************
 * Some of this code should be moved into the core functions,
 * or dropped completely,
 * but let's get it working (again) first...
 */

#if defined(CONFIG_ARM_DCC) && !defined(CONFIG_BAUDRATE)
#define CONFIG_BAUDRATE 115200
#endif

#if  0 //comment by luozl 20200602
static int init_baudrate (void)
{
	char tmp[64];	/* long enough for environment variables */
	int i = getenv_f("baudrate", tmp, sizeof (tmp));
	gd->bd->bi_baudrate = gd->baudrate = (i > 0)
			? (int) simple_strtoul (tmp, NULL, 10)
			: CONFIG_BAUDRATE;

	return (0);
}
#else
static int init_baudrate (void)
{
	
	gd->bd->bi_baudrate = gd->baudrate = 115200;
	return (0);
}
#endif

static save_uboot_version()
{
 char* uboot_ver = NULL;
 
 uboot_ver = getenv("uboot_ver");
 if ((!uboot_ver) || 
    (uboot_ver && memcmp(version_string,uboot_ver,sizeof(version_string)))) {
     setenv("uboot_ver",version_string);
     saveenv();
 }
}

static int display_banner (void)
{
	printf ("%s\n\n", version_string);
	debug ("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	       _armboot_start, _bss_start, _bss_end);
#ifdef CONFIG_MODEM_SUPPORT
	debug ("Modem Support enabled\n");
#endif
#ifdef CONFIG_USE_IRQ
	debug ("IRQ Stack: %08lx\n", IRQ_STACK_START);
	debug ("FIQ Stack: %08lx\n", FIQ_STACK_START);
#endif

	return (0);
}

/*
 * WARNING: this code looks "cleaner" than the PowerPC version, but
 * has the disadvantage that you either get nothing, or everything.
 * On PowerPC, you might see "DRAM: " before the system hangs - which
 * gives a simple yet clear indication which part of the
 * initialization if failing.
 */
static int display_dram_config (void)
{
	int i;

#ifdef DEBUG
	puts ("RAM Configuration:\n");

	for(i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
		printf ("Bank #%d: %08lx ", i, gd->bd->bi_dram[i].start);
		print_size (gd->bd->bi_dram[i].size, "\n");
	}
#else
	ulong size = 0;

	for (i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->bd->bi_dram[i].size;
	}
	puts("DRAM:  ");
	print_size(size, "\n");
#endif

	return (0);
}

static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
		"bne 1b":"=r" (loops):"0" (loops));
}

#ifndef CONFIG_SYS_NO_FLASH
static void display_flash_config (ulong size)
{
	puts ("Flash: ");
	print_size (size, "\n");
}
#endif /* CONFIG_SYS_NO_FLASH */

#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
extern int bf3710_regs_init(void);
extern int mt9m111_sensor_config( void);

static int init_func_i2c (void)
{
	puts ("I2C:   ");
	//i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	puts ("ready\n");
	return (0);
}
#endif

#if defined(CONFIG_CMD_PCI) || defined (CONFIG_PCI)
#include <pci.h>
static int arm_pci_init(void)
{
	pci_init();
	return 0;
}
#endif /* CONFIG_CMD_PCI || CONFIG_PCI */

/*
 * Breathe some life into the board...
 *
 * Initialize a serial port as console, and carry out some hardware
 * tests.
 *
 * The first part of initialization is running from Flash memory;
 * its main purpose is to initialize the RAM so that we
 * can relocate the monitor code to RAM.
 */

/*
 * All attempts to come up with a "common" initialization sequence
 * that works for all boards and architectures failed: some of the
 * requirements are just _too_ different. To get rid of the resulting
 * mess of board dependent #ifdef'ed code we now make the whole
 * initialization sequence configurable to the user.
 *
 * The requirements for any new initalization function is simple: it
 * receives a pointer to the "global data" structure as it's only
 * argument, and returns an integer return code, where 0 means
 * "continue" and != 0 means "fatal error, hang the system".
 */
typedef int (init_fnc_t) (void);

int print_cpuinfo (void);

init_fnc_t *init_sequence[] = {
#if defined(CONFIG_ARCH_CPU_INIT)
	arch_cpu_init,		/* basic arch cpu dependent setup */
#endif
	board_init,		/* basic board dependent setup */
#if defined(CONFIG_USE_IRQ)
	interrupt_init,		/* set up exceptions */
#endif
	//timer_init,		/* initialize timer */
#ifdef CONFIG_FSL_ESDHC
	get_clocks,
#endif
	//env_init,		/* initialize environment */
	init_baudrate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */
	console_init_f,		/* stage 1 init of console */
	display_banner,		/* say that we are here */
#if defined(CONFIG_DISPLAY_CPUINFO)
	print_cpuinfo,		/* display cpu info (and speed) */
#endif
#if defined(CONFIG_DISPLAY_BOARDINFO)
	checkboard,		/* display board info */
#endif
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
	init_func_i2c,
#endif
	//dram_init,		/* configure available RAM banks */
#if defined(CONFIG_CMD_PCI) || defined (CONFIG_PCI)
	arm_pci_init,
#endif
	//display_dram_config,
	NULL,
};

typedef unsigned int u32;
typedef unsigned long long  u64;
//#define TLB_ADDR   (*(volatile unsigned int *)( 0xfc0000) 
#define TLB_ADDR     (0xc0000)
#define CONFIG_SYS_PCIE1_PHYS_BASE		0x1000000000ULL
#define CONFIG_SYS_PCIE2_PHYS_BASE		0x1800000000ULL
#define CONFIG_SYS_PCIE1_VIRT_ADDR		0x14000000UL   //0x24000000
#define CONFIG_SYS_PCIE2_VIRT_ADDR		0x34000000UL   //0x34000000
#define CONFIG_SYS_PCIE1_MMAP_SIZE		(192 * 1024 * 1024) /* 192M */
#define CONFIG_SYS_PCIE2_MMAP_SIZE		(1 * 1024 * 1024) /* 1M */



#define PMD_TYPE_TABLE		0x3
#define PMD_TYPE_SECT		0x1

/* AttrIndx[2:0] */
#define PMD_ATTRINDX(t)		((t) << 2)

/* Section */
#define PMD_SECT_AF		(1 << 10)

#define BLOCK_SIZE_L1		(1UL << 30)
#define BLOCK_SIZE_L2		(1UL << 21)

/* TTBCR flags */
#define TTBCR_EAE		(1 << 31)
#define TTBCR_T0SZ(x)		((x) << 0)
#define TTBCR_T1SZ(x)		((x) << 16)
#define TTBCR_USING_TTBR0	(TTBCR_T0SZ(0) | TTBCR_T1SZ(0))
#define TTBCR_IRGN0_NC		(0 << 8)
#define TTBCR_IRGN0_WBWA	(1 << 8)
#define TTBCR_IRGN0_WT		(2 << 8)
#define TTBCR_IRGN0_WBNWA	(3 << 8)
#define TTBCR_IRGN0_MASK	(3 << 8)
#define TTBCR_ORGN0_NC		(0 << 10)
#define TTBCR_ORGN0_WBWA	(1 << 10)
#define TTBCR_ORGN0_WT		(2 << 10)
#define TTBCR_ORGN0_WBNWA	(3 << 10)
#define TTBCR_ORGN0_MASK	(3 << 10)
#define TTBCR_SHARED_NON	(0 << 12)
#define TTBCR_SHARED_OUTER	(2 << 12)
#define TTBCR_SHARED_INNER	(3 << 12)
#define TTBCR_EPD0		(0 << 7)
#define TTBCR			(TTBCR_SHARED_NON | \
				 TTBCR_ORGN0_NC	| \
				 TTBCR_IRGN0_NC	| \
				 TTBCR_USING_TTBR0 | \
				 TTBCR_EAE)


#define MT_MAIR0		0xeeaa4400
#define MT_MAIR1		0xff000004
#define MT_STRONLY_ORDER	0
#define MT_NORMAL_NC		1
#define MT_DEVICE_MEM		4
#define MT_NORMAL		7

#define CR_M		(1 << 0)	/* MMU enable			*/
#define CR_A		(1 << 1)	/* Alignment abort enable	*/
#define CR_C		(1 << 2)	/* Dcache enable		*/

static inline unsigned int get_cr(void)
{
	unsigned int val;

    /*
	if (is_hyp())
		asm volatile("mrc p15, 4, %0, c1, c0, 0	@ get CR" : "=r" (val)
								  :
								  : "cc");
	else
	*/
		asm volatile("mrc p15, 0, %0, c1, c0, 0	@ get CR" : "=r" (val)
								  :
								  : "cc");
	return val;
}

static inline void set_cr(unsigned int val)
{
	/*
	if (is_hyp())
		asm volatile("mcr p15, 4, %0, c1, c0, 0	@ set CR" :
								  : "r" (val)
								  : "cc");
	else
	*/
		asm volatile("mcr p15, 0, %0, c1, c0, 0	@ set CR" :
								  : "r" (val)
								  : "cc");
	//isb();
}


/* The phy_addr must be aligned to 4KB */
static inline void set_pgsection(u32 *page_table, u32 index, u64 phy_addr,
				 u32 memory_type)
{
	u64 value;

	value = phy_addr | PMD_TYPE_SECT | PMD_SECT_AF;
	value |= PMD_ATTRINDX(memory_type);
	page_table[2 * index] = value & 0xFFFFFFFF;
	page_table[2 * index + 1] = (value >> 32) & 0xFFFFFFFF;
}

static inline void set_pgtable(u32 *page_table, u32 index, u32 phy_addr)
{
	u32 value = phy_addr | PMD_TYPE_TABLE;

	page_table[2 * index] = value;
	page_table[2 * index + 1] = 0;
}


void mmu_setup2(void)
{
	//return 0;
	
	//u32 *level0_table = (u32 *)gd->arch.tlb_addr;
	//u32 *level1_table = (u32 *)(gd->arch.tlb_addr + 0x1000);
	u32 *level0_table = (u32 *)TLB_ADDR;
	u32 *level1_table = (u32 *)(TLB_ADDR + 0x1000);
	u64 va_start = 0;
	u32 reg;
	int i;
	

	/* Level 0 Table 2-3 are used to map DDR */
	set_pgsection(level0_table, 3, 3 * BLOCK_SIZE_L1, MT_NORMAL);
	set_pgsection(level0_table, 2, 2 * BLOCK_SIZE_L1, MT_NORMAL);
	/* Level 0 Table 1 is used to map device */
	set_pgsection(level0_table, 1, 1 * BLOCK_SIZE_L1, MT_DEVICE_MEM);
	/* Level 0 Table 0 is used to map device including PCIe MEM */
	set_pgtable(level0_table, 0, (u32)level1_table);

	/* Level 1 has 512 entries */
	for (i = 0; i < 512; i++) {
		/* Mapping for PCIe 1 */
		if (va_start >= CONFIG_SYS_PCIE1_VIRT_ADDR &&
		    va_start < (CONFIG_SYS_PCIE1_VIRT_ADDR +
				 CONFIG_SYS_PCIE1_MMAP_SIZE))
			set_pgsection(level1_table, i,
				      CONFIG_SYS_PCIE1_PHYS_BASE + va_start,
				      MT_DEVICE_MEM);
		/* Mapping for PCIe 2 */
		else if (va_start >= CONFIG_SYS_PCIE2_VIRT_ADDR &&
			 va_start < (CONFIG_SYS_PCIE2_VIRT_ADDR +
				     CONFIG_SYS_PCIE2_MMAP_SIZE))
			set_pgsection(level1_table, i,
				      CONFIG_SYS_PCIE2_PHYS_BASE + va_start,
				      MT_DEVICE_MEM);
		else
			set_pgsection(level1_table, i,
				      va_start,
				      MT_DEVICE_MEM);
		va_start += BLOCK_SIZE_L2;
	}

	//asm volatile("dsb sy;isb");
	asm volatile("mcr p15, 0, %0, c2, c0, 2" /* Write RT to TTBCR */
			: : "r" (TTBCR) : "memory");
	asm volatile("mcrr p15, 0, %0, %1, c2" /* TTBR 0 */
			: : "r" ((u32)level0_table), "r" (0) : "memory");
	asm volatile("mcr p15, 0, %0, c10, c2, 0" /* write MAIR 0 */
			: : "r" (MT_MAIR0) : "memory");
	asm volatile("mcr p15, 0, %0, c10, c2, 1" /* write MAIR 1 */
			: : "r" (MT_MAIR1) : "memory");

	/* Set the access control to all-supervisor */
	asm volatile("mcr p15, 0, %0, c3, c0, 0"
		     : : "r" (~0));

	/* Enable the mmu */
	reg = get_cr();
	set_cr(reg | CR_M);
}

void start_armboot (void)
{
	init_fnc_t **init_fnc_ptr;
	char *s;
#if defined(CONFIG_VFD) || defined(CONFIG_LCD)
	unsigned long addr;
#endif
	mmu_setup2();
	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t*)(_armboot_start - CONFIG_SYS_MALLOC_LEN - sizeof(gd_t));
	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("": : :"memory");

	memset ((void*)gd, 0, sizeof (gd_t));
	gd->bd = (bd_t*)((char*)gd - sizeof(bd_t));
	memset (gd->bd, 0, sizeof (bd_t));

	gd->flags |= GD_FLG_RELOC;

	monitor_flash_len = _bss_start - _armboot_start;

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0) {
			hang ();
		}
	}
	 puts("hello,luozl\n");
	 puts("no mmu_setup2\n");
	/* armboot_start is defined in the board-specific linker script */
	mem_malloc_init (_armboot_start - CONFIG_SYS_MALLOC_LEN,
			CONFIG_SYS_MALLOC_LEN);

#ifndef CONFIG_SYS_NO_FLASH
	/* configure available FLASH banks */
	display_flash_config (flash_init ());
#endif /* CONFIG_SYS_NO_FLASH */

#ifdef CONFIG_VFD
#	ifndef PAGE_SIZE
#	  define PAGE_SIZE 4096
#	endif
	/*
	 * reserve memory for VFD display (always full pages)
	 */
	/* bss_end is defined in the board-specific linker script */
	addr = (_bss_end + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
	vfd_setmem (addr);
	gd->fb_base = addr;
#endif /* CONFIG_VFD */

#ifdef CONFIG_LCD
	/* board init may have inited fb_base */
	if (!gd->fb_base) {
#		ifndef PAGE_SIZE
#		  define PAGE_SIZE 4096
#		endif
		/*
		 * reserve memory for LCD display (always full pages)
		 */
		/* bss_end is defined in the board-specific linker script */
		addr = (_bss_end + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
		lcd_setmem (addr);
		gd->fb_base = addr;
	}
#endif /* CONFIG_LCD */

#if defined(CONFIG_CMD_NAND)
	puts ("NAND:  ");
	nand_init();		/* go init the NAND */
    #ifdef USE_BBT
    bbt_cache_init();
    #endif
#endif

#if defined(CONFIG_CMD_ONENAND)
	onenand_init();
#endif

#ifdef CONFIG_HAS_DATAFLASH
	AT91F_DataflashInit();
	dataflash_print_info();
#endif
#ifdef CONFIG_SPI_FLASH
	puts("SF probe: Skip\n");
/*
	flash = spi_flash_probe(0, 0, CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);
	if (!flash) {
		printf("Not Found\n");
	}
*/
	//spi_init();
#endif

#ifdef CONFIG_GENERIC_MMC
/*
 * MMC initialization is called before relocating env.
 * Thus It is required that operations like pin multiplexer
 * be put in board_init.
 */
	puts ("MMC:   ");
	mmc_initialize (gd->bd);
#endif
	puts("env_relocate\n");
	/* initialize environment */
	env_relocate ();
#ifdef CONFIG_VFD
	/* must do this after the framebuffer is allocated */
	drv_vfd_init();
#endif /* CONFIG_VFD */
	//puts("serial_initialize\n");
#ifdef CONFIG_SERIAL_MULTI
	serial_initialize();
#endif
	/* IP Address */
	gd->bd->bi_ip_addr = getenv_IPaddr ("ipaddr");
	stdio_init ();	/* get the devices list going. */
	jumptable_init ();

#if defined(CONFIG_API)
	/* Initialize API */
	api_init ();
#endif
	//puts ("jumptable_init end\n");

	console_init_r ();	/* fully init console as a device */
	puts("3\n");
//	puts ("jumptable_init end\n");
#if defined(CONFIG_ARCH_MISC_INIT)
	/* miscellaneous arch dependent initialisations */
	arch_misc_init ();
#endif
#if defined(CONFIG_MISC_INIT_R)
	/* miscellaneous platform dependent initialisations */
	misc_init_r ();
#endif

	/* enable exceptions */
	enable_interrupts ();

	/* Perform network card initialisation if necessary */
#ifdef CONFIG_DRIVER_TI_EMAC
	/* XXX: this needs to be moved to board init */
extern void davinci_eth_set_mac_addr (const u_int8_t *addr);
	if (getenv ("ethaddr")) {
		uchar enetaddr[6];
		eth_getenv_enetaddr("ethaddr", enetaddr);
		davinci_eth_set_mac_addr(enetaddr);
	}
#endif

#if defined(CONFIG_DRIVER_SMC91111) || defined (CONFIG_DRIVER_LAN91C96)
	/* XXX: this needs to be moved to board init */
	if (getenv ("ethaddr")) {
		uchar enetaddr[6];
		eth_getenv_enetaddr("ethaddr", enetaddr);
		smc_set_mac_addr(enetaddr);
	}
#endif /* CONFIG_DRIVER_SMC91111 || CONFIG_DRIVER_LAN91C96 */

	/* Initialize from environment */
	if ((s = getenv ("loadaddr")) != NULL) {
		load_addr = simple_strtoul (s, NULL, 16);
	}
#if defined(CONFIG_CMD_NET)
	if ((s = getenv ("bootfile")) != NULL) {
		copy_filename (BootFile, s, sizeof (BootFile));
	}
#endif

#ifdef BOARD_LATE_INIT
	board_late_init ();
#endif

#ifdef CONFIG_BITBANGMII
	bb_miiphy_init();
#endif
#if defined(CONFIG_CMD_NET)
#if defined(CONFIG_NET_MULTI)
	puts ("Net:   ");
#endif
	eth_initialize(gd->bd);
#if defined(CONFIG_RESET_PHY_R)
	debug ("Reset Ethernet PHY\n");
	reset_phy();
#endif
#endif
#ifdef __G1__SOC__
	system_pll_init();  //set g1 soc system cpu/vpu/gpu... module clk
#endif
	//extern void set_all_led_off(void);
	//set_all_led_off();
    //save_uboot_version();
	/* main_loop() can return to retry autoboot, if so just run it again. */
	printf("enter to main_loop\n");
	//(*(volatile unsigned int *)(0x120000 + 0x08)) = 0x56;
	for (;;) {
		main_loop ();
	}

	/* NOTREACHED - no way out of command loop except booting */
}

void hang (void)
{
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;);
}
