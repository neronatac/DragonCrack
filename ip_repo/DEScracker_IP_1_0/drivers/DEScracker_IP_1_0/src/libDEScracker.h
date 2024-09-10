
#ifndef LIBDESCRACKER_H
#define LIBDESCRACKER_H


/****************** Include Files ********************/
#include "xil_types.h"
#include "xstatus.h"
#include "xil_io.h"

/****************** Constants definition ********************/
#define DESCRACKER_IP_CONTROL_STATUS_OFFSET       0*4
#define DESCRACKER_IP_PLAINTEXT_HIGH_OFFSET       1*4
#define DESCRACKER_IP_PLAINTEXT_LOW_OFFSET        2*4
#define DESCRACKER_IP_MASK1_HIGH_OFFSET           3*4
#define DESCRACKER_IP_MASK1_LOW_OFFSET            4*4
#define DESCRACKER_IP_MASK2_HIGH_OFFSET           5*4
#define DESCRACKER_IP_MASK2_LOW_OFFSET            6*4
#define DESCRACKER_IP_REF1_HIGH_OFFSET            7*4
#define DESCRACKER_IP_REF1_LOW_OFFSET             8*4
#define DESCRACKER_IP_REF2_HIGH_OFFSET            9*4
#define DESCRACKER_IP_REF2_LOW_OFFSET             10*4
#define DESCRACKER_IP_STARTKEYSx_OFFSET           11*4
#define DESCRACKER_IP_ENDKEYSx_OFFSET             133*4
#define DESCRACKER_IP_CURRENTKEYSx_OFFSET         255*4
#define DESCRACKER_IP_ENDEDOUTSx_OFFSET           377*4
#define DESCRACKER_IP_RESULTSAVAILABLEx_OFFSET    379*4
#define DESCRACKER_IP_RESULTSFULLx_OFFSET         381*4
#define DESCRACKER_IP_MATCHOUTSx_OFFSET           383*4


#define DESCRACKER_IP_RESET_SHIFT 0
#define DESCRACKER_IP_ENABLE_SHIFT 1


#define DESCRAKER_IP_STATUS_NOTRESET_DISABLED  0b00
#define DESCRAKER_IP_STATUS_NOTRESET_ENABLED   0b01
#define DESCRAKER_IP_STATUS_RESET_DISABLED     0b10
#define DESCRAKER_IP_STATUS_RESET_ENABLED      0b11


/**************************** Type Definitions *****************************/

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
	u16 DeviceId;		/**< Unique ID  of device */
	UINTPTR BaseAddress;	/**< Device base address */
	int WorkersNumber;	/**< Number of workers */
} DESCracker_Config;

/**
 * The DEScracker driver instance data. The user is required to allocate a
 * variable of this type for every DEScracker in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	UINTPTR BaseAddress;	/**< Device base address */
	u32 IsReady;		/**< Device is initialized and ready */
	int WorkersNumber;	/**< Number of workers */
} DESCracker;

/**
 *
 * Write a value to a DESCRACKER_IP register. A 32 bit write is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is written.
 *
 * @param   BaseAddress is the base address of the DESCRACKER_IPdevice.
 * @param   RegOffset is the register offset from the base to write to.
 * @param   Data is the data written to the register.
 *
 * @return  None.
 *
 * @note
 * C-style signature:
 * 	void DESCRACKER_IP_mWriteReg(u32 BaseAddress, unsigned RegOffset, u32 Data)
 *
 */
#define DESCRACKER_IP_mWriteReg(BaseAddress, RegOffset, Data) \
  	Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))

/**
 *
 * Read a value from a DESCRACKER_IP register. A 32 bit read is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is read from the register. The most significant data
 * will be read as 0.
 *
 * @param   BaseAddress is the base address of the DESCRACKER_IP device.
 * @param   RegOffset is the register offset from the base to write to.
 *
 * @return  Data is the data from the register.
 *
 * @note
 * C-style signature:
 * 	u32 DESCRACKER_IP_mReadReg(u32 BaseAddress, unsigned RegOffset)
 *
 */
#define DESCRACKER_IP_mReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))

/************************** Function Prototypes ****************************/
/**
 *
 * Run a self-test on the driver/device. Note this may be a destructive test if
 * resets of the device are performed.
 *
 * If the hardware system is not built correctly, this function may never
 * return to the caller.
 *
 * @param   baseaddr_p is the base address of the DESCRACKER_IP instance to be worked on.
 *
 * @return
 *
 *    - XST_SUCCESS   if all self-test code passed
 *    - XST_FAILURE   if any self-test code failed
 *
 * @note    Caching must be turned off for this function to work.
 * @note    Self test may fail if data memory and device are not on the same bus.
 *
 */
XStatus DESCRACKER_IP_Reg_SelfTest(void * baseaddr_p);

/*
 * Initialization functions in DEScracker_IP_sinit.c
 */
DESCracker_Config *DESCracker_LookupConfig(u16 DeviceId);
XStatus DESCracker_Initialize(DESCracker * InstancePtr, u16 DeviceId);


/*
 * API Basic functions implemented in DEScracker_IP.c
 */
XStatus DESCracker_CfgInitialize(DESCracker * InstancePtr, DESCracker_Config * Config, UINTPTR EffectiveAddr);
void DESCracker_GetVersion(DESCracker *InstancePtr, int* major, int* minor);
int DESCracker_GetStatus(DESCracker *InstancePtr);
void DESCracker_Reset(DESCracker *InstancePtr);
void DESCracker_Enable(DESCracker *InstancePtr);
void DESCracker_Disable(DESCracker *InstancePtr);
void DESCracker_SetPlaintext(DESCracker *InstancePtr, u64 plaintext);
u64 DESCracker_GetPlaintext(DESCracker *InstancePtr);
void DESCracker_SetMask(DESCracker *InstancePtr, unsigned int number, u64 mask);
u64 DESCracker_GetMask(DESCracker *InstancePtr, unsigned int number);
void DESCracker_SetRef(DESCracker *InstancePtr, unsigned int number, u64 ref);
u64 DESCracker_GetRef(DESCracker *InstancePtr, unsigned int number);
void DESCracker_SetStartKey(DESCracker *InstancePtr, unsigned int number, u64 key);
u64 DESCracker_GetStartKey(DESCracker *InstancePtr, unsigned int number);
void DESCracker_SetEndKey(DESCracker *InstancePtr, unsigned int number, u64 key);
u64 DESCracker_GetEndKey(DESCracker *InstancePtr, unsigned int number);
u64 DESCracker_GetCurrentKey(DESCracker *InstancePtr, unsigned int number);
int DESCracker_Ended(DESCracker *InstancePtr, unsigned int number);
u64 DESCracker_EndedAll(DESCracker *InstancePtr);
int DESCracker_ResultAvailable(DESCracker *InstancePtr, unsigned int number);
u64 DESCracker_ResultAvailableAll(DESCracker *InstancePtr);
int DESCracker_ResultFull(DESCracker *InstancePtr, unsigned int number);
u64 DESCracker_ResultFullAll(DESCracker *InstancePtr);
void DESCracker_GetResult(DESCracker *InstancePtr, unsigned int number, int *match_nbr, u64 *key);

#endif // LIBDESCRACKER_H
