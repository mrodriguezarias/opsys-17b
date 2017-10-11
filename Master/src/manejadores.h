#ifndef MANEJADORES_H
#define MANEJADORES_H

#include "funcionesMaster.h"
#include "connection.h"
#include <protocol.h>
#include <commons/log.h>

struct{
	char* script_transf;
	char* script_reduc;
	char* arch;
	char* arch_result;
} job;

pthread_t hilo_transformacion;
pthread_t hilo_rl;
pthread_t hilo_rg;

void manejador_yama(t_packet);

void manejador_worker();




#endif /* MANEJADORES_H */
