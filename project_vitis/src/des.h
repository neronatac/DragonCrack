#ifndef DES_H
#define DES_H


// Status values
#define DES_STATUS_WAITING  0x01
#define DES_STATUS_RUNNING  0x02
#define DES_STATUS_FINISHED 0x03
#define DES_STATUS_ERROR    0xFF

/**
 * Initializes the DESCracker peripheral
 * @return
 * 		- XST_SUCCESS if successful
 * 		- another Xstatus if not
 */
XStatus DESCrackerInit();

/**
 * Executes commands associated with the DES peripheral.
 * See `process` for more details.
 */
void process_des(u8 cmd, u8 len, u8* data);

/**
 * Manages the exhaust of keys (must be called in a loop)
 */
void handle_exhaust();

#endif // DES_H
