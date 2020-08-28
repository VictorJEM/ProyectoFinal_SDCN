/*#include <stdio.h>
#include <string.h>
#include <stdlib.h>*/
#include <semaphore.h>

//Estructuras
typedef struct kvTDA{
	char* clave;
	char* valor;
	struct kvTDA* siguiente;
} kvObjeto;

typedef struct kvStoreTDA{
	char* id;
	int elementos;
	int numeroBuckets;
	kvObjeto** buckets;
	sem_t** semaforos;
} kvStore;

//Funciones
//kvStore* getKVPtr(kvStore** array, int size, char* kv_id, int numeroBuckets);

//kvStore* mapearHashtable(kvStore** array, int size, char* kv_id);

kvStore* crearKVStore(int numeroBuckets, char* kv_id);

void put(kvStore* hashtable, char* clave, char* valor);

char* get(kvStore* hashtable, char* clave);

int remover(kvStore* hashtable, char* clave);

void borrar(kvStore* hashtable);
