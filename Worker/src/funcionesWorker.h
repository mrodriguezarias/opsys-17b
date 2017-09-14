#ifndef FUNCIONESWORKER_H
#define FUNCIONESWORKER_H


#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <process.h>
#include <file.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <config.h>
#include <protocol.h>
#include <log.h>
#include <serial.h>

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <config.h>
#include <arpa/inet.h>

typedef struct {
	char * IP_FILESYSTEM, *PUERTO_FILESYSTEM, *NOMBRE_NODO, *PUERTO_WORKER,
			*PUERTO_DATANODE, *RUTA_DATABIN;

} tWorker;
int socketEscuchandoMaster;

void mostrar_configuracion();


void listen_to_master();
void creoHijos();

#endif
