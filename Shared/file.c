#include "file.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <limits.h>
#include <sys/stat.h>
#include <string.h>
#include <mstring.h>
#include <fcntl.h>
#include <libgen.h>
#include <errno.h>

#include <stdio.h>

// ========== Directorios del sistema ==========

const char *file_homedir() {
	char *homedir;
	if((homedir = getenv("HOME")) == NULL) {
		homedir = getpwuid(getuid())->pw_dir;
	}
	return homedir;
}

const char *file_sysdir() {
	static char dir[PATH_MAX] = {0};
	if(!*dir) {
		char *execdir = file_dir(file_proc());
		char *path = mstring_format("%s/../..", execdir);
		free(execdir);
		realpath(path, dir);
		free(path);
	}
	return dir;
}

const char *file_userdir() {
	static char dir[PATH_MAX] = {0};
	if(!*dir) {
		snprintf(dir, PATH_MAX, "%s/yatpos", file_homedir());
	}
	return dir;
}

const char *file_rscdir() {
	static char dir[PATH_MAX] = {0};
	if(!*dir) {
		snprintf(dir, PATH_MAX, "%s/Shared/rsc", file_sysdir());
	}
	return dir;
}

void file_create_sysdirs() {
	file_mkdir(file_userdir());

	char *confdir = mstring_format("%s/config", file_userdir());
	file_mkdir(confdir);
	free(confdir);

	char *logdir = mstring_format("%s/logs", file_userdir());
	file_mkdir(logdir);
	free(logdir);
}

// ========== Funciones de archivos ==========

const char *file_proc() {
	static char proc[PATH_MAX] = {0};
	if(!*proc) {
		readlink("/proc/self/exe", proc, PATH_MAX);
	}
	return proc;
}

bool file_exists(const char *path) {
	return access(path, F_OK) != -1;
}

bool file_isdir(const char *path) {
	struct stat s;
	stat(path, &s);
	return s.st_mode & S_IFDIR;
}

void file_mkdir(const char *path) {
	if(file_exists(path)) return;
	mode_t perms = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;

	char mpath[PATH_MAX];
	strncpy(mpath, path, PATH_MAX);

	for(char *p = mpath + 1; *p; p++) {
		if(*p == '/') {
			*p = '\0';
			mkdir(mpath, perms);
			*p = '/';
		}
	}

	mkdir(mpath, perms);
}

size_t file_size(const char *path) {
	struct stat s;
	stat(path, &s);
	return s.st_size;
}

char *file_dir(const char *path) {
	char *dir = strdup(path);
	char *slash = strrchr(dir, '/');
	if(slash) *slash = '\0';
	return dir;
}

const char *file_name(const char *path) {
	char *slash = strrchr(path, '/');
	return slash ? slash + 1 : path;
}

void file_copy(const char *source, const char *target) {
	int fd_from = open(source, O_RDONLY);
	if(fd_from == -1) return;

	char path[PATH_MAX] = {0};
	if(file_isdir(target)) {
		snprintf(path, PATH_MAX, "%s/%s", target, file_name(source));
	} else {
		strncpy(path, target, PATH_MAX);
	}

	int fd_to = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0664);
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
	close(fd_from);
	if(fd_to != -1) close(fd_to);
}

