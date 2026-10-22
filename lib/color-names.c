/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2012]
 */
#समावेश "c.h"
#समावेश "color-names.h"

#समावेश <ctype.h>

काष्ठा ul_color_name {
	स्थिर अक्षर *name;
	स्थिर अक्षर *seq;
};

/*
 * qsort/bsearch buddy
 */
अटल पूर्णांक cmp_color_name(स्थिर व्योम *a0, स्थिर व्योम *b0)
{
	स्थिर काष्ठा ul_color_name
		*a = (स्थिर काष्ठा ul_color_name *) a0,
		*b = (स्थिर काष्ठा ul_color_name *) b0;
	वापस म_भेद(a->name, b->name);
}

/*
 * Maintains human readable color names
 */
स्थिर अक्षर *color_sequence_from_colorname(स्थिर अक्षर *str)
{
	अटल स्थिर काष्ठा ul_color_name basic_schemes[] = {
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
	काष्ठा ul_color_name कुंजी = { .name = str };
	स्थिर काष्ठा ul_color_name *res;

	अगर (!str)
		वापस शून्य;

	res = द्वा_खोज(&कुंजी, basic_schemes, ARRAY_आकार(basic_schemes),
				माप(काष्ठा ul_color_name),
				cmp_color_name);
	वापस res ? res->seq : शून्य;
}


पूर्णांक color_is_sequence(स्थिर अक्षर *color)
{
	अगर (color && *color == 0x1B) {
		माप_प्रकार len = म_माप(color);

		अगर (len >= 4 &&
		    *(color + 1) == '[' &&
		    है_अंक(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		वापस 1;
	}

	वापस 0;
}

/* canonicalize sequence */
अटल पूर्णांक __color_canonicalize(स्थिर अक्षर *str, अक्षर **seq)
{
	अक्षर *in, *out;
	पूर्णांक len;

	अगर (!str)
		वापस -EINVAL;

	*seq = शून्य;

	/* convert color names like "red" to the real sequence */
	अगर (*str != '\\' && !म_अक्षर(str, ';') && है_अक्षर(*str)) {
		स्थिर अक्षर *s = color_sequence_from_colorname(str);

		अगर (s) {
			*seq = strdup(s);
			वापस *seq ? 0 : -ENOMEM;
		}
		वापस 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	अगर ((len = asprintf(seq, "\033[%sm", str)) < 1)
		वापस -ENOMEM;

	क्रम (in = *seq, out = *seq; in && *in; in++) {
		अगर (*in != '\\') {
			*out++ = *in;
			जारी;
		}
		चयन(*(in + 1)) {
		हाल 'a':
			*out++ = '\a';	/* Bell */
			अवरोध;
		हाल 'b':
			*out++ = '\b';	/* Backspace */
			अवरोध;
		हाल 'e':
			*out++ = '\033';	/* Escape */
			अवरोध;
		हाल 'f':
			*out++ = '\f';	/* Form Feed */
			अवरोध;
		हाल 'n':
			*out++ = '\n';	/* Newline */
			अवरोध;
		हाल 'r':
			*out++ = '\r';	/* Carriage Return */
			अवरोध;
		हाल 't':
			*out++ = '\t';	/* Tab */
			अवरोध;
		हाल 'v':
			*out++ = '\v';	/* Vertical Tab */
			अवरोध;
		हाल '\\':
			*out++ = '\\';	/* Backslash */
			अवरोध;
		हाल '_':
			*out++ = ' ';	/* Space */
			अवरोध;
		हाल '#':
			*out++ = '#';	/* Hash mark */
			अवरोध;
		हाल '?':
			*out++ = '?';	/* Question mark */
			अवरोध;
		शेष:
			*out++ = *in;
			*out++ = *(in + 1);
			अवरोध;
		}
		in++;
	}

	अगर (out) {
		निश्चित ((out - *seq) <= len);
		*out = '\0';
	}

	वापस 0;
}

अक्षर *color_get_sequence(स्थिर अक्षर *color)
{
	अक्षर *seq = शून्य;
	पूर्णांक rc = __color_canonicalize(color, &seq);

	वापस rc ? शून्य : seq;
}
