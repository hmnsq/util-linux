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
	ध्रुव वर्ण *name;
	ध्रुव वर्ण *seq;
};

/*
 * qsort/bsearch buddy
 */
स्थिर पूर्णाङ्क cmp_color_name(ध्रुव शून्य *a0, ध्रुव शून्य *b0)
{
	ध्रुव संरचना ul_color_name
		*a = (ध्रुव संरचना ul_color_name *) a0,
		*b = (ध्रुव संरचना ul_color_name *) b0;
	प्रत्यागम वाक्यं_तुलय(a->name, b->name);
}

/*
 * Maintains human readable color names
 */
ध्रुव वर्ण *color_sequence_from_colorname(ध्रुव वर्ण *str)
{
	स्थिर ध्रुव संरचना ul_color_name basic_schemes[] = {
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
	संरचना ul_color_name कुञ्चिका = { .name = str };
	ध्रुव संरचना ul_color_name *res;

	यदि (!str)
		प्रत्यागम रिक्त;

	res = bsearch(&कुञ्चिका, basic_schemes, ARRAY_आकार(basic_schemes),
				परिमाण(संरचना ul_color_name),
				cmp_color_name);
	प्रत्यागम res ? res->seq : रिक्त;
}


पूर्णाङ्क color_is_sequence(ध्रुव वर्ण *color)
{
	यदि (color && *color == 0x1B) {
		size_t len = वाक्यदैर्घ्यम्(color);

		यदि (len >= 4 &&
		    *(color + 1) == '[' &&
		    अङ्कः_वा(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		प्रत्यागम 1;
	}

	प्रत्यागम 0;
}

/* canonicalize sequence */
स्थिर पूर्णाङ्क __color_canonicalize(ध्रुव वर्ण *str, वर्ण **seq)
{
	वर्ण *अन्तर्, *बहिर्गमनम्;
	पूर्णाङ्क len;

	यदि (!str)
		प्रत्यागम -EINVAL;

	*seq = रिक्त;

	/* convert color names like "red" to the real sequence */
	यदि (*str != '\\' && !strchr(str, ';') && अक्षरम्_वा(*str)) {
		ध्रुव वर्ण *s = color_sequence_from_colorname(str);

		यदि (s) {
			*seq = strdup(s);
			प्रत्यागम *seq ? 0 : -ENOMEM;
		}
		प्रत्यागम 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	यदि ((len = asprintf(seq, "\033[%sm", str)) < 1)
		प्रत्यागम -ENOMEM;

	प्रति (अन्तर् = *seq, बहिर्गमनम् = *seq; अन्तर् && *अन्तर्; अन्तर्++) {
		यदि (*अन्तर् != '\\') {
			*बहिर्गमनम्++ = *अन्तर्;
			अनुवर्त;
		}
		विकल्प(*(अन्तर् + 1)) {
		पक्ष 'a':
			*बहिर्गमनम्++ = '\a';	/* Bell */
			भङ्ग;
		पक्ष 'b':
			*बहिर्गमनम्++ = '\b';	/* Backspace */
			भङ्ग;
		पक्ष 'e':
			*बहिर्गमनम्++ = '\033';	/* Escape */
			भङ्ग;
		पक्ष 'f':
			*बहिर्गमनम्++ = '\f';	/* Form Feed */
			भङ्ग;
		पक्ष 'n':
			*बहिर्गमनम्++ = '\n';	/* Newline */
			भङ्ग;
		पक्ष 'r':
			*बहिर्गमनम्++ = '\r';	/* Carriage Return */
			भङ्ग;
		पक्ष 't':
			*बहिर्गमनम्++ = '\t';	/* Tab */
			भङ्ग;
		पक्ष 'v':
			*बहिर्गमनम्++ = '\v';	/* Vertical Tab */
			भङ्ग;
		पक्ष '\\':
			*बहिर्गमनम्++ = '\\';	/* Backslash */
			भङ्ग;
		पक्ष '_':
			*बहिर्गमनम्++ = ' ';	/* Space */
			भङ्ग;
		पक्ष '#':
			*बहिर्गमनम्++ = '#';	/* Hash mark */
			भङ्ग;
		पक्ष '?':
			*बहिर्गमनम्++ = '?';	/* Question mark */
			भङ्ग;
		स्वभाव:
			*बहिर्गमनम्++ = *अन्तर्;
			*बहिर्गमनम्++ = *(अन्तर् + 1);
			भङ्ग;
		}
		अन्तर्++;
	}

	यदि (बहिर्गमनम्) {
		प्रतिज्ञा ((बहिर्गमनम् - *seq) <= len);
		*बहिर्गमनम् = '\0';
	}

	प्रत्यागम 0;
}

वर्ण *color_get_sequence(ध्रुव वर्ण *color)
{
	वर्ण *seq = रिक्त;
	पूर्णाङ्क rc = __color_canonicalize(color, &seq);

	प्रत्यागम rc ? रिक्त : seq;
}
