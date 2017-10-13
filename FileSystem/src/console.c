#include <mstring.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system.h>

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
		{"cpblock", cmd_cpblock, "Copia un bloque de un archivo en un nodo", "archivo bloque nodo"},
		{"cpfrom", cmd_cpfrom, "Copia un archivo local a yamafs", "archivo_local directorio_yamafs"},
		{"cpto", cmd_cpto, "Copia un archivo yamafs al sistema local", "archivo_yamafs directorio_local"},
		{"debug", cmd_debug, "Comando para depuración", "nodes | dirs"},
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
			execute_line(line);
			write_history(history_file);
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
	puts("TODO");
}

static void cmd_cpblock() {
	puts("TODO");
}

static void cmd_cpfrom() {
	if(num_args() != 2) show_usage();
	char *source_path = extract_arg(1);
	char *yama_dir = extract_arg(2);

	yfile_cpfrom(source_path, yama_dir);
	free(source_path);
	free(yama_dir);
}

static void cmd_cpto() {
	puts("TODO");
}

static void cmd_debug() {
	if(mstring_equal(current_args, "nodes")) {
		nodelist_print();
	} else if(mstring_equal(current_args, "dirs")) {
		dirtree_print();
	} else if(mstring_equal(current_args, "files")) {
		filetable_print();
	}
}

static void cmd_format() {
	fs.formatted = true;
}

static void cmd_help() {
	int printed = 0;

	for(int i = 0; commands[i].name; i++) {
		if(!*current_args || (strcmp(current_args, commands[i].name) == 0)) {
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
	puts("TODO");
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

	dirtree_ls(path);
	filetable_ls(path);
	free(path);
}

static void cmd_md5() {
	puts("TODO");
}

static void cmd_mkdir() {
	puts("TODO");
}

static void cmd_mv() {
	puts("TODO");
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
}

static void cmd_rm() {
	puts("TODO");
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
