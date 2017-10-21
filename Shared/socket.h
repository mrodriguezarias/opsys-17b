#ifndef SOCKET_H_
#define SOCKET_H_

#include <sys/types.h>
#include <sys/select.h>
#include <stddef.h>
#include <stdbool.h>

typedef int t_socket;

typedef struct {
    fd_set set;
    t_socket max;
} t_fdset;

/**
 * Si se indica la IP, crea un socket y lo conecta al servidor de la IP y
 * puerto especificados. Si no se indica la IP, crea un socket y lo prepara
 * para escuchar conexiones en el puerto especificado.
 * @param ip Dirección IP del servidor.
 * @param port Puerto.
 * @return Descriptor del socket.
 */
t_socket socket_init(const char *ip, const char *port);

/**
 * Crea un socket de servidor para conectarse con un cliente a través de un
 * puerto determinado.
 * @param port Puerto de escucha.
 * @return Descriptor del socket del cliente.
 */
t_socket socket_listen(const char *port);

/*
 * Función bloqueante que espera por conexiones en un socket servidor y las
 * acepta devolviendo el socket cliente.
 * @param sv_sock Descriptor del socket del servidor.
 * @return Descriptor del socket del cliente (-1 si hubo error).
 */
t_socket socket_accept(t_socket sv_sock);

/**
 * Crea un socket de cliente para conectarse con un servidor en una dirección
 * IP y un puerto determinados.
 * @param ip Dirección IP del servidor.
 * @param port Puerto del servidor.
 * @return Descriptor del socket del servidor.
 */
t_socket socket_connect(const char *ip, const char *port);

/**
 * Envía datos binarios por una conexión abierta en un determinado socket.
 * @param sockfd Descriptor del socket.
 * @param message Mensaje a enviar.
 * @param size Tamaño de los datos.
 * @return Número de bytes enviados.
 */
size_t socket_send_bytes(t_socket sockfd, const char *message, size_t size);

/**
 * Recibe datos binarios por una conexión abierta en un determinado socket.
 * @param sockfd Descriptor del socket.
 * @param message Mensaje a recibir.
 * @param size Tamaño de los datos.
 * @return Número de bytes recibidos (-1 si hubo error).
 */
size_t socket_receive_bytes(t_socket sockfd, char *message, size_t size);

/**
 * Envía una cadena de texto por una conexión abierta en un determinado socket.
 * @param sockfd Descriptor del socket.
 * @param message Mensaje a enviar.
 */
void socket_send_string(t_socket sockfd, const char *message);

/**
 * Recibe una cadena de texto por una conexión abierta en un determinado socket.
 * La cadena recibida debe ser liberada con free() después de usarla.
 * @param sockfd Descriptor del socket.
 * @return Mensaje recibido.
 */
char *socket_receive_string(t_socket sockfd);

/**
 * Crea un conjunto de sockets para ser usado por socket_select().
 * @return Conjunto de sockets.
 */
t_fdset socket_set_create(void);

/**
 * Agrega un socket a un conjunto de sockets.
 * @param fd Socket a agregar.
 * @param fds Conjunto de sockets.
 */
void socket_set_add(t_socket fd, t_fdset *fds);

/**
 * Elimina un socket de un conjunto de sockets.
 * @param fd Socket a eliminar.
 * @param fds Conjunto de sockets.
 */
void socket_set_remove(t_socket fd, t_fdset *fds);

/**
 * Verifica si un socket está presente en un conjunto de sockets.
 * @param fd Socket a verificar.
 * @param fds Conjunto de sockets.
 */
int socket_set_contains(t_socket fd, t_fdset *fds);

/**
 * Verifica si un socket está activo.
 * @param fd Socket a verificar.
 * @return Valor lógico con el resultado.
 */
bool socket_alive(t_socket fd);

/**
 * Realiza un select() sobre un determinado set de sockets.
 * @param sockets Set de sockets.
 * @return Sockets seleccionados.
 */
t_fdset socket_select(t_fdset fds);

/**
 * Cierra un socket abierto con socket_listen() o socket_connect().
 * @param sockfd Descriptor del socket a cerrar.
 */
void socket_close(t_socket sockfd);

///////////////////////////////////////

#include <netinet/in.h>

int crearSocket();
int obtenerSocketMaximoInicial(int socketYama, int socketDataNode);
void inicializarSOCKADDR_IN(struct sockaddr_in* direccion, char* direccionIP, char* puerto);
void reutilizarSocket(int socketFD);
void asignarDirecciones(int socketFD, const struct sockaddr* sockDIR);

#endif /* SOCKET_H_ */

