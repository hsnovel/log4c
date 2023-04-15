#ifndef LOG4C_H
#define LOG4C_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#ifdef _MSC_VER
#include <windows.h>
#else
#include <pthread.h>
#endif

/*
 * LOG4C_DISABLE_COLOR -> Disable colors
 * LOG4C_ENABLE_BOLD_COLORS -> Enable bold colors
 */

/*
 * Cannot get line numbers inside functions without specifiying them as
 * an argument in the function call. You have to #define them otherwise
 * you have to pass __LINE__ and __FILE__ to functions which is an
 * extra typing...
 */

#if !defined(LOG4C_RELEASE) && !defined(LOG4C_DISABLE)
#define log_notag(...) _log(LOG_NOTAG, __LINE__, __FILE__, __VA_ARGS__)
#define log_ok(...) _log(LOG_OK, __LINE__, __FILE__, __VA_ARGS__)
#define log_trace(...) _log(LOG_TRACE, __LINE__, __FILE__, __VA_ARGS__)
#define log_debug(...) _log(LOG_DEBUG, __LINE__, __FILE__, __VA_ARGS__)
#define log_info(...)  _log(LOG_INFO, __LINE__, __FILE__, __VA_ARGS__)
#define log_warn(...)  _log(LOG_WARN, __LINE__, __FILE__, __VA_ARGS__)
#define log_error(...) _log(LOG_ERROR, __LINE__, __FILE__, __VA_ARGS__)
#define log_fatal(...) _log(LOG_FATAL, __LINE__, __FILE__, __VA_ARGS__)
#endif

#if defined(LOG4C_DISABLE) && !defined(LOG4C_RELEASE)
#define log_notag(...)
#define log_ok(...)
#define log_trace(...)
#define log_debug(...)
#define log_info(...)
#define log_warn(...)
#define log_error(...)
#define log_fatal(...)
#endif

/* Disable all messages expect error, fatal */
#if defined(LOG4C_RELEASE) && !defined(LOG4C_DISABLE)
#define log_notag(...)
#define log_ok(...)
#define log_trace(...)
#define log_debug(...)
#define log_info(...)
#define log_warn(...)
#define log_error(...) _log(LOG_ERROR, __LINE__, __FILE__, __VA_ARGS__)
#define log_fatal(...) _log(LOG_FATAL, __LINE__, __FILE__, __VA_ARGS__)
#endif

#ifdef LOG4C_DISABLE_NOTAG
#define log_notag(...)
#endif
#ifdef LOG4C_DISABLE_OK
#define log_warn(...)
#endif
#ifdef LOG4C_DISABLE_TRACE
#define log_trace(...)
#endif
#ifdef LOG4C_DISABLE_DEBUG
#define log_debug(...)
#endif
#ifdef LOG4C_DISABLE_INFO
#define log_info(...)
#endif
#ifdef LOG4C_DISABLE_WARN
#define log_warn(...)
#endif
#ifdef LOG4C_DISABLE_ERROR
#define log_error(...)
#endif
#ifdef LOG4C_DISABLE_FATAL
#define log_fatal(...)
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum { LOG_NOTAG, LOG_OK, LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

static struct {
	int level;
	unsigned int quiet;
	FILE* terminal_descriptor;
	FILE* file_descriptor;
#ifdef _MSC_VER
	CRITICAL_SECTION mutex;
#else
	pthread_mutex_t mutex;
#endif
	unsigned int thread_safe_initialized;
} _log_global_settings = {
	.level = LOG_NOTAG,
	.quiet = 0,
	.terminal_descriptor = 0,
	.file_descriptor = 0,
	.thread_safe_initialized = 0,
};

#if !defined LOG4C_RELEASE
static const char* _log_level_strings[] = {
	"NOTAG", "OK", "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};
#endif

#if !defined(LOG4C_DISABLE_COLOR)
#if defined(LOG4C_ENABLE_BOLD_COLORS)
static const char* _log_level_colors[] = {
	"\x1b[1;39m", "\x1b[1;32m", "\x1b[1;35m", "\x1b[1;34m", "\x1b[1;32m", "\x1b[1;33m", "\x1b[1;31m", "\x1b[1;31m"
};
#else
#if !defined(LOG4C_RELEASE)
static const char* _log_level_colors[] = {
	"\x1b[39m", "\x1b[32m", "\x1b[35m", "\x1b[34m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[31m"
};
#endif
#endif
#endif

inline static void log_disable_thread_safe()
{
	_log_global_settings.thread_safe_initialized = 0;
}

inline static void log_enable_thread_safe()
{
	if (_log_global_settings.thread_safe_initialized == 0) {
#ifdef _MSC_VER
		InitializeCriticalSection(&_log_global_settings.mutex);
#else
		pthread_mutex_init(&_log_global_settings.mutex, NULL);
#endif
	}
	_log_global_settings.thread_safe_initialized = 1;
}

inline static void log_set_quiet(unsigned int quiet)
{
	_log_global_settings.quiet = quiet;
}

inline static void log_set_level(int level)
{
	_log_global_settings.level = level;
}

inline static void log_append_file(const char *path)
{
#ifndef _WIN32
	FILE *f = fopen(path, "a");
	_log_global_settings.file_descriptor = f;
#else
	FILE *f;
	fopen_s(&f, path, "a");
	_log_global_settings.file_descriptor = f;
#endif
}

inline static void log_append_fp(FILE* file_descriptor)
{
	_log_global_settings.file_descriptor = file_descriptor;
}

inline static void log_detach_fp()
{
	_log_global_settings.file_descriptor = NULL;
}

inline static void _log_handle_config(int level, int *scoped_level)
{
	/* Mutex */
	if (_log_global_settings.thread_safe_initialized) {
#ifdef _MSC_VER
		EnterCriticalSection(&_log_global_settings.mutex);
#else
		pthread_mutex_lock(&_log_global_settings.mutex);
#endif
	}

	/* Quiet Mode */
	if (_log_global_settings.quiet == 0)
		_log_global_settings.terminal_descriptor = stderr;
	else
		_log_global_settings.terminal_descriptor = stdout;

	/* Min Level */
	if (level >= _log_global_settings.level)
		*scoped_level = level;
	else
		*scoped_level = _log_global_settings.level;
}

#if !defined (_MSC_VER)

/* UNIX version of logging function */
static void _log(int level, int line, const char* file_name, const char* args, ...)
{
	int scoped_level;
	struct tm* current_time;
	va_list variadic_list;

	_log_handle_config(level, &scoped_level);

	time_t tx = time(NULL);
	current_time = localtime(&tx);

	char date[16];

	va_start(variadic_list, args);
	date[strftime(date, sizeof(date), "%H:%M:%S", current_time)] = '\0';


#if !defined(LOG4C_RELEASE)
#if !defined(LOG4C_DISABLE_COLOR)
	fprintf(_log_global_settings.terminal_descriptor, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m",
		date, _log_level_colors[scoped_level], _log_level_strings[scoped_level], file_name, line);
#else
	fprintf(_log_global_settings.terminal_descriptor, "%s %-5s %s:%d ", date, _log_level_strings[scoped_level],
		file_name, line);
#endif
#endif

	/* Print the string */
	vfprintf(_log_global_settings.terminal_descriptor, args, variadic_list);
	putc('\n', _log_global_settings.terminal_descriptor);

#if !defined LOG4C_RELEASE
	if (_log_global_settings.file_descriptor) {
		va_list file_list;
		va_copy(file_list, variadic_list);

		fprintf(_log_global_settings.file_descriptor, "%s %-5s %s:%d: ",
			date, _log_level_strings[scoped_level], file_name, line);

		vfprintf(_log_global_settings.file_descriptor, args, file_list);
		putc('\n', _log_global_settings.file_descriptor);
		fflush(_log_global_settings.file_descriptor);
	}
#endif

	/*
	 * Normaly terminals are line buffered but we obviously:
	 *
	 *   - cannot rely on that as stream buffering mode might be
	 *     set to something different by the programmer
	 *   - cannot guarantee that the terminal will flush on a new line
	 *
	 * So we flush the file stream to prevent loss of logs.
	 */

	fflush(_log_global_settings.terminal_descriptor);

	va_end(variadic_list);
	if (_log_global_settings.thread_safe_initialized) {
		pthread_mutex_unlock(&_log_global_settings.mutex);
	}
}

#else

/* Win32 version of logging function */
static void _log(int level, int line, const char* file_name, const char* args, ...)
{
	int scoped_level;
	struct tm* current_time;
	va_list variadic_list;

	_log_handle_config(level, &scoped_level);

	va_start(variadic_list, args);
	SYSTEMTIME LocalTime;
	GetLocalTime(&LocalTime);


#if !defined(LOG4C_RELEASE)
#if !defined(LOG4C_DISABLE_COLOR)
	fprintf(_log_global_settings.terminal_descriptor, "%d:%d:%d %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m",
		LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond, _log_level_colors[scoped_level], _log_level_strings[scoped_level], file_name, line);
#else
	fprintf(_log_global_settings.terminal_descriptor, "%s %-5s %s:%d ", date, _log_level_strings[scoped_level],
		file_name, line);
#endif
#endif

	/* Print string */
	vfprintf(_log_global_settings.terminal_descriptor, args, variadic_list);
	putc('\n', _log_global_settings.terminal_descriptor);

#if !defined(LOG4C_RELEASE)
	vfprintf(_log_global_settings.terminal_descriptor, args, variadic_list);
	putc('\n', _log_global_settings.terminal_descriptor);

	fflush(_log_global_settings.terminal_descriptor);
#endif

#if !defined(LOG4C_RELEASE)
	if (_log_global_settings.file_descriptor) {
		va_list file_list;
		va_copy(file_list, variadic_list);
		fprintf(_log_global_settings.file_descriptor, "%d:%d:%d %-5s %s:%d: ",
			LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond, _log_level_strings[scoped_level], file_name, line);
		vfprintf(_log_global_settings.file_descriptor, args, file_list);
		putc('\n', _log_global_settings.file_descriptor);
		fflush(_log_global_settings.file_descriptor);
	}
#endif

	va_end(variadic_list);
	if (_log_global_settings.thread_safe_initialized) {
		LeaveCriticalSection(&_log_global_settings.mutex);
	}
}

#endif

#ifdef __cplusplus
}
#endif
#endif
