/*
 * JSON output formatting functions.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#شامل <stdio.h>
#شامل <inttypes.h>
#شامل <ctype.h>
#شامل <cctype.h>

#شامل "c.h"
#شامل "jsonwrt.h"

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
ایستا تهی fputs_quoted_case_json(ثابت نویسه *data, FILE *out, صحیح dir, size_t size)
{
	ثابت نویسه *p;

	fputc('"', out);
	برای (p = data; p && *p && (!size || p < data + size); p++) {

		ثابت unsigned صحیح c = (unsigned صحیح) *p;

		/* From http://www.json.org
		 *
		 * The double-quote and backslashes would break out a string or
		 * init an escape sequence if not escaped.
		 *
		 * Note that single-quotes and forward slashes, while they're
		 * in the JSON spec, don't break double-quoted strings.
		 */
		اگر (c == '"' || c == '\\') {
			fputc('\\', out);
			fputc(c, out);
			ادامه;
		}

		/* All non-control characters OK; do the case swap as required. */
		اگر (c >= 0x20) {
			/*
			 * Don't use locale sensitive ctype.h functions for regular
			 * ASCII chars, because for example with Turkish locale
			 * (aka LANG=tr_TR.UTF-8) toupper('I') returns 'I'.
			 */
			اگر (c <= 127)
				fputc(dir ==  1 ? c_toupper(c) :
				      dir == -1 ? c_tolower(c) : *p, out);
			وگرنه
				fputc(dir ==  1 ? toupper(c) :
				      dir == -1 ? tolower(c) : *p, out);
			ادامه;
		}

		/* In addition, all chars under ' ' break Node's/V8/Chrome's, and
		 * Firefox's JSON.parse function
		 */
		انتخاب (c) {
			/* Handle short-hand cases to reduce output size.  C
			 * has most of the same stuff here, so if there's an
			 * "Escape for C" function somewhere in the STL, we
			 * should probably be using it.
			 */
			حالت '\b':
				fputs("\\b", out);
				بشکن;
			حالت '\t':
				fputs("\\t", out);
				بشکن;
			حالت '\n':
				fputs("\\n", out);
				بشکن;
			حالت '\f':
				fputs("\\f", out);
				بشکن;
			حالت '\r':
				fputs("\\r", out);
				بشکن;
			پیشفرض:
				/* Other assorted control characters */
				fprintf(out, "\\u00%02x", c);
				بشکن;
		}
	}
	fputc('"', out);
}

#define fputs_quoted_json(_d, _o)       fputs_quoted_case_json(_d, _o, 0, 0)
#define fputs_quoted_json_upper(_d, _o) fputs_quoted_case_json(_d, _o, 1, 0)
#define fputs_quoted_json_lower(_d, _o) fputs_quoted_case_json(_d, _o, -1, 0)

تهی ul_jsonwrt_init(ساختار ul_jsonwrt *fmt, FILE *out, صحیح indent, شمارش ul_json_format json_format)
{
	fmt->out = out;
	fmt->indent = indent;
	fmt->json_format = json_format;
	fmt->after_close = 0;
}

صحیح ul_jsonwrt_is_ready(ساختار ul_jsonwrt *fmt)
{
	بازگشت fmt->out == پوچ ? 0 : 1;
}

تهی ul_jsonwrt_indent(ساختار ul_jsonwrt *fmt)
{
	صحیح i;

	برای (i = 0; i < fmt->indent; i++)
		fputs("   ", fmt->out);
}

ایستا تهی print_name(ساختار ul_jsonwrt *fmt, ثابت نویسه *name)
{
	/* Values on the top level are not members of an array or object, so
	 * they are never comma separated; ul_jsonwrt_close() has already
	 * terminated the previous one by a newline. */
	اگر (fmt->after_close && (fmt->indent > 0 || name)) {
		اگر (fmt->json_format == UL_JSON_سطر
		    || fmt->json_format == UL_JSON_COMPACT)
			fputs(",", fmt->out);
		وگرنه
			fputs(name ? ",\n" : ",", fmt->out);
	}

	انتخاب (fmt->json_format) {
	حالت UL_JSON_سطر:
	حالت UL_JSON_COMPACT:
		بشکن;
	حالت UL_JSON_PRETTY:
	پیشفرض:
		اگر (name || !fmt->after_close)
			ul_jsonwrt_indent(fmt);
		بشکن;
	}
	اگر (name)
		fputs_quoted_json_lower(name, fmt->out);
}

تهی ul_jsonwrt_open(ساختار ul_jsonwrt *fmt, ثابت نویسه *name, صحیح type)
{
	ثابت نویسه *s = "";

	print_name(fmt, name);

	انتخاب (type) {
	حالت UL_JSON_OBJECT:
		اگر (fmt->json_format == UL_JSON_سطر)
			s = "{";
		وگرنه اگر (fmt->json_format == UL_JSON_COMPACT) {
			/* At nesting depth 2, put the object opens on a new line
			 * for readability; otherwise keep fully compact (no newline). */
			اگر (fmt->indent == 2)
				s = name ? ":{" : "\n{";
			وگرنه
				s = "{";
		} وگرنه
			s = name ? ": {\n" : "{\n";
		fmt->indent++;
		بشکن;
	حالت UL_JSON_ARRAY:
		اگر (fmt->json_format == UL_JSON_سطر)
			s = "[";
		وگرنه اگر (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":[" : "[";
		وگرنه
			s = name ? ": [\n" : "[\n";
		fmt->indent++;
		بشکن;
	حالت UL_JSON_مقدار:
		اگر (fmt->json_format == UL_JSON_سطر)
			s = name ? ":" : "";
		وگرنه اگر (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":" : "";
		وگرنه
			s = name ? ": " : " ";
		بشکن;
	}
	fputs(s, fmt->out);
	fmt->after_close = 0;
}

تهی ul_jsonwrt_empty(ساختار ul_jsonwrt *fmt, ثابت نویسه *name, صحیح type)
{
	ثابت نویسه *s = "";

	print_name(fmt, name);

	انتخاب (type) {
	حالت UL_JSON_OBJECT:
		اگر (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": {}" : "{}";
		وگرنه
			s = name ? ":{}" : "{}";
		بشکن;
	حالت UL_JSON_ARRAY:
		اگر (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": []" : "[]";
		وگرنه
			s = name ? ":[]" : "[]";
		بشکن;
	حالت UL_JSON_مقدار:
		اگر (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": null" : "null";
		وگرنه
			s = name ? ":null" : "null";
		بشکن;
	}
	fputs(s, fmt->out);
	fmt->after_close = 1;
}

تهی ul_jsonwrt_close(ساختار ul_jsonwrt *fmt, صحیح type)
{
	نویسه endchr;

	أكد(fmt->indent > 0);

	انتخاب (type) {
	حالت UL_JSON_OBJECT:
		fmt->indent--;
		endchr = '}';
		بشکن;
	حالت UL_JSON_ARRAY:
		fmt->indent--;
		endchr = ']';
		بشکن;
	حالت UL_JSON_مقدار:
		fmt->after_close = 1;
		بازگشت;
	پیشفرض:
		fmt->after_close = 1;
		بازگشت;
	}

	/* In COMPACT, when closing a direct child of root (indent==1),
	 * insert a newline so the closing bracket starts on its own line. */
	اگر (fmt->json_format == UL_JSON_COMPACT && fmt->indent == 1)
		fputc('\n', fmt->out);

	انتخاب (fmt->json_format) {
	حالت UL_JSON_سطر:
	حالت UL_JSON_COMPACT:
		بشکن;
	حالت UL_JSON_PRETTY:
	پیشفرض:
		fputs("\n", fmt->out);
		ul_jsonwrt_indent(fmt);
		بشکن;
	}

	fputc(endchr, fmt->out);

	/* Terminate the value on the top level. The writer may be re-initialized
	 * between the values (for example libsmartcols does it for every
	 * scols_table_print_range() call), so the terminator has to be printed
	 * by the value itself rather than by the next one (see print_name()). */
	اگر (fmt->indent == 0)
		fputc('\n', fmt->out);

	fmt->after_close = 1;
}


تهی ul_jsonwrt_flush(ساختار ul_jsonwrt *fmt)
{
	fflush(fmt->out);
}

تهی ul_jsonwrt_مقدار_raw(ساختار ul_jsonwrt *fmt,
			ثابت نویسه *name, ثابت نویسه *data)
{
	ul_jsonwrt_مقدار_open(fmt, name);
	اگر (data && *data)
		fputs(data, fmt->out);
	وگرنه
		fputs("null", fmt->out);
	ul_jsonwrt_مقدار_close(fmt);
}

تهی ul_jsonwrt_مقدار_s(ساختار ul_jsonwrt *fmt,
			ثابت نویسه *name, ثابت نویسه *data)
{
	ul_jsonwrt_مقدار_open(fmt, name);
	اگر (data && *data)
		fputs_quoted_json(data, fmt->out);
	وگرنه
		fputs("null", fmt->out);
	ul_jsonwrt_مقدار_close(fmt);
}

تهی ul_jsonwrt_مقدار_s_sized(ساختار ul_jsonwrt *fmt,
			      ثابت نویسه *name, ثابت نویسه *data, size_t size)
{
	ul_jsonwrt_مقدار_open(fmt, name);
	اگر (data && *data)
		fputs_quoted_case_json(data, fmt->out, 0, size);
	وگرنه
		fputs("null", fmt->out);
	ul_jsonwrt_مقدار_close(fmt);
}

تهی ul_jsonwrt_مقدار_u64(ساختار ul_jsonwrt *fmt,
			ثابت نویسه *name, uint64_t data)
{
	ul_jsonwrt_مقدار_open(fmt, name);
	fprintf(fmt->out, "%"PRIu64, data);
	ul_jsonwrt_مقدار_close(fmt);
}

تهی ul_jsonwrt_مقدار_double(ساختار ul_jsonwrt *fmt,
			ثابت نویسه *name, long دوگانه data)
{
	ul_jsonwrt_مقدار_open(fmt, name);
	fprintf(fmt->out, "%Lg", data);
	ul_jsonwrt_مقدار_close(fmt);
}

تهی ul_jsonwrt_مقدار_boolean(ساختار ul_jsonwrt *fmt,
			ثابت نویسه *name, صحیح data)
{
	ul_jsonwrt_مقدار_open(fmt, name);
	fputs(data ? "true" : "false", fmt->out);
	ul_jsonwrt_مقدار_close(fmt);
}
