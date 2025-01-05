/*
 * des.c
 *
 *  Created on: 10 July 2024
 *      Author: s.marcuzzi
 */

#include "xstatus.h"
#include "xparameters.h"
#include "libDEScracker.h"
#include "des.h"
#include "constants.h"
#include "utils.h"


#define RESULTS_BUFFER_SIZE 1024

DESCracker DESCrackerInst;
u32 fixed_part_width;
u32 fixed_key_start, fixed_key_end;
u64 fixed_key_current;  // u64 to be able to stop when current>end and end=0xFF... (no overflow)
int key_range_set = 0, has_finished = 0;
u64 results_buffer[RESULTS_BUFFER_SIZE];
int results_buffer_idx = 0;
int new_results_status = 1;
int first_time[32] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};


XStatus DESCrackerInit()
{
	DESCracker_Config* pDESCrackerConfig;
	XStatus status;

	pDESCrackerConfig = DESCracker_LookupConfig(XPAR_DESCRACKER_IP_0_DEVICE_ID);
	if (pDESCrackerConfig == NULL) return XST_FAILURE;
	xil_printf("DESCracker base address 0x%x\n\r", pDESCrackerConfig->BaseAddress);

	status = DESCracker_CfgInitialize(&DESCrackerInst, pDESCrackerConfig, pDESCrackerConfig->BaseAddress);
	if (status != XST_SUCCESS){
		xil_printf("Bad status returned by DESCracker_CfgInitialize: %d\n\r", status);
		return status;
	}

	// set global useful variables
	fixed_part_width = 56 - DESCrackerInst.VariablePartWidth;

	return XST_SUCCESS;
}


void process_des(u8 cmd, u8 len, u8* data)
{
	// commands switch
	switch (cmd)
	{
	case CMD_DES_GET_VERSION:
		unsigned int major, minor;
		DESCracker_GetVersion(&DESCrackerInst, &major, &minor);
		add_int_to_response(major);
		add_int_to_response(minor);
		break;

	case CMD_DES_GET_PARAMS:
		add_int_to_response(DESCrackerInst.WorkersNumber);
		add_int_to_response(DESCrackerInst.VariablePartWidth);
		break;

	case CMD_DES_SET_PLAINTEXT:
		u64 plaintext = u64_from_buffer(data);
		DESCracker_SetPlaintext(&DESCrackerInst, plaintext);
		break;

	case CMD_DES_GET_PLAINTEXT:
		add_u64_to_response(DESCracker_GetPlaintext(&DESCrackerInst));
		break;

	case CMD_DES_SET_MASK:
		u64 mask = u64_from_buffer(data+1);
		DESCracker_SetMask(&DESCrackerInst, (int)data[0], mask);
		break;

	case CMD_DES_GET_MASK:
		add_u64_to_response(DESCracker_GetMask(&DESCrackerInst, (int)data[0]));
		break;

	case CMD_DES_SET_REF:
		u64 ref = u64_from_buffer(data+1);
		DESCracker_SetRef(&DESCrackerInst, (int)data[0], ref);
		break;

	case CMD_DES_GET_REF:
		add_u64_to_response(DESCracker_GetRef(&DESCrackerInst, (int)data[0]));
		break;

	case CMD_DES_SET_KEY_RANGE:
		fixed_key_start = (u32)(u56_from_buffer(data) >> DESCrackerInst.VariablePartWidth);
		fixed_key_end = (u32)(u56_from_buffer(data+7) >> DESCrackerInst.VariablePartWidth);
		fixed_key_current = fixed_key_start;

		key_range_set = 1;
		has_finished = 0;
		for (int i=0; i<32; i++) first_time[i] = 1;
		results_buffer_idx = 0;
		break;

	case CMD_DES_GET_KEY_RANGE:
		u64 key_start = (u64)fixed_key_start << DESCrackerInst.VariablePartWidth;
		u64 key_end = ((u64)(fixed_key_end + 1) << DESCrackerInst.VariablePartWidth) - 1;
		add_u56_to_response(key_start);
		add_u56_to_response(key_end);
		break;

	case CMD_DES_GET_CURRENT_CHUNK:
		add_u32_to_response(fixed_key_current);
		break;

	case CMD_DES_GET_STATUS:
		if (key_range_set == 0 && has_finished == 0) add_u32_to_response(DES_STATUS_WAITING);
		else if (key_range_set == 1 && has_finished == 0) add_u32_to_response(DES_STATUS_RUNNING);
		else if (key_range_set == 0 && has_finished == 1) add_u32_to_response(DES_STATUS_FINISHED);
		else add_u32_to_response(DES_STATUS_ERROR);  // should not happen
		break;

	case CMD_DES_GET_RESULTS:
		// just 3 results at most because packet size is limited to 255
		for(int i=0; i<3; i++) {
			if (results_buffer_idx > 0) {
				add_u64_to_response(results_buffer[--results_buffer_idx]);
			}
		}
		break;

	case CMD_DES_SET_NEW_RESULTS_STATUS: // in case of Timeout on Python side, this can be usefull to stop getting new results
		// 0 to stop getting new results
		new_results_status = u32_from_buffer(data);
		break;

	case CMD_DES_GET_NEW_RESULTS_STATUS:
		add_int_to_response(new_results_status);
		break;

	default:
		break;
	}
}


void handle_exhaust()
{
	if (key_range_set == 0) return;

	u32 result_available_all = DESCracker_ResultAvailableAll(&DESCrackerInst);
	u32 ended_all = DESCracker_EndedAll(&DESCrackerInst);

	if (__builtin_popcount(ended_all) == DESCrackerInst.WorkersNumber && fixed_key_current > fixed_key_end){
		has_finished = 1;
		key_range_set = 0;
		return;
	}


	for(int worker=0; worker < DESCrackerInst.WorkersNumber; worker++) {
		if (result_available_all & 1 && results_buffer_idx < RESULTS_BUFFER_SIZE && new_results_status != 0) {
			DESCracker_SetWorker(&DESCrackerInst, worker);
			u64 result = DESCracker_GetResultRaw(&DESCrackerInst);
			results_buffer[results_buffer_idx++] = result;
		} else if ((ended_all & 1) || first_time[worker]) {
			if (fixed_key_current > fixed_key_end) continue;

			first_time[worker] = 0;

			DESCracker_SetWorker(&DESCrackerInst, worker);
			DESCracker_SetFixedKey(&DESCrackerInst, fixed_key_current);
			DESCracker_Reset(&DESCrackerInst, 1 << worker);
			fixed_key_current += 1;
		}
		result_available_all >>= 1;
		ended_all >>= 1;
	}
}
