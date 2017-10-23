#ifndef FUNCIONES_YAMA_H_
#define FUNCIONES_YAMA_H_


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <log.h>
#include <protocol.h>
#include <serial.h>
#include <socket.h>
#include <struct.h>
#include <config.h>
#include "struct.h"
#include <yfile.h>
#include <commons/collections/list.h>





void planificar();
void mostrar_configuracion();
void llenarArrayPlanificador(t_workerPlanificacion[],int,int *);
void verificarCondicion(int, int *,t_workerPlanificacion[],int *,t_yfile);
respuestaOperacion* serial_unpackRespuestaOperacion(t_serial *);



#endif
