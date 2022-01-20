
#include "memoryOperateBase.h"

double** allocationMemoryDoulble(int r, int c)
{
	int i = 0;
	double** p;
	p = (double**)malloc(r*sizeof(double*));
	for (i = 0; i<r; i++)
	{
		p[i] = (double*)malloc(c*sizeof(double));
	}
	return p;
}

int** allocationMemoryInt(int r, int c)
{
	int i = 0;
	int** p;
	p = (int**)malloc(r*sizeof(int*));
	for (i = 0; i<r; i++)
	{
		p[i] = (int*)malloc(c*sizeof(int));
	}
	return p;
}

void* freeMemoryDoulble(double** p, int r)
{
	if (NULL==p)
	{
		return NULL;
	}
	int i = 0;
	for (i = 0; i<r; i++)
	{
		free(p[i]);
	}
	free(p);
	return NULL;
}
void* freeMemoryInt(int** p, int r)
{
	if (NULL==p)
	{
		return NULL;
	}
	int i = 0;
	for (i = 0; i<r; i++)
	{
		free(p[i]);
	}
	free(p);
	return NULL;
}

char** allocationMemoryString(int r, int c)
{
	int i = 0;
	char** p;
	p = (char**)malloc(r*sizeof(char*));
	for (i = 0; i<r; i++)
	{
		p[i] = (char*)malloc(c*sizeof(char));
	}
	return p;
}

void* freeMemoryString(char** p, int r)
{
	if (NULL==p)
	{
		return NULL;
	}
	int i = 0;
	for (i = 0; i<r; i++)
	{
		free(p[i]);
	}
	free(p);
	return NULL;
}


