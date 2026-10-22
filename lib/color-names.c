/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2012]
 */
#अन्तर्भाव "c.h"
#अन्तर्भाव "color-names.h"

#अन्तर्भाव <ctype.h>

संरचना ul_color_name {
	अचर वर्ण *name;
	अचर वर्ण *seq;
};

/*
 * qsort/bsearch buddy
 */
स्थिर पूर्णाङ्क cmp_color_name(अचर शून्य *a0, अचर शून्य *b0)
{
	अचर संरचना ul_color_name
		*a = (अचर संरचना ul_color_name *) a0,
		*b = (अचर संरचना ul_color_name *) b0;
	फर्काऊ म_भेद(a->name, b->name);
}

/*
 * Maintains human readable color names
 */
अचर वर्ण *color_sequence_from_colorname(अचर वर्ण *str)
{
	स्थिर अचर संरचना ul_color_name basic_schemes[] = {
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
	संरचना ul_color_name साँचो = { .name = str };
	अचर संरचना ul_color_name *res;

	यदि (!str)
		फर्काऊ रिक्त;

	res = द्वा_खोज(&साँचो, basic_schemes, ARRAY_SIZE(basic_schemes),
				परिमाण(संरचना ul_color_name),
				cmp_color_name);
	फर्काऊ res ? res->seq : रिक्त;
}


पूर्णाङ्क color_is_sequence(अचर वर्ण *color)
{
	यदि (color && *color == 0x1B) {
		माप_प्रकार len = म_माप(color);

		यदि (len >= 4 &&
		    *(color + 1) == '[' &&
		    है_अंक(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		फर्काऊ 1;
	}

	फर्काऊ 0;
}

/* canonicalize sequence */
स्थिर पूर्णाङ्क __color_canonicalize(अचर वर्ण *str, वर्ण **seq)
{
	वर्ण *in, *out;
	पूर्णाङ्क len;

	यदि (!str)
		फर्काऊ -EINVAL;

	*seq = रिक्त;

	/* convert color names like "red" to the real sequence */
	यदि (*str != '\\' && !म_अक्षर(str, ';') && है_अक्षर(*str)) {
		अचर वर्ण *s = color_sequence_from_colorname(str);

		यदि (s) {
			*seq = strdup(s);
			फर्काऊ *seq ? 0 : -ENOMEM;
		}
		फर्काऊ 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	यदि ((len = asprintf(seq, "\033[%sm", str)) < 1)
		फर्काऊ -ENOMEM;

	लागि (in = *seq, out = *seq; in && *in; in++) {
		यदि (*in != '\\') {
			*out++ = *in;
			जारी;
		}
		विकल्प(*(in + 1)) {
		पक्ष 'a':
			*out++ = '\a';	/* Bell */
			रोक;
		पक्ष 'b':
			*out++ = '\b';	/* Backspace */
			रोक;
		पक्ष 'e':
			*out++ = '\033';	/* Escape */
			रोक;
		पक्ष 'f':
			*out++ = '\f';	/* Form Feed */
			रोक;
		पक्ष 'n':
			*out++ = '\n';	/* Newline */
			रोक;
		पक्ष 'r':
			*out++ = '\r';	/* Carriage Return */
			रोक;
		पक्ष 't':
			*out++ = '\t';	/* Tab */
			रोक;
		पक्ष 'v':
			*out++ = '\v';	/* Vertical Tab */
			रोक;
		पक्ष '\\':
			*out++ = '\\';	/* Backslash */
			रोक;
		पक्ष '_':
			*out++ = ' ';	/* Space */
			रोक;
		पक्ष '#':
			*out++ = '#';	/* Hash mark */
			रोक;
		पक्ष '?':
			*out++ = '?';	/* Question mark */
			रोक;
		स्वभाव:
			*out++ = *in;
			*out++ = *(in + 1);
			रोक;
		}
		in++;
	}

	यदि (out) {
		प्रतिज्ञा ((out - *seq) <= len);
		*out = '\0';
	}

	फर्काऊ 0;
}

वर्ण *color_get_sequence(अचर वर्ण *color)
{
	वर्ण *seq = रिक्त;
	पूर्णाङ्क rc = __color_canonicalize(color, &seq);

	फर्काऊ rc ? रिक्त : seq;
}
