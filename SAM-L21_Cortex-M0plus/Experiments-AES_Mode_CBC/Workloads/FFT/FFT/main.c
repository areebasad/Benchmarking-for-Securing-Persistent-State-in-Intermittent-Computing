#include <atmel_start.h>
#include "kissfft/kiss_fft.h"
// Add NDEBUG to compile at Toolchain -> ARM/GNU Preprocessing Assembler -> Symbols
// Also in GNU C Compiler -> Symbols
#define NFFT 128

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	
	START_MEASURE(DGI_GPIO2);
	size_t buflen = sizeof(kiss_fft_cpx)*NFFT;

	kiss_fft_cpx  * in = (kiss_fft_cpx*)malloc(buflen);
	kiss_fft_cpx  * out= (kiss_fft_cpx*)malloc(buflen);
	kiss_fft_cfg  cfg = kiss_fft_alloc(NFFT,0,NULL,NULL);
	STOP_MEASURE(DGI_GPIO2);
	
	for (int k=0;k<NFFT;++k) {
		in[k].r = (k % 65536) - 32768;
		in[k].i = (k % 65536) - 32768;
	}

	START_MEASURE(DGI_GPIO3);
	kiss_fft(cfg,in,out);
	STOP_MEASURE(DGI_GPIO3);

	START_MEASURE(DGI_GPIO2);
	kiss_fft_free(in);
	kiss_fft_free(out);
	kiss_fft_free(cfg);
	STOP_MEASURE(DGI_GPIO2);

	END_MEASUREMENT;
	
	return 0;
}
