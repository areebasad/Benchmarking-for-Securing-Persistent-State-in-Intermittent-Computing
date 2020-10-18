#include <atmel_start.h>
#include <stdlib.h>

#include "trustzone_veneer.h"

#define MIN_NUM_BYTES 1
#define MAX_NUM_BYTES 6000 //6496 // Fills memory completely

#define SLEEP

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	
	uint8_t *input = malloc(sizeof(uint8_t) * MAX_NUM_BYTES);
	
	for (size_t num_bytes = MIN_NUM_BYTES; num_bytes <= MAX_NUM_BYTES; num_bytes++) {
		//size_t num_bytes = MAX_NUM_BYTES;
		// Fill with sequential data.
		for (size_t byte = 0; byte < num_bytes; byte++) {
			input[byte] = byte; // Will wrap at 0xff.
		}

		START_MEASURE(DGI_GPIO2);
		// Store data in secure RAM
		nsc_store_data(input, num_bytes);
		STOP_MEASURE(DGI_GPIO2);
		
		SLEEP
		
		// Overwrite the memory
		for (size_t byte = 0; byte < num_bytes; byte++) {
			input[byte] = 0xfe;
		}
		
		
		START_MEASURE(DGI_GPIO3);
		// Read from secure RAM
		nsc_load_data(input, num_bytes);
		STOP_MEASURE(DGI_GPIO3);
		
		//// Check if memory has correct data
		//for (size_t byte = 0; byte < num_bytes; byte++) {
		//	if (input[byte] !=  byte % 0xff) {
		//		gpio_set_pin_level(DGI_GPIO2, GPIO_HIGH);
		//	}
		//}
		
	}
	
	// Free the memory
	free(input);

	END_MEASUREMENT;
}
