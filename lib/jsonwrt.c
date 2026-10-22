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
ساکن خالی fputs_quoted_case_json(ثابت حرف *ڈیٹا, FILE *برآمد, صحیح dir, size_t سائز)
{
	ثابت حرف *p;

	fputc('"', برآمد);
	برائے (p = ڈیٹا; p && *p && (!سائز || p < ڈیٹا + سائز); p++) {

		ثابت غیرعلامتی صحیح c = (غیرعلامتی صحیح) *p;

		/* From http://www.json.org
		 *
		 * The double-quote and backslashes would break out a string or
		 * init an escape sequence if not escaped.
		 *
		 * Note that single-quotes and forward slashes, while they're
		 * in the JSON spec, don't break double-quoted strings.
		 */
		اگر (c == '"' || c == '\\') {
			fputc('\\', برآمد);
			fputc(c, برآمد);
			جاری;
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
				      dir == -1 ? c_tolower(c) : *p, برآمد);
			ورنہ
				fputc(dir ==  1 ? بڑا_حرف(c) :
				      dir == -1 ? tolower(c) : *p, برآمد);
			جاری;
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
				fputs("\\b", برآمد);
				توڑو;
			حالت '\t':
				fputs("\\t", برآمد);
				توڑو;
			حالت '\n':
				fputs("\\n", برآمد);
				توڑو;
			حالت '\f':
				fputs("\\f", برآمد);
				توڑو;
			حالت '\r':
				fputs("\\r", برآمد);
				توڑو;
			طےشدہ:
				/* Other assorted control characters */
				fprintf(برآمد, "\\u00%02x", c);
				توڑو;
		}
	}
	fputc('"', برآمد);
}

#تعریف_کرو fputs_quoted_json(_d, _o)       fputs_quoted_case_json(_d, _o, 0, 0)
#تعریف_کرو fputs_quoted_json_بالائی(_d, _o) fputs_quoted_case_json(_d, _o, 1, 0)
#تعریف_کرو fputs_quoted_json_زیریں(_d, _o) fputs_quoted_case_json(_d, _o, -1, 0)

خالی ul_jsonwrt_init(ساخت ul_jsonwrt *fmt, FILE *برآمد, صحیح indent, شمار ul_json_format json_format)
{
	fmt->برآمد = برآمد;
	fmt->indent = indent;
	fmt->json_format = json_format;
	fmt->after_close = 0;
}

صحیح ul_jsonwrt_is_ready(ساخت ul_jsonwrt *fmt)
{
	واپس fmt->برآمد == تہی ? 0 : 1;
}

خالی ul_jsonwrt_indent(ساخت ul_jsonwrt *fmt)
{
	صحیح i;

	برائے (i = 0; i < fmt->indent; i++)
		fputs("   ", fmt->برآمد);
}

ساکن خالی print_name(ساخت ul_jsonwrt *fmt, ثابت حرف *name)
{
	/* Values on the top level are not members of an array or object, so
	 * they are never comma separated; ul_jsonwrt_close() has already
	 * terminated the previous one by a newline. */
	اگر (fmt->after_close && (fmt->indent > 0 || name)) {
		اگر (fmt->json_format == UL_JSON_سطر
		    || fmt->json_format == UL_JSON_COMPACT)
			fputs(",", fmt->برآمد);
		ورنہ
			fputs(name ? ",\n" : ",", fmt->برآمد);
	}

	انتخاب (fmt->json_format) {
	حالت UL_JSON_سطر:
	حالت UL_JSON_COMPACT:
		توڑو;
	حالت UL_JSON_PRETTY:
	طےشدہ:
		اگر (name || !fmt->after_close)
			ul_jsonwrt_indent(fmt);
		توڑو;
	}
	اگر (name)
		fputs_quoted_json_زیریں(name, fmt->برآمد);
}

خالی ul_jsonwrt_open(ساخت ul_jsonwrt *fmt, ثابت حرف *name, صحیح type)
{
	ثابت حرف *s = "";

	print_name(fmt, name);

	انتخاب (type) {
	حالت UL_JSON_OBJECT:
		اگر (fmt->json_format == UL_JSON_سطر)
			s = "{";
		ورنہ اگر (fmt->json_format == UL_JSON_COMPACT) {
			/* At nesting depth 2, put the object opens on a new line
			 * for readability; otherwise keep fully compact (no newline). */
			اگر (fmt->indent == 2)
				s = name ? ":{" : "\n{";
			ورنہ
				s = "{";
		} ورنہ
			s = name ? ": {\n" : "{\n";
		fmt->indent++;
		توڑو;
	حالت UL_JSON_ARRAY:
		اگر (fmt->json_format == UL_JSON_سطر)
			s = "[";
		ورنہ اگر (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":[" : "[";
		ورنہ
			s = name ? ": [\n" : "[\n";
		fmt->indent++;
		توڑو;
	حالت UL_JSON_قدر:
		اگر (fmt->json_format == UL_JSON_سطر)
			s = name ? ":" : "";
		ورنہ اگر (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":" : "";
		ورنہ
			s = name ? ": " : " ";
		توڑو;
	}
	fputs(s, fmt->برآمد);
	fmt->after_close = 0;
}

خالی ul_jsonwrt_خالی_خانے(ساخت ul_jsonwrt *fmt, ثابت حرف *name, صحیح type)
{
	ثابت حرف *s = "";

	print_name(fmt, name);

	انتخاب (type) {
	حالت UL_JSON_OBJECT:
		اگر (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": {}" : "{}";
		ورنہ
			s = name ? ":{}" : "{}";
		توڑو;
	حالت UL_JSON_ARRAY:
		اگر (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": []" : "[]";
		ورنہ
			s = name ? ":[]" : "[]";
		توڑو;
	حالت UL_JSON_قدر:
		اگر (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": null" : "null";
		ورنہ
			s = name ? ":null" : "null";
		توڑو;
	}
	fputs(s, fmt->برآمد);
	fmt->after_close = 1;
}

خالی ul_jsonwrt_close(ساخت ul_jsonwrt *fmt, صحیح type)
{
	حرف endchr;

	أكد(fmt->indent > 0);

	انتخاب (type) {
	حالت UL_JSON_OBJECT:
		fmt->indent--;
		endchr = '}';
		توڑو;
	حالت UL_JSON_ARRAY:
		fmt->indent--;
		endchr = ']';
		توڑو;
	حالت UL_JSON_قدر:
		fmt->after_close = 1;
		واپس;
	طےشدہ:
		fmt->after_close = 1;
		واپس;
	}

	/* In COMPACT, when closing a direct child of root (indent==1),
	 * insert a newline so the closing bracket starts on its own line. */
	اگر (fmt->json_format == UL_JSON_COMPACT && fmt->indent == 1)
		fputc('\n', fmt->برآمد);

	انتخاب (fmt->json_format) {
	حالت UL_JSON_سطر:
	حالت UL_JSON_COMPACT:
		توڑو;
	حالت UL_JSON_PRETTY:
	طےشدہ:
		fputs("\n", fmt->برآمد);
		ul_jsonwrt_indent(fmt);
		توڑو;
	}

	fputc(endchr, fmt->برآمد);

	/* Terminate the value on the top level. The writer may be re-initialized
	 * between the values (for example libsmartcols does it for every
	 * scols_table_print_range() call), so the terminator has to be printed
	 * by the value itself rather than by the next one (see print_name()). */
	اگر (fmt->indent == 0)
		fputc('\n', fmt->برآمد);

	fmt->after_close = 1;
}


خالی ul_jsonwrt_flush(ساخت ul_jsonwrt *fmt)
{
	fflush(fmt->برآمد);
}

خالی ul_jsonwrt_قدر_raw(ساخت ul_jsonwrt *fmt,
			ثابت حرف *name, ثابت حرف *ڈیٹا)
{
	ul_jsonwrt_قدر_open(fmt, name);
	اگر (ڈیٹا && *ڈیٹا)
		fputs(ڈیٹا, fmt->برآمد);
	ورنہ
		fputs("null", fmt->برآمد);
	ul_jsonwrt_قدر_close(fmt);
}

خالی ul_jsonwrt_قدر_s(ساخت ul_jsonwrt *fmt,
			ثابت حرف *name, ثابت حرف *ڈیٹا)
{
	ul_jsonwrt_قدر_open(fmt, name);
	اگر (ڈیٹا && *ڈیٹا)
		fputs_quoted_json(ڈیٹا, fmt->برآمد);
	ورنہ
		fputs("null", fmt->برآمد);
	ul_jsonwrt_قدر_close(fmt);
}

خالی ul_jsonwrt_قدر_s_sized(ساخت ul_jsonwrt *fmt,
			      ثابت حرف *name, ثابت حرف *ڈیٹا, size_t سائز)
{
	ul_jsonwrt_قدر_open(fmt, name);
	اگر (ڈیٹا && *ڈیٹا)
		fputs_quoted_case_json(ڈیٹا, fmt->برآمد, 0, سائز);
	ورنہ
		fputs("null", fmt->برآمد);
	ul_jsonwrt_قدر_close(fmt);
}

خالی ul_jsonwrt_قدر_u64(ساخت ul_jsonwrt *fmt,
			ثابت حرف *name, uint64_t ڈیٹا)
{
	ul_jsonwrt_قدر_open(fmt, name);
	fprintf(fmt->برآمد, "%"PRIu64, ڈیٹا);
	ul_jsonwrt_قدر_close(fmt);
}

خالی ul_jsonwrt_قدر_double(ساخت ul_jsonwrt *fmt,
			ثابت حرف *name, طویل دوہرا ڈیٹا)
{
	ul_jsonwrt_قدر_open(fmt, name);
	fprintf(fmt->برآمد, "%Lg", ڈیٹا);
	ul_jsonwrt_قدر_close(fmt);
}

خالی ul_jsonwrt_قدر_boolean(ساخت ul_jsonwrt *fmt,
			ثابت حرف *name, صحیح ڈیٹا)
{
	ul_jsonwrt_قدر_open(fmt, name);
	fputs(ڈیٹا ? "true" : "false", fmt->برآمد);
	ul_jsonwrt_قدر_close(fmt);
}
