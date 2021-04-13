#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <stdarg.h>

#define CRITSTR "\e[91m[CRITICAL]:\e[0m "
#define INFOSTR "\e[92m[INFO]:\e[0m "
#define ERRSTR  "\e[93m[ERROR]:\e[0m "

#define MAXLOGLEN (10241)


/* Initializes the logger output file
    to 'path'. STDOUT is used if 'path'
    is NULL */
void init_log_manager(const char* path);


/* Used for logging basic server info */
void log_info(const char* fmt, ...);


/* Used for logging server errors */
void log_err(const char* fmt, ...);


/* Used for logging critical server errors
    (This will send sigterm to terminate the program) */
void log_crit(const char* fmt, ...);


#endif
