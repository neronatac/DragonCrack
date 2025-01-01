#ifndef LIBDESCRACKER_H
#define LIBDESCRACKER_H


/****************** Include Files ********************/
#include "xil_types.h"
#include "xstatus.h"
#include "xil_io.h"

/****************** Constants definition ********************/
#define DESCRACKER_IP_VERSION_OFFSET              0*4
#define DESCRACKER_IP_RESETS_OFFSET               1*4
#define DESCRACKER_IP_PLAINTEXT_HIGH_OFFSET       2*4
#define DESCRACKER_IP_PLAINTEXT_LOW_OFFSET        3*4
#define DESCRACKER_IP_MASK1_HIGH_OFFSET           4*4
#define DESCRACKER_IP_MASK1_LOW_OFFSET            5*4
#define DESCRACKER_IP_MASK2_HIGH_OFFSET           6*4
#define DESCRACKER_IP_MASK2_LOW_OFFSET            7*4
#define DESCRACKER_IP_REF1_HIGH_OFFSET            8*4
#define DESCRACKER_IP_REF1_LOW_OFFSET             9*4
#define DESCRACKER_IP_REF2_HIGH_OFFSET            10*4
#define DESCRACKER_IP_REF2_LOW_OFFSET             11*4
#define DESCRACKER_IP_ENDEDOUTS_OFFSET            12*4
#define DESCRACKER_IP_RESULTSAVAILABLE_OFFSET     13*4
#define DESCRACKER_IP_RESULTSFULL_OFFSET          14*4
#define DESCRACKER_IP_WORKER_OFFSET               15*4
#define DESCRACKER_IP_FIXEDKEY_OFFSET             16*4
#define DESCRACKER_IP_MATCHOUT_HIGH_OFFSET        17*4
#define DESCRACKER_IP_MATCHOUT_LOW_OFFSET         18*4


/**************************** Type Definitions *****************************/

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
	u16 DeviceId;		/**< Unique ID  of device */
	UINTPTR BaseAddress;	/**< Device base address */
	int WorkersNumber;	/**< Number of workers */
	int VariablePartWidth; /**< Width of the variable part of the key (in bits)*/
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
	int VariablePartWidth; /**< Width of the variable part of the key (in bits)*/
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

/*
 * Initialization functions in DEScracker_IP_sinit.c
 */
DESCracker_Config *DESCracker_LookupConfig(u16 DeviceId);
XStatus DESCracker_Initialize(DESCracker * InstancePtr, u16 DeviceId);


/*
 * API Basic functions implemented in DEScracker_IP.c
 */
XStatus DESCracker_CfgInitialize(DESCracker * InstancePtr, DESCracker_Config * Config, UINTPTR EffectiveAddr);
void DESCracker_GetVersion(DESCracker *InstancePtr, unsigned int* major, unsigned int* minor);
u32 DESCracker_GetResets(DESCracker *InstancePtr);
void DESCracker_Reset(DESCracker *InstancePtr, u32 workers);
void DESCracker_SetPlaintext(DESCracker *InstancePtr, u64 plaintext);
u64 DESCracker_GetPlaintext(DESCracker *InstancePtr);
void DESCracker_SetMask(DESCracker *InstancePtr, unsigned int number, u64 mask);
u64 DESCracker_GetMask(DESCracker *InstancePtr, unsigned int number);
void DESCracker_SetRef(DESCracker *InstancePtr, unsigned int number, u64 ref);
u64 DESCracker_GetRef(DESCracker *InstancePtr, unsigned int number);
unsigned int DESCracker_Ended(DESCracker *InstancePtr, unsigned int number);
u32 DESCracker_EndedAll(DESCracker *InstancePtr);
unsigned int DESCracker_ResultAvailable(DESCracker *InstancePtr, unsigned int number);
u32 DESCracker_ResultAvailableAll(DESCracker *InstancePtr);
unsigned int DESCracker_ResultFull(DESCracker *InstancePtr, unsigned int number);
u32 DESCracker_ResultFullAll(DESCracker *InstancePtr);
void DESCracker_SetWorker(DESCracker *InstancePtr, unsigned int number);
unsigned int DESCracker_GetWorker(DESCracker *InstancePtr);
void DESCracker_SetFixedKey(DESCracker *InstancePtr, u32 key);
u32 DESCracker_GetFixedKey(DESCracker *InstancePtr);
void DESCracker_GetResult(DESCracker *InstancePtr, unsigned int *match_nbr, u64 *key);

#endif // LIBDESCRACKER_H
