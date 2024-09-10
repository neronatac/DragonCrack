/*
 * utils.h
 *
 *  Created on: 15 mars 2023
 *      Author: s.marcuzzi
 */

#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#include "xil_types.h"
#include <stdint.h>


/**
 * Adds a buffer to the response buffer and updates response length.
 * @param buffer buffer to copy into response
 * @param len number of bytes to copy
 */
void add_u8_buf_to_response(u8* buffer, u32 len);

/**
 * Adds an U32 to the response buffer and updates response length.
 * @param val number to add to response
 */
void add_u32_to_response(u32 val);

/**
 * Adds an U64 to the response buffer and updates response length.
 * @param val number to add to response
 */
void add_u64_to_response(u64 val);

/**
 * Adds an 56-bit integer to the response buffer and updates response length.
 * @param val number to add to response. MSB is ignored.
 */
void add_u56_to_response(u64 val);

/**
 * Adds an int to the response buffer and updates response length.
 * @param val number to add to response
 */
void add_int_to_response(int val);

/**
 * Gets a u64 from an u8 buffer.
 * @param buf buffer to work on
 */
u64 u64_from_buffer(u8 *buf);

/**
 * Gets a 56-bit int from an u8 buffer.
 * MSB of returned u64 is 0x00.
 * @param buf buffer to work on
 */
u64 u56_from_buffer(u8 *buf);

/**
 * Gets a u32 from an u8 buffer.
 * @param buf buffer to work on
 */
u32 u32_from_buffer(u8 *buf);


#endif /* SRC_UTILS_H_ */
