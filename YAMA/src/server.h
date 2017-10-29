#ifndef SERVER_H_
#define SERVER_H_
#include <yfile.h>
#include "struct.h"

void listen_to_master(void);
void requerirInformacionFilesystem(t_serial*);
void reciboInformacionSolicitada(t_yfile*,int);
void envioMasterErrorArchivo(int);
void enviarEtapa_transformacion_Master(int,t_workerPlanificacion[],mlist_t*,int);
char* generarNombreArchivoTemporalTransf(int, int);
void actualizoTablaEstado(char*,int,int,int,char*);
bool verificoFinalizacionTransformacion(char* nodo,int bloque,int job);
void mandarEtapaReduccionLocal(int,char*,t_infoNodo*,mlist_t*,char*);
t_infoNodo* BuscoIP_PUERTO(char*);
mlist_t* BuscoArchivosTemporales(char*,int,int);
char* generarNombreTemporal_local(char*,int);
void abortarJob(int);
bool verificoFinalizacionRl(char*,int);
void mandarEtapaReduccionGL(int,int);
mlist_t* BuscoNodos(int, int);
#endif /* SERVER_H_ */
