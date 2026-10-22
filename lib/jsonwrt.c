/*
 * JSON output formatting functions.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <inttypes.h>
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <cctype.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "jsonwrt.h"

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
স্থির শূন্য fputs_quoted_case_json(ধ্রুব বর্ণ *data, FILE *out, পূর্ণাঙ্ক dir, size_t size)
{
	ধ্রুব বর্ণ *p;

	fputc('"', out);
	প্রতি (p = data; p && *p && (!size || p < data + size); p++) {

		ধ্রুব unsigned পূর্ণাঙ্ক c = (unsigned পূর্ণাঙ্ক) *p;

		/* From http://www.json.org
		 *
		 * The double-quote and backslashes would break out a string or
		 * init an escape sequence if not escaped.
		 *
		 * Note that single-quotes and forward slashes, while they're
		 * in the JSON spec, don't break double-quoted strings.
		 */
		যদি (c == '"' || c == '\\') {
			fputc('\\', out);
			fputc(c, out);
			অনুবর্ত;
		}

		/* All non-control characters OK; do the case swap as required. */
		যদি (c >= 0x20) {
			/*
			 * Don't use locale sensitive ctype.h functions for regular
			 * ASCII chars, because for example with Turkish locale
			 * (aka LANG=tr_TR.UTF-8) toupper('I') returns 'I'.
			 */
			যদি (c <= 127)
				fputc(dir ==  1 ? c_toupper(c) :
				      dir == -1 ? c_tolower(c) : *p, out);
			অন্যথা
				fputc(dir ==  1 ? toupper(c) :
				      dir == -1 ? tolower(c) : *p, out);
			অনুবর্ত;
		}

		/* In addition, all chars under ' ' break Node's/V8/Chrome's, and
		 * Firefox's JSON.parse function
		 */
		বিকল্প (c) {
			/* Handle short-hand cases to reduce output size.  C
			 * has most of the same stuff here, so if there's an
			 * "Escape for C" function somewhere in the STL, we
			 * should probably be using it.
			 */
			পক্ষ '\b':
				fputs("\\b", out);
				ভঙ্গ;
			পক্ষ '\t':
				fputs("\\t", out);
				ভঙ্গ;
			পক্ষ '\n':
				fputs("\\n", out);
				ভঙ্গ;
			পক্ষ '\f':
				fputs("\\f", out);
				ভঙ্গ;
			পক্ষ '\r':
				fputs("\\r", out);
				ভঙ্গ;
			স্বভাব:
				/* Other assorted control characters */
				fprintf(out, "\\u00%02x", c);
				ভঙ্গ;
		}
	}
	fputc('"', out);
}

#define fputs_quoted_json(_d, _o)       fputs_quoted_case_json(_d, _o, 0, 0)
#define fputs_quoted_json_upper(_d, _o) fputs_quoted_case_json(_d, _o, 1, 0)
#define fputs_quoted_json_lower(_d, _o) fputs_quoted_case_json(_d, _o, -1, 0)

শূন্য ul_jsonwrt_init(সংরচনা ul_jsonwrt *fmt, FILE *out, পূর্ণাঙ্ক indent, গণনা ul_json_format json_format)
{
	fmt->out = out;
	fmt->indent = indent;
	fmt->json_format = json_format;
	fmt->after_close = 0;
}

পূর্ণাঙ্ক ul_jsonwrt_is_ready(সংরচনা ul_jsonwrt *fmt)
{
	প্রত্যাগম fmt->out == রিক্ত ? 0 : 1;
}

শূন্য ul_jsonwrt_indent(সংরচনা ul_jsonwrt *fmt)
{
	পূর্ণাঙ্ক i;

	প্রতি (i = 0; i < fmt->indent; i++)
		fputs("   ", fmt->out);
}

স্থির শূন্য print_name(সংরচনা ul_jsonwrt *fmt, ধ্রুব বর্ণ *name)
{
	/* Values on the top level are not members of an array or object, so
	 * they are never comma separated; ul_jsonwrt_close() has already
	 * terminated the previous one by a newline. */
	যদি (fmt->after_close && (fmt->indent > 0 || name)) {
		যদি (fmt->json_format == UL_JSON_সারি
		    || fmt->json_format == UL_JSON_COMPACT)
			fputs(",", fmt->out);
		অন্যথা
			fputs(name ? ",\n" : ",", fmt->out);
	}

	বিকল্প (fmt->json_format) {
	পক্ষ UL_JSON_সারি:
	পক্ষ UL_JSON_COMPACT:
		ভঙ্গ;
	পক্ষ UL_JSON_PRETTY:
	স্বভাব:
		যদি (name || !fmt->after_close)
			ul_jsonwrt_indent(fmt);
		ভঙ্গ;
	}
	যদি (name)
		fputs_quoted_json_lower(name, fmt->out);
}

শূন্য ul_jsonwrt_open(সংরচনা ul_jsonwrt *fmt, ধ্রুব বর্ণ *name, পূর্ণাঙ্ক type)
{
	ধ্রুব বর্ণ *s = "";

	print_name(fmt, name);

	বিকল্প (type) {
	পক্ষ UL_JSON_OBJECT:
		যদি (fmt->json_format == UL_JSON_সারি)
			s = "{";
		অন্যথা যদি (fmt->json_format == UL_JSON_COMPACT) {
			/* At nesting depth 2, put the object opens on a new line
			 * for readability; otherwise keep fully compact (no newline). */
			যদি (fmt->indent == 2)
				s = name ? ":{" : "\n{";
			অন্যথা
				s = "{";
		} অন্যথা
			s = name ? ": {\n" : "{\n";
		fmt->indent++;
		ভঙ্গ;
	পক্ষ UL_JSON_ARRAY:
		যদি (fmt->json_format == UL_JSON_সারি)
			s = "[";
		অন্যথা যদি (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":[" : "[";
		অন্যথা
			s = name ? ": [\n" : "[\n";
		fmt->indent++;
		ভঙ্গ;
	পক্ষ UL_JSON_মান:
		যদি (fmt->json_format == UL_JSON_সারি)
			s = name ? ":" : "";
		অন্যথা যদি (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":" : "";
		অন্যথা
			s = name ? ": " : " ";
		ভঙ্গ;
	}
	fputs(s, fmt->out);
	fmt->after_close = 0;
}

শূন্য ul_jsonwrt_empty(সংরচনা ul_jsonwrt *fmt, ধ্রুব বর্ণ *name, পূর্ণাঙ্ক type)
{
	ধ্রুব বর্ণ *s = "";

	print_name(fmt, name);

	বিকল্প (type) {
	পক্ষ UL_JSON_OBJECT:
		যদি (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": {}" : "{}";
		অন্যথা
			s = name ? ":{}" : "{}";
		ভঙ্গ;
	পক্ষ UL_JSON_ARRAY:
		যদি (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": []" : "[]";
		অন্যথা
			s = name ? ":[]" : "[]";
		ভঙ্গ;
	পক্ষ UL_JSON_মান:
		যদি (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": null" : "null";
		অন্যথা
			s = name ? ":null" : "null";
		ভঙ্গ;
	}
	fputs(s, fmt->out);
	fmt->after_close = 1;
}

শূন্য ul_jsonwrt_close(সংরচনা ul_jsonwrt *fmt, পূর্ণাঙ্ক type)
{
	বর্ণ endchr;

	প্রতিজ্ঞা(fmt->indent > 0);

	বিকল্প (type) {
	পক্ষ UL_JSON_OBJECT:
		fmt->indent--;
		endchr = '}';
		ভঙ্গ;
	পক্ষ UL_JSON_ARRAY:
		fmt->indent--;
		endchr = ']';
		ভঙ্গ;
	পক্ষ UL_JSON_মান:
		fmt->after_close = 1;
		প্রত্যাগম;
	স্বভাব:
		fmt->after_close = 1;
		প্রত্যাগম;
	}

	/* In COMPACT, when closing a direct child of root (indent==1),
	 * insert a newline so the closing bracket starts on its own line. */
	যদি (fmt->json_format == UL_JSON_COMPACT && fmt->indent == 1)
		fputc('\n', fmt->out);

	বিকল্প (fmt->json_format) {
	পক্ষ UL_JSON_সারি:
	পক্ষ UL_JSON_COMPACT:
		ভঙ্গ;
	পক্ষ UL_JSON_PRETTY:
	স্বভাব:
		fputs("\n", fmt->out);
		ul_jsonwrt_indent(fmt);
		ভঙ্গ;
	}

	fputc(endchr, fmt->out);

	/* Terminate the value on the top level. The writer may be re-initialized
	 * between the values (for example libsmartcols does it for every
	 * scols_table_print_range() call), so the terminator has to be printed
	 * by the value itself rather than by the next one (see print_name()). */
	যদি (fmt->indent == 0)
		fputc('\n', fmt->out);

	fmt->after_close = 1;
}


শূন্য ul_jsonwrt_flush(সংরচনা ul_jsonwrt *fmt)
{
	fflush(fmt->out);
}

শূন্য ul_jsonwrt_মান_raw(সংরচনা ul_jsonwrt *fmt,
			ধ্রুব বর্ণ *name, ধ্রুব বর্ণ *data)
{
	ul_jsonwrt_মান_open(fmt, name);
	যদি (data && *data)
		fputs(data, fmt->out);
	অন্যথা
		fputs("null", fmt->out);
	ul_jsonwrt_মান_close(fmt);
}

শূন্য ul_jsonwrt_মান_s(সংরচনা ul_jsonwrt *fmt,
			ধ্রুব বর্ণ *name, ধ্রুব বর্ণ *data)
{
	ul_jsonwrt_মান_open(fmt, name);
	যদি (data && *data)
		fputs_quoted_json(data, fmt->out);
	অন্যথা
		fputs("null", fmt->out);
	ul_jsonwrt_মান_close(fmt);
}

শূন্য ul_jsonwrt_মান_s_sized(সংরচনা ul_jsonwrt *fmt,
			      ধ্রুব বর্ণ *name, ধ্রুব বর্ণ *data, size_t size)
{
	ul_jsonwrt_মান_open(fmt, name);
	যদি (data && *data)
		fputs_quoted_case_json(data, fmt->out, 0, size);
	অন্যথা
		fputs("null", fmt->out);
	ul_jsonwrt_মান_close(fmt);
}

শূন্য ul_jsonwrt_মান_u64(সংরচনা ul_jsonwrt *fmt,
			ধ্রুব বর্ণ *name, uint64_t data)
{
	ul_jsonwrt_মান_open(fmt, name);
	fprintf(fmt->out, "%"PRIu64, data);
	ul_jsonwrt_মান_close(fmt);
}

শূন্য ul_jsonwrt_মান_double(সংরচনা ul_jsonwrt *fmt,
			ধ্রুব বর্ণ *name, long দ্বিগুণ data)
{
	ul_jsonwrt_মান_open(fmt, name);
	fprintf(fmt->out, "%Lg", data);
	ul_jsonwrt_মান_close(fmt);
}

শূন্য ul_jsonwrt_মান_boolean(সংরচনা ul_jsonwrt *fmt,
			ধ্রুব বর্ণ *name, পূর্ণাঙ্ক data)
{
	ul_jsonwrt_মান_open(fmt, name);
	fputs(data ? "true" : "false", fmt->out);
	ul_jsonwrt_মান_close(fmt);
}
