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
	स्थिरांक अक्षर *name;
	स्थिरांक अक्षर *seq;
};

/*
 * qsort/bsearch buddy
 */
स्थिर पूर्णांक cmp_color_name(स्थिरांक शून्य *a0, स्थिरांक शून्य *b0)
{
	स्थिरांक संरचना ul_color_name
		*a = (स्थिरांक संरचना ul_color_name *) a0,
		*b = (स्थिरांक संरचना ul_color_name *) b0;
	परत म_भेद(a->name, b->name);
}

/*
 * Maintains human readable color names
 */
स्थिरांक अक्षर *color_sequence_from_colorname(स्थिरांक अक्षर *str)
{
	स्थिर स्थिरांक संरचना ul_color_name basic_schemes[] = {
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
	संरचना ul_color_name किल्ली = { .name = str };
	स्थिरांक संरचना ul_color_name *res;

	जर (!str)
		परत रिक्त;

	res = द्वा_खोज(&किल्ली, basic_schemes, ARRAY_SIZE(basic_schemes),
				आकार(संरचना ul_color_name),
				cmp_color_name);
	परत res ? res->seq : रिक्त;
}


पूर्णांक color_is_sequence(स्थिरांक अक्षर *color)
{
	जर (color && *color == 0x1B) {
		माप_प्रकार len = म_माप(color);

		जर (len >= 4 &&
		    *(color + 1) == '[' &&
		    है_अंक(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		परत 1;
	}

	परत 0;
}

/* canonicalize sequence */
स्थिर पूर्णांक __color_canonicalize(स्थिरांक अक्षर *str, अक्षर **seq)
{
	अक्षर *in, *out;
	पूर्णांक len;

	जर (!str)
		परत -EINVAL;

	*seq = रिक्त;

	/* convert color names like "red" to the real sequence */
	जर (*str != '\\' && !म_अक्षर(str, ';') && है_अक्षर(*str)) {
		स्थिरांक अक्षर *s = color_sequence_from_colorname(str);

		जर (s) {
			*seq = strdup(s);
			परत *seq ? 0 : -ENOMEM;
		}
		परत 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	जर ((len = asprintf(seq, "\033[%sm", str)) < 1)
		परत -ENOMEM;

	साठी (in = *seq, out = *seq; in && *in; in++) {
		जर (*in != '\\') {
			*out++ = *in;
			सुरूठेवा;
		}
		निवड(*(in + 1)) {
		स्थिती 'a':
			*out++ = '\a';	/* Bell */
			थांबा;
		स्थिती 'b':
			*out++ = '\b';	/* Backspace */
			थांबा;
		स्थिती 'e':
			*out++ = '\033';	/* Escape */
			थांबा;
		स्थिती 'f':
			*out++ = '\f';	/* Form Feed */
			थांबा;
		स्थिती 'n':
			*out++ = '\n';	/* Newline */
			थांबा;
		स्थिती 'r':
			*out++ = '\r';	/* Carriage Return */
			थांबा;
		स्थिती 't':
			*out++ = '\t';	/* Tab */
			थांबा;
		स्थिती 'v':
			*out++ = '\v';	/* Vertical Tab */
			थांबा;
		स्थिती '\\':
			*out++ = '\\';	/* Backslash */
			थांबा;
		स्थिती '_':
			*out++ = ' ';	/* Space */
			थांबा;
		स्थिती '#':
			*out++ = '#';	/* Hash mark */
			थांबा;
		स्थिती '?':
			*out++ = '?';	/* Question mark */
			थांबा;
		पूर्वनिर्धारित:
			*out++ = *in;
			*out++ = *(in + 1);
			थांबा;
		}
		in++;
	}

	जर (out) {
		प्रतिज्ञा ((out - *seq) <= len);
		*out = '\0';
	}

	परत 0;
}

अक्षर *color_get_sequence(स्थिरांक अक्षर *color)
{
	अक्षर *seq = रिक्त;
	पूर्णांक rc = __color_canonicalize(color, &seq);

	परत rc ? रिक्त : seq;
}
