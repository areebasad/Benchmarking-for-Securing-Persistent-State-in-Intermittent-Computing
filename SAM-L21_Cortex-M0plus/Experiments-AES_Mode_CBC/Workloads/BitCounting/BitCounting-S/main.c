#include <atmel_start.h>

#define NUM_UINTS 1024
/*
Verify in python using:

bitcount = "".join([bin(n) for n in range(NUM_UINTS)]).count('1')

*/

// Allocate input.
//unsigned int *input = malloc(sizeof(unsigned int) * NUM_UINTS);
unsigned int input[NUM_UINTS];

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();

	// Fill with sequential data.
	for (size_t n = 0; n < NUM_UINTS; n++) {
		input[n] = n;
	}

	static int result = 0;
	
	START_MEASURE(DGI_GPIO2);
	// Add up bit count.
	for (size_t n = 0; n < NUM_UINTS; n++) {
		result += __builtin_popcount(input[n]);
	}
	STOP_MEASURE(DGI_GPIO2);

	END_MEASUREMENT;
	
	return 0;
}
