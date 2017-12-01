#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <stdbool.h>
#include <yfile.h>

struct {
	bool formatted;
	bool yama_connected;
} fs;

#endif /* FILESYSTEM_H_ */
