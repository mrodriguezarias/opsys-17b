#include <mstring.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system.h>
#include <protocol.h>
#include <path.h>

#include "nodelist.h"
#include "dirtree.h"
#include "FileSystem.h"
#include "filetable.h"

#define show_usage() {show_usage_for(current_cmd); return;}

// ========== Estructuras ==========

typedef struct {
	char *name;
	void (*func)(void);
	char *info;
	char *usage;
} t_command;

static int current_cmd = -1;
static char *current_args = NULL;

// ========== Declaraciones ==========

static void cmd_cat(void);
static void cmd_clear(void);
static void cmd_cpblock(void);
static void cmd_cpfrom(void);
static void cmd_cpto(void);
static void cmd_debug(void);
static void cmd_format(void);
static void cmd_help(void);
static void cmd_info(void);
static void cmd_ls(void);
static void cmd_md5(void);
static void cmd_mkdir(void);
static void cmd_mv(void);
static void cmd_quit(void);
static void cmd_rename(void);
static void cmd_rm(void);

static void init_console();
static void term_console();
static void show_usage_for(int cmd);
static char *extract_arg(int no);
static int num_args(void);
static void execute_line(const char *);
static t_command *find_command(const char *);
static void command_not_found(const char *);
static char **rl_completion(const char*, int, int);
static char *rl_generator(const char*, int);

// ========== Variables globales ==========

static t_command commands[] = {
		{"cat", cmd_cat, "Muestra el contenido de un archivo", "archivo"},
		{"clear", cmd_clear, "", ""},
		{"cpblock", cmd_cpblock, "Copia un bloque de un archivo en un nodo", "archivo bloque nodo"},
		{"cpfrom", cmd_cpfrom, "Copia un archivo local a yamafs", "archivo_local directorio_yamafs"},
		{"cpto", cmd_cpto, "Copia un archivo yamafs al sistema local", "archivo_yamafs directorio_local"},
		{"debug", cmd_debug, "", ""},
		{"exit", cmd_quit, "", ""},
		{"format", cmd_format, "Da formato al sistema de archivos", ""},
		{"help", cmd_help, "Lista los comandos disponibles", "[comando]"},
		{"info", cmd_info, "Muestra información de un archivo", "archivo"},
		{"ls", cmd_ls, "Lista los archivos de un directorio", "directorio"},
		{"md5", cmd_md5, "Muestra el MD5 de un archivo", "archivo"},
		{"mkdir", cmd_mkdir, "Crea un directorio", "directorio"},
		{"mv", cmd_mv, "Mueve un archivo o directorio", "ruta_original ruta_final"},
		{"quit", cmd_quit, "Termina la ejecución del proceso", ""},
		{"rename", cmd_rename, "Renombra un archivo o directorio", "ruta_original nombre_final"},
		{"rm", cmd_rm, "Elimina un archivo, directorio o bloque", "archivo | -d directorio | -b archivo bloque copia"},
		{NULL, NULL, NULL, NULL}
};

static bool should_quit = false;
static char *history_file = NULL;

// ========== Funciones públicas ==========

void console() {
	char *line;
	init_console();

	while(!should_quit && (line = readline("> "))) {
		mstring_trim(line);
		fflush(stdout);

		if(*line) {
			add_history(line);
			write_history(history_file);
			execute_line(line);
		}

		free(line);
	}

	term_console();
}

// ========== Funciones privadas ==========

static void init_console() {
	history_file = mstring_create("%s/.history", system_userdir());
	read_history(history_file);
	rl_attempted_completion_function = rl_completion;
}

static void term_console() {
	free(history_file);
}

static void show_usage_for(int cmd) {
	printf("Uso: %s %s\n", commands[cmd].name, commands[cmd].usage);
}

static char *extract_arg(int no) {
	if(mstring_isempty(current_args)) return NULL;
	int i = 0;
	char *p = current_args;
	int start = 0;
	int end = 0;
	bool q = false;
	bool s = false;
	do {
		while(!q && isspace(*p)) { s = true; p++; }
		if(*p == '"' || *p == '\'') q = !q;
		if(s || *p == '\0') {
			s = false;
			i++;
			end = p - current_args;
			if(no == i) {
				char *copy = mstring_trim(mstring_copy(current_args, start, end));
				char *a = copy, *b = mstring_end(copy);
				if(*a == '"' || *a == '\'') *a = ' ';
				if(*b == '"' || *b == '\'') *b = ' ';
				return mstring_trim(copy);
			}
			start = end;
		}
	} while(*p++ != '\0');
	return NULL;
}

static int num_args() {
	char *arg = NULL;
	int count = 0;
	int i = 1;
	while(arg = extract_arg(i++), arg != NULL) {
		if(!mstring_isempty(arg)) {
			count++;
		}
		free(arg);
	}
	return count;
}

// ========== Funciones de comandos ==========

static void cmd_cat() {
	if(num_args() != 1) show_usage();
	char *path = extract_arg(1);
	if(filetable_contains(path)) {
		filetable_cat(path);
	} else {
		fprintf(stderr, "Error: archivo inexistente.\n");
	}
	free(path);
}

static void cmd_clear() {
	printf("\033c");
}

static void cmd_cpblock() {
	puts("TODO");
}

static void cmd_cpfrom() {
	if(num_args() != 2) show_usage();
	char *path = extract_arg(1);
	char *dir = extract_arg(2);

	char *upath = path_create(PTYPE_USER, path);
	free(path);

	if(path_isfile(upath)) {
		filetable_cpfrom(upath, dir);
	} else {
		fprintf(stderr, "Error: archivo inexistente.\n");
	}

	free(upath);
	free(dir);
}

static void cmd_cpto() {
	if(num_args() != 2) show_usage();
	char *source_path = extract_arg(1);
	char *local_dir = extract_arg(2);

	filetable_cpto(source_path, local_dir);
	free(source_path);
	free(local_dir);
}

static void cmd_debug() {
	if(mstring_equal(current_args, "nodes")) {
		nodelist_print();
	} else if(mstring_equal(current_args, "dirs")) {
		dirtree_print();
	} else if(mstring_equal(current_args, "files")) {
		filetable_print();
	} else if(mstring_equal(current_args, "recv")) {
		puts("Prueba de petición de un bloque a un DataNode. Datos recibidos:");
		t_node *node = nodelist_find("NODO1");
		t_serial *serial = serial_pack("ii", 0, 0);
		t_packet request = protocol_packet(OP_REQUEST_BLOCK, serial);
		protocol_send_packet(request, node->socket);
		serial_destroy(serial);

		t_packet response = protocol_receive_packet(node->socket);
		printf("Received: %s\n", (char*)response.content->data);
		serial_destroy(response.content);
	}
}

static void cmd_format() {
	printf("!: El filesystem se va a formatear."
			" Continuar? \n"
			"[S]í. [N]o. \n");

	char* scan;
	scan = readline("> ");
	if (mstring_equali(scan, "S")) {
		if (nodelist_length() > 0) {

			dirtree_clear();
			filetable_clear();
			nodelist_format();

			printf("\nFilesystem formateado.\n");
			fs.formatted = true;
		}else{
			printf("\nNo hay nodos conectados. Aguardar la conexión de al menos un nodo.\n");
		}
	}
	free(scan);
}

static void cmd_help() {
	int printed = 0;

	for(int i = 0; commands[i].name; i++) {
		if((!*current_args || mstring_equal(current_args, commands[i].name))
				&& !mstring_isempty(commands[i].info)) {
			printf("%s\t\t%s.\n", commands[i].name, commands[i].info);
			printed++;
			if(*current_args) {
				show_usage_for(i);
				break;
			}
		}
	}

	if(!printed) command_not_found(current_args);
}

static void cmd_info() {
	if(num_args() != 1) show_usage();
	char *path = extract_arg(1);
	if(filetable_contains(path)) {
		filetable_info(path);
	} else {
		fprintf(stderr, "Error: archivo inexistente.\n");
	}
	free(path);
}

static void cmd_ls() {
	if(num_args() != 1) show_usage();
	char *path = extract_arg(1);
	if(!dirtree_contains(path)) {
		puts("Directorio inexistente.");
		free(path);
		return;
	}

	size_t nd = dirtree_count(path);
	size_t nf = filetable_count(path);
	printf("%zi directorio%s, %zi archivo%s%s\n", nd, nd == 1 ? "" : "s",
			nf, nf == 1 ? "" : "s", nf + nd == 0 ? "." : ":");

	dirtree_list(path);
	filetable_list(path);
	free(path);
}

static void cmd_md5() {
	puts("TODO");
}

static void cmd_mkdir() {
	if(num_args() != 1) show_usage();
	char *path = extract_arg(1);
	if(dirtree_contains(path)) {
		fprintf(stderr, "Error: ya existe el directorio.\n");
	} else {
		dirtree_add(path);
	}
	free(path);
}

static void cmd_mv() {
	if(num_args() != 2) show_usage();
	char *path = extract_arg(1);
	char *new_path = extract_arg(2);

	if(dirtree_contains(path)) {
		dirtree_move(path, new_path);
	} else if(filetable_contains(path)) {
		filetable_move(path, new_path);
	} else {
		fprintf(stderr, "Error: ruta inexistente.\n");
	}

	free(path);
	free(new_path);
}

static void cmd_quit() {
	should_quit = true;
}

static void cmd_rename() {
	if(num_args() != 2) show_usage();
	char *path = extract_arg(1);
	char *new_name = extract_arg(2);

	if(dirtree_contains(path)) {
		dirtree_rename(path, new_name);
	} else if(filetable_contains(path)) {
		filetable_rename(path, new_name);
	} else {
		fprintf(stderr, "Error: ruta inexistente.\n");
	}

	free(path);
	free(new_name);
}

static void cmd_rm() {
	int nargs = num_args();
	if(nargs == 0) show_usage();
	char *path = extract_arg(1);

	char mode = 'f';
	if(*path == '-' && mstring_length(path) == 2) {
		mode = *(path + 1);
	}

	switch(mode) {
	case 'f':
		if(nargs != 1) show_usage();
		if(filetable_contains(path)) {
			filetable_remove(path);
		} else {
			fprintf(stderr, "Error: archivo inexistente.\n");
		}
		break;
	case 'd':
		if(nargs != 2) show_usage();
		mstring_format(&path, "%s", extract_arg(2));
		if(!dirtree_contains(path)) {
			fprintf(stderr, "Error: directorio inexistente.\n");
		} else if(dirtree_count(path) + filetable_count(path) > 0) {
			fprintf(stderr, "Error: directorio no vacío.\n");
		} else {
			dirtree_remove(path);
		}
		break;
	case 'b':
		if(nargs != 4) show_usage();
		mstring_format(&path, "%s", extract_arg(2));
		puts("TODO");
		break;
	default:
		show_usage();
	}

	free(path);
}

// ========== Funciones para libreadline ==========

static void execute_line(const char *line) {
	char *cmd = (char*) line;
	char zero = '\0';
	char *args = &zero;

	char *space = strchr(line, ' ');
	if(space) {
		*space = '\0';
		args = space + 1;
		mstring_trim(args);
	}

	t_command *command = find_command(cmd);
	if(!command) {
		command_not_found(cmd);
		return;
	}

	// Lo comento para no tener que estar haciendo format a cada rato.
	// Cuando terminemos con los otros comandos lo descomentamos.
//	if(!fs.formatted
//			&& !mstring_equal(command->name, "format")
//			&& !mstring_equal(command->name, "debug")
//			&& !mstring_equal(command->name, "help")
//			&& !mstring_equal(command->name, "clear")
//			&& !mstring_equal(command->name, "quit")) {
//		printf("\nEl Filesystem no se encuentra formateado.\n"
//				"Para poder operar proceda a formatear con el comando <<format>>\n");
//		return;
//	}

	current_cmd = command - commands;
	current_args = args;
	(*(command->func))();
}

static t_command *find_command(const char *name) {
	for(int i = 0; commands[i].name; i++) {
		if(strcmp(name, commands[i].name) == 0) {
			return &commands[i];
		}
	}

	return NULL;
}

static void command_not_found(const char *name) {
	printf("No existe el comando '%s'. Comandos posibles:\n", name);

	for(int i = 0; commands[i].name; i++) {
		if(i > 0 && i % 6 == 0) printf("\n");
		printf("%s\t", commands[i].name);
	}

	printf("\n");
}

static char **rl_completion(const char *text, int start, int end) {
	return rl_completion_matches(text, &rl_generator);
}

static char *rl_generator(const char *text, int state) {
	static int list_index, len;
	char *name;

	if(!state) {
		list_index = 0;
		len = strlen(text);
	}

	while((name = commands[list_index++].name)) {
		if(strncmp(name, text, len) == 0)
			return strdup(name);
	}

	return NULL;
}
