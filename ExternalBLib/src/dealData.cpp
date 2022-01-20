/*
 * dealData.c
 *
 *  Created on: Aug 5, 2019
 *      Author: lgh
 */


#include "dealData.h"

//---------------private-------------------

static SaveDataList* SaveDataL=NULL;

//保存数据线程的响应函数;

static void* _saveDataToFile(void* arg)
{
	SaveData* savedata=(SaveData*)arg;
	int i=0;
	int err=0;
	int _data_num=get_queue_cow_num(&(savedata->queue_torque));

	double* ddata=(double*)malloc(_data_num*sizeof(double));
	int* idata=(int*)malloc(_data_num*sizeof(int));

	int type=get_queue_data_type(&(savedata->queue_torque));
	int qsize=get_queue_size(&(savedata->queue_torque));
	int qsize_2_3=qsize*2/3;
	int qsize_1_3=qsize/3;
	int qsizen=0;
	while(savedata->flag)
	{
		qsizen=get_queue_data_num_lock(&(savedata->queue_torque));
		if (qsizen>qsize_2_3)  //如果队列数据已经大于2/3了，则读的时候就快点读取;
		{
			usleep(100); //如果缓冲区队列数据比较多时，则快点读;
		}else if(qsizen<=qsize_1_3) //缓冲区数据比较少时，则慢点读;
		{
			usleep(1000);
		}else
		{
			usleep(500); //正常情况下读取数据;
		}
		if (0==type)  //整数类型;
		{
			err=queue_read1(&(savedata->queue_torque),  idata);
			if (-1==err)
			{
				continue;
			}
			int _data_num=get_queue_data_cow_num(&(savedata->queue_torque));
			pthread_mutex_lock(&(savedata->filelock));
			for(i=0;i<_data_num;i++)
			{
				fprintf(savedata->_DATA_FILE,"%d,",idata[i]); //把数据往文件中写;
			}
			fprintf(savedata->_DATA_FILE,"\n");
			fflush(savedata->_DATA_FILE);
			pthread_mutex_unlock(&(savedata->filelock));
		}
		else
		{
			err=queue_read1(&(savedata->queue_torque),  ddata);
			if (-1==err)
			{
				continue;
			}
			int _data_num=get_queue_data_cow_num(&(savedata->queue_torque));
			pthread_mutex_lock(&(savedata->filelock));
			for(i=0;i<_data_num;i++)
			{
				fprintf(savedata->_DATA_FILE,"%.15f,",ddata[i]);
			}
			fprintf(savedata->_DATA_FILE,"\n");
			fflush(savedata->_DATA_FILE);
			pthread_mutex_unlock(&(savedata->filelock));
		}
	}

	free(ddata);
	free(idata);
	savedata->flag=-1;
	return NULL;
}

#define LINE_LENGTH 5000
static int __read_data_file_line(FILE*in, double* ddata, int* idata, int num)
{
	char line[LINE_LENGTH];
	char* pp=line;
	char tmp[100];
	int index_c=0;
	memset(tmp,0,100);
	int index=0;
	int _flag=0;
	memset(line,0,LINE_LENGTH);
	if (fgets(line, LINE_LENGTH, in)==NULL)
	{
		return 1;
	}

	while (!((*pp=='\n')||(*pp=='\r'||*pp=='\0')))
	{
		if ((*pp>='0'&&*pp<='9')||*pp=='.'||*pp=='-')
		{
			tmp[index_c++]=*pp;
			_flag=1;
		}
		else
		{
			if (1==_flag)
			{
				if (NULL!=ddata)
				{
					ddata[index]=atof(tmp);
				}
				if (NULL!=idata)
				{
					idata[index]=atoi(tmp);
				}

				memset(tmp,0,100);
				index_c=0;
				index++;
				_flag=0;
				if (index>=num)
				{
					return 0;
				}
			}
		}
		pp++;
	}
	if (0==index)  //此行无数据
	{
		return -1;
	}
	if (NULL!=ddata)
	{
		ddata[index]=atof(tmp);
	}
	if (NULL!=idata)
	{
		idata[index]=atoi(tmp);
	}
	return 0;
}


//保存数据线程
static void* _readDataToFile(void* arg)
{
	SaveData* savedata=(SaveData*)arg;
	int err=0;
	int _data_num=get_queue_cow_num(&(savedata->queue_torque));

	double* ddata=(double*)malloc(_data_num*sizeof(double));
	int* idata=(int*)malloc(_data_num*sizeof(int));
	int type=get_queue_data_type(&(savedata->queue_torque));
	int qsize=get_queue_size(&(savedata->queue_torque));
	int qsize_2_3=qsize*2/3;
	int qsize_1_3=qsize/3;
	int qsizen=0;
	while(savedata->flag)
	{
		qsizen=get_queue_data_num_lock(&(savedata->queue_torque));
		if (qsizen>qsize_2_3)
		{
			usleep(1000);
			continue;
		}else if(qsizen<=qsize_1_3)
		{
			usleep(100);
		}else
		{
			usleep(500);
		}
		if (0==type)
		{
			pthread_mutex_lock(&(savedata->filelock));
			err=__read_data_file_line(savedata->_DATA_FILE, NULL, idata,_data_num);
			pthread_mutex_unlock(&(savedata->filelock));
			if (1==err)
			{
				break;
			}else if(-1==err)
			{
				continue;
			}
			queue_write1(&(savedata->queue_torque),  idata);
		}
		else
		{
			pthread_mutex_lock(&(savedata->filelock));
			err=__read_data_file_line(savedata->_DATA_FILE, ddata, NULL,_data_num);
			pthread_mutex_unlock(&(savedata->filelock));
			if (1==err)
			{
				break;
			}else if(-1==err)
			{
				continue;
			}
			queue_write1(&(savedata->queue_torque),  ddata);
		}
	}

	free(ddata);
	free(idata);
	savedata->flag=-1;
	return NULL;
}


/****************************
 *  初始化存储数据队列;
 ****************************/
static int _init_save_data(SaveData* savedata, char* name, int type, int row, int col)
{
	int err=0;
	if (0!=queue_init2(&(savedata->queue_torque),type, row, col))
	{
		perror("initQueue failure\n");
		return -2;
	}
	pthread_mutex_init(&(savedata->filelock),NULL); //初始化锁


	char _name[MAX_CHAR_NUM];
	memset(_name, 0, sizeof(_name));
	if (strlen(name)>250)
	{
		sprintf(_name,"%s.txt","NULL");
	}
	else
	{
		sprintf(_name,"%s.txt",name);
	}
	if ((savedata->_DATA_FILE=fopen(_name, "w"))==NULL)
	{
		printf("cannot open %s\n", name);
		return -3;
	}
	savedata->flag=1;  //文件打开标志;

	//--------------------------------------
	pthread_attr_init( &(savedata->attr));
	pthread_attr_setdetachstate(&(savedata->attr),PTHREAD_CREATE_DETACHED);
	err=pthread_create(&(savedata->saveDataPth),&(savedata->attr), _saveDataToFile, savedata);
	if(err != 0)
	{
		printf(" save data \"%s\" pthread create error\n",_name);
		return -4;
	}

	return 0;
}

/*
 * type:0:int 1:double
 * */
static int _init_read_data(SaveData* savedata, char* name, int type, int row, int col)
{
	int err=0;
	if (0!=queue_init2(&(savedata->queue_torque),type, row, col))
	{
		printf("initQueue failure\n");
		return -2;
	}
	pthread_mutex_init(&(savedata->filelock),NULL);//初始化锁

	//-------------------------
	char _name[MAX_CHAR_NUM];
	memset(_name,0,sizeof(_name));
	if (strlen(name)>250)
	{
		sprintf(_name,"%s.txt","NULL");
	}
	else
	{
		sprintf(_name,"%s.txt",name);
	}
	if ((savedata->_DATA_FILE=fopen(_name, "r"))==NULL)
	{
		printf("cannot open %s\n", name);
		return -3;
	}
	savedata->flag=1;

	//------------------------
	pthread_attr_init( &(savedata->attr) );
	pthread_attr_setdetachstate(&(savedata->attr),PTHREAD_CREATE_DETACHED);
	err=pthread_create(&(savedata->saveDataPth),&(savedata->attr), _readDataToFile, savedata );
	if(err != 0)
	{
		printf(" read data \"%s\" pthread create error\n",_name);
		return -4;
	}

	return 0;
}

static void _delete_save_data(SaveData* savedata)
{
	if (-1!=savedata->flag)
	{
		savedata->flag=0;
		while (-1!=savedata->flag)
		{
			usleep(500);
		}
	}
	queue_free(&(savedata->queue_torque));
	fclose(savedata->_DATA_FILE);
}


//创建用于保存数据的线程链表,头节点;
void CreateSaveDataList()
{
	if (NULL!=SaveDataL)  //已经初始化
	{
		return;
	}
	SaveDataList* L=(SaveDataList*)malloc(sizeof(SaveDataList));
	memset(L->Name,'\0',sizeof(L->Name));
	strcpy(L->Name,"head");
    L->next=NULL;
    L->index=0;
    SaveDataL=L;
}

SaveDataList* FindSaveDataList(char* Name)
{
	SaveDataList* L=SaveDataL;
	SaveDataList* p=NULL;
	p=L->next;
	while(NULL!=p)
	{
		if (0==strcmp(p->Name, Name))
		{
			break;
		}
		p=p->next;
	}
	return p;
}

SaveDataList* FindSaveDataList_index(int index)
{
	SaveDataList* L=SaveDataL;
	SaveDataList* p=NULL;
	p=L->next;
	while(NULL!=p)
	{
		if (p->index==index)
		{
			break;
		}
		p=p->next;
	}
	return p;
}

//将名为 Name的节点插入到链表中;
int SaveDataListInsert2(char* Name, int type, int row, int cow)
{
	SaveDataList* L=SaveDataL;  //首先新创建一个链表节点，使其指向头节点;
	if (NULL!=FindSaveDataList(Name)) //如果该名称的头节点存在则不再继续创建;
	{
		//如果创建的名字已经存在;
      //  printf("%s is exist! Please rename!\n",Name);
		return -1;
	}
	SaveDataList* Lnext=(SaveDataList*)malloc(sizeof(SaveDataList)); //创建新的节点;
	Lnext->next=L->next;  //新节点的指针指向头节点的下一个节点;
	L->next=Lnext; //头节点的指针指向新创建的节点;
	memset(Lnext->Name,'\0',sizeof(Lnext->Name)); //给新创建的节点赋值;
	strcpy(Lnext->Name,Name);
	Lnext->index=L->index;
	L->index++;
	int err=_init_save_data(&(Lnext->saveData), Name, type,row , cow);
	if (0!=err)
	{
		printf("SaveDataListInsert failure\n");
		return err;
	}
    return 0;
}

int SaveDataListInsert(char* Name)
{
	return SaveDataListInsert2(Name, 1, ROW, COW);
}

int ReadDataListInsert2(char* Name, int type, int row, int cow)
{
	SaveDataList* L=SaveDataL;
	if (NULL!=FindSaveDataList(Name))
	{
//		Rdebug("%s is exist! Please rename!\n",Name);
		return -1;
	}
	SaveDataList* Lnext=(SaveDataList*)malloc(sizeof(SaveDataList));
	Lnext->next=L->next;
	L->next=Lnext;
	memset(Lnext->Name,'\0',sizeof(Lnext->Name));
	strcpy(Lnext->Name,Name);
	Lnext->index=L->index;
	L->index++;
	int err=_init_read_data(&(Lnext->saveData), Name, type,row , cow);
	if (0!=err)
	{
		printf("SaveDataListInsert failure\n");
		return err;
	}
    return 0;
}

int ReadDataListInsert(char* Name)
{
	return ReadDataListInsert2(Name, 1, ROW, COW);
}

int SaveDataListDelete(char* Name)
{
	SaveDataList* L=SaveDataL;
	SaveDataList* p=NULL;
	SaveDataList* q=NULL;
	p=L;
	q=L->next;
	while(NULL!=q)
	{
		if (0==strcmp(q->Name,Name))
		{
			p->next=q->next;
			_delete_save_data(&(q->saveData));
			free(q);
			return 0;
		}
		p=q;
		q=p->next;
	}
	printf("%s is not exit!\n",Name);
	return -1;
}

SaveData* FindSaveData(char* Name)
{
	SaveDataList* L=FindSaveDataList(Name);
	if (NULL==L)
	{
		printf("%s is not exist!\n",Name);
		return NULL;
	}
	return &(L->saveData);
}

SaveData* FindSaveData_index(int index)
{
	SaveDataList* L=FindSaveDataList_index(index);
	if (NULL==L)
	{
		printf("index %d is not exist!\n",index);
		return NULL;
	}
	return &(L->saveData);
}

int getSaveData_num()
{
	if (NULL==SaveDataL)
	{
		return 0;
	}
	return SaveDataL->index;
}

char* getSaveData_name(int index,char* name)
{
	SaveDataList* L=FindSaveDataList_index(index);
	if (NULL==L)
	{
		printf("index %d is not exist!\n",index);
		return NULL;
	}
	if (NULL==name)
	{
		return L->Name;
	}
	else
	{
		strcpy(name,L->Name);
		return name;
	}
}


