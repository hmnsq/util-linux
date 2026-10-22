/*
 * JSON output formatting functions.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <inttypes.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <cctype.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "jsonwrt.h"

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
સ્થિર શૂન્ય fputs_quoted_case_json(અચલ અક્ષર *data, FILE *out, પૂર્ણાંક dir, size_t size)
{
	અચલ અક્ષર *p;

	fputc('"', out);
	માટે (p = data; p && *p && (!size || p < data + size); p++) {

		અચલ unsigned પૂર્ણાંક c = (unsigned પૂર્ણાંક) *p;

		/* From http://www.json.org
		 *
		 * The double-quote and backslashes would break out a string or
		 * init an escape sequence if not escaped.
		 *
		 * Note that single-quotes and forward slashes, while they're
		 * in the JSON spec, don't break double-quoted strings.
		 */
		જો (c == '"' || c == '\\') {
			fputc('\\', out);
			fputc(c, out);
			ચાલુ;
		}

		/* All non-control characters OK; do the case swap as required. */
		જો (c >= 0x20) {
			/*
			 * Don't use locale sensitive ctype.h functions for regular
			 * ASCII chars, because for example with Turkish locale
			 * (aka LANG=tr_TR.UTF-8) toupper('I') returns 'I'.
			 */
			જો (c <= 127)
				fputc(dir ==  1 ? c_toupper(c) :
				      dir == -1 ? c_tolower(c) : *p, out);
			નહીંતર
				fputc(dir ==  1 ? toupper(c) :
				      dir == -1 ? tolower(c) : *p, out);
			ચાલુ;
		}

		/* In addition, all chars under ' ' break Node's/V8/Chrome's, and
		 * Firefox's JSON.parse function
		 */
		પસંદગી (c) {
			/* Handle short-hand cases to reduce output size.  C
			 * has most of the same stuff here, so if there's an
			 * "Escape for C" function somewhere in the STL, we
			 * should probably be using it.
			 */
			સ્થિતિ '\b':
				fputs("\\b", out);
				તોડો;
			સ્થિતિ '\t':
				fputs("\\t", out);
				તોડો;
			સ્થિતિ '\n':
				fputs("\\n", out);
				તોડો;
			સ્થિતિ '\f':
				fputs("\\f", out);
				તોડો;
			સ્થિતિ '\r':
				fputs("\\r", out);
				તોડો;
			મૂળભૂત:
				/* Other assorted control characters */
				fprintf(out, "\\u00%02x", c);
				તોડો;
		}
	}
	fputc('"', out);
}

#define fputs_quoted_json(_d, _o)       fputs_quoted_case_json(_d, _o, 0, 0)
#define fputs_quoted_json_upper(_d, _o) fputs_quoted_case_json(_d, _o, 1, 0)
#define fputs_quoted_json_lower(_d, _o) fputs_quoted_case_json(_d, _o, -1, 0)

શૂન્ય ul_jsonwrt_init(સંરચના ul_jsonwrt *fmt, FILE *out, પૂર્ણાંક indent, ગણના ul_json_format json_format)
{
	fmt->out = out;
	fmt->indent = indent;
	fmt->json_format = json_format;
	fmt->after_close = 0;
}

પૂર્ણાંક ul_jsonwrt_is_ready(સંરચના ul_jsonwrt *fmt)
{
	પાછા fmt->out == ખાલી ? 0 : 1;
}

શૂન્ય ul_jsonwrt_indent(સંરચના ul_jsonwrt *fmt)
{
	પૂર્ણાંક i;

	માટે (i = 0; i < fmt->indent; i++)
		fputs("   ", fmt->out);
}

સ્થિર શૂન્ય print_name(સંરચના ul_jsonwrt *fmt, અચલ અક્ષર *name)
{
	/* Values on the top level are not members of an array or object, so
	 * they are never comma separated; ul_jsonwrt_close() has already
	 * terminated the previous one by a newline. */
	જો (fmt->after_close && (fmt->indent > 0 || name)) {
		જો (fmt->json_format == UL_JSON_લીટી
		    || fmt->json_format == UL_JSON_COMPACT)
			fputs(",", fmt->out);
		નહીંતર
			fputs(name ? ",\n" : ",", fmt->out);
	}

	પસંદગી (fmt->json_format) {
	સ્થિતિ UL_JSON_લીટી:
	સ્થિતિ UL_JSON_COMPACT:
		તોડો;
	સ્થિતિ UL_JSON_PRETTY:
	મૂળભૂત:
		જો (name || !fmt->after_close)
			ul_jsonwrt_indent(fmt);
		તોડો;
	}
	જો (name)
		fputs_quoted_json_lower(name, fmt->out);
}

શૂન્ય ul_jsonwrt_open(સંરચના ul_jsonwrt *fmt, અચલ અક્ષર *name, પૂર્ણાંક type)
{
	અચલ અક્ષર *s = "";

	print_name(fmt, name);

	પસંદગી (type) {
	સ્થિતિ UL_JSON_OBJECT:
		જો (fmt->json_format == UL_JSON_લીટી)
			s = "{";
		નહીંતર જો (fmt->json_format == UL_JSON_COMPACT) {
			/* At nesting depth 2, put the object opens on a new line
			 * for readability; otherwise keep fully compact (no newline). */
			જો (fmt->indent == 2)
				s = name ? ":{" : "\n{";
			નહીંતર
				s = "{";
		} નહીંતર
			s = name ? ": {\n" : "{\n";
		fmt->indent++;
		તોડો;
	સ્થિતિ UL_JSON_ARRAY:
		જો (fmt->json_format == UL_JSON_લીટી)
			s = "[";
		નહીંતર જો (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":[" : "[";
		નહીંતર
			s = name ? ": [\n" : "[\n";
		fmt->indent++;
		તોડો;
	સ્થિતિ UL_JSON_કિંમત:
		જો (fmt->json_format == UL_JSON_લીટી)
			s = name ? ":" : "";
		નહીંતર જો (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":" : "";
		નહીંતર
			s = name ? ": " : " ";
		તોડો;
	}
	fputs(s, fmt->out);
	fmt->after_close = 0;
}

શૂન્ય ul_jsonwrt_empty(સંરચના ul_jsonwrt *fmt, અચલ અક્ષર *name, પૂર્ણાંક type)
{
	અચલ અક્ષર *s = "";

	print_name(fmt, name);

	પસંદગી (type) {
	સ્થિતિ UL_JSON_OBJECT:
		જો (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": {}" : "{}";
		નહીંતર
			s = name ? ":{}" : "{}";
		તોડો;
	સ્થિતિ UL_JSON_ARRAY:
		જો (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": []" : "[]";
		નહીંતર
			s = name ? ":[]" : "[]";
		તોડો;
	સ્થિતિ UL_JSON_કિંમત:
		જો (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": null" : "null";
		નહીંતર
			s = name ? ":null" : "null";
		તોડો;
	}
	fputs(s, fmt->out);
	fmt->after_close = 1;
}

શૂન્ય ul_jsonwrt_close(સંરચના ul_jsonwrt *fmt, પૂર્ણાંક type)
{
	અક્ષર endchr;

	પ્રતિજ્ઞા(fmt->indent > 0);

	પસંદગી (type) {
	સ્થિતિ UL_JSON_OBJECT:
		fmt->indent--;
		endchr = '}';
		તોડો;
	સ્થિતિ UL_JSON_ARRAY:
		fmt->indent--;
		endchr = ']';
		તોડો;
	સ્થિતિ UL_JSON_કિંમત:
		fmt->after_close = 1;
		પાછા;
	મૂળભૂત:
		fmt->after_close = 1;
		પાછા;
	}

	/* In COMPACT, when closing a direct child of root (indent==1),
	 * insert a newline so the closing bracket starts on its own line. */
	જો (fmt->json_format == UL_JSON_COMPACT && fmt->indent == 1)
		fputc('\n', fmt->out);

	પસંદગી (fmt->json_format) {
	સ્થિતિ UL_JSON_લીટી:
	સ્થિતિ UL_JSON_COMPACT:
		તોડો;
	સ્થિતિ UL_JSON_PRETTY:
	મૂળભૂત:
		fputs("\n", fmt->out);
		ul_jsonwrt_indent(fmt);
		તોડો;
	}

	fputc(endchr, fmt->out);

	/* Terminate the value on the top level. The writer may be re-initialized
	 * between the values (for example libsmartcols does it for every
	 * scols_table_print_range() call), so the terminator has to be printed
	 * by the value itself rather than by the next one (see print_name()). */
	જો (fmt->indent == 0)
		fputc('\n', fmt->out);

	fmt->after_close = 1;
}


શૂન્ય ul_jsonwrt_flush(સંરચના ul_jsonwrt *fmt)
{
	fflush(fmt->out);
}

શૂન્ય ul_jsonwrt_કિંમત_raw(સંરચના ul_jsonwrt *fmt,
			અચલ અક્ષર *name, અચલ અક્ષર *data)
{
	ul_jsonwrt_કિંમત_open(fmt, name);
	જો (data && *data)
		fputs(data, fmt->out);
	નહીંતર
		fputs("null", fmt->out);
	ul_jsonwrt_કિંમત_close(fmt);
}

શૂન્ય ul_jsonwrt_કિંમત_s(સંરચના ul_jsonwrt *fmt,
			અચલ અક્ષર *name, અચલ અક્ષર *data)
{
	ul_jsonwrt_કિંમત_open(fmt, name);
	જો (data && *data)
		fputs_quoted_json(data, fmt->out);
	નહીંતર
		fputs("null", fmt->out);
	ul_jsonwrt_કિંમત_close(fmt);
}

શૂન્ય ul_jsonwrt_કિંમત_s_sized(સંરચના ul_jsonwrt *fmt,
			      અચલ અક્ષર *name, અચલ અક્ષર *data, size_t size)
{
	ul_jsonwrt_કિંમત_open(fmt, name);
	જો (data && *data)
		fputs_quoted_case_json(data, fmt->out, 0, size);
	નહીંતર
		fputs("null", fmt->out);
	ul_jsonwrt_કિંમત_close(fmt);
}

શૂન્ય ul_jsonwrt_કિંમત_u64(સંરચના ul_jsonwrt *fmt,
			અચલ અક્ષર *name, uint64_t data)
{
	ul_jsonwrt_કિંમત_open(fmt, name);
	fprintf(fmt->out, "%"PRIu64, data);
	ul_jsonwrt_કિંમત_close(fmt);
}

શૂન્ય ul_jsonwrt_કિંમત_double(સંરચના ul_jsonwrt *fmt,
			અચલ અક્ષર *name, long દ્વિગુણ data)
{
	ul_jsonwrt_કિંમત_open(fmt, name);
	fprintf(fmt->out, "%Lg", data);
	ul_jsonwrt_કિંમત_close(fmt);
}

શૂન્ય ul_jsonwrt_કિંમત_boolean(સંરચના ul_jsonwrt *fmt,
			અચલ અક્ષર *name, પૂર્ણાંક data)
{
	ul_jsonwrt_કિંમત_open(fmt, name);
	fputs(data ? "true" : "false", fmt->out);
	ul_jsonwrt_કિંમત_close(fmt);
}
