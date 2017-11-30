#ifndef YAMA_H_
#define YAMA_H_

#include <socket.h>
#include <mlist.h>
#include <semaphore.h>

typedef struct {
	t_socket fs_socket;
} t_yama;

extern t_yama yama;
mlist_t * listaEstados;
extern int numeroJob;
extern mlist_t* listaNodosActivos;
extern int retardoPlanificacion;
extern char* algoritmoBalanceo;
//extern pthread_mutex_t mutexPlanificacion;
extern sem_t semPlanificacion;


void inicializoVariablesGlobalesConfig();

#endif /* YAMA_H_ */
