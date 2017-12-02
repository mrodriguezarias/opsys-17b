#ifndef MASTER_H_
#define MASTER_H_

#include <socket.h>
#include <commons/string.h>
#include <file.h>
#include <mlist.h>
#include <thread.h>
#include <mtime.h>

enum {TRANSFORMACION, REDUCCION_LOCAL, REDUCCION_GLOBAL, ALMACENAMIENTO};

typedef struct{
	thread_t* hilo;
	char* nodo;
	int etapa;
	bool active;
	int result;
} t_hilos;

struct{
	char* path_transf;
	char* path_reduc;
	char* arch;
	char* arch_result;
} job;

struct{
	t_file* fd_transf;
	char* script_transf;
	t_file* fd_reduc;
	char* script_reduc;
} script;

bool job_active;
mlist_t* hilos;
pthread_mutex_t mutex_hilos;
sem_t* sem;
thread_t* hilo_node_drop;
extern int IDJOB;

struct{
	int transf;
	int reducc;
}tareasParalelo;

t_socket yama_socket;

struct{
	mtime_t job_init;
	mtime_t job_end;
	mtime_t transf_init;
	mtime_t transf_end;
	mtime_t rl_init;
	mtime_t rl_end;
	mtime_t rg_init;
	mtime_t rg_end;
}times;

#endif /* MASTER_H_ */
