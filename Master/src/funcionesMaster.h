#ifndef FUNCIONESMASTER_H_
#define FUNCIONESMASTER_H_

#include <string.h>
#include <serial.h>
#include "Master.h"
#include "manejadores.h"
#include <file.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

void actualizar_hilo(int);

void enviar_operacion_worker(int, t_socket, t_serial*);

void enviar_resultado_yama(int, t_serial*);

t_hilos* set_hilo(int);

void cargar_scripts(char*, char*);

void liberar_scripts();

time_t get_current_time();

const char *datetime(time_t);

const char *timediff(time_t, time_t);

void verificarParalelismo();

void init(char*[]);
void terminate();

#endif //MASTER_H
