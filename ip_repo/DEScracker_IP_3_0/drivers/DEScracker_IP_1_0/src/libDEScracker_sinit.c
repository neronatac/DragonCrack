#include "xstatus.h"
#include "xparameters.h"
#include "libDEScracker.h"
#include "libDEScracker_internal.h"

#ifndef XPAR_DESCRACKER_IP_NUM_INSTANCES
#define XPAR_DESCRACKER_IP_NUM_INSTANCES    0 // DEScreacker instances
#endif

/**
* Lookup the device configuration based on the unique device ID.  The table
* ConfigTable contains the configuration info for each device in the system.
*
* @param	DeviceId is the device identifier to lookup.
*
* @return
*		- A pointer of data type DelayGen_Config which points to the
*		  device configuration if DeviceID is found.
*		- NULL if DeviceID is not found.
*
* @note		None.
*
******************************************************************************/
DESCracker_Config *DESCracker_LookupConfig(u16 DeviceId)
{
	DESCracker_Config *CfgPtr = NULL;

	int idx;

	for (idx = 0; idx < XPAR_DESCRACKER_IP_NUM_INSTANCES; idx++) {
		if (DESCracker_ConfigTable[idx].DeviceId == DeviceId) {
			CfgPtr = &DESCracker_ConfigTable[idx];
			break;
		}
	}

	return CfgPtr;
}

/**
* Initialize the DESCracker instance provided by the caller based on the
* given DeviceID.
*
* Nothing is done except to initialize the InstancePtr.
*
* @param	InstancePtr is a pointer to an DESCracker instance. The memory the
*		pointer references must be pre-allocated by the caller. Further
*		calls to manipulate the instance/driver through the DESCracker API
*		must be made with this pointer.
* @param	DeviceId is the unique id of the device controlled by this DESCracker
*		instance. Passing in a device id associates the generic DESCracker
*		instance to a specific device, as chosen by the caller or
*		application developer.
*
* @return
*		- XST_SUCCESS if the initialization was successful.
*		- XST_DEVICE_NOT_FOUND  if the device configuration data was not
*		  found for a device with the supplied device ID.
*
* @note		None.
*
*****************************************************************************/
XStatus DESCracker_Initialize(DESCracker * InstancePtr, u16 DeviceId)
{
	DESCracker_Config *ConfigPtr;

	/*
	 * Assert arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Lookup configuration data in the device configuration table.
	 * Use this configuration info down below when initializing this
	 * driver.
	 */
	ConfigPtr = DESCracker_LookupConfig(DeviceId);
	if (ConfigPtr == (DESCracker_Config *) NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return DESCracker_CfgInitialize(InstancePtr, ConfigPtr,
				   ConfigPtr->BaseAddress);
}
