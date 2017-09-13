#ifndef STRUCT_H
#define STRUCT_H


#include <protocol.h>
#include <serial.h>
#include <commons/collections/list.h>


typedef struct {
	char nodo[64];                 //Nodo 1
	char ip[64];   //192.168.1.10:5000  aaa.ddd.c.dd:ppppp contando \0
	char puerto[64]; //Divido en dos al puerto y la ip, para tener mas facilidad en su uso
	char archivo_etapa[64]; /* /tmp/Master1-temp38 */
} tDatosEtapa, tAlmacenadoFinal;

typedef struct {
	char nodo[64];                 //Nodo 1
	char ip[64];   //192.168.1.10:5000  aaa.ddd.c.dd:ppppp contando \0
	char puerto[64]; //Divido en dos al puerto y la ip, para tener mas facilidad en su uso
	char archivo_etapa[64];
	int bloque;				  //38
	int bytes_ocupados;		  //10180
} tEtapaTransformacion;

typedef struct {
	char nodo[64];                 //Nodo 1
	char ip[64];   //192.168.1.10:5000  aaa.ddd.c.dd:ppppp contando \0
	char puerto[64]; //Divido en dos al puerto y la ip, para tener mas facilidad en su uso
	char archivo_etapa[64]; /* /tmp/Master1-temp38 */
	t_list * archivos_temporales_de_transformacion;
} tEtapaReduccionLocal;

typedef struct {
	char nodo[64];                 //Nodo 1
	char ip[64];   //192.168.1.10:5000  aaa.ddd.c.dd:ppppp contando \0
	char puerto[64]; //Divido en dos al puerto y la ip, para tener mas facilidad en su uso
	char archivo_etapa[64]; /* /tmp/Master1-temp38 */
	char archivo_temporal_de_rl[64]; // rl = reduccion_local
	bool * encargado;
} tEtapaReduccionGlobal;



#endif STRUCT_H
