/**
************************************************************************
 Author		: Hafiz Areeb Asad
 Date		: 29th April, 2020
 Description: Encryption/Decryption using wolfSSL library (Software). 
	GPIO pins used to measure energy consumption and time taken, 
	by each task i.e. encrypt, decrypt, writing on flash and reading from flash.
	Help and code snippets taken from wolfSSL/Atmel documentation
	and from Erik's Code.                                                                     
************************************************************************
*/

#include <atmel_start.h>
#include "atmel_start_pins.h"
#include "hpl/crya/hpl_crya.h"


#include <hal_gpio.h>
#include <hal_delay.h>
#include <stdio.h>
#include <stdlib.h> //malloc
#include <string.h>
#include <stdbool.h>
#include "wolfssl/wolfcrypt/settings.h"
#include "wolfssl/wolfcrypt/aes.h"

#define STEP_SIZE 16
#define MIN_NUM_BYTES STEP_SIZE
#define MAX_NUM_BYTES 6000 //6496


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

static uint8_t hello_str[] = "\r\nCrypto Accelerator \r\n";

static struct io_descriptor *terminal_io;

void UART_EDBG_init()
{
	usart_sync_get_io_descriptor(&USART_0, &terminal_io);
	usart_sync_enable(&USART_0);
}

/* The function performs encryption, storing result on flash,
	reading the result and then decrypting it.
	AES ECB mode with key length of 256 is used in this file.
*/	
void aes_SW_measurement(void)
{
	// Example Vectors From FIPS-197:-
	//                 PLAINTEXT: 00112233445566778899aabbccddeeff
	//                 KEY: 000102030405060708090a0b0c0d0e0f
	//                 CIPHER: 69c4e0d86a7b0430d8cdb78070b4c55a
	
	/*
	const uint8_t plaintext[] = {
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
	};*/

	Aes enc;
	Aes dec;
	
	const uint8_t key[32] = {
		0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
		0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4
	};

	
	static uint8_t iv[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
	
	// Allocate buffer memory
	uint8_t *input = malloc(sizeof(uint8_t) * MAX_NUM_BYTES);

    delay_ms(10);

	for (size_t num_bytes = MIN_NUM_BYTES; num_bytes <= MAX_NUM_BYTES; num_bytes += STEP_SIZE) {
		
		// Fill with sequential data.
		for (size_t byte = 0; byte < num_bytes; byte++) {
			input[byte] = byte; // Will wrap at 0xff.
			//input[byte] = 0xfa;
		}
			
		wc_AesSetKey(&enc, key, sizeof(key), iv, AES_ENCRYPTION);

		// A. Encrypt
		START_MEASURE(DGI_GPIO2);
		//io_write(terminal_io, "Encryption", sizeof(uint8_t)*10);		(For debugging OR use breakpoints)
		/* ECB - encrypt*/				
		wc_AesEncryptDirect(&enc, input, input);
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

		// C. Read from flash
		START_MEASURE(DGI_GPIO3);
		// Read from flash
		FLASH_0_read(target_addr, input, num_bytes);
		STOP_MEASURE(DGI_GPIO3);
	
		wc_AesSetKey(&dec, key, sizeof(key), iv, AES_DECRYPTION);

		// D. Decrypt
		START_MEASURE(DGI_GPIO2);
		/* ECB - decrypt*/
		wc_AesDecryptDirect(&dec, input, input);
		STOP_MEASURE(DGI_GPIO2);
		
	}
		// Free the memory
		free(input);

		END_MEASUREMENT;
		
	return true;
	
	
}

int main(void)
{
	// Initialize drivers...
	atmel_start_init();
	UART_EDBG_init();
	
	// Start measurements
	aes_SW_measurement();
    
}
