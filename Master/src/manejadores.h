#ifndef MANEJADORES_H
#define MANEJADORES_H

#include "funcionesMaster.h"
#include "connection.h"
#include <protocol.h>
#include <file.h>
#include <log.h>
#include <thread.h>
#include <protocol.h>
#include <pthread.h>
#include <serial.h>
#include <socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <struct.h>
#include <sys/wait.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include "funcionesMaster.h"
#include "Master.h"

enum {TRANSFORMACION, REPLANIFICACION, REDUCCION_LOCAL, REDUCCION_GLOBAL, ALMACENAMIENTO};

typedef struct{
	thread_t* hilo;
	int etapa;
	bool active;
} t_hilos;

struct{
	char* path_transf;
	char* path_reduc;
	char* arch;
	char* arch_result;
} job;

struct{
	t_file* fd_transf;
	char* script_transf;
	t_file* fd_reduc;
	char* script_reduc;
} script;

bool job_active;
mlist_t* hilos;
pthread_mutex_t mutex_hilos;

void manejador_yama(t_packet);

void manejador_worker();


#endif /* MANEJADORES_H */
