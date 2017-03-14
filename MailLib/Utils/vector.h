#pragma once
#ifndef VECTOR_H__
#define VECTOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#define VECTOR_INITIAL_SIZE 1

typedef struct vector_
{
	void** data;
	int	   size;
	int    count;
} CVECTOR;

void  vector_init(CVECTOR*);
int	  vector_count(CVECTOR*);
void  vector_add(CVECTOR*, void*);
void  vector_set(CVECTOR*, int, void*);
void* vector_get(CVECTOR*, int);
void* vector_delete(CVECTOR*, int);
void  vector_free(CVECTOR*);

#ifdef __cplusplus
}
#endif

#endif
