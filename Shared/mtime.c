#include "mtime.h"
#include <time.h>
#include <sys/time.h>
#include <mstring.h>
#include <stdlib.h>
#include <stdio.h>
#include <number.h>

struct tm local_time(time_t time);
char *time_string(mtime_t time, bool precise);
char *diff_string(mtime_t time, bool precise);

// ========== Funciones públicas ==========

inline mtime_t mtime_now() {
	struct timeval time;
	gettimeofday(&time, NULL);
	return (mtime_t) time.tv_sec * 1000 + time.tv_usec / 1000;
}

char *mtime_date(mtime_t time) {
	struct tm lt = local_time(time / 1000);
	return mstring_create("%d-%02d-%02d", lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday);
}

char *mtime_time(mtime_t time) {
	return time_string(time, false);
}

char *mtime_datetime(mtime_t time) {
	char *sdate = mtime_date(time);
	char *stime = mtime_time(time);
	char *datetime = mstring_create("%s %s", sdate, stime);
	free(sdate);
	free(stime);
	return datetime;
}

mtime_t mtime_diff(mtime_t t1, mtime_t t2) {
	return number_abs((t_number) t1 - t2);
}

char *mtime_formatted(mtime_t time, int mode) {
	if(mode & MTIME_DIFF) {
		return diff_string(time, mode & MTIME_PRECISE);
	}
	char *ftime = time_string(time, mode & MTIME_PRECISE);
	if(mode & MTIME_DATE) {
		char *date = mtime_date(time);
		mstring_format(&ftime, "%s %s", date, ftime);
		free(date);
	}
	return ftime;
}

void mtime_print(mtime_t time, int mode) {
	char *tf = mtime_formatted(time, mode);
	printf("%s\n", tf);
	free(tf);
}

// ========== Funciones privadas ==========

struct tm local_time(time_t time) {
	struct tm lt;
	localtime_r(&time, &lt);
	return lt;
}

char *time_string(mtime_t time, bool precise) {
	struct tm lt = local_time(time / 1000);
	char *tstr = mstring_create("%02d:%02d:%02d", lt.tm_hour, lt.tm_min, lt.tm_sec);
	if(precise)
		mstring_format(&tstr, "%s.%03d", tstr, time % 1000);
	return tstr;
}

char *diff_string(mtime_t time, bool precise) {
	mtime_t tsec = time / 1000;
	int s = tsec % 60;
	mtime_t tmin = tsec / 60;
	int m = tmin % 60;
	int h = tmin / 60;
	char *tstr = mstring_create("%02d:%02d:%02d", h, m, s);
	if(precise)
		mstring_format(&tstr, "%s.%03d", tstr, time % 1000);
	return tstr;
}
