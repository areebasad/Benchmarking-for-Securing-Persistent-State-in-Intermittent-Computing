/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "driver_examples.h"
#include "driver_init.h"
#include "utils.h"

#ifndef NVMCTRL_ROW_SIZE
#define NVMCTRL_ROW_SIZE (NVMCTRL_PAGE_SIZE * NVMCTRL_ROW_PAGES)
#endif
static uint8_t flash_src_data[FLASH_PAGE_SIZE];
static uint8_t flash_chk_data[FLASH_PAGE_SIZE];

/**
 * Example of using FLASH_0 to read and write buffer.
 */
void FLASH_0_example(void)
{
	uint32_t i;

	/* Target address.
	 * Make sure the address is reasonable to your application.
	 * It might a address in NVM memory of FLASH, DATAFLASH, AUX FLASH.
	 */
	uint32_t target_addr = FLASH_ADDR + FLASH_SIZE - NVMCTRL_ROW_SIZE;

	/* Initialize source data */
	for (i = 0; i < FLASH_PAGE_SIZE; i++) {
		flash_src_data[i] = i;
	}

	/* Erase row in flash */
	FLASH_0_erase_row(target_addr);

	/* Write data to flash */
	FLASH_0_write_page(target_addr, flash_src_data, FLASH_PAGE_SIZE);

	/* Read data from flash */
	FLASH_0_read(target_addr, flash_chk_data, FLASH_PAGE_SIZE);

	/* Check data */
	for (i = 0; i < FLASH_PAGE_SIZE; i++) {
		if (flash_src_data[i] != flash_chk_data[i]) {
			while (1)
				; /* Error happen */
		}
	}
}
