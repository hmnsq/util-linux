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
ساكن فراغ fputs_quoted_case_json(ثابت حرف *انکڑے, FILE *باہرلا, صحيح dir, size_t ناپ)
{
	ثابت حرف *p;

	fputc('"', باہرلا);
	لكل (p = انکڑے; p && *p && (!ناپ || p < انکڑے + ناپ); p++) {

		ثابت بنانشانی صحيح c = (بنانشانی صحيح) *p;

		/* From http://www.json.org
		 *
		 * The double-quote and backslashes would break out a string or
		 * init an escape sequence if not escaped.
		 *
		 * Note that single-quotes and forward slashes, while they're
		 * in the JSON spec, don't break double-quoted strings.
		 */
		إذا (c == '"' || c == '\\') {
			fputc('\\', باہرلا);
			fputc(c, باہرلا);
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
				      dir == -1 ? c_tolower(c) : *p, باہرلا);
			وإلا
				fputc(dir ==  1 ? وڈا_حرف(c) :
				      dir == -1 ? tolower(c) : *p, باہرلا);
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
				fputs("\\b", باہرلا);
				اكسر;
			حالة '\t':
				fputs("\\t", باہرلا);
				اكسر;
			حالة '\n':
				fputs("\\n", باہرلا);
				اكسر;
			حالة '\f':
				fputs("\\f", باہرلا);
				اكسر;
			حالة '\r':
				fputs("\\r", باہرلا);
				اكسر;
			افتراضي:
				/* Other assorted control characters */
				fprintf(باہرلا, "\\u00%02x", c);
				اكسر;
		}
	}
	fputc('"', باہرلا);
}

#تعریف_کرو fputs_quoted_json(_d, _o)       fputs_quoted_case_json(_d, _o, 0, 0)
#تعریف_کرو fputs_quoted_json_اتلا(_d, _o) fputs_quoted_case_json(_d, _o, 1, 0)
#تعریف_کرو fputs_quoted_json_ہیٹھلا(_d, _o) fputs_quoted_case_json(_d, _o, -1, 0)

فراغ ul_jsonwrt_init(بنية ul_jsonwrt *fmt, FILE *باہرلا, صحيح indent, تعداد ul_json_format json_format)
{
	fmt->باہرلا = باہرلا;
	fmt->indent = indent;
	fmt->json_format = json_format;
	fmt->after_close = 0;
}

صحيح ul_jsonwrt_is_ready(بنية ul_jsonwrt *fmt)
{
	أرجع fmt->باہرلا == عدم ? 0 : 1;
}

فراغ ul_jsonwrt_indent(بنية ul_jsonwrt *fmt)
{
	صحيح i;

	لكل (i = 0; i < fmt->indent; i++)
		fputs("   ", fmt->باہرلا);
}

ساكن فراغ print_name(بنية ul_jsonwrt *fmt, ثابت حرف *name)
{
	/* Values on the top level are not members of an array or object, so
	 * they are never comma separated; ul_jsonwrt_close() has already
	 * terminated the previous one by a newline. */
	إذا (fmt->after_close && (fmt->indent > 0 || name)) {
		إذا (fmt->json_format == UL_JSON_سطر
		    || fmt->json_format == UL_JSON_COMPACT)
			fputs(",", fmt->باہرلا);
		وإلا
			fputs(name ? ",\n" : ",", fmt->باہرلا);
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
		fputs_quoted_json_ہیٹھلا(name, fmt->باہرلا);
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
	حالة UL_JSON_قدر:
		إذا (fmt->json_format == UL_JSON_سطر)
			s = name ? ":" : "";
		وإلا إذا (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":" : "";
		وإلا
			s = name ? ": " : " ";
		اكسر;
	}
	fputs(s, fmt->باہرلا);
	fmt->after_close = 0;
}

فراغ ul_jsonwrt_خالی_تھاواں(بنية ul_jsonwrt *fmt, ثابت حرف *name, صحيح type)
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
	حالة UL_JSON_قدر:
		إذا (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": null" : "null";
		وإلا
			s = name ? ":null" : "null";
		اكسر;
	}
	fputs(s, fmt->باہرلا);
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
	حالة UL_JSON_قدر:
		fmt->after_close = 1;
		أرجع;
	افتراضي:
		fmt->after_close = 1;
		أرجع;
	}

	/* In COMPACT, when closing a direct child of root (indent==1),
	 * insert a newline so the closing bracket starts on its own line. */
	إذا (fmt->json_format == UL_JSON_COMPACT && fmt->indent == 1)
		fputc('\n', fmt->باہرلا);

	بدل (fmt->json_format) {
	حالة UL_JSON_سطر:
	حالة UL_JSON_COMPACT:
		اكسر;
	حالة UL_JSON_PRETTY:
	افتراضي:
		fputs("\n", fmt->باہرلا);
		ul_jsonwrt_indent(fmt);
		اكسر;
	}

	fputc(endchr, fmt->باہرلا);

	/* Terminate the value on the top level. The writer may be re-initialized
	 * between the values (for example libsmartcols does it for every
	 * scols_table_print_range() call), so the terminator has to be printed
	 * by the value itself rather than by the next one (see print_name()). */
	إذا (fmt->indent == 0)
		fputc('\n', fmt->باہرلا);

	fmt->after_close = 1;
}


فراغ ul_jsonwrt_flush(بنية ul_jsonwrt *fmt)
{
	fflush(fmt->باہرلا);
}

فراغ ul_jsonwrt_قدر_raw(بنية ul_jsonwrt *fmt,
			ثابت حرف *name, ثابت حرف *انکڑے)
{
	ul_jsonwrt_قدر_open(fmt, name);
	إذا (انکڑے && *انکڑے)
		fputs(انکڑے, fmt->باہرلا);
	وإلا
		fputs("null", fmt->باہرلا);
	ul_jsonwrt_قدر_close(fmt);
}

فراغ ul_jsonwrt_قدر_s(بنية ul_jsonwrt *fmt,
			ثابت حرف *name, ثابت حرف *انکڑے)
{
	ul_jsonwrt_قدر_open(fmt, name);
	إذا (انکڑے && *انکڑے)
		fputs_quoted_json(انکڑے, fmt->باہرلا);
	وإلا
		fputs("null", fmt->باہرلا);
	ul_jsonwrt_قدر_close(fmt);
}

فراغ ul_jsonwrt_قدر_s_sized(بنية ul_jsonwrt *fmt,
			      ثابت حرف *name, ثابت حرف *انکڑے, size_t ناپ)
{
	ul_jsonwrt_قدر_open(fmt, name);
	إذا (انکڑے && *انکڑے)
		fputs_quoted_case_json(انکڑے, fmt->باہرلا, 0, ناپ);
	وإلا
		fputs("null", fmt->باہرلا);
	ul_jsonwrt_قدر_close(fmt);
}

فراغ ul_jsonwrt_قدر_u64(بنية ul_jsonwrt *fmt,
			ثابت حرف *name, uint64_t انکڑے)
{
	ul_jsonwrt_قدر_open(fmt, name);
	fprintf(fmt->باہرلا, "%"PRIu64, انکڑے);
	ul_jsonwrt_قدر_close(fmt);
}

فراغ ul_jsonwrt_قدر_double(بنية ul_jsonwrt *fmt,
			ثابت حرف *name, لما مزدوج انکڑے)
{
	ul_jsonwrt_قدر_open(fmt, name);
	fprintf(fmt->باہرلا, "%Lg", انکڑے);
	ul_jsonwrt_قدر_close(fmt);
}

فراغ ul_jsonwrt_قدر_boolean(بنية ul_jsonwrt *fmt,
			ثابت حرف *name, صحيح انکڑے)
{
	ul_jsonwrt_قدر_open(fmt, name);
	fputs(انکڑے ? "true" : "false", fmt->باہرلا);
	ul_jsonwrt_قدر_close(fmt);
}
