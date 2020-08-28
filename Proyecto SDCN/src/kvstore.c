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

kvStore* partitionHashTable = NULL;
int numBuckets;
int control = 0;
int id_int;

void signalHandler(int sig){

	if(control == 1){
		system("clear");
		printf("******* Servidor kvStore *******\n");
		printf("\nPuede usar el comando ""Ctrl + C"" para visualizar los buckets de la partición...\n");
	}

	//Nuevo
	//La HashTable ya se está creada y el usuario solo añade, obtiene valores a ella
	if(partitionHashTable != NULL){
		printf("\n======== Partición %s ========\n", partitionHashTable->id );
		//La variable i se puede igualar a una global para especificar la partición, pero puede haber un problema al acceder a la clave y el valor
		for(int i = 0 ; i < partitionHashTable->numeroBuckets ; i++){
			if( (*((partitionHashTable->buckets)+i))==NULL ){	
				printf("Bucket %d: Vacío\n",id_int*(partitionHashTable->numeroBuckets)+i);
			}else{

				kvObjeto* inicio = *((partitionHashTable->buckets)+i);
				kvObjeto* auxiliar = NULL;
				auxiliar = inicio;
				int bandera = 0;

				while(auxiliar != NULL){

					if(bandera == 0){
						printf("Bucket %d: clave-> %s ; valor-> %s\n",id_int*(partitionHashTable->numeroBuckets)+i,auxiliar->clave,auxiliar->valor);
						bandera = 1;
					}else{
						printf("\t  clave-> %s ; valor-> %s\n",auxiliar->clave,auxiliar->valor);
					}
					auxiliar = auxiliar->siguiente;

				}

			}
		}
		control = 1;

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
	char* clave;
	char* valor;
	char* obtenido;
	char respuesta[100] = {0};
	int enviado;

	if( strcmp(funcion,"PUT") == 0 ){
		clave = strtok_r(NULL,",",&sol);
		valor = strtok_r(NULL,"\0",&sol);

		if(clave==NULL || valor==NULL){
			//VALIDAR SOLICITUD INCOMPLETA
			snprintf(respuesta, 100, "ERROR, la solicitud no está completa!!!\nUso: PUT,clave,valor");
			enviado = write(server_fd_conectado, respuesta, 100);
			if(enviado < 0){
				perror("¡Error al enviar la respuesta al cliente!\n");
				exit(-1);
			}
		}else{
			put(partitionHashTable, clave, valor);
			snprintf(respuesta, 100, "OK, bucket insertado!!!");
			enviado = write(server_fd_conectado, respuesta, 100);
			if(enviado < 0){
				perror("¡Error al enviar la respuesta al cliente!\n");
				exit(-1);
			}
		}

	}else if( strcmp(funcion,"GET") == 0 ){
		clave = strtok_r(NULL,",",&sol);

		if( clave==NULL ){
			//VALIDAR SOLICITUD INCOMPLETA
			snprintf(respuesta, 100, "ERROR, la solicitud no está completa!!!\nUso: GET,clave");
			enviado = write(server_fd_conectado, respuesta, 100);
			if(enviado < 0){
				perror("¡Error al enviar la respuesta al cliente!\n");
				exit(-1);
			}
		}else{
			obtenido = get(partitionHashTable, clave);
			if(obtenido == NULL){
				snprintf(respuesta, 100, "ERROR, la clave NO existe!!!");
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

	}else if( strcmp(funcion,"REMOVE") == 0 ){
		clave = strtok_r(NULL,",",&sol);

		if( clave==NULL ){
			//VALIDAR SOLICITUD INCOMPLETA
			snprintf(respuesta, 100, "ERROR, la solicitud no está completa!!!\nUso: REMOVE,clave");
			enviado = write(server_fd_conectado, respuesta, 100);
			if(enviado < 0){
				perror("¡Error al enviar la respuesta al cliente!\n");
				exit(-1);
			}
		}else{
			//Remueve bucket del kvStore
			int removido = remover(partitionHashTable, clave);
			
			if(removido == 1){
				snprintf(respuesta, 100, "OK, objeto removido!!!");
				enviado = write(server_fd_conectado, respuesta, 100);
				if(enviado < 0){
					perror("¡Error al enviar la respuesta al cliente!\n");
					exit(-1);
				}
			}else if(removido == 0){
				snprintf(respuesta, 100, "ERROR, clave no existe!!!");
				enviado = write(server_fd_conectado, respuesta, 100);
				if(enviado < 0){
					perror("¡Error al enviar la respuesta al cliente!\n");
					exit(-1);
				}
			}
			
		}

	}else if( strcmp(funcion,"DELETE") == 0 ){
		
		//Vacia la partición del kvStore
		borrar(partitionHashTable);
		snprintf(respuesta, 100, "OK, partición del kvStore vaciado!!");
		enviado = write(server_fd_conectado, respuesta, 100);
		if(enviado < 0){
			perror("¡Error al enviar la respuesta al cliente!\n");
			exit(-1);
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
		printf("Uso: ./kvstore <interfaz> <numero de puerto> <numero de buckets> <número de partición>\n");
		exit(-1);
	}

	if(argc != 5){
		printf("¡Parámetros excesivos o insuficientes!\n");
		printf("Uso: ./kvstore <interfaz> <numero de puerto> <numero de buckets> <número de partición>\n");
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

	//Validar ingreso de número de partición
	int cant_3 = esNumero(argv[4]);
	if(cant_3 < 0){
		printf("¡Número de partición inválido!\n");
		exit(-1);
	}
	id_int = atoi(argv[4]);

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

	//Crea una partición de la hashtable
	partitionHashTable = crearKVStore(numBuckets, argv[4]);

	signal(SIGINT,signalHandler);
	printf("***** Servidor kvStore *****\n");
	printf("\nPuede usar el comando ""Ctrl + C"" para visualizar los buckets de la partición...\n");
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