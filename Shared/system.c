#include <file.h>
#include <limits.h>
#include <mstring.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
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
		char *execdir = file_dir(system_proc());
		char *path = mstring_format("%s/../..", execdir);
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

void system_init() {
	file_mkdir(system_userdir());

	char *confdir = mstring_format("%s/config", system_userdir());
	file_mkdir(confdir);
	free(confdir);

	char *logdir = mstring_format("%s/logs", system_userdir());
	file_mkdir(logdir);
	free(logdir);
}

const char *system_proc() {
	static char proc[PATH_MAX] = {0};
	if(!*proc) {
		readlink("/proc/self/exe", proc, PATH_MAX);
	}
	return proc;
}
