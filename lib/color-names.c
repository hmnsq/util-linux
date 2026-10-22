/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2012]
 */
#شامل "c.h"
#شامل "color-names.h"

#شامل <ctype.h>

ساختار ul_color_name {
	ثابت نویسه *name;
	ثابت نویسه *seq;
};

/*
 * qsort/bsearch buddy
 */
ایستا صحیح cmp_color_name(ثابت تهی *a0, ثابت تهی *b0)
{
	ثابت ساختار ul_color_name
		*a = (ثابت ساختار ul_color_name *) a0,
		*b = (ثابت ساختار ul_color_name *) b0;
	بازگشت strcmp(a->name, b->name);
}

/*
 * Maintains human readable color names
 */
ثابت نویسه *color_sequence_from_colorname(ثابت نویسه *str)
{
	ایستا ثابت ساختار ul_color_name basic_schemes[] = {
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
	ساختار ul_color_name کلید = { .name = str };
	ثابت ساختار ul_color_name *res;

	اگر (!str)
		بازگشت پوچ;

	res = bsearch(&کلید, basic_schemes, ARRAY_SIZE(basic_schemes),
				اندازه(ساختار ul_color_name),
				cmp_color_name);
	بازگشت res ? res->seq : پوچ;
}


صحیح color_is_sequence(ثابت نویسه *color)
{
	اگر (color && *color == 0x1B) {
		size_t len = strlen(color);

		اگر (len >= 4 &&
		    *(color + 1) == '[' &&
		    isdigit(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		بازگشت 1;
	}

	بازگشت 0;
}

/* canonicalize sequence */
ایستا صحیح __color_canonicalize(ثابت نویسه *str, نویسه **seq)
{
	نویسه *in, *out;
	صحیح len;

	اگر (!str)
		بازگشت -EINVAL;

	*seq = پوچ;

	/* convert color names like "red" to the real sequence */
	اگر (*str != '\\' && !strchr(str, ';') && isalpha(*str)) {
		ثابت نویسه *s = color_sequence_from_colorname(str);

		اگر (s) {
			*seq = strdup(s);
			بازگشت *seq ? 0 : -ENOMEM;
		}
		بازگشت 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	اگر ((len = asprintf(seq, "\033[%sm", str)) < 1)
		بازگشت -ENOMEM;

	برای (in = *seq, out = *seq; in && *in; in++) {
		اگر (*in != '\\') {
			*out++ = *in;
			ادامه;
		}
		انتخاب(*(in + 1)) {
		حالت 'a':
			*out++ = '\a';	/* Bell */
			بشکن;
		حالت 'b':
			*out++ = '\b';	/* Backspace */
			بشکن;
		حالت 'e':
			*out++ = '\033';	/* Escape */
			بشکن;
		حالت 'f':
			*out++ = '\f';	/* Form Feed */
			بشکن;
		حالت 'n':
			*out++ = '\n';	/* Newline */
			بشکن;
		حالت 'r':
			*out++ = '\r';	/* Carriage Return */
			بشکن;
		حالت 't':
			*out++ = '\t';	/* Tab */
			بشکن;
		حالت 'v':
			*out++ = '\v';	/* Vertical Tab */
			بشکن;
		حالت '\\':
			*out++ = '\\';	/* Backslash */
			بشکن;
		حالت '_':
			*out++ = ' ';	/* Space */
			بشکن;
		حالت '#':
			*out++ = '#';	/* Hash mark */
			بشکن;
		حالت '?':
			*out++ = '?';	/* Question mark */
			بشکن;
		پیشفرض:
			*out++ = *in;
			*out++ = *(in + 1);
			بشکن;
		}
		in++;
	}

	اگر (out) {
		أكد ((out - *seq) <= len);
		*out = '\0';
	}

	بازگشت 0;
}

نویسه *color_get_sequence(ثابت نویسه *color)
{
	نویسه *seq = پوچ;
	صحیح rc = __color_canonicalize(color, &seq);

	بازگشت rc ? پوچ : seq;
}
