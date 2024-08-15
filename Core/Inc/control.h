/*
 * control.h
 *
 *  Created on: Aug 11, 2024
 *      Author: nv
 */

#ifndef INC_CONTROL_H_
#define INC_CONTROL_H_

#include "peripheral.h"
#include <stdlib.h>
#include <string.h>

void Go();

extern bool dataAvailable;
extern char data[6];
void DataAvailable();
void DataInit();
void DataWrite();


#endif /* INC_CONTROL_H_ */
