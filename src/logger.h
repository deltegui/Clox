#ifndef LOGGER_H
#define LOGGER_H

/**
 *  Logs to stderr a message and exits with code.
 */
void log_fatal(int code, char* message);

/**
 *  Logs to stderr a message with line and exits with code.
 */
void log_fatal_line(int code, int line, char* message);

/**
 * Logs to stderr a message
 */
void log_error(int line, char* message);

#endif