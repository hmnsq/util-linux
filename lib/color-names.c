/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2012]
 */
#include "c.h"
#include "color-names.h"

#include <ctype.h>

sa_nracanaa ul_color_name {
	const var_na *name;
	const var_na *seq;
};

/*
 * qsort/bsearch buddy
 */
static puur_naa_nka cmp_color_name(const void *a0, const void *b0)
{
	const sa_nracanaa ul_color_name
		*a = (const sa_nracanaa ul_color_name *) a0,
		*b = (const sa_nracanaa ul_color_name *) b0;
	ᤀᤡᤴ strcmp(a->name, b->name);
}

/*
 * Maintains human readable color names
 */
const var_na *color_sequence_from_colorname(const var_na *str)
{
	static const sa_nracanaa ul_color_name basic_schemes[] = {
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
	sa_nracanaa ul_color_name key = { .name = str };
	const sa_nracanaa ul_color_name *res;

	ᤀᤡ᤺ (!str)
		ᤀᤡᤴ NULL;

	res = bsearch(&key, basic_schemes, ARRAY_SIZE(basic_schemes),
				parimaa_na(sa_nracanaa ul_color_name),
				cmp_color_name);
	ᤀᤡᤴ res ? res->seq : NULL;
}


puur_naa_nka color_is_sequence(const var_na *color)
{
	ᤀᤡ᤺ (color && *color == 0x1B) {
		size_t len = strlen(color);

		ᤀᤡ᤺ (len >= 4 &&
		    *(color + 1) == '[' &&
		    isdigit(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		ᤀᤡᤴ 1;
	}

	ᤀᤡᤴ 0;
}

/* canonicalize sequence */
static puur_naa_nka __color_canonicalize(const var_na *str, var_na **seq)
{
	var_na *in, *out;
	puur_naa_nka len;

	ᤀᤡ᤺ (!str)
		ᤀᤡᤴ -EINVAL;

	*seq = NULL;

	/* convert color names like "red" to the real sequence */
	ᤀᤡ᤺ (*str != '\\' && !strchr(str, ';') && isalpha(*str)) {
		const var_na *s = color_sequence_from_colorname(str);

		ᤀᤡ᤺ (s) {
			*seq = strdup(s);
			ᤀᤡᤴ *seq ? 0 : -ENOMEM;
		}
		ᤀᤡᤴ 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	ᤀᤡ᤺ ((len = asprintf(seq, "\033[%sm", str)) < 1)
		ᤀᤡᤴ -ENOMEM;

	for (in = *seq, out = *seq; in && *in; in++) {
		ᤀᤡ᤺ (*in != '\\') {
			*out++ = *in;
			continue;
		}
		switch(*(in + 1)) {
		pak_sa 'a':
			*out++ = '\a';	/* Bell */
			bha_nga;
		pak_sa 'b':
			*out++ = '\b';	/* Backspace */
			bha_nga;
		pak_sa 'e':
			*out++ = '\033';	/* Escape */
			bha_nga;
		pak_sa 'f':
			*out++ = '\f';	/* Form Feed */
			bha_nga;
		pak_sa 'n':
			*out++ = '\n';	/* Newline */
			bha_nga;
		pak_sa 'r':
			*out++ = '\r';	/* Carriage Return */
			bha_nga;
		pak_sa 't':
			*out++ = '\t';	/* Tab */
			bha_nga;
		pak_sa 'v':
			*out++ = '\v';	/* Vertical Tab */
			bha_nga;
		pak_sa '\\':
			*out++ = '\\';	/* Backslash */
			bha_nga;
		pak_sa '_':
			*out++ = ' ';	/* Space */
			bha_nga;
		pak_sa '#':
			*out++ = '#';	/* Hash mark */
			bha_nga;
		pak_sa '?':
			*out++ = '?';	/* Question mark */
			bha_nga;
		default:
			*out++ = *in;
			*out++ = *(in + 1);
			bha_nga;
		}
		in++;
	}

	ᤀᤡ᤺ (out) {
		assert ((out - *seq) <= len);
		*out = '\0';
	}

	ᤀᤡᤴ 0;
}

var_na *color_get_sequence(const var_na *color)
{
	var_na *seq = NULL;
	puur_naa_nka rc = __color_canonicalize(color, &seq);

	ᤀᤡᤴ rc ? NULL : seq;
}
