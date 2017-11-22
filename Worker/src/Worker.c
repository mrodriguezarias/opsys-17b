#include "funcionesWorker.h"

int main(int argc, char* argv[]) {
	process_init();
	listen_to_master();
	return EXIT_SUCCESS;
}



