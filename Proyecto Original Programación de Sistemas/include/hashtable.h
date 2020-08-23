/*#include <stdio.h>
#include <string.h>
#include <stdlib.h>*/
#include <semaphore.h>

//Estructuras
typedef struct kvTDA{
	char* clave;
	char* valor;
} kvObjeto;

typedef struct kvStoreTDA{
	char* id;
	int elementos;
	int numeroBuckets;
	kvObjeto** buckets;
	sem_t** semaforos;
} kvStore;

//Funciones
kvStore* getKVPtr(kvStore** array, int size, char* kv_id, int numeroBuckets);

kvStore* mapearHashtable(kvStore** array, int size, char* kv_id);

kvStore* crearKVStore(char* kv_id, int numeroBuckets);

void put(kvStore* hashtable, char* clave, char* valor);

char* get(kvStore* hashtable, char* clave);

void remover(kvStore* hashtable, char* clave);

void borrar(kvStore* hashtable);
