/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2012]
 */
#அந்தர்பாவ "c.h"
#அந்தர்பாவ "color-names.h"

#அந்தர்பாவ <ctype.h>

கட்டமைப்பு ul_color_name {
	மாறிலி எழுத்து *name;
	மாறிலி எழுத்து *seq;
};

/*
 * qsort/bsearch buddy
 */
நிலையான முழு cmp_color_name(மாறிலி வெறுமை *a0, மாறிலி வெறுமை *b0)
{
	மாறிலி கட்டமைப்பு ul_color_name
		*a = (மாறிலி கட்டமைப்பு ul_color_name *) a0,
		*b = (மாறிலி கட்டமைப்பு ul_color_name *) b0;
	திரும்பு strcmp(a->name, b->name);
}

/*
 * Maintains human readable color names
 */
மாறிலி எழுத்து *color_sequence_from_colorname(மாறிலி எழுத்து *str)
{
	நிலையான மாறிலி கட்டமைப்பு ul_color_name basic_schemes[] = {
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
	கட்டமைப்பு ul_color_name சாவி = { .name = str };
	மாறிலி கட்டமைப்பு ul_color_name *res;

	எனில் (!str)
		திரும்பு வெற்று;

	res = bsearch(&சாவி, basic_schemes, ARRAY_SIZE(basic_schemes),
				அளவு(கட்டமைப்பு ul_color_name),
				cmp_color_name);
	திரும்பு res ? res->seq : வெற்று;
}


முழு color_is_sequence(மாறிலி எழுத்து *color)
{
	எனில் (color && *color == 0x1B) {
		size_t len = strlen(color);

		எனில் (len >= 4 &&
		    *(color + 1) == '[' &&
		    isdigit(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		திரும்பு 1;
	}

	திரும்பு 0;
}

/* canonicalize sequence */
நிலையான முழு __color_canonicalize(மாறிலி எழுத்து *str, எழுத்து **seq)
{
	எழுத்து *in, *out;
	முழு len;

	எனில் (!str)
		திரும்பு -EINVAL;

	*seq = வெற்று;

	/* convert color names like "red" to the real sequence */
	எனில் (*str != '\\' && !strchr(str, ';') && isalpha(*str)) {
		மாறிலி எழுத்து *s = color_sequence_from_colorname(str);

		எனில் (s) {
			*seq = strdup(s);
			திரும்பு *seq ? 0 : -ENOMEM;
		}
		திரும்பு 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	எனில் ((len = asprintf(seq, "\033[%sm", str)) < 1)
		திரும்பு -ENOMEM;

	க்கு (in = *seq, out = *seq; in && *in; in++) {
		எனில் (*in != '\\') {
			*out++ = *in;
			தொடர்;
		}
		தேர்வு(*(in + 1)) {
		நிலை 'a':
			*out++ = '\a';	/* Bell */
			நிறுத்து;
		நிலை 'b':
			*out++ = '\b';	/* Backspace */
			நிறுத்து;
		நிலை 'e':
			*out++ = '\033';	/* Escape */
			நிறுத்து;
		நிலை 'f':
			*out++ = '\f';	/* Form Feed */
			நிறுத்து;
		நிலை 'n':
			*out++ = '\n';	/* Newline */
			நிறுத்து;
		நிலை 'r':
			*out++ = '\r';	/* Carriage Return */
			நிறுத்து;
		நிலை 't':
			*out++ = '\t';	/* Tab */
			நிறுத்து;
		நிலை 'v':
			*out++ = '\v';	/* Vertical Tab */
			நிறுத்து;
		நிலை '\\':
			*out++ = '\\';	/* Backslash */
			நிறுத்து;
		நிலை '_':
			*out++ = ' ';	/* Space */
			நிறுத்து;
		நிலை '#':
			*out++ = '#';	/* Hash mark */
			நிறுத்து;
		நிலை '?':
			*out++ = '?';	/* Question mark */
			நிறுத்து;
		இயல்பு:
			*out++ = *in;
			*out++ = *(in + 1);
			நிறுத்து;
		}
		in++;
	}

	எனில் (out) {
		ப்ரதிஜ்ஞா ((out - *seq) <= len);
		*out = '\0';
	}

	திரும்பு 0;
}

எழுத்து *color_get_sequence(மாறிலி எழுத்து *color)
{
	எழுத்து *seq = வெற்று;
	முழு rc = __color_canonicalize(color, &seq);

	திரும்பு rc ? வெற்று : seq;
}
