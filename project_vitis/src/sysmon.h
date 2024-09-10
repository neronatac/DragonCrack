/*
 * sysmon.h
 *
 *  Created on: 9 mars 2023
 *      Author: s.marcuzzi
 */

#ifndef SRC_SYSMON_H_
#define SRC_SYSMON_H_

#include "xstatus.h"

XStatus sysmonInit();
void sysmonGetTempMeasurements(int* current, int* max, int* min);

#endif /* SRC_SYSMON_H_ */
