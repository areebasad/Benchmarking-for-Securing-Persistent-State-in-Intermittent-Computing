#include <atmel_start.h>

/* typedef for non-secure callback functions */
typedef void (*ns_funcptr_void) (void) __attribute__((cmse_nonsecure_call));

/* TZ_START_NS: Start address of non-secure application */
#define TZ_START_NS 0x00008000

void load_ns_application(void)
{
	ns_funcptr_void NonSecure_ResetHandler;
	/* - Set non-secure main stack (MSP_NS) */
	__TZ_set_MSP_NS(*((uint32_t *)(TZ_START_NS)));
	/* - Get non-secure reset handler */
	NonSecure_ResetHandler = (ns_funcptr_void)(*((uint32_t *)((TZ_START_NS) + 4U)));
	/* Start non-secure state software application */
	NonSecure_ResetHandler();
}

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	
	// Load non-secure application
	load_ns_application();
}
