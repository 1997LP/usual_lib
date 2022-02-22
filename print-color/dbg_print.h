#include <stdio.h>
#include <stdarg.h>

#define PRINT_FONT_RED "\033[31m"
#define PRINT_FONT_YEL "\033[33m"
#define PRINT_FONT_BLU "\033[34m"
#define PRINT_FONT_WHI "\033[37m"

static inline  void print_info(const char *info, ...)
{
	va_list params;
	char msg[512];
	va_start(params, info);
	vsnprintf(msg, sizeof(msg), info, params);
	fprintf(stderr, "%s%s", " Info:    ", msg);
	va_end(params);
}
static inline void print_warning(const char *warn, ...)
{
	va_list params;
	char msg[512];
	va_start(params, warn);
	vsnprintf(msg, sizeof(msg), warn, params);
	fprintf(stderr, "%s%s", " Warning: ", msg);	
	va_end(params);
}
static inline  void print_error(const char *error, ...)
{
	va_list params;
	char msg[512];
	va_start(params, error);
	vsnprintf(msg, sizeof(msg), error, params);
	fprintf(stderr, "%s%s", " Error:   ", msg);	
	va_end(params);
}

#define pr_info(fmt, ...) \
	do {                  \
		print_info(PRINT_FONT_BLU"[%s:%d] "PRINT_FONT_WHI fmt, \
				__func__, __LINE__, ##__VA_ARGS__); \
	} while (0);

#define pr_warning(fmt, ...) \
	do { 					\
		print_warning(PRINT_FONT_YEL"[%s:%d] "PRINT_FONT_WHI fmt, \
				__func__, __LINE__, ##__VA_ARGS__); \
	} while (0); 	

#define pr_err(fmt, ...) \
	do {				\
		print_error(PRINT_FONT_RED"[%s:%d] "PRINT_FONT_WHI fmt, \
				__func__, __LINE__, ##__VA_ARGS__); \
	} while (0);






