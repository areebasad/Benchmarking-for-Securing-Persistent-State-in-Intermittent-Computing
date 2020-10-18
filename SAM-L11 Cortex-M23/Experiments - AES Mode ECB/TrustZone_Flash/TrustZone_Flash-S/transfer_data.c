
/*
 * transfer_data.c
 *
 * Created: 2019-03-05 13:39:41
 *  Author: erikw_000
 */ 
#include "transfer_data.h"

#include <atmel_start.h>
#include <stdlib.h>
#include <string.h>

#ifndef FLASH_PAGE_SIZE
#define FLASH_PAGE_SIZE 64
#endif

#ifndef NVMCTRL_FLASH_SIZE
#define NVMCTRL_FLASH_SIZE 65536
#endif

#ifndef NVMCTRL_ROW_PAGES
#define NVMCTRL_ROW_PAGES 4
#endif

#ifndef NVMCTRL_ROW_SIZE
#define NVMCTRL_ROW_SIZE (NVMCTRL_PAGE_SIZE * NVMCTRL_ROW_PAGES)
#endif

/**
 * \brief Get data from non secure world and store it in *secure_data
 */
void secure_store_data(uint8_t *data, size_t num_bytes) {
	// Check that the data is actually coming from the non-secure world (prevent from leaking secure data)
	data = cmse_check_address_range(data, num_bytes, CMSE_NONSECURE);
	// Save to flash
	// Put data at end of flash.
	uint32_t target_addr = FLASH_ADDR + FLASH_SIZE - num_bytes;
	target_addr -= target_addr % NVMCTRL_ROW_SIZE;
	
	for (uint32_t page_index = 0; page_index * FLASH_PAGE_SIZE < num_bytes; page_index += 1) {
		if (page_index % NVMCTRL_ROW_PAGES == 0) {
			/* Erase row in flash */
			FLASH_0_erase_row(target_addr + page_index * FLASH_PAGE_SIZE);
		}
		FLASH_0_write_page(target_addr + page_index * FLASH_PAGE_SIZE, &data[page_index * FLASH_PAGE_SIZE], FLASH_PAGE_SIZE);
		// TODO: decrease FLASH_PAGE_SIZE to correct number on last write if not multiple of FLASH_PAGE_SIZE.
	}
}

/**
 * \brief Return secure_data to non secure world
 */
void secure_load_data(uint8_t *data, size_t num_bytes) {
	// Check that the data is actually in the non-secure world (prevent from overwriting secure data)
	data = cmse_check_address_range(data, num_bytes, CMSE_NONSECURE);
	// Read from flash
	// Read data from end of flash.
	uint32_t target_addr = FLASH_ADDR + FLASH_SIZE - num_bytes;
	target_addr -= target_addr % NVMCTRL_ROW_SIZE;
	FLASH_0_read(target_addr, data, num_bytes);
}