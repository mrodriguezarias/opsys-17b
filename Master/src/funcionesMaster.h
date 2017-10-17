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

#endif //MASTER_H
