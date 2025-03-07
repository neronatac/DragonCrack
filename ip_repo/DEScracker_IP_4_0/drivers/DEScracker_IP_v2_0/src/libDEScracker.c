

/***************************** Include Files *******************************/
#include "libDEScracker.h"

/************************** Function Definitions ***************************/

/**
* Initialize the DESCracker instance provided by the caller based on the
* given configuration data.
*
* Nothing is done except to initialize the InstancePtr.
*
* @param	InstancePtr is a pointer to a DESCracker instance. The memory the
*		pointer references must be pre-allocated by the caller. Further
*		calls to manipulate the driver through the DESCracker API must be
*		made with this pointer.
* @param	Config is a reference to a structure containing information
*		about a specific DESCracker device. This function initializes an
*		InstancePtr object for a specific device specified by the
*		contents of Config. This function can initialize multiple
*		instance objects with the use of multiple calls giving different
*		Config information on each call.
* @param 	EffectiveAddr is the device base address in the virtual memory
*		address space. The caller is responsible for keeping the address
*		mapping from EffectiveAddr to the device physical base address
*		unchanged once this function is invoked. Unexpected errors may
*		occur if the address mapping changes after this function is
*		called. If address translation is not used, use
*		Config->BaseAddress for this parameters, passing the physical
*		address instead.
*
* @return
* 		- XST_SUCCESS if the initialization is successful.
*
* @note		None.
*
*****************************************************************************/
XStatus DESCracker_CfgInitialize(DESCracker * InstancePtr, DESCracker_Config * Config,
			UINTPTR EffectiveAddr)
{
	/* Assert arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Set some default values. */
	InstancePtr->BaseAddress = EffectiveAddr;

	InstancePtr->WorkersNumber = Config->WorkersNumber;

	/*
	 * Indicate the instance is now ready to use, initialized without error
	 */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	return XST_SUCCESS;
}

/**
 * Gets the version number of the IP.
 * @param InstancePtr: DESCracker instance to work on
 * @param major: major version number
 * @param minor: minor version number
 */
void DESCracker_GetVersion(DESCracker *InstancePtr, unsigned int* major, unsigned int* minor)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int version = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_VERSION_OFFSET);
	*major = (version >> 24) & 0xFF;
	*minor = (version >> 16) & 0xFF;
}

/**
 * Gets the status of the IP (reset and enable bit fields).
 * @param InstancePtr: DESCracker instance to work on
 * @return: one of DESCRAKER_IP_STATUS_xxx status
 */
u32 DESCracker_GetStatus(DESCracker *InstancePtr)
{
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_CONTROL_OFFSET) & 0b11;
}

/**
 * Resets the workers by setting then unsetting the reset bit in control register
 * @param InstancePtr: DESCracker instance to work on
 */
void DESCracker_Reset(DESCracker *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // set reset bit
	int res = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_CONTROL_OFFSET);
    res |= 1 << DESCRACKER_IP_RESET_SHIFT;
    DESCRACKER_IP_mWriteReg(InstancePtr->BaseAddress, DESCRACKER_IP_CONTROL_OFFSET, res);

    // unset reset bit
    res &= ~(1 << DESCRACKER_IP_RESET_SHIFT);
    DESCRACKER_IP_mWriteReg(InstancePtr->BaseAddress, DESCRACKER_IP_CONTROL_OFFSET, res);
}

/**
 * Sets the enable bit in control register
 * @param InstancePtr: DESCracker instance to work on
 */
void DESCracker_Enable(DESCracker *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // set enable bit
	int res = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_CONTROL_OFFSET);
    res |= 1 << DESCRACKER_IP_ENABLE_SHIFT;
    DESCRACKER_IP_mWriteReg(InstancePtr->BaseAddress, DESCRACKER_IP_CONTROL_OFFSET, res);
}

/**
 * Resets the enable bit in control register
 * @param InstancePtr: DESCracker instance to work on
 */
void DESCracker_Disable(DESCracker *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // reset enable bit
	int res = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_CONTROL_OFFSET);
    res &= ~(1 << DESCRACKER_IP_ENABLE_SHIFT);
    DESCRACKER_IP_mWriteReg(InstancePtr->BaseAddress, DESCRACKER_IP_CONTROL_OFFSET, res);
}

/**
 * Sets the plaintext
 * @param InstancePtr: DESCracker instance to work on
 * @param plaintext: plaintext to set
 */
void DESCracker_SetPlaintext(DESCracker *InstancePtr, u64 plaintext)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    u32 high = (plaintext >> 32) & 0xFFFFFFFF;
    u32 low = plaintext & 0xFFFFFFFF;

    DESCRACKER_IP_mWriteReg(InstancePtr->BaseAddress, DESCRACKER_IP_PLAINTEXT_HIGH_OFFSET, high);
    DESCRACKER_IP_mWriteReg(InstancePtr->BaseAddress, DESCRACKER_IP_PLAINTEXT_LOW_OFFSET, low);
}

/**
 * Gets the plaintext
 * @param InstancePtr: DESCracker instance to work on
 */
u64 DESCracker_GetPlaintext(DESCracker *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    u32 high = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_PLAINTEXT_HIGH_OFFSET);
    u32 low = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_PLAINTEXT_LOW_OFFSET);

    return ((u64)high << 32) | low;
}

/**
 * Sets a mask
 * @param InstancePtr: DESCracker instance to work on
 * @param number: mask number
 * @param mask: mask to set
 */
void DESCracker_SetMask(DESCracker *InstancePtr, unsigned int number, u64 mask)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
    Xil_AssertVoid(number == 1 || number == 2);

    u32 high = (mask >> 32) & 0xFFFFFFFF;
    u32 low = mask & 0xFFFFFFFF;

    if (number == 1) {
        DESCRACKER_IP_mWriteReg(InstancePtr->BaseAddress, DESCRACKER_IP_MASK1_HIGH_OFFSET, high);
        DESCRACKER_IP_mWriteReg(InstancePtr->BaseAddress, DESCRACKER_IP_MASK1_LOW_OFFSET, low);
    } else {
        DESCRACKER_IP_mWriteReg(InstancePtr->BaseAddress, DESCRACKER_IP_MASK2_HIGH_OFFSET, high);
        DESCRACKER_IP_mWriteReg(InstancePtr->BaseAddress, DESCRACKER_IP_MASK2_LOW_OFFSET, low);
    }
}

/**
 * Gets a mask
 * @param InstancePtr: DESCracker instance to work on
 * @param number: mask number
 */
u64 DESCracker_GetMask(DESCracker *InstancePtr, unsigned int number)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
    Xil_AssertNonvoid(number == 1 || number == 2);

    u32 high = 0;
    u32 low = 0;

    if (number == 1) {
        high = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_MASK1_HIGH_OFFSET);
        low = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_MASK1_LOW_OFFSET);
    } else {
        high = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_MASK2_HIGH_OFFSET);
        low = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_MASK2_LOW_OFFSET);
    }

    return ((u64)high << 32) | low;
}

/**
 * Sets a reference ciphertext
 * @param InstancePtr: DESCracker instance to work on
 * @param number: reference number
 * @param ref: reference ciphertext to set
 */
void DESCracker_SetRef(DESCracker *InstancePtr, unsigned int number, u64 ref)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
    Xil_AssertVoid(number == 1 || number == 2);

    u32 high = (ref >> 32) & 0xFFFFFFFF;
    u32 low = ref & 0xFFFFFFFF;

    if (number == 1) {
        DESCRACKER_IP_mWriteReg(InstancePtr->BaseAddress, DESCRACKER_IP_REF1_HIGH_OFFSET, high);
        DESCRACKER_IP_mWriteReg(InstancePtr->BaseAddress, DESCRACKER_IP_REF1_LOW_OFFSET, low);
    } else {
        DESCRACKER_IP_mWriteReg(InstancePtr->BaseAddress, DESCRACKER_IP_REF2_HIGH_OFFSET, high);
        DESCRACKER_IP_mWriteReg(InstancePtr->BaseAddress, DESCRACKER_IP_REF2_LOW_OFFSET, low);
    }
}

/**
 * Gets a reference
 * @param InstancePtr: DESCracker instance to work on
 * @param number: reference number
 */
u64 DESCracker_GetRef(DESCracker *InstancePtr, unsigned int number)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
    Xil_AssertNonvoid(number == 1 || number == 2);

    u32 high = 0;
    u32 low = 0;

    if (number == 1) {
        high = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_REF1_HIGH_OFFSET);
        low = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_REF1_LOW_OFFSET);
    } else {
        high = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_REF2_HIGH_OFFSET);
        low = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_REF2_LOW_OFFSET);
    }

    return ((u64)high << 32) | low;
}

/**
 * Checks if a worker ended
 * @param InstancePtr: DESCracker instance to work on
 * @param number: worker number
 */
unsigned int DESCracker_Ended(DESCracker *InstancePtr, unsigned int number)
{
    Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
    Xil_AssertNonvoid(number < InstancePtr->WorkersNumber);

    u32 res = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_ENDEDOUTS_OFFSET);

    return (res >> number) & 1;
}

/**
 * Gets all ended bits in a single u32
 * @param InstancePtr: DESCracker instance to work on
 */
u32 DESCracker_EndedAll(DESCracker *InstancePtr)
{
    Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_ENDEDOUTS_OFFSET);
}

/**
 * Checks if a result is available for a specific worker
 * @param InstancePtr: DESCracker instance to work on
 * @param number: worker number
 */
unsigned int DESCracker_ResultAvailable(DESCracker *InstancePtr, unsigned int number)
{
    Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
    Xil_AssertNonvoid(number < InstancePtr->WorkersNumber);

    u32 res = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_RESULTSAVAILABLE_OFFSET);

    return (res >> number) & 1;
}

/**
 * Gets all result_available bits in a single u32
 * @param InstancePtr: DESCracker instance to work on
 */
u32 DESCracker_ResultAvailableAll(DESCracker *InstancePtr)
{
    Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_RESULTSAVAILABLE_OFFSET);
}

/**
 * Checks if a result is full for a specific worker
 * @param InstancePtr: DESCracker instance to work on
 * @param number: worker number
 */
unsigned int DESCracker_ResultFull(DESCracker *InstancePtr, unsigned int number)
{
    Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
    Xil_AssertNonvoid(number < InstancePtr->WorkersNumber);

    u32 res = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_RESULTSFULL_OFFSET);

    return (res >> number) & 1;
}

/**
 * Gets all result_full bits in a signle u64
 * @param InstancePtr: DESCracker instance to work on
 */
u32 DESCracker_ResultFullAll(DESCracker *InstancePtr)
{
    Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_RESULTSFULL_OFFSET);
}

/**
 * Sets the current worker used by START_KEY, END_KEY and MATCH_RESULT registers
 * @param InstancePtr: DESCracker instance to work on
 * @param number: worker number
 */
void DESCracker_SetWorker(DESCracker *InstancePtr, unsigned int number)
{
    Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
    Xil_AssertVoid(number < InstancePtr->WorkersNumber);

    DESCRACKER_IP_mWriteReg(InstancePtr->BaseAddress, DESCRACKER_IP_WORKER_OFFSET, number);
}

/**
 * Gets the current worker
 * @param InstancePtr: DESCracker instance to work on
 */
unsigned int DESCracker_GetWorker(DESCracker *InstancePtr)
{
    Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_WORKER_OFFSET);
}

/**
 * Sets the start key for the worker specified by the WORKER register
 * @param InstancePtr: DESCracker instance to work on
 * @param key: start key to set
 */
void DESCracker_SetStartKey(DESCracker *InstancePtr, u64 key)
{
    Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    u32 high = (key >> 32) & 0xFFFFFFFF;
    u32 low = key & 0xFFFFFFFF;

    DESCRACKER_IP_mWriteReg(InstancePtr->BaseAddress, DESCRACKER_IP_STARTKEY_HIGH_OFFSET, high);
    DESCRACKER_IP_mWriteReg(InstancePtr->BaseAddress, DESCRACKER_IP_STARTKEY_LOW_OFFSET, low);
}

/**
 * Gets the start key for the worker specified by the WORKER register
 * @param InstancePtr: DESCracker instance to work on
 */
u64 DESCracker_GetStartKey(DESCracker *InstancePtr)
{
    Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    u32 high = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_STARTKEY_HIGH_OFFSET);
    u32 low = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_STARTKEY_LOW_OFFSET);

    return ((u64)high << 32) | low;
}

/**
 * Sets the end key for the worker specified by the WORKER register
 * @param InstancePtr: DESCracker instance to work on
 * @param key: end key to set
 */
void DESCracker_SetEndKey(DESCracker *InstancePtr, u64 key)
{
    Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    u32 high = (key >> 32) & 0xFFFFFFFF;
    u32 low = key & 0xFFFFFFFF;

    DESCRACKER_IP_mWriteReg(InstancePtr->BaseAddress, DESCRACKER_IP_ENDKEY_HIGH_OFFSET, high);
    DESCRACKER_IP_mWriteReg(InstancePtr->BaseAddress, DESCRACKER_IP_ENDKEY_LOW_OFFSET, low);
}

/**
 * Gets the end key for the worker specified by the WORKER register
 * @param InstancePtr: DESCracker instance to work on
 */
u64 DESCracker_GetEndKey(DESCracker *InstancePtr)
{
    Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    u32 high = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_ENDKEY_HIGH_OFFSET);
    u32 low = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_ENDKEY_LOW_OFFSET);

    return ((u64)high << 32) | low;
}

/**
 * Gets the result for the worker specified by the WORKER register
 * @param InstancePtr: DESCracker instance to work on
 * @param match_nbr: 0 or 1, corresponding to reference 1 or 2
 * @param key: key that raised a result
 */
void DESCracker_GetResult(DESCracker *InstancePtr, unsigned int *match_nbr, u64 *key)
{
    Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // read high first! (because of a problem in AXI)
    u32 high = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_MATCHOUT_HIGH_OFFSET);
    u32 low = DESCRACKER_IP_mReadReg(InstancePtr->BaseAddress, DESCRACKER_IP_MATCHOUT_LOW_OFFSET);

    u32 key_high = high & 0xFFFFFF;

    *key = ((u64)key_high << 32) | low;
    *match_nbr = high >> 31;
}