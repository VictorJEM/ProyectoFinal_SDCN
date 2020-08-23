#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "hashtable.h"
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#define COLA 1024

kvStore** arr;
int numBuckets;
sem_t semaforo;
int KVSIZE = 250;
int control = 0;

void signalHandler(int sig){

	int flag = 0;

	for(int i = 0 ; i < KVSIZE ; i++){

		if(arr[i] != NULL){

			printf("\n======== %s ========\n",(arr[i])->id);
			for(int j = 0 ; j < (arr[i])->numeroBuckets ; j++){
				if( (*(((arr[i])->buckets)+j))==NULL ){	
					printf("Bucket %d: Vacío\n",j);
				}else{
					printf("Bucket %d: clave-> %s ; valor-> %s\n",j,(*(((arr[i])->buckets)+j))->clave,(*(((arr[i])->buckets)+j))->valor);
				}
			}
			flag = flag + 1;
		}

	}
	if(flag == 0){
		printf("No hay kvStores creadas!!!\n");
	}

}

void* atenderSolicitud( void* vargp ){
	
	int server_fd_conectado = *((int*)vargp);

	char solicitud[1000] = {0};
	int sol_recib = read(server_fd_conectado, solicitud, 1000);
	if(sol_recib < 0){
		perror("¡Error al recibir la solicitud!\n");
		exit(-1);
	}
	char* sol = solicitud;
	char* funcion = strtok_r(sol,",",&sol);
	char* kv_id;
	char* clave;
	char* valor;
	kvStore* tabla;
	char* obtenido;
	char respuesta[100] = {0};
	int enviado;

	if( strcmp(funcion,"PUT") == 0 ){
		kv_id = strtok_r(NULL,",",&sol);
		clave = strtok_r(NULL,",",&sol);
		valor = strtok_r(NULL,"\0",&sol);

		if(kv_id==NULL || clave==NULL || valor==NULL){
			//VALIDAR SOLICITUD INCOMPLETA
			snprintf(respuesta, 100, "ERROR, la solicitud no está completa!!!\nUso: PUT,kv_id,clave,valor");
			enviado = write(server_fd_conectado, respuesta, 100);
			if(enviado < 0){
				perror("¡Error al enviar la respuesta al cliente!\n");
				exit(-1);
			}
		}else{

			tabla = mapearHashtable(arr, KVSIZE, kv_id);
			if(tabla == NULL){
				sem_wait(&semaforo);
				tabla = getKVPtr(arr, KVSIZE, kv_id, numBuckets);
				if(tabla == NULL){
					if(control == KVSIZE){
						KVSIZE = KVSIZE*2;
						arr = (kvStore**)realloc( arr, sizeof(kvStore*)*KVSIZE );
						if( arr == NULL ){
							KVSIZE = KVSIZE/2;
							snprintf(respuesta, 100, "ERROR, no se logró crear el kvStore!!!");
							enviado = write(server_fd_conectado, respuesta, 100);
							if(enviado < 0){
								perror("¡Error al enviar la respuesta al cliente!\n");
								exit(-1);
							}
						}else{
							tabla = getKVPtr(arr, KVSIZE, kv_id, numBuckets);
							put(tabla, clave, valor);
							control = control + 1;
							snprintf(respuesta, 100, "OK, bucket insertado!!!");
							enviado = write(server_fd_conectado, respuesta, 100);
							if(enviado < 0){
								perror("¡Error al enviar la respuesta al cliente!\n");
								exit(-1);
							}
						}
					}else{
						snprintf(respuesta, 100, "ERROR, no se logró crear el kvStore!!!");
						enviado = write(server_fd_conectado, respuesta, 100);
						if(enviado < 0){
							perror("¡Error al enviar la respuesta al cliente!\n");
							exit(-1);
						}
					}
				}else{
					put(tabla, clave, valor);
					control = control + 1;
					snprintf(respuesta, 100, "OK, bucket insertado!!!");
					enviado = write(server_fd_conectado, respuesta, 100);
					if(enviado < 0){
						perror("¡Error al enviar la respuesta al cliente!\n");
						exit(-1);
					}
				}
				sem_post(&semaforo);
			}else{
				put(tabla, clave, valor);
				snprintf(respuesta, 100, "OK, bucket insertado!!!");
				enviado = write(server_fd_conectado, respuesta, 100);
				if(enviado < 0){
					perror("¡Error al enviar la respuesta al cliente!\n");
					exit(-1);
				}
			}
		}

	}else if( strcmp(funcion,"GET") == 0 ){
		kv_id = strtok_r(NULL,",",&sol);
		clave = strtok_r(NULL,"\0",&sol);

		if(kv_id==NULL || clave==NULL){
			//VALIDAR SOLICITUD INCOMPLETA
			snprintf(respuesta, 100, "ERROR, la solicitud no está completa!!!\nUso: GET,kv_id,clave");
			enviado = write(server_fd_conectado, respuesta, 100);
			if(enviado < 0){
				perror("¡Error al enviar la respuesta al cliente!\n");
				exit(-1);
			}
		}else{
			tabla = mapearHashtable(arr, KVSIZE, kv_id);
			if(tabla == NULL){
				//Mostrar ERROR
				snprintf(respuesta, 100, "ERROR, el kvStore no existe!!!");
				enviado = write(server_fd_conectado, respuesta, 100);
				if(enviado < 0){
					perror("¡Error al enviar la respuesta al cliente!\n");
					exit(-1);
				}
			}else{
				obtenido = get(tabla, clave);
				if(obtenido == NULL){
					snprintf(respuesta, 100, "ERROR, el kvStore existe pero NO el bucket!!!");
					enviado = write(server_fd_conectado, respuesta, 100);
					if(enviado < 0){
						perror("¡Error al enviar la respuesta al cliente!\n");
						exit(-1);
					}
				}else{
					snprintf(respuesta, 100, "OK, %s", obtenido);
					enviado = write(server_fd_conectado, respuesta, 100);
					if(enviado < 0){
						perror("¡Error al enviar la respuesta al cliente!\n");
						exit(-1);
					}
				}
			}
		}

	}else if( strcmp(funcion,"REMOVE") == 0 ){
		kv_id = strtok_r(NULL,",",&sol);
		clave = strtok_r(NULL,"\0",&sol);

		if(kv_id==NULL || clave==NULL){
			//VALIDAR SOLICITUD INCOMPLETA
			snprintf(respuesta, 100, "ERROR, la solicitud no está completa!!!\nUso: REMOVE,kv_id,clave");
			enviado = write(server_fd_conectado, respuesta, 100);
			if(enviado < 0){
				perror("¡Error al enviar la respuesta al cliente!\n");
				exit(-1);
			}
		}else{
			tabla = mapearHashtable(arr, KVSIZE, kv_id);
			if(tabla == NULL){
				//Mostrar ERROR
				snprintf(respuesta, 100, "ERROR, el kvStore no existe!!!");
				enviado = write(server_fd_conectado, respuesta, 100);
				if(enviado < 0){
					perror("¡Error al enviar la respuesta al cliente!\n");
					exit(-1);
				}
			}else{
				//Remueve bucket del kvStore
				remover(tabla, clave);
				snprintf(respuesta, 100, "OK, bucket removido!!!");
				enviado = write(server_fd_conectado, respuesta, 100);
				if(enviado < 0){
					perror("¡Error al enviar la respuesta al cliente!\n");
					exit(-1);
				}
			}
		}

	}else if( strcmp(funcion,"DELETE") == 0 ){
		kv_id = strtok_r(NULL,",",&sol);

		if(kv_id == NULL){			
			//VALIDAR SOLICITUD INCOMPLETA
			snprintf(respuesta, 100, "ERROR, la solicitud no está completa!!!\nUso: DELETE,kv_id");
			enviado = write(server_fd_conectado, respuesta, 100);
			if(enviado < 0){
				perror("¡Error al enviar la respuesta al cliente!\n");
				exit(-1);
			}

		}else{
			tabla = mapearHashtable(arr, KVSIZE, kv_id);
			if(tabla == NULL){
				//Mostrar ERROR
				snprintf(respuesta, 100, "ERROR, el kvStore no existe!!!");
				enviado = write(server_fd_conectado, respuesta, 100);
				if(enviado < 0){
					perror("¡Error al enviar la respuesta al cliente!\n");
					exit(-1);
				}
			}else{
				//Borra kvStore
				borrar(tabla);
				for(int v = 0 ; v < KVSIZE ; v++){
					if( *(arr+v) == tabla ){											
						*(arr+v) = NULL;
						break;
					}
				}
				snprintf(respuesta, 100, "OK, kvStore eliminado!!!");
				enviado = write(server_fd_conectado, respuesta, 100);
				if(enviado < 0){
					perror("¡Error al enviar la respuesta al cliente!\n");
					exit(-1);
				}
			}
		}
		
	}else{
		snprintf(respuesta, 100, "ERROR, la solicitud enviada es desconocida!!!");
		enviado = write(server_fd_conectado, respuesta, 100);
		if(enviado < 0){
			perror("¡Error al enviar la respuesta al cliente!\n");
			exit(-1);
		}

	}

	close(server_fd_conectado);

	pthread_exit(NULL);

}

int esNumero(char* cadena){
	for( ; *cadena; ++cadena ){
		if( '0' > *cadena || '9' < *cadena ){
			return -1;
		}
	}
	return 0;
}

int main(int argc, char** argv){

	if(argc == 1){
		printf("Uso: ./kvstore <interfaz> <numero de puerto> <numero de buckets>\n");
		exit(-1);
	}

	if(argc != 4){
		printf("¡Parámetros excesivos o insuficientes!\n");
		printf("Uso: ./kvstore <interfaz> <numero de puerto> <numero de buckets>\n");
		exit(-1);
	}

	//Validar ingreso del número de puerto
	int cant = esNumero(argv[2]);
	if(cant < 0){
		printf("¡Número de puerto inválido!\n");
		exit(-1);
	}
	int puerto = atoi(argv[2]);
	if(puerto == 0){
		printf("¡Número de puerto inválido!\n");
		exit(-1);
	}

	//Validar ingreso del número de buckets
	int cant2 = esNumero(argv[3]);
	if(cant2 < 0){
		printf("¡Número de buckets inválido!\n");
		exit(-1);
	}
	numBuckets = atoi(argv[3]);
	if(numBuckets == 0){
		printf("¡Número de buckets inválido!\n");
		exit(-1);
	}

	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fd < 0){
		perror("¡No se pudo asignar el descriptor de socket!");
		exit(-1);
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(puerto);
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);

	int enlace = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(enlace < 0){
		perror("¡Error en bind!\n");
		exit(-1);
	}		

	int escuchar = listen(server_fd, COLA);
	if(escuchar < 0){
		perror("¡Error en listen!\n");
		exit(-1);
	}

	int* server_fd_conectado;

	arr = (kvStore**)malloc( sizeof(kvStore*)*KVSIZE );
	for(int x = 0 ; x < KVSIZE ; x++){
		*(arr + x) = NULL;
	}

	
	if(sem_init(&semaforo, 0, 1) != 0){
		perror("¡Error al iniciar el semaforo!\n");
		exit(-1);
	}

	signal(SIGINT,signalHandler);
	printf("***** Servidor kvStore *****\n");
	printf("\nPuede usar el comando ""Ctrl + C"" para visualizar las kvStores...\n");
	while(1){

		//CREACIÓN DE HILOS
		server_fd_conectado = (int*)malloc(sizeof(int));
		*server_fd_conectado = accept(server_fd, NULL, 0);
		//printf("Se conectó alguien!!!\n");

		pthread_t tid;
		pthread_create(&tid, NULL, atenderSolicitud, server_fd_conectado);
		pthread_detach(tid);
		
	}

	return 0;
}
