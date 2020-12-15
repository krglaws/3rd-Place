#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>

#include <log_manager.h>


// this is the output file for all logs
// which can be configured via CLI args
static FILE* logfile = NULL;


// temporary storage buffer for incoming log messages
static char logbuff[MAXLOGLEN + 1];


void init_logger(const char* path)
{
  if (path == NULL)
  {
    logfile = stdout;
    return;
  }

  if ((logfile = fopen(path, "a")) == NULL)
  {
    fprintf(stderr, "init_logger(): failed to open log file '%s'\n", path);
    exit(EXIT_FAILURE);
  }
}


void terminate_logger()
{
  fclose(logfile);
}


void log_info(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  __log(INFOSTR, fmt, ap);

  va_end(ap);
}


void log_warn(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  __log(WARNSTR, fmt, ap);

  va_end(ap);
}


void log_err(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  __log(ERRSTR, fmt, ap);

  va_end(ap);
}


void log_crit(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  __log(CRITSTR, fmt, ap);

  va_end(ap);

  // terminate server process gracefully
  raise(SIGTERM);
}


static void __log(const char* logtype, const char* fmt, va_list ap)
{
  // process format string
  vsprintf(logbuff, fmt, ap);

  int len;
  char* buffptr = logbuff;
  char* nlloc;

  // for each '\n' char in message,
  //  print preceding 'logtype' string
  do
  {
    if ((nlloc = strstr(buffptr, "\n")) != NULL)
    {
      len = (int) (nlloc - buffptr);
    }
    else
    {
      len = strlen(buffptr);
    }

    fprintf(logfile, "%s%.*s\n", logtype, len, buffptr);
    buffptr += (len + 1);

  } while (*buffptr);

  memset(logbuff, 0, MAXLOGLEN + 1);
}
