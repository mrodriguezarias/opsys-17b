#ifndef thread_h
#define thread_h

#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

typedef pthread_t thread_t;
typedef pthread_mutex_t mutex_t;

/**
 * Crea un hilo de usuario usando la biblioteca de hilos POSIX.
 * @param routine Rutina que ejecutará el hilo.
 * @param arg Argumento pasado a la rutina.
 * @return Identificador del hilo creado.
 */
thread_t thread_create(void *routine, void *arg);

/**
 * Función bloqueante que espera a que termine de ejecutar un hilo.
 * @param thread Hilo a esperar.
 */
void thread_wait(thread_t thread);

/**
 * Manda una señal al hilo para cancelarlo y espera a que el hilo termine.
 * @param thread Hilo que se va a terminar.
 */
void thread_kill(thread_t thread);

/**
 * Envía una señal a un determinado hilo.
 * @param thread Hilo a enviar la señal.
 * @param signal Señal a enviar.
 */
void thread_signal_send(thread_t thread, int signal);

/**
 * Establece la rutina que se ejecutará al recibir determinada señal.
 * @param signal Descriptor de la señal.
 * @param routine Rutina que se ejecutará.
 */
void thread_signal_capture(int signal, void (*routine)(int));

/**
 * Crea un semáforo de exclusión mutua (mutex).
 * @return Semáforo mutex.
 */
mutex_t thread_mutex(void);

/**
 * Bloquea un semáforo de exclusión mutua.
 * @param mutex Semáforo a bloquear.
 */
void thread_mutex_lock(mutex_t *mutex);

/**
 * Desbloquea un semáforo de exclusión mutua.
 * @param mutex Semáforo a desbloquear.
 */
void thread_mutex_unlock(mutex_t *mutex);

/**
 * Destruye un semáforo de exclusión mutua.
 * @param mutex Semáforo a destruir.
 */
void thread_mutex_destroy(mutex_t *mutex);

/**
 * Crea un semáforo contador.
 * @return Semáforo contador.
 */
sem_t thread_sem(unsigned value);

/**
 * Disminuye el valor de un semáforo contador.
 * Si el valor pasa a ser negativo, se bloquea.
 * @param sem Semáforo contador.
 */
void thread_sem_wait(sem_t *sem);

/**
 * Aumenta el valor de un semáforo contador.
 * @param sem Semáforo contador.
 */
void thread_sem_signal(sem_t *sem);

/**
 * Destruye un semáforo contador.
 * @param mutex Semáforo a destruir.
 */
void thread_sem_destroy(sem_t *sem);

#endif /* thread_h */
