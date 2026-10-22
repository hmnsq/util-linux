/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2012]
 */
#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "color-names.h"

#ಅನ್ತರ್ಭಾವ <ctype.h>

ಸಂರಚನಾ ul_color_name {
	ಧ್ರುವ ವರ್ಣ *name;
	ಧ್ರುವ ವರ್ಣ *seq;
};

/*
 * qsort/bsearch buddy
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cmp_color_name(ಧ್ರುವ ಶೂನ್ಯ *a0, ಧ್ರುವ ಶೂನ್ಯ *b0)
{
	ಧ್ರುವ ಸಂರಚನಾ ul_color_name
		*a = (ಧ್ರುವ ಸಂರಚನಾ ul_color_name *) a0,
		*b = (ಧ್ರುವ ಸಂರಚನಾ ul_color_name *) b0;
	ಪ್ರತ್ಯಾಗಮ strcmp(a->name, b->name);
}

/*
 * Maintains human readable color names
 */
ಧ್ರುವ ವರ್ಣ *color_sequence_from_colorname(ಧ್ರುವ ವರ್ಣ *str)
{
	ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ ul_color_name basic_schemes[] = {
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
	ಸಂರಚನಾ ul_color_name ಕೀಲಿ = { .name = str };
	ಧ್ರುವ ಸಂರಚನಾ ul_color_name *res;

	ಯದಿ (!str)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	res = bsearch(&ಕೀಲಿ, basic_schemes, ARRAY_SIZE(basic_schemes),
				ಪರಿಮಾಣ(ಸಂರಚನಾ ul_color_name),
				cmp_color_name);
	ಪ್ರತ್ಯಾಗಮ res ? res->seq : ರಿಕ್ತ;
}


ಪೂರ್ಣಾಙ್ಕ color_is_sequence(ಧ್ರುವ ವರ್ಣ *color)
{
	ಯದಿ (color && *color == 0x1B) {
		size_t len = strlen(color);

		ಯದಿ (len >= 4 &&
		    *(color + 1) == '[' &&
		    isdigit(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

/* canonicalize sequence */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __color_canonicalize(ಧ್ರುವ ವರ್ಣ *str, ವರ್ಣ **seq)
{
	ವರ್ಣ *in, *out;
	ಪೂರ್ಣಾಙ್ಕ len;

	ಯದಿ (!str)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	*seq = ರಿಕ್ತ;

	/* convert color names like "red" to the real sequence */
	ಯದಿ (*str != '\\' && !strchr(str, ';') && isalpha(*str)) {
		ಧ್ರುವ ವರ್ಣ *s = color_sequence_from_colorname(str);

		ಯದಿ (s) {
			*seq = strdup(s);
			ಪ್ರತ್ಯಾಗಮ *seq ? 0 : -ENOMEM;
		}
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	ಯದಿ ((len = asprintf(seq, "\033[%sm", str)) < 1)
		ಪ್ರತ್ಯಾಗಮ -ENOMEM;

	ಪ್ರತಿ (in = *seq, out = *seq; in && *in; in++) {
		ಯದಿ (*in != '\\') {
			*out++ = *in;
			ಅನುವರ್ತ;
		}
		ವಿಕಲ್ಪ(*(in + 1)) {
		ಪಕ್ಷ 'a':
			*out++ = '\a';	/* Bell */
			ಭಙ್ಗ;
		ಪಕ್ಷ 'b':
			*out++ = '\b';	/* Backspace */
			ಭಙ್ಗ;
		ಪಕ್ಷ 'e':
			*out++ = '\033';	/* Escape */
			ಭಙ್ಗ;
		ಪಕ್ಷ 'f':
			*out++ = '\f';	/* Form Feed */
			ಭಙ್ಗ;
		ಪಕ್ಷ 'n':
			*out++ = '\n';	/* Newline */
			ಭಙ್ಗ;
		ಪಕ್ಷ 'r':
			*out++ = '\r';	/* Carriage Return */
			ಭಙ್ಗ;
		ಪಕ್ಷ 't':
			*out++ = '\t';	/* Tab */
			ಭಙ್ಗ;
		ಪಕ್ಷ 'v':
			*out++ = '\v';	/* Vertical Tab */
			ಭಙ್ಗ;
		ಪಕ್ಷ '\\':
			*out++ = '\\';	/* Backslash */
			ಭಙ್ಗ;
		ಪಕ್ಷ '_':
			*out++ = ' ';	/* Space */
			ಭಙ್ಗ;
		ಪಕ್ಷ '#':
			*out++ = '#';	/* Hash mark */
			ಭಙ್ಗ;
		ಪಕ್ಷ '?':
			*out++ = '?';	/* Question mark */
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			*out++ = *in;
			*out++ = *(in + 1);
			ಭಙ್ಗ;
		}
		in++;
	}

	ಯದಿ (out) {
		ಪ್ರತಿಜ್ಞಾ ((out - *seq) <= len);
		*out = '\0';
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ವರ್ಣ *color_get_sequence(ಧ್ರುವ ವರ್ಣ *color)
{
	ವರ್ಣ *seq = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ rc = __color_canonicalize(color, &seq);

	ಪ್ರತ್ಯಾಗಮ rc ? ರಿಕ್ತ : seq;
}
