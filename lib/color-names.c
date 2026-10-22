/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2012]
 */
#অন্তর্ভাব "c.h"
#অন্তর্ভাব "color-names.h"

#অন্তর্ভাব <ctype.h>

সংরচনা ul_color_name {
	ধ্রুব বর্ণ *name;
	ধ্রুব বর্ণ *seq;
};

/*
 * qsort/bsearch buddy
 */
স্থির পূর্ণাঙ্ক cmp_color_name(ধ্রুব শূন্য *a0, ধ্রুব শূন্য *b0)
{
	ধ্রুব সংরচনা ul_color_name
		*a = (ধ্রুব সংরচনা ul_color_name *) a0,
		*b = (ধ্রুব সংরচনা ul_color_name *) b0;
	প্রত্যাগম strcmp(a->name, b->name);
}

/*
 * Maintains human readable color names
 */
ধ্রুব বর্ণ *color_sequence_from_colorname(ধ্রুব বর্ণ *str)
{
	স্থির ধ্রুব সংরচনা ul_color_name basic_schemes[] = {
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
	সংরচনা ul_color_name চাবি = { .name = str };
	ধ্রুব সংরচনা ul_color_name *res;

	যদি (!str)
		প্রত্যাগম রিক্ত;

	res = bsearch(&চাবি, basic_schemes, ARRAY_SIZE(basic_schemes),
				পরিমাণ(সংরচনা ul_color_name),
				cmp_color_name);
	প্রত্যাগম res ? res->seq : রিক্ত;
}


পূর্ণাঙ্ক color_is_sequence(ধ্রুব বর্ণ *color)
{
	যদি (color && *color == 0x1B) {
		size_t len = strlen(color);

		যদি (len >= 4 &&
		    *(color + 1) == '[' &&
		    isdigit(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		প্রত্যাগম 1;
	}

	প্রত্যাগম 0;
}

/* canonicalize sequence */
স্থির পূর্ণাঙ্ক __color_canonicalize(ধ্রুব বর্ণ *str, বর্ণ **seq)
{
	বর্ণ *in, *out;
	পূর্ণাঙ্ক len;

	যদি (!str)
		প্রত্যাগম -EINVAL;

	*seq = রিক্ত;

	/* convert color names like "red" to the real sequence */
	যদি (*str != '\\' && !strchr(str, ';') && isalpha(*str)) {
		ধ্রুব বর্ণ *s = color_sequence_from_colorname(str);

		যদি (s) {
			*seq = strdup(s);
			প্রত্যাগম *seq ? 0 : -ENOMEM;
		}
		প্রত্যাগম 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	যদি ((len = asprintf(seq, "\033[%sm", str)) < 1)
		প্রত্যাগম -ENOMEM;

	প্রতি (in = *seq, out = *seq; in && *in; in++) {
		যদি (*in != '\\') {
			*out++ = *in;
			অনুবর্ত;
		}
		বিকল্প(*(in + 1)) {
		পক্ষ 'a':
			*out++ = '\a';	/* Bell */
			ভঙ্গ;
		পক্ষ 'b':
			*out++ = '\b';	/* Backspace */
			ভঙ্গ;
		পক্ষ 'e':
			*out++ = '\033';	/* Escape */
			ভঙ্গ;
		পক্ষ 'f':
			*out++ = '\f';	/* Form Feed */
			ভঙ্গ;
		পক্ষ 'n':
			*out++ = '\n';	/* Newline */
			ভঙ্গ;
		পক্ষ 'r':
			*out++ = '\r';	/* Carriage Return */
			ভঙ্গ;
		পক্ষ 't':
			*out++ = '\t';	/* Tab */
			ভঙ্গ;
		পক্ষ 'v':
			*out++ = '\v';	/* Vertical Tab */
			ভঙ্গ;
		পক্ষ '\\':
			*out++ = '\\';	/* Backslash */
			ভঙ্গ;
		পক্ষ '_':
			*out++ = ' ';	/* Space */
			ভঙ্গ;
		পক্ষ '#':
			*out++ = '#';	/* Hash mark */
			ভঙ্গ;
		পক্ষ '?':
			*out++ = '?';	/* Question mark */
			ভঙ্গ;
		স্বভাব:
			*out++ = *in;
			*out++ = *(in + 1);
			ভঙ্গ;
		}
		in++;
	}

	যদি (out) {
		প্রতিজ্ঞা ((out - *seq) <= len);
		*out = '\0';
	}

	প্রত্যাগম 0;
}

বর্ণ *color_get_sequence(ধ্রুব বর্ণ *color)
{
	বর্ণ *seq = রিক্ত;
	পূর্ণাঙ্ক rc = __color_canonicalize(color, &seq);

	প্রত্যাগম rc ? রিক্ত : seq;
}
