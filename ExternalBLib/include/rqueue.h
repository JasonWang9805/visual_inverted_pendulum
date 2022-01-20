/*
 * rqueue.h
 *
 *  Created on: 2018-5-31
 *      Author: hanbing
 */

#ifndef RQUEUE_H_
#define RQUEUE_H_
#include <stdlib.h>
#include <stdio.h>

#ifndef ROW
#define ROW 200
#endif
#ifndef COW
#define COW 200
#endif
typedef struct{
	int** data_i;     // 整型数据;
	double** data_f;  // 浮点型数据;

	int type;         // 数据类型;
	int queue_row;    // 队列的行;
	int queue_cow;    // 队列的列;

	int head;  //头;
	int tail;  //尾;
	int data_num;  //数据个数;
	int queue_full_flag;   //队列是否已满，0：未满，1：已满;
	pthread_mutex_t lock;  //互斥锁;
}R_QUEUE;


/*initialize queue
 * R_DQUEUE: queue data
 *type: 0:int 1:double
 *
 * return 0：right; other: wrong
 * */
int queue_init2(R_QUEUE* r_queue,int type, int row, int cow);

int queue_init(R_QUEUE* r_queue);

void queue_free(R_QUEUE* r_queue);

/*
 * return 0: 正常返回，-1:队列满（丢弃对尾）：<-1:wrong
 * */
int queue_write(R_QUEUE* r_queue, void* data, int cow);

int queue_write1(R_QUEUE* r_queue, void* data);





int queue_write_int(R_QUEUE* r_queue, int* data, int cow);

int queue_write_double(R_QUEUE* r_queue, double* data, int cow);

int queue_read_int(R_QUEUE* r_queue, int* data, int cow, int index);


int queue_read_int_all(R_QUEUE* r_queue, int** data, int cow);

int queue_only_read_int(R_QUEUE* r_queue, int* data, int cow, int index);

int queue_read_double(R_QUEUE* r_queue, double* data, int cow, int index);

int queue_read_double_all(R_QUEUE* r_queue, double** data, int cow);
int queue_only_read_double(R_QUEUE* r_queue, double* data, int cow, int index);

/*//读取并弹出队列
 * cow:读取cow列
 * return 0: 正常返回，-1:队列空：<-1:wrong
 * */
int queue_read(R_QUEUE* r_queue, void* data, int cow);


//读取队列头并弹出队列
int queue_read1(R_QUEUE* r_queue, void* data);

//读取指定索引数据（0,1,2...）并弹出该数据及之前数据
int queue_read2(R_QUEUE* r_queue, void* data, int index);

//读取并弹出队列（阻塞）
int queue_read_block(R_QUEUE* r_queue, void* data);

//读取指定索引数据（0,1,2...）,数据不弹出
int queue_read_only(R_QUEUE* r_queue, void* data, int index);

//读取队列头数据,数据不弹出
int queue_read_only1(R_QUEUE* r_queue, void* data);

int queue_read_all(R_QUEUE* r_queue, void** data, int cow);

int queue_read_all1(R_QUEUE* r_queue, void** data);

int queue_delete(R_QUEUE* r_queue);

int is_queue_full(R_QUEUE* r_queue);

int is_queue_full_lock(R_QUEUE* r_queue);

int is_queue_empty_lock(R_QUEUE* r_queue);

int get_queue_data_num(R_QUEUE* r_queue);

int get_queue_data_num_lock(R_QUEUE* r_queue);

int get_queue_data_cow_num(R_QUEUE* r_queue);

int get_queue_cow_num(R_QUEUE* r_queue);

int get_queue_data_type(R_QUEUE* r_queue);

int get_queue_size(R_QUEUE* r_queue);

#endif /* RQUEUE_H_ */
















