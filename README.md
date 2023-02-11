# log4c

log4c is a single header thread safe fast logging library for C and C++.

![sreenshot](https://user-images.githubusercontent.com/53369750/215074252-5ad1e9aa-71ad-49e6-9809-bf18d4a7b6bf.png)

# Usage

Do not put "\n" at the end of the logging functions, they are added by default. \
If you want to disable logging define ```LOG4C_DISABLE``` before including the library.

If ```LOG4C_RELEASE``` is defined, only enable ```log_error``` and ```log_fatal``` functions,
disable printing line number, date etc.. Only prints arguments passed to these functions.
This mode disables logging to a file.

Preferance
```cpp
#define LOG4C_DISABLE_COLOR      /* Disable error codes to be printed with VT colors */
#define LOG4C_ENABLE_BOLD_COLORS /* Works only if LOG4C_DISABLE_COLOR is not set */
```

Logging functions
```cpp
log_notag(...);
log_ok(...);
log_trace(...);
log_debug(...);
log_info(...);
log_warn(...);
log_error(...);
log_fatal(...);
```

Thread safety
```cpp
log_enable_thread_safe();   /* Enables thread safety globally  */
log_disable_thread_safe();  /* Disables thread safety globally */
```

Quiet mode, set to 0 by default, set quiet to 1 enable.
```cpp
log_set_quiet(unsigned int quiet); /* If enabled do not print to stderr. */
```

Set the minimum warning level.
```cpp
log_set_level(int level);
```

Output log to file. Note if you use ```log_append_file``` it will open the file in the append mode to prevent
loss of old data. If this is not what you want you can open the file in any mode you want and call
```log_append_fp```

```cpp
log_append_file(const char *path);
log_append_fp(FILE *file_descriptor);
log_detach_fp(); /* Remove file descriptor. Disable logging to file. */
```
