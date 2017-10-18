/*
 * struct.h
 *
 *  Created on: 16/10/2017
 *      Author: utnso
 */

#ifndef STRUCT_H_
#define STRUCT_H_

typedef struct {
	int bloque;
	size_t bytesOcupados;
	t_block_copy copies[2];
} tinformacionArchivo;

typedef struct {
	char *node;
	int blockno;
} t_block_copy;


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
	int nombreWorker;
	mlist_t* bloque;
}t_workerPlanificacion;

typedef struct{
	char* nodo;
	int ip;
	int puerto;

}t_infoNodo;



#endif /* STRUCT_H_ */
