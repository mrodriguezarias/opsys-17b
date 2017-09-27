#include <string.h>
#include <ctype.h>
#include <mstring.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static char *create_format_template(const char *format, va_list args);

// ========== Funciones públicas ==========

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

char *mstring_create(const char *format, ...) {
	char *str = NULL;
	va_list args;
	va_start(args, format);
	str = create_format_template(format, args);
	va_end(args);
	return str;
}

void mstring_format(char **string, const char *format, ...) {
	char *str = NULL;
	va_list args;
	va_start(args, format);
	str = create_format_template(format, args);
	va_end(args);
	free(*string);
	*string = str;
}

char *mstring_copy(const char *string, int start, int end) {
	if(end < 0) end = strlen(string) - 1;
	return mstring_create("%.*s", end - start + 1, string + start);
}

bool mstring_isempty(const char *string) {
	if(string == NULL) return true;
	char *copy = strdup(string);
	mstring_trim(copy);
	bool empty = *copy == '\0';
	free(copy);
	return empty;
}

char *mstring_empty(char **string) {
	if(string == NULL) {
		return strdup("");
	}
	if(*string != NULL) {
		free(*string);
	}
	*string = strdup("");
	return *string;
}

bool mstring_equal(const char *str1, const char *str2) {
	return strcmp(str1, str2) == 0;
}

bool mstring_equali(const char *str1, const char *str2) {
	return strcasecmp(str1, str2) == 0;
}

bool mstring_contains(const char *string, const char *substring) {
	return strstr(string, substring) != NULL;
}

int mstring_toint(const char *string) {
	return (int) strtol(string, NULL, 0);
}

bool mstring_asc(const char *str1, const char *str2) {
	return strcmp(str1, str2) <= 0;
}

bool mstring_desc(const char *str1, const char *str2) {
	return strcmp(str1, str2) >= 0;
}

// ========== Funciones privadas ==========

static char *create_format_template(const char *format, va_list args) {
	char buffer[1024];
	vsnprintf(buffer, sizeof buffer, format, args);
	return strdup(buffer);
}
