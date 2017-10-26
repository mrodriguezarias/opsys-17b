#ifndef SERVER_H_
#define SERVER_H_
#include <yfile.h>
#include "struct.h"

void listen_to_master(void);
void requerirInformacionFilesystem(t_serial*);
void reciboInformacionSolicitada(mlist_t*,t_yfile*,int);
void envioMasterErrorArchivo(int);
void enviarEtapa_transformacion_Master(int,t_workerPlanificacion[],mlist_t*,mlist_t*,int);
char* generarNombreArchivoTemporal(int, int);
void actualizoTablaEstado(char*,int,char*);
bool verificoFinalizacionTransformacion(char* nodo,int bloque);
void mandarEtapaTransformacionLocal(int,char*,t_infoNodo*,mlist_t*,char*);
t_infoNodo* BuscoIP_PUERTO(char*);
mlist_t* BuscoArchivosTemporales(char*,int);
char* generarNombreTemporal_local(char*,int);

#endif /* SERVER_H_ */
