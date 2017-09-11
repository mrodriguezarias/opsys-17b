#include <commons/log.h>
#include <mstring.h>
#include <process.h>
#include <stdarg.h>
#include <stdlib.h>
#include <system.h>

#define LOG_ENABLED true
#define LOG_MAX 256

static t_log *logger(bool print);
static void template(bool error, bool print, const char *format, va_list args);

// ========== Funciones públicas ==========

void log_inform(const char *format, ...) {
	va_list args;
	va_start(args, format);
	template(false, false, format, args);
	va_end(args);
}

void log_print(const char *format, ...) {
	va_list args;
	va_start(args, format);
	template(false, true, format, args);
	va_end(args);
}

void log_report(const char *format, ...) {
	va_list args;
	va_start(args, format);
	template(true, true, format, args);
	va_end(args);
}

// ========== Funciones privadas ==========

static t_log *logger(bool print) {
	static t_log *logger = NULL, *printer;
	if(logger == NULL) {
		char *pname = (char*) process_name(process_current());
		char *logfile = mstring_create("%s/logs/%s.log", system_userdir(), pname);
		logger = log_create(logfile, pname, false, LOG_LEVEL_TRACE);
		printer = log_create(logfile, pname, true, LOG_LEVEL_TRACE);
		free(logfile);
	}
	return print ? printer : logger;
}

static void template(bool error, bool print, const char *format, va_list args) {
	if(!LOG_ENABLED || process_current() == PROC_UNDEFINED) return;

	char log[LOG_MAX];
	vsnprintf(log, LOG_MAX, format, args);

	if(error) {
		log_error(logger(print), log);
	} else {
		log_debug(logger(print), log);
	}
}
