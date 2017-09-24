#include <file.h>
#include <process.h>
#include <stddef.h>
#include <stdlib.h>
#include <system.h>
#include <log.h>
#include <commons/config.h>
#include <commons/collections/dictionary.h>
#include <config.h>
#include <mstring.h>

static const char *config_name(void);
static char *config_file(bool user);
static void set_defaults(void);
static void check_properties(t_config *config);

static t_config *config = NULL;

// ========== Funciones públicas ==========

void config_load() {
	if(config != NULL) return;

	char *config_path = config_file(true);
	if(!file_exists(config_path)) {
		set_defaults();
	}
	config = config_create(config_path);
	free(config_path);

	if(config == NULL) {
		log_report("Error al intentar cargar archivo de configuración");
		exit(EXIT_FAILURE);
	}

	check_properties(config);
}

const char *config_get(const char *property) {
	return config_has_property(config, (char*)property) ? config_get_string_value(config, (char*)property) : NULL;
}

// ========== Funciones privadas ==========

static const char *config_name() {
	static const char *node = "Node";
	t_process process = process_current();
	if(process == PROC_WORKER || process == PROC_DATANODE) {
		return node;
	}
	return process_name(process);
}

static char *config_file(bool user) {
	const char *dir = user ? system_userdir() : system_rscdir();
	return mstring_create("%s/config/%s.cnf", dir, config_name());
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
			log_report("Falta definir la propiedad %s en el archivo de configuración", key);
			should_exit = true;
		}
	}

	dictionary_iterator(defaults->properties, iterator);
	config_destroy(defaults);
	if(should_exit) exit(EXIT_FAILURE);
}
