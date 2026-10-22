/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2012]
 */
#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "color-names.h"

#ಅನ್ತರ್ಭಾವ <ctype.h>

ರಚನೆ ul_color_name {
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *seq;
};

/*
 * qsort/bsearch buddy
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cmp_color_name(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *a0, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *b0)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ ul_color_name
		*a = (ಸ್ಥಿರಾಂಕ ರಚನೆ ul_color_name *) a0,
		*b = (ಸ್ಥಿರಾಂಕ ರಚನೆ ul_color_name *) b0;
	ಹಿಂತಿರುಗಿ strcmp(a->name, b->name);
}

/*
 * Maintains human readable color names
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *color_sequence_from_colorname(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ ul_color_name basic_schemes[] = {
		{ "black",	UL_COLOR_BLACK           },
		{ "blink",      UL_COLOR_BLINK           },
		{ "blue",	UL_COLOR_BLUE            },
		{ "bold",       UL_COLOR_BOLD		 },
		{ "brown",	UL_COLOR_BROWN           },
		{ "cyan",	UL_COLOR_CYAN            },
		{ "darkgray",	UL_COLOR_DARK_ಬೂದು       },
		{ "gray",	UL_COLOR_ಬೂದು            },
		{ "green",	UL_COLOR_GREEN           },
		{ "halfbright", UL_COLOR_HALFBRIGHT	 },
		{ "lightblue",	UL_COLOR_BOLD_BLUE       },
		{ "lightcyan",	UL_COLOR_BOLD_CYAN       },
		{ "lightgray",	UL_COLOR_ಬೂದು            },
		{ "lightgreen", UL_COLOR_BOLD_GREEN      },
		{ "lightmagenta", UL_COLOR_BOLD_MAGENTA  },
		{ "lightred",	UL_COLOR_BOLD_RED        },
		{ "magenta",	UL_COLOR_MAGENTA         },
		{ "red",	UL_COLOR_RED             },
		{ "reset",      UL_COLOR_RESET,          },
		{ "reverse",    UL_COLOR_ಹಿಮ್ಮುಖ         },
		{ "white",      UL_COLOR_WHITE           },
		{ "yellow",	UL_COLOR_BOLD_YELLOW     }
	};
	ರಚನೆ ul_color_name ಕೀಲಿ = { .name = str };
	ಸ್ಥಿರಾಂಕ ರಚನೆ ul_color_name *res;

	ಆದರೆ (!str)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	res = bsearch(&ಕೀಲಿ, basic_schemes, ARRAY_ಗಾತ್ರ(basic_schemes),
				ಗಾತ್ರ(ರಚನೆ ul_color_name),
				cmp_color_name);
	ಹಿಂತಿರುಗಿ res ? res->seq : ಖಾಲಿ;
}


ಪೂರ್ಣಾಂಕ color_is_sequence(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *color)
{
	ಆದರೆ (color && *color == 0x1B) {
		size_t len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(color);

		ಆದರೆ (len >= 4 &&
		    *(color + 1) == '[' &&
		    ಅಂಕಿಯೇ(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		ಹಿಂತಿರುಗಿ 1;
	}

	ಹಿಂತಿರುಗಿ 0;
}

/* canonicalize sequence */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __color_canonicalize(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಅಕ್ಷರ **seq)
{
	ಅಕ್ಷರ *ಒಳ, *ಹೊರಹೋಗು;
	ಪೂರ್ಣಾಂಕ len;

	ಆದರೆ (!str)
		ಹಿಂತಿರುಗಿ -EINVAL;

	*seq = ಖಾಲಿ;

	/* convert color names like "red" to the real sequence */
	ಆದರೆ (*str != '\\' && !strchr(str, ';') && ಅಕ್ಷರವೇ(*str)) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = color_sequence_from_colorname(str);

		ಆದರೆ (s) {
			*seq = strdup(s);
			ಹಿಂತಿರುಗಿ *seq ? 0 : -ENOMEM;
		}
		ಹಿಂತಿರುಗಿ 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	ಆದರೆ ((len = asprintf(seq, "\033[%sm", str)) < 1)
		ಹಿಂತಿರುಗಿ -ENOMEM;

	ಗಾಗಿ (ಒಳ = *seq, ಹೊರಹೋಗು = *seq; ಒಳ && *ಒಳ; ಒಳ++) {
		ಆದರೆ (*ಒಳ != '\\') {
			*ಹೊರಹೋಗು++ = *ಒಳ;
			ಮುಂದುವರಿ;
		}
		ಆಯ್ಕೆ(*(ಒಳ + 1)) {
		ಸ್ಥಿತಿ 'a':
			*ಹೊರಹೋಗು++ = '\a';	/* Bell */
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'b':
			*ಹೊರಹೋಗು++ = '\b';	/* Backspace */
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'e':
			*ಹೊರಹೋಗು++ = '\033';	/* Escape */
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'f':
			*ಹೊರಹೋಗು++ = '\f';	/* Form Feed */
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'n':
			*ಹೊರಹೋಗು++ = '\n';	/* Newline */
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'r':
			*ಹೊರಹೋಗು++ = '\r';	/* Carriage Return */
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 't':
			*ಹೊರಹೋಗು++ = '\t';	/* Tab */
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'v':
			*ಹೊರಹೋಗು++ = '\v';	/* Vertical Tab */
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '\\':
			*ಹೊರಹೋಗು++ = '\\';	/* Backslash */
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '_':
			*ಹೊರಹೋಗು++ = ' ';	/* Space */
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '#':
			*ಹೊರಹೋಗು++ = '#';	/* Hash mark */
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '?':
			*ಹೊರಹೋಗು++ = '?';	/* Question mark */
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			*ಹೊರಹೋಗು++ = *ಒಳ;
			*ಹೊರಹೋಗು++ = *(ಒಳ + 1);
			ನಿಲ್ಲಿಸು;
		}
		ಒಳ++;
	}

	ಆದರೆ (ಹೊರಹೋಗು) {
		ಪ್ರತಿಜ್ಞಾ ((ಹೊರಹೋಗು - *seq) <= len);
		*ಹೊರಹೋಗು = '\0';
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಅಕ್ಷರ *color_get_sequence(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *color)
{
	ಅಕ್ಷರ *seq = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ rc = __color_canonicalize(color, &seq);

	ಹಿಂತಿರುಗಿ rc ? ಖಾಲಿ : seq;
}
