#define _XOPEN_SOURCE 500
#include <openssl/md5.h>
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <mstring.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <system.h>
#include <stdarg.h>
#define __USE_BSD
#include <dirent.h>
#include <file.h>
#include <unistd.h>
#include <data.h>
#ifndef __USE_XOPEN_EXTENDED
#define __USE_XOPEN_EXTENDED
#endif
#include <ftw.h>
#include "path.h"

#define FILE_PERMS 0664

static void show_error_and_exit(const char *path, const char *action);
static int remove_routine(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
static size_t size_of_file(const char *path);
static void traverse_dir(const char *dpath, void (*fn)(struct dirent *entry));

// ========== Funciones públicas ==========

char *_path_create(t_ptype type, const char *scount, ...) {
	va_list ap;
	va_start(ap, scount);
	int count = mstring_isempty(scount) ? 0 : mstring_count(scount, ",") + 1;
	char *root = va_arg(ap, char*);
	char *path = mstring_duplicate(root);
	mstring_replace(&path, "yamafs:", "");
	if(mstring_hasprefix(path, "./")) {
		mstring_format(&path, "%s", path + 2);
	}

	if(type != PTYPE_YAMA) {
		mstring_replace(&path, "~", system_homedir());
		char *rpath = realpath(path, NULL);
		if(rpath != NULL) {
			free(path);
			path = rpath;
		}
	}

	if(*path != '/') {
		switch(type) {
		case PTYPE_YATPOS: mstring_format(&path, "%s/%s", system_userdir(), path); break;
		case PTYPE_USER: mstring_format(&path, "%s/%s", system_cwd(), path); break;
		case PTYPE_YAMA: mstring_format(&path, "/%s", path); break;
		}
	}
	if(type == PTYPE_YAMA) mstring_format(&path, "yamafs:%s", path);

	while(--count) {
		char *end = mstring_end(path);
		if(*end == '/') *end = '\0';

		char *cur = va_arg(ap, char*);
		if(*cur == '/') cur++;
		mstring_format(&path, "%s/%s", path, cur);
	}
	va_end(ap);
	return path;
}

bool path_exists(const char *path) {
	char *upath = system_upath(path);
	bool exists = access(upath, F_OK) != -1;
	free(upath);
	return exists;
}

bool path_isdir(const char *path) {
	char *upath = system_upath(path);
	bool exists = access(upath, F_OK) != -1;
	struct stat s;
	stat(upath, &s);
	bool isdir = s.st_mode & S_IFDIR;
	free(upath);
	return exists && isdir;
}

bool path_isfile(const char *path) {
	return path_exists(path) && !path_isdir(path);
}

bool path_istext(const char *path) {
	if(!path_isfile(path)) return false;

	char *upath = system_upath(path);
	FILE *fd = fopen(upath, "r");
	size_t size = path_size(path);
	int scansize = 64;
	if(size < scansize) scansize = size;

	bool istext = true;
	while(scansize-- && istext) {
		if(fgetc(fd) == '\0') istext = false;
	}

	if(!istext) goto istext_end;

	char *cmd = mstring_create("file %s", upath);
	FILE *pd = popen(cmd, "r");
	free(cmd);
	if(pd == NULL) show_error_and_exit(upath, "verificar");
	char *output = mstring_buffer();
	fgets(output, MSTRING_MAXSIZE, pd);
	pclose(pd);

	istext = mstring_contains(output, "text");
	istext_end:
	fclose(fd);
	free(upath);
	return istext;
}

bool path_isbin(const char *path) {
	return path_isfile(path) && !path_istext(path);
}

bool path_isempty(const char *path) {
	if(!path_isdir(path)) {
		return path_size(path) == 0;
	}
	int count = 0;
	void fn(struct dirent *entry) {
		count++;
	}
	traverse_dir(path, fn);
	return count == 0;
}

bool path_equal(const char *path1, const char *path2) {
	char *p1 = mstring_duplicate(path1);
	char *p2 = mstring_duplicate(path2);
	if(mstring_hassuffix(p1, "/")) *mstring_end(p1) = '\0';
	if(mstring_hassuffix(p2, "/")) *mstring_end(p2) = '\0';
	bool r = mstring_equali(p1, p2);
	free(p1);
	free(p2);
	return r;
}

void path_mkdir(const char *path) {
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

size_t path_size(const char *path) {
	char *upath = system_upath(path);
	size_t size = 0;

	if(!path_isdir(path)) {
		size = size_of_file(upath);
		free(upath);
		return size;
	}

	int routine(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
		size += size_of_file(fpath);
		return 0;
	}

	nftw(upath, routine, 64, FTW_PHYS);
	return size;
}

char *path_sizep(const char *path) {
	return mstring_bsize(path_size(path));
}

char *path_dir(const char *path) {
	char *upath = system_upath(path);

	char *end = mstring_end(upath);
	if(*end == '/') {
		if(mstring_count(upath, "/") == 1) return upath;
		*end = '\0';
	}

	char *slash = strrchr(upath, '/');
	if(strchr(upath, '/') == slash) slash++;
	if(slash) *slash = '\0';
	return upath;
}

const char *path_name(const char *path) {
	char *slash = strrchr(path, '/');
	return slash ? slash + 1 : path;
}

char *path_temp() {
	char *tmp = NULL, c;
	int r;
	do {
		mstring_empty(&tmp);
		while(strlen(tmp) < 32) {
			r = system_rand();
			c = r % 3 == 2 ? '0' + r % 10 : (r % 3 ? 'A' : 'a') + r % 26;
			mstring_format(&tmp, "%s%c", tmp, c);
		}
		mstring_format(&tmp, "tmp/%s", tmp);
	} while(path_exists(tmp));
	return tmp;
}

void path_mkfile(const char *path) {
	char *upath = system_upath(path);
	char *dir = path_dir(upath);
	path_mkdir(dir);
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

void path_copy(const char *source, const char *target) {
	char *usource = system_upath(source);
	int fd_from = open(source, O_RDONLY);
	if(fd_from == -1) {
		free(usource);
		return;
	}

	char *utarget = system_upath(target);
	if(path_isdir(utarget)) {
		mstring_format(&utarget, "%s/%s", utarget, path_name(source));
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

void path_move(const char *source, const char *target) {
	path_copy(source, target);
	path_remove(source);
}

void path_remove(const char *path) {
	if(!path_exists(path)) return;
	char *upath = system_upath(path);
	if(path_isdir(path)) {
		int r = nftw(upath, remove_routine, 64, FTW_DEPTH | FTW_PHYS);
		if(r == -1) show_error_and_exit(upath, "remover");
	} else {
		unlink(upath);
	}
	free(upath);
}

void path_files(const char *dpath, void (*routine)(const char *path)) {
	void fn(struct dirent *entry) {
		if(entry->d_type != DT_REG) return;
		char *path = path_create(PTYPE_YATPOS, dpath, entry->d_name);
		routine(path);
		free(path);
	}
	traverse_dir(dpath, fn);
}

char *path_md5(const char *path) {
	char *upath = system_upath(path);
	FILE *fp = fopen(upath, "r");
	if(fp == NULL) show_error_and_exit(upath, "abrir");
	free(upath);

	size_t length = 4096;
	char *buffer = alloca(length);
	int bytes = 0;

	MD5_CTX ctx;
	MD5_Init(&ctx);
	while(bytes = fread(buffer, 1, length, fp), bytes != 0) {
		MD5_Update(&ctx, buffer, bytes);
	}

	unsigned char hash[MD5_DIGEST_LENGTH];
	MD5_Final(hash, &ctx);
	fclose(fp);

	char *md5 = mstring_empty(NULL);
	for(int i = 0; i < MD5_DIGEST_LENGTH; i++) {
		mstring_format(&md5, "%s%02x", md5, hash[i]);
	}
	return md5;
}

void path_truncate(const char *path, size_t size) {
	path_mkfile(path);
	char *upath = system_upath(path);
	truncate(upath, size);
	free(upath);
}

void path_sort(const char *path) {
	if(!path_istext(path)) return;
	t_file *file = file_open(path);

	mlist_t *lines = mlist_create();
	bool reader(const char *line) {
		mlist_append(lines, mstring_create("%s", line));
		return true;
	}
	file_ltraverse(file, reader);

	mlist_sort(lines, mstring_asc);
	file_clear(file);

	void writer(const char *line) {
		file_writeline(file, line);
	}
	mlist_traverse(lines, writer);
	mlist_destroy(lines, free);
	file_close(file);
}

void path_merge(mlist_t *sources, const char *target) {
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

	t_file *output = file_create(target);
	mlist_t *list = mlist_copy(files);
	mlist_traverse(list, read_file);
	printf("CANTIDAD DE ARCHIVOS A MERGIAR: files:%d, list:%d",mlist_length(files),mlist_length(list));
	while(true) {
		mlist_traverse(list, read_file);
		mlist_t *aux = mlist_filter(list, line_set);
		mlist_destroy(list, NULL);
		list = aux;
		if(mlist_length(list) == 0) break;
		mlist_sort(list, compare_lines);
		t_cont *cont = mlist_first(list);
		file_writeline(output, cont->line);
		free(cont->line);
		cont->line = NULL;
	}
	void free_cont(t_cont *cont) {
		file_close(cont->file);
		free(cont);
	}
	mlist_destroy(files, free_cont);
	file_close(output);
}

bool path_reduce(const char *input, const char *script, const char *output) {
	char *scrpath = path_create(PTYPE_USER, script);
	if(!path_exists(scrpath)) {
		free(scrpath);
		return false;
	}

	char *inpath = path_create(PTYPE_YATPOS, input);
	char *outpath = path_create(PTYPE_YATPOS, output);

	char *command = mstring_create("cat %s | %s > %s", inpath, scrpath, outpath);
	int r = system(command);

	free(inpath);
	free(outpath);
	free(command);
	free(scrpath);
	return r == 0;
}

// ========== Funciones privadas ==========

static void show_error_and_exit(const char *path, const char *action) {
	system_exit("Error al %s ruta %s%s%s", action, path, errno == 0 ? "." : ": ", errno == 0 ? "" : strerror(errno));
}

static int remove_routine(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
	int r = remove(fpath);
	if(r == -1) show_error_and_exit(fpath, "remover");
	return r;
}

static size_t size_of_file(const char *path) {
	struct stat s;
	stat(path, &s);
	return s.st_size;
}

static void traverse_dir(const char *dpath, void (*fn)(struct dirent *entry)) {
	char *upath = system_upath(dpath);
	DIR *dir = opendir(upath);
	if(dir == NULL) show_error_and_exit(upath, "leer directorio");

	struct dirent *entry;
	while(entry = readdir(dir), entry != NULL) {
		if(mstring_equal(entry->d_name, ".") || mstring_equal(entry->d_name, "..")) continue;
		fn(entry);
	}
	closedir(dir);
}
