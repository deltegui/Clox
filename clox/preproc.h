#ifndef clox_preproc_h
#define clox_preproc_h

#include "scanner.h"

Token scan_preproc_token();
void init_preproc(const char* source);
void free_preproc();

#endif
