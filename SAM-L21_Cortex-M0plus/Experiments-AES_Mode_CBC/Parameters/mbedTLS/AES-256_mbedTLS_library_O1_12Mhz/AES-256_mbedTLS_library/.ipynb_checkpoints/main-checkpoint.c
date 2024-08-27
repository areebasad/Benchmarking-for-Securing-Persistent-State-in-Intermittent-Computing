/**
************************************************************************
 Author		: Hafiz Areeb Asad
 Date		: 29th April, 2020
 Description: Encryption/Decryption using mbedTLS library (Software). 
	GPIO pins used to measure energy consumption and time taken, 
	each by encrypt, decrypt, writing on flash and reading from flash.
	Help and code snippets taken from mbedtLS/Atmel documentation
	and from Erik's Code.                                                                     
************************************************************************

* Support: 
*/

#include "atmel_start.h"
#include <hal_gpio.h>
#include <hal_delay.h>
#include <stdlib.h> //malloc
#include "mbedtls/aes.h"
#include "mbedtls/cmac.h"

#define STEP_SIZE 16
#define MIN_NUM_BYTES STEP_SIZE
#define MAX_NUM_BYTES 6000 //6000 //6496 //6144

#define AES_KEY_SIZE 256

#define SLEEP delay_ms(2);

#ifndef FLASH_PAGE_SIZE
#define FLASH_PAGE_SIZE 64
#endif

#ifndef NVMCTRL_FLASH_SIZE
#define NVMCTRL_FLASH_SIZE 262144 //256 KB
#endif

#ifndef NVMCTRL_ROW_PAGES
#define NVMCTRL_ROW_PAGES 4
#endif

#ifndef NVMCTRL_ROW_SIZE
#define NVMCTRL_ROW_SIZE (NVMCTRL_PAGE_SIZE * NVMCTRL_ROW_PAGES)
#endif

static mbedtls_aes_context aes;
static mbedtls_aes_context aes2;

static const uint8_t key_128[16] = {
0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81 };

const const uint8_t key_256[32] = {
	0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
	0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4
};

static uint8_t iv[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
static uint8_t iv2[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f }; /*(This is same but needed for decryption. Note: AES Module was not doing decryption without iv2  )*/


void aes_measurement(void)
{
	/* Example Vectors From FIPS-197:-
	                 PLAINTEXT: 00112233445566778899aabbccddeeff
	                 KEY: 000102030405060708090a0b0c0d0e0f
	                 CIPHER: 69c4e0d86a7b0430d8cdb78070b4c55a	*/	
	/*
	const uint8_t plaintext[] = {
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
	};*/
	
	
	mbedtls_aes_setkey_enc( &aes, key_256, AES_KEY_SIZE );
	mbedtls_aes_setkey_dec( &aes2, key_256, AES_KEY_SIZE );
	
	// Allocate buffer memory
	uint8_t *input = malloc(sizeof(uint8_t) * MAX_NUM_BYTES);
	//uint8_t input[MAX_NUM_BYTES];
	
	// Allocate buffer memory
	uint8_t *output = malloc(sizeof(uint8_t) * MAX_NUM_BYTES);
	//uint8_t input[MAX_NUM_BYTES];

    delay_ms(10);

	for (size_t num_bytes = MIN_NUM_BYTES; num_bytes <= MAX_NUM_BYTES; num_bytes += STEP_SIZE) {
		
		// Fill with sequential data.
		for (size_t byte = 0; byte < num_bytes; byte++) {
			input[byte] = byte; // 0x00 to write 0's OR 0xFF to write 1's.
			//input[byte] = 0xfa;
		}
			

		// Start encryption
		START_MEASURE(DGI_GPIO2);
		//io_write(terminal_io, "Encryption", sizeof(uint8_t)*10);
		/*encrypt*/		
		mbedtls_aes_crypt_cbc( &aes, MBEDTLS_AES_ENCRYPT, num_bytes, iv, input, input);
		STOP_MEASURE(DGI_GPIO2);
		
		/* Save to flash
		   Put data at end of flash.
	    **/		
		START_MEASURE(DGI_GPIO3);
		//********************************************** Solution - 1 @ To write on emulation (RWW) Area
		if (_rww_flash_write(&FLASH_0.dev, NVMCTRL_RWW_EEPROM_ADDR, input, num_bytes ) != ERR_NONE) {
			while (1)
			; /* Trap here when flash write error happen */
		}
		
		//*********************************************  Solution - 2 @ To write on Flash Main Application Area
		
		//--------------------------------------------To write the whole data ----------------------------- 
		//uint32_t target_addr = FLASH_ADDR + FLASH_SIZE - num_bytes;
		//target_addr -= target_addr % NVMCTRL_ROW_SIZE;
	
		//flash_write(&FLASH_0, target_addr, output, num_bytes);
		
		// ------------------------------------------- To write page by page use the code below (Avoid loops)---------
		//for (uint32_t page_index = 0; page_index * FLASH_PAGE_SIZE < num_bytes; page_index += 1) {
			/* Write data to flash */
			//flash_write(&FLASH_0, target_addr + page_index * FLASH_PAGE_SIZE, &input[page_index * FLASH_PAGE_SIZE], FLASH_PAGE_SIZE);
		//}
		STOP_MEASURE(DGI_GPIO3);
		
		SLEEP
				
		// Overwrite the memory
		for (size_t byte = 0; byte < num_bytes; byte++) {
			input[byte] = 0xfe;
		}

		// Start reading from flash
		START_MEASURE(DGI_GPIO3);
		
		/* Read data from RWWEE flash (Solution 1) */
		if (_rww_flash_read(&FLASH_0.dev, NVMCTRL_RWW_EEPROM_ADDR, input, num_bytes) != ERR_NONE) {
			while (1)
			; /* Trap here when flash read error happen */
		}
		
		/* Read data from flash (Solution 2) */
		//flash_read(&FLASH_0, target_addr, output, num_bytes);
		STOP_MEASURE(DGI_GPIO3);
		
	

		// Start decryption
		START_MEASURE(DGI_GPIO2);
		/*decrypt*/
		mbedtls_aes_crypt_cbc( &aes2, MBEDTLS_AES_DECRYPT, num_bytes, iv2, input, input);
		STOP_MEASURE(DGI_GPIO2);
		
		//// Check if memory has correct data
		/*for (size_t byte = 0; byte < num_bytes; byte++) {
		//	if (output[byte] != byte % 0xff) {
		//		gpio_set_pin_level(DGI_GPIO2, GPIO_HIGH);
		//	}
		}*/
		
		
	}
		// Free the memory
		free(input);

		END_MEASUREMENT;
}

int main(void)
{
	atmel_start_init();
	
	// Start measurements
	aes_measurement();

	/*while (true) {
		delay_ms(500);
		gpio_toggle_pin_level(LED0);
	}*/
	
}