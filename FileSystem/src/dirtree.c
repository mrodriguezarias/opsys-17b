#include "dirtree.h"
#include <path.h>
#include <file.h>
#include <mstring.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <system.h>
#include <mlist.h>

#define DAT_PATH "metadata/directorios.dat"
#define MAX_SIZE 100

#define dir_exists(dir) ((dir) != NULL && (dir)->parent >= -1)

static t_directory dirs[MAX_SIZE];
static t_directory *root = dirs;

static t_file *file = NULL;
static void *map = NULL;

typedef struct {
	char *name;
	int depth;
	mlist_t *children;
} t_print_child;

typedef struct {
	char *bar;
	mlist_t *children;
} t_print_level;

static const char *files_dir(void);
static char *path_to_files(t_directory *dir);
static mlist_t *children_of_dir(t_directory *parent);
static void get_children(mlist_t *children, int index, int depth);
static t_directory *add_directory(const char *name, int parent);
static char *create_normal_path(const char *path);
static t_directory *find_dir_by_index(int index);
static t_directory *find_dir_by_name(const char *name, int parent);
static void remove_children(t_directory *dir);
static void remove_directory(t_directory *dir);
static void map_file(void);
static void save_to_file(void);

// ========== Funciones públicas ==========

void dirtree_init() {
	if(path_exists(DAT_PATH)) {
		map_file();
		memcpy(dirs, map, sizeof(t_directory) * MAX_SIZE);
		return;
	}

	for(int i = 0; i < MAX_SIZE; i++) {
		t_directory *dir = dirs + i;
		dir->index = i;
		dir->parent = -2;
	}

	add_directory("/", -1);
	save_to_file();
}

int dirtree_size() {
	int size = 0;
	for(t_directory *dir = dirs; dir < dirs + MAX_SIZE; dir++) {
		if(dir_exists(dir)) size++;
	}
	return size;
}

t_directory *dirtree_add(const char *path) {
	if(dir_exists(dirs + MAX_SIZE -1)) return NULL;
	if(mstring_equal(path, root->name)) return root;

	t_directory *dir = NULL;
	char *npath = create_normal_path(path);
	char *p = npath + 1;

	char *name = p;
	char *par_name = root->name;
	int par_parent = root->parent;

	for(; *p; p++) {
		if(*p != '/') continue;
		*p = '\0';
		t_directory *parent = find_dir_by_name(par_name, par_parent);
		dir = add_directory(name, parent->index);
		par_name = name;
		par_parent = parent->index;
		name = p + 1;
	}

	free(npath);
	save_to_file();
	return dir;
}

t_directory *dirtree_find(const char *path) {
	t_directory *found = NULL;
	bool exists = true;

	char *ypath = path_create(PTYPE_YAMA, path);
	char *npath = create_normal_path(ypath);
	free(ypath);

	char *p = npath;
	char *name = root->name;
	int par = root->parent;

	for(; *p; p++) {
		if(*p != '/') continue;
		*p = '\0';
		t_directory *dir = find_dir_by_name(name, par);
		if(!dir_exists(dir)) {
			exists = false;
			break;
		}
		found = dir;
		name = p + 1;
		par = dir->index;
	}

	free(npath);
	return exists ? found : NULL;
}

bool dirtree_contains(const char *path) {
	return dirtree_find(path) != NULL;
}

void dirtree_traverse(void (*routine)(t_directory *dir)) {
	for(t_directory *cur = dirs; cur < dirs + MAX_SIZE; cur++) {
		if(dir_exists(cur)) routine(cur);
	}
}

void dirtree_move(const char *path, const char *new_path) {
	t_directory *dir = dirtree_find(path);
	if(dir == NULL || dir == root) return;

	char *npath = create_normal_path(new_path);
	*mstring_end(npath) = '\0';

	if(!mstring_hassuffix(npath, dir->name)) {
		mstring_format(&npath, "%s/%s", npath, dir->name);
	}

	if(dirtree_contains(npath)) {
		free(npath);
		return;
	}

	char *dpath = path_dir(npath);
	free(npath);

	t_directory *parent = dirtree_add(dpath);
	free(dpath);

	dir->parent = parent->index;
	save_to_file();
}

void dirtree_rename(const char *path, const char *new_name) {
	t_directory *dir = dirtree_find(path);
	if(dir == NULL || dir == root) return;

	char *npath = create_normal_path(path);
	*mstring_end(npath) = '\0';
	*strrchr(npath, '/') = '\0';

	mstring_format(&npath, "%s/%s", npath, new_name);
	bool exists = dirtree_contains(npath);
	free(npath);

	if(exists) return;
	strcpy(dir->name, new_name);
	save_to_file();
}

void dirtree_remove(const char *path) {
	t_directory *dir = dirtree_find(path);
	remove_children(dir);
	remove_directory(dir);
	save_to_file();
}

void dirtree_clear() {
	for(t_directory *dir = dirs; dir < dirs + MAX_SIZE; dir++) {
		remove_directory(dir);
	}
	save_to_file();
}

void dirtree_print() {
	mlist_t *children = mlist_create();
	printf("/\n");
	get_children(children, 0, 0);

	bool sorter(t_print_child *child1, t_print_child *child2) {
		return mstring_asc(child1->name, child2->name);
	}

	char *corner = "└";
	char *nocorner = "├";
	char *bar = "│";
	char *nobar = " ";

	int prev_depth = 0;
	mlist_t *levels = mlist_create();

	void add_level(mlist_t *children) {
		t_print_level *level = malloc(sizeof(t_print_level));
		level->bar = bar;
		level->children = children;
		mlist_append(levels, level);
	}

	add_level(children);

	void printer(t_print_child *child) {
		bool cond(t_print_child *elem) { return elem == child; }
		bool has_children = mlist_length(child->children) > 0;
		char *arm = nocorner;
		for(int i = 0; i < prev_depth - child->depth; i++) {
			t_print_level *level = mlist_pop(levels, mlist_length(levels) - 1);
			free(level);
		}
		t_print_level *cur = mlist_last(levels);
		if(mlist_index(cur->children, cond) == mlist_length(cur->children) - 1) {
			arm = corner;
			if(has_children) {
				t_print_level *level = mlist_get(levels, child->depth);
				level->bar = nobar;
			}
		}

		if(has_children) {
			add_level(child->children);
		}

		char *spaces = mstring_empty(NULL);
		for(int i = 0; i < child->depth; i++) {
			t_print_level *level = mlist_get(levels, i);
			mstring_format(&spaces, "%s%s   ", spaces, level->bar);
		}

		printf("%s%s── %s\n", spaces, arm, child->name);
		free(spaces);

		prev_depth = child->depth;
		mlist_sort(child->children, sorter);
		mlist_traverse(child->children, printer);
	}

	mlist_sort(children, sorter);
	mlist_traverse(children, printer);

	mlist_destroy(levels, free);

	void free_children(t_print_child *child) {
		if(child->children == NULL) return;
		mlist_traverse(child->children, free_children);
		child->children = NULL;
	}

	mlist_traverse(children, free_children);
	mlist_destroy(children, free);
}

size_t dirtree_count(const char *path) {
	t_directory *parent = dirtree_find(path);
	if(parent == NULL) return 0;
	mlist_t *children = children_of_dir(parent);
	size_t count = mlist_length(children);
	mlist_destroy(children, NULL);
	return count;
}

void dirtree_list(const char *path) {
	t_directory *parent = dirtree_find(path);
	if(parent == NULL) return;
	mlist_t *children = children_of_dir(parent);
	bool sorter(t_directory *dir1, t_directory *dir2) {
		return mstring_asc(dir1->name, dir2->name);
	}
	mlist_sort(children, sorter);
	void printer(t_directory *dir) {
		printf("%s/\n", dir->name);
	}
	mlist_traverse(children, printer);
	mlist_destroy(children, NULL);
}

char *dirtree_rpath(const char *ypath) {
	t_directory *dir = dirtree_find(ypath);
	if(dir == NULL) return NULL;
	return path_to_files(dir);
}

char *dirtree_ypath(const char *rpath) {
	if(!mstring_hasprefix(rpath, files_dir())) return NULL;
	int index = mstring_toint(rpath);
	char *ypath = mstring_empty(NULL);
	t_directory *dir;

	while(dir = find_dir_by_index(index), dir != NULL && dir->index > 0) {
		mstring_format(&ypath, "%s/%s", dir->name, ypath);
		index = dir->parent;
	}

	*mstring_end(ypath) = '\0';
	mstring_format(&ypath, "yamafs:/%s", ypath);
	return ypath;
}

void dirtree_term() {
	save_to_file();
	file_unmap(file, map);
	file_close(file);
	map = NULL;
	file = NULL;
}

// ========== Funciones privadas ==========

static const char *files_dir() {
	static char fdir[PATH_MAX] = {0};
	if(!*fdir) {
		snprintf(fdir, PATH_MAX, "%s/metadata/archivos", system_userdir());
	}
	return fdir;
}

static char *path_to_files(t_directory *dir) {
	return mstring_create("%s/%i", files_dir(), dir->index);
}

static mlist_t *children_of_dir(t_directory *parent) {
	mlist_t *children = mlist_create();
	for(t_directory *dir = dirs; dir < dirs + MAX_SIZE; dir++) {
		if(dir_exists(dir) && dir->parent == parent->index) {
			mlist_append(children, dir);
		}
	}
	return children;
}

static void get_children(mlist_t *children, int index, int depth) {
	for(t_directory *dir = dirs; dir < dirs + MAX_SIZE; dir++) {
		if(dir_exists(dir) && dir->parent == index) {
			t_print_child *child = malloc(sizeof(t_print_child));
			child->name = dir->name;
			child->depth = depth;
			child->children = mlist_create();
			mlist_append(children, child);
			get_children(child->children, dir->index, depth + 1);
		}
	}
}

static t_directory *add_directory(const char *name, int parent) {
	t_directory *pdir = find_dir_by_name(name, parent);
	if(pdir != NULL) return pdir;

	for(int i = 0; i < MAX_SIZE; i++) {
		if(!dir_exists(dirs + i)) {
			pdir = dirs + i;
			break;
		}
	}

	strcpy(pdir->name, name);
	pdir->parent = parent;

	char *dir_path = path_to_files(pdir);
	path_mkdir(dir_path);
	free(dir_path);

	return pdir;
}

static char *create_normal_path(const char *path) {
	char *p = (char*) path;

	const char *pref = "yamafs:";
	if(mstring_hasprefix(p, pref)) p += strlen(pref);

	bool s_bar = p[0] == '/';
	bool e_bar = p[strlen(p)-1] == '/';

	return mstring_create("%s%s%s", s_bar ? "" : "/", p, e_bar ? "" : "/");
}

static t_directory *find_dir_by_index(int index) {
	for(t_directory *dir = dirs; dir < dirs + MAX_SIZE; dir++) {
		if(dir_exists(dir) && dir->index == index) {
			return dir;
		}
	}
	return NULL;
}

static t_directory *find_dir_by_name(const char *name, int parent) {
	for(t_directory *dir = dirs; dir < dirs + MAX_SIZE; dir++) {
		if(dir_exists(dir) && mstring_equal(dir->name, name) && dir->parent == parent) {
			return dir;
		}
	}
	return NULL;
}

static void remove_children(t_directory *dir) {
	if(dir == NULL) return;
	for(t_directory *cur = dirs; cur < dirs + MAX_SIZE; cur++) {
		if(dir_exists(cur) && cur->parent == dir->index) {
			remove_children(cur);
			remove_directory(cur);
		}
	}
}

static void remove_directory(t_directory *dir) {
	if(dir == NULL || dir->index == 0) return;
	dir->parent = -2;

	char *dir_path = path_to_files(dir);
	path_remove(dir_path);
	free(dir_path);
}

static void map_file() {
	if(file != NULL) return;
	if(!path_exists(DAT_PATH)) {
		path_truncate(DAT_PATH, sizeof(t_directory) * MAX_SIZE);
	}
	file = file_open(DAT_PATH);
	map = file_map(file);
}

static void save_to_file() {
	map_file();
	size_t size = sizeof(t_directory) * MAX_SIZE;
	memcpy(map, dirs, size);
	file_sync(file, map);
}
