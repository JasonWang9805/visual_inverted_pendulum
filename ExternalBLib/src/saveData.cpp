/*
 * saveData.c
 *
 *  Created on: 2017年10月18日
 *      Author: hanbing
 */

#include "dealData.h"
#include "saveData.h"


//------------------public---------------
/*创建保存数据
 * name 数据名字(对应数据文件名字)
 * */
int CreateSaveData(char* name)
{
	CreateSaveDataList();
	return SaveDataListInsert(name);
}

/*创建保存数据
 * name 数据名字(对应数据文件名字)
 * type: 数据类型  0:int,  1:double
 * row: 数据队列的最大行数 (过小可能存在数据丢失现象)
 * cow: 数据的列数
 * */
int CreateSaveData1(char* name, int type, int row, int col)
{
	CreateSaveDataList();
	return SaveDataListInsert2(name, type, row, col);
}


int DeleteSaveData(char* name)
{
	return SaveDataListDelete(name);
}

/*保存数据
 * data 存放数据
 * return 0 right ,-1 队列已满, -2数据空间不存在
 * */
int  RSaveData(char* name, void* data )
{
	SaveData* sd=FindSaveData(name);
	if (NULL==sd)
	{
		return -2;
	}
	int col=get_queue_data_cow_num(&(sd->queue_torque));
	return queue_write(&(sd->queue_torque), data, col);
}

/*保存数据
 * data 存放数据
 * col 实际保存数据列数
 * return 0 right ,-1 队列已满, -2数据空间不存在
 * */
int  RSaveData1(char* name, void* data, int col)
{
	SaveData* sd=FindSaveData(name);
	if (NULL==sd)
	{
		return -2;
	}
	int _col=get_queue_cow_num(&(sd->queue_torque));
	if (col>_col)
	{
		 col=_col;
	}
	return queue_write(&(sd->queue_torque), data, col);
}

/*保存数据 (无需创建数据空间)
 * data 存放数据
 * col 数据列数
 * return 0 right ,-1 队列已满, -2数据空间不存在, -3:空间创建失败
 * */
int RSaveDataFast(char* name, void* data, int col)
{
	if (-1>CreateSaveData1(name, 1, ROW, col))
	{
		return -3;
	}
	return RSaveData(name, data);
}


/*******************************************
 * 保存数据（无需创建数据空间）
 * data 存放数据
 * type: 数据类型  0:int,  1:double
 * row: 数据队列的最大行数（过小可能存在数据丢失现象）
 * cow: 数据的列数
 * return 0 right ,-1 队列已满, -2数据空间不存在, -3:空间创建失败
 ********************************************/
int RSaveDataFast1(char* name, int type, int row, int col, void* data )
{
	if (-1 > CreateSaveData1(name, type, row, col))
	{
		return -3;
	}
	return RSaveData(name, data );
}


