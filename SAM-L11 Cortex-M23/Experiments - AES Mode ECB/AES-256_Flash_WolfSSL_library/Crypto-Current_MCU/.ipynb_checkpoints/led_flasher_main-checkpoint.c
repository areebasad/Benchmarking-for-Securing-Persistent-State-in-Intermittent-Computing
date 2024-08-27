/**
 * \file
 *
 * \brief Application implement
 *
 * Copyright (c) 2015-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
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

bool test_aes(void)
{
	// Example Vectors From FIPS-197
	//                 PLAINTEXT: 00112233445566778899aabbccddeeff
	//                 KEY: 000102030405060708090a0b0c0d0e0f
	//                 CIPHER: 69c4e0d86a7b0430d8cdb78070b4c55a

	const uint8_t plaintext[] = {
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
	};
	const uint8_t key[] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
	};
	const uint8_t cipher[] = {
		0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30,
		0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a
	};
	uint8_t result[16];
	uint32_t i;
	
	// Allocate buffer memory
	uint8_t *input = malloc(sizeof(uint8_t) * MAX_NUM_BYTES);

    delay_ms(10);
    
	for (size_t num_bytes = MIN_NUM_BYTES; num_bytes <= MAX_NUM_BYTES; num_bytes += STEP_SIZE) {
		
		// Fill with sequential data.
		for (size_t byte = 0; byte < num_bytes; byte++) {
			input[byte] = byte; // Will wrap at 0xff.
			//input[byte] = 0xfa;
		}
		
		


		//Start---------------------------------------------------------------------------------------------------------------------//
		START_MEASURE(DGI_GPIO2);
		//io_write(terminal_io, "Encryption", sizeof(uint8_t)*10);		
		for (size_t count = 0;  count < num_bytes/STEP_SIZE; count++) {
			crya_aes_encrypt(key, 4, input + (count*STEP_SIZE), input + (count*STEP_SIZE));
		}
		STOP_MEASURE(DGI_GPIO2);
		//End------------------------------------------------------------------------------------------------------------------------//
		
		/* Save to flash
		   Put data at end of flash.
	    **/		
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
		// Read from flash
		FLASH_0_read(target_addr, input, num_bytes);
		STOP_MEASURE(DGI_GPIO3);

		
		
		
		START_MEASURE(DGI_GPIO2);
		for (size_t count = 0;  count < num_bytes/STEP_SIZE; count++) {	
			crya_aes_decrypt(key, 4, input + (count*STEP_SIZE), input + (count*STEP_SIZE));
		}
		STOP_MEASURE(DGI_GPIO2);
		
		

		
		// Free the memory
		free(input);

		END_MEASUREMENT;
	    //io_write(terminal_io, input + (num_bytes -16), sizeof(STEP_SIZE));
		
		//crya_aes_decrypt(key, 4, input + (num_bytes -16) , input + (num_bytes -16));
		//io_write(terminal_io, input + (num_bytes -16), sizeof(STEP_SIZE));
		
	}
	return true;
	
	/*
	
	for (i = 0; i < 16; i++) {
		if (result[i] != cipher[i]) {
			break;
		}
	}
	if (i < 16) {
		return false;
	}
	

	crya_aes_decrypt(key, 4, result, result);
	io_write(terminal_io, result, sizeof(result));

	for (i = 0; i < 16; i++) {
		if (result[i] != plaintext[i]) {
			break;
		}
	}
	

	if (i < 16) {
		return false;
		} else {
		return true;
	}
	*/
}



int main(void)
{
	atmel_start_init();
	
	UART_EDBG_init();
	
	//io_write(terminal_io, hello_str, sizeof(hello_str) - 1);

	bool f = test_aes();
    /*
	while (true) {
		delay_ms(500);
		gpio_toggle_pin_level(LED0);
	}
    */
}
