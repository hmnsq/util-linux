/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2012]
 */
#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "color-names.h"

#അന്തര്ഭാവ <ctype.h>

ഘടന ul_color_name {
	സ്ഥിരം അക്ഷരം *name;
	സ്ഥിരം അക്ഷരം *seq;
};

/*
 * qsort/bsearch buddy
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ cmp_color_name(സ്ഥിരം ശൂന്യം *a0, സ്ഥിരം ശൂന്യം *b0)
{
	സ്ഥിരം ഘടന ul_color_name
		*a = (സ്ഥിരം ഘടന ul_color_name *) a0,
		*b = (സ്ഥിരം ഘടന ul_color_name *) b0;
	തിരികെ strcmp(a->name, b->name);
}

/*
 * Maintains human readable color names
 */
സ്ഥിരം അക്ഷരം *color_sequence_from_colorname(സ്ഥിരം അക്ഷരം *str)
{
	സ്ഥിതം സ്ഥിരം ഘടന ul_color_name basic_schemes[] = {
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
	ഘടന ul_color_name താക്കോൽ = { .name = str };
	സ്ഥിരം ഘടന ul_color_name *res;

	എങ്കിൽ (!str)
		തിരികെ ശൂന്യ;

	res = bsearch(&താക്കോൽ, basic_schemes, ARRAY_SIZE(basic_schemes),
				വലുപ്പം(ഘടന ul_color_name),
				cmp_color_name);
	തിരികെ res ? res->seq : ശൂന്യ;
}


പൂർണ്ണസംഖ്യ color_is_sequence(സ്ഥിരം അക്ഷരം *color)
{
	എങ്കിൽ (color && *color == 0x1B) {
		size_t len = strlen(color);

		എങ്കിൽ (len >= 4 &&
		    *(color + 1) == '[' &&
		    isdigit(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		തിരികെ 1;
	}

	തിരികെ 0;
}

/* canonicalize sequence */
സ്ഥിതം പൂർണ്ണസംഖ്യ __color_canonicalize(സ്ഥിരം അക്ഷരം *str, അക്ഷരം **seq)
{
	അക്ഷരം *in, *out;
	പൂർണ്ണസംഖ്യ len;

	എങ്കിൽ (!str)
		തിരികെ -EINVAL;

	*seq = ശൂന്യ;

	/* convert color names like "red" to the real sequence */
	എങ്കിൽ (*str != '\\' && !strchr(str, ';') && isalpha(*str)) {
		സ്ഥിരം അക്ഷരം *s = color_sequence_from_colorname(str);

		എങ്കിൽ (s) {
			*seq = strdup(s);
			തിരികെ *seq ? 0 : -ENOMEM;
		}
		തിരികെ 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	എങ്കിൽ ((len = asprintf(seq, "\033[%sm", str)) < 1)
		തിരികെ -ENOMEM;

	വേണ്ടി (in = *seq, out = *seq; in && *in; in++) {
		എങ്കിൽ (*in != '\\') {
			*out++ = *in;
			തുടരുക;
		}
		തിരഞ്ഞെടുപ്പ്(*(in + 1)) {
		അവസ്ഥ 'a':
			*out++ = '\a';	/* Bell */
			നിർത്തുക;
		അവസ്ഥ 'b':
			*out++ = '\b';	/* Backspace */
			നിർത്തുക;
		അവസ്ഥ 'e':
			*out++ = '\033';	/* Escape */
			നിർത്തുക;
		അവസ്ഥ 'f':
			*out++ = '\f';	/* Form Feed */
			നിർത്തുക;
		അവസ്ഥ 'n':
			*out++ = '\n';	/* Newline */
			നിർത്തുക;
		അവസ്ഥ 'r':
			*out++ = '\r';	/* Carriage Return */
			നിർത്തുക;
		അവസ്ഥ 't':
			*out++ = '\t';	/* Tab */
			നിർത്തുക;
		അവസ്ഥ 'v':
			*out++ = '\v';	/* Vertical Tab */
			നിർത്തുക;
		അവസ്ഥ '\\':
			*out++ = '\\';	/* Backslash */
			നിർത്തുക;
		അവസ്ഥ '_':
			*out++ = ' ';	/* Space */
			നിർത്തുക;
		അവസ്ഥ '#':
			*out++ = '#';	/* Hash mark */
			നിർത്തുക;
		അവസ്ഥ '?':
			*out++ = '?';	/* Question mark */
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			*out++ = *in;
			*out++ = *(in + 1);
			നിർത്തുക;
		}
		in++;
	}

	എങ്കിൽ (out) {
		പ്രതിജ്ഞാ ((out - *seq) <= len);
		*out = '\0';
	}

	തിരികെ 0;
}

അക്ഷരം *color_get_sequence(സ്ഥിരം അക്ഷരം *color)
{
	അക്ഷരം *seq = ശൂന്യ;
	പൂർണ്ണസംഖ്യ rc = __color_canonicalize(color, &seq);

	തിരികെ rc ? ശൂന്യ : seq;
}
