#include "mstring.h"
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

char *mstring_trim(char *string) {
	if(string == NULL || string[0] == '\0') {
		return string;
	}
	
	size_t length = strlen(string);
    char *start = string;
    char *oend = string + length - 1;
    char *end = oend;
    
    while(isspace(*start)) start++;
    while(isspace(*end) && end != start) end--;

    if(end != oend) *(end + 1) = '\0';
    else if(start != string && end == start) *start = '\0';
    
    if(start != string) {
		char *p = string;
        while(*start) *p++ = *start++;
        *p = '\0';
    }
    
    return string;
}

char *mstring_format(const char *format, ...) {
	char buffer[1024];
	va_list ap;
	va_start(ap, format);
	vsnprintf(buffer, sizeof buffer, format, ap);
	va_end(ap);
	return strdup(buffer);
}

bool mstring_empty(const char *string) {
	if(string == NULL) return true;
	char *copy = strdup(string);
	mstring_trim(copy);
	bool empty = *copy == '\0';
	free(copy);
	return empty;
}
