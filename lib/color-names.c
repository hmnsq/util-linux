/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2012]
 */
#شامل "c.h"
#شامل "color-names.h"

#شامل <ctype.h>

ساخت ul_color_name {
	ثابت حرف *name;
	ثابت حرف *seq;
};

/*
 * qsort/bsearch buddy
 */
ساکن صحیح cmp_color_name(ثابت خالی *a0, ثابت خالی *b0)
{
	ثابت ساخت ul_color_name
		*a = (ثابت ساخت ul_color_name *) a0,
		*b = (ثابت ساخت ul_color_name *) b0;
	واپس strcmp(a->name, b->name);
}

/*
 * Maintains human readable color names
 */
ثابت حرف *color_sequence_from_colorname(ثابت حرف *str)
{
	ساکن ثابت ساخت ul_color_name basic_schemes[] = {
		{ "black",	UL_COLOR_BLACK           },
		{ "blink",      UL_COLOR_BLINK           },
		{ "blue",	UL_COLOR_BLUE            },
		{ "bold",       UL_COLOR_BOLD		 },
		{ "brown",	UL_COLOR_BROWN           },
		{ "cyan",	UL_COLOR_CYAN            },
		{ "darkgray",	UL_COLOR_DARK_سرمئی       },
		{ "gray",	UL_COLOR_سرمئی            },
		{ "green",	UL_COLOR_GREEN           },
		{ "halfbright", UL_COLOR_HALFBRIGHT	 },
		{ "lightblue",	UL_COLOR_BOLD_BLUE       },
		{ "lightcyan",	UL_COLOR_BOLD_CYAN       },
		{ "lightgray",	UL_COLOR_سرمئی            },
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
	ساخت ul_color_name کلید = { .name = str };
	ثابت ساخت ul_color_name *res;

	اگر (!str)
		واپس تہی;

	res = bsearch(&کلید, basic_schemes, ARRAY_سائز(basic_schemes),
				حجم(ساخت ul_color_name),
				cmp_color_name);
	واپس res ? res->seq : تہی;
}


صحیح color_is_sequence(ثابت حرف *color)
{
	اگر (color && *color == 0x1B) {
		size_t len = لڑی_لمبائی(color);

		اگر (len >= 4 &&
		    *(color + 1) == '[' &&
		    ہندسہ_ہے(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		واپس 1;
	}

	واپس 0;
}

/* canonicalize sequence */
ساکن صحیح __color_canonicalize(ثابت حرف *str, حرف **seq)
{
	حرف *اندر, *برآمد;
	صحیح len;

	اگر (!str)
		واپس -EINVAL;

	*seq = تہی;

	/* convert color names like "red" to the real sequence */
	اگر (*str != '\\' && !strchr(str, ';') && حرف_ہے(*str)) {
		ثابت حرف *s = color_sequence_from_colorname(str);

		اگر (s) {
			*seq = strdup(s);
			واپس *seq ? 0 : -ENOMEM;
		}
		واپس 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	اگر ((len = asprintf(seq, "\033[%sm", str)) < 1)
		واپس -ENOMEM;

	برائے (اندر = *seq, برآمد = *seq; اندر && *اندر; اندر++) {
		اگر (*اندر != '\\') {
			*برآمد++ = *اندر;
			جاری;
		}
		انتخاب(*(اندر + 1)) {
		حالت 'a':
			*برآمد++ = '\a';	/* Bell */
			توڑو;
		حالت 'b':
			*برآمد++ = '\b';	/* Backspace */
			توڑو;
		حالت 'e':
			*برآمد++ = '\033';	/* Escape */
			توڑو;
		حالت 'f':
			*برآمد++ = '\f';	/* Form Feed */
			توڑو;
		حالت 'n':
			*برآمد++ = '\n';	/* Newline */
			توڑو;
		حالت 'r':
			*برآمد++ = '\r';	/* Carriage Return */
			توڑو;
		حالت 't':
			*برآمد++ = '\t';	/* Tab */
			توڑو;
		حالت 'v':
			*برآمد++ = '\v';	/* Vertical Tab */
			توڑو;
		حالت '\\':
			*برآمد++ = '\\';	/* Backslash */
			توڑو;
		حالت '_':
			*برآمد++ = ' ';	/* Space */
			توڑو;
		حالت '#':
			*برآمد++ = '#';	/* Hash mark */
			توڑو;
		حالت '?':
			*برآمد++ = '?';	/* Question mark */
			توڑو;
		طےشدہ:
			*برآمد++ = *اندر;
			*برآمد++ = *(اندر + 1);
			توڑو;
		}
		اندر++;
	}

	اگر (برآمد) {
		أكد ((برآمد - *seq) <= len);
		*برآمد = '\0';
	}

	واپس 0;
}

حرف *color_get_sequence(ثابت حرف *color)
{
	حرف *seq = تہی;
	صحیح rc = __color_canonicalize(color, &seq);

	واپس rc ? تہی : seq;
}
