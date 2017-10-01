#ifndef MANEJADORES_H
#define MANEJADORES_H

#include "funcionesMaster.h"
#include <protocol.h>
#include <commons/log.h>


pthread_t hilo_transformacion;
pthread_t hilo_rl;
pthread_t hilo_rg;

void manejador_yama(t_packet,char*,char*,char* );

void manejador_worker();




#endif /* MANEJADORES_H */
