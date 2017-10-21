#ifndef SERVER_H_
#define SERVER_H_

#include <yfile.h>

void server_start(void);

void server_set_current_file(t_yfile *file);

void server_end(void);

#endif /* SERVER_H_ */
