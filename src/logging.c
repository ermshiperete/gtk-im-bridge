#include "logging.h"
#include <glib/gprintf.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>

static FILE *log_file = NULL;

static const char *log_file_path = "/tmp/gtk-im-bridge.log";

static void
logging_truncate()
{
  FILE *tmp = fopen(log_file_path, "w");
  fclose(tmp);
}

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

static void
logging_flush()
{
  if (log_file != NULL) {
    fflush(log_file);
    fclose(log_file);
    log_file = NULL;
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

static int indent_level = 0;

static void
indent(int level)
{
  if (indent_level < 0) {
    printf("WARNING: indent_level=%d\n", indent_level);
    fprintf(log_file, "WARNING: indent_level=%d\n", indent_level);
  }
  printf("[%s] ", get_timestamp());
  fprintf(log_file, "[%s] ", get_timestamp());
  for (int i = 0; i < level; i++)
  {
    printf("  ");
    fprintf(log_file, "  ");
  }
}

void logging_init(void)
{
  logging_truncate();
  logging_ensure_open();
  printf("\n========================================\n");
  printf("GTK-IM-Bridge initialized at %s\n", get_timestamp());
  printf("========================================\n");
  if (log_file)
  {
    fprintf(log_file, "\n========================================\n");
    fprintf(log_file, "GTK-IM-Bridge initialized at %s\n", get_timestamp());
    fprintf(log_file, "========================================\n");
    logging_flush();
  }
}

void logging_function_enter(const char *function_name, const char *params_format, ...)
{
  logging_ensure_open();
  if (log_file)
  {
    va_list args;
    indent(indent_level++);
    fprintf(log_file, ">> %s(", function_name);
    printf(">> %s(", function_name);

    if (params_format && strlen(params_format) > 0)
    {
      va_start(args, params_format);
      va_list args_copy;
      va_copy(args_copy, args);
      vfprintf(log_file, params_format, args);
      vprintf(params_format, args_copy);
      va_end(args_copy);
      va_end(args);
    }

    fprintf(log_file, ")\n");
    printf(")\n");
    logging_flush();
  }
}

void logging_function_exit(const char *function_name, const char *result_format, ...)
{
  logging_ensure_open();
  indent(--indent_level);
  if (log_file)
  {
    va_list args;
    fprintf(log_file, "<< %s returns: ", function_name);
    printf("<< %s returns: ", function_name);

    if (result_format && strlen(result_format) > 0)
    {
      va_start(args, result_format);
      va_list args_copy;
      va_copy(args_copy, args);
      vfprintf(log_file, result_format, args);
      vprintf(result_format, args_copy);
      va_end(args_copy);
      va_end(args);
    }
    else
    {
      fprintf(log_file, "(void)");
      printf("(void)");
    }

    fprintf(log_file, "\n");
    printf("\n");
    logging_flush();
  }
}

void logging_signal(const char *signal_name, const char *details_format, ...)
{
  logging_ensure_open();
  if (log_file)
  {
    va_list args;
    indent(indent_level);
    fprintf(log_file, "SIGNAL: %s", signal_name);
    printf("SIGNAL: %s", signal_name);

    if (details_format && strlen(details_format) > 0)
    {
      fprintf(log_file, " | ");
      printf(" | ");
      va_start(args, details_format);
      va_list args_copy;
      va_copy(args_copy, args);
      vfprintf(log_file, details_format, args);
      vprintf(details_format, args_copy);
      va_end(args_copy);
      va_end(args);
    }

    fprintf(log_file, "\n");
    printf("\n");
    logging_flush();
  }
}

void logging_error(const char *format, ...)
{
  logging_ensure_open();
  if (log_file)
  {
    va_list args;
    indent(indent_level);
    fprintf(log_file, "ERROR: ");
    printf("ERROR: ");
    va_start(args, format);
    va_list args_copy;
    va_copy(args_copy, args);
    vfprintf(log_file, format, args);
    vprintf(format, args_copy);
    va_end(args_copy);
    va_end(args);
    fprintf(log_file, "\n");
    printf("\n");
    logging_flush();
  }
}

void logging_message(const char *format, ...)
{
  logging_ensure_open();
  if (log_file)
  {
    va_list args;
    indent(indent_level);
    va_start(args, format);
    va_list args_copy;
    va_copy(args_copy, args);
    vfprintf(log_file, format, args);
    vprintf(format, args_copy);
    va_end(args_copy);
    va_end(args);
    fprintf(log_file, "\n");
    printf("\n");
    logging_flush();
  }
}
