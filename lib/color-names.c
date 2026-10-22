/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2012]
 */
#অন্তর্ভাব "c.h"
#অন্তর্ভাব "color-names.h"

#অন্তর্ভাব <ctype.h>

কাঠামো ul_color_name {
	ধ্রুবক অক্ষর *name;
	ধ্রুবক অক্ষর *seq;
};

/*
 * qsort/bsearch buddy
 */
স্থির পূর্ণসংখ্যা cmp_color_name(ধ্রুবক শূন্য *a0, ধ্রুবক শূন্য *b0)
{
	ধ্রুবক কাঠামো ul_color_name
		*a = (ধ্রুবক কাঠামো ul_color_name *) a0,
		*b = (ধ্রুবক কাঠামো ul_color_name *) b0;
	ফেরত strcmp(a->name, b->name);
}

/*
 * Maintains human readable color names
 */
ধ্রুবক অক্ষর *color_sequence_from_colorname(ধ্রুবক অক্ষর *str)
{
	স্থির ধ্রুবক কাঠামো ul_color_name basic_schemes[] = {
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
	কাঠামো ul_color_name চাবি = { .name = str };
	ধ্রুবক কাঠামো ul_color_name *res;

	যদি (!str)
		ফেরত ফাঁকা;

	res = bsearch(&চাবি, basic_schemes, ARRAY_আকার(basic_schemes),
				আকার(কাঠামো ul_color_name),
				cmp_color_name);
	ফেরত res ? res->seq : ফাঁকা;
}


পূর্ণসংখ্যা color_is_sequence(ধ্রুবক অক্ষর *color)
{
	যদি (color && *color == 0x1B) {
		size_t len = strlen(color);

		যদি (len >= 4 &&
		    *(color + 1) == '[' &&
		    isdigit(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		ফেরত 1;
	}

	ফেরত 0;
}

/* canonicalize sequence */
স্থির পূর্ণসংখ্যা __color_canonicalize(ধ্রুবক অক্ষর *str, অক্ষর **seq)
{
	অক্ষর *in, *out;
	পূর্ণসংখ্যা len;

	যদি (!str)
		ফেরত -EINVAL;

	*seq = ফাঁকা;

	/* convert color names like "red" to the real sequence */
	যদি (*str != '\\' && !strchr(str, ';') && isalpha(*str)) {
		ধ্রুবক অক্ষর *s = color_sequence_from_colorname(str);

		যদি (s) {
			*seq = strdup(s);
			ফেরত *seq ? 0 : -ENOMEM;
		}
		ফেরত 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	যদি ((len = asprintf(seq, "\033[%sm", str)) < 1)
		ফেরত -ENOMEM;

	জন্য (in = *seq, out = *seq; in && *in; in++) {
		যদি (*in != '\\') {
			*out++ = *in;
			চালু;
		}
		নির্বাচন(*(in + 1)) {
		ক্ষেত্র 'a':
			*out++ = '\a';	/* Bell */
			বিরতি;
		ক্ষেত্র 'b':
			*out++ = '\b';	/* Backspace */
			বিরতি;
		ক্ষেত্র 'e':
			*out++ = '\033';	/* Escape */
			বিরতি;
		ক্ষেত্র 'f':
			*out++ = '\f';	/* Form Feed */
			বিরতি;
		ক্ষেত্র 'n':
			*out++ = '\n';	/* Newline */
			বিরতি;
		ক্ষেত্র 'r':
			*out++ = '\r';	/* Carriage Return */
			বিরতি;
		ক্ষেত্র 't':
			*out++ = '\t';	/* Tab */
			বিরতি;
		ক্ষেত্র 'v':
			*out++ = '\v';	/* Vertical Tab */
			বিরতি;
		ক্ষেত্র '\\':
			*out++ = '\\';	/* Backslash */
			বিরতি;
		ক্ষেত্র '_':
			*out++ = ' ';	/* Space */
			বিরতি;
		ক্ষেত্র '#':
			*out++ = '#';	/* Hash mark */
			বিরতি;
		ক্ষেত্র '?':
			*out++ = '?';	/* Question mark */
			বিরতি;
		পূর্বনির্ধারিত:
			*out++ = *in;
			*out++ = *(in + 1);
			বিরতি;
		}
		in++;
	}

	যদি (out) {
		প্রতিজ্ঞা ((out - *seq) <= len);
		*out = '\0';
	}

	ফেরত 0;
}

অক্ষর *color_get_sequence(ধ্রুবক অক্ষর *color)
{
	অক্ষর *seq = ফাঁকা;
	পূর্ণসংখ্যা rc = __color_canonicalize(color, &seq);

	ফেরত rc ? ফাঁকা : seq;
}
