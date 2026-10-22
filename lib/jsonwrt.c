/*
 * JSON output formatting functions.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <inttypes.h>
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <cctype.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "jsonwrt.h"

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
സ്ഥിതം ശൂന്യം fputs_quoted_case_json(സ്ഥിരം അക്ഷരം *data, FILE *out, പൂർണ്ണസംഖ്യ dir, size_t size)
{
	സ്ഥിരം അക്ഷരം *p;

	fputc('"', out);
	വേണ്ടി (p = data; p && *p && (!size || p < data + size); p++) {

		സ്ഥിരം unsigned പൂർണ്ണസംഖ്യ c = (unsigned പൂർണ്ണസംഖ്യ) *p;

		/* From http://www.json.org
		 *
		 * The double-quote and backslashes would break out a string or
		 * init an escape sequence if not escaped.
		 *
		 * Note that single-quotes and forward slashes, while they're
		 * in the JSON spec, don't break double-quoted strings.
		 */
		എങ്കിൽ (c == '"' || c == '\\') {
			fputc('\\', out);
			fputc(c, out);
			തുടരുക;
		}

		/* All non-control characters OK; do the case swap as required. */
		എങ്കിൽ (c >= 0x20) {
			/*
			 * Don't use locale sensitive ctype.h functions for regular
			 * ASCII chars, because for example with Turkish locale
			 * (aka LANG=tr_TR.UTF-8) toupper('I') returns 'I'.
			 */
			എങ്കിൽ (c <= 127)
				fputc(dir ==  1 ? c_toupper(c) :
				      dir == -1 ? c_tolower(c) : *p, out);
			അല്ലെങ്കിൽ
				fputc(dir ==  1 ? toupper(c) :
				      dir == -1 ? tolower(c) : *p, out);
			തുടരുക;
		}

		/* In addition, all chars under ' ' break Node's/V8/Chrome's, and
		 * Firefox's JSON.parse function
		 */
		തിരഞ്ഞെടുപ്പ് (c) {
			/* Handle short-hand cases to reduce output size.  C
			 * has most of the same stuff here, so if there's an
			 * "Escape for C" function somewhere in the STL, we
			 * should probably be using it.
			 */
			അവസ്ഥ '\b':
				fputs("\\b", out);
				നിർത്തുക;
			അവസ്ഥ '\t':
				fputs("\\t", out);
				നിർത്തുക;
			അവസ്ഥ '\n':
				fputs("\\n", out);
				നിർത്തുക;
			അവസ്ഥ '\f':
				fputs("\\f", out);
				നിർത്തുക;
			അവസ്ഥ '\r':
				fputs("\\r", out);
				നിർത്തുക;
			സ്ഥിരസ്ഥിതി:
				/* Other assorted control characters */
				fprintf(out, "\\u00%02x", c);
				നിർത്തുക;
		}
	}
	fputc('"', out);
}

#define fputs_quoted_json(_d, _o)       fputs_quoted_case_json(_d, _o, 0, 0)
#define fputs_quoted_json_upper(_d, _o) fputs_quoted_case_json(_d, _o, 1, 0)
#define fputs_quoted_json_lower(_d, _o) fputs_quoted_case_json(_d, _o, -1, 0)

ശൂന്യം ul_jsonwrt_init(ഘടന ul_jsonwrt *fmt, FILE *out, പൂർണ്ണസംഖ്യ indent, എണ്ണൽ ul_json_format json_format)
{
	fmt->out = out;
	fmt->indent = indent;
	fmt->json_format = json_format;
	fmt->after_close = 0;
}

പൂർണ്ണസംഖ്യ ul_jsonwrt_is_ready(ഘടന ul_jsonwrt *fmt)
{
	തിരികെ fmt->out == ശൂന്യ ? 0 : 1;
}

ശൂന്യം ul_jsonwrt_indent(ഘടന ul_jsonwrt *fmt)
{
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; i < fmt->indent; i++)
		fputs("   ", fmt->out);
}

സ്ഥിതം ശൂന്യം print_name(ഘടന ul_jsonwrt *fmt, സ്ഥിരം അക്ഷരം *name)
{
	/* Values on the top level are not members of an array or object, so
	 * they are never comma separated; ul_jsonwrt_close() has already
	 * terminated the previous one by a newline. */
	എങ്കിൽ (fmt->after_close && (fmt->indent > 0 || name)) {
		എങ്കിൽ (fmt->json_format == UL_JSON_വരി
		    || fmt->json_format == UL_JSON_COMPACT)
			fputs(",", fmt->out);
		അല്ലെങ്കിൽ
			fputs(name ? ",\n" : ",", fmt->out);
	}

	തിരഞ്ഞെടുപ്പ് (fmt->json_format) {
	അവസ്ഥ UL_JSON_വരി:
	അവസ്ഥ UL_JSON_COMPACT:
		നിർത്തുക;
	അവസ്ഥ UL_JSON_PRETTY:
	സ്ഥിരസ്ഥിതി:
		എങ്കിൽ (name || !fmt->after_close)
			ul_jsonwrt_indent(fmt);
		നിർത്തുക;
	}
	എങ്കിൽ (name)
		fputs_quoted_json_lower(name, fmt->out);
}

ശൂന്യം ul_jsonwrt_open(ഘടന ul_jsonwrt *fmt, സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ type)
{
	സ്ഥിരം അക്ഷരം *s = "";

	print_name(fmt, name);

	തിരഞ്ഞെടുപ്പ് (type) {
	അവസ്ഥ UL_JSON_OBJECT:
		എങ്കിൽ (fmt->json_format == UL_JSON_വരി)
			s = "{";
		അല്ലെങ്കിൽ എങ്കിൽ (fmt->json_format == UL_JSON_COMPACT) {
			/* At nesting depth 2, put the object opens on a new line
			 * for readability; otherwise keep fully compact (no newline). */
			എങ്കിൽ (fmt->indent == 2)
				s = name ? ":{" : "\n{";
			അല്ലെങ്കിൽ
				s = "{";
		} അല്ലെങ്കിൽ
			s = name ? ": {\n" : "{\n";
		fmt->indent++;
		നിർത്തുക;
	അവസ്ഥ UL_JSON_ARRAY:
		എങ്കിൽ (fmt->json_format == UL_JSON_വരി)
			s = "[";
		അല്ലെങ്കിൽ എങ്കിൽ (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":[" : "[";
		അല്ലെങ്കിൽ
			s = name ? ": [\n" : "[\n";
		fmt->indent++;
		നിർത്തുക;
	അവസ്ഥ UL_JSON_മൂല്യം:
		എങ്കിൽ (fmt->json_format == UL_JSON_വരി)
			s = name ? ":" : "";
		അല്ലെങ്കിൽ എങ്കിൽ (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":" : "";
		അല്ലെങ്കിൽ
			s = name ? ": " : " ";
		നിർത്തുക;
	}
	fputs(s, fmt->out);
	fmt->after_close = 0;
}

ശൂന്യം ul_jsonwrt_empty(ഘടന ul_jsonwrt *fmt, സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ type)
{
	സ്ഥിരം അക്ഷരം *s = "";

	print_name(fmt, name);

	തിരഞ്ഞെടുപ്പ് (type) {
	അവസ്ഥ UL_JSON_OBJECT:
		എങ്കിൽ (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": {}" : "{}";
		അല്ലെങ്കിൽ
			s = name ? ":{}" : "{}";
		നിർത്തുക;
	അവസ്ഥ UL_JSON_ARRAY:
		എങ്കിൽ (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": []" : "[]";
		അല്ലെങ്കിൽ
			s = name ? ":[]" : "[]";
		നിർത്തുക;
	അവസ്ഥ UL_JSON_മൂല്യം:
		എങ്കിൽ (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": null" : "null";
		അല്ലെങ്കിൽ
			s = name ? ":null" : "null";
		നിർത്തുക;
	}
	fputs(s, fmt->out);
	fmt->after_close = 1;
}

ശൂന്യം ul_jsonwrt_close(ഘടന ul_jsonwrt *fmt, പൂർണ്ണസംഖ്യ type)
{
	അക്ഷരം endchr;

	പ്രതിജ്ഞാ(fmt->indent > 0);

	തിരഞ്ഞെടുപ്പ് (type) {
	അവസ്ഥ UL_JSON_OBJECT:
		fmt->indent--;
		endchr = '}';
		നിർത്തുക;
	അവസ്ഥ UL_JSON_ARRAY:
		fmt->indent--;
		endchr = ']';
		നിർത്തുക;
	അവസ്ഥ UL_JSON_മൂല്യം:
		fmt->after_close = 1;
		തിരികെ;
	സ്ഥിരസ്ഥിതി:
		fmt->after_close = 1;
		തിരികെ;
	}

	/* In COMPACT, when closing a direct child of root (indent==1),
	 * insert a newline so the closing bracket starts on its own line. */
	എങ്കിൽ (fmt->json_format == UL_JSON_COMPACT && fmt->indent == 1)
		fputc('\n', fmt->out);

	തിരഞ്ഞെടുപ്പ് (fmt->json_format) {
	അവസ്ഥ UL_JSON_വരി:
	അവസ്ഥ UL_JSON_COMPACT:
		നിർത്തുക;
	അവസ്ഥ UL_JSON_PRETTY:
	സ്ഥിരസ്ഥിതി:
		fputs("\n", fmt->out);
		ul_jsonwrt_indent(fmt);
		നിർത്തുക;
	}

	fputc(endchr, fmt->out);

	/* Terminate the value on the top level. The writer may be re-initialized
	 * between the values (for example libsmartcols does it for every
	 * scols_table_print_range() call), so the terminator has to be printed
	 * by the value itself rather than by the next one (see print_name()). */
	എങ്കിൽ (fmt->indent == 0)
		fputc('\n', fmt->out);

	fmt->after_close = 1;
}


ശൂന്യം ul_jsonwrt_flush(ഘടന ul_jsonwrt *fmt)
{
	fflush(fmt->out);
}

ശൂന്യം ul_jsonwrt_മൂല്യം_raw(ഘടന ul_jsonwrt *fmt,
			സ്ഥിരം അക്ഷരം *name, സ്ഥിരം അക്ഷരം *data)
{
	ul_jsonwrt_മൂല്യം_open(fmt, name);
	എങ്കിൽ (data && *data)
		fputs(data, fmt->out);
	അല്ലെങ്കിൽ
		fputs("null", fmt->out);
	ul_jsonwrt_മൂല്യം_close(fmt);
}

ശൂന്യം ul_jsonwrt_മൂല്യം_s(ഘടന ul_jsonwrt *fmt,
			സ്ഥിരം അക്ഷരം *name, സ്ഥിരം അക്ഷരം *data)
{
	ul_jsonwrt_മൂല്യം_open(fmt, name);
	എങ്കിൽ (data && *data)
		fputs_quoted_json(data, fmt->out);
	അല്ലെങ്കിൽ
		fputs("null", fmt->out);
	ul_jsonwrt_മൂല്യം_close(fmt);
}

ശൂന്യം ul_jsonwrt_മൂല്യം_s_sized(ഘടന ul_jsonwrt *fmt,
			      സ്ഥിരം അക്ഷരം *name, സ്ഥിരം അക്ഷരം *data, size_t size)
{
	ul_jsonwrt_മൂല്യം_open(fmt, name);
	എങ്കിൽ (data && *data)
		fputs_quoted_case_json(data, fmt->out, 0, size);
	അല്ലെങ്കിൽ
		fputs("null", fmt->out);
	ul_jsonwrt_മൂല്യം_close(fmt);
}

ശൂന്യം ul_jsonwrt_മൂല്യം_u64(ഘടന ul_jsonwrt *fmt,
			സ്ഥിരം അക്ഷരം *name, uint64_t data)
{
	ul_jsonwrt_മൂല്യം_open(fmt, name);
	fprintf(fmt->out, "%"PRIu64, data);
	ul_jsonwrt_മൂല്യം_close(fmt);
}

ശൂന്യം ul_jsonwrt_മൂല്യം_double(ഘടന ul_jsonwrt *fmt,
			സ്ഥിരം അക്ഷരം *name, long ഇരട്ട data)
{
	ul_jsonwrt_മൂല്യം_open(fmt, name);
	fprintf(fmt->out, "%Lg", data);
	ul_jsonwrt_മൂല്യം_close(fmt);
}

ശൂന്യം ul_jsonwrt_മൂല്യം_boolean(ഘടന ul_jsonwrt *fmt,
			സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ data)
{
	ul_jsonwrt_മൂല്യം_open(fmt, name);
	fputs(data ? "true" : "false", fmt->out);
	ul_jsonwrt_മൂല്യം_close(fmt);
}
