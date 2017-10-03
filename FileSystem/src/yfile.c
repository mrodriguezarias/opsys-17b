#include "yfile.h"

#include <file.h>
#include <mlist.h>
#include <path.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <mstring.h>
#include <system.h>

#include "dirtree.h"

static const char *files_dir(void);
static char *real_path(const char *yama_path);

// ========== Funciones públicas ==========

struct yfile {
	char *path;
	size_t size;
	t_ftype type;
	mlist_t *blocks;
};

t_yfile *yfile_create(const char *path, t_ftype type) {
	char *pdir = path_dir(path);
	dirtree_add(pdir);
	free(pdir);

	char *rpath = real_path(path);
	path_create(rpath);
	free(rpath);

	t_yfile *file = malloc(sizeof(t_yfile));
	file->path = mstring_duplicate(path);
	file->size = 0;
	file->type = type;
	file->blocks = mlist_create();
	return file;
}

void yfile_cpfrom(const char *file_path, const char *dir_path) {
	dirtree_add(dir_path);

}

// ========== Funciones privadas ==========

static const char *files_dir() {
	static char *fdir = NULL;
	if(fdir == NULL) {
		fdir = mstring_create("%s/metadata/archivos", system_userdir());
	}
	return fdir;
}

static char *real_path(const char *yama_path) {
	char *pdir = path_dir(yama_path);
	t_directory *dir = dirtree_find(pdir);
	free(pdir);
	if(dir == NULL) return NULL;
	return mstring_create("%s/%d/%s", files_dir(), dir->index, path_name(yama_path));
}
