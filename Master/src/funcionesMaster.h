#ifndef FUNCIONESMASTER_H_
#define FUNCIONESMASTER_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <protocol.h>
#include <log.h>
#include <socket.h>
#include <serial.h>
#include <struct.h>
#include <config.h>

#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>






#include<pthread.h>

#define PORT "3790"
#define MAXSIZE 1024

#define MAX_IP_LEN 16   // aaa.bbb.ccc.ddd -> son 15 caracteres, 16 contando un '\0'
#define MAX_PORT_LEN 6  // 65535 -> 5 digitos, 6 contando un '\0'
#define MAX_RUTA 25
#define RUTA_CONFIG "config_master"

struct data {
	unsigned d;
	short h;
	double f;
	char s[16];
};


typedef struct {
	char * YAMA_IP, *YAMA_PUERTO,*WORKER_IP ,*PUERTO_WORKER;
} tMaster;


struct {
	t_socket yama_socket;
	t_socket worker_socket;
} master;







int socket_yama,socket_worker;
struct sockaddr_in direccion_master;
pthread_t * hilo_worker;
//pthread_t * hilo_yama;







void crearLogger();
tMaster * getConfigMaster();
void mostrar_configuracion();
void liberarConfiguracionMaster(tMaster*masterAux);
void iniciar_master(tMaster * masterAux);


//void connect_to_yama(tMaster * masterAux);
void connect_to_yama();
void connect_to_worker(const char * ip,const char*port);
void terminate(void);



void liberarRecursos();


#endif //MASTER_H
