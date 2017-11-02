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
#include <unistd.h>





void planificar(t_workerPlanificacion[], int, mlist_t*);
void mostrar_configuracion();
void llenarArrayPlanificador(t_workerPlanificacion[],int,int *);
int Disponibilidad(int, char*);
void verificarCondicion(int, int *,t_workerPlanificacion[],int*,mlist_t*);
respuestaOperacionTranf* serial_unpackRespuestaOperacion(t_serial *);
respuestaOperacion* serial_unpackrespuestaOperacion(t_serial *);
bool NodoConCopia_is_active(char*);
void destruirlista(void*);
void abortarJob(int, int, int);
void abortarJobEnTablaEstado(int,int);
int obtenerPosicionCargaNodo(char *);
void actualizarCargaDelNodo(char*, int, int, int, int);
int existeElJobEnLaCopia(int, mlist_t *);
bool nodoEstaEnLaCopia(t_block*, int, char*);
void generarEtapaTransformacionAEnviarParaCopia(int , t_block* , int , int , mlist_t* );
int reciboInformacionSolicitada(int , mlist_t* ,t_yfile* ,int );
void eliminarCargaJobDelNodo(int , mlist_t *);
int cargaActual(char*);

#endif
