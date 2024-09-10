#ifndef DES_H
#define DES_H

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

#endif // DES_H
