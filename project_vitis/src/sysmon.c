/*
 * sysmon.c
 *
 *  Created on: 9 mars 2023
 *      Author: s.marcuzzi
 */

#include "sysmon.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xsysmon.h"
#include "xstatus.h"


static XSysMon SysMonInst;


/**
 * Initializes the Sysmon peripheral
 * @return
 * 		- XST_SUCCESS if successful
 * 		- another Xstatus if not
 */
XStatus sysmonInit(){
	XSysMon_Config* pSysmonConfig;
	XStatus status;

	pSysmonConfig = XSysMon_LookupConfig(XPAR_SYSMON_0_DEVICE_ID);
	if (pSysmonConfig == NULL) return XST_FAILURE;
	xil_printf("Sysmon base address 0x%x\n\r", pSysmonConfig->BaseAddress);

	status = XSysMon_CfgInitialize(&SysMonInst, pSysmonConfig, pSysmonConfig->BaseAddress);
	if (status != XST_SUCCESS){
		xil_printf("Bad status returned by XSysMon_CfgInitialize: %d\n\r", status);
		return status;
	}

	status = XSysMon_SelfTest(&SysMonInst);
	if (status != XST_SUCCESS){
		xil_printf("Bad status returned by XSysMon_SelfTest: %d\n\r", status);
		return status;
	}

	return XST_SUCCESS;
}


/**
 * Gets temperature measurements.
 * @param current	variable to store current temperature in
 * @param max		variable to store max temperature in
 * @param min		variable to store min temperature in
 */
void sysmonGetTempMeasurements(int* current, int* max, int* min){
	u16 tempRawData;
	float tempData;

	XSysMon_StartAdcConversion(&SysMonInst);

	tempRawData = XSysMon_GetAdcData(&SysMonInst, XSM_CH_TEMP);
	tempData = XSysMon_RawToTemperature(tempRawData);
	*current = (int)(tempData);

	tempRawData = XSysMon_GetMinMaxMeasurement(&SysMonInst, XSM_MAX_TEMP);
	tempData = XSysMon_RawToTemperature(tempRawData);
	*max = (int)(tempData);

	tempRawData = XSysMon_GetMinMaxMeasurement(&SysMonInst, XSM_MIN_TEMP);
	tempData = XSysMon_RawToTemperature(tempRawData);
	*min = (int)(tempData);
}

