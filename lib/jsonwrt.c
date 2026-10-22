/*
 * JSON output formatting functions.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <inttypes.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <cctype.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "jsonwrt.h"

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
ସ୍ଥିର ଶୂନ୍ୟ fputs_quoted_case_json(ଅଚଳ ଅକ୍ଷର *data, FILE *out, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir, size_t size)
{
	ଅଚଳ ଅକ୍ଷର *p;

	fputc('"', out);
	ପାଇଁ (p = data; p && *p && (!size || p < data + size); p++) {

		ଅଚଳ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା c = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *p;

		/* From http://www.json.org
		 *
		 * The double-quote and backslashes would break out a string or
		 * init an escape sequence if not escaped.
		 *
		 * Note that single-quotes and forward slashes, while they're
		 * in the JSON spec, don't break double-quoted strings.
		 */
		ଯଦି (c == '"' || c == '\\') {
			fputc('\\', out);
			fputc(c, out);
			ଜାରି;
		}

		/* All non-control characters OK; do the case swap as required. */
		ଯଦି (c >= 0x20) {
			/*
			 * Don't use locale sensitive ctype.h functions for regular
			 * ASCII chars, because for example with Turkish locale
			 * (aka LANG=tr_TR.UTF-8) toupper('I') returns 'I'.
			 */
			ଯଦି (c <= 127)
				fputc(dir ==  1 ? c_toupper(c) :
				      dir == -1 ? c_tolower(c) : *p, out);
			ଅନ୍ୟଥା
				fputc(dir ==  1 ? toupper(c) :
				      dir == -1 ? tolower(c) : *p, out);
			ଜାରି;
		}

		/* In addition, all chars under ' ' break Node's/V8/Chrome's, and
		 * Firefox's JSON.parse function
		 */
		ବାଛ (c) {
			/* Handle short-hand cases to reduce output size.  C
			 * has most of the same stuff here, so if there's an
			 * "Escape for C" function somewhere in the STL, we
			 * should probably be using it.
			 */
			ସ୍ଥିତି '\b':
				fputs("\\b", out);
				ଭାଙ୍ଗ;
			ସ୍ଥିତି '\t':
				fputs("\\t", out);
				ଭାଙ୍ଗ;
			ସ୍ଥିତି '\n':
				fputs("\\n", out);
				ଭାଙ୍ଗ;
			ସ୍ଥିତି '\f':
				fputs("\\f", out);
				ଭାଙ୍ଗ;
			ସ୍ଥିତି '\r':
				fputs("\\r", out);
				ଭାଙ୍ଗ;
			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
				/* Other assorted control characters */
				fprintf(out, "\\u00%02x", c);
				ଭାଙ୍ଗ;
		}
	}
	fputc('"', out);
}

#define fputs_quoted_json(_d, _o)       fputs_quoted_case_json(_d, _o, 0, 0)
#define fputs_quoted_json_upper(_d, _o) fputs_quoted_case_json(_d, _o, 1, 0)
#define fputs_quoted_json_lower(_d, _o) fputs_quoted_case_json(_d, _o, -1, 0)

ଶୂନ୍ୟ ul_jsonwrt_init(ସଂରଚନା ul_jsonwrt *fmt, FILE *out, ପୂର୍ଣ୍ଣସଂଖ୍ୟା indent, ଗଣନା ul_json_format json_format)
{
	fmt->out = out;
	fmt->indent = indent;
	fmt->json_format = json_format;
	fmt->after_close = 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_jsonwrt_is_ready(ସଂରଚନା ul_jsonwrt *fmt)
{
	ଫେରନ୍ତୁ fmt->out == ଖାଲି ? 0 : 1;
}

ଶୂନ୍ୟ ul_jsonwrt_indent(ସଂରଚନା ul_jsonwrt *fmt)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = 0; i < fmt->indent; i++)
		fputs("   ", fmt->out);
}

ସ୍ଥିର ଶୂନ୍ୟ print_name(ସଂରଚନା ul_jsonwrt *fmt, ଅଚଳ ଅକ୍ଷର *name)
{
	/* Values on the top level are not members of an array or object, so
	 * they are never comma separated; ul_jsonwrt_close() has already
	 * terminated the previous one by a newline. */
	ଯଦି (fmt->after_close && (fmt->indent > 0 || name)) {
		ଯଦି (fmt->json_format == UL_JSON_ଧାଡ଼ି
		    || fmt->json_format == UL_JSON_COMPACT)
			fputs(",", fmt->out);
		ଅନ୍ୟଥା
			fputs(name ? ",\n" : ",", fmt->out);
	}

	ବାଛ (fmt->json_format) {
	ସ୍ଥିତି UL_JSON_ଧାଡ଼ି:
	ସ୍ଥିତି UL_JSON_COMPACT:
		ଭାଙ୍ଗ;
	ସ୍ଥିତି UL_JSON_PRETTY:
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଯଦି (name || !fmt->after_close)
			ul_jsonwrt_indent(fmt);
		ଭାଙ୍ଗ;
	}
	ଯଦି (name)
		fputs_quoted_json_lower(name, fmt->out);
}

ଶୂନ୍ୟ ul_jsonwrt_open(ସଂରଚନା ul_jsonwrt *fmt, ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା type)
{
	ଅଚଳ ଅକ୍ଷର *s = "";

	print_name(fmt, name);

	ବାଛ (type) {
	ସ୍ଥିତି UL_JSON_OBJECT:
		ଯଦି (fmt->json_format == UL_JSON_ଧାଡ଼ି)
			s = "{";
		ଅନ୍ୟଥା ଯଦି (fmt->json_format == UL_JSON_COMPACT) {
			/* At nesting depth 2, put the object opens on a new line
			 * for readability; otherwise keep fully compact (no newline). */
			ଯଦି (fmt->indent == 2)
				s = name ? ":{" : "\n{";
			ଅନ୍ୟଥା
				s = "{";
		} ଅନ୍ୟଥା
			s = name ? ": {\n" : "{\n";
		fmt->indent++;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି UL_JSON_ARRAY:
		ଯଦି (fmt->json_format == UL_JSON_ଧାଡ଼ି)
			s = "[";
		ଅନ୍ୟଥା ଯଦି (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":[" : "[";
		ଅନ୍ୟଥା
			s = name ? ": [\n" : "[\n";
		fmt->indent++;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି UL_JSON_ମୂଲ୍ୟ:
		ଯଦି (fmt->json_format == UL_JSON_ଧାଡ଼ି)
			s = name ? ":" : "";
		ଅନ୍ୟଥା ଯଦି (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":" : "";
		ଅନ୍ୟଥା
			s = name ? ": " : " ";
		ଭାଙ୍ଗ;
	}
	fputs(s, fmt->out);
	fmt->after_close = 0;
}

ଶୂନ୍ୟ ul_jsonwrt_empty(ସଂରଚନା ul_jsonwrt *fmt, ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା type)
{
	ଅଚଳ ଅକ୍ଷର *s = "";

	print_name(fmt, name);

	ବାଛ (type) {
	ସ୍ଥିତି UL_JSON_OBJECT:
		ଯଦି (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": {}" : "{}";
		ଅନ୍ୟଥା
			s = name ? ":{}" : "{}";
		ଭାଙ୍ଗ;
	ସ୍ଥିତି UL_JSON_ARRAY:
		ଯଦି (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": []" : "[]";
		ଅନ୍ୟଥା
			s = name ? ":[]" : "[]";
		ଭାଙ୍ଗ;
	ସ୍ଥିତି UL_JSON_ମୂଲ୍ୟ:
		ଯଦି (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": null" : "null";
		ଅନ୍ୟଥା
			s = name ? ":null" : "null";
		ଭାଙ୍ଗ;
	}
	fputs(s, fmt->out);
	fmt->after_close = 1;
}

ଶୂନ୍ୟ ul_jsonwrt_close(ସଂରଚନା ul_jsonwrt *fmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା type)
{
	ଅକ୍ଷର endchr;

	ପ୍ରତିଜ୍ଞା(fmt->indent > 0);

	ବାଛ (type) {
	ସ୍ଥିତି UL_JSON_OBJECT:
		fmt->indent--;
		endchr = '}';
		ଭାଙ୍ଗ;
	ସ୍ଥିତି UL_JSON_ARRAY:
		fmt->indent--;
		endchr = ']';
		ଭାଙ୍ଗ;
	ସ୍ଥିତି UL_JSON_ମୂଲ୍ୟ:
		fmt->after_close = 1;
		ଫେରନ୍ତୁ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		fmt->after_close = 1;
		ଫେରନ୍ତୁ;
	}

	/* In COMPACT, when closing a direct child of root (indent==1),
	 * insert a newline so the closing bracket starts on its own line. */
	ଯଦି (fmt->json_format == UL_JSON_COMPACT && fmt->indent == 1)
		fputc('\n', fmt->out);

	ବାଛ (fmt->json_format) {
	ସ୍ଥିତି UL_JSON_ଧାଡ଼ି:
	ସ୍ଥିତି UL_JSON_COMPACT:
		ଭାଙ୍ଗ;
	ସ୍ଥିତି UL_JSON_PRETTY:
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		fputs("\n", fmt->out);
		ul_jsonwrt_indent(fmt);
		ଭାଙ୍ଗ;
	}

	fputc(endchr, fmt->out);

	/* Terminate the value on the top level. The writer may be re-initialized
	 * between the values (for example libsmartcols does it for every
	 * scols_table_print_range() call), so the terminator has to be printed
	 * by the value itself rather than by the next one (see print_name()). */
	ଯଦି (fmt->indent == 0)
		fputc('\n', fmt->out);

	fmt->after_close = 1;
}


ଶୂନ୍ୟ ul_jsonwrt_flush(ସଂରଚନା ul_jsonwrt *fmt)
{
	fflush(fmt->out);
}

ଶୂନ୍ୟ ul_jsonwrt_ମୂଲ୍ୟ_raw(ସଂରଚନା ul_jsonwrt *fmt,
			ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ଅକ୍ଷର *data)
{
	ul_jsonwrt_ମୂଲ୍ୟ_open(fmt, name);
	ଯଦି (data && *data)
		fputs(data, fmt->out);
	ଅନ୍ୟଥା
		fputs("null", fmt->out);
	ul_jsonwrt_ମୂଲ୍ୟ_close(fmt);
}

ଶୂନ୍ୟ ul_jsonwrt_ମୂଲ୍ୟ_s(ସଂରଚନା ul_jsonwrt *fmt,
			ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ଅକ୍ଷର *data)
{
	ul_jsonwrt_ମୂଲ୍ୟ_open(fmt, name);
	ଯଦି (data && *data)
		fputs_quoted_json(data, fmt->out);
	ଅନ୍ୟଥା
		fputs("null", fmt->out);
	ul_jsonwrt_ମୂଲ୍ୟ_close(fmt);
}

ଶୂନ୍ୟ ul_jsonwrt_ମୂଲ୍ୟ_s_sized(ସଂରଚନା ul_jsonwrt *fmt,
			      ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ଅକ୍ଷର *data, size_t size)
{
	ul_jsonwrt_ମୂଲ୍ୟ_open(fmt, name);
	ଯଦି (data && *data)
		fputs_quoted_case_json(data, fmt->out, 0, size);
	ଅନ୍ୟଥା
		fputs("null", fmt->out);
	ul_jsonwrt_ମୂଲ୍ୟ_close(fmt);
}

ଶୂନ୍ୟ ul_jsonwrt_ମୂଲ୍ୟ_u64(ସଂରଚନା ul_jsonwrt *fmt,
			ଅଚଳ ଅକ୍ଷର *name, uint64_t data)
{
	ul_jsonwrt_ମୂଲ୍ୟ_open(fmt, name);
	fprintf(fmt->out, "%"PRIu64, data);
	ul_jsonwrt_ମୂଲ୍ୟ_close(fmt);
}

ଶୂନ୍ୟ ul_jsonwrt_ମୂଲ୍ୟ_double(ସଂରଚନା ul_jsonwrt *fmt,
			ଅଚଳ ଅକ୍ଷର *name, long ଦ୍ୱିଗୁଣ data)
{
	ul_jsonwrt_ମୂଲ୍ୟ_open(fmt, name);
	fprintf(fmt->out, "%Lg", data);
	ul_jsonwrt_ମୂଲ୍ୟ_close(fmt);
}

ଶୂନ୍ୟ ul_jsonwrt_ମୂଲ୍ୟ_boolean(ସଂରଚନା ul_jsonwrt *fmt,
			ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା data)
{
	ul_jsonwrt_ମୂଲ୍ୟ_open(fmt, name);
	fputs(data ? "true" : "false", fmt->out);
	ul_jsonwrt_ମୂଲ୍ୟ_close(fmt);
}
