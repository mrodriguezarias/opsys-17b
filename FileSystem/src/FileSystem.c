#include <process.h>
#include <stdlib.h>
#include "console.h"
#include "server.h"

int main() {
	process_init(PROC_FILESYSTEM);
	server();
	console();
	return EXIT_SUCCESS;
}
