#include <ctype.h>
#include <mstring.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <number.h>

static char *create_format_template(const char *format, va_list args);

// ========== Funciones públicas ==========

char *mstring_duplicate(const char *string) {
	return string == NULL ? NULL : strdup(string);
}

size_t mstring_length(const char *string) {
	return strlen(string);
}

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
	if(end < 0) end = strlen(string) + end;
	return mstring_create("%.*s", end - start, string + start);
}

char *mstring_crop(char **string, int length) {
	if(strlen(*string) <= length) return *string;
	*(*string + length) = '\0';
	mstring_format(string, "%s", *string);
	return *string;
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
	if(str1 == NULL && str2 == NULL) return true;
	if(str1 == NULL || str2 == NULL) return false;
	return strcmp(str1, str2) == 0;
}

bool mstring_equali(const char *str1, const char *str2) {
	if(str1 == NULL && str2 == NULL) return true;
	if(str1 == NULL || str2 == NULL) return false;
	return strcasecmp(str1, str2) == 0;
}

bool mstring_contains(const char *string, const char *substring) {
	return mstring_find(string, substring) != NULL;
}

char *mstring_find(const char *string, const char *substring) {
	return strstr(string, substring);
}

int mstring_count(const char *string, const char *substring) {
	int count = 0;
	int len = strlen(substring);
	char *p = (char*) string;
	while(p = mstring_find(p, substring), p != NULL) {
		p += len;
		count++;
	}
	return count;
}

int mstring_index(const char *string, const char *substring) {
	char *p = mstring_find(string, substring);
	return p != NULL ? p - string : -1;
}

int mstring_toint(const char *string) {
	char *p = (char*) string;
	while(*p != '\0' && !isdigit(*p)) p++;
	return (int) strtol(p, NULL, 0);
}

bool mstring_asc(const char *str1, const char *str2) {
	return strcmp(str1, str2) <= 0;
}

bool mstring_desc(const char *str1, const char *str2) {
	return strcmp(str1, str2) >= 0;
}

char *mstring_repeat(const char *string, int times) {
	char *repeated = mstring_empty(NULL);
	while(times--) {
		mstring_format(&repeated, "%s%s", repeated, string);
	}
	return repeated;
}

bool mstring_replace(char **string, const char *substring, const char *replacement) {
	int i = mstring_index(*string, substring);
	if(i == -1) return false;

	char *result = mstring_copy(*string, 0, i);
	mstring_format(&result, "%s%s%s", result, replacement, *string + i + strlen(substring));

	free(*string);
	*string = result;
	return true;
}

bool mstring_hasprefix(const char *string, const char *prefix) {
	return strncmp(string, prefix, strlen(prefix)) == 0;
}

bool mstring_hassuffix(const char *string, const char *suffix) {
	int suffix_len = strlen(suffix);
	return strncmp(string + strlen(string) - suffix_len, suffix, suffix_len) == 0;
}

char *mstring_end(const char *string) {
	int len = strlen(string);
	return (char*)(len > 0 ? string + len - 1 : string);
}

char *mstring_bsize(size_t size) {
	static const char *pref[] = {" ", "K", "M", "G", "T"};
	const unsigned short mult = 1024;
	const char *x = size < mult ? " " : "i";
	float s = size;
	int i = 0;
	while(s >= mult || number_round(s, 1) == mult) {
		i++;
		s /= mult;
	}
	return mstring_create("%.1f %s%sB", s, pref[i], x);
}

// ========== Funciones privadas ==========

static char *create_format_template(const char *format, va_list args) {
	char buffer[MSTRING_MAXSIZE];
	vsnprintf(buffer, sizeof buffer, format, args);
	return strdup(buffer);
}
