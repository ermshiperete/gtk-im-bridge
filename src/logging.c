#include "logging.h"
#include <glib/gprintf.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>

static FILE *log_file = NULL;

static const char *log_file_path = "/tmp/gtk-im-bridge.log";

static void
logging_ensure_open(void)
{
  if (log_file == NULL)
    {
      log_file = fopen(log_file_path, "a");
      if (log_file == NULL)
        {
          g_warning("Failed to open log file %s", log_file_path);
          return;
        }
      setvbuf(log_file, NULL, _IOLBF, 0); /* Line buffered */
    }
}

static char *
get_timestamp(void)
{
  static char timestamp[32];
  time_t now = time(NULL);
  struct tm *timeinfo = localtime(&now);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
  return timestamp;
}

void
logging_init(void)
{
  logging_ensure_open();
  if (log_file)
    {
      fprintf(log_file, "\n========================================\n");
      fprintf(log_file, "GTK-IM-Bridge initialized at %s\n", get_timestamp());
      fprintf(log_file, "========================================\n");
      fflush(log_file);
    }
}

void
logging_function_enter(const char *function_name, const char *params_format, ...)
{
  logging_ensure_open();
  if (log_file)
    {
      va_list args;
      fprintf(log_file, "[%s] >> %s(", get_timestamp(), function_name);
      
      if (params_format && strlen(params_format) > 0)
        {
          va_start(args, params_format);
          vfprintf(log_file, params_format, args);
          va_end(args);
        }
      
      fprintf(log_file, ")\n");
      fflush(log_file);
    }
}

void
logging_function_exit(const char *function_name, const char *result_format, ...)
{
  logging_ensure_open();
  if (log_file)
    {
      va_list args;
      fprintf(log_file, "[%s] << %s returns: ", get_timestamp(), function_name);
      
      if (result_format && strlen(result_format) > 0)
        {
          va_start(args, result_format);
          vfprintf(log_file, result_format, args);
          va_end(args);
        }
      else
        {
          fprintf(log_file, "(void)");
        }
      
      fprintf(log_file, "\n");
      fflush(log_file);
    }
}

void
logging_signal(const char *signal_name, const char *details_format, ...)
{
  logging_ensure_open();
  if (log_file)
    {
      va_list args;
      fprintf(log_file, "[%s] SIGNAL: %s", get_timestamp(), signal_name);
      
      if (details_format && strlen(details_format) > 0)
        {
          fprintf(log_file, " | ");
          va_start(args, details_format);
          vfprintf(log_file, details_format, args);
          va_end(args);
        }
      
      fprintf(log_file, "\n");
      fflush(log_file);
    }
}

void
logging_error(const char *format, ...)
{
  logging_ensure_open();
  if (log_file)
    {
      va_list args;
      fprintf(log_file, "[%s] ERROR: ", get_timestamp());
      va_start(args, format);
      vfprintf(log_file, format, args);
      va_end(args);
      fprintf(log_file, "\n");
      fflush(log_file);
    }
}
