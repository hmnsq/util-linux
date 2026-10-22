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
		{ "darkgray",	UL_COLOR_DARK_خړ       },
		{ "gray",	UL_COLOR_خړ            },
		{ "green",	UL_COLOR_GREEN           },
		{ "halfbright", UL_COLOR_HALFBRIGHT	 },
		{ "lightblue",	UL_COLOR_BOLD_BLUE       },
		{ "lightcyan",	UL_COLOR_BOLD_CYAN       },
		{ "lightgray",	UL_COLOR_خړ            },
		{ "lightgreen", UL_COLOR_BOLD_GREEN      },
		{ "lightmagenta", UL_COLOR_BOLD_MAGENTA  },
		{ "lightred",	UL_COLOR_BOLD_RED        },
		{ "magenta",	UL_COLOR_MAGENTA         },
		{ "red",	UL_COLOR_RED             },
		{ "reset",      UL_COLOR_RESET,          },
		{ "reverse",    UL_COLOR_سرچپه         },
		{ "white",      UL_COLOR_WHITE           },
		{ "yellow",	UL_COLOR_BOLD_YELLOW     }
	};
	بنية ul_color_name کلی = { .name = str };
	ثابت بنية ul_color_name *res;

	إذا (!str)
		أرجع عدم;

	res = bsearch(&کلی, basic_schemes, ARRAY_اندازه(basic_schemes),
				حجم(بنية ul_color_name),
				cmp_color_name);
	أرجع res ? res->seq : عدم;
}


صحيح color_is_sequence(ثابت حرف *color)
{
	إذا (color && *color == 0x1B) {
		size_t len = مزي_اوږدوالی(color);

		إذا (len >= 4 &&
		    *(color + 1) == '[' &&
		    که_عدد(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		أرجع 1;
	}

	أرجع 0;
}

/* canonicalize sequence */
ساكن صحيح __color_canonicalize(ثابت حرف *str, حرف **seq)
{
	حرف *دننه, *بهر;
	صحيح len;

	إذا (!str)
		أرجع -EINVAL;

	*seq = عدم;

	/* convert color names like "red" to the real sequence */
	إذا (*str != '\\' && !strchr(str, ';') && که_توری(*str)) {
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

	لكل (دننه = *seq, بهر = *seq; دننه && *دننه; دننه++) {
		إذا (*دننه != '\\') {
			*بهر++ = *دننه;
			استمر;
		}
		بدل(*(دننه + 1)) {
		حالة 'a':
			*بهر++ = '\a';	/* Bell */
			اكسر;
		حالة 'b':
			*بهر++ = '\b';	/* Backspace */
			اكسر;
		حالة 'e':
			*بهر++ = '\033';	/* Escape */
			اكسر;
		حالة 'f':
			*بهر++ = '\f';	/* Form Feed */
			اكسر;
		حالة 'n':
			*بهر++ = '\n';	/* Newline */
			اكسر;
		حالة 'r':
			*بهر++ = '\r';	/* Carriage Return */
			اكسر;
		حالة 't':
			*بهر++ = '\t';	/* Tab */
			اكسر;
		حالة 'v':
			*بهر++ = '\v';	/* Vertical Tab */
			اكسر;
		حالة '\\':
			*بهر++ = '\\';	/* Backslash */
			اكسر;
		حالة '_':
			*بهر++ = ' ';	/* Space */
			اكسر;
		حالة '#':
			*بهر++ = '#';	/* Hash mark */
			اكسر;
		حالة '?':
			*بهر++ = '?';	/* Question mark */
			اكسر;
		افتراضي:
			*بهر++ = *دننه;
			*بهر++ = *(دننه + 1);
			اكسر;
		}
		دننه++;
	}

	إذا (بهر) {
		أكد ((بهر - *seq) <= len);
		*بهر = '\0';
	}

	أرجع 0;
}

حرف *color_get_sequence(ثابت حرف *color)
{
	حرف *seq = عدم;
	صحيح rc = __color_canonicalize(color, &seq);

	أرجع rc ? عدم : seq;
}
