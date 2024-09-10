/*
 * main.c
 *
 *  Created on: 9 mars 2023
 *      Author: s.marcuzzi
 */

#include "xstatus.h"
#include "platform.h"
#include "ethernet.h"
#include "xil_printf.h"
#include "des.h"
#include "sysmon.h"

int main()
{
	XStatus status;

	// Init phase
	init_platform();

	status = sysmonInit();
	if(status != XST_SUCCESS) {
		xil_printf("Sysmon init failed\r\n");
		return status;
	}

	status = DESCrackerInit();
	if(status != XST_SUCCESS) {
		xil_printf("DEScrackerIP init failed\r\n");
		return status;
	}

	status = ethernetInit();
	if(status != XST_SUCCESS) {
		xil_printf("Ethernet init failed\r\n");
		return status;
	}

	// process phase
	ethernet_mainloop();

	// --- never reached ---

	// exit phase
	cleanup_platform();

	return 0;
}
