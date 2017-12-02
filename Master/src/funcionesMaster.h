#ifndef FUNCIONESMASTER_H_
#define FUNCIONESMASTER_H_

#include <string.h>
#include <serial.h>
#include "Master.h"
#include "manejadores.h"
#include <file.h>
#include <mtime.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

void kill_thread(t_hilos*);

void node_drop();

void actualizar_hilo(int);

void guardar_socket(t_socket socket);

bool enviar_operacion_worker(int, t_socket, t_serial*);

void response_worker(t_socket, int*);

bool enviar_resultado_yama(int, t_serial*);

t_hilos* set_hilo(int, char*);

void cargar_scripts(char*, char*);

void liberar_scripts();

time_t get_current_time();

const char *datetime(time_t);

const char *timediff(time_t, time_t);

void verificarParalelismo(int);

void init(char*[]);
void terminate();

#endif //MASTER_H
