/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2012]
 */
#అన్తర్భావ "c.h"
#అన్తర్భావ "color-names.h"

#అన్తర్భావ <ctype.h>

నిర్మాణం ul_color_name {
	స్థిరరాశి అక్షరం *name;
	స్థిరరాశి అక్షరం *seq;
};

/*
 * qsort/bsearch buddy
 */
స్థిర పూర్ణాంకం cmp_color_name(స్థిరరాశి శూన్య *a0, స్థిరరాశి శూన్య *b0)
{
	స్థిరరాశి నిర్మాణం ul_color_name
		*a = (స్థిరరాశి నిర్మాణం ul_color_name *) a0,
		*b = (స్థిరరాశి నిర్మాణం ul_color_name *) b0;
	తిరిగి strcmp(a->name, b->name);
}

/*
 * Maintains human readable color names
 */
స్థిరరాశి అక్షరం *color_sequence_from_colorname(స్థిరరాశి అక్షరం *str)
{
	స్థిర స్థిరరాశి నిర్మాణం ul_color_name basic_schemes[] = {
		{ "black",	UL_COLOR_BLACK           },
		{ "blink",      UL_COLOR_BLINK           },
		{ "blue",	UL_COLOR_BLUE            },
		{ "bold",       UL_COLOR_BOLD		 },
		{ "brown",	UL_COLOR_BROWN           },
		{ "cyan",	UL_COLOR_CYAN            },
		{ "darkgray",	UL_COLOR_DARK_బూడిద       },
		{ "gray",	UL_COLOR_బూడిద            },
		{ "green",	UL_COLOR_GREEN           },
		{ "halfbright", UL_COLOR_HALFBRIGHT	 },
		{ "lightblue",	UL_COLOR_BOLD_BLUE       },
		{ "lightcyan",	UL_COLOR_BOLD_CYAN       },
		{ "lightgray",	UL_COLOR_బూడిద            },
		{ "lightgreen", UL_COLOR_BOLD_GREEN      },
		{ "lightmagenta", UL_COLOR_BOLD_MAGENTA  },
		{ "lightred",	UL_COLOR_BOLD_RED        },
		{ "magenta",	UL_COLOR_MAGENTA         },
		{ "red",	UL_COLOR_RED             },
		{ "reset",      UL_COLOR_RESET,          },
		{ "reverse",    UL_COLOR_తిరగేయి         },
		{ "white",      UL_COLOR_WHITE           },
		{ "yellow",	UL_COLOR_BOLD_YELLOW     }
	};
	నిర్మాణం ul_color_name కీలకం = { .name = str };
	స్థిరరాశి నిర్మాణం ul_color_name *res;

	ఒకవేళ (!str)
		తిరిగి ఖాళీ;

	res = bsearch(&కీలకం, basic_schemes, ARRAY_కొలత(basic_schemes),
				పరిమాణం(నిర్మాణం ul_color_name),
				cmp_color_name);
	తిరిగి res ? res->seq : ఖాళీ;
}


పూర్ణాంకం color_is_sequence(స్థిరరాశి అక్షరం *color)
{
	ఒకవేళ (color && *color == 0x1B) {
		size_t len = అక్షరశ్రేణి_పొడవు(color);

		ఒకవేళ (len >= 4 &&
		    *(color + 1) == '[' &&
		    అంకెనా(*(color + 2)) &&
		    *(color + len - 1) == 'm')
		తిరిగి 1;
	}

	తిరిగి 0;
}

/* canonicalize sequence */
స్థిర పూర్ణాంకం __color_canonicalize(స్థిరరాశి అక్షరం *str, అక్షరం **seq)
{
	అక్షరం *లోపల, *బహిర్గతం;
	పూర్ణాంకం len;

	ఒకవేళ (!str)
		తిరిగి -EINVAL;

	*seq = ఖాళీ;

	/* convert color names like "red" to the real sequence */
	ఒకవేళ (*str != '\\' && !strchr(str, ';') && అక్షరమా(*str)) {
		స్థిరరాశి అక్షరం *s = color_sequence_from_colorname(str);

		ఒకవేళ (s) {
			*seq = strdup(s);
			తిరిగి *seq ? 0 : -ENOMEM;
		}
		తిరిగి 1;
	}

	/* convert xx;yy sequences to "\033[xx;yy" */
	ఒకవేళ ((len = asprintf(seq, "\033[%sm", str)) < 1)
		తిరిగి -ENOMEM;

	కొరకు (లోపల = *seq, బహిర్గతం = *seq; లోపల && *లోపల; లోపల++) {
		ఒకవేళ (*లోపల != '\\') {
			*బహిర్గతం++ = *లోపల;
			కొనసాగు;
		}
		ఎంపిక(*(లోపల + 1)) {
		స్థితి 'a':
			*బహిర్గతం++ = '\a';	/* Bell */
			ఆపు;
		స్థితి 'b':
			*బహిర్గతం++ = '\b';	/* Backspace */
			ఆపు;
		స్థితి 'e':
			*బహిర్గతం++ = '\033';	/* Escape */
			ఆపు;
		స్థితి 'f':
			*బహిర్గతం++ = '\f';	/* Form Feed */
			ఆపు;
		స్థితి 'n':
			*బహిర్గతం++ = '\n';	/* Newline */
			ఆపు;
		స్థితి 'r':
			*బహిర్గతం++ = '\r';	/* Carriage Return */
			ఆపు;
		స్థితి 't':
			*బహిర్గతం++ = '\t';	/* Tab */
			ఆపు;
		స్థితి 'v':
			*బహిర్గతం++ = '\v';	/* Vertical Tab */
			ఆపు;
		స్థితి '\\':
			*బహిర్గతం++ = '\\';	/* Backslash */
			ఆపు;
		స్థితి '_':
			*బహిర్గతం++ = ' ';	/* Space */
			ఆపు;
		స్థితి '#':
			*బహిర్గతం++ = '#';	/* Hash mark */
			ఆపు;
		స్థితి '?':
			*బహిర్గతం++ = '?';	/* Question mark */
			ఆపు;
		అప్రమేయ:
			*బహిర్గతం++ = *లోపల;
			*బహిర్గతం++ = *(లోపల + 1);
			ఆపు;
		}
		లోపల++;
	}

	ఒకవేళ (బహిర్గతం) {
		ప్రతిజ్ఞా ((బహిర్గతం - *seq) <= len);
		*బహిర్గతం = '\0';
	}

	తిరిగి 0;
}

అక్షరం *color_get_sequence(స్థిరరాశి అక్షరం *color)
{
	అక్షరం *seq = ఖాళీ;
	పూర్ణాంకం rc = __color_canonicalize(color, &seq);

	తిరిగి rc ? ఖాళీ : seq;
}
