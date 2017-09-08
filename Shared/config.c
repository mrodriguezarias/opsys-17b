#include "config.h"
#include <file.h>
#include <mstring.h>
#include <process.h>
#include <stddef.h>
#include <stdlib.h>
#include <log.h>
#include <commons/collections/dictionary.h>

static const char *config_name(void);
static char *config_file(bool user);
static void set_defaults(void);
static void check_properties(t_config *config);

static t_config *configs[PROCESS_COUNT];

// ========== Funciones públicas ==========

void config_load() {
	char *config_path = config_file(true);
	if(!file_exists(config_path)) {
		set_defaults();
	}

	t_config *config = config_create(config_path);
	free(config_path);

	if(config == NULL) {
		logep("Error al intentar cargar archivo de configuración");
		exit(EXIT_FAILURE);
	}

	check_properties(config);
	configs[process_current()] = config;
}

const char *config_get(const char *property) {
	return config_get_string_value(configs[process_current()], (char*)property);
}

// ========== Funciones privadas ==========

static const char *config_name() {
	static const char *node = "Node";
	t_process process = process_current();
	if(process == WORKER || process == DATANODE) {
		return node;
	}
	return process_name(process);
}

static char *config_file(bool user) {
	const char *dir = user ? file_userdir() : file_rscdir();
	return mstring_format("%s/config/%s.cnf", dir, config_name());
}

static void set_defaults() {
	char *user_config = config_file(true);
	char *default_config = config_file(false);
	file_copy(default_config, user_config);
	free(default_config);
	free(user_config);
}

static void check_properties(t_config *config) {
	char *default_file = config_file(false);
	t_config *defaults = config_create(default_file);

	free(default_file);

	bool should_exit = false;
	void iterator(char *key, void *_) {
		if(!config_has_property(config, key)) {
			logep("Falta definir la propiedad %s en el archivo de configuración", key);
			should_exit = true;
		}
	}

	dictionary_iterator(defaults->properties, iterator);
	config_destroy(defaults);
	if(should_exit) exit(EXIT_FAILURE);
}
