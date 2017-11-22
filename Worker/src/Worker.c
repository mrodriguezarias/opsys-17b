#include "funcionesWorker.h"

int main(int argc, char* argv[]) {
	process_init();
	//data_open(config_get("RUTA_DATABIN"),mstring_toint(config_get("DATABIN_SIZE")));
	listen_to_master();
	data_close();
	return EXIT_SUCCESS;
}



