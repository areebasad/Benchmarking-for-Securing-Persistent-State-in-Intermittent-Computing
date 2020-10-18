
/*
 * transfer_data.h
 *
 * Created: 2019-03-05 13:39:26
 *  Author: erikw_000
 */ 

#ifndef TRANSFER_DATA_H_
#define TRANSFER_DATA_H_

#include "atmel_start.h"

/**
 * \brief Get data from non secure world and store it in *secure_data
 */
void secure_store_data(uint8_t *data, size_t num_bytes);

/**
 * \brief Return secure_data to non secure world
 */
void secure_load_data(uint8_t *data, size_t num_bytes);

#endif /* TRANSFER_DATA_H_ */