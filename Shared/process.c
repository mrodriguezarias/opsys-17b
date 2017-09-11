#include <config.h>
#include <process.h>
#include <system.h>

static t_process current = PROC_UNDEFINED;

// ========== Funciones públicas ==========

void process_init(t_process process) {
	current = process;
	system_init();
	config_load();
}

t_process process_current() {
	return current;
}

const char *process_name(t_process process) {
	static char *names[] = {"(Undefined)", "YAMA", "FileSystem", "Master", "Worker", "DataNode"};
	return names[process];
}
