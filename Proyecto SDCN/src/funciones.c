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

kvStore* crearKVStore(int numeroBuckets, char* kv_id){

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

	hashtable->id = (char*)malloc(sizeof(char)*50);
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

		kvObjeto* auxiliar;
		auxiliar = (*((hashtable->buckets)+indice));

		if(strcmp( auxiliar->clave, clave ) == 0){
			strcpy( auxiliar->valor, valor );
			//OJO: hay que hacer post al semáforo antes de retornar
			sem_post(*((hashtable->semaforos)+indice));
			return;
		}

		while( (auxiliar->siguiente) != NULL ){
			auxiliar = auxiliar->siguiente;
			if(strcmp( auxiliar->clave, clave ) == 0){
				strcpy( auxiliar->valor, valor );
				sem_post(*((hashtable->semaforos)+indice));
				return;
			}
		}

		kvObjeto* nuevo = (kvObjeto*)malloc( sizeof(kvObjeto) );
		nuevo->clave = (char*)malloc(sizeof(char)*400);
		nuevo->valor = (char*)malloc(sizeof(char)*400);
		nuevo->siguiente = NULL;
		strcpy( nuevo->clave, clave );
		strcpy( nuevo->valor, valor );
		hashtable->elementos = (hashtable->elementos) + 1;

		auxiliar->siguiente = nuevo;

	}else{

		//Bucket NULL, crea el objeto y llena el bucket
		*((hashtable->buckets)+indice) = (kvObjeto*)malloc( sizeof(kvObjeto) );
		(*((hashtable->buckets)+indice))->clave = (char*)malloc(sizeof(char)*400);
 		(*((hashtable->buckets)+indice))->valor = (char*)malloc(sizeof(char)*400);
		(*((hashtable->buckets)+indice))->siguiente = NULL;

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

	unsigned long result = hash((unsigned char*)clave);
	int index = abs((int)result)%(hashtable->numeroBuckets);

	sem_wait(*((hashtable->semaforos)+index));

	if( *((hashtable->buckets)+index) != NULL ){

		kvObjeto* auxiliar;
		auxiliar = (*((hashtable->buckets)+index));

		if(strcmp( auxiliar->clave, clave ) == 0){
			sem_post(*((hashtable->semaforos)+index));
			return auxiliar->valor;
		}

		while( (auxiliar->siguiente) != NULL ){
			auxiliar = auxiliar->siguiente;
			if(strcmp( auxiliar->clave, clave ) == 0){
				sem_post(*((hashtable->semaforos)+index));
				return auxiliar->valor;
			}
		}
		sem_post(*((hashtable->semaforos)+index));
		return NULL;

	}else{

		sem_post(*((hashtable->semaforos)+index));
		return NULL;

	}

}

int remover(kvStore* hashtable, char* clave){

	if(hashtable == NULL){
		printf("¡Hashtable NULL!\n");
		exit(0);
	}
	
	if(clave == NULL){
		printf("¡Clave NULL!\n");
		exit(0);
	}

	unsigned long result = hash((unsigned char*)clave);
	int index = abs((int)result)%(hashtable->numeroBuckets);

	sem_wait(*((hashtable->semaforos)+index));

	if( *((hashtable->buckets)+index) != NULL ){

		kvObjeto* auxiliar_1;
		kvObjeto* auxiliar_2;
		auxiliar_1 = (*((hashtable->buckets)+index));

		//Comprobar que el objeto a eliminar está primero.

		if(strcmp( auxiliar_1->clave, clave ) == 0){
			
			if( (auxiliar_1->siguiente) != NULL ){

				(*((hashtable->buckets)+index)) = auxiliar_1->siguiente;
				free( auxiliar_1->clave );
				free( auxiliar_1->valor );
				auxiliar_1->siguiente = NULL;
				free( auxiliar_1 );

			}else{

				(*((hashtable->buckets)+index)) = NULL;
				free( auxiliar_1->clave );
				free( auxiliar_1->valor );
				free( auxiliar_1 );

			}
			sem_post(*((hashtable->semaforos)+index));
			return 1;

		}else{

			if( (auxiliar_1->siguiente) == NULL ){
				sem_post(*((hashtable->semaforos)+index));
				return 0;
			}

		}

		//Que llegue aquí quiere decir que por lo menos hay 2 objetos.
		auxiliar_2 = auxiliar_1->siguiente;

		while( (auxiliar_2->siguiente) != NULL ){

			if(strcmp( auxiliar_2->clave, clave ) == 0){
				auxiliar_1->siguiente = auxiliar_2->siguiente;
				free( auxiliar_2->clave );
				free( auxiliar_2->valor );
				auxiliar_2->siguiente = NULL;
				free( auxiliar_2 );
				sem_post(*((hashtable->semaforos)+index));
				return 1;
			}

			auxiliar_1 = auxiliar_2;
			auxiliar_2 = auxiliar_2->siguiente;

		}

		//Si llega aquí significa que buscó hasta el último objeto y falta comprobarlo
		if(strcmp( auxiliar_2->clave, clave ) == 0){
			auxiliar_1->siguiente = NULL;
			free( auxiliar_2->clave );
			free( auxiliar_2->valor );
			auxiliar_2->siguiente = NULL;
			free( auxiliar_2 );
			sem_post(*((hashtable->semaforos)+index));
			return 1;
		}else{
			sem_post(*((hashtable->semaforos)+index));
			return 0;
		}

	}else{
		//Bucket vacío en ese índice, nada que remover
		return 0;

	}

}

//Vacía por completo la partición del kvStore
void borrar(kvStore* hashtable){

	if(hashtable == NULL){
		printf("¡Hashtable NULL!\n");
		exit(0);
	}
	
	for(int i = 0 ; i < (hashtable->numeroBuckets) ; i++){

		if( *((hashtable->buckets)+i) != NULL ){

			sem_wait(*((hashtable->semaforos)+i));

			kvObjeto* auxiliar_1;
			auxiliar_1 = (*((hashtable->buckets)+i));
			
			while( (auxiliar_1->siguiente) != NULL ){

				(*((hashtable->buckets)+i)) = auxiliar_1->siguiente;
				free( auxiliar_1->clave );
				free( auxiliar_1->valor );
				auxiliar_1->siguiente = NULL;
				free( auxiliar_1 );

			}
			
			free( (*((hashtable->buckets)+i))->clave );
			free( (*((hashtable->buckets)+i))->valor );
			free( (*((hashtable->buckets)+i)) );
			(*((hashtable->buckets)+i)) = NULL;
			
			sem_post(*((hashtable->semaforos)+i));

		}

	}

}