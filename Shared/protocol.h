#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <process.h>
#include <serial.h>
#include <socket.h>
#include <stdbool.h>

#define RESPONSE_OK 0
#define RESPONSE_ERROR -1

typedef enum {
	OP_UNDEFINED,
	OP_PING,
	OP_HANDSHAKE,
	OP_RESPONSE,
	OP_INIT_JOB, //master -> Yama
	OP_NODE_INFO,
	OP_REQUEST_BLOCK,
	OP_SEND_BLOCK,
	OP_REQUEST_FILE_INFO,			// yama -> filesystem

	OP_NODES_ACTIVE_INFO,			// filesystem -> yama
	OP_ARCHIVO_NODES,			// filesystem -> yama
	OP_ARCHIVO_INEXISTENTE,			// filesystem -> yama
	OP_INICIAR_TRANSFORMACION,		// yama -> master
  	OP_INICIAR_REDUCCION_LOCAL,		// yama -> master
	OP_INICIAR_REDUCCION_GLOBAL,	// yama -> master
	OP_INICIAR_ALMACENAMIENTO,		// yama -> master
	OP_ERROR_JOB,					// yama -> master

	OP_TRANSFORMACION_LISTA,		// master -> yama
	OP_REDUCCION_LOCAL_LISTA,		// master -> yama
	OP_REDUCCION_GLOBAL_LISTA,		// master -> yama
	OP_ALMACENAMIENTO_LISTA,		// master -> yama

	OP_GETBLOQUE,					// filesystem -> datanode
	OP_SETBLOQUE,					// filesystem -> datanode

	REGISTRARNODO,					// filesystem -> datanode

} t_operation;

//interrupciones del job
enum{FS_NO_ESTABLE = -1,
	ARCHIVO_INEXISTENTE = -2,
	ERROR_REPLANIFICACION = -3,
	ERROR_REDUCCION_LOCAL = -4,
	ERROR_REDUCCION_GLOBAL = -5
}; //yama -> master

typedef struct {
	t_process sender;		// Proceso remitente
	t_operation operation;	// Operación a realizar
	t_serial *content;		// Contenido serializado
} t_packet;

/**
 * Crea un paquete para enviar una operación.
 * @param operation Operación a realizar.
 * @param content Contenido serializado.
 * @return Paquete.
 */
t_packet protocol_packet(t_operation operation, t_serial *content);

/**
 * Envía un packete a un determinado socket.
 * @param packet Paquete.
 * @param socket Descriptor del socket.
 * @return Valor lógico indicando si se pudo enviar el paquete.
 */
bool protocol_send_packet(t_packet packet, t_socket socket);

/**
 * Recibe un paquete de un determinado socket.
 * Si se recibe contenido, luego de usarlo debe ser liberado con free().
 * @param socket Descriptor del socket.
 * @return Paquete.
 */
t_packet protocol_receive_packet(t_socket socket);

/**
 * Envía un apretón de manos.
 * @param socket Descriptor del socket.
 */
void protocol_send_handshake(t_socket socket);

/**
 * Recibe un apretón de manos y verifica que sea del proceso que corresponda.
 * @param socket Descriptor del socket.
 * @param process Proceso del que se espera recibir el apretón de manos.
 * @return Valor lógico indicando si se recibió correctamente.
 */
bool protocol_receive_handshake(t_socket socket, t_process process);

/**
 * Envía un código de respuesta a un socket.
 * @param socket Descriptor del socket.
 * @param code Código de respuesta a enviar.
 */
void protocol_send_response(t_socket socket, int code);

/**
 * Recibe un código de respuesta de un socket.
 * @param socket Descriptor del socket.
 * @return Código de respuesta (-1 = error).
 */
int protocol_receive_response(t_socket socket);

#endif /* PROTOCOL_H_ */
