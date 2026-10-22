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
স্থির শূন্য fputs_quoted_case_json(ধ্রুবক অক্ষর *তথ্য, FILE *out, পূর্ণসংখ্যা dir, size_t আকার_2)
{
	ধ্রুবক অক্ষর *p;

	fputc('"', out);
	জন্য (p = তথ্য; p && *p && (!আকার_2 || p < তথ্য + আকার_2); p++) {

		ধ্রুবক unsigned পূর্ণসংখ্যা c = (unsigned পূর্ণসংখ্যা) *p;

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
			চালু;
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
			চালু;
		}

		/* In addition, all chars under ' ' break Node's/V8/Chrome's, and
		 * Firefox's JSON.parse function
		 */
		নির্বাচন (c) {
			/* Handle short-hand cases to reduce output size.  C
			 * has most of the same stuff here, so if there's an
			 * "Escape for C" function somewhere in the STL, we
			 * should probably be using it.
			 */
			ক্ষেত্র '\b':
				fputs("\\b", out);
				বিরতি;
			ক্ষেত্র '\t':
				fputs("\\t", out);
				বিরতি;
			ক্ষেত্র '\n':
				fputs("\\n", out);
				বিরতি;
			ক্ষেত্র '\f':
				fputs("\\f", out);
				বিরতি;
			ক্ষেত্র '\r':
				fputs("\\r", out);
				বিরতি;
			পূর্বনির্ধারিত:
				/* Other assorted control characters */
				fprintf(out, "\\u00%02x", c);
				বিরতি;
		}
	}
	fputc('"', out);
}

#define fputs_quoted_json(_d, _o)       fputs_quoted_case_json(_d, _o, 0, 0)
#define fputs_quoted_json_upper(_d, _o) fputs_quoted_case_json(_d, _o, 1, 0)
#define fputs_quoted_json_lower(_d, _o) fputs_quoted_case_json(_d, _o, -1, 0)

শূন্য ul_jsonwrt_init(কাঠামো ul_jsonwrt *fmt, FILE *out, পূর্ণসংখ্যা indent, গণনা ul_json_format json_format)
{
	fmt->out = out;
	fmt->indent = indent;
	fmt->json_format = json_format;
	fmt->after_close = 0;
}

পূর্ণসংখ্যা ul_jsonwrt_is_ready(কাঠামো ul_jsonwrt *fmt)
{
	ফেরত fmt->out == ফাঁকা ? 0 : 1;
}

শূন্য ul_jsonwrt_indent(কাঠামো ul_jsonwrt *fmt)
{
	পূর্ণসংখ্যা i;

	জন্য (i = 0; i < fmt->indent; i++)
		fputs("   ", fmt->out);
}

স্থির শূন্য print_name(কাঠামো ul_jsonwrt *fmt, ধ্রুবক অক্ষর *name)
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

	নির্বাচন (fmt->json_format) {
	ক্ষেত্র UL_JSON_সারি:
	ক্ষেত্র UL_JSON_COMPACT:
		বিরতি;
	ক্ষেত্র UL_JSON_PRETTY:
	পূর্বনির্ধারিত:
		যদি (name || !fmt->after_close)
			ul_jsonwrt_indent(fmt);
		বিরতি;
	}
	যদি (name)
		fputs_quoted_json_lower(name, fmt->out);
}

শূন্য ul_jsonwrt_open(কাঠামো ul_jsonwrt *fmt, ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা type)
{
	ধ্রুবক অক্ষর *s = "";

	print_name(fmt, name);

	নির্বাচন (type) {
	ক্ষেত্র UL_JSON_OBJECT:
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
		বিরতি;
	ক্ষেত্র UL_JSON_ARRAY:
		যদি (fmt->json_format == UL_JSON_সারি)
			s = "[";
		অন্যথা যদি (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":[" : "[";
		অন্যথা
			s = name ? ": [\n" : "[\n";
		fmt->indent++;
		বিরতি;
	ক্ষেত্র UL_JSON_মান:
		যদি (fmt->json_format == UL_JSON_সারি)
			s = name ? ":" : "";
		অন্যথা যদি (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":" : "";
		অন্যথা
			s = name ? ": " : " ";
		বিরতি;
	}
	fputs(s, fmt->out);
	fmt->after_close = 0;
}

শূন্য ul_jsonwrt_empty(কাঠামো ul_jsonwrt *fmt, ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা type)
{
	ধ্রুবক অক্ষর *s = "";

	print_name(fmt, name);

	নির্বাচন (type) {
	ক্ষেত্র UL_JSON_OBJECT:
		যদি (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": {}" : "{}";
		অন্যথা
			s = name ? ":{}" : "{}";
		বিরতি;
	ক্ষেত্র UL_JSON_ARRAY:
		যদি (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": []" : "[]";
		অন্যথা
			s = name ? ":[]" : "[]";
		বিরতি;
	ক্ষেত্র UL_JSON_মান:
		যদি (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": null" : "null";
		অন্যথা
			s = name ? ":null" : "null";
		বিরতি;
	}
	fputs(s, fmt->out);
	fmt->after_close = 1;
}

শূন্য ul_jsonwrt_close(কাঠামো ul_jsonwrt *fmt, পূর্ণসংখ্যা type)
{
	অক্ষর endchr;

	প্রতিজ্ঞা(fmt->indent > 0);

	নির্বাচন (type) {
	ক্ষেত্র UL_JSON_OBJECT:
		fmt->indent--;
		endchr = '}';
		বিরতি;
	ক্ষেত্র UL_JSON_ARRAY:
		fmt->indent--;
		endchr = ']';
		বিরতি;
	ক্ষেত্র UL_JSON_মান:
		fmt->after_close = 1;
		ফেরত;
	পূর্বনির্ধারিত:
		fmt->after_close = 1;
		ফেরত;
	}

	/* In COMPACT, when closing a direct child of root (indent==1),
	 * insert a newline so the closing bracket starts on its own line. */
	যদি (fmt->json_format == UL_JSON_COMPACT && fmt->indent == 1)
		fputc('\n', fmt->out);

	নির্বাচন (fmt->json_format) {
	ক্ষেত্র UL_JSON_সারি:
	ক্ষেত্র UL_JSON_COMPACT:
		বিরতি;
	ক্ষেত্র UL_JSON_PRETTY:
	পূর্বনির্ধারিত:
		fputs("\n", fmt->out);
		ul_jsonwrt_indent(fmt);
		বিরতি;
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


শূন্য ul_jsonwrt_flush(কাঠামো ul_jsonwrt *fmt)
{
	fflush(fmt->out);
}

শূন্য ul_jsonwrt_মান_raw(কাঠামো ul_jsonwrt *fmt,
			ধ্রুবক অক্ষর *name, ধ্রুবক অক্ষর *তথ্য)
{
	ul_jsonwrt_মান_open(fmt, name);
	যদি (তথ্য && *তথ্য)
		fputs(তথ্য, fmt->out);
	অন্যথা
		fputs("null", fmt->out);
	ul_jsonwrt_মান_close(fmt);
}

শূন্য ul_jsonwrt_মান_s(কাঠামো ul_jsonwrt *fmt,
			ধ্রুবক অক্ষর *name, ধ্রুবক অক্ষর *তথ্য)
{
	ul_jsonwrt_মান_open(fmt, name);
	যদি (তথ্য && *তথ্য)
		fputs_quoted_json(তথ্য, fmt->out);
	অন্যথা
		fputs("null", fmt->out);
	ul_jsonwrt_মান_close(fmt);
}

শূন্য ul_jsonwrt_মান_s_sized(কাঠামো ul_jsonwrt *fmt,
			      ধ্রুবক অক্ষর *name, ধ্রুবক অক্ষর *তথ্য, size_t আকার_2)
{
	ul_jsonwrt_মান_open(fmt, name);
	যদি (তথ্য && *তথ্য)
		fputs_quoted_case_json(তথ্য, fmt->out, 0, আকার_2);
	অন্যথা
		fputs("null", fmt->out);
	ul_jsonwrt_মান_close(fmt);
}

শূন্য ul_jsonwrt_মান_u64(কাঠামো ul_jsonwrt *fmt,
			ধ্রুবক অক্ষর *name, uint64_t তথ্য)
{
	ul_jsonwrt_মান_open(fmt, name);
	fprintf(fmt->out, "%"PRIu64, তথ্য);
	ul_jsonwrt_মান_close(fmt);
}

শূন্য ul_jsonwrt_মান_double(কাঠামো ul_jsonwrt *fmt,
			ধ্রুবক অক্ষর *name, long দ্বিগুণ তথ্য)
{
	ul_jsonwrt_মান_open(fmt, name);
	fprintf(fmt->out, "%Lg", তথ্য);
	ul_jsonwrt_মান_close(fmt);
}

শূন্য ul_jsonwrt_মান_boolean(কাঠামো ul_jsonwrt *fmt,
			ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা তথ্য)
{
	ul_jsonwrt_মান_open(fmt, name);
	fputs(তথ্য ? "true" : "false", fmt->out);
	ul_jsonwrt_মান_close(fmt);
}
