/*
 * platform.h
 *
 *  Created on: 13 mars 2023
 *      Author: s.marcuzzi
 */

#ifndef SRC_PLATFORM_H_
#define SRC_PLATFORM_H_

#include "xscutimer.h"


#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_0_BASEADDR


/* Platform timer is calibrated for 250 ms, so kept interval value 4 to call
 * eth_link_detect() at every one second
 */
#define ETH_LINK_DETECT_INTERVAL 4


void init_platform();
void cleanup_platform();
void platform_setup_timer();
void platform_setup_interrupts();
void platform_enable_interrupts();
void timer_callback(XScuTimer * TimerInstance);

#endif /* SRC_PLATFORM_H_ */
