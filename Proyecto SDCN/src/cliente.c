#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>




typedef struct pair{
	char* IP;
	char* puerto;
} network_pair;

unsigned long hash(unsigned char* str){
	unsigned long hash = 5381;
	int c;
	while((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

double obtenerTiempoActual(){
	struct timespec tsp;
	clock_gettime( CLOCK_REALTIME, &tsp);
	double secs = (double)tsp.tv_sec;
	double nano = (double)tsp.tv_nsec / 1000000000.0;
	return secs + nano;
}


int main(int argc, char** argv){

	if(argc == 1){
		printf("Uso: ./cliente <solicitud>\n");
		exit(-1);
	}

	if(argc != 2){
		printf("¡Argumentos insuficientes o excesivos!\n");
		printf("Uso: ./cliente <solicitud>\n");
		exit(-1);
	}

	network_pair* lista = (network_pair*)malloc( sizeof(network_pair)*4 );

	lista[0].IP = "127.0.0.1";
	lista[0].puerto = "50000";

	lista[1].IP = "127.0.0.2";
	lista[1].puerto = "50000";

	lista[2].IP = "127.0.0.3";
	lista[2].puerto = "50000";
	
	lista[3].IP = "127.0.0.4";
	lista[3].puerto = "50000";

	char* req = (char*)malloc(sizeof(char)*100);
	strcpy(req,argv[1]);
	int S;

	if( strcmp(req,"DELETE") == 0 ){
		srand(time(NULL));
		S = rand()%4;
	}else{
		char* func = strtok_r(req,",",&req);
		char* key;
		char* value;

		if(strcmp(func,"PUT") == 0){
			key = strtok_r(NULL,",",&req);
			value = strtok_r(NULL,"\0",&req);
			if(key != NULL){
				if(value != NULL){
					S = hash((unsigned char*)key)%4;
				}else{
					printf("PUT: Falta valor, ejecute otra vez!!!\n");
					exit(-1);
				}
			}else{
				printf("PUT: Faltan clave y valor, ejecute otra vez!!!\n");
				exit(-1);
			}
		}else if(strcmp(func,"GET") == 0){
			key = strtok_r(NULL,"\0",&req);
			if(key != NULL){
				S = hash((unsigned char*)key)%4;
			}else{
				printf("GET: Falta clave, ejecute otra vez!!!\n");
				exit(-1);
			}
		}else if(strcmp(func,"REMOVE") == 0){
			key = strtok_r(NULL,"\0",&req);
			if(key != NULL){
				S = hash((unsigned char*)key)%4;
			}else{
				printf("REMOVE: Falta clave, ejecute otra vez!!!\n");
				exit(-1);
			}
		}else{
			printf("Solicitud desconocida!!!\n");
			exit(-1);
		}


	}

	int puerto = atoi(lista[S].puerto);

	int client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(client_fd < 0){
		perror("¡No se logró asignar el descriptor de socket cliente!\n");
		exit(-1);
	}

	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(puerto);
	client_addr.sin_addr.s_addr = inet_addr(lista[S].IP);

	int conectar = connect(client_fd, (struct sockaddr *)&client_addr, sizeof(client_addr));
	if(conectar<0){
		close(client_fd);
		perror("¡Error en connect!\n");
		exit(-1);
	}
	//CONEXIÓN ESTABLECIDA

	double t_inicial;
	double t_final;
	char buffer[100] = {0};		

	//Envía requerimiento
	t_inicial = obtenerTiempoActual();
	int ruta_enviada = write(client_fd, argv[1], 1000);
	if(ruta_enviada < 0){
		perror("¡Error al enviar la ruta!\n");
		exit(-1);
	}

	//Espera y recibe respuesta
	int size_recv = read(client_fd, buffer, 100);
	t_final = obtenerTiempoActual();
	if(size_recv < 0){
		perror("¡Error al recibir la respuesta del servidor!\n");
		exit(-1);
	}
	
	umask(0);
	int file_descriptor = open("../latencias.txt", O_CREAT | O_RDWR | O_APPEND, 0666);
	char latencia[10]; 
	sprintf(latencia,"%f\n",t_final-t_inicial);
	write(file_descriptor,latencia,strlen(latencia));
	//printf("Latencia: %.6f\n",t_final-t_inicial);
	printf("Recibido: %s\n",buffer);

	close(client_fd);
	close(file_descriptor);

	free(lista);

	return 0;

}
