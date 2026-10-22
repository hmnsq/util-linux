/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2012]
 */
#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "color-names.h"

#ଅନ୍ତର୍ଭାଵ <ctype.h>

ସଂରଚନା ul_color_name {
	ଅଚଳ ଅକ୍ଷର *name;
	ଅଚଳ ଅକ୍ଷର *seq;
};

/*
 * qsort/bsearch buddy
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmp_color_name(ଅଚଳ ଶୂନ୍ୟ *a0, ଅଚଳ ଶୂନ୍ୟ *b0)
{
	ଅଚଳ ସଂରଚନା ul_color_name
		*a = (ଅଚଳ ସଂରଚନା ul_color_name *) a0,
		*b = (ଅଚଳ ସଂରଚନା ul_color_name *) b0;
	ଫେରନ୍ତୁ strcmp(a->name, b->name);
}

/*
 * Maintains human readable color names
 */
ଅଚଳ ଅକ୍ଷର *color_sequence_from_colorname(ଅଚଳ ଅକ୍ଷର *str)
{
	ସ୍ଥିର ଅଚଳ ସଂରଚନା ul_color_name basic_schemes[] = {
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
	ସଂରଚନା ul_color_name ଚାବି = { .name = str };
	ଅଚଳ ସଂରଚନା ul_color_name *res;

	ଯଦି (!str)
		ଫେରନ୍ତୁ ଖାଲି;

	res = bsearch(&ଚାବି, basic_schemes, ARRAY_SIZE(basic_schemes),
				ଆକାର(ସଂରଚନା ul_color_name),
				cmp_color_name);
	ଫେରନ୍ତୁ res ? res->seq : ଖାଲି;
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା color_is_sequence(ଅଚଳ ଅକ୍ଷର *color)
{
	ଯଦି (color && *color == 0x1B) {
		size_t len = strlen(color);

		ଯଦି (len >= 4 &&
		    *(color + 1) == '[' &&
		    isdigit(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		ଫେରନ୍ତୁ 1;
	}

	ଫେରନ୍ତୁ 0;
}

/* canonicalize sequence */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __color_canonicalize(ଅଚଳ ଅକ୍ଷର *str, ଅକ୍ଷର **seq)
{
	ଅକ୍ଷର *in, *out;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;

	ଯଦି (!str)
		ଫେରନ୍ତୁ -EINVAL;

	*seq = ଖାଲି;

	/* convert color names like "red" to the real sequence */
	ଯଦି (*str != '\\' && !strchr(str, ';') && isalpha(*str)) {
		ଅଚଳ ଅକ୍ଷର *s = color_sequence_from_colorname(str);

		ଯଦି (s) {
			*seq = strdup(s);
			ଫେରନ୍ତୁ *seq ? 0 : -ENOMEM;
		}
		ଫେରନ୍ତୁ 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	ଯଦି ((len = asprintf(seq, "\033[%sm", str)) < 1)
		ଫେରନ୍ତୁ -ENOMEM;

	ପାଇଁ (in = *seq, out = *seq; in && *in; in++) {
		ଯଦି (*in != '\\') {
			*out++ = *in;
			ଜାରି;
		}
		ବାଛ(*(in + 1)) {
		ସ୍ଥିତି 'a':
			*out++ = '\a';	/* Bell */
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'b':
			*out++ = '\b';	/* Backspace */
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'e':
			*out++ = '\033';	/* Escape */
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'f':
			*out++ = '\f';	/* Form Feed */
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'n':
			*out++ = '\n';	/* Newline */
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'r':
			*out++ = '\r';	/* Carriage Return */
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 't':
			*out++ = '\t';	/* Tab */
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'v':
			*out++ = '\v';	/* Vertical Tab */
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '\\':
			*out++ = '\\';	/* Backslash */
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '_':
			*out++ = ' ';	/* Space */
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '#':
			*out++ = '#';	/* Hash mark */
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '?':
			*out++ = '?';	/* Question mark */
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			*out++ = *in;
			*out++ = *(in + 1);
			ଭାଙ୍ଗ;
		}
		in++;
	}

	ଯଦି (out) {
		ପ୍ରତିଜ୍ଞା ((out - *seq) <= len);
		*out = '\0';
	}

	ଫେରନ୍ତୁ 0;
}

ଅକ୍ଷର *color_get_sequence(ଅଚଳ ଅକ୍ଷର *color)
{
	ଅକ୍ଷର *seq = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = __color_canonicalize(color, &seq);

	ଫେରନ୍ତୁ rc ? ଖାଲି : seq;
}
