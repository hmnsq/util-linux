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
ساكن فراغ fputs_quoted_case_json(ثابت حرف *داده, FILE *بیرون, صحيح dir, size_t اندازه)
{
	ثابت حرف *p;

	fputc('"', بیرون);
	لكل (p = داده; p && *p && (!اندازه || p < داده + اندازه); p++) {

		ثابت بیعلامت صحيح c = (بیعلامت صحيح) *p;

		/* From http://www.json.org
		 *
		 * The double-quote and backslashes would break out a string or
		 * init an escape sequence if not escaped.
		 *
		 * Note that single-quotes and forward slashes, while they're
		 * in the JSON spec, don't break double-quoted strings.
		 */
		إذا (c == '"' || c == '\\') {
			fputc('\\', بیرون);
			fputc(c, بیرون);
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
				      dir == -1 ? c_tolower(c) : *p, بیرون);
			وإلا
				fputc(dir ==  1 ? به_بزرگ(c) :
				      dir == -1 ? tolower(c) : *p, بیرون);
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
				fputs("\\b", بیرون);
				اكسر;
			حالة '\t':
				fputs("\\t", بیرون);
				اكسر;
			حالة '\n':
				fputs("\\n", بیرون);
				اكسر;
			حالة '\f':
				fputs("\\f", بیرون);
				اكسر;
			حالة '\r':
				fputs("\\r", بیرون);
				اكسر;
			افتراضي:
				/* Other assorted control characters */
				fprintf(بیرون, "\\u00%02x", c);
				اكسر;
		}
	}
	fputc('"', بیرون);
}

#تعریف_کن fputs_quoted_json(_d, _o)       fputs_quoted_case_json(_d, _o, 0, 0)
#تعریف_کن fputs_quoted_json_فوقانی(_d, _o) fputs_quoted_case_json(_d, _o, 1, 0)
#تعریف_کن fputs_quoted_json_پایینی(_d, _o) fputs_quoted_case_json(_d, _o, -1, 0)

فراغ ul_jsonwrt_init(بنية ul_jsonwrt *fmt, FILE *بیرون, صحيح indent, تعداد ul_json_format json_format)
{
	fmt->بیرون = بیرون;
	fmt->indent = indent;
	fmt->json_format = json_format;
	fmt->after_close = 0;
}

صحيح ul_jsonwrt_is_ready(بنية ul_jsonwrt *fmt)
{
	أرجع fmt->بیرون == عدم ? 0 : 1;
}

فراغ ul_jsonwrt_indent(بنية ul_jsonwrt *fmt)
{
	صحيح i;

	لكل (i = 0; i < fmt->indent; i++)
		fputs("   ", fmt->بیرون);
}

ساكن فراغ print_name(بنية ul_jsonwrt *fmt, ثابت حرف *name)
{
	/* Values on the top level are not members of an array or object, so
	 * they are never comma separated; ul_jsonwrt_close() has already
	 * terminated the previous one by a newline. */
	إذا (fmt->after_close && (fmt->indent > 0 || name)) {
		إذا (fmt->json_format == UL_JSON_سطر
		    || fmt->json_format == UL_JSON_COMPACT)
			fputs(",", fmt->بیرون);
		وإلا
			fputs(name ? ",\n" : ",", fmt->بیرون);
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
		fputs_quoted_json_پایینی(name, fmt->بیرون);
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
	حالة UL_JSON_مقدار:
		إذا (fmt->json_format == UL_JSON_سطر)
			s = name ? ":" : "";
		وإلا إذا (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":" : "";
		وإلا
			s = name ? ": " : " ";
		اكسر;
	}
	fputs(s, fmt->بیرون);
	fmt->after_close = 0;
}

فراغ ul_jsonwrt_خالی(بنية ul_jsonwrt *fmt, ثابت حرف *name, صحيح type)
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
	حالة UL_JSON_مقدار:
		إذا (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": null" : "null";
		وإلا
			s = name ? ":null" : "null";
		اكسر;
	}
	fputs(s, fmt->بیرون);
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
	حالة UL_JSON_مقدار:
		fmt->after_close = 1;
		أرجع;
	افتراضي:
		fmt->after_close = 1;
		أرجع;
	}

	/* In COMPACT, when closing a direct child of root (indent==1),
	 * insert a newline so the closing bracket starts on its own line. */
	إذا (fmt->json_format == UL_JSON_COMPACT && fmt->indent == 1)
		fputc('\n', fmt->بیرون);

	بدل (fmt->json_format) {
	حالة UL_JSON_سطر:
	حالة UL_JSON_COMPACT:
		اكسر;
	حالة UL_JSON_PRETTY:
	افتراضي:
		fputs("\n", fmt->بیرون);
		ul_jsonwrt_indent(fmt);
		اكسر;
	}

	fputc(endchr, fmt->بیرون);

	/* Terminate the value on the top level. The writer may be re-initialized
	 * between the values (for example libsmartcols does it for every
	 * scols_table_print_range() call), so the terminator has to be printed
	 * by the value itself rather than by the next one (see print_name()). */
	إذا (fmt->indent == 0)
		fputc('\n', fmt->بیرون);

	fmt->after_close = 1;
}


فراغ ul_jsonwrt_flush(بنية ul_jsonwrt *fmt)
{
	fflush(fmt->بیرون);
}

فراغ ul_jsonwrt_مقدار_raw(بنية ul_jsonwrt *fmt,
			ثابت حرف *name, ثابت حرف *داده)
{
	ul_jsonwrt_مقدار_open(fmt, name);
	إذا (داده && *داده)
		fputs(داده, fmt->بیرون);
	وإلا
		fputs("null", fmt->بیرون);
	ul_jsonwrt_مقدار_close(fmt);
}

فراغ ul_jsonwrt_مقدار_s(بنية ul_jsonwrt *fmt,
			ثابت حرف *name, ثابت حرف *داده)
{
	ul_jsonwrt_مقدار_open(fmt, name);
	إذا (داده && *داده)
		fputs_quoted_json(داده, fmt->بیرون);
	وإلا
		fputs("null", fmt->بیرون);
	ul_jsonwrt_مقدار_close(fmt);
}

فراغ ul_jsonwrt_مقدار_s_sized(بنية ul_jsonwrt *fmt,
			      ثابت حرف *name, ثابت حرف *داده, size_t اندازه)
{
	ul_jsonwrt_مقدار_open(fmt, name);
	إذا (داده && *داده)
		fputs_quoted_case_json(داده, fmt->بیرون, 0, اندازه);
	وإلا
		fputs("null", fmt->بیرون);
	ul_jsonwrt_مقدار_close(fmt);
}

فراغ ul_jsonwrt_مقدار_u64(بنية ul_jsonwrt *fmt,
			ثابت حرف *name, uint64_t داده)
{
	ul_jsonwrt_مقدار_open(fmt, name);
	fprintf(fmt->بیرون, "%"PRIu64, داده);
	ul_jsonwrt_مقدار_close(fmt);
}

فراغ ul_jsonwrt_مقدار_double(بنية ul_jsonwrt *fmt,
			ثابت حرف *name, دراز مزدوج داده)
{
	ul_jsonwrt_مقدار_open(fmt, name);
	fprintf(fmt->بیرون, "%Lg", داده);
	ul_jsonwrt_مقدار_close(fmt);
}

فراغ ul_jsonwrt_مقدار_boolean(بنية ul_jsonwrt *fmt,
			ثابت حرف *name, صحيح داده)
{
	ul_jsonwrt_مقدار_open(fmt, name);
	fputs(داده ? "true" : "false", fmt->بیرون);
	ul_jsonwrt_مقدار_close(fmt);
}
