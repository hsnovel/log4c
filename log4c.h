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

#ifndef LOG4C_DISABLE
#define log_notag(...) _log(LOG_NOTAG, __LINE__, __FILE__, __VA_ARGS__)
#define log_ok(...) _log(LOG_OK, __LINE__, __FILE__, __VA_ARGS__)
#define log_trace(...) _log(LOG_TRACE, __LINE__, __FILE__, __VA_ARGS__)
#define log_debug(...) _log(LOG_DEBUG, __LINE__, __FILE__, __VA_ARGS__)
#define log_info(...)  _log(LOG_INFO, __LINE__, __FILE__, __VA_ARGS__)
#define log_warn(...)  _log(LOG_WARN, __LINE__, __FILE__, __VA_ARGS__)
#define log_error(...) _log(LOG_ERROR, __LINE__, __FILE__, __VA_ARGS__)
#define log_fatal(...) _log(LOG_FATAL, __LINE__, __FILE__, __VA_ARGS__)

#else

#define log_notag(...)
#define log_ok(...)
#define log_trace(...)
#define log_debug(...)
#define log_info(...)
#define log_warn(...)
#define log_error(...)
#define log_fatal(...)

#endif

#ifdef __cplusplus
extern "C" {
#endif

enum { LOG_NOTAG, LOG_OK, LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

static struct {
	int level;
	unsigned int quiet;
	void* terminal_descriptor;
	void* file_descriptor;
#ifdef _MSC_VER
	CRITICAL_SECTION mutex;
#else
	pthread_mutex_t mutex;
#endif
	unsigned int thread_safe_initialized;
} _log_global_settings = {
	.level = LOG_NOTAG,
	.quiet = 0,
	.terminal_descriptor = (void*)0,
	.file_descriptor = 0,
	.thread_safe_initialized = 0,
};

static const char* _log_level_strings[] = {
	"NOTAG", "OK", "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#if !defined(LOG4C_DISABLE_COLOR)
#if defined(LOG4C_ENABLE_BOLD_COLORS)
static const char* _log_level_colors[] = {
	"\x1b[1;39m", "\x1b[1;32m", "\x1b[1;35m", "\x1b[1;34m", "\x1b[1;32m", "\x1b[1;33m", "\x1b[1;31m", "\x1b[1;31m"
};
#else
static const char* _log_level_colors[] = {
	"\x1b[39m", "\x1b[32m", "\x1b[35m", "\x1b[34m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[31m"
};
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

inline static void log_output_file(void* file_descriptor)
{
	_log_global_settings.file_descriptor = (FILE*)file_descriptor;
}

inline static void log_disable_output_file()
{
	_log_global_settings.file_descriptor = NULL;
}

static void _log(int level, int line, const char* file_name, const char* args, ...)
{
	if (_log_global_settings.thread_safe_initialized) {
#ifdef _MSC_VER
		EnterCriticalSection(&_log_global_settings.mutex);
#else
		pthread_mutex_lock(&_log_global_settings.mutex);
#endif
	}

	int scoped_level;
	struct tm* current_time;
	va_list variadic_list;

	if (_log_global_settings.quiet == 0)
		_log_global_settings.terminal_descriptor = stderr;
	else
		_log_global_settings.terminal_descriptor = stdout;

	if (level >= _log_global_settings.level)
		scoped_level = level;
	else
		scoped_level = _log_global_settings.level;

#ifdef _MSC_VER
#else
	time_t tx = time(NULL);
	current_time = localtime(&tx);
#endif

	char buf[16];

	va_start(variadic_list, args);
#ifdef _MSC_VER
	SYSTEMTIME LocalTime;
	GetLocalTime(&LocalTime);
#else
	buf[strftime(buf, sizeof(buf), "%H:%M:%S", current_time)] = '\0';
#endif

#if !defined(LOG4C_DISABLE_COLOR)
#ifdef _MSC_VER
	fprintf((FILE*)_log_global_settings.terminal_descriptor, "%d:%d:%d %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m",
		LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond, _log_level_colors[scoped_level], _log_level_strings[scoped_level], file_name, line);
#else
	fprintf((FILE*)_log_global_settings.terminal_descriptor, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m",
		buf, _log_level_colors[scoped_level], _log_level_strings[scoped_level], file_name, line);
#endif
#else
	fprintf((FILE*)_log_global_settings.terminal_descriptor, "%s %-5s %s:%d ", buf, _log_level_strings[scoped_level],
		file_name, line);
#endif

	if (_log_global_settings.file_descriptor) {
		va_list file_list;
		va_copy(file_list, variadic_list);
#ifdef _MSC_VER
		fprintf((FILE*)_log_global_settings.file_descriptor, "%d:%d:%d %-5s %s:%d: ",
			LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond, _log_level_strings[scoped_level], file_name, line);
#else
		fprintf((FILE*)_log_global_settings.file_descriptor, "%s %-5s %s:%d: ",
			buf, _log_level_strings[scoped_level], file_name, line);
#endif
		vfprintf((FILE*)_log_global_settings.file_descriptor, args, file_list);
		putc('\n', (FILE*)_log_global_settings.file_descriptor);
		fflush((FILE*)_log_global_settings.file_descriptor);
	}


	/* Print to terminal */
	vfprintf((FILE*)_log_global_settings.terminal_descriptor, args, variadic_list);
	putc('\n', (FILE*)_log_global_settings.terminal_descriptor);

	/*
	 * Normaly terminals are line buffered but we obviously:
	 *
	 *   - cannot rely on that as stream buffering mode might be
	 *     set to something different by the programmer
	 *   - cannot guarantee that the terminal will flush on a new line
	 *
	 * So we flush the file stream to prevent loss of logs.
	 */

	fflush((FILE*)_log_global_settings.terminal_descriptor);

	va_end(variadic_list);
	if (_log_global_settings.thread_safe_initialized) {
#ifdef _MSC_VER
		LeaveCriticalSection(&_log_global_settings.mutex);
#else
		pthread_mutex_unlock(&_log_global_settings.mutex);
#endif
	}

}

#ifdef __cplusplus
}
#endif
#endif
