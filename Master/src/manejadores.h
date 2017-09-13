#ifndef MANEJADORES_H
#define MANEJADORES_H

#include "funcionesMaster.h"
#include <protocol.h>
#include <commons/log.h>

//typedef enum{
//	INICIAR_TRANSFORMACION   =1,
//	INICIAR_REDUCCION_LOCAL  =2,
//	INICIAR_REDUCCION_GLOBAL =3,
//	INICIAR_ALMACENAMIENTO   =4,
//
//}tProtocolo;



void manejador_yama(t_packet );
void manejador_worker();




#endif MANEJADORES_H
