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

#define FILE_PERMS 0664

char *fpath(int fd);
void checkfd(int fd, const char *path, const char *action);
void checkfp(FILE *fp, const char *path, const char *action);
void show_error_and_exit(int fd, const char *path, const char *action);
static t_file *open_file(const char *path, bool create);

// ========== Funciones públicas ==========

bool file_exists(const char *path) {
	char *upath = system_upath(path);
	bool exists = access(upath, F_OK) != -1;
	free(upath);
	return exists;
}

bool file_isdir(const char *path) {
	char *upath = system_upath(path);
	bool exists = access(upath, F_OK) != -1;
	struct stat s;
	stat(upath, &s);
	bool isdir = s.st_mode & S_IFDIR;
	free(upath);
	return exists && isdir;
}

bool file_istext(const char *path) {
	char *upath = system_upath(path);
	t_file *fd = fopen(upath, "r");
	size_t size = file_size(path);
	int scansize = 64;
	if(size < scansize) scansize = size;

	bool istext = true;
	while(scansize-- && istext) {
		if(fgetc(fd) == '\0') istext = false;
	}

	if(!istext) goto istext_end;

	char *cmd = mstring_create("file %s", upath);
	t_file *pd = popen(cmd, "r");
	free(cmd);
	checkfp(pd, upath, "verificar");
	char *output = file_readline(pd);
	pclose(pd);
	istext = mstring_contains(output, "text");
	free(output);
	istext_end:
	fclose(fd);
	free(upath);
	return istext;
}

void file_mkdir(const char *path) {
	char *upath = system_upath(path);
	if(access(upath, F_OK) != -1) {
		free(upath);
		return;
	}
	mode_t perms = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
	for(char *p = upath + 1; *p; p++) {
		if(*p == '/') {
			*p = '\0';
			mkdir(upath, perms);
			*p = '/';
		}
	}
	mkdir(upath, perms);
	free(upath);
}

size_t file_size(const char *path) {
	char *upath = system_upath(path);
	struct stat s;
	stat(upath, &s);
	free(upath);
	return s.st_size;
}

char *file_sizep(const char *path) {
	static const char *pref[] = {"", "K", "M", "G", "T"};
	const size_t size = file_size(path);
	const unsigned short mult = 1024;
	const char *x = size < mult ? "" : "i";
	float s = size;
	int i = 0;
	while(s >= mult) {
		i++;
		s /= mult;
	}
	return mstring_create("%.1f %s%sB", s, pref[i], x);
}

char *file_dir(const char *path) {
	char *upath = system_upath(path);
	char *slash = strrchr(upath, '/');
	if(slash) *slash = '\0';
	return upath;
}

const char *file_name(const char *path) {
	char *slash = strrchr(path, '/');
	return slash ? slash + 1 : path;
}

char *file_tmpname() {
	static bool seed = false;
	if(!seed) {
		srand(time(NULL));
		seed = true;
	}
	char *tmp = NULL, c;
	int r;
	do {
		mstring_empty(&tmp);
		while(strlen(tmp) < 32) {
			r = rand();
			c = r % 3 == 2 ? '0' + r % 10 : (r % 3 ? 'A' : 'a') + r % 26;
			mstring_format(&tmp, "%s%c", tmp, c);
		}
		mstring_format(&tmp, "tmp/%s", tmp);
	} while(file_exists(tmp));
	return tmp;
}

void file_create(const char *path) {
	char *upath = system_upath(path);
	char *dir = file_dir(upath);
	file_mkdir(dir);
	free(dir);

	int fd = open(upath, O_CREAT, FILE_PERMS);
	if(fd == -1) {
		fprintf(stderr, "Error al crear archivo %s: %s\n", upath, strerror(errno));
		free(upath);
		exit(EXIT_FAILURE);
	}
	free(upath);
	close(fd);
}

void file_recreate(const char *path) {
	file_delete(path);
	file_create(path);
}

void file_copy(const char *source, const char *target) {
	char *usource = system_upath(source);
	int fd_from = open(source, O_RDONLY);
	if(fd_from == -1) {
		free(usource);
		return;
	}

	char *utarget = system_upath(target);
	if(file_isdir(utarget)) {
		mstring_format(&utarget, "%s/%s", utarget, file_name(source));
	}

	int fd_to = open(utarget, O_WRONLY | O_CREAT | O_TRUNC, FILE_PERMS);
	if(fd_to == -1) goto exit;

	char buffer[4096];
	ssize_t nread;
	while(nread = read(fd_from, buffer, sizeof buffer), nread > 0) {
		char *p = buffer;
		ssize_t nwritten;
		do {
			nwritten = write(fd_to, p, nread);
			if(nwritten == -1 && errno != EINTR) goto exit;
			nread -= nwritten;
			p += nwritten;
		} while(nread > 0);
	}

	exit:
	free(usource);
	free(utarget);
	close(fd_from);
	if(fd_to != -1) close(fd_to);
}

void file_delete(const char *path) {
	char *upath = system_upath(path);
	remove(upath);
	free(upath);
}

t_file *file_new(const char *path) {
	return open_file(path, true);
}

t_file *file_open(const char *path) {
	return open_file(path, false);
}

char *file_path(t_file *file) {
	return fpath(fileno(file));
}

char *file_readline(t_file *file) {
	if(file == NULL) return NULL;
	char *line = NULL;
	size_t len = 0;
	ssize_t read = getline(&line, &len, file);
	if(read == -1) {
		if(errno != 0) show_error_and_exit(fileno(file), NULL, "leer");
		return NULL;
	}
	if(line[read - 1] == '\n') {
		line[read - 1] = '\0';
	}
	return line;
}

char *file_readlines(t_file *file) {
	if(file == NULL) return NULL;
	size_t len = 64;
	char *line = malloc(64);
	char *lines = strdup("");
	while(getline(&line, &len, file) != -1) {
		char *tmp = mstring_create("%s%s", lines, line);
		free(lines);
		lines = tmp;
	}
	free(line);
	if(errno != 0) show_error_and_exit(fileno(file), NULL, "leer");
	return lines;
}

void file_writeline(const char *line, t_file *file) {
	fputs(line, file);
	if(line[strlen(line)-1] != '\n') {
		fputs("\n", file);
	}
}

void file_truncate(const char *path, size_t size) {
	char *upath = system_upath(path);
	int fd = open(upath, O_CREAT | O_WRONLY, FILE_PERMS);
	free(upath);
	ftruncate(fd, size);
	close(fd);
}

void file_merge(mlist_t *sources, const char *target) {
	typedef struct {
		t_file *file;
		char *line;
	} t_cont;
	t_cont *map_cont(const char *source) {
		t_cont *cont = malloc(sizeof(cont));
		cont->file = file_open(source);
		cont->line = NULL;
		return cont;
	}
	bool line_set(t_cont *cont) {
		return cont->line != NULL;
	}
	void read_file(t_cont *cont) {
		if(!line_set(cont)) {
			char *aux = file_readline(cont->file);
			free(cont->line);
			cont->line = aux;
		}
	}
	bool compare_lines(t_cont *cont1, t_cont *cont2) {
		return mstring_asc(cont1->line, cont2->line);
	}
	mlist_t *files = mlist_map(sources, map_cont);
	t_file *output = file_new(target);
	mlist_t *list = mlist_copy(files);
	while(true) {
		mlist_traverse(list, read_file);
		mlist_t *aux = mlist_filter(list, line_set);
		mlist_destroy(list, NULL);
		list = aux;
		if(mlist_length(list) == 0) break;
		mlist_sort(list, compare_lines);
		t_cont *cont = mlist_first(list);
		file_writeline(cont->line, output);
		free(cont->line);
		cont->line = NULL;
	}
	void free_cont(t_cont *cont) {
		file_close(cont->file);
		free(cont);
	}
	mlist_destroy(files, free_cont);
}

t_fmap *file_map(const char *path) {
	int fd = open(path, O_RDWR);
	checkfd(fd, path, "abrir");
	size_t size = file_size(path);
	void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(addr == MAP_FAILED) {
		show_error_and_exit(fd, path, "mapear a memoria");
	}
	t_fmap *map = malloc(sizeof(t_fmap));
	map->data = addr;
	map->size = size;
	return map;
}

void file_unmap(t_fmap *map) {
	munmap(map->data, map->size);
	free(map);
}

void file_close(t_file *file) {
	if(file != NULL) {
		fclose(file);
	}
}

// ========== Funciones privadas ==========

char *fpath(int fd) {
	char link[PATH_MAX], path[PATH_MAX];
	snprintf(link, PATH_MAX, "/proc/self/fd/%i", fd);
	readlink(link, path, PATH_MAX);
	return strdup(path);
}

void checkfd(int fd, const char *path, const char *action) {
	if(fd == -1) {
		show_error_and_exit(fd, path, action);
	}
}

void checkfp(FILE *fp, const char *path, const char *action) {
	if(fp == NULL) {
		show_error_and_exit(fileno(fp), path, action);
	}
}

void show_error_and_exit(int fd, const char *path, const char *action) {
	char *ppath = (char*) path;
	if(ppath == NULL) ppath = fpath(fd);
	log_report("Error al %s archivo %s: %s", action, ppath, strerror(errno));
	free(ppath);
	exit(EXIT_FAILURE);
}

static t_file *open_file(const char *path, bool create) {
	char *upath = system_upath(path);
	t_file *file = fopen(upath, create ? "w+" : "r+");
	checkfp(file, upath, "abrir");
	free(upath);
	return file;
}

