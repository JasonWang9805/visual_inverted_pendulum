/*
 * memoryOperateBase.h
 *
 *  Created on: May 11, 2020
 *      Author: lgh
 */

#ifndef INCLUDE_MEMORYOPERATEBASE_H_
#define INCLUDE_MEMORYOPERATEBASE_H_

#include "stdlib.h"
#include "stdio.h"

double** allocationMemoryDoulble(int r, int c);

int** allocationMemoryInt(int r, int c);

void* freeMemoryDoulble(double** p, int r);

void* freeMemoryInt(int** p, int r);

char** allocationMemoryString(int r, int c);

void* freeMemoryString(char** p, int r);


#endif /* INCLUDE_MEMORYOPERATEBASE_H_ */
