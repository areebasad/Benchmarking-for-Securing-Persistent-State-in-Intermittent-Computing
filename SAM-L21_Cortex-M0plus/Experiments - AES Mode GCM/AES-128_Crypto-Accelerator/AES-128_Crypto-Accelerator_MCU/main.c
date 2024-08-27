/**
************************************************************************
 Author		: Hafiz Areeb Asad
 Date		: 24th April, 2020
 Description: Encryption/Decryption using AES module (Hardware). 
	GPIO pins used to measure energy consumption and time taken 
	by each task i.e. encrypt, decrypt, writing on flash and reading from flash.
	Help and code snippets taken from Microchip/Atmel documentation
	and from Erik's Code.                                                                     
************************************************************************

* Support: 
*/

#include "atmel_start.h"
#include <hal_gpio.h>
#include <hal_delay.h>
#include <stdlib.h> //malloc

#define STEP_SIZE 16
#define MIN_NUM_BYTES STEP_SIZE
#define MAX_NUM_BYTES 4144 //6496 //6144

#define SLEEP delay_ms(2);

#ifndef FLASH_PAGE_SIZE
#define FLASH_PAGE_SIZE 64
#endif

#ifndef NVMCTRL_FLASH_SIZE
#define NVMCTRL_FLASH_SIZE 262144	//256 KB
#endif

#ifndef NVMCTRL_ROW_PAGES
#define NVMCTRL_ROW_PAGES 4
#endif

#ifndef NVMCTRL_ROW_SIZE
#define NVMCTRL_ROW_SIZE (NVMCTRL_PAGE_SIZE * NVMCTRL_ROW_PAGES)
#endif


static const uint8_t key_128[16] = {
0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81 };

static const uint8_t key_256[32] = {
	0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
	0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };


/* Initialization Vector */
static uint8_t iv[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
static uint8_t iv2[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f }; /*(This is same but needed for decryption. Note: AES Module was not doing decryption without iv2  )*/

/* The function performs encryption, storing result on flash,
	reading the result and then decrypting it.
	The function contains code to encrypt and decrypt 
	in following modes of AES i.e. ECB, CBC and GCM.
	AES-GCM Mode is used in this file.
*/	
void aes_HW_measurement(void)
{
	
	// Allocate buffer memory (Heap)
	uint8_t *input = malloc( MAX_NUM_BYTES * sizeof(uint8_t));
	//uint8_t input[MAX_NUM_BYTES];// = {0x00};
	delay_ms(10);

	uint8_t additional_data[2] = {0x00, 0x01};
	uint8_t tag_data[2];
	
	uint8_t *output = malloc( MAX_NUM_BYTES * sizeof(uint8_t));
	//uint8_t output[MAX_NUM_BYTES];// = {0x00};

	delay_ms(10);

	aes_sync_enable(&CRYPTOGRAPHY_0);
	
	for (size_t num_bytes = MIN_NUM_BYTES; num_bytes <= MAX_NUM_BYTES; num_bytes += STEP_SIZE) {
		
		// Fill with sequential data.
		for (size_t byte = 0; byte < num_bytes; byte++) {
			input[byte] = byte;  //rand() % 255, 0x00 to write 0's OR 0xFF to write 1's.
			//input[byte] = 0xfa;
		}

		aes_sync_set_encrypt_key(&CRYPTOGRAPHY_0, key_128, AES_KEY_128);
		// A. Encrypt
		START_MEASURE(DGI_GPIO2);
		// io_write(terminal_io, "Encryption", sizeof(uint8_t)*10);		(For debugging OR use breakpoints)
		
		// Note*: To change/disable countermeasure type for preventing side-channel attacks go to hpl_aes.c
		
		/* Note: In file hpl_aes.c 
			Data type block variable changed from uint8 to uint32 to encrypt/decrypt input of more than 4080 bytes.
			(AES_CTRLA_CTYPE(0x0F) added to aes_write_CTRLA_reg.  
		*/
		
		// For ECB Mode - encrypt
		/*for (size_t count = 0;  count < num_bytes/STEP_SIZE; count++) {
			aes_sync_ecb_crypt(&CRYPTOGRAPHY_0, AES_ENCRYPT, input + (count*STEP_SIZE), input + (count*STEP_SIZE));
			//aes_sync_cbc_crypt(&CRYPTOGRAPHY_0, AES_ENCRYPT, input + (count*STEP_SIZE), output+ (count*STEP_SIZE), STEP_SIZE, iv);  (CBC Mode, avoid loops)

		}*/
		// For CBC Mode - encrypt
		//aes_sync_cbc_crypt(&CRYPTOGRAPHY_0, AES_ENCRYPT, input, input, num_bytes, iv);
		// For GCM Mode - encrypt
		aes_sync_gcm_crypt_and_tag(&CRYPTOGRAPHY_0, AES_ENCRYPT, input, input, num_bytes,iv, 16, additional_data, 2, tag_data, 2);
		STOP_MEASURE(DGI_GPIO2);		
	
		// B. Write on flash
		START_MEASURE(DGI_GPIO3);
		/* Solution - 1 @ To write on emulation (RWW) Area */
		if (_rww_flash_write(&FLASH_0.dev, NVMCTRL_RWW_EEPROM_ADDR, input, num_bytes ) != ERR_NONE) {
			while (1)
			; /* Trap here when flash write error happen */
		}
		
		/* Solution - 2 @ To write on Flash Main Application Area */
		
		//--------------------------------------------To write the whole data ----------------------------- 
		//uint32_t target_addr = FLASH_ADDR + FLASH_SIZE - num_bytes;
		//target_addr -= target_addr % NVMCTRL_ROW_SIZE;
	
		//flash_write(&FLASH_0, target_addr, output, num_bytes);
		
		// ------------------------------------------- To write page by page use the code below (Avoid loops)---------
		/*for (uint32_t page_index = 0; page_index * FLASH_PAGE_SIZE < num_bytes; page_index += 1) {
			// Write data to flash 
			flash_write(&FLASH_0, target_addr + page_index * FLASH_PAGE_SIZE, &input[page_index * FLASH_PAGE_SIZE], FLASH_PAGE_SIZE);
		}*/
		STOP_MEASURE(DGI_GPIO3);
		
		SLEEP
		
		// Overwrite the memory
		for (size_t byte = 0; byte < num_bytes; byte++) {
			input[byte] = 0x00;
		}	
		
		// C. Read from flash
		START_MEASURE(DGI_GPIO3);
		/* Read data from RWWEE (Read while write) flash (Solution 1) */
		if (_rww_flash_read(&FLASH_0.dev, NVMCTRL_RWW_EEPROM_ADDR, input, num_bytes) != ERR_NONE) {
			while (1)
			; /* Trap here when flash read error happen */
		}
		
		/* Read data from flash (Solution 2) */
		//flash_read(&FLASH_0, target_addr, output, num_bytes);
		STOP_MEASURE(DGI_GPIO3);
		
		SLEEP

		aes_sync_set_decrypt_key(&CRYPTOGRAPHY_0, key_128, AES_KEY_128);
		
		// D. Decrypt
		START_MEASURE(DGI_GPIO2);
		/* For ECB - decrypt*/
		/*for (size_t count = 0;  count < num_bytes/STEP_SIZE; count++) {
			aes_sync_ecb_crypt(&CRYPTOGRAPHY_0, AES_DECRYPT, output, result);
			//aes_sync_cbc_crypt(&CRYPTOGRAPHY_0, AES_DECRYPT, output + (count*STEP_SIZE), result+ (count*STEP_SIZE), STEP_SIZE, iv2); (Avoid loops)
		}*/
		/* For CBC - decrypt*/
		//aes_sync_cbc_crypt(&CRYPTOGRAPHY_0, AES_DECRYPT, input, output, num_bytes, iv2);
		/* For GCM - decrypt*/
		aes_sync_gcm_auth_decrypt(&CRYPTOGRAPHY_0, input,output,num_bytes,iv,16,additional_data,2,tag_data,2);

		STOP_MEASURE(DGI_GPIO2);
	
	}
		END_MEASUREMENT;
		// Free the memory if malloc(heap) is used
		free(input); free(output); 
}


int main(void)
{
	// Initialize drivers...
	atmel_start_init();

	// Perform measurement for AES-GCM on crypto-accelerator
	aes_HW_measurement();

}
