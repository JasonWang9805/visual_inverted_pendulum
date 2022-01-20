/*
 * saveData.h
 *
 *  Created on: 2017年10月18日
 *      Author: hanbing
 */

#ifndef INCLUDE_SAVEDATA_H_
#define INCLUDE_SAVEDATA_H_

/*创建保存数据
 * name 数据名字(对应数据文件名字)
 * */
int CreateSaveData(char* name);

/*创建保存数据
 * name 数据名字(对应数据文件名字)
 * type: 数据类型  0:int,  1:double
 * row: 数据队列的最大行数（过小可能存在数据丢失现象,但不是所要存数据的最大组数。因为利用RSaveData()写数据和保存数据到文件是同步进行的，所以该数也没有必要过大，大小根据系统写文件速度而定，一般100左右足够）
 * cow: 数据的列数
 *
 * return 0:right, other: wrong
 * */
int CreateSaveData1(char* name, int type, int row, int col);

int DeleteSaveData(char* name);

/*保存数据
 * data 存放数据
 * return 0 right ,-1 队列已满, -2数据空间不存在
 * */
int  RSaveData(char* name, void* data );

/*保持数据
 * data 存放数据
 * col 实际保存数据列数
 * return 0 right ,-1 队列已满, -2数据空间不存在
 * */
int  RSaveData1(char* name, void* data, int col);



/*保存数据（无需创建数据空间）
 * data 存放数据
 * col 数据列数
 * return 0 right ,-1 队列已满, -2数据空间不存在, -3:空间创建失败
 * */
int RSaveDataFast(char* name, void* data, int col);

/*保存数据（无需创建数据空间）
 * data 存放数据
 * type: 数据类型  0:int,  1:double
 * row: 数据队列的最大行数（过小可能存在数据丢失现象）
 * cow: 数据的列数
 * return 0 right ,-1 队列已满, -2数据空间不存在, -3:空间创建失败
 * */
int RSaveDataFast1(char* name,int type, int row, int col, void* data );

#endif /* INCLUDE_SAVEDATA_H_ */
