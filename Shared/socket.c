#include "socket.h"

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BACKLOG 5
#define MAXSIZE 1024

static void check_descriptor(int descriptor);
static struct addrinfo *create_addrinfo(const char *ip, const char *port);
static t_socket create_socket(struct addrinfo *addr);
static size_t sendall(t_socket sockfd, const void *buf, size_t len);
static size_t recvall(t_socket sockfd, void *buf, size_t len);

// ========== Public functions ==========

t_socket socket_init(const char *ip, const char *port) {
	struct addrinfo *cur, *addr = create_addrinfo(ip, port);
	t_socket sockfd = -1;
	int ret = -1;

	for(cur = addr; cur != NULL; cur = cur->ai_next) {
		sockfd = create_socket(cur);
		if(sockfd == -1) continue;

		if(ip == NULL) {
			ret = bind(sockfd, cur->ai_addr, cur->ai_addrlen);
		} else {
			ret = connect(sockfd, cur->ai_addr, cur->ai_addrlen);
		}

		if(ret != -1) break;
		socket_close(sockfd);
	}

	freeaddrinfo(addr);
	check_descriptor(sockfd);
	check_descriptor(ret);

	if(ip == NULL) {
		check_descriptor(listen(sockfd, BACKLOG));
	}

	return sockfd;
}

t_socket socket_listen(const char *port) {
	t_socket sv_sock = socket_init(NULL, port);
	t_socket cli_sock = socket_accept(sv_sock);

	socket_close(sv_sock);
	return cli_sock;
}

t_socket socket_accept(t_socket sv_sock) {
	struct sockaddr rem_addr;
	socklen_t addr_size = sizeof rem_addr;

	t_socket cli_sock = accept(sv_sock, &rem_addr, &addr_size);
	return cli_sock;
}

t_socket socket_connect(const char *ip, const char *port) {
	return socket_init(ip, port);
}

size_t socket_send_string(t_socket sockfd, const char *message) {
	return sendall(sockfd, message, strlen(message) + 1);
}

size_t socket_send_bytes(t_socket sockfd, const char *message, size_t size) {
	return sendall(sockfd, message, size);
}

size_t socket_receive_string(t_socket sockfd, char *message) {
	return recvall(sockfd, message, MAXSIZE);
}

size_t socket_receive_bytes(t_socket sockfd, char *message, size_t size) {
	return recvall(sockfd, message, size);
}

fdset_t socket_set_create() {
	fdset_t fds;
	fds.max = -1;
	FD_ZERO(&fds.set);
	return fds;
}

void socket_set_add(fdset_t *fds, t_socket fd) {
	FD_SET(fd, &fds->set);
	if(fd > fds->max) {
		fds->max = fd;
	}
}

void socket_set_remove(fdset_t *fds, t_socket fd) {
	if(!FD_ISSET(fd, &fds->set)) return;
	FD_CLR(fd, &fds->set);
	if(fd == fds->max) {
		fds->max--;
	}
}

int socket_set_contains(fdset_t *fds, t_socket fd) {
	return FD_ISSET(fd, &fds->set);
}

void socket_close(t_socket sockfd) {
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
}

// ========== Private functions ==========

static void check_descriptor(int descriptor) {
	if(descriptor == -1) {
		fprintf(stderr, "%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

static struct addrinfo *create_addrinfo(const char *ip, const char *port) {
	struct addrinfo hints, *addr;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if(ip == NULL) hints.ai_flags = AI_PASSIVE;

	int status = getaddrinfo(ip, port, &hints, &addr);
	if(status != 0) {
		fprintf(stderr, "%s\n", gai_strerror(status));
	}

	return addr;
}

static t_socket create_socket(struct addrinfo *addr) {
	t_socket sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

	if(sockfd != -1) {
		int reuse = 1;
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);
	}

	return sockfd;
}

static size_t sendall(t_socket sockfd, const void *buf, size_t len) {
	size_t bytes_sent = 0;

	while(bytes_sent < len) {
		ssize_t n = send(sockfd, buf + bytes_sent, len - bytes_sent, 0);
		check_descriptor(n);
		bytes_sent += n;
	}

	return bytes_sent;
}

static size_t recvall(t_socket sockfd, void *buf, size_t len) {
	size_t bytes_received = 0;

	while(bytes_received < len) {
		ssize_t n = recv(sockfd, buf + bytes_received, len - bytes_received, 0);
		check_descriptor(n);
		bytes_received += n;
		if(n == 0 || (len == MAXSIZE && ((char*)buf)[bytes_received - 1] == '\0')) {
			break;
		}
	}

	return bytes_received;
}

//////////////////////////////////////////////////

#include <arpa/inet.h>

int crearSocket() // funcion para crear socket
{
	int socketfd;
	if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) //inicializamos el socket
			{
		perror("Socket Fail");
		exit(1);
	}
	return socketfd; //devolvemos el socket creado
}


int obtenerSocketMaximoInicial(int socketYama, int socketDataNode) {
	int socketMaximoInicial = 0;

	if (socketYama > socketDataNode) {
		socketMaximoInicial = socketYama;
	} else {
		socketMaximoInicial = socketDataNode;
	}
	return socketMaximoInicial;
}

void inicializarSOCKADDR_IN(struct sockaddr_in* direccion, char* direccionIP,
		char* puerto) // La funcion transforma sola los datos de host a network
{
	direccion->sin_family = AF_INET;
	direccion->sin_addr.s_addr = inet_addr(direccionIP);
	direccion->sin_port = htons(atoi(puerto));
	memset(&(direccion->sin_zero), '\0', 8);
	return;
}

void reutilizarSocket(int socketFD) {
	int yes = 1;
	if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) //el socket se puede reutilizar
			{
		perror("setsockopt");
		exit(2);
	}
	return;
}

void asignarDirecciones(int socketFD, const struct sockaddr* sockDIR) //Asociamos el puerto y direccion al socket
{
	if (bind(socketFD, sockDIR, sizeof(struct sockaddr)) == -1) {
		perror("Bind fail");
		exit(3);
	}
	return;
}
