/*
 * struct.h
 *
 *  Created on: 16/10/2017
 *      Author: utnso
 */

#ifndef STRUCT_H_
#define STRUCT_H_
#include <stdint.h>
#include <mlist.h>
#include <serial.h>
#include <stdlib.h>

typedef struct{
	int job;
	int master;
	char* nodo;
	int block;
	char* etapa;
	char* archivoTemporal;
	char* estado;

}t_Estado;

typedef struct{
	uint32_t disponibilidad;
	char*  nombreWorker;
	mlist_t* bloque;
}t_workerPlanificacion;

typedef struct{
	char* nodo;
	char* ip;
	char* puerto;

}t_infoNodo;

mlist_t *nodelist_unpack(t_serial *);

typedef struct{
	int idJOB;
	char* nodo;
	int bloque;
	int response;
	char* file;
} respuestaOperacionTranf;

typedef struct{
	int idJOB;
	char* nodo;
	int response;
}respuestaOperacion;

typedef struct{
	char* nodo;
	char* archivoTemporal;
}t_nodotemporal;

typedef struct{
	int idJOB;
	char* file;
}t_pedidoTrans;

#endif /* STRUCT_H_ */
