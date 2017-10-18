#include "server.h"

#include <config.h>
#include <log.h>
#include <mstring.h>
#include <process.h>
#include <protocol.h>
#include <serial.h>
#include <socket.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <struct.h>
#include "funcionesYAMA.h"

void init_job(t_serial *content);

void listen_to_master() {
	log_print("Escuchando puertos de master");
	t_socket sv_sock = socket_init(NULL, config_get("MASTER_PUERTO"));
	t_fdset sockets = socket_set_create();
	socket_set_add(sv_sock, &sockets);
	while(true) {
		t_fdset selected = socket_select(sockets);

		for(t_socket sock = 3; sock <= sockets.max; sock++) {
			if(!socket_set_contains(sock, &selected)) continue;
			if(sock == sv_sock) {
				t_socket cli_sock = socket_accept(sv_sock);
				t_packet packet = protocol_receive_packet(cli_sock);
				if(packet.operation == OP_HANDSHAKE && packet.sender == PROC_MASTER) {
					socket_set_add(cli_sock, &sockets);
					log_inform("Conectado proceso Master por socket %i", cli_sock);
				} else {
					socket_close(cli_sock);
				}
			} else {
				t_packet packet = protocol_receive_packet(sock);
				if(packet.operation == OP_UNDEFINED) {
					socket_close(sock);
					socket_set_remove(sock, &sockets);
					continue;
				}

				switch(packet.operation) {
				case OP_INIT_JOB: init_job(packet.content);
					log_print("OP_INIT_JOB");
					//recibo lista de nodos conectados

					//pido informacion del archivo al filesystem
					//void protocol_send_response(socket_FileSystem,OP_ARCHIVOS_INFO);
					//recibo info
					t_packet packet;
					packet = protocol_receive_packet(socket_FileSystem);
					//mlist_t* listInformacionArch = list_informacionArchivo_unpack(packet.content);
					//planifico
					int tamaniolistaNodos = mlist_length(listaNodosConectados);
					t_workerPlanificacion planificador[tamaniolistaNodos];
					planificar(planificador, tamaniolistaNodos);
					//inicio etapa de tranformacion
					tEtapaTransformacion* et = new_etapa_transformacion("Nodo1","127.0.0.1","5050",35,100,"/tmp/Master1-temp38");
					mlist_t* lista = mlist_create();
					mlist_append(lista,et);
					//agregarAtablaEstado(nodo,sock,bloque,"Transformacion",archivo_etapa,"En proceso");
					numeroJob +=1;
					mandar_etapa_transformacion(lista,sock);


					break;
				default:
					log_report("Operación desconocida: %s", packet.operation);
				}

				serial_destroy(packet.content);

//				char *string = socket_receive_string(sock);
//				if(string) {
//					printf("Recibido: %s\n", string);
//					log_inform("Recibido: %s \n",string);
//					//planificicar(script);
//				} else {
//					log_inform("Desconectado proceso Master de socket %i", sock);
//					socket_close(sock);
//					socket_set_remove(sock, &sockets);
//				}

//				switch(packet.operation){
//				case(TRANSFORMACION_LISTA):
//						break;
//				case(REDUCCION_LOCAL_LISTA):
//						break;
//				case(REDUCCION_GLOBAL_LISTA):
//						break;
//				default:
//					break;
//				}
			}
		}
	}
}

void init_job(t_serial *content) {
	char *file = mstring_empty(NULL);
	serial_unpack(content, "s", &file);
	log_print("Comenzando tarea para archivo %s", file);
	free(file);
}


void agregarAtablaEstado(char* nodo,int Master,int bloque,char* etapa,char* archivo_temporal,char* estado){
	t_Estado*  nuevoEstado;
	nuevoEstado->job = numeroJob;
	nuevoEstado->master = Master;
	nuevoEstado->nodo = nodo;
	nuevoEstado->block = bloque;
	nuevoEstado->etapa = etapa;
	nuevoEstado->archivoTemporal = archivo_temporal;
	nuevoEstado->estado = estado;
	mlist_append(listaEstados,nuevoEstado);

}
