/*
 * process.h
 *
 *  Created on: 13 mars 2023
 *      Author: s.marcuzzi
 */

#ifndef SRC_PROCESS_H_
#define SRC_PROCESS_H_

#include "xil_types.h"

/**
 * Executes a command and fills the response buffer.
 * @param command_buffer Buffer containing the command and associated data
 *
 * The command buffer is as follow:
 * |--------|--------|---------------|
 * | 1 byte | 1 byte | [0-255] bytes |
 * |--------|--------|---------------|
 * |  CMD   |  LEN   |     DATA      |
 * |--------|--------|---------------|
 *
 * The response buffer is as follow:
 * |--------|---------------|
 * | 1 byte | [0-255] bytes |
 * |--------|---------------|
 * |  LEN   |     DATA      |
 * |--------|---------------|
 *
 * If no response is given, the first byte of response buffer contains 0x00 and no response is sent.
 */
void process(u8* command_buffer);

/**
 * Executes commands not associated with a particular peripheral.
 * See `process` for more details.
 */
void process_general(u8 cmd, u8 len, u8* data);

#endif /* SRC_PROCESS_H_ */
