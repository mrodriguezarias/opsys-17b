#ifndef PROCESS_H_
#define PROCESS_H_

#define PROCESS_COUNT 5

typedef enum {
	PROC_UNDEFINED,
	PROC_YAMA,
	PROC_FILESYSTEM,
	PROC_MASTER,
	PROC_WORKER,
	PROC_DATANODE
} t_process;

/**
 * Inicializa un proceso.
 * Debe llamarse al inicio de cada proceso.
 */
void process_init(void);

/**
 * Devuelve el proceso actual en ejecución.
 * @return Proceso en ejecución.
 */
t_process process_current(void);

/**
 * Devuelve el nombre del proceso pasado por parámetro.
 * @param process Proceso.
 * @return Nombre del proceso.
 */
const char *process_name(t_process process);

/**
 * Devuelve el tipo de proceso de un determinado nombre.
 * @param name Nombre del proceso.
 * @return Proceso.
 */
t_process process_type(const char *name);

/**
 * Libera los recursos inicializados por process_init().
 */
void process_term();

#endif /* PROCESS_H_ */
