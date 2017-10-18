#ifndef FUNCIONESMASTER_H_
#define FUNCIONESMASTER_H_

#include <string.h>
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

void cargar_scripts(char*, char*);

void liberar_scripts();

time_t get_current_time();

const char *datetime(time_t);

const char *timediff(time_t, time_t);

void verificarParalelismo();

void terminate();

#endif //MASTER_H
