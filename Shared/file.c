#include "file.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <libgen.h>
#include <errno.h>
#include <mstring.h>
#include <system.h>
#include <log.h>
#include <time.h>
#include <sys/mman.h>
#include <path.h>

struct file {
	FILE *fp;
	char *path;
	t_ftype type;
};

static char *fpath(int fd);
static void show_error_and_exit(const char *path, const char *action);
static t_file *create_file(const char *path, bool trunc);
static char *get_line(FILE *fp, char *line);

// ========== Funciones públicas ==========

t_file *file_create(const char *path) {
	return create_file(path, true);
}

t_file *file_open(const char *path) {
	return create_file(path, false);
}

const char *file_path(t_file *file) {
	return file->path;
}

t_ftype file_type(t_file *file) {
	return file->type;
}

size_t file_size(t_file *file) {
	return path_size(file->path);
}

FILE *file_pointer(t_file *file) {
	return file->fp;
}

void file_rewind(t_file *file) {
	if(file != NULL) {
		rewind(file->fp);
	}
}

char *file_readline(t_file *file) {
	if(file == NULL) return NULL;
	return get_line(file->fp, NULL);
}

void file_writeline(t_file *file, const char *line) {
	if(file == NULL) return;
	fputs(line, file->fp);
	if(*mstring_end(line) != '\n') {
		fputs("\n", file->fp);
	}
}

void file_ltraverse(t_file *file, void (*routine)(const char *line)) {
	if(file == NULL) return;
	rewind(file->fp);
	char *line = mstring_empty(NULL);
	while(line = get_line(file->fp, line), line != NULL) {
		routine(line);
	}
	free(line);
}

void file_btraverse(t_file *file, void (*routine)(const void *block, size_t size)) {
	if(file == NULL) return;
	rewind(file->fp);
	size_t cap = 4096;
	char *buf = alloca(cap);
	size_t read = 0;

	while(read = fread(buf, 1, cap, file->fp), read > 0) {
		routine(buf, read);
	}
}

void file_clear(t_file *file) {
	ftruncate(fileno(file->fp), 0);
	rewind(file->fp);
}

void *file_map(t_file *file) {
	void *map = mmap(NULL, file_size(file), PROT_READ | PROT_WRITE, MAP_SHARED, fileno(file->fp), 0);
	if(map == MAP_FAILED) {
		show_error_and_exit(file->path, "mapear a memoria");
	}
	return map;
}

void file_sync(t_file *file, void *map) {
	msync(map, file_size(file), MS_SYNC);
}

void file_unmap(t_file *file, void *map) {
	file_sync(file, map);
	munmap(map, file_size(file));
}

void file_close(t_file *file) {
	if(file == NULL) return;
	fclose(file->fp);
	free(file->path);
}

// ========== Funciones privadas ==========

static char *fpath(int fd) {
	char link[PATH_MAX], path[PATH_MAX];
	snprintf(link, PATH_MAX, "/proc/self/fd/%i", fd);
	readlink(link, path, PATH_MAX);
	return strdup(path);
}

static void show_error_and_exit(const char *path, const char *action) {
	system_exit("Error al %s archivo %s: %s", action, path, strerror(errno));
}

static t_file *create_file(const char *path, bool trunc) {
	if(path_isdir(path)) return NULL;
	path_mkfile(path);

	char *upath = system_upath(path);
	FILE *fp = fopen(upath, trunc ? "w+" : "r+");
	if(fp == NULL) show_error_and_exit(upath, "abrir");

	t_file *file = malloc(sizeof(t_file));
	file->fp = fp;
	file->path = upath;
	file->type = path_istext(path) ? FTYPE_TXT : FTYPE_BIN;
	return file;
}

static char *get_line(FILE *fp, char *line) {
	size_t len = 0;
	int r = getline(&line, &len, fp);
	if(r == -1) {
		line = NULL;
		if(!feof(fp) && errno != 0)
			show_error_and_exit(fpath(fileno(fp)), "leer línea de");
	}
	if(line != NULL) {
		char *end = mstring_end(line);
		if(*end == '\n') *end = '\0';
	}
	return line;
}
