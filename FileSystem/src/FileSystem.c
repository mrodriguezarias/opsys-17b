#include <config.h>
#include <process.h>
#include "console.h"
#include "server.h"

int main() {
	process_init(PROC_FILESYSTEM);
	config_load();
	escucharPuertosDataNodeYYama();
	console();
	return 0;
}
