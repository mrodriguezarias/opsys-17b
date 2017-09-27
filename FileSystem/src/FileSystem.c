#include <process.h>
#include <stdlib.h>
#include "console.h"
#include "server.h"

#include <mlist.h>
#include <string.h>
#include <file.h>

int main() {
	process_init(PROC_FILESYSTEM);
	server();
	console();
	return EXIT_SUCCESS;
}
