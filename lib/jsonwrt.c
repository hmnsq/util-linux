/*
 * JSON output formatting functions.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ضمن <stdio.h>
#ضمن <inttypes.h>
#ضمن <ctype.h>
#ضمن <cctype.h>

#ضمن "c.h"
#ضمن "jsonwrt.h"

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
ساكن فراغ fputs_quoted_case_json(ثابت حرف *data, FILE *out, صحيح dir, size_t size)
{
	ثابت حرف *p;

	fputc('"', out);
	لكل (p = data; p && *p && (!size || p < data + size); p++) {

		ثابت unsigned صحيح c = (unsigned صحيح) *p;

		/* From http://www.json.org
		 *
		 * The double-quote and backslashes would break out a string or
		 * init an escape sequence if not escaped.
		 *
		 * Note that single-quotes and forward slashes, while they're
		 * in the JSON spec, don't break double-quoted strings.
		 */
		إذا (c == '"' || c == '\\') {
			fputc('\\', out);
			fputc(c, out);
			استمر;
		}

		/* All non-control characters OK; do the case swap as required. */
		إذا (c >= 0x20) {
			/*
			 * Don't use locale sensitive ctype.h functions for regular
			 * ASCII chars, because for example with Turkish locale
			 * (aka LANG=tr_TR.UTF-8) toupper('I') returns 'I'.
			 */
			إذا (c <= 127)
				fputc(dir ==  1 ? c_toupper(c) :
				      dir == -1 ? c_tolower(c) : *p, out);
			وإلا
				fputc(dir ==  1 ? toupper(c) :
				      dir == -1 ? tolower(c) : *p, out);
			استمر;
		}

		/* In addition, all chars under ' ' break Node's/V8/Chrome's, and
		 * Firefox's JSON.parse function
		 */
		بدل (c) {
			/* Handle short-hand cases to reduce output size.  C
			 * has most of the same stuff here, so if there's an
			 * "Escape for C" function somewhere in the STL, we
			 * should probably be using it.
			 */
			حالة '\b':
				fputs("\\b", out);
				اكسر;
			حالة '\t':
				fputs("\\t", out);
				اكسر;
			حالة '\n':
				fputs("\\n", out);
				اكسر;
			حالة '\f':
				fputs("\\f", out);
				اكسر;
			حالة '\r':
				fputs("\\r", out);
				اكسر;
			افتراضي:
				/* Other assorted control characters */
				fprintf(out, "\\u00%02x", c);
				اكسر;
		}
	}
	fputc('"', out);
}

#define fputs_quoted_json(_d, _o)       fputs_quoted_case_json(_d, _o, 0, 0)
#define fputs_quoted_json_upper(_d, _o) fputs_quoted_case_json(_d, _o, 1, 0)
#define fputs_quoted_json_lower(_d, _o) fputs_quoted_case_json(_d, _o, -1, 0)

فراغ ul_jsonwrt_init(بنية ul_jsonwrt *fmt, FILE *out, صحيح indent, تعداد ul_json_format json_format)
{
	fmt->out = out;
	fmt->indent = indent;
	fmt->json_format = json_format;
	fmt->after_close = 0;
}

صحيح ul_jsonwrt_is_ready(بنية ul_jsonwrt *fmt)
{
	إرجاع fmt->out == عدم ? 0 : 1;
}

فراغ ul_jsonwrt_indent(بنية ul_jsonwrt *fmt)
{
	صحيح i;

	لكل (i = 0; i < fmt->indent; i++)
		fputs("   ", fmt->out);
}

ساكن فراغ print_name(بنية ul_jsonwrt *fmt, ثابت حرف *name)
{
	/* Values on the top level are not members of an array or object, so
	 * they are never comma separated; ul_jsonwrt_close() has already
	 * terminated the previous one by a newline. */
	إذا (fmt->after_close && (fmt->indent > 0 || name)) {
		إذا (fmt->json_format == UL_JSON_سطر
		    || fmt->json_format == UL_JSON_COMPACT)
			fputs(",", fmt->out);
		وإلا
			fputs(name ? ",\n" : ",", fmt->out);
	}

	بدل (fmt->json_format) {
	حالة UL_JSON_سطر:
	حالة UL_JSON_COMPACT:
		اكسر;
	حالة UL_JSON_PRETTY:
	افتراضي:
		إذا (name || !fmt->after_close)
			ul_jsonwrt_indent(fmt);
		اكسر;
	}
	إذا (name)
		fputs_quoted_json_lower(name, fmt->out);
}

فراغ ul_jsonwrt_open(بنية ul_jsonwrt *fmt, ثابت حرف *name, صحيح type)
{
	ثابت حرف *s = "";

	print_name(fmt, name);

	بدل (type) {
	حالة UL_JSON_OBJECT:
		إذا (fmt->json_format == UL_JSON_سطر)
			s = "{";
		وإلا إذا (fmt->json_format == UL_JSON_COMPACT) {
			/* At nesting depth 2, put the object opens on a new line
			 * for readability; otherwise keep fully compact (no newline). */
			إذا (fmt->indent == 2)
				s = name ? ":{" : "\n{";
			وإلا
				s = "{";
		} وإلا
			s = name ? ": {\n" : "{\n";
		fmt->indent++;
		اكسر;
	حالة UL_JSON_ARRAY:
		إذا (fmt->json_format == UL_JSON_سطر)
			s = "[";
		وإلا إذا (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":[" : "[";
		وإلا
			s = name ? ": [\n" : "[\n";
		fmt->indent++;
		اكسر;
	حالة UL_JSON_قيمة:
		إذا (fmt->json_format == UL_JSON_سطر)
			s = name ? ":" : "";
		وإلا إذا (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":" : "";
		وإلا
			s = name ? ": " : " ";
		اكسر;
	}
	fputs(s, fmt->out);
	fmt->after_close = 0;
}

فراغ ul_jsonwrt_empty(بنية ul_jsonwrt *fmt, ثابت حرف *name, صحيح type)
{
	ثابت حرف *s = "";

	print_name(fmt, name);

	بدل (type) {
	حالة UL_JSON_OBJECT:
		إذا (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": {}" : "{}";
		وإلا
			s = name ? ":{}" : "{}";
		اكسر;
	حالة UL_JSON_ARRAY:
		إذا (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": []" : "[]";
		وإلا
			s = name ? ":[]" : "[]";
		اكسر;
	حالة UL_JSON_قيمة:
		إذا (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": null" : "null";
		وإلا
			s = name ? ":null" : "null";
		اكسر;
	}
	fputs(s, fmt->out);
	fmt->after_close = 1;
}

فراغ ul_jsonwrt_close(بنية ul_jsonwrt *fmt, صحيح type)
{
	حرف endchr;

	أكد(fmt->indent > 0);

	بدل (type) {
	حالة UL_JSON_OBJECT:
		fmt->indent--;
		endchr = '}';
		اكسر;
	حالة UL_JSON_ARRAY:
		fmt->indent--;
		endchr = ']';
		اكسر;
	حالة UL_JSON_قيمة:
		fmt->after_close = 1;
		إرجاع;
	افتراضي:
		fmt->after_close = 1;
		إرجاع;
	}

	/* In COMPACT, when closing a direct child of root (indent==1),
	 * insert a newline so the closing bracket starts on its own line. */
	إذا (fmt->json_format == UL_JSON_COMPACT && fmt->indent == 1)
		fputc('\n', fmt->out);

	بدل (fmt->json_format) {
	حالة UL_JSON_سطر:
	حالة UL_JSON_COMPACT:
		اكسر;
	حالة UL_JSON_PRETTY:
	افتراضي:
		fputs("\n", fmt->out);
		ul_jsonwrt_indent(fmt);
		اكسر;
	}

	fputc(endchr, fmt->out);

	/* Terminate the value on the top level. The writer may be re-initialized
	 * between the values (for example libsmartcols does it for every
	 * scols_table_print_range() call), so the terminator has to be printed
	 * by the value itself rather than by the next one (see print_name()). */
	إذا (fmt->indent == 0)
		fputc('\n', fmt->out);

	fmt->after_close = 1;
}


فراغ ul_jsonwrt_flush(بنية ul_jsonwrt *fmt)
{
	fflush(fmt->out);
}

فراغ ul_jsonwrt_قيمة_raw(بنية ul_jsonwrt *fmt,
			ثابت حرف *name, ثابت حرف *data)
{
	ul_jsonwrt_قيمة_open(fmt, name);
	إذا (data && *data)
		fputs(data, fmt->out);
	وإلا
		fputs("null", fmt->out);
	ul_jsonwrt_قيمة_close(fmt);
}

فراغ ul_jsonwrt_قيمة_s(بنية ul_jsonwrt *fmt,
			ثابت حرف *name, ثابت حرف *data)
{
	ul_jsonwrt_قيمة_open(fmt, name);
	إذا (data && *data)
		fputs_quoted_json(data, fmt->out);
	وإلا
		fputs("null", fmt->out);
	ul_jsonwrt_قيمة_close(fmt);
}

فراغ ul_jsonwrt_قيمة_s_sized(بنية ul_jsonwrt *fmt,
			      ثابت حرف *name, ثابت حرف *data, size_t size)
{
	ul_jsonwrt_قيمة_open(fmt, name);
	إذا (data && *data)
		fputs_quoted_case_json(data, fmt->out, 0, size);
	وإلا
		fputs("null", fmt->out);
	ul_jsonwrt_قيمة_close(fmt);
}

فراغ ul_jsonwrt_قيمة_u64(بنية ul_jsonwrt *fmt,
			ثابت حرف *name, uint64_t data)
{
	ul_jsonwrt_قيمة_open(fmt, name);
	fprintf(fmt->out, "%"PRIu64, data);
	ul_jsonwrt_قيمة_close(fmt);
}

فراغ ul_jsonwrt_قيمة_double(بنية ul_jsonwrt *fmt,
			ثابت حرف *name, long مزدوج data)
{
	ul_jsonwrt_قيمة_open(fmt, name);
	fprintf(fmt->out, "%Lg", data);
	ul_jsonwrt_قيمة_close(fmt);
}

فراغ ul_jsonwrt_قيمة_boolean(بنية ul_jsonwrt *fmt,
			ثابت حرف *name, صحيح data)
{
	ul_jsonwrt_قيمة_open(fmt, name);
	fputs(data ? "true" : "false", fmt->out);
	ul_jsonwrt_قيمة_close(fmt);
}
