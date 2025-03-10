#include <common.h>
#include <jffs2/jffs2.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include <atheros.h>
#include "ath_flash.h"

#if !defined(ATH_DUAL_FLASH)
#	define	ath_spi_flash_print_info	flash_print_info
#endif

/*
 * globals
 */
flash_info_t flash_info[CFG_MAX_FLASH_BANKS];

/*
 * statics
 */
static void ath_spi_write_enable(void);
static void ath_spi_poll(void);
#if !defined(ATH_SST_FLASH)
static void ath_spi_write_page(uint32_t addr, uint8_t * data, int len);
#endif
static void ath_spi_sector_erase(uint32_t addr);

static void
ath_spi_read_id(void)
{
	u32 rd;

	ath_reg_wr_nf(ATH_SPI_WRITE, ATH_SPI_CS_DIS);
	ath_spi_bit_banger(ATH_SPI_CMD_RDID);
	ath_spi_delay_8();
	ath_spi_delay_8();
	ath_spi_delay_8();
	ath_spi_go();

	rd = ath_reg_rd(ATH_SPI_RD_STATUS);

	printf("Flash Manuf Id 0x%x, DeviceId0 0x%x, DeviceId1 0x%x\n",
		(rd >> 16) & 0xff, (rd >> 8) & 0xff, (rd >> 0) & 0xff);
}


#ifdef ATH_SST_FLASH
void ath_spi_flash_unblock(void)
{
	ath_spi_write_enable();
	ath_spi_bit_banger(ATH_SPI_CMD_WRITE_SR);
	ath_spi_bit_banger(0x0);
	ath_spi_go();
	ath_spi_poll();
}
#endif

unsigned long flash_init(void)
{
#if !(defined(CONFIG_WASP_SUPPORT) || defined(CONFIG_MACH_QCA955x) || defined(CONFIG_MACH_QCA953x))
#ifdef ATH_SST_FLASH
	ath_reg_wr_nf(ATH_SPI_CLOCK, 0x3);
	ath_spi_flash_unblock();
	ath_reg_wr(ATH_SPI_FS, 0);
#else
	ath_reg_wr_nf(ATH_SPI_CLOCK, 0x43);
#endif
#endif
	ath_reg_rmw_set(ATH_SPI_FS, 1);
	ath_spi_read_id();
	ath_reg_rmw_clear(ATH_SPI_FS, 1);

	/*
	 * hook into board specific code to fill flash_info
	 */
	return (flash_get_geom(&flash_info[0]));
}

void
ath_spi_flash_print_info(flash_info_t *info)
{
	printf("The hell do you want flinfo for??\n");
}

int
flash_erase(flash_info_t *info, int s_first, int s_last)
{
	int i, sector_size = info->size / info->sector_count;

	printf("\nFirst %#x last %#x sector size %#x\n",
		s_first, s_last, sector_size);

	for (i = s_first; i <= s_last; i++) {
		printf("\b\b\b\b%4d", i);
		ath_spi_sector_erase(i * sector_size);
	}
	ath_spi_done();
	printf("\n");

	return 0;
}

/*
 * Write a buffer from memory to flash:
 * 0. Assumption: Caller has already erased the appropriate sectors.
 * 1. call page programming for every 256 bytes
 */
#ifdef ATH_SST_FLASH
void
ath_spi_flash_chip_erase(void)
{
	ath_spi_write_enable();
	ath_spi_bit_banger(ATH_SPI_CMD_CHIP_ERASE);
	ath_spi_go();
	ath_spi_poll();
}

int
write_buff(flash_info_t *info, uchar *src, ulong dst, ulong len)
{
	uint32_t val;

	dst = dst - CFG_FLASH_BASE;
	printf("write len: %lu dst: 0x%x src: %p\n", len, dst, src);

	for (; len; len--, dst++, src++) {
		ath_spi_write_enable();	// dont move this above 'for'
		ath_spi_bit_banger(ATH_SPI_CMD_PAGE_PROG);
		ath_spi_send_addr(dst);

		val = *src & 0xff;
		ath_spi_bit_banger(val);

		ath_spi_go();
		ath_spi_poll();
	}
	/*
	 * Disable the Function Select
	 * Without this we can't read from the chip again
	 */
	ath_reg_wr(ATH_SPI_FS, 0);

	if (len) {
		// how to differentiate errors ??
		return ERR_PROG_ERROR;
	} else {
		return ERR_OK;
	}
}
#else
int
write_buff(flash_info_t *info, uchar *source, ulong addr, ulong len)
{
	int total = 0, len_this_lp, bytes_this_page;
	ulong dst;
	uchar *src;

	printf("write addr: %x\n", addr);
	addr = addr - CFG_FLASH_BASE;

	while (total < len) {
		src = source + total;
		dst = addr + total;
		bytes_this_page =
			ATH_SPI_PAGE_SIZE - (addr % ATH_SPI_PAGE_SIZE);
		len_this_lp =
			((len - total) >
			bytes_this_page) ? bytes_this_page : (len - total);
		ath_spi_write_page(dst, src, len_this_lp);
		total += len_this_lp;
	}

	ath_spi_done();

	return 0;
}
#endif

static void
ath_spi_write_enable()
{
	ath_reg_wr_nf(ATH_SPI_FS, 1);
	ath_reg_wr_nf(ATH_SPI_WRITE, ATH_SPI_CS_DIS);
	ath_spi_bit_banger(ATH_SPI_CMD_WREN);
	ath_spi_go();
}

static void
ath_spi_poll()
{
	int rd;

	do {
		ath_reg_wr_nf(ATH_SPI_WRITE, ATH_SPI_CS_DIS);
		ath_spi_bit_banger(ATH_SPI_CMD_RD_STATUS);
		ath_spi_delay_8();
		rd = (ath_reg_rd(ATH_SPI_RD_STATUS) & 1);
	} while (rd);
}

#if !defined(ATH_SST_FLASH)
static void
ath_spi_write_page(uint32_t addr, uint8_t *data, int len)
{
	int i;
	uint8_t ch;

	display(0x77);
	ath_spi_write_enable();
	ath_spi_bit_banger(ATH_SPI_CMD_PAGE_PROG);
	ath_spi_send_addr(addr);

	for (i = 0; i < len; i++) {
		ch = *(data + i);
		ath_spi_bit_banger(ch);
	}

	ath_spi_go();
	display(0x66);
	ath_spi_poll();
	display(0x6d);
}
#endif

static void
ath_spi_sector_erase(uint32_t addr)
{
	ath_spi_write_enable();
	ath_spi_bit_banger(ATH_SPI_CMD_SECTOR_ERASE);
	ath_spi_send_addr(addr);
	ath_spi_go();
	display(0x7d);
	ath_spi_poll();
}

#ifdef ATH_DUAL_FLASH
void flash_print_info(flash_info_t *info)
{
	ath_spi_flash_print_info(NULL);
	ath_nand_flash_print_info(NULL);
}
#endif

#ifdef FW_RECOVERY/*  by huangwenzhong, 03May13 */

/******************************************************************************
* FUNCTION      	: ar7240_auf_gpio_init()
* AUTHOR        	: HouXB <houxubo@tp-link.net>
* DESCRIPTION  		: set input and output indicator gpio, when auto upload firmware 
* INPUT				: 
*
* OUTPUT        	: 
* RETURN        	: 
* OTHERS        	: 
******************************************************************************/
static void ath_gpio_set_val(int reg, int gpio, int val)
{
	if (val & 0x1) {
		ath_reg_rmw_set(reg, (1 << gpio));
	} else {
		ath_reg_rmw_clear(reg, (1 << gpio));
	}
}

void ath_auf_gpio_init()
{
	/* use reset button as input indicator */
	ath_gpio_set_val(GPIO_OE_ADDRESS, FW_BUTTON_GPIO, 1);
	
	/* use wps led as output indicator */
	ath_gpio_set_val(GPIO_OE_ADDRESS, FW_LED_GPIO, 0);	
}

/******************************************************************************
* FUNCTION      	: ar7240_is_rst_btn_pressed()
* AUTHOR        	: HouXB <houxubo@tp-link.net>
* DESCRIPTION  		: check whether the reset button was pressed 
* INPUT				: 
*
* OUTPUT        	: 
* RETURN        	: 1, pressed; 0, not pressed
* OTHERS        	: 
******************************************************************************/
int ath_is_rst_btn_pressed()
{
	int val;
	int old_val;

	udelay(10 * 1000); /* delay 1ms for input value stabile. by HouXB, 27Apr11 */
	old_val = ath_reg_rd(GPIO_IN_ADDRESS);
	udelay(1000);
	val = ath_reg_rd(GPIO_IN_ADDRESS);

	/* make sure the btn was pressed. by HouXB, 27Apr11 */
	if(old_val != val)
	{
		return 0;
	}
	val = ((val & (1 << FW_BUTTON_GPIO)) >> FW_BUTTON_GPIO);
	/* when pressed val is 0, return 1 to indicate pressed */
	return (1 - val);
}

/******************************************************************************
* FUNCTION      	: ar7240_usb_led_on()
* AUTHOR        	: HouXB <houxubo@tp-link.net>
* DESCRIPTION  		: auto upload firmware output indicator 
* INPUT				: 
*
* OUTPUT        	: 
* RETURN        	: 
* OTHERS        	: 
******************************************************************************/
void ath_fw_led_on()
{	
	ath_gpio_set_val(GPIO_OUT_ADDRESS, FW_LED_GPIO, FW_LED_ON);
}

void ath_fw_led_off()
{
	ath_gpio_set_val(GPIO_OUT_ADDRESS, FW_LED_GPIO, FW_LED_OFF);
}
#endif

