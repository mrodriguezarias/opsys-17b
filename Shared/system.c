#include <limits.h>
#include <mstring.h>
#include <path.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include "system.h"

const char *system_homedir() {
	char *homedir;
	if((homedir = getenv("HOME")) == NULL) {
		homedir = getpwuid(getuid())->pw_dir;
	}
	return homedir;
}

const char *system_basedir() {
	static char dir[PATH_MAX] = {0};
	if(!*dir) {
		char *execdir = path_dir(system_proc());
		char *path = mstring_create("%s/../..", execdir);
		free(execdir);
		realpath(path, dir);
		free(path);
	}
	return dir;
}

const char *system_userdir() {
	static char dir[PATH_MAX] = {0};
	if(!*dir) {
		snprintf(dir, PATH_MAX, "%s/yatpos", system_homedir());
	}
	return dir;
}

const char *system_rscdir() {
	static char dir[PATH_MAX] = {0};
	if(!*dir) {
		snprintf(dir, PATH_MAX, "%s/Shared/rsc", system_basedir());
	}
	return dir;
}

char *system_upath(const char *path) {
	char *upath = mstring_duplicate(path);
	if(*path != '/' && !mstring_hasprefix(path, "yamafs:")) {
		mstring_format(&upath, "%s/%s", system_userdir(), upath);
	}
	return upath;
}

char *system_lpath(const char *path) {
	char *lpath = mstring_duplicate(path);
	if(*path != '/') {
		mstring_format(&lpath, "%s/%s", system_cwd(), lpath);
	}
	return lpath;
}

void system_init() {
	path_mkdir(system_userdir());
	char *dirs[] = {"config", "logs", "tmp", "metadata/archivos", "metadata/bitmaps", NULL};

	for(char **dir = dirs; *dir != NULL; dir++) {
		char *path = mstring_create("%s/%s", system_userdir(), *dir);
		path_mkdir(path);
		free(path);
	}
}

const char *system_proc() {
	static char proc[PATH_MAX] = {0};
	if(!*proc) {
		readlink("/proc/self/exe", proc, PATH_MAX);
	}
	return proc;
}

const char *system_cwd() {
	static char cwd[PATH_MAX] = {0};
	if(!*cwd) {
		getcwd(cwd, PATH_MAX);
	}
	return cwd;
}

void system_exit(const char *error, ...) {
	if(mstring_isempty(error)) exit(EXIT_SUCCESS);

	char *err = mstring_duplicate(error);
	char *end = mstring_end(err);
	if(*end != '\n') mstring_format(&err, "%s\n", err);

	va_list args;
	va_start(args, error);
	vfprintf(stderr, err, args);
	va_end(args);

	free(err);
	exit(EXIT_FAILURE);
}
