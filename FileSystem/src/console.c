#include <file.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>
#include "mstring.h"

// ========== Estructuras ==========

typedef struct {
	char *name;
	void (*func)(char *);
	char *info;
	char *usage;
} t_command;

// ========== Declaraciones ==========

static void cmd_cat(char *);
static void cmd_cpblock(char *);
static void cmd_cpfrom(char *);
static void cmd_cpto(char *);
static void cmd_format(char *);
static void cmd_help(char *);
static void cmd_info(char *);
static void cmd_ls(char *);
static void cmd_md5(char *);
static void cmd_mkdir(char *);
static void cmd_mv(char *);
static void cmd_quit(char *);
static void cmd_rename(char *);
static void cmd_rm(char *);

static void init_console();
static void term_console();
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
		}

		free(line);
	}

	term_console();
}

// ========== Funciones privadas ==========

static void init_console() {
	history_file = mstring_format("%s/.history", file_userdir());
	read_history(history_file);
	rl_attempted_completion_function = rl_completion;
}

static void term_console() {
	write_history(history_file);
	free(history_file);
}

// ========== Funciones de comandos ==========

static void cmd_cat(char *args) {
	puts("TODO");
}

static void cmd_cpblock(char *args) {
	puts("TODO");
}

static void cmd_cpfrom(char *args) {
	puts("TODO");
}

static void cmd_cpto(char *args) {
	puts("TODO");
}

static void cmd_format(char *args) {
	puts("TODO");
}

static void cmd_help(char *args) {
	int printed = 0;

	for(int i = 0; commands[i].name; i++) {
		if(!*args || (strcmp(args, commands[i].name) == 0)) {
			printf("%s\t\t%s.\n", commands[i].name, commands[i].info);
			printed++;
			if(*args) {
				printf("Uso: %s %s\n", commands[i].name, commands[i].usage);
				break;
			}
		}
	}

	if(!printed) command_not_found(args);
}

static void cmd_info(char *args) {
	puts("TODO");
}

static void cmd_ls(char *args) {
	puts("TODO");
}

static void cmd_md5(char *args) {
	puts("TODO");
}

static void cmd_mkdir(char *args) {
	puts("TODO");
}

static void cmd_mv(char *args) {
	puts("TODO");
}

static void cmd_quit(char *args) {
	should_quit = true;
}

static void cmd_rename(char *args) {
	puts("TODO");
}

static void cmd_rm(char *args) {
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

	(*(command->func))(args);
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
	rl_attempted_completion_over = 1;
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
