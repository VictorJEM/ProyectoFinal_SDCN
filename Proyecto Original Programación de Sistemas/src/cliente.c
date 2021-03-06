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

double obtenerTiempoActual(){
	struct timespec tsp;
	clock_gettime( CLOCK_REALTIME, &tsp);
	double secs = (double)tsp.tv_sec;
	double nano = (double)tsp.tv_nsec / 1000000000.0;
	return secs + nano;
}

int main(int argc, char** argv){

	if(argc == 1){
		printf("Uso: ./cliente <interfaz> <numero de puerto> <solicitud>\n");
		exit(-1);
	}

	if(argc != 4){
		printf("¡Argumentos insuficientes o excesivos!\n");
		printf("Uso: ./cliente <interfaz> <numero de puerto> <solicitud>\n");
		exit(-1);
	}

	int puerto = atoi(argv[2]);

	int client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(client_fd < 0){
		perror("¡No se logró asignar el descriptor de socket cliente!\n");
		exit(-1);
	}

	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(puerto);
	client_addr.sin_addr.s_addr = inet_addr(argv[1]);

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

	t_inicial = obtenerTiempoActual();
	int ruta_enviada = write(client_fd, argv[3], 1000);
	if(ruta_enviada < 0){
		perror("¡Error al enviar la ruta!\n");
		exit(-1);
	}

	int size_recv = read(client_fd, buffer, 100);
	t_final = obtenerTiempoActual();
	if(size_recv < 0){
		perror("¡Error al recibir la respuesta del servidor!\n");
		exit(-1);
	}
	
	umask(0);
	int file_descriptor = open("../latenciasOriginal.txt", O_CREAT | O_RDWR | O_APPEND, 0666);
	char latencia[10]; 
	sprintf(latencia,"%f\n",t_final-t_inicial);
	write(file_descriptor,latencia,strlen(latencia));
	
	//printf("\n");
	printf("Recibido: %s\n",buffer);

	close(client_fd);
	close(file_descriptor);

	return 0;

}
