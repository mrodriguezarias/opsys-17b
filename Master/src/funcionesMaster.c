#include "funcionesMaster.h"


void cargar_scripts(char* path_transf, char* path_reduc) {

	 //script.fd_transf = file_open(path_transf);
	 //script.script_transf = file_map(script.fd_transf);

	// script.fd_reduc = file_open(path_reduc);
	// script.script_reduc = file_map(script.fd_transf);

}

void liberar_scripts() {

	file_unmap(script.fd_transf, script.script_transf);
	file_unmap(script.fd_reduc, script.script_reduc);

}
