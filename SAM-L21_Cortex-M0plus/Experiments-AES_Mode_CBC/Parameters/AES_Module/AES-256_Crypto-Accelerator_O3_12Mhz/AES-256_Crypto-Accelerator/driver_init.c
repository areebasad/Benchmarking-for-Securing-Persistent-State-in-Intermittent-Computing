/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "driver_init.h"
#include <peripheral_clk_config.h>
#include <utils.h>
#include <hal_init.h>

struct aes_sync_descriptor CRYPTOGRAPHY_0;

struct flash_descriptor FLASH_0;

void FLASH_0_CLOCK_init(void)
{

	hri_mclk_set_AHBMASK_NVMCTRL_bit(MCLK);
}

void FLASH_0_init(void)
{
	FLASH_0_CLOCK_init();
	flash_init(&FLASH_0, NVMCTRL);
}

void delay_driver_init(void)
{
	delay_init(SysTick);
}

/**
 * \brief AES initialization function
 *
 * Enables AES peripheral, clocks and initializes AES driver
 */
void CRYPTOGRAPHY_0_init(void)
{
	hri_mclk_set_APBCMASK_AES_bit(MCLK);
	aes_sync_init(&CRYPTOGRAPHY_0, AES);
}

void system_init(void)
{
	init_mcu();

	// GPIO on PB10

	gpio_set_pin_level(LED0,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(LED0, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(LED0, GPIO_PIN_FUNCTION_OFF);

	FLASH_0_init();

	delay_driver_init();
	CRYPTOGRAPHY_0_init();
}
