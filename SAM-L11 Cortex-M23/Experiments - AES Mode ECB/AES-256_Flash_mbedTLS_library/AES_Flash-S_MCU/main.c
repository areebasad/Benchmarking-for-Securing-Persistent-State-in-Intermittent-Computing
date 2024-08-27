 #include <atmel_start.h>
#include <stdlib.h>
#include <string.h>

#include "mbedtls/aes.h"
#include "mbedtls/cmac.h"

#define STEP_SIZE MBEDTLS_AES_BLOCK_SIZE
#define MIN_NUM_BYTES STEP_SIZE
#define MAX_NUM_BYTES 6000 //6496
//#define MIN_NUM_BYTES STEP_SIZE*32
//#define MAX_NUM_BYTES STEP_SIZE*32+1 //6000 //6496

#define AES_KEY_SIZE 256

#define SLEEP

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

static mbedtls_aes_context aes;
static mbedtls_aes_context aes2;

#if(AES_KEY_SIZE == 128)
static const uint8_t key[16] = {
	0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81 };
#elif(AES_KEY_SIZE == 256)
static const uint8_t key[32] = {
	0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
	0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };
#else
#error Only 128 and 256 are supported for AES_KEY_SIZE
#endif

static uint8_t iv[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
static uint8_t iv2[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	
	mbedtls_aes_setkey_enc( &aes, key, AES_KEY_SIZE );
	mbedtls_aes_setkey_dec( &aes2, key, AES_KEY_SIZE );
	
	// Allocate MAX_AES_BLOCKS * MBEDTLS_AES_BLOCK_SIZE bytes.
	uint8_t *input = malloc(sizeof(uint8_t) * MAX_NUM_BYTES);
	
	delay_ms(10);
	
	for (size_t num_bytes = MIN_NUM_BYTES; num_bytes <= MAX_NUM_BYTES; num_bytes += STEP_SIZE) {
		//size_t num_bytes = MAX_AES_BLOCKS * MBEDTLS_AES_BLOCK_SIZE;
		// Fill with sequential data.
		for (size_t byte = 0; byte < num_bytes; byte++) {
			input[byte] = byte; // Will wrap at 0xff.
			//input[byte] = 0xfa;
		}

		// A. Encrypt
		START_MEASURE(DGI_GPIO2);
		mbedtls_aes_crypt_ecb( &aes, MBEDTLS_AES_ENCRYPT, input, input);
		STOP_MEASURE(DGI_GPIO2);
		
		// B. Write on flash
		START_MEASURE(DGI_GPIO3);
		uint32_t target_addr = FLASH_ADDR + FLASH_SIZE - num_bytes;
		target_addr -= target_addr % NVMCTRL_ROW_SIZE;
	
		for (uint32_t page_index = 0; page_index * FLASH_PAGE_SIZE < num_bytes; page_index += 1) {
			if (page_index % NVMCTRL_ROW_PAGES == 0) {
				/* Erase row in flash */
				FLASH_0_erase_row(target_addr + page_index * FLASH_PAGE_SIZE);
			}
			FLASH_0_write_page(target_addr + page_index * FLASH_PAGE_SIZE, &input[page_index * FLASH_PAGE_SIZE], FLASH_PAGE_SIZE);
			// TODO: decrease FLASH_PAGE_SIZE to correct number on last write if not multiple of FLASH_PAGE_SIZE.
		}
		STOP_MEASURE(DGI_GPIO3);
	
		SLEEP
	
		// Overwrite the memory
		for (size_t byte = 0; byte < num_bytes; byte++) {
			input[byte] = 0xfe;
		}
		
		START_MEASURE(DGI_GPIO3);
		// C. Read from flash
		FLASH_0_read(target_addr, input, num_bytes);
		STOP_MEASURE(DGI_GPIO3);
		
		START_MEASURE(DGI_GPIO2);
		// D. Decrypt in place.
		mbedtls_aes_crypt_ecb( &aes2, MBEDTLS_AES_DECRYPT, input, input);
		STOP_MEASURE(DGI_GPIO2);
	
		//// Check if memory has correct data
		//for (size_t byte = 0; byte < num_bytes; byte++) {
		//	if (output[byte] != byte % 0xff) {
		//		gpio_set_pin_level(DGI_GPIO2, GPIO_HIGH);
		//	}
		//}
	}
	
	// Free the memory
	free(input);

	END_MEASUREMENT;
}
