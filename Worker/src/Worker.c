#include "funcionesWorker.h"

#define MAX_IP_LEN 16   // aaa.bbb.ccc.ddd -> son 15 caracteres, 16 contando un '\0'
#define MAX_PORT_LEN 6  // 65535 -> 5 digitos, 6 contando un '\0'
#define MAX_NOMBRE_NODO 5
#define MAX_RUTA 25
#define MAXIMO_TAMANIO_DATOS 256 //definiendo el tamanio maximo
#define MAXCONEXIONESLISTEN 10





//funciones


int main(int argc, char* argv[]) {

	process_init(PROC_WORKER);
	mostrar_configuracion();
	listen_to_master();

	return EXIT_SUCCESS;
}


