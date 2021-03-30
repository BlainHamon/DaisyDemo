#ifndef __logger_hpp
#define __logger_hpp

#include <Arduino.h>
#include <string.h>

void writeLog(const char * text);
void writeLogln(const char * text = NULL);
void writeLog(String *text);
void writeLogln(String *text);
void writeLogf(const char *format, ...);

#endif