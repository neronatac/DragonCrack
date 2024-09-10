#include "libDEScracker.h"
#include "xparameters.h"

/**
 * This table contains configuration information for each DEScracker device
 * in the system.
 */
DESCracker_Config DESCracker_ConfigTable[] = {
	{
#ifdef XPAR_DESCRACKER_IP_NUM_INSTANCES
			XPAR_DESCRACKER_IP_0_DEVICE_ID,
			XPAR_DESCRACKER_IP_0_S00_AXI_BASEADDR,
			XPAR_DESCRACKER_IP_0_NBR_WORKERS
#endif
	}
};