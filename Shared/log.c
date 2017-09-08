#include <commons/log.h>
#include <file.h>
#include <mstring.h>
#include <process.h>
#include <stdarg.h>
#include <stdlib.h>

#define LOG_ENABLED true
#define LOG_MAX 256

static t_log *logger(bool print);
static void logtemplate(bool error, bool print, const char *format, va_list args);

// ========== Funciones públicas ==========

// [i: info; e: error]
// [f: file; p: print]

void logif(const char *format, ...) {
	va_list args;
	va_start(args, format);
	logtemplate(false, false, format, args);
	va_end(args);
}

void logef(const char *format, ...) {
	va_list args;
	va_start(args, format);
	logtemplate(true, false, format, args);
	va_end(args);
}

void logip(const char *format, ...) {
	va_list args;
	va_start(args, format);
	logtemplate(false, true, format, args);
	va_end(args);
}

void logep(const char *format, ...) {
	va_list args;
	va_start(args, format);
	logtemplate(true, true, format, args);
	va_end(args);
}

// ========== Funciones privadas ==========

static t_log *logger(bool print) {
	static t_log *logger = NULL, *printer;
	if(logger == NULL) {
		char *pname = (char*) process_name(process_current());
		char *logfile = mstring_format("%s/logs/%s.log", file_userdir(), pname);
		logger = log_create(logfile, pname, false, LOG_LEVEL_TRACE);
		printer = log_create(logfile, pname, true, LOG_LEVEL_TRACE);
		free(logfile);
	}
	return print ? printer : logger;
}

static void logtemplate(bool error, bool print, const char *format, va_list args) {
	if(!LOG_ENABLED) return;

	char log[LOG_MAX];
	vsnprintf(log, LOG_MAX, format, args);

	if(error) {
		log_error(logger(print), log);
	} else {
		log_debug(logger(print), log);
	}
}
