/*
 * JSON output formatting functions.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <inttypes.h>
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <cctype.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "jsonwrt.h"

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
நிலையான வெறுமை fputs_quoted_case_json(மாறிலி எழுத்து *data, FILE *out, முழு dir, size_t size)
{
	மாறிலி எழுத்து *p;

	fputc('"', out);
	க்கு (p = data; p && *p && (!size || p < data + size); p++) {

		மாறிலி unsigned முழு c = (unsigned முழு) *p;

		/* From http://www.json.org
		 *
		 * The double-quote and backslashes would break out a string or
		 * init an escape sequence if not escaped.
		 *
		 * Note that single-quotes and forward slashes, while they're
		 * in the JSON spec, don't break double-quoted strings.
		 */
		எனில் (c == '"' || c == '\\') {
			fputc('\\', out);
			fputc(c, out);
			தொடர்;
		}

		/* All non-control characters OK; do the case swap as required. */
		எனில் (c >= 0x20) {
			/*
			 * Don't use locale sensitive ctype.h functions for regular
			 * ASCII chars, because for example with Turkish locale
			 * (aka LANG=tr_TR.UTF-8) toupper('I') returns 'I'.
			 */
			எனில் (c <= 127)
				fputc(dir ==  1 ? c_toupper(c) :
				      dir == -1 ? c_tolower(c) : *p, out);
			இல்லையெனில்
				fputc(dir ==  1 ? toupper(c) :
				      dir == -1 ? tolower(c) : *p, out);
			தொடர்;
		}

		/* In addition, all chars under ' ' break Node's/V8/Chrome's, and
		 * Firefox's JSON.parse function
		 */
		தேர்வு (c) {
			/* Handle short-hand cases to reduce output size.  C
			 * has most of the same stuff here, so if there's an
			 * "Escape for C" function somewhere in the STL, we
			 * should probably be using it.
			 */
			நிலை '\b':
				fputs("\\b", out);
				நிறுத்து;
			நிலை '\t':
				fputs("\\t", out);
				நிறுத்து;
			நிலை '\n':
				fputs("\\n", out);
				நிறுத்து;
			நிலை '\f':
				fputs("\\f", out);
				நிறுத்து;
			நிலை '\r':
				fputs("\\r", out);
				நிறுத்து;
			இயல்பு:
				/* Other assorted control characters */
				fprintf(out, "\\u00%02x", c);
				நிறுத்து;
		}
	}
	fputc('"', out);
}

#define fputs_quoted_json(_d, _o)       fputs_quoted_case_json(_d, _o, 0, 0)
#define fputs_quoted_json_upper(_d, _o) fputs_quoted_case_json(_d, _o, 1, 0)
#define fputs_quoted_json_lower(_d, _o) fputs_quoted_case_json(_d, _o, -1, 0)

வெறுமை ul_jsonwrt_init(கட்டமைப்பு ul_jsonwrt *fmt, FILE *out, முழு indent, எண்ணிக்கை ul_json_format json_format)
{
	fmt->out = out;
	fmt->indent = indent;
	fmt->json_format = json_format;
	fmt->after_close = 0;
}

முழு ul_jsonwrt_is_ready(கட்டமைப்பு ul_jsonwrt *fmt)
{
	திரும்பு fmt->out == வெற்று ? 0 : 1;
}

வெறுமை ul_jsonwrt_indent(கட்டமைப்பு ul_jsonwrt *fmt)
{
	முழு i;

	க்கு (i = 0; i < fmt->indent; i++)
		fputs("   ", fmt->out);
}

நிலையான வெறுமை print_name(கட்டமைப்பு ul_jsonwrt *fmt, மாறிலி எழுத்து *name)
{
	/* Values on the top level are not members of an array or object, so
	 * they are never comma separated; ul_jsonwrt_close() has already
	 * terminated the previous one by a newline. */
	எனில் (fmt->after_close && (fmt->indent > 0 || name)) {
		எனில் (fmt->json_format == UL_JSON_வரி
		    || fmt->json_format == UL_JSON_COMPACT)
			fputs(",", fmt->out);
		இல்லையெனில்
			fputs(name ? ",\n" : ",", fmt->out);
	}

	தேர்வு (fmt->json_format) {
	நிலை UL_JSON_வரி:
	நிலை UL_JSON_COMPACT:
		நிறுத்து;
	நிலை UL_JSON_PRETTY:
	இயல்பு:
		எனில் (name || !fmt->after_close)
			ul_jsonwrt_indent(fmt);
		நிறுத்து;
	}
	எனில் (name)
		fputs_quoted_json_lower(name, fmt->out);
}

வெறுமை ul_jsonwrt_open(கட்டமைப்பு ul_jsonwrt *fmt, மாறிலி எழுத்து *name, முழு type)
{
	மாறிலி எழுத்து *s = "";

	print_name(fmt, name);

	தேர்வு (type) {
	நிலை UL_JSON_OBJECT:
		எனில் (fmt->json_format == UL_JSON_வரி)
			s = "{";
		இல்லையெனில் எனில் (fmt->json_format == UL_JSON_COMPACT) {
			/* At nesting depth 2, put the object opens on a new line
			 * for readability; otherwise keep fully compact (no newline). */
			எனில் (fmt->indent == 2)
				s = name ? ":{" : "\n{";
			இல்லையெனில்
				s = "{";
		} இல்லையெனில்
			s = name ? ": {\n" : "{\n";
		fmt->indent++;
		நிறுத்து;
	நிலை UL_JSON_ARRAY:
		எனில் (fmt->json_format == UL_JSON_வரி)
			s = "[";
		இல்லையெனில் எனில் (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":[" : "[";
		இல்லையெனில்
			s = name ? ": [\n" : "[\n";
		fmt->indent++;
		நிறுத்து;
	நிலை UL_JSON_மதிப்பு:
		எனில் (fmt->json_format == UL_JSON_வரி)
			s = name ? ":" : "";
		இல்லையெனில் எனில் (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":" : "";
		இல்லையெனில்
			s = name ? ": " : " ";
		நிறுத்து;
	}
	fputs(s, fmt->out);
	fmt->after_close = 0;
}

வெறுமை ul_jsonwrt_empty(கட்டமைப்பு ul_jsonwrt *fmt, மாறிலி எழுத்து *name, முழு type)
{
	மாறிலி எழுத்து *s = "";

	print_name(fmt, name);

	தேர்வு (type) {
	நிலை UL_JSON_OBJECT:
		எனில் (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": {}" : "{}";
		இல்லையெனில்
			s = name ? ":{}" : "{}";
		நிறுத்து;
	நிலை UL_JSON_ARRAY:
		எனில் (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": []" : "[]";
		இல்லையெனில்
			s = name ? ":[]" : "[]";
		நிறுத்து;
	நிலை UL_JSON_மதிப்பு:
		எனில் (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": null" : "null";
		இல்லையெனில்
			s = name ? ":null" : "null";
		நிறுத்து;
	}
	fputs(s, fmt->out);
	fmt->after_close = 1;
}

வெறுமை ul_jsonwrt_close(கட்டமைப்பு ul_jsonwrt *fmt, முழு type)
{
	எழுத்து endchr;

	ப்ரதிஜ்ஞா(fmt->indent > 0);

	தேர்வு (type) {
	நிலை UL_JSON_OBJECT:
		fmt->indent--;
		endchr = '}';
		நிறுத்து;
	நிலை UL_JSON_ARRAY:
		fmt->indent--;
		endchr = ']';
		நிறுத்து;
	நிலை UL_JSON_மதிப்பு:
		fmt->after_close = 1;
		திரும்பு;
	இயல்பு:
		fmt->after_close = 1;
		திரும்பு;
	}

	/* In COMPACT, when closing a direct child of root (indent==1),
	 * insert a newline so the closing bracket starts on its own line. */
	எனில் (fmt->json_format == UL_JSON_COMPACT && fmt->indent == 1)
		fputc('\n', fmt->out);

	தேர்வு (fmt->json_format) {
	நிலை UL_JSON_வரி:
	நிலை UL_JSON_COMPACT:
		நிறுத்து;
	நிலை UL_JSON_PRETTY:
	இயல்பு:
		fputs("\n", fmt->out);
		ul_jsonwrt_indent(fmt);
		நிறுத்து;
	}

	fputc(endchr, fmt->out);

	/* Terminate the value on the top level. The writer may be re-initialized
	 * between the values (for example libsmartcols does it for every
	 * scols_table_print_range() call), so the terminator has to be printed
	 * by the value itself rather than by the next one (see print_name()). */
	எனில் (fmt->indent == 0)
		fputc('\n', fmt->out);

	fmt->after_close = 1;
}


வெறுமை ul_jsonwrt_flush(கட்டமைப்பு ul_jsonwrt *fmt)
{
	fflush(fmt->out);
}

வெறுமை ul_jsonwrt_மதிப்பு_raw(கட்டமைப்பு ul_jsonwrt *fmt,
			மாறிலி எழுத்து *name, மாறிலி எழுத்து *data)
{
	ul_jsonwrt_மதிப்பு_open(fmt, name);
	எனில் (data && *data)
		fputs(data, fmt->out);
	இல்லையெனில்
		fputs("null", fmt->out);
	ul_jsonwrt_மதிப்பு_close(fmt);
}

வெறுமை ul_jsonwrt_மதிப்பு_s(கட்டமைப்பு ul_jsonwrt *fmt,
			மாறிலி எழுத்து *name, மாறிலி எழுத்து *data)
{
	ul_jsonwrt_மதிப்பு_open(fmt, name);
	எனில் (data && *data)
		fputs_quoted_json(data, fmt->out);
	இல்லையெனில்
		fputs("null", fmt->out);
	ul_jsonwrt_மதிப்பு_close(fmt);
}

வெறுமை ul_jsonwrt_மதிப்பு_s_sized(கட்டமைப்பு ul_jsonwrt *fmt,
			      மாறிலி எழுத்து *name, மாறிலி எழுத்து *data, size_t size)
{
	ul_jsonwrt_மதிப்பு_open(fmt, name);
	எனில் (data && *data)
		fputs_quoted_case_json(data, fmt->out, 0, size);
	இல்லையெனில்
		fputs("null", fmt->out);
	ul_jsonwrt_மதிப்பு_close(fmt);
}

வெறுமை ul_jsonwrt_மதிப்பு_u64(கட்டமைப்பு ul_jsonwrt *fmt,
			மாறிலி எழுத்து *name, uint64_t data)
{
	ul_jsonwrt_மதிப்பு_open(fmt, name);
	fprintf(fmt->out, "%"PRIu64, data);
	ul_jsonwrt_மதிப்பு_close(fmt);
}

வெறுமை ul_jsonwrt_மதிப்பு_double(கட்டமைப்பு ul_jsonwrt *fmt,
			மாறிலி எழுத்து *name, long இரட்டை data)
{
	ul_jsonwrt_மதிப்பு_open(fmt, name);
	fprintf(fmt->out, "%Lg", data);
	ul_jsonwrt_மதிப்பு_close(fmt);
}

வெறுமை ul_jsonwrt_மதிப்பு_boolean(கட்டமைப்பு ul_jsonwrt *fmt,
			மாறிலி எழுத்து *name, முழு data)
{
	ul_jsonwrt_மதிப்பு_open(fmt, name);
	fputs(data ? "true" : "false", fmt->out);
	ul_jsonwrt_மதிப்பு_close(fmt);
}
