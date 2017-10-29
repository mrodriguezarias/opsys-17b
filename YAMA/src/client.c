#include "client.h"

#include <config.h>
#include <log.h>
#include <protocol.h>
#include <socket.h>

#include "YAMA.h"

int connect_to_filesystem() {
	t_socket socket = socket_connect(config_get("FS_IP"), config_get("FS_PUERTO"));
	protocol_send_handshake(socket);
	int response = protocol_receive_response(socket);
	if (response == RESPONSE_ERROR){
		log_print("Conexión rechazada. El FileSystem no se encuentra estable.");
		socket_close(socket);
	}else{
		log_inform("Conectado a proceso FileSystem por socket %i", socket);
		yama.fs_socket = socket;
	}
	return response;
}
