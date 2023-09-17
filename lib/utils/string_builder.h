#ifndef _STRING_BUILDER_H_INCLUDE_GUARD
#define _STRING_BUILDER_H_INCLUDE_GUARD

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <inttypes.h>

#define STRBLD_OK 0
#define STRBLD_FAIL 1
#define STRBLD_TRUNCATED 2

#define STRBLD_NPOS SIZE_MAX

/** Newline character(s) to use when building strings. */
#ifndef STRBLD_NEWLINE
#define STRBLD_NEWLINE "\n"
#endif

typedef struct strbld_t
{
    char* buffer;
    size_t size;
    size_t capacity;
} strbld_t;

int strbld_init(strbld_t* sb, char* buffer, size_t size);

int strbld_append(strbld_t* sb, const char* value);

int strbld_append_char(strbld_t* sb, char value);

int strbld_append_line(strbld_t* sb, const char* value);

int strbld_append_html(strbld_t* sb, const char* value, const char* html_tag);

const char* strbld_get(strbld_t* sb, size_t* strlen);

#ifdef __cplusplus
}
#endif

#endif // _STRING_BUILDER_H_INCLUDE_GUARD
