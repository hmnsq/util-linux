/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2012]
 */
#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "color-names.h"

#અન્તર્ભાવ <ctype.h>

સંરચના ul_color_name {
	અચલ અક્ષર *name;
	અચલ અક્ષર *seq;
};

/*
 * qsort/bsearch buddy
 */
સ્થિર પૂર્ણાંક cmp_color_name(અચલ શૂન્ય *a0, અચલ શૂન્ય *b0)
{
	અચલ સંરચના ul_color_name
		*a = (અચલ સંરચના ul_color_name *) a0,
		*b = (અચલ સંરચના ul_color_name *) b0;
	પાછા strcmp(a->name, b->name);
}

/*
 * Maintains human readable color names
 */
અચલ અક્ષર *color_sequence_from_colorname(અચલ અક્ષર *str)
{
	સ્થિર અચલ સંરચના ul_color_name basic_schemes[] = {
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
	સંરચના ul_color_name ચાવી = { .name = str };
	અચલ સંરચના ul_color_name *res;

	જો (!str)
		પાછા ખાલી;

	res = bsearch(&ચાવી, basic_schemes, ARRAY_SIZE(basic_schemes),
				કદ(સંરચના ul_color_name),
				cmp_color_name);
	પાછા res ? res->seq : ખાલી;
}


પૂર્ણાંક color_is_sequence(અચલ અક્ષર *color)
{
	જો (color && *color == 0x1B) {
		size_t len = strlen(color);

		જો (len >= 4 &&
		    *(color + 1) == '[' &&
		    isdigit(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		પાછા 1;
	}

	પાછા 0;
}

/* canonicalize sequence */
સ્થિર પૂર્ણાંક __color_canonicalize(અચલ અક્ષર *str, અક્ષર **seq)
{
	અક્ષર *in, *out;
	પૂર્ણાંક len;

	જો (!str)
		પાછા -EINVAL;

	*seq = ખાલી;

	/* convert color names like "red" to the real sequence */
	જો (*str != '\\' && !strchr(str, ';') && isalpha(*str)) {
		અચલ અક્ષર *s = color_sequence_from_colorname(str);

		જો (s) {
			*seq = strdup(s);
			પાછા *seq ? 0 : -ENOMEM;
		}
		પાછા 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	જો ((len = asprintf(seq, "\033[%sm", str)) < 1)
		પાછા -ENOMEM;

	માટે (in = *seq, out = *seq; in && *in; in++) {
		જો (*in != '\\') {
			*out++ = *in;
			ચાલુ;
		}
		પસંદગી(*(in + 1)) {
		સ્થિતિ 'a':
			*out++ = '\a';	/* Bell */
			તોડો;
		સ્થિતિ 'b':
			*out++ = '\b';	/* Backspace */
			તોડો;
		સ્થિતિ 'e':
			*out++ = '\033';	/* Escape */
			તોડો;
		સ્થિતિ 'f':
			*out++ = '\f';	/* Form Feed */
			તોડો;
		સ્થિતિ 'n':
			*out++ = '\n';	/* Newline */
			તોડો;
		સ્થિતિ 'r':
			*out++ = '\r';	/* Carriage Return */
			તોડો;
		સ્થિતિ 't':
			*out++ = '\t';	/* Tab */
			તોડો;
		સ્થિતિ 'v':
			*out++ = '\v';	/* Vertical Tab */
			તોડો;
		સ્થિતિ '\\':
			*out++ = '\\';	/* Backslash */
			તોડો;
		સ્થિતિ '_':
			*out++ = ' ';	/* Space */
			તોડો;
		સ્થિતિ '#':
			*out++ = '#';	/* Hash mark */
			તોડો;
		સ્થિતિ '?':
			*out++ = '?';	/* Question mark */
			તોડો;
		મૂળભૂત:
			*out++ = *in;
			*out++ = *(in + 1);
			તોડો;
		}
		in++;
	}

	જો (out) {
		પ્રતિજ્ઞા ((out - *seq) <= len);
		*out = '\0';
	}

	પાછા 0;
}

અક્ષર *color_get_sequence(અચલ અક્ષર *color)
{
	અક્ષર *seq = ખાલી;
	પૂર્ણાંક rc = __color_canonicalize(color, &seq);

	પાછા rc ? ખાલી : seq;
}
