#ifndef MUDDLED_C_DECL_H
#define MUDDLED_C_DECL_H

/* Kinda miffed and confused why I need this on OSX */

#ifdef __cplusplus
#define BEGIN_DECL extern "C" {
#define END_DECL }
#else
#define BEGIN_DECL
#define END_DECL
#endif

BEGIN_DECL

extern char *strtok(char *, const char *);
#ifndef memcpy
extern void *memcpy(void *, const void *, size_t);
#endif
#ifndef memset
extern void *memset(void *, int, size_t);
#endif
#ifndef strcat
extern char *strcat(char *, const char *);
#endif
#ifndef strncat
extern char *strncat(char *, const char *, size_t);
#endif
extern size_t strlen(const char *);
#ifndef strcpy
extern char *strcpy(char *, const char *);
#endif
#ifndef strncpy
extern char *strncpy(char *, const char *, size_t);
#endif
#ifndef strcmp
extern int strcmp(const char *, const char *);
#endif
#ifndef strcasecmp
extern int strcasecmp(const char *, const char *);
#endif
extern char *strrchr(const char *, int);
#ifndef strchr
extern char *strchr(const char *, int);
#endif
extern char *strstr(const char *, const char *);
#ifndef isascii
extern int isascii(int);
#endif

END_DECL

#endif
