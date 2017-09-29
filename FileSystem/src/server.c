#include "server.h"
#include <stddef.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <config.h>
#include <protocol.h>
#include "estructuras.h"
#include <mlist.h>

#define MAXIMO_TAMANIO_DATOS 256 //definiendo el tamanio maximo

typedef struct {
	int id;
	int tamanio;
} t_cabecera;

//funciones
void handshakeConDataNode(int, struct sockaddr_in, fd_set,fd_set, int);
void handshakeConYama(int, struct sockaddr_in, fd_set,fd_set, int);

void server() {
	fd_set master;
	fd_set read_fds;
	int maximo_Sockets;
	//datanode
	int socketEscuchandoDataNode;
	struct sockaddr_in direccion_DataNode;

	//crea socket para el datanode
	inicializarSOCKADDR_IN(&direccion_DataNode, "127.0.0.1", (char*) config_get("PUERTO_DATANODE"));
	socketEscuchandoDataNode = crearSocket();

	//Yama
	struct sockaddr_in direccion_Yama;
	int socketEscuchandoYama;

	//crea socket para el yama
	inicializarSOCKADDR_IN(&direccion_Yama, "127.0.0.1", (char*) config_get("PUERTO_YAMA"));
	socketEscuchandoYama = crearSocket();

	//socket servidor
	reutilizarSocket(socketEscuchandoDataNode); // obviar el mensaje "address already in use"
	asignarDirecciones(socketEscuchandoDataNode,
			(struct sockaddr *) &direccion_DataNode); // asignamos el Struct de las direcciones al socket
	reutilizarSocket(socketEscuchandoYama);
	asignarDirecciones(socketEscuchandoYama, (struct sockaddr *) &direccion_Yama);

	//Preparo para escuchar los 2 puertos en el master
	printf("Esperando conexiones entrantes \n");
	FD_ZERO(&master);
	FD_SET(socketEscuchandoDataNode, &master);
	FD_SET(socketEscuchandoYama, &master);
	maximo_Sockets = obtenerSocketMaximoInicial(socketEscuchandoYama,socketEscuchandoDataNode);

	if (listen(socketEscuchandoDataNode, 10) == -1) {
		perror("listen");
		exit(1);
	}
	if (listen(socketEscuchandoYama, 10) == -1) {
		perror("listen");
		exit(1);
	}
	//Comienza el Select
	for (;;) {
		int nuevoSocket;
		FD_ZERO(&read_fds);
		struct sockaddr_in remoteaddr;
		socklen_t addrlen;
		read_fds = master; // cópialo
		if (select((maximo_Sockets) + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}

		if (FD_ISSET(socketEscuchandoDataNode, &read_fds)) { // ¡¡tenemos datos de una datanode!!
			printf("Escucho algo por datanode \n");
			addrlen = sizeof(remoteaddr);
			if ((nuevoSocket = accept(socketEscuchandoDataNode,
					(struct sockaddr *) &remoteaddr, (socklen_t *) &addrlen))
					== -1) {
				perror("Error de aceptacion de un datanode \n");
			} else {
				FD_SET(nuevoSocket, &master); // añadir al conjunto maestro
				FD_SET(nuevoSocket, &read_fds);
				maximo_Sockets =
						(maximo_Sockets < nuevoSocket) ?
								nuevoSocket : maximo_Sockets;
				printf("Ya acepte un datanode \n ");
				handshakeConDataNode(nuevoSocket, remoteaddr, master, read_fds,maximo_Sockets);
				inicializarNodo(nuevoSocket);
				//ejecuto lo que tengo que hacerr para el datanodee....
			}
		} else if (FD_ISSET(socketEscuchandoYama, &read_fds)) { // ¡¡tenemos datos de un Yama!!
			addrlen = sizeof(remoteaddr);

			////////// Debe estar en estado estable el filesystem para poder aceptar conexion de un YAMA


			if ((nuevoSocket = accept(socketEscuchandoYama,
					(struct sockaddr *) &remoteaddr, &addrlen)) == -1) {
				perror("Error de aceptacion de Yama \n");
			} else {
				FD_SET(nuevoSocket, &master); // añadir al conjunto maestro
				FD_SET(nuevoSocket, &read_fds);
				maximo_Sockets =
						(maximo_Sockets < nuevoSocket) ?
								nuevoSocket : maximo_Sockets;
				handshakeConYama(nuevoSocket, remoteaddr, master, read_fds, maximo_Sockets);
				//ejecuto rutina con yama

			}
		}

		else {
			/*Si no entro en los if anteriores es por que no es conexion nueva por
			 ninguno de los dos sockets, por lo tanto es algun socket descriptor
			 ya almacenado que tiene actividad.
			 luego gestionar mensaje*/
			int socketFor;
			int bytes;
			t_cabecera header;
			for (socketFor = 0; socketFor < (maximo_Sockets + 1); socketFor++) {
				if (FD_ISSET(socketFor, &read_fds)) {
					if ((bytes = recv(socketFor, &header, sizeof(header), 0)) //recibe un mensaje el cual en caso de error es menor a cero y se limpian el FD y se cierra el socket
							<= 0) {
						FD_CLR(socketFor, &read_fds);
						FD_CLR(socketFor, &master);
						close(socketFor);
					} else {
						///switch con mensajes
					}
				}
			}

		}
	}
}

void handshakeConDataNode(int nuevoSocket, struct sockaddr_in remoteaddr, fd_set master,
		fd_set read_fds, int maximo_Sockets) {
	t_packet packet = protocol_receive(nuevoSocket);
		if(packet.operation == OP_HANDSHAKE && packet.sender == PROC_DATANODE) {
			printf("FileSystem: nueva conexion desde %s en socket %d\n",
					inet_ntoa(remoteaddr.sin_addr), nuevoSocket);
			if (send(nuevoSocket, "Acceso concedido!!", 19, 0) == -1) {
				perror("Error en el send");
			}
		} else {
			printf(
					"El proceso que requirio acceso, no posee los permisos adecuados\n");
			if (send(nuevoSocket, "No esta autorizado", 19,
					0) == -1) {
				perror("Error en el send");
			}
			close(nuevoSocket);
		}

}

void handshakeConYama(int nuevoSocket, struct sockaddr_in remoteaddr, fd_set master,
		fd_set read_fds, int maximo_Sockets) {

	t_packet packet = protocol_receive(nuevoSocket);
	if(packet.operation == OP_HANDSHAKE && packet.sender == PROC_YAMA) {
		printf("FileSystem: nueva conexion desde %s en socket %d\n",
				inet_ntoa(remoteaddr.sin_addr), nuevoSocket);
		if (send(nuevoSocket, "Acceso concedido!!", 19, 0) == -1) {
			perror("Error en el send");
		}
	} else {
		printf(
				"El proceso que requirio acceso, no posee los permisos adecuados\n");
		if (send(nuevoSocket, "No esta autorizado", 19,
				0) == -1) {
			perror("Error en el send");
		}
		close(nuevoSocket);
	}
}

void inicializarNodo(int socketNodo){
	int operacion = REGISTRARNODO;
	int envio = send(socketNodo,&operacion,sizeof(operacion),0);
	printf("mande la inicializacion %d \n",envio);
	//recibo lo que necesito.
	Nodo* infoDelNodo;
	t_packet packet  = protocol_receive(socketNodo);
	printf("pase el receive del protocolo \n");
	if(packet.sender == PROC_DATANODE){
		 infoDelNodo = infoNodo_unpack(packet.content);
	}
	free(packet.content.data);
	registrarNodo(infoDelNodo,socketNodo);
}

Nodo* infoNodo_unpack(t_serial serial){
	Nodo* infoDelNodo = malloc(sizeof(Nodo));
	serial_unpack(serial, "si", &infoDelNodo->nombreNodo, &infoDelNodo->total);
	return infoDelNodo;
}

void registrarNodo(Nodo* unNodo,int socketNodo){
	if(estadoAnteriorexistente){
		busquedaNodo_GLOBAL =  unNodo->nombreNodo;
		listaNodosConectados = list_create();
		if(list_any_satisfy(tablaNodos.nodos,*encontreNodo)){
		list_add(listaNodosConectados,unNodo->nombreNodo);
		}
		else{
			close(socketNodo);
		}
	}
	else{
		tablaNodos.tamanio += unNodo->total;
		tablaNodos.libre += unNodo->total;
		unNodo->libre = unNodo->total;
		list_add(tablaNodos.nodos,unNodo);
		char* datosAguardar = malloc(sizeof(char)*((list_size(tablaNodos.nodos)*9)+4));
		sprintf(datosAguardar,"%d",tablaNodos.tamanio);
		config_set_value(archivoNodos,"TAMANIO",datosAguardar);
		sprintf(datosAguardar,"%d",tablaNodos.libre);
		config_set_value(archivoNodos,"LIBRE",datosAguardar);
		datosAguardar = pasarlistaDenodosAChar(tablaNodos.nodos);
		printf("el nombre del nodo es %s \n",unNodo->nombreNodo);

		config_set_value(archivoNodos,"NODOS",datosAguardar); ///ROMPE ESTA LINEA

		char* key =  malloc(12*sizeof(char*));

		sprintf(key,"%sTotal",unNodo->nombreNodo);
		sprintf(datosAguardar,"%d",unNodo->total);
		config_set_value(archivoNodos,key,datosAguardar);
		sprintf(datosAguardar,"%d",unNodo->total);
		sprintf(key,"%sLibre",unNodo->nombreNodo);
		config_set_value(archivoNodos,key,datosAguardar);
		free(key);
		//debo crear el bitmap de ese nodo
		t_config* configBitmapNodo;
		char* nombreEstructura = malloc(sizeof(char)*16);
		sprintf(nombreEstructura,"bitmaps/%s",unNodo->nombreNodo);
		char* rutaArchivo = config_file(nombreEstructura,"dat");
		fopen(rutaArchivo,"w+");
		configBitmapNodo = config_create(rutaArchivo);
		char* bitmap = generarBitmap(unNodo->total);
		config_set_value(configBitmapNodo,"BITMAP",bitmap);
		config_save(configBitmapNodo);
		free(nombreEstructura);

	}
}

char* pasarlistaDenodosAChar(t_list* listaDeNodos){
 int i = 0;
 Nodo* unNodo = list_get(listaDeNodos, i);
 char* datosAguardar = malloc(sizeof(char)*((list_size(tablaNodos.nodos)*9)+4));
 sprintf(datosAguardar, "[%s", unNodo->nombreNodo);
 for(i=1; i < list_size(listaDeNodos); i++){
  unNodo = list_get(listaDeNodos, i);
  sprintf(datosAguardar,",%s",unNodo->nombreNodo);
 }
 //sprintf(datosAguardar, ",%s]", unNodo->nombreNodo);
 strcat(datosAguardar,"]");
 printf("datos a guardar %s \n",datosAguardar);
 free(unNodo);
 return datosAguardar;
}

bool encontreNodo(void* unNodo){
	printf("nodo que voy a comprar: %s \n",((Nodo*) unNodo)->nombreNodo);
	return !strcmp(((Nodo*) unNodo)->nombreNodo,busquedaNodo_GLOBAL) ? true : false;
}

char* generarBitmap(int totalBloquesNodo){
	char* bitmap = malloc(sizeof(char)*totalBloquesNodo);
	strcpy(bitmap,"");
	strcat(bitmap,"[0");
	int i;
	for(i=1;i<totalBloquesNodo;i++){
		strcat(bitmap,",0");
	}
	strcat(bitmap,"]");
	return bitmap;
}

