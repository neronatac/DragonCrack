/*
 * process.c
 *
 *  Created on: 13 mars 2023
 *      Author: s.marcuzzi
 */
#include "process.h"
#include "xil_types.h"
#include "xil_mem.h"
#include "xil_printf.h"

#include "constants.h"
#include "sysmon.h"
#include "utils.h"
#include "des.h"

u8 response_buffer[RESP_LEN_FIELD_LENGTH + RESP_DATA_MAX_SIZE];

/**
 * Executes a command and fills the response buffer.
 * @param command_buffer Buffer containing the command and associated data
 * @param response_buffer Buffer filled with the response
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
void process(u8* command_buffer)
{
    // reset LEN of response buffer
    response_buffer[0] = 0;

	// helpers
	u8 cmd = command_buffer[0];
	u8 len = command_buffer[1];
    u8* data = command_buffer+2;

	// debug
	xil_printf("New command! CMD=0x%x LEN=0x%x\r\n", cmd, len);

    // go to the specific process
    switch (cmd & 0xF0)
    {
        case CMD_GENERAL_BASE:
            process_general(cmd, len, data);
            break;

        default:
            process_des(cmd, len, data);
            break;
    }

    // debug
    xil_printf("Resp len=0x%x\r\n", response_buffer[0]);
}


void process_general(u8 cmd, u8 len, u8* data)
{
    // variables used in switch
    int current_temp, max_temp, min_temp;

	// commands switch
	switch (cmd)
	{
	case CMD_GET_VERSION:
		add_int_to_response(0);  // major
		add_int_to_response(1);  // minor

	case CMD_ECHO:
		add_u8_buf_to_response(data, len);
		break;

	case CMD_GET_TEMPERATURE:
		sysmonGetTempMeasurements(&current_temp, &max_temp, &min_temp);

        add_int_to_response(current_temp);
        add_int_to_response(max_temp);
        add_int_to_response(min_temp);
		break;

	default:
		break;
	}
}
