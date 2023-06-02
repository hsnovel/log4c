#ifndef LOG4C_H
#define LOG4C_H

#include <stdio.h>

enum { LOG4C_NOTAG, LOG4C_OK, LOG4C_TRACE, LOG4C_DEBUG, LOG4C_INFO, LOG4C_WARN, LOG4C_ERROR, LOG4C_FATAL };

void _log(int level, int line, const char* file_name, const char* args, ...);

void log_disable_thread_safe();
void log_enable_thread_safe();
void log_set_quiet(unsigned int quiet);
void log_set_level(int level);
void log_append_fp(FILE* file_descriptor);
void log_detach_fp();
int log_append_file(const char *path);

/*
 * Cannot get line numbers inside functions without specifiying them as
 * an argument in the function call. You have to #define them otherwise
 * you have to pass __LINE__ and __FILE__ to functions which is an
 * extra typing...
 */

#ifdef LOG4C_DISABLE_NOTAG
#define log_notag(...)
#else
#define log_notag(...) _log(LOG4C_NOTAG, __LINE__, __FILE__, __VA_ARGS__)
#endif

#ifdef LOG4C_DISABLE_OK
#define log_ok(...)
#else
#define log_ok(...) _log(LOG4C_OK, __LINE__, __FILE__, __VA_ARGS__)
#endif

#ifdef LOG4C_DISABLE_TRACE
#define log_trace(...)
#else
#define log_trace(...) _log(LOG4C_TRACE, __LINE__, __FILE__, __VA_ARGS__)
#endif

#ifdef LOG4C_DISABLE_DEBUG
#define log_debug(...)
#else
#define log_debug(...) _log(LOG4C_DEBUG, __LINE__, __FILE__, __VA_ARGS__)
#endif

#ifdef LOG4C_DISABLE_INFO
#define log_info(...)
#else
#define log_info(...)  _log(LOG4C_INFO, __LINE__, __FILE__, __VA_ARGS__)
#endif

#ifdef LOG4C_DISABLE_WARN
#define log_warn(...)
#else
#define log_warn(...)  _log(LOG4C_WARN, __LINE__, __FILE__, __VA_ARGS__)
#endif

#ifdef LOG4C_DISABLE_ERROR
#define log_error(...)
#else
#define log_error(...) _log(LOG4C_ERROR, __LINE__, __FILE__, __VA_ARGS__)
#endif

#ifdef LOG4C_DISABLE_FATAL
#define log_fatal(...)
#else
#define log_fatal(...) _log(LOG4C_FATAL, __LINE__, __FILE__, __VA_ARGS__)
#endif

#endif
