all: bin/cliente bin/kvstore

bin/cliente: obj/cliente.o
	gcc -Wall -g obj/cliente.o -o bin/cliente

bin/kvstore: obj/kvstore.o obj/funciones.o
	gcc -Wall -g -pthread -I include/ obj/kvstore.o obj/funciones.o -o bin/kvstore

obj/cliente.o: src/cliente.c
	gcc -Wall -c -g src/cliente.c -o obj/cliente.o

obj/kvstore.o: src/kvstore.c
	gcc -Wall -c -I include/ -g src/kvstore.c -o obj/kvstore.o

obj/funciones.o: src/funciones.c
	gcc -Wall -c -I include/ -g src/funciones.c -o obj/funciones.o

.PHONY: clean
clean:
	rm obj/* bin/*
