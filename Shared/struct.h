#ifndef STRUCT_H
#define STRUCT_H

#include <stdio.h>
#include <stdlib.h>
#include <protocol.h>
#include <serial.h>
#include <string.h>
#include <mlist.h>
#include <commons/collections/list.h>
#include <commons/string.h>

#define SI "Si"
#define NO "No"


typedef struct {
	char* nodo;                 //Nodo 1
	char* ip;   //192.168.1.10:5000  aaa.ddd.c.dd:ppppp contando \0
	char* puerto; //Divido en dos al puerto y la ip, para tener mas facilidad en su uso
	char* archivo_etapa; /* /tmp/Master1-temp38 */
} tDatosEtapa, tAlmacenadoFinal;

typedef struct {
	char *nodo;                 //Nodo 1
	char *ip;   //192.168.1.10:5000  aaa.ddd.c.dd:ppppp contando \0
	char *puerto; //Divido en dos al puerto y la ip, para tener mas facilidad en su uso
	int bloque;				  //38
	int bytes_ocupados;		  //10180
	char *archivo_etapa;
} tEtapaTransformacion;

typedef struct{
	tEtapaTransformacion et;
	char * archivo_trans;
	char * srcipt_tranformador;
}tEtapaTransformacionBis;

typedef struct {
	char* nodo;                 //Nodo 1
	char* ip;   //192.168.1.10:5000  aaa.ddd.c.dd:ppppp contando \0
	char* puerto; //Divido en dos al puerto y la ip, para tener mas facilidad en su uso
	mlist_t* archivos_temporales_de_transformacion;
	//char * archivos_temporales; //Este dato va a almacenar los archivos temporales de transformacion, no encontre una manera para serializar y deserializar la lista
	char* archivo_etapa; /* /tmp/Master1-temp38 */
} tEtapaReduccionLocal;

typedef struct {
	char* nodo;                 //Nodo 1
	char* ip;   //192.168.1.10:5000  aaa.ddd.c.dd:ppppp contando \0
	char* puerto; //Divido en dos al puerto y la ip, para tener mas facilidad en su uso
	char* archivo_temporal_de_rl; // rl = reduccion_local
	char* archivo_etapa; /* /tmp/Master1-temp38 */
	char* encargado;
} tEtapaReduccionGlobal;


tEtapaTransformacion *new_etapa_transformacion(const char*nodo,const char*ip,const char*puerto,int bloque,int bytes_ocuapdos,const char * archivo_etapa);
t_serial *etapa_transformacion_pack(tEtapaTransformacion* transformacion);
t_serial* list_transformacion_pack(mlist_t* list);
tEtapaTransformacion* etapa_transformacion_unpack(t_serial *serial);
mlist_t* list_transformacion_unpack(t_serial* serial);
void mandar_etapa_transformacion(mlist_t* list,t_socket sock);


tEtapaReduccionLocal* new_etapa_rl(const char*nodo,const char*ip,const char*puerto,mlist_t*archivos_temporales,const char * archivo_etapa);
t_serial *etapa_rl_pack(tEtapaReduccionLocal*);
tEtapaReduccionLocal* etapa_rl_unpack(t_serial*);
void mandar_etapa_rl(tEtapaReduccionLocal*,t_socket);
/* Como no pude serializar y deserializar una lista, decidi meter en un string todos los directorios que se encuentra en etapa_temporal y de ahí ver si esta
 * el archivo que queremos
 * @param1: cadena con los directorios de todos los archivos (se obtiene de etapa_rl_unpack
 * @param2: cadena con el directorio del archivo deseado
 * */
char * obtener_archivo_temporal(char * archivos, char * archivo_a_obtener);


tEtapaReduccionGlobal* new_etapa_rg(const char*nodo,const char*ip,const char*puerto,char*archivo_temporal_rl,const char * archivo_etapa,char * encargado);
t_serial *etapa_rg_pack(tEtapaReduccionGlobal*);
t_serial* list_reduccionGlobal_pack(mlist_t*);
tEtapaReduccionGlobal* etapa_rg_unpack(t_serial *serial);
mlist_t* list_reduccionGlobal_unpack(t_serial*);
void mandar_etapa_rg(mlist_t*,t_socket sock);

tAlmacenadoFinal* new_etapa_af(const char*, const char*, const char*, const char*);
t_serial *etapa_af_pack(tAlmacenadoFinal*);
tAlmacenadoFinal* etapa_af_unpack(t_serial*);
void mandar_etapa_af(tAlmacenadoFinal*,t_socket);

#endif /* STRUCT_H */
