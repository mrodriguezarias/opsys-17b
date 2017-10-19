#ifndef MANEJADORES_H
#define MANEJADORES_H

#include "funcionesMaster.h"
#include "connection.h"
#include <protocol.h>
#include <file.h>
#include <log.h>
#include <thread.h>
#include <protocol.h>
#include <pthread.h>
#include <serial.h>
#include <socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <struct.h>
#include <sys/wait.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include "Master.h"


void manejador_yama(t_packet);

void manejador_worker();


#endif /* MANEJADORES_H */
