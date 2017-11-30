#ifndef SERVER_H_
#define SERVER_H_
#include <yfile.h>
#include "struct.h"

typedef struct{
	char * nodo;
	int cargaActual;
	int cargaHistorica;
	mlist_t * cargaPorJob;
} t_cargaPorNodo;

typedef struct{
	int job;
	int cargaDelJob;
} t_cargaPorJob;

extern mlist_t * listaCargaPorNodo;

void listen_to_master(void);
void requerirInformacionFilesystem(t_serial*);
void enviarEtapa_transformacion_Master(int,int,t_workerPlanificacion[],mlist_t*,int);
void agregarAtablaEstado(int, char*,int,int,char*,char*,char*);
char* generarNombreArchivoTemporalTransf(int,int, int);
void actualizoTablaEstado(char*,int,int,int,char*);
bool verificoFinalizacionTransformacion(char* nodo,int bloque,int job);
void mandarEtapaReduccionLocal(int,int,char*,t_infoNodo*,mlist_t*,char*);
t_infoNodo* BuscoIP_PUERTO(char*);
mlist_t* BuscoArchivosTemporales(char*,int,int);
char* generarNombreTemporal_local(char*,int,int);
bool verificoFinalizacionRl(int,int);
void mandarEtapaReduccionGL(int,int);
mlist_t* BuscoNodos(int, int);
char* seleccionarEncargado(mlist_t*, int);
char* generarArchivoRG(int, int);
void mandarEtapaAlmacenadoFinal(char*,int,int);
char* BuscoNodoEncargado(int);
void completarPrimeraVez();
void agregarCargaNodoSegunLoPlanificado(int , t_workerPlanificacion[], int );
void crearCargaPorNodo(char*);

#endif /* SERVER_H_ */
