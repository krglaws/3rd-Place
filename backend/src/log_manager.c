#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>

#include <log_manager.h>


static void log_all(const char* logtype, const char* fmt, va_list ap);
static void terminate_log_manager();


// this is the output file for all logs
// which can be configured via CLI args
static FILE* logfile = NULL;


// temporary storage buffer for incoming log messages
static char logbuff[MAXLOGLEN + 1];


static void log_all(const char* logtype, const char* fmt, va_list ap)
{
  // get datetime
  char timebuff[32];
  time_t rawtime = time(NULL);
  struct tm* ptm = localtime(&rawtime);
  strftime(timebuff, 32, "%x %T", ptm);

  // process format string
  vsprintf(logbuff, fmt, ap);

  int len;
  char* buffptr = logbuff;
  char* nlloc;

  // for each '\n' char in message,
  // print preceding 'logtype' string
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

    fprintf(logfile, "(%s)%s%.*s\n", timebuff, logtype, len, buffptr);
    buffptr += (len + 1);

  } while (*buffptr);

  memset(logbuff, 0, MAXLOGLEN + 1);
}


void init_log_manager(const char* path)
{

  if (path == NULL || strlen(path) == 0)
  {
    logfile = stdout;
  }

  else if ((logfile = fopen(path, "a")) == NULL)
  {
    fprintf(stderr, "init_logger(): failed to open log file '%s'\n", path);
    exit(EXIT_FAILURE);
  }

  atexit(&terminate_log_manager);

  log_info("Log Manager Initialized.");
}


static void terminate_log_manager()
{
  log_info("Terminating Log Manager...");
  fclose(logfile);
}


void log_info(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  log_all(INFOSTR, fmt, ap);

  va_end(ap);
}


void log_err(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  log_all(ERRSTR, fmt, ap);

  va_end(ap);
}


void log_crit(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  log_all(CRITSTR, fmt, ap);

  va_end(ap);

  // terminate server process gracefully
  raise(SIGTERM);
}
