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
static 整数 cmp_color_name(const void *a0, const void *b0)
{
	const struct ul_color_name
		*a = (const struct ul_color_name *) a0,
		*b = (const struct ul_color_name *) b0;
	返回 strcmp(a->name, b->name);
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

	如果 (!str)
		返回 NULL;

	res = bsearch(&key, basic_schemes, ARRAY_SIZE(basic_schemes),
				sizeof(struct ul_color_name),
				cmp_color_name);
	返回 res ? res->seq : NULL;
}


整数 color_is_sequence(const char *color)
{
	如果 (color && *color == 0x1B) {
		size_t len = strlen(color);

		如果 (len >= 4 &&
		    *(color + 1) == '[' &&
		    isdigit(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		返回 1;
	}

	返回 0;
}

/* canonicalize sequence */
static 整数 __color_canonicalize(const char *str, char **seq)
{
	char *in, *out;
	整数 len;

	如果 (!str)
		返回 -EINVAL;

	*seq = NULL;

	/* convert color names like "red" to the real sequence */
	如果 (*str != '\\' && !strchr(str, ';') && isalpha(*str)) {
		const char *s = color_sequence_from_colorname(str);

		如果 (s) {
			*seq = strdup(s);
			返回 *seq ? 0 : -ENOMEM;
		}
		返回 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	如果 ((len = asprintf(seq, "\033[%sm", str)) < 1)
		返回 -ENOMEM;

	为 (in = *seq, out = *seq; in && *in; in++) {
		如果 (*in != '\\') {
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

	如果 (out) {
		assert ((out - *seq) <= len);
		*out = '\0';
	}

	返回 0;
}

char *color_get_sequence(const char *color)
{
	char *seq = NULL;
	整数 rc = __color_canonicalize(color, &seq);

	返回 rc ? NULL : seq;
}
