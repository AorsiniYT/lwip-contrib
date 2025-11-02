#ifndef _STRINGS_H_
#define _STRINGS_H_

#include <string.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Thread-safe version of strtok
char *strtok_r(char *str, const char *delim, char **saveptr);

// Get string length with maximum limit
size_t strnlen(const char *s, size_t maxlen);

#ifdef __cplusplus
}
#endif

#endif /* _STRINGS_H_ */