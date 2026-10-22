/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2012]
 */
#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "color-names.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>

ਬਣਤਰ ul_color_name {
	ਅਚਲ ਅੱਖਰ *name;
	ਅਚਲ ਅੱਖਰ *seq;
};

/*
 * qsort/bsearch buddy
 */
ਸਥਿਰ ਪੂਰਨ cmp_color_name(ਅਚਲ ਖਾਲੀ *a0, ਅਚਲ ਖਾਲੀ *b0)
{
	ਅਚਲ ਬਣਤਰ ul_color_name
		*a = (ਅਚਲ ਬਣਤਰ ul_color_name *) a0,
		*b = (ਅਚਲ ਬਣਤਰ ul_color_name *) b0;
	ਵਾਪਸ strcmp(a->name, b->name);
}

/*
 * Maintains human readable color names
 */
ਅਚਲ ਅੱਖਰ *color_sequence_from_colorname(ਅਚਲ ਅੱਖਰ *str)
{
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ ul_color_name basic_schemes[] = {
		{ "black",	UL_COLOR_BLACK           },
		{ "blink",      UL_COLOR_BLINK           },
		{ "blue",	UL_COLOR_BLUE            },
		{ "bold",       UL_COLOR_BOLD		 },
		{ "brown",	UL_COLOR_BROWN           },
		{ "cyan",	UL_COLOR_CYAN            },
		{ "darkgray",	UL_COLOR_DARK_ਸਲੇਟੀ       },
		{ "gray",	UL_COLOR_ਸਲੇਟੀ            },
		{ "green",	UL_COLOR_GREEN           },
		{ "halfbright", UL_COLOR_HALFBRIGHT	 },
		{ "lightblue",	UL_COLOR_BOLD_BLUE       },
		{ "lightcyan",	UL_COLOR_BOLD_CYAN       },
		{ "lightgray",	UL_COLOR_ਸਲੇਟੀ            },
		{ "lightgreen", UL_COLOR_BOLD_GREEN      },
		{ "lightmagenta", UL_COLOR_BOLD_MAGENTA  },
		{ "lightred",	UL_COLOR_BOLD_RED        },
		{ "magenta",	UL_COLOR_MAGENTA         },
		{ "red",	UL_COLOR_RED             },
		{ "reset",      UL_COLOR_RESET,          },
		{ "reverse",    UL_COLOR_ਉਲਟ         },
		{ "white",      UL_COLOR_WHITE           },
		{ "yellow",	UL_COLOR_BOLD_YELLOW     }
	};
	ਬਣਤਰ ul_color_name ਕੁੰਜੀ = { .name = str };
	ਅਚਲ ਬਣਤਰ ul_color_name *res;

	ਜੇ (!str)
		ਵਾਪਸ ਨਿਰਾ;

	res = bsearch(&ਕੁੰਜੀ, basic_schemes, ARRAY_ਮਾਪ(basic_schemes),
				ਆਕਾਰ(ਬਣਤਰ ul_color_name),
				cmp_color_name);
	ਵਾਪਸ res ? res->seq : ਨਿਰਾ;
}


ਪੂਰਨ color_is_sequence(ਅਚਲ ਅੱਖਰ *color)
{
	ਜੇ (color && *color == 0x1B) {
		size_t len = ਲੜੀ_ਲੰਬਾਈ(color);

		ਜੇ (len >= 4 &&
		    *(color + 1) == '[' &&
		    ਅੰਕ_ਹੈ(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		ਵਾਪਸ 1;
	}

	ਵਾਪਸ 0;
}

/* canonicalize sequence */
ਸਥਿਰ ਪੂਰਨ __color_canonicalize(ਅਚਲ ਅੱਖਰ *str, ਅੱਖਰ **seq)
{
	ਅੱਖਰ *ਵਿੱਚ, *ਬਾਹਰਲਾ;
	ਪੂਰਨ len;

	ਜੇ (!str)
		ਵਾਪਸ -EINVAL;

	*seq = ਨਿਰਾ;

	/* convert color names like "red" to the real sequence */
	ਜੇ (*str != '\\' && !strchr(str, ';') && ਅੱਖਰ_ਹੈ(*str)) {
		ਅਚਲ ਅੱਖਰ *s = color_sequence_from_colorname(str);

		ਜੇ (s) {
			*seq = strdup(s);
			ਵਾਪਸ *seq ? 0 : -ENOMEM;
		}
		ਵਾਪਸ 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	ਜੇ ((len = asprintf(seq, "\033[%sm", str)) < 1)
		ਵਾਪਸ -ENOMEM;

	ਲਈ (ਵਿੱਚ = *seq, ਬਾਹਰਲਾ = *seq; ਵਿੱਚ && *ਵਿੱਚ; ਵਿੱਚ++) {
		ਜੇ (*ਵਿੱਚ != '\\') {
			*ਬਾਹਰਲਾ++ = *ਵਿੱਚ;
			ਜਾਰੀ;
		}
		ਚੋਣ(*(ਵਿੱਚ + 1)) {
		ਹਾਲਤ 'a':
			*ਬਾਹਰਲਾ++ = '\a';	/* Bell */
			ਰੋਕੋ;
		ਹਾਲਤ 'b':
			*ਬਾਹਰਲਾ++ = '\b';	/* Backspace */
			ਰੋਕੋ;
		ਹਾਲਤ 'e':
			*ਬਾਹਰਲਾ++ = '\033';	/* Escape */
			ਰੋਕੋ;
		ਹਾਲਤ 'f':
			*ਬਾਹਰਲਾ++ = '\f';	/* Form Feed */
			ਰੋਕੋ;
		ਹਾਲਤ 'n':
			*ਬਾਹਰਲਾ++ = '\n';	/* Newline */
			ਰੋਕੋ;
		ਹਾਲਤ 'r':
			*ਬਾਹਰਲਾ++ = '\r';	/* Carriage Return */
			ਰੋਕੋ;
		ਹਾਲਤ 't':
			*ਬਾਹਰਲਾ++ = '\t';	/* Tab */
			ਰੋਕੋ;
		ਹਾਲਤ 'v':
			*ਬਾਹਰਲਾ++ = '\v';	/* Vertical Tab */
			ਰੋਕੋ;
		ਹਾਲਤ '\\':
			*ਬਾਹਰਲਾ++ = '\\';	/* Backslash */
			ਰੋਕੋ;
		ਹਾਲਤ '_':
			*ਬਾਹਰਲਾ++ = ' ';	/* Space */
			ਰੋਕੋ;
		ਹਾਲਤ '#':
			*ਬਾਹਰਲਾ++ = '#';	/* Hash mark */
			ਰੋਕੋ;
		ਹਾਲਤ '?':
			*ਬਾਹਰਲਾ++ = '?';	/* Question mark */
			ਰੋਕੋ;
		ਮੂਲ:
			*ਬਾਹਰਲਾ++ = *ਵਿੱਚ;
			*ਬਾਹਰਲਾ++ = *(ਵਿੱਚ + 1);
			ਰੋਕੋ;
		}
		ਵਿੱਚ++;
	}

	ਜੇ (ਬਾਹਰਲਾ) {
		ਪ੍ਰਤਿਜ੍ਞਾ ((ਬਾਹਰਲਾ - *seq) <= len);
		*ਬਾਹਰਲਾ = '\0';
	}

	ਵਾਪਸ 0;
}

ਅੱਖਰ *color_get_sequence(ਅਚਲ ਅੱਖਰ *color)
{
	ਅੱਖਰ *seq = ਨਿਰਾ;
	ਪੂਰਨ rc = __color_canonicalize(color, &seq);

	ਵਾਪਸ rc ? ਨਿਰਾ : seq;
}
