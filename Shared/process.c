#include <config.h>
#include <process.h>
#include <system.h>
#include <mstring.h>
#include <path.h>

static t_process current = PROC_UNDEFINED;

static char *names[] = {"(Undefined)", "YAMA", "FileSystem", "Master", "Worker", "DataNode"};

// ========== Funciones públicas ==========

void process_init() {
	current = process_type(path_name(system_proc()));
	system_init();
	config_load();
}

t_process process_current() {
	return current;
}

const char *process_name(t_process process) {
	return names[process];
}

t_process process_type(const char *name) {
	int l = sizeof(names) / sizeof(*names);
	for(int i = 0; i < l; i++) {
		if(mstring_equal(name, names[i])) return i;
	}
	return PROC_UNDEFINED;
}
