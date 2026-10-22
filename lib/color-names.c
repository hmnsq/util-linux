/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2012]
 */
#ضمن "c.h"
#ضمن "color-names.h"

#ضمن <ctype.h>

بنية ul_color_name {
	ثابت حرف *name;
	ثابت حرف *seq;
};

/*
 * qsort/bsearch buddy
 */
ساكن صحيح cmp_color_name(ثابت فراغ *a0, ثابت فراغ *b0)
{
	ثابت بنية ul_color_name
		*a = (ثابت بنية ul_color_name *) a0,
		*b = (ثابت بنية ul_color_name *) b0;
	أرجع strcmp(a->name, b->name);
}

/*
 * Maintains human readable color names
 */
ثابت حرف *color_sequence_from_colorname(ثابت حرف *str)
{
	ساكن ثابت بنية ul_color_name basic_schemes[] = {
		{ "black",	UL_COLOR_BLACK           },
		{ "blink",      UL_COLOR_BLINK           },
		{ "blue",	UL_COLOR_BLUE            },
		{ "bold",       UL_COLOR_BOLD		 },
		{ "brown",	UL_COLOR_BROWN           },
		{ "cyan",	UL_COLOR_CYAN            },
		{ "darkgray",	UL_COLOR_DARK_سلیٹی       },
		{ "gray",	UL_COLOR_سلیٹی            },
		{ "green",	UL_COLOR_GREEN           },
		{ "halfbright", UL_COLOR_HALFBRIGHT	 },
		{ "lightblue",	UL_COLOR_BOLD_BLUE       },
		{ "lightcyan",	UL_COLOR_BOLD_CYAN       },
		{ "lightgray",	UL_COLOR_سلیٹی            },
		{ "lightgreen", UL_COLOR_BOLD_GREEN      },
		{ "lightmagenta", UL_COLOR_BOLD_MAGENTA  },
		{ "lightred",	UL_COLOR_BOLD_RED        },
		{ "magenta",	UL_COLOR_MAGENTA         },
		{ "red",	UL_COLOR_RED             },
		{ "reset",      UL_COLOR_RESET,          },
		{ "reverse",    UL_COLOR_الٹ         },
		{ "white",      UL_COLOR_WHITE           },
		{ "yellow",	UL_COLOR_BOLD_YELLOW     }
	};
	بنية ul_color_name کلید = { .name = str };
	ثابت بنية ul_color_name *res;

	إذا (!str)
		أرجع عدم;

	res = bsearch(&کلید, basic_schemes, ARRAY_ناپ(basic_schemes),
				حجم(بنية ul_color_name),
				cmp_color_name);
	أرجع res ? res->seq : عدم;
}


صحيح color_is_sequence(ثابت حرف *color)
{
	إذا (color && *color == 0x1B) {
		size_t len = لڑی_لمبائی(color);

		إذا (len >= 4 &&
		    *(color + 1) == '[' &&
		    ہندسہ_اے(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		أرجع 1;
	}

	أرجع 0;
}

/* canonicalize sequence */
ساكن صحيح __color_canonicalize(ثابت حرف *str, حرف **seq)
{
	حرف *وچ, *باہرلا;
	صحيح len;

	إذا (!str)
		أرجع -EINVAL;

	*seq = عدم;

	/* convert color names like "red" to the real sequence */
	إذا (*str != '\\' && !strchr(str, ';') && حرف_اے(*str)) {
		ثابت حرف *s = color_sequence_from_colorname(str);

		إذا (s) {
			*seq = strdup(s);
			أرجع *seq ? 0 : -ENOMEM;
		}
		أرجع 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	إذا ((len = asprintf(seq, "\033[%sm", str)) < 1)
		أرجع -ENOMEM;

	لكل (وچ = *seq, باہرلا = *seq; وچ && *وچ; وچ++) {
		إذا (*وچ != '\\') {
			*باہرلا++ = *وچ;
			استمر;
		}
		بدل(*(وچ + 1)) {
		حالة 'a':
			*باہرلا++ = '\a';	/* Bell */
			اكسر;
		حالة 'b':
			*باہرلا++ = '\b';	/* Backspace */
			اكسر;
		حالة 'e':
			*باہرلا++ = '\033';	/* Escape */
			اكسر;
		حالة 'f':
			*باہرلا++ = '\f';	/* Form Feed */
			اكسر;
		حالة 'n':
			*باہرلا++ = '\n';	/* Newline */
			اكسر;
		حالة 'r':
			*باہرلا++ = '\r';	/* Carriage Return */
			اكسر;
		حالة 't':
			*باہرلا++ = '\t';	/* Tab */
			اكسر;
		حالة 'v':
			*باہرلا++ = '\v';	/* Vertical Tab */
			اكسر;
		حالة '\\':
			*باہرلا++ = '\\';	/* Backslash */
			اكسر;
		حالة '_':
			*باہرلا++ = ' ';	/* Space */
			اكسر;
		حالة '#':
			*باہرلا++ = '#';	/* Hash mark */
			اكسر;
		حالة '?':
			*باہرلا++ = '?';	/* Question mark */
			اكسر;
		افتراضي:
			*باہرلا++ = *وچ;
			*باہرلا++ = *(وچ + 1);
			اكسر;
		}
		وچ++;
	}

	إذا (باہرلا) {
		أكد ((باہرلا - *seq) <= len);
		*باہرلا = '\0';
	}

	أرجع 0;
}

حرف *color_get_sequence(ثابت حرف *color)
{
	حرف *seq = عدم;
	صحيح rc = __color_canonicalize(color, &seq);

	أرجع rc ? عدم : seq;
}
