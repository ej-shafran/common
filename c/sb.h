#ifndef __SB_H
#define __SB_H

#include "./da.h"

typedef da(char) String_Builder;

#define sb_append_cstr(sb, cstr)          \
	do {                              \
		const char *s = (cstr);   \
		int n = strlen(s);        \
		da_append_many(sb, s, n); \
	} while (0);
#define sb_append_nul(sb) da_append(sb, '\0')

#endif // __SB_H
