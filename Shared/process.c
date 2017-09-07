#include <file.h>
#include "process.h"

static t_process current = UNDEFINED;

// ========== Funciones públicas ==========

void process_init(t_process process) {
	current = process;
	file_create_sysdirs();
}

t_process process_current() {
	return current;
}

const char *process_name(t_process process) {
	static char *names[] = {"YAMA", "FileSystem", "Master", "Worker", "DataNode"};
	return names[process];
}
