/***********************************************************************************/
/*																				  * /
/* Vektor-Klasse in C                                                             * /
/*                                                                                * /
/* Quelle:                                                                        * /
/* https://gist.github.com/EmilHernvall/953968                                    * /
/*                                                                                * /
/***********************************************************************************/
#pragma once
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "vector.h"

// Intialisierung
void vector_init(CVECTOR* v)
{
	v->data = NULL;
	v->size = 0;
	v->count = 0;
}

// Groesse des Vektors
int vector_count(CVECTOR* v)
{
	return v->count;
}

// Hinzufuegen von Elementen
void vector_add(CVECTOR* v, void* e)
{
	if (v->size == 0)
	{
		v->size = VECTOR_INITIAL_SIZE;
		v->data = (void**)malloc(sizeof(void*) * v->size);
		memset(v->data, '\0', sizeof(void*) * v->size);
	}

	if (v->size == v->count)
	{
		v->size++;
		v->data = (void**)realloc(v->data, sizeof(void*) * v->size);
	}

	v->data[v->count] = (void*)e;
	v->count++;
}

// Setzen von Werten
void vector_set(CVECTOR* v, int index, void* e)
{
	if (index >= v->count)
	{
		return;
	}

	v->data[index] = e;
}

// Rueckgabe eines Pointers auf ein Element
void* vector_get(CVECTOR* v, int index)
{
	if (index >= v->count)
	{
		return NULL;
	}

	return v->data[index];
}

// Loeschen eines Elements
void* vector_delete(CVECTOR* v, int index)
{
	int j = index;
	int i;
	void* res;
	if (index >= v->count)
	{
		return NULL;
	}
	res = v->data[j];
	for (i = index + 1; i < v->count; ++i)
	{
		v->data[j] = v->data[i];
		++j;
	}
	v->count--;
	return res;
}

// Freigabe des Speichers
void vector_free(CVECTOR* v)
{
	free(v->data);
	v->data = NULL;
}


// Tests
//int main(void)
//{
//	CVECTOR v;
//	vector_init(&v);
//
//	vector_add(&v, "emil");
//	vector_add(&v, "hannes");
//	vector_add(&v, "lydia");
//	vector_add(&v, "olle");
//	vector_add(&v, "erik");
//
//	int i;
//	printf("first round:\n");
//	for (i = 0; i < vector_count(&v); i++)
//  {
//		printf("%s\n", (char*)vector_get(&v, i));
//	}
//
//	vector_delete(&v, 1);
//	vector_delete(&v, 3);
//
//	printf("second round:\n");
//	for (i = 0; i < vector_count(&v); i++)
//  {
//		printf("%s\n", (char*)vector_get(&v, i));
//	}
//
//	vector_free(&v);
//
//	return 0;
//}