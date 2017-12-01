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
int obtenerPosicionCargaNodo(char *);
void actualizarCargaDelNodo(char*, int, int, int);
int existeElJobEnLaCopia(int, mlist_t *);
bool nodoEstaEnLaCopia(t_block*, int, char*);
void generarEtapaTransformacionAEnviarParaCopia(int , t_block* , int , int , mlist_t* );
t_yfile* reciboInformacionSolicitada(int,int);
void eliminarCargaJobDelNodo(int , mlist_t *);
int cargaActual(char*);
void replanificacion(char*, const char*,int,int);
t_pedidoTrans* serial_unpackPedido(t_serial*);
void eliminarEstadosMultiples(int,int, char*);
void finalizarJobGlobalEnTablaEstado(int,int, char*);
void finalizarJobGlobal(int, int, int, char*);
void eliminarCargasReduccionesLocales(char*,int,int);
int obtenerHistorico(char *);
void avanzoPosicion(int *,int,t_workerPlanificacion[]);
void asignoBloque(t_workerPlanificacion[], int *,int*,int);
void avisarErrorMaster(int, int , int );
int buscarIdJobParaMasterCaido(int);

#endif
