/*
 * dealData.h
 *
 *  Created on: Aug 5, 2019
 *      Author: lgh
 */



#ifndef DEALDATA_H_
#define DEALDATA_H_

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include "rqueue.h"
#include <string.h>

#define MAX_CHAR_NUM 300

//每一个链表结构体;
typedef struct {
	R_QUEUE queue_torque;
	FILE* _DATA_FILE;
	pthread_mutex_t filelock;  //互斥锁;
	int flag;
	pthread_attr_t attr; //线程属性;
	pthread_t saveDataPth;
}SaveData;


//存储数据的链表结构体;
typedef struct SaveDataList{
	SaveData saveData;  //用于存储数据的结构体;
	char Name[MAX_CHAR_NUM]; //节点名称;
	int index; //节点编号;
	struct SaveDataList* next; //指向下一个节点的指针;
}SaveDataList;


void CreateSaveDataList();

SaveDataList* FindSaveDataList(char* Name);

SaveDataList* FindSaveDataList_index(int index);

int SaveDataListInsert2(char* Name, int type, int row, int cow);

int SaveDataListInsert(char* Name);

int ReadDataListInsert2(char* Name, int type, int row, int cow);

int ReadDataListInsert(char* Name);

int SaveDataListDelete(char* Name);

SaveData* FindSaveData(char* Name);

SaveData* FindSaveData_index(int index);

int getSaveData_num();

char* getSaveData_name(int index,char* name);


#endif /* INCLUDE_DEALDATA_H_ */
