#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "hashtable.h"
#include <semaphore.h>

unsigned long hash(unsigned char* str){

	unsigned long hash = 5381;
	int c;

	while((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;

}

//Función para crear un KVStore en caso de que no exista
kvStore* getKVPtr(kvStore** array, int size, char* kv_id, int numeroBuckets){

	if(array == NULL){
		printf("¡ERROR: DOBLE PUNTERO NULL!\n");
		exit(0);
	}
	if(size <= 0){
		printf("¡ERROR: TAMAÑO ARRAY INCORRECTO!\n");
		exit(0);
	}
	if(kv_id == NULL){
		printf("¡ERROR: ID NULL!\n");
		exit(0);
	}
	if(numeroBuckets <= 0){
		printf("¡ERROR: NÚMERO DE BUCKETS INCORRECTO!\n");
		exit(0);
	}

	kvStore* tabla = NULL;
	for(int i = 0 ; i < size ; i++){
		if( *(array + i) == NULL ){
			*(array + i) = crearKVStore(kv_id,numeroBuckets);
			tabla = *(array + i);
			return tabla;
		}
	}

	return tabla;

}

//Función para buscar KVStore que ya existen
kvStore* mapearHashtable(kvStore** array, int size, char* kv_id){

	if(array == NULL){
		printf("¡ERROR: DOBLE PUNTERO NULL!\n");
		exit(0);
	}
	if(size <= 0){
		printf("¡ERROR: TAMAÑO ARRAY INCORRECTO!\n");
		exit(0);
	}
	if(kv_id == NULL){
		printf("¡ERROR: ID NULL!\n");
		exit(0);
	}

	for(int i = 0 ; i < size ; i++){

		if( (*(array + i)) != NULL ){
			if( strcmp( (*(array + i))->id, kv_id ) == 0 ){
				return (*(array + i));
			}
		}
	}

	return NULL;

}

kvStore* crearKVStore(char* kv_id, int numeroBuckets){

	if(kv_id == NULL){
		printf("¡ERROR: ID NULL!\n");
		exit(0);
	}

	if(numeroBuckets <= 0){
		printf("¡El número de buckets pasado como parámetro es negativo o igual a cero!\n");
		exit(0);
	}

	kvStore* hashtable = (kvStore*)malloc( sizeof(kvStore) );
	if(hashtable == NULL){
		printf("¡No se logró asignar memoria para la hashtable!\n");
		exit(0);
	}

	hashtable->id = (char*)malloc(sizeof(char)*200);
	strcpy(hashtable->id,kv_id);
	hashtable->elementos = 0;
	hashtable->numeroBuckets = numeroBuckets;

	hashtable->buckets = (kvObjeto**)malloc( (hashtable->numeroBuckets)*sizeof(kvObjeto*) );
	if(hashtable->buckets == NULL){
		printf("¡No se logró asignar memoria para los buckets!\n");
	}
	for(int i = 0 ; i < (hashtable->numeroBuckets) ; i++){
		*((hashtable->buckets)+i) = NULL;
	}

	hashtable->semaforos = (sem_t**)malloc( (hashtable->numeroBuckets)*sizeof(sem_t*) );
	for(int j = 0 ; j < (hashtable->numeroBuckets) ; j++){
		*((hashtable->semaforos)+j) = (sem_t*)malloc(sizeof(sem_t));
		sem_init(*((hashtable->semaforos)+j),0,1);
	}

	return hashtable;

}

void put(kvStore* hashtable, char* clave, char* valor){

	if(hashtable == NULL){
		printf("¡Hashtable NULL!\n");
		exit(0);
	}

	if(clave == NULL){
		printf("¡Clave NULL!\n");
		exit(0);
	}

	if(valor == NULL){
		printf("¡Valor NULL!\n");
		exit(0);
	}
	
	unsigned long res = hash((unsigned char*)clave);
	
	int indice = abs((int)res)%(hashtable->numeroBuckets);

	sem_wait(*((hashtable->semaforos)+indice));
	if( *((hashtable->buckets)+indice) != NULL ){

		strcpy( (*((hashtable->buckets)+indice))->clave, clave );
		strcpy( (*((hashtable->buckets)+indice))->valor, valor );

	}else{

		*((hashtable->buckets)+indice) = (kvObjeto*)malloc( sizeof(kvObjeto) );
		(*((hashtable->buckets)+indice))->clave = (char*)malloc(sizeof(char)*400);
 		(*((hashtable->buckets)+indice))->valor = (char*)malloc(sizeof(char)*400);

		strcpy( (*((hashtable->buckets)+indice))->clave, clave );
		strcpy( (*((hashtable->buckets)+indice))->valor, valor );
		hashtable->elementos = (hashtable->elementos) + 1;

	}
	sem_post(*((hashtable->semaforos)+indice));
}

char* get(kvStore* hashtable, char* clave){

	if(hashtable == NULL){
		printf("¡Hashtable NULL!\n");
		exit(0);
	}

	if(clave == NULL){
		printf("¡Clave NULL!\n");
		exit(0);
	}

	for(int i = 0 ; i < (hashtable->numeroBuckets) ; i++){

		if( *((hashtable->buckets)+i) != NULL ){		
			if(strcmp( (*((hashtable->buckets)+i))->clave, clave ) == 0){
				return (*((hashtable->buckets)+i))->valor;
			}
		}
	}

	return NULL;
}

void remover(kvStore* hashtable, char* clave){

	if(hashtable == NULL){
		printf("¡Hashtable NULL!\n");
		exit(0);
	}
	
	if(clave == NULL){
		printf("¡Clave NULL!\n");
		exit(0);
	}

	for(int i = 0 ; i < (hashtable->numeroBuckets) ; i++){

		if( *((hashtable->buckets)+i) != NULL ){		
			if(strcmp( (*((hashtable->buckets)+i))->clave, clave ) == 0){
				free( *((hashtable->buckets)+i) );
				*((hashtable->buckets)+i) = NULL;
			}
		}

	}

}

void borrar(kvStore* hashtable){

	if(hashtable == NULL){
		printf("¡Hashtable NULL!\n");
		exit(0);
	}

	free(hashtable->id);

	for(int i = 0 ; i < (hashtable->numeroBuckets) ; i++){

		if( *((hashtable->buckets)+i) != NULL ){
			free( *((hashtable->buckets)+i) );
			*((hashtable->buckets)+i) = NULL;
		}

	}

	free( hashtable->buckets );
	hashtable->buckets = NULL;
	free( hashtable );
	hashtable = NULL;

}
