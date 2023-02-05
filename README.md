# log4c

log4c is a single header thread safe fast logging library for C and C++.

![sreenshot](https://user-images.githubusercontent.com/53369750/215074252-5ad1e9aa-71ad-49e6-9809-bf18d4a7b6bf.png)

# Usage

Do not put "\n" at the end of the logging functions, they are added by default. \
If you want to disable logging define ```LOG4C_DISABLE``` before including the library.

Preferance
```cpp
#define LOG4C_DISABLE_COLOR      // Disable error codes to be printed with VT colors
#define LOG4C_ENABLE_BOLD_COLORS // Works only if LOG4C_ENABLE_COLOR is not set 
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
log_enable_thread_safe();   // Enables thread safety globally
log_disable_thread_safe();  // Disables thread safety globally
```

Quiet mode
```cpp
log_set_quiet(unsigned int quiet); // If enabled do not print to stderr.
```

Set global warning level
```cpp
log_set_level(int level); /* Set the minimum warning level. */
```

Output log to file
```cpp
log_output_file(void *file_descriptor); // Pass a file pointer provided by the C standart library.
log_disable_output_file();             // Remove file descriptor. Disable logging to file.
```
