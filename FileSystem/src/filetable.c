#include "filetable.h"
#include <mlist.h>

static mlist_t *files = NULL;

void filetable_init() {
	if(files != NULL) return;
	files = mlist_create();

}
