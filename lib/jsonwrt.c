/*
 * JSON output formatting functions.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <inttypes.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <cctype.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "jsonwrt.h"

/*
 * Requirements enumerated via testing (V8, Firefox, IE11):
 *
 * var charsToEscape = [];
 * for (var i = 0; i < 65535; i += 1) {
 *	try {
 *		JSON.parse('{"sample": "' + String.fromCodePoint(i) + '"}');
 *	} catch (e) {
 *		charsToEscape.push(i);
 *	}
 * }
 */
ಸ್ಥಿರ ಶೂನ್ಯ fputs_quoted_case_json(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ, FILE *ಹೊರಹೋಗು, ಪೂರ್ಣಾಂಕ dir, size_t ಗಾತ್ರ_2)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;

	fputc('"', ಹೊರಹೋಗು);
	ಗಾಗಿ (p = ದತ್ತಾಂಶ; p && *p && (!ಗಾತ್ರ_2 || p < ದತ್ತಾಂಶ + ಗಾತ್ರ_2); p++) {

		ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ c = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) *p;

		/* From http://www.json.org
		 *
		 * The double-quote and backslashes would break out a string or
		 * init an escape sequence if not escaped.
		 *
		 * Note that single-quotes and forward slashes, while they're
		 * in the JSON spec, don't break double-quoted strings.
		 */
		ಆದರೆ (c == '"' || c == '\\') {
			fputc('\\', ಹೊರಹೋಗು);
			fputc(c, ಹೊರಹೋಗು);
			ಮುಂದುವರಿ;
		}

		/* All non-control characters OK; do the case swap as required. */
		ಆದರೆ (c >= 0x20) {
			/*
			 * Don't use locale sensitive ctype.h functions for regular
			 * ASCII chars, because for example with Turkish locale
			 * (aka LANG=tr_TR.UTF-8) toupper('I') returns 'I'.
			 */
			ಆದರೆ (c <= 127)
				fputc(dir ==  1 ? c_toupper(c) :
				      dir == -1 ? c_tolower(c) : *p, ಹೊರಹೋಗು);
			ಇಲ್ಲದಿದ್ದರೆ
				fputc(dir ==  1 ? ದೊಡ್ಡ_ಅಕ್ಷರ(c) :
				      dir == -1 ? tolower(c) : *p, ಹೊರಹೋಗು);
			ಮುಂದುವರಿ;
		}

		/* In addition, all chars under ' ' break Node's/V8/Chrome's, and
		 * Firefox's JSON.parse function
		 */
		ಆಯ್ಕೆ (c) {
			/* Handle short-hand cases to reduce output size.  C
			 * has most of the same stuff here, so if there's an
			 * "Escape for C" function somewhere in the STL, we
			 * should probably be using it.
			 */
			ಸ್ಥಿತಿ '\b':
				fputs("\\b", ಹೊರಹೋಗು);
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ '\t':
				fputs("\\t", ಹೊರಹೋಗು);
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ '\n':
				fputs("\\n", ಹೊರಹೋಗು);
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ '\f':
				fputs("\\f", ಹೊರಹೋಗು);
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ '\r':
				fputs("\\r", ಹೊರಹೋಗು);
				ನಿಲ್ಲಿಸು;
			ಪೂರ್ವನಿಯೋಜಿತ:
				/* Other assorted control characters */
				fprintf(ಹೊರಹೋಗು, "\\u00%02x", c);
				ನಿಲ್ಲಿಸು;
		}
	}
	fputc('"', ಹೊರಹೋಗು);
}

#ವ್ಯಾಖ್ಯಾನಿಸು fputs_quoted_json(_d, _o)       fputs_quoted_case_json(_d, _o, 0, 0)
#ವ್ಯಾಖ್ಯಾನಿಸು fputs_quoted_json_ಊರ್ಧ್ವ(_d, _o) fputs_quoted_case_json(_d, _o, 1, 0)
#ವ್ಯಾಖ್ಯಾನಿಸು fputs_quoted_json_ಅಧೋ(_d, _o) fputs_quoted_case_json(_d, _o, -1, 0)

ಶೂನ್ಯ ul_jsonwrt_init(ರಚನೆ ul_jsonwrt *fmt, FILE *ಹೊರಹೋಗು, ಪೂರ್ಣಾಂಕ indent, ಎಣಿಕೆ ul_json_format json_format)
{
	fmt->ಹೊರಹೋಗು = ಹೊರಹೋಗು;
	fmt->indent = indent;
	fmt->json_format = json_format;
	fmt->after_close = 0;
}

ಪೂರ್ಣಾಂಕ ul_jsonwrt_is_ready(ರಚನೆ ul_jsonwrt *fmt)
{
	ಹಿಂತಿರುಗಿ fmt->ಹೊರಹೋಗು == ಖಾಲಿ ? 0 : 1;
}

ಶೂನ್ಯ ul_jsonwrt_indent(ರಚನೆ ul_jsonwrt *fmt)
{
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < fmt->indent; i++)
		fputs("   ", fmt->ಹೊರಹೋಗು);
}

ಸ್ಥಿರ ಶೂನ್ಯ print_name(ರಚನೆ ul_jsonwrt *fmt, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	/* Values on the top level are not members of an array or object, so
	 * they are never comma separated; ul_jsonwrt_close() has already
	 * terminated the previous one by a newline. */
	ಆದರೆ (fmt->after_close && (fmt->indent > 0 || name)) {
		ಆದರೆ (fmt->json_format == UL_JSON_ಸಾಲು
		    || fmt->json_format == UL_JSON_COMPACT)
			fputs(",", fmt->ಹೊರಹೋಗು);
		ಇಲ್ಲದಿದ್ದರೆ
			fputs(name ? ",\n" : ",", fmt->ಹೊರಹೋಗು);
	}

	ಆಯ್ಕೆ (fmt->json_format) {
	ಸ್ಥಿತಿ UL_JSON_ಸಾಲು:
	ಸ್ಥಿತಿ UL_JSON_COMPACT:
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ UL_JSON_PRETTY:
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಆದರೆ (name || !fmt->after_close)
			ul_jsonwrt_indent(fmt);
		ನಿಲ್ಲಿಸು;
	}
	ಆದರೆ (name)
		fputs_quoted_json_ಅಧೋ(name, fmt->ಹೊರಹೋಗು);
}

ಶೂನ್ಯ ul_jsonwrt_open(ರಚನೆ ul_jsonwrt *fmt, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ type)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = "";

	print_name(fmt, name);

	ಆಯ್ಕೆ (type) {
	ಸ್ಥಿತಿ UL_JSON_OBJECT:
		ಆದರೆ (fmt->json_format == UL_JSON_ಸಾಲು)
			s = "{";
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (fmt->json_format == UL_JSON_COMPACT) {
			/* At nesting depth 2, put the object opens on a new line
			 * for readability; otherwise keep fully compact (no newline). */
			ಆದರೆ (fmt->indent == 2)
				s = name ? ":{" : "\n{";
			ಇಲ್ಲದಿದ್ದರೆ
				s = "{";
		} ಇಲ್ಲದಿದ್ದರೆ
			s = name ? ": {\n" : "{\n";
		fmt->indent++;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ UL_JSON_ARRAY:
		ಆದರೆ (fmt->json_format == UL_JSON_ಸಾಲು)
			s = "[";
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":[" : "[";
		ಇಲ್ಲದಿದ್ದರೆ
			s = name ? ": [\n" : "[\n";
		fmt->indent++;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ UL_JSON_ಮೌಲ್ಯ:
		ಆದರೆ (fmt->json_format == UL_JSON_ಸಾಲು)
			s = name ? ":" : "";
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":" : "";
		ಇಲ್ಲದಿದ್ದರೆ
			s = name ? ": " : " ";
		ನಿಲ್ಲಿಸು;
	}
	fputs(s, fmt->ಹೊರಹೋಗು);
	fmt->after_close = 0;
}

ಶೂನ್ಯ ul_jsonwrt_ಖಾಲಿಗಳು(ರಚನೆ ul_jsonwrt *fmt, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ type)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = "";

	print_name(fmt, name);

	ಆಯ್ಕೆ (type) {
	ಸ್ಥಿತಿ UL_JSON_OBJECT:
		ಆದರೆ (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": {}" : "{}";
		ಇಲ್ಲದಿದ್ದರೆ
			s = name ? ":{}" : "{}";
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ UL_JSON_ARRAY:
		ಆದರೆ (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": []" : "[]";
		ಇಲ್ಲದಿದ್ದರೆ
			s = name ? ":[]" : "[]";
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ UL_JSON_ಮೌಲ್ಯ:
		ಆದರೆ (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": null" : "null";
		ಇಲ್ಲದಿದ್ದರೆ
			s = name ? ":null" : "null";
		ನಿಲ್ಲಿಸು;
	}
	fputs(s, fmt->ಹೊರಹೋಗು);
	fmt->after_close = 1;
}

ಶೂನ್ಯ ul_jsonwrt_close(ರಚನೆ ul_jsonwrt *fmt, ಪೂರ್ಣಾಂಕ type)
{
	ಅಕ್ಷರ endchr;

	ಪ್ರತಿಜ್ಞಾ(fmt->indent > 0);

	ಆಯ್ಕೆ (type) {
	ಸ್ಥಿತಿ UL_JSON_OBJECT:
		fmt->indent--;
		endchr = '}';
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ UL_JSON_ARRAY:
		fmt->indent--;
		endchr = ']';
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ UL_JSON_ಮೌಲ್ಯ:
		fmt->after_close = 1;
		ಹಿಂತಿರುಗಿ;
	ಪೂರ್ವನಿಯೋಜಿತ:
		fmt->after_close = 1;
		ಹಿಂತಿರುಗಿ;
	}

	/* In COMPACT, when closing a direct child of root (indent==1),
	 * insert a newline so the closing bracket starts on its own line. */
	ಆದರೆ (fmt->json_format == UL_JSON_COMPACT && fmt->indent == 1)
		fputc('\n', fmt->ಹೊರಹೋಗು);

	ಆಯ್ಕೆ (fmt->json_format) {
	ಸ್ಥಿತಿ UL_JSON_ಸಾಲು:
	ಸ್ಥಿತಿ UL_JSON_COMPACT:
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ UL_JSON_PRETTY:
	ಪೂರ್ವನಿಯೋಜಿತ:
		fputs("\n", fmt->ಹೊರಹೋಗು);
		ul_jsonwrt_indent(fmt);
		ನಿಲ್ಲಿಸು;
	}

	fputc(endchr, fmt->ಹೊರಹೋಗು);

	/* Terminate the value on the top level. The writer may be re-initialized
	 * between the values (for example libsmartcols does it for every
	 * scols_table_print_range() call), so the terminator has to be printed
	 * by the value itself rather than by the next one (see print_name()). */
	ಆದರೆ (fmt->indent == 0)
		fputc('\n', fmt->ಹೊರಹೋಗು);

	fmt->after_close = 1;
}


ಶೂನ್ಯ ul_jsonwrt_flush(ರಚನೆ ul_jsonwrt *fmt)
{
	fflush(fmt->ಹೊರಹೋಗು);
}

ಶೂನ್ಯ ul_jsonwrt_ಮೌಲ್ಯ_raw(ರಚನೆ ul_jsonwrt *fmt,
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ)
{
	ul_jsonwrt_ಮೌಲ್ಯ_open(fmt, name);
	ಆದರೆ (ದತ್ತಾಂಶ && *ದತ್ತಾಂಶ)
		fputs(ದತ್ತಾಂಶ, fmt->ಹೊರಹೋಗು);
	ಇಲ್ಲದಿದ್ದರೆ
		fputs("null", fmt->ಹೊರಹೋಗು);
	ul_jsonwrt_ಮೌಲ್ಯ_close(fmt);
}

ಶೂನ್ಯ ul_jsonwrt_ಮೌಲ್ಯ_s(ರಚನೆ ul_jsonwrt *fmt,
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ)
{
	ul_jsonwrt_ಮೌಲ್ಯ_open(fmt, name);
	ಆದರೆ (ದತ್ತಾಂಶ && *ದತ್ತಾಂಶ)
		fputs_quoted_json(ದತ್ತಾಂಶ, fmt->ಹೊರಹೋಗು);
	ಇಲ್ಲದಿದ್ದರೆ
		fputs("null", fmt->ಹೊರಹೋಗು);
	ul_jsonwrt_ಮೌಲ್ಯ_close(fmt);
}

ಶೂನ್ಯ ul_jsonwrt_ಮೌಲ್ಯ_s_sized(ರಚನೆ ul_jsonwrt *fmt,
			      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ, size_t ಗಾತ್ರ_2)
{
	ul_jsonwrt_ಮೌಲ್ಯ_open(fmt, name);
	ಆದರೆ (ದತ್ತಾಂಶ && *ದತ್ತಾಂಶ)
		fputs_quoted_case_json(ದತ್ತಾಂಶ, fmt->ಹೊರಹೋಗು, 0, ಗಾತ್ರ_2);
	ಇಲ್ಲದಿದ್ದರೆ
		fputs("null", fmt->ಹೊರಹೋಗು);
	ul_jsonwrt_ಮೌಲ್ಯ_close(fmt);
}

ಶೂನ್ಯ ul_jsonwrt_ಮೌಲ್ಯ_u64(ರಚನೆ ul_jsonwrt *fmt,
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, uint64_t ದತ್ತಾಂಶ)
{
	ul_jsonwrt_ಮೌಲ್ಯ_open(fmt, name);
	fprintf(fmt->ಹೊರಹೋಗು, "%"PRIu64, ದತ್ತಾಂಶ);
	ul_jsonwrt_ಮೌಲ್ಯ_close(fmt);
}

ಶೂನ್ಯ ul_jsonwrt_ಮೌಲ್ಯ_double(ರಚನೆ ul_jsonwrt *fmt,
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ದೀರ್ಘ ದ್ವಿಗುಣ ದತ್ತಾಂಶ)
{
	ul_jsonwrt_ಮೌಲ್ಯ_open(fmt, name);
	fprintf(fmt->ಹೊರಹೋಗು, "%Lg", ದತ್ತಾಂಶ);
	ul_jsonwrt_ಮೌಲ್ಯ_close(fmt);
}

ಶೂನ್ಯ ul_jsonwrt_ಮೌಲ್ಯ_boolean(ರಚನೆ ul_jsonwrt *fmt,
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ ದತ್ತಾಂಶ)
{
	ul_jsonwrt_ಮೌಲ್ಯ_open(fmt, name);
	fputs(ದತ್ತಾಂಶ ? "true" : "false", fmt->ಹೊರಹೋಗು);
	ul_jsonwrt_ಮೌಲ್ಯ_close(fmt);
}
