#ifndef PROCESS_H_
#define PROCESS_H_

#define PROCESS_COUNT 5

typedef enum { UNDEFINED = -1, YAMA, FILESYS, MASTER, WORKER, DATANODE } t_process;

/**
 * Inicializa un proceso.
 * Debe llamarse al inicio de cada proceso.
 * @param process Proceso a inicializar.
 */
void process_init(t_process process);

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

#endif /* PROCESS_H_ */
