#ifndef FUNCIONES_YAMA_H_
#define FUNCIONES_YAMA_H_


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <log.h>
#include <protocol.h>
#include <serial.h>
#include <socket.h>

#include <commons/collections/list.h>


typedef struct{
	char * nodo;                 //Nodo 1
	char * ip;   //192.168.1.10:5000  aaa.ddd.c.dd:ppppp contando \0
	char * puerto;//Divido en dos al puerto y la ip, para tener mas facilidad en su uso
	char * archivo_etapa;        /* /tmp/Master1-temp38 */
}tDatosEtapa,tAlmacenadoFinal;

typedef struct{
	tDatosEtapa  etapa_transformacion;
	int bloque;				  //38
	int bytes_ocupados;		  //10180
}tEtapaTransformacion;

typedef struct{
	tDatosEtapa  etapa_reduccion_local;
    t_list * archivos_temporales_de_transformacion;
}tEtapaReduccionLocal;

typedef struct{
	tDatosEtapa  etapa_reduccion_global;
    char * archivo_temporal_de_rl; // rl = reduccion_local
    bool * encargado;
}tEtapaReduccionGlobal;


void planificar(char * script);







#endif
