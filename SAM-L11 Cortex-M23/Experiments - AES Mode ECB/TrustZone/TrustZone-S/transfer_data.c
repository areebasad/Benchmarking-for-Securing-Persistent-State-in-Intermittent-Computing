/*
 * transfer_data.c
 *
 * Created: 2019-02-28 14:11:36
 *  Author: erikw_000
 */
#include "transfer_data.h"

#include <atmel_start.h>
#include <stdlib.h>
#include <string.h>

// static pointer for data array
static uint8_t *secure_data;

/**
 * \brief Get data from non secure world and store it in *secure_data
 */
void secure_store_data(uint8_t *data, size_t num_bytes) {
	// Check that the data is actually coming from the non-secure world (prevent from leaking secure data)
	data = cmse_check_address_range(data, num_bytes, CMSE_NONSECURE);
	// Allocate memory in *secure_data
	secure_data = malloc(sizeof(uint8_t) * num_bytes);
	// Store in *secure_data
	memcpy(secure_data, data, sizeof(uint8_t) * num_bytes);
}

/**
 * \brief Return secure_data to non secure world
 */
void secure_load_data(uint8_t *data, size_t num_bytes) {
	// Check that the data is actually in the non-secure world (prevent from overwriting secure data)
	data = cmse_check_address_range(data, num_bytes, CMSE_NONSECURE);
	// Allocate memory in *data
	//data = malloc(sizeof(uint8_t) * num_bytes);
	// Store in *data
	memcpy(data, secure_data, sizeof(uint8_t) * num_bytes);
	// Free the memory
	free(secure_data);
}