#ifndef STRUCT_H
#define STRUCT_H

#include <stdio.h>
#include <stdlib.h>
#include <protocol.h>
#include <serial.h>
#include <string.h>
#include <commons/collections/list.h>

#define SI "Si"
#define NO "No"


typedef struct {
	char nodo[64];                 //Nodo 1
	char ip[64];   //192.168.1.10:5000  aaa.ddd.c.dd:ppppp contando \0
	char puerto[64]; //Divido en dos al puerto y la ip, para tener mas facilidad en su uso
	char archivo_etapa[64]; /* /tmp/Master1-temp38 */
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
	char nodo[64];                 //Nodo 1
	char ip[64];   //192.168.1.10:5000  aaa.ddd.c.dd:ppppp contando \0
	char puerto[64]; //Divido en dos al puerto y la ip, para tener mas facilidad en su uso
	t_list * archivos_temporales_de_transformacion;
	//char * archivos_temporales; //Este dato va a almacenar los archivos temporales de transformacion, no encontre una manera para serializar y deserializar la lista
	char archivo_etapa[64]; /* /tmp/Master1-temp38 */
} tEtapaReduccionLocal;

typedef struct {
	char nodo[64];                 //Nodo 1
	char ip[64];   //192.168.1.10:5000  aaa.ddd.c.dd:ppppp contando \0
	char puerto[64]; //Divido en dos al puerto y la ip, para tener mas facilidad en su uso
	char archivo_temporal_de_rl[64]; // rl = reduccion_local
	char archivo_etapa[64]; /* /tmp/Master1-temp38 */
	char encargado[2];
} tEtapaReduccionGlobal;


tEtapaTransformacion new_etapa_transformacion(const char*nodo,const char*ip,const char*puerto,int bloque,int bytes_ocuapdos,const char * archivo_etapa);
t_serial etapa_transformacion_pack(tEtapaTransformacion transformacion);
tEtapaTransformacion etapa_transformacion_unpack(t_serial serial);
void mandar_etapa_transformacion(tEtapaTransformacion et,t_socket sock);


tEtapaReduccionLocal new_etapa_rl(const char*nodo,const char*ip,const char*puerto,t_list*archivos_temporales,const char * archivo_etapa);
t_serial etapa_rl_pack(tEtapaReduccionLocal);
tEtapaReduccionLocal etapa_rl_unpack(t_serial);
void mandar_etapa_rl(tEtapaReduccionLocal rl,t_socket sock);
/* Como no pude serializar y deserializar una lista, decidi meter en un string todos los directorios que se encuentra en etapa_temporal y de ahí ver si esta
 * el archivo que queremos
 * @param1: cadena con los directorios de todos los archivos (se obtiene de etapa_rl_unpack
 * @param2: cadena con el directorio del archivo deseado
 * */
char * obtener_archivo_temporal(char * archivos, char * archivo_a_obtener);


tEtapaReduccionGlobal new_etapa_rg(const char*nodo,const char*ip,const char*puerto,char*archivo_temporal_rl,const char * archivo_etapa,char * encargado);
t_serial etapa_rg_pack(tEtapaReduccionGlobal rg);
tEtapaReduccionGlobal etapa_rg_unpack(t_serial serial);
void mandar_etapa_rg(tEtapaReduccionGlobal rg,t_socket sock);
#endif /* STRUCT_H */
