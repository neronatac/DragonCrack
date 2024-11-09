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


DESCracker DESCrackerInst;


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
		break;

	case CMD_DES_GET_STATUS:
		add_u32_to_response(DESCracker_GetStatus(&DESCrackerInst));
		break;

	case CMD_DES_RESET:
		DESCracker_Reset(&DESCrackerInst);
		break;

	case CMD_DES_ENABLE:
		DESCracker_Enable(&DESCrackerInst);
		break;

	case CMD_DES_DISABLE:
		DESCracker_Disable(&DESCrackerInst);
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

	case CMD_DES_ENDED:
		add_int_to_response(DESCracker_Ended(&DESCrackerInst, u32_from_buffer(data)));
		break;

	case CMD_DES_ENDED_ALL:
		add_u64_to_response(DESCracker_EndedAll(&DESCrackerInst));
		break;

	case CMD_DES_RES_AVAILABLE:
		add_int_to_response(DESCracker_ResultAvailable(&DESCrackerInst, u32_from_buffer(data)));
		break;

	case CMD_DES_RES_AVAILABLE_ALL:
		add_u64_to_response(DESCracker_ResultAvailableAll(&DESCrackerInst));
		break;

	case CMD_DES_RES_FULL:
		add_int_to_response(DESCracker_ResultFull(&DESCrackerInst, u32_from_buffer(data)));
		break;

	case CMD_DES_RES_FULL_ALL:
		add_u64_to_response(DESCracker_ResultFullAll(&DESCrackerInst));
		break;

	case CMD_DES_SET_WORKER:
		DESCracker_SetWorker(&DESCrackerInst, u32_from_buffer(data));
		break;

	case CMD_DES_GET_WORKER:
		add_u32_to_response(DESCracker_GetWorker(&DESCrackerInst));
		break;

	case CMD_DES_SET_START_KEY:
		u64 start_key = u56_from_buffer(data);
		DESCracker_SetStartKey(&DESCrackerInst, start_key);
		break;

	case CMD_DES_GET_START_KEY:
		add_u56_to_response(DESCracker_GetStartKey(&DESCrackerInst));
		break;

	case CMD_DES_SET_END_KEY:
		u64 end_key = u56_from_buffer(data);
		DESCracker_SetEndKey(&DESCrackerInst, end_key);
		break;

	case CMD_DES_GET_END_KEY:
		add_u56_to_response(DESCracker_GetEndKey(&DESCrackerInst));
		break;

	case CMD_DES_GET_RESULT:
		u64 result;
		unsigned int match_nbr;
		DESCracker_GetResult(&DESCrackerInst, &match_nbr, &result);
		add_int_to_response(match_nbr);
		add_u56_to_response(result);
		break;

	default:
		break;
	}
}
