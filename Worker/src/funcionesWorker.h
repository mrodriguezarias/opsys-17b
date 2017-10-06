#ifndef FUNCIONESWORKER_H
#define FUNCIONESWORKER_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>

#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <config.h>
#include <data.h>
#include <file.h>
#include <log.h>
#include <mstring.h>
#include <process.h>
#include <protocol.h>
#include <serial.h>
#include <socket.h>
#include <struct.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>

#define MAX_IP_LEN 16   // aaa.bbb.ccc.ddd -> son 15 caracteres, 16 contando un '\0'
#define MAX_PORT_LEN 6  // 65535 -> 5 digitos, 6 contando un '\0'
#define MAX_NOMBRE_NODO 5
#define MAX_RUTA 25
#define MAXIMO_TAMANIO_DATOS 256 //definiendo el tamanio maximo
#define MAXCONEXIONESLISTEN 10

#define SIZE 1024

#define LECTURA_HIJO pipe_hijoAPadre[0]
#define ESCRITURA_HIJO pipe_hijoAPadre[1]
#define LECTURA_PADRE pipe_padreAHijo[0]
#define ESCRITURA_PADRE pipe_padreAHijo[1]


typedef struct {
	char * IP_FILESYSTEM, *PUERTO_FILESYSTEM, *NOMBRE_NODO, *PUERTO_WORKER,
			*PUERTO_DATANODE, *RUTA_DATABIN;
} tWorker;



void listen_to_master();

void manejador_fork(t_packet packet,int socket);
void manejador_fork2(t_packet packet, int socketFor);
#endif
