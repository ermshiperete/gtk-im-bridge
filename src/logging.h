#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <stdio.h>

/**
 * Initialize the logging system. Must be called once at startup.
 */
void logging_init(void);

/**
 * Log a function entry with formatted parameters.
 */
void logging_function_enter(const char *function_name, const char *params_format, ...);

/**
 * Log a function exit with a return value (can be NULL for void functions).
 */
void logging_function_exit(const char *function_name, const char *result_format, ...);

/**
 * Log a signal emission.
 */
void logging_signal(const char *signal_name, const char *details_format, ...);

/**
 * Log an error or warning.
 */
void logging_error(const char *format, ...);

void logging_message(const char *format, ...);

#define LOG_ENTER(func, fmt, ...) \
  logging_function_enter(func, fmt, ##__VA_ARGS__)

#define LOG_EXIT(func, fmt, ...) \
  logging_function_exit(func, fmt, ##__VA_ARGS__)

#define LOG_SIGNAL(sig, fmt, ...) \
  logging_signal(sig, fmt, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
  logging_error(fmt, ##__VA_ARGS__)

#define LOG_MESSAGE(fmt, ...) \
  logging_message(fmt, ##__VA_ARGS__)

#endif // __LOGGING_H__
