#include "server.h"
#include <config.h>
#include <stddef.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define MAXIMO_TAMANIO_DATOS 256 //definiendo el tamanio maximo

typedef struct {
	int id;
	int tamanio;
} t_cabecera;

//funciones
void handshakeConDataNode(int, struct sockaddr_in, fd_set,fd_set, int);
void handshakeConYama(int, struct sockaddr_in, fd_set,fd_set, int);

void escucharPuertosDataNodeYYama() {
	fd_set master;
	fd_set read_fds;
	int maximo_Sockets;
	//datanode
	int socketEscuchandoDataNode;
	struct sockaddr_in direccion_DataNode;

	//crea socket para el datanode
	inicializarSOCKADDR_IN(&direccion_DataNode, "127.0.0.1", (char*) config_get("PUERTO_DATANODE"));
	socketEscuchandoDataNode = crearSocket(); //abstraccion, se debe armar la funcion

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
				handshakeConDataNode(nuevoSocket, remoteaddr, master, read_fds,
						maximo_Sockets);
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
	char buffer_select[MAXIMO_TAMANIO_DATOS];
	int cantidadBytes;

	cantidadBytes = recv(nuevoSocket, buffer_select, sizeof(buffer_select), 0);
	buffer_select[cantidadBytes] = '\0';
	printf("Credencial recibida: %s\n", buffer_select);
	if (!strcmp(buffer_select, "Yatpos-DataNode\0")) { //Yatpos es la credencial que autoriza al proceso a seguir conectado
		printf("FileSystem: nueva conexion desde %s en socket %d\n",
				inet_ntoa(remoteaddr.sin_addr), nuevoSocket);
		if (send(nuevoSocket, "Bienvenido al FileSystem!", 26, 0) == -1) {
			perror("Error en el send");
		}
	} else {
		printf(
				"El proceso que requirio acceso, no posee los permisos adecuados\n");
		if (send(nuevoSocket, "Usted no esta autorizado!", MAXIMO_TAMANIO_DATOS,
				0) == -1) {
			perror("Error en el send");
		}
		close(nuevoSocket);
	}
}

void handshakeConYama(int nuevoSocket, struct sockaddr_in remoteaddr, fd_set master,
		fd_set read_fds, int maximo_Sockets) {
	char buffer_select[MAXIMO_TAMANIO_DATOS];
	int cantidadBytes;

	cantidadBytes = recv(nuevoSocket, buffer_select, sizeof(buffer_select), 0);
	buffer_select[cantidadBytes] = '\0';
	printf("Credencial recibida: %s\n", buffer_select);
	if (!strcmp(buffer_select, "Yatpos-Yama\0")) { //Yatpos es la credencial que autoriza al proceso a seguir conectado
		printf("FileSystem: nueva conexion desde %s en socket %d\n",
				inet_ntoa(remoteaddr.sin_addr), nuevoSocket);
		if (send(nuevoSocket, "Bienvenido al FileSystem!", 26, 0) == -1) {
			perror("Error en el send");
		}
	} else {
		printf(
				"El proceso que requirio acceso, no posee los permisos adecuados\n");
		if (send(nuevoSocket, "Usted no esta autorizado!", MAXIMO_TAMANIO_DATOS,
				0) == -1) {
			perror("Error en el send");
		}
		close(nuevoSocket);
	}
}
