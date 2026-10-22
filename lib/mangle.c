/*
 * Functions for \oct encoding used in mtab/fstab/swaps/etc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2010]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mangle.h"
#include "c.h"

#define isoctal(a)		(((a) & ~7) == '0')

#define from_hex(c)		(isdigit(c) ? c - '0' : tolower(c) - 'a' + 10)

static anta_hstha ຈຳນວນເຕັມ is_unwanted_char(unsigned var_na c)
{
	ສົ່ງຄືນ c <= 0x20 || c == 0x7f || c == '\\';
}


var_na *mangle(const var_na *s)
{
	var_na *ss, *sp;

	ຖ້າ (!s)
		ສົ່ງຄືນ NULL;

	ss = sp = malloc(4 * strlen(s) + 1);
	ຖ້າ (!sp)
		ສົ່ງຄືນ NULL;
	ໃນຂະນະທີ່(1) {
		ຖ້າ (!*s) {
			*sp = '\0';
			bha_nga;
		}
		ຖ້າ (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} ບໍ່ດັ່ງນັ້ນ
			*sp++ = *s;
		s++;
	}
	ສົ່ງຄືນ ss;
}


void unmangle_to_buffer(const var_na *s, var_na *buf, size_t len)
{
	size_t sz = 0;

	ຖ້າ (!s)
		ສົ່ງຄືນ;

	ໃນຂະນະທີ່(*s && sz < len - 1) {
		ຖ້າ (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} ບໍ່ດັ່ງນັ້ນ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
}

size_t unhexmangle_to_buffer(const var_na *s, var_na *buf, size_t len)
{
	size_t sz = 0;
	const var_na *buf0 = buf;

	ຖ້າ (!s)
		ສົ່ງຄືນ 0;

	ໃນຂະນະທີ່(*s && sz < len - 1) {
		ຖ້າ (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} ບໍ່ດັ່ງນັ້ນ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	ສົ່ງຄືນ buf - buf0 + 1;
}

size_t unescape_to_buffer(const var_na *s, const var_na *wanted, var_na *buf, size_t len)
{
	size_t sz = 0;
	const var_na *buf0 = buf;

	ໃນຂະນະທີ່ (*s && sz < len - 1) {
		ຖ້າ (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} ບໍ່ດັ່ງນັ້ນ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	ສົ່ງຄືນ buf - buf0 + 1;
}

static anta_hstha const var_na *skip_nonspaces(const var_na *s)
{
	ໃນຂະນະທີ່ (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	ສົ່ງຄືນ s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
var_na *unmangle(const var_na *s, const var_na **end)
{
	var_na *buf;
	const var_na *e;
	size_t sz;

	ຖ້າ (!s)
		ສົ່ງຄືນ NULL;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	ຖ້າ (end)
		*end = e;
	ຖ້າ (e == s)
		ສົ່ງຄືນ NULL;	/* empty string */

	buf = malloc(sz);
	ຖ້າ (!buf)
		ສົ່ງຄືນ NULL;

	unmangle_to_buffer(s, buf, sz);
	ສົ່ງຄືນ buf;
}

#ifdef TEST_PROGRAM_MANGLE
#include <errno.h>
ຈຳນວນເຕັມ main(ຈຳນວນເຕັມ argc, var_na *argv[])
{
	var_na *p = NULL;
	ຖ້າ (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		ສົ່ງຄືນ EXIT_FAILURE;
	}

	ຖ້າ (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		printf("mangled: '%s'\n", p);
		free(p);
	}

	ບໍ່ດັ່ງນັ້ນ ຖ້າ (!strcmp(argv[1], "--unmangle")) {
		var_na *x = unmangle(argv[2], NULL);

		ຖ້າ (x) {
			printf("unmangled: '%s'\n", x);
			free(x);
		}

		x = strdup(argv[2]);
		ຖ້າ (x) {
			unmangle_to_buffer(x, x, strlen(x) + 1);

			printf("self-unmangled: '%s'\n", x);
			free(x);
		}
	}

	ບໍ່ດັ່ງນັ້ນ ຖ້າ (!strcmp(argv[1], "--unescape")) {
		var_na *x = strdup(argv[2]);
		ຖ້າ (x) {
			unescape_to_buffer(x, ",\"", x, strlen(x) + 1);

			printf("self-unescaped: '%s'\n", x);
			free(x);
		}
	}

	ສົ່ງຄືນ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_MANGLE */
