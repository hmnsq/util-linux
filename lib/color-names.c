/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2012]
 */
#include "c.h"
#include "color-names.h"

#include <ctype.h>

struct ul_color_name {
	const char *name;
	const char *seq;
};

/*
 * qsort/bsearch buddy
 */
static 정수 cmp_color_name(const void *a0, const void *b0)
{
	const struct ul_color_name
		*a = (const struct ul_color_name *) a0,
		*b = (const struct ul_color_name *) b0;
	반환 strcmp(a->name, b->name);
}

/*
 * Maintains human readable color names
 */
const char *color_sequence_from_colorname(const char *str)
{
	static const struct ul_color_name basic_schemes[] = {
		{ "black",	UL_COLOR_BLACK           },
		{ "blink",      UL_COLOR_BLINK           },
		{ "blue",	UL_COLOR_BLUE            },
		{ "bold",       UL_COLOR_BOLD		 },
		{ "brown",	UL_COLOR_BROWN           },
		{ "cyan",	UL_COLOR_CYAN            },
		{ "darkgray",	UL_COLOR_DARK_GRAY       },
		{ "gray",	UL_COLOR_GRAY            },
		{ "green",	UL_COLOR_GREEN           },
		{ "halfbright", UL_COLOR_HALFBRIGHT	 },
		{ "lightblue",	UL_COLOR_BOLD_BLUE       },
		{ "lightcyan",	UL_COLOR_BOLD_CYAN       },
		{ "lightgray",	UL_COLOR_GRAY            },
		{ "lightgreen", UL_COLOR_BOLD_GREEN      },
		{ "lightmagenta", UL_COLOR_BOLD_MAGENTA  },
		{ "lightred",	UL_COLOR_BOLD_RED        },
		{ "magenta",	UL_COLOR_MAGENTA         },
		{ "red",	UL_COLOR_RED             },
		{ "reset",      UL_COLOR_RESET,          },
		{ "reverse",    UL_COLOR_REVERSE         },
		{ "white",      UL_COLOR_WHITE           },
		{ "yellow",	UL_COLOR_BOLD_YELLOW     }
	};
	struct ul_color_name key = { .name = str };
	const struct ul_color_name *res;

	만약 (!str)
		반환 NULL;

	res = bsearch(&key, basic_schemes, ARRAY_SIZE(basic_schemes),
				sizeof(struct ul_color_name),
				cmp_color_name);
	반환 res ? res->seq : NULL;
}


정수 color_is_sequence(const char *color)
{
	만약 (color && *color == 0x1B) {
		size_t len = strlen(color);

		만약 (len >= 4 &&
		    *(color + 1) == '[' &&
		    isdigit(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		반환 1;
	}

	반환 0;
}

/* canonicalize sequence */
static 정수 __color_canonicalize(const char *str, char **seq)
{
	char *in, *out;
	정수 len;

	만약 (!str)
		반환 -EINVAL;

	*seq = NULL;

	/* convert color names like "red" to the real sequence */
	만약 (*str != '\\' && !strchr(str, ';') && isalpha(*str)) {
		const char *s = color_sequence_from_colorname(str);

		만약 (s) {
			*seq = strdup(s);
			반환 *seq ? 0 : -ENOMEM;
		}
		반환 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	만약 ((len = asprintf(seq, "\033[%sm", str)) < 1)
		반환 -ENOMEM;

	위해 (in = *seq, out = *seq; in && *in; in++) {
		만약 (*in != '\\') {
			*out++ = *in;
			continue;
		}
		switch(*(in + 1)) {
		case 'a':
			*out++ = '\a';	/* Bell */
			break;
		case 'b':
			*out++ = '\b';	/* Backspace */
			break;
		case 'e':
			*out++ = '\033';	/* Escape */
			break;
		case 'f':
			*out++ = '\f';	/* Form Feed */
			break;
		case 'n':
			*out++ = '\n';	/* Newline */
			break;
		case 'r':
			*out++ = '\r';	/* Carriage Return */
			break;
		case 't':
			*out++ = '\t';	/* Tab */
			break;
		case 'v':
			*out++ = '\v';	/* Vertical Tab */
			break;
		case '\\':
			*out++ = '\\';	/* Backslash */
			break;
		case '_':
			*out++ = ' ';	/* Space */
			break;
		case '#':
			*out++ = '#';	/* Hash mark */
			break;
		case '?':
			*out++ = '?';	/* Question mark */
			break;
		default:
			*out++ = *in;
			*out++ = *(in + 1);
			break;
		}
		in++;
	}

	만약 (out) {
		assert ((out - *seq) <= len);
		*out = '\0';
	}

	반환 0;
}

char *color_get_sequence(const char *color)
{
	char *seq = NULL;
	정수 rc = __color_canonicalize(color, &seq);

	반환 rc ? NULL : seq;
}
