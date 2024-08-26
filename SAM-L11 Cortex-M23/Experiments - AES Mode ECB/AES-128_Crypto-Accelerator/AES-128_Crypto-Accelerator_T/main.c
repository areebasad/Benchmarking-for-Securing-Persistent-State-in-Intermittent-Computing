/**
 * Author: Areeb
 * Created: 2020
 */

#include <atmel_start.h>
//#include "atmel_start_pins.h"
#include "hpl/crya/hpl_crya.h"
//#include <hal_gpio.h>
//#include <hal_delay.h>
#include <stdio.h>
#include <stdlib.h> //malloc
#include <string.h>
#include <stdbool.h>

#define STEP_SIZE 16
#define MIN_NUM_BYTES STEP_SIZE
#define MAX_NUM_BYTES 6000 //6496

#define SLEEP delay_ms(2);

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


/* The function performs encryption, storing result on flash,
	reading the result and then decrypting it.
*/
void aes_HW_experiment(void)
{
	// Example Vectors From FIPS-197
	//                 PLAINTEXT: 00112233445566778899aabbccddeeff
	//                 KEY: 000102030405060708090a0b0c0d0e0f
	//                 CIPHER: 69c4e0d86a7b0430d8cdb78070b4c55a

	const uint8_t key[] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
	};
	

	// Allocate buffer memory
	//uint8_t input[MAX_NUM_BYTES]; 
	uint8_t *input = malloc(sizeof(uint8_t) * MAX_NUM_BYTES);
	delay_ms(10);

	// Loop to iterate checkpoint sizes
	for (size_t num_bytes = MIN_NUM_BYTES; num_bytes <= MAX_NUM_BYTES; num_bytes += STEP_SIZE) {
		
		// Initialization, fill with sequential data.
		for (size_t byte = 0; byte < num_bytes; byte++) {
			input[byte] = byte; // Will wrap at 0xff.
			//input[byte] = 0xfa;
		}
		
		// A. Encrypt
		START_MEASURE(DGI_GPIO2);
		//io_write(terminal_io, "Encryption", sizeof(uint8_t)*10);		
		for (size_t count = 0;  count < num_bytes/STEP_SIZE; count++)
		 {
			crya_aes_encrypt(key, 4, input + (count*STEP_SIZE), input + (count*STEP_SIZE));
		}
		STOP_MEASURE(DGI_GPIO2);
		
		// B. Write on flash	
		START_MEASURE(DGI_GPIO3);
		
		uint32_t target_addr = FLASH_ADDR + FLASH_SIZE - num_bytes ;
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
		
		// Overwrite the RAM variable
		for (size_t byte = 0; byte < num_bytes; byte++) {
			input[byte] = 0xfe;
		}

		// C. Read from flash
		START_MEASURE(DGI_GPIO3);
		FLASH_0_read(target_addr, input, num_bytes);
		STOP_MEASURE(DGI_GPIO3);

		// D. Decrypt
		START_MEASURE(DGI_GPIO2);
		for (size_t count = 0;  count < num_bytes/STEP_SIZE; count++) {	
			
			crya_aes_decrypt(key, 4, input + (count*STEP_SIZE), input + (count*STEP_SIZE));
		}
		STOP_MEASURE(DGI_GPIO2);

		// Test code lines
	    	//io_write(terminal_io, input + (num_bytes -16), sizeof(STEP_SIZE));
		//crya_aes_decrypt(key, 4, input + (num_bytes -16) , input + (num_bytes -16));
		//io_write(terminal_io, input + (num_bytes -16), sizeof(STEP_SIZE));
		
	} // End for loop
	
		// Free the memory
		free(input);

		END_MEASUREMENT;
}

int main(void)
{
	// Initialize drivers...
	atmel_start_init();

	// Perform experiment
	aes_HW_experiment(); 

}
