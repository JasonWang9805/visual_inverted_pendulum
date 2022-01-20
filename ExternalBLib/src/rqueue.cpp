/*
 * rqueue.c
 *
 *  Created on: 2018-5-31
 *      Author: hanbing
 */
#include "rqueue.h"
#include "pthread.h"
#include "unistd.h"
#include "memoryOperateBase.h"

int queue_init2(R_QUEUE* r_queue,int type, int row, int cow)
{
	r_queue->type=type;
	r_queue->queue_row=row;
	r_queue->queue_cow=cow;
	switch (r_queue->type)
	{
	case 0: //0，就是存储整型数据, 创建内存空间;
		r_queue->data_i=allocationMemoryInt(r_queue->queue_row, r_queue->queue_cow);
		r_queue-> data_f=NULL;
		break;
	case 1: //1，就是浮点型数据， 创建内存空间;
		r_queue-> data_f=allocationMemoryDoulble(r_queue->queue_row, r_queue->queue_cow);
		r_queue->data_i=NULL;
		break;
	default:
		printf("queue data type error!\n");
		return -1;
		break;
	}

	r_queue->head=0;
	r_queue->tail=0;
	r_queue->data_num=cow;
	r_queue->queue_full_flag=0;
	pthread_mutex_init(&(r_queue->lock),NULL);//初始化锁
	return 0;
}

int queue_init(R_QUEUE* r_queue)
{
	return queue_init2(r_queue,0, ROW, COW);
}

void queue_free(R_QUEUE* r_queue)
{
	switch (r_queue->type)
	{
	case 0:
		if (NULL!=r_queue->data_i)
		{
			freeMemoryInt(r_queue->data_i, r_queue->queue_row);
			r_queue->data_i=NULL;
		}
		break;
	case 1:
		if (NULL!=r_queue->data_f)
		{
			freeMemoryDoulble(r_queue-> data_f, r_queue->queue_row);
			r_queue-> data_f=NULL;
		}
		break;
	default:
		printf("queue data type error!\n");
		break;
	}
}

int queue_write_int(R_QUEUE* r_queue, int* data, int cow)
{
	if (r_queue->queue_cow<cow)
	{
		printf("write data are too many\n");
		return -2;
	}

	if (cow<=0)
	{
		return 0;
	}

	int i=0;
	r_queue->data_num=cow;
	for (i=0;i<cow;i++)
	{
		r_queue->data_i[r_queue->tail][i]=data[i];
	}
	r_queue->tail++;
	if (r_queue->tail%(r_queue->queue_row)==0)
	{
		r_queue->tail=0;
	}

	if (0!=r_queue->queue_full_flag)//丢弃队尾
	{
		r_queue->head++;
		if (r_queue->head%(r_queue->queue_row)==0)
		{
			r_queue->head=0;
		}
		return -1;
	}

	if (r_queue->head==r_queue->tail)
	{
		r_queue->queue_full_flag=1;
		return -1;
	}
	return 0;
}

 int queue_write_double(R_QUEUE* r_queue, double* data, int cow)
{
	if (r_queue->queue_cow<cow)
	{
		printf("write data are too many\n");
		return -2;
	}

	if (cow<=0)
	{
		return 0;
	}

	int i=0;
	r_queue->data_num=cow;
	for (i=0;i<cow;i++)
	{
		r_queue->data_f[r_queue->tail][i]=data[i];
	}
	r_queue->tail++;
	if (r_queue->tail%(r_queue->queue_row)==0)
	{
		r_queue->tail=0;
	}

	if (0!=r_queue->queue_full_flag)//丢弃队尾
	{
		r_queue->head++;
		if (r_queue->head%(r_queue->queue_row)==0)
		{
			r_queue->head=0;
		}
		return -1;
	}

	if (r_queue->head==r_queue->tail)
	{
		r_queue->queue_full_flag=1;
		return -1;
	}
	return 0;
}

int queue_write(R_QUEUE* r_queue, void* data, int cow)
{
	pthread_mutex_lock(&(r_queue->lock));
	int ret=0;
	switch (r_queue->type)
	{
	case 0:
		ret=queue_write_int(r_queue, (int*)data, cow);
		break;
	case 1:
		ret=queue_write_double(r_queue, (double*)data, cow);
		break;
	default:
		printf("queue data type error!\n");
		ret=-3;
		break;
	}
	pthread_mutex_unlock(&(r_queue->lock));
	return ret;
}

int queue_write1(R_QUEUE* r_queue, void* data)
{
	int col=get_queue_data_cow_num(r_queue);
	return queue_write(r_queue, data, col);
}

int queue_read_int(R_QUEUE* r_queue, int* data, int cow, int index)
{
	if (r_queue->queue_cow<cow)
	{
		printf("read data are too many\n");
		return -2;
	}

	int i=0;
	int count_index=0;
	while (count_index<=index)
	{
		if ((r_queue->head==r_queue->tail)&&(0==r_queue->queue_full_flag))
		{
			printf("queue is empty!\n");
			return -1;
		}
		r_queue->queue_full_flag=0;

		if (count_index==index)
		{
			if (NULL!=data)
			{
				for (i=0;i<cow;i++)
				{
					data[i]=r_queue->data_i[r_queue->head][i];
				}
			}
		}

		r_queue->head++;
		if (r_queue->head%(r_queue->queue_row)==0)
		{
			r_queue->head=0;
		}
		count_index++;
	}
	return 0;
}


int queue_read_int_all(R_QUEUE* r_queue, int** data, int cow)
{
	if (r_queue->queue_cow<cow)
	{
		printf("read data are too many\n");
		return -2;
	}

	int i=0;
	if ((r_queue->head==r_queue->tail)&&(0==r_queue->queue_full_flag))
	{
//		printf("queue is empty!\n");
		return -1;
	}

	int head=r_queue->head;
	int count=0;
	while(count<get_queue_data_num(r_queue))
	{
		for (i=0;i<cow;i++)
		{
			data[count][i]=r_queue->data_i[head][i];
		}
		count++;

		head++;
		if (head%(r_queue->queue_row)==0)
		{
			head=0;
		}
	}
	return 0;
}

int queue_only_read_int(R_QUEUE* r_queue, int* data, int cow, int index)
{
	if (r_queue->queue_cow<cow)
	{
		printf("read data are too many\n");
		return -2;
	}

	int i=0;
	int count_index=0;
	int head=r_queue->head;
	int queue_full_flag=r_queue->queue_full_flag;
	while (count_index<=index)
	{
		if ((head==r_queue->tail)&&(0==queue_full_flag))
		{
	//		printf("queue is empty!\n");
			return -1;
		}
		queue_full_flag=0;
		if (count_index==index)
		{
			if (NULL!=data)
			{
				for (i=0;i<cow;i++)
				{
					data[i]=r_queue->data_i[head][i];
				}
			}
		}

		head++;
		if (head%(r_queue->queue_row)==0)
		{
			head=0;
		}
		count_index++;
	}

	return 0;
}

int queue_read_double(R_QUEUE* r_queue, double* data, int cow, int index)
{
	if (r_queue->queue_cow<cow)
	{
		printf("read data are too many\n");
		return -2;
	}

	int i=0;
	int count_index=0;
	while (count_index<=index)
	{
		if ((r_queue->head==r_queue->tail)&&(0==r_queue->queue_full_flag))
		{
	//		printf("queue is empty!\n");
			return -1;
		}
		r_queue->queue_full_flag=0;

		if (count_index==index)
		{
			if (NULL!=data)
			{
				for (i=0;i<cow;i++)
				{
					data[i]=r_queue->data_f[r_queue->head][i];
				}
			}
		}

		r_queue->head++;
		if (r_queue->head%(r_queue->queue_row)==0)
		{
			r_queue->head=0;
		}
		count_index++;
	}

	return 0;
}

int queue_read_double_all(R_QUEUE* r_queue, double** data, int cow)
{
	if (r_queue->queue_cow<cow)
	{
		printf("read data are too many\n");
		return -2;
	}

	int i=0;
	if ((r_queue->head==r_queue->tail)&&(0==r_queue->queue_full_flag))
	{
//		printf("queue is empty!\n");
		return -1;
	}

	int head=r_queue->head;
	int count=0;
	while(count<get_queue_data_num(r_queue))
	{
		for (i=0;i<cow;i++)
		{
			data[count][i]=r_queue->data_f[head][i];
		}
		count++;

		head++;
		if (head%(r_queue->queue_row)==0)
		{
			head=0;
		}
	}
	return 0;
}

int queue_only_read_double(R_QUEUE* r_queue, double* data, int cow, int index)
{
	if (r_queue->queue_cow<cow)
	{
		printf("read data are too many\n");
		return -2;
	}

	int i=0;
	int count_index=0;
	int head=r_queue->head;
	int queue_full_flag=r_queue->queue_full_flag;
	while (count_index<=index)
	{
		if ((head==r_queue->tail)&&(0==queue_full_flag))
		{
	//		printf("queue is empty!\n");
			return -1;
		}
		queue_full_flag=0;
		if (count_index==index)
		{
			if (NULL!=data)
			{
				for (i=0;i<cow;i++)
				{
					data[i]=r_queue->data_f[head][i];
				}
			}
		}

		head++;
		if (head%(r_queue->queue_row)==0)
		{
			head=0;
		}
		count_index++;
	}

	return 0;
}

/*//读取并弹出队列
 * cow:读取cow列
 * return 0: 正常返回，-1:队列空：<-1:wrong
 * */
int queue_read(R_QUEUE* r_queue, void* data, int cow)
{
	pthread_mutex_lock(&(r_queue->lock)); //打开锁;
	int ret=0;
	switch (r_queue->type)
	{
	case 0:
		ret=queue_read_int(r_queue, (int*)data, cow, 0);
		break;
	case 1:
		ret=queue_read_double(r_queue, (double*)data, cow,0);
		break;
	default:
		printf("queue data type error!\n");
		ret=-3;
		break;
	}
	pthread_mutex_unlock(&(r_queue->lock));
	return ret;
}

//读取并弹出队列（阻塞）
int queue_read_block(R_QUEUE* r_queue, void* data)
{
	int ret=-1;
	while (1)
	{
		ret=queue_read1(r_queue, data);
		if (-1!=ret)
		{
			break;
		}
		usleep(200);  //200us
	}
	return ret;
}

//读取队列头并弹出队列
int queue_read1(R_QUEUE* r_queue, void* data)
{
	int col=get_queue_data_cow_num(r_queue);
	return queue_read(r_queue, data, col);
}

//读取指定索引数据（0,1,2...）并弹出该数据及之前数据
int queue_read2(R_QUEUE* r_queue, void* data, int index)
{
	pthread_mutex_lock(&(r_queue->lock));
	int col=get_queue_data_cow_num(r_queue);
	int ret=0;
	switch (r_queue->type)
	{
	case 0:
		ret=queue_read_int(r_queue, (int*)data, col,index);
		break;
	case 1:
		ret=queue_read_double(r_queue, (double*)data, col,index);
		break;
	default:
		printf("queue data type error!\n");
		ret=-3;
		break;
	}
	pthread_mutex_unlock(&(r_queue->lock));
	return ret;
}

//读取指定索引数据（0,1,2...）,数据不弹出
int queue_read_only(R_QUEUE* r_queue, void* data, int index)
{
	pthread_mutex_lock(&(r_queue->lock));  //上锁;
	int col=get_queue_data_cow_num(r_queue);
	int ret=0;
	switch (r_queue->type)
	{
	case 0:
		ret=queue_only_read_int(r_queue, (int*)data, col,index);
		break;
	case 1:
		ret=queue_only_read_double(r_queue, (double*)data, col,index);
		break;
	default:
		printf("queue data type error!\n");
		ret=-3;
		break;
	}
	pthread_mutex_unlock(&(r_queue->lock));
	return ret;
}

//读取队列头数据,数据不弹出
int queue_read_only1(R_QUEUE* r_queue, void* data)
{
	return queue_read_only(r_queue, data, 0);
}

int queue_read_all(R_QUEUE* r_queue, void** data, int cow)
{
	pthread_mutex_lock(&(r_queue->lock)); //对队列上锁;
	int ret=0;
	switch (r_queue->type)
	{
	case 0:
		ret=queue_read_int_all(r_queue, (int**)data, cow);
		break;
	case 1:
		ret=queue_read_double_all(r_queue, (double**)data, cow);
		break;
	default:
		printf("queue data type error!\n");
		ret=-3;
		break;
	}
	pthread_mutex_unlock(&(r_queue->lock));
	return ret;
}

int queue_read_all1(R_QUEUE* r_queue, void** data)
{
	int col=get_queue_data_cow_num(r_queue);
	return queue_read_all(r_queue, data, col);
}

int queue_delete(R_QUEUE* r_queue)
{
	pthread_mutex_lock(&(r_queue->lock));
	if ((r_queue->head==r_queue->tail)&&(0==r_queue->queue_full_flag))
	{
//		printf("queue is empty!\n");
		return -1;
	}
	r_queue->queue_full_flag=0;
	r_queue->tail--;
	if (-1==r_queue->tail)
	{
		r_queue->tail=r_queue->queue_row-1;
	}
	pthread_mutex_unlock(&(r_queue->lock));
	return 0;
}

int is_queue_full(R_QUEUE* r_queue)
{
	return r_queue->queue_full_flag;
}

int is_queue_full_lock(R_QUEUE* r_queue)
{
	pthread_mutex_lock(&(r_queue->lock));
	int res=is_queue_full(r_queue);
	pthread_mutex_unlock(&(r_queue->lock));
	return res;
}

int is_queue_empty_lock(R_QUEUE* r_queue)
{
	pthread_mutex_lock(&(r_queue->lock));
	int ret=get_queue_data_num(r_queue);
	if (0==ret)
	{
		ret=1;
	}
	else
	{
		ret=0;
	}
	pthread_mutex_unlock(&(r_queue->lock));
	return ret;
}

int get_queue_data_num(R_QUEUE* r_queue)
{
	int res=r_queue->tail-r_queue->head;
	if (res<0||(1==is_queue_full(r_queue)))
	{
		res=r_queue->queue_row+res;
	}
	return res;
}

int get_queue_data_num_lock(R_QUEUE* r_queue)
{
	pthread_mutex_lock(&(r_queue->lock));
	int res=get_queue_data_num(r_queue);
	pthread_mutex_unlock(&(r_queue->lock));
	return res;
}

int get_queue_data_cow_num(R_QUEUE* r_queue)
{
	return r_queue->data_num;
}

//读取队列的行数;
int get_queue_cow_num(R_QUEUE* r_queue)
{
	return r_queue->queue_cow;
}

//读取队列的数据类型;
int get_queue_data_type(R_QUEUE* r_queue)
{
	return r_queue->type;
}

int get_queue_size(R_QUEUE* r_queue)
{
	return r_queue->queue_row;
}


