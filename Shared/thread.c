#include "thread.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void ptcheck(int ret);
static void signal_dummy_action(int signal);

// ========== Funciones públicas ==========

thread_t thread_create(void *routine, void *arg) {
	pthread_t thread;
	ptcheck(pthread_create(&thread, NULL, routine, arg));
	return thread;
}

void thread_wait(thread_t thread) {
	pthread_join(thread, NULL);
}

void thread_kill(thread_t thread) {
	pthread_kill(thread, SIGTERM);
	pthread_join(thread, NULL);
}

void thread_signal_capture(int signal, void (*routine)(int)) {
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sa.sa_handler = routine != NULL ? routine : signal_dummy_action;
	ptcheck(sigaction(signal, &sa, NULL));
}

void thread_signal_send(thread_t thread, int signal) {
	pthread_kill(thread, signal);
}

mutex_t thread_mutex() {
	mutex_t mutex;
	ptcheck(pthread_mutex_init(&mutex, NULL));
	return mutex;
}

void thread_mutex_lock(mutex_t *mutex) {
	ptcheck(pthread_mutex_lock(mutex));
}

void thread_mutex_unlock(mutex_t *mutex) {
	ptcheck(pthread_mutex_unlock(mutex));
}

void thread_mutex_destroy(mutex_t *mutex) {
	ptcheck(pthread_mutex_destroy(mutex));
}

sem_t thread_sem(unsigned value) {
	sem_t sem;
	sem_init(&sem, 0, value);
	return sem;
}

void thread_sem_wait(sem_t *sem) {
	ptcheck(sem_wait(sem));
}

void thread_sem_signal(sem_t *sem) {
	ptcheck(sem_post(sem));
}

void thread_sem_destroy(sem_t *sem) {
	ptcheck(sem_destroy(sem));
}

// ========== Funciones privadas ==========

static void ptcheck(int ret) {
	if(ret != 0) {
		fprintf(stderr, "Error de hilos: %s\n", strerror(ret));
		exit(EXIT_FAILURE);
	}
}

static void signal_dummy_action(int signal) {
	signal++;
}
