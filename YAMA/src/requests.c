#include <file.h>

typedef enum {
	STAGE_INIT,
	STAGE_TRANSFORM,
	STAGE_LOCAL_REDUCTION,
	STAGE_GLOBAL_REDUCTION,
	STAGE_END
} t_stage;

typedef enum {
	REQSTATUS_ERROR = -1,
	REQSTATUS_STARTED,
	REQSTATUS_INPROGRESS,
	REQSTATUS_ENDED,
} t_reqstatus;

typedef struct {
	int job;
	int master;
	int node;
	int block;
	t_stage stage;
	char tmpfile[PATH_MAX];
} t_request;
