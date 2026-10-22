/*
 * JSON output formatting functions.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <inttypes.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cctype.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "jsonwrt.h"

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
ਸਥਿਰ ਖਾਲੀ fputs_quoted_case_json(ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ, FILE *ਬਾਹਰਲਾ, ਪੂਰਨ dir, size_t ਮਾਪ)
{
	ਅਚਲ ਅੱਖਰ *p;

	fputc('"', ਬਾਹਰਲਾ);
	ਲਈ (p = ਅੰਕੜੇ; p && *p && (!ਮਾਪ || p < ਅੰਕੜੇ + ਮਾਪ); p++) {

		ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ c = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) *p;

		/* From http://www.json.org
		 *
		 * The double-quote and backslashes would break out a string or
		 * init an escape sequence if not escaped.
		 *
		 * Note that single-quotes and forward slashes, while they're
		 * in the JSON spec, don't break double-quoted strings.
		 */
		ਜੇ (c == '"' || c == '\\') {
			fputc('\\', ਬਾਹਰਲਾ);
			fputc(c, ਬਾਹਰਲਾ);
			ਜਾਰੀ;
		}

		/* All non-control characters OK; do the case swap as required. */
		ਜੇ (c >= 0x20) {
			/*
			 * Don't use locale sensitive ctype.h functions for regular
			 * ASCII chars, because for example with Turkish locale
			 * (aka LANG=tr_TR.UTF-8) toupper('I') returns 'I'.
			 */
			ਜੇ (c <= 127)
				fputc(dir ==  1 ? c_toupper(c) :
				      dir == -1 ? c_tolower(c) : *p, ਬਾਹਰਲਾ);
			ਨਹੀਂਤਾਂ
				fputc(dir ==  1 ? ਵੱਡਾ_ਅੱਖਰ(c) :
				      dir == -1 ? tolower(c) : *p, ਬਾਹਰਲਾ);
			ਜਾਰੀ;
		}

		/* In addition, all chars under ' ' break Node's/V8/Chrome's, and
		 * Firefox's JSON.parse function
		 */
		ਚੋਣ (c) {
			/* Handle short-hand cases to reduce output size.  C
			 * has most of the same stuff here, so if there's an
			 * "Escape for C" function somewhere in the STL, we
			 * should probably be using it.
			 */
			ਹਾਲਤ '\b':
				fputs("\\b", ਬਾਹਰਲਾ);
				ਰੋਕੋ;
			ਹਾਲਤ '\t':
				fputs("\\t", ਬਾਹਰਲਾ);
				ਰੋਕੋ;
			ਹਾਲਤ '\n':
				fputs("\\n", ਬਾਹਰਲਾ);
				ਰੋਕੋ;
			ਹਾਲਤ '\f':
				fputs("\\f", ਬਾਹਰਲਾ);
				ਰੋਕੋ;
			ਹਾਲਤ '\r':
				fputs("\\r", ਬਾਹਰਲਾ);
				ਰੋਕੋ;
			ਮੂਲ:
				/* Other assorted control characters */
				fprintf(ਬਾਹਰਲਾ, "\\u00%02x", c);
				ਰੋਕੋ;
		}
	}
	fputc('"', ਬਾਹਰਲਾ);
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fputs_quoted_json(_d, _o)       fputs_quoted_case_json(_d, _o, 0, 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fputs_quoted_json_ਉੱਤਲਾ(_d, _o) fputs_quoted_case_json(_d, _o, 1, 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fputs_quoted_json_ਹੇਠਲਾ(_d, _o) fputs_quoted_case_json(_d, _o, -1, 0)

ਖਾਲੀ ul_jsonwrt_init(ਬਣਤਰ ul_jsonwrt *fmt, FILE *ਬਾਹਰਲਾ, ਪੂਰਨ indent, ਗਿਣਤੀ ul_json_format json_format)
{
	fmt->ਬਾਹਰਲਾ = ਬਾਹਰਲਾ;
	fmt->indent = indent;
	fmt->json_format = json_format;
	fmt->after_close = 0;
}

ਪੂਰਨ ul_jsonwrt_is_ready(ਬਣਤਰ ul_jsonwrt *fmt)
{
	ਵਾਪਸ fmt->ਬਾਹਰਲਾ == ਨਿਰਾ ? 0 : 1;
}

ਖਾਲੀ ul_jsonwrt_indent(ਬਣਤਰ ul_jsonwrt *fmt)
{
	ਪੂਰਨ i;

	ਲਈ (i = 0; i < fmt->indent; i++)
		fputs("   ", fmt->ਬਾਹਰਲਾ);
}

ਸਥਿਰ ਖਾਲੀ print_name(ਬਣਤਰ ul_jsonwrt *fmt, ਅਚਲ ਅੱਖਰ *name)
{
	/* Values on the top level are not members of an array or object, so
	 * they are never comma separated; ul_jsonwrt_close() has already
	 * terminated the previous one by a newline. */
	ਜੇ (fmt->after_close && (fmt->indent > 0 || name)) {
		ਜੇ (fmt->json_format == UL_JSON_ਸਤਰ
		    || fmt->json_format == UL_JSON_COMPACT)
			fputs(",", fmt->ਬਾਹਰਲਾ);
		ਨਹੀਂਤਾਂ
			fputs(name ? ",\n" : ",", fmt->ਬਾਹਰਲਾ);
	}

	ਚੋਣ (fmt->json_format) {
	ਹਾਲਤ UL_JSON_ਸਤਰ:
	ਹਾਲਤ UL_JSON_COMPACT:
		ਰੋਕੋ;
	ਹਾਲਤ UL_JSON_PRETTY:
	ਮੂਲ:
		ਜੇ (name || !fmt->after_close)
			ul_jsonwrt_indent(fmt);
		ਰੋਕੋ;
	}
	ਜੇ (name)
		fputs_quoted_json_ਹੇਠਲਾ(name, fmt->ਬਾਹਰਲਾ);
}

ਖਾਲੀ ul_jsonwrt_open(ਬਣਤਰ ul_jsonwrt *fmt, ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ type)
{
	ਅਚਲ ਅੱਖਰ *s = "";

	print_name(fmt, name);

	ਚੋਣ (type) {
	ਹਾਲਤ UL_JSON_OBJECT:
		ਜੇ (fmt->json_format == UL_JSON_ਸਤਰ)
			s = "{";
		ਨਹੀਂਤਾਂ ਜੇ (fmt->json_format == UL_JSON_COMPACT) {
			/* At nesting depth 2, put the object opens on a new line
			 * for readability; otherwise keep fully compact (no newline). */
			ਜੇ (fmt->indent == 2)
				s = name ? ":{" : "\n{";
			ਨਹੀਂਤਾਂ
				s = "{";
		} ਨਹੀਂਤਾਂ
			s = name ? ": {\n" : "{\n";
		fmt->indent++;
		ਰੋਕੋ;
	ਹਾਲਤ UL_JSON_ARRAY:
		ਜੇ (fmt->json_format == UL_JSON_ਸਤਰ)
			s = "[";
		ਨਹੀਂਤਾਂ ਜੇ (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":[" : "[";
		ਨਹੀਂਤਾਂ
			s = name ? ": [\n" : "[\n";
		fmt->indent++;
		ਰੋਕੋ;
	ਹਾਲਤ UL_JSON_ਮੁੱਲ:
		ਜੇ (fmt->json_format == UL_JSON_ਸਤਰ)
			s = name ? ":" : "";
		ਨਹੀਂਤਾਂ ਜੇ (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":" : "";
		ਨਹੀਂਤਾਂ
			s = name ? ": " : " ";
		ਰੋਕੋ;
	}
	fputs(s, fmt->ਬਾਹਰਲਾ);
	fmt->after_close = 0;
}

ਖਾਲੀ ul_jsonwrt_ਖਾਲੀ_ਥਾਵਾਂ(ਬਣਤਰ ul_jsonwrt *fmt, ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ type)
{
	ਅਚਲ ਅੱਖਰ *s = "";

	print_name(fmt, name);

	ਚੋਣ (type) {
	ਹਾਲਤ UL_JSON_OBJECT:
		ਜੇ (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": {}" : "{}";
		ਨਹੀਂਤਾਂ
			s = name ? ":{}" : "{}";
		ਰੋਕੋ;
	ਹਾਲਤ UL_JSON_ARRAY:
		ਜੇ (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": []" : "[]";
		ਨਹੀਂਤਾਂ
			s = name ? ":[]" : "[]";
		ਰੋਕੋ;
	ਹਾਲਤ UL_JSON_ਮੁੱਲ:
		ਜੇ (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": null" : "null";
		ਨਹੀਂਤਾਂ
			s = name ? ":null" : "null";
		ਰੋਕੋ;
	}
	fputs(s, fmt->ਬਾਹਰਲਾ);
	fmt->after_close = 1;
}

ਖਾਲੀ ul_jsonwrt_close(ਬਣਤਰ ul_jsonwrt *fmt, ਪੂਰਨ type)
{
	ਅੱਖਰ endchr;

	ਪ੍ਰਤਿਜ੍ਞਾ(fmt->indent > 0);

	ਚੋਣ (type) {
	ਹਾਲਤ UL_JSON_OBJECT:
		fmt->indent--;
		endchr = '}';
		ਰੋਕੋ;
	ਹਾਲਤ UL_JSON_ARRAY:
		fmt->indent--;
		endchr = ']';
		ਰੋਕੋ;
	ਹਾਲਤ UL_JSON_ਮੁੱਲ:
		fmt->after_close = 1;
		ਵਾਪਸ;
	ਮੂਲ:
		fmt->after_close = 1;
		ਵਾਪਸ;
	}

	/* In COMPACT, when closing a direct child of root (indent==1),
	 * insert a newline so the closing bracket starts on its own line. */
	ਜੇ (fmt->json_format == UL_JSON_COMPACT && fmt->indent == 1)
		fputc('\n', fmt->ਬਾਹਰਲਾ);

	ਚੋਣ (fmt->json_format) {
	ਹਾਲਤ UL_JSON_ਸਤਰ:
	ਹਾਲਤ UL_JSON_COMPACT:
		ਰੋਕੋ;
	ਹਾਲਤ UL_JSON_PRETTY:
	ਮੂਲ:
		fputs("\n", fmt->ਬਾਹਰਲਾ);
		ul_jsonwrt_indent(fmt);
		ਰੋਕੋ;
	}

	fputc(endchr, fmt->ਬਾਹਰਲਾ);

	/* Terminate the value on the top level. The writer may be re-initialized
	 * between the values (for example libsmartcols does it for every
	 * scols_table_print_range() call), so the terminator has to be printed
	 * by the value itself rather than by the next one (see print_name()). */
	ਜੇ (fmt->indent == 0)
		fputc('\n', fmt->ਬਾਹਰਲਾ);

	fmt->after_close = 1;
}


ਖਾਲੀ ul_jsonwrt_flush(ਬਣਤਰ ul_jsonwrt *fmt)
{
	fflush(fmt->ਬਾਹਰਲਾ);
}

ਖਾਲੀ ul_jsonwrt_ਮੁੱਲ_raw(ਬਣਤਰ ul_jsonwrt *fmt,
			ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ)
{
	ul_jsonwrt_ਮੁੱਲ_open(fmt, name);
	ਜੇ (ਅੰਕੜੇ && *ਅੰਕੜੇ)
		fputs(ਅੰਕੜੇ, fmt->ਬਾਹਰਲਾ);
	ਨਹੀਂਤਾਂ
		fputs("null", fmt->ਬਾਹਰਲਾ);
	ul_jsonwrt_ਮੁੱਲ_close(fmt);
}

ਖਾਲੀ ul_jsonwrt_ਮੁੱਲ_s(ਬਣਤਰ ul_jsonwrt *fmt,
			ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ)
{
	ul_jsonwrt_ਮੁੱਲ_open(fmt, name);
	ਜੇ (ਅੰਕੜੇ && *ਅੰਕੜੇ)
		fputs_quoted_json(ਅੰਕੜੇ, fmt->ਬਾਹਰਲਾ);
	ਨਹੀਂਤਾਂ
		fputs("null", fmt->ਬਾਹਰਲਾ);
	ul_jsonwrt_ਮੁੱਲ_close(fmt);
}

ਖਾਲੀ ul_jsonwrt_ਮੁੱਲ_s_sized(ਬਣਤਰ ul_jsonwrt *fmt,
			      ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ, size_t ਮਾਪ)
{
	ul_jsonwrt_ਮੁੱਲ_open(fmt, name);
	ਜੇ (ਅੰਕੜੇ && *ਅੰਕੜੇ)
		fputs_quoted_case_json(ਅੰਕੜੇ, fmt->ਬਾਹਰਲਾ, 0, ਮਾਪ);
	ਨਹੀਂਤਾਂ
		fputs("null", fmt->ਬਾਹਰਲਾ);
	ul_jsonwrt_ਮੁੱਲ_close(fmt);
}

ਖਾਲੀ ul_jsonwrt_ਮੁੱਲ_u64(ਬਣਤਰ ul_jsonwrt *fmt,
			ਅਚਲ ਅੱਖਰ *name, uint64_t ਅੰਕੜੇ)
{
	ul_jsonwrt_ਮੁੱਲ_open(fmt, name);
	fprintf(fmt->ਬਾਹਰਲਾ, "%"PRIu64, ਅੰਕੜੇ);
	ul_jsonwrt_ਮੁੱਲ_close(fmt);
}

ਖਾਲੀ ul_jsonwrt_ਮੁੱਲ_double(ਬਣਤਰ ul_jsonwrt *fmt,
			ਅਚਲ ਅੱਖਰ *name, ਲੰਮਾ ਦੁੱਗਣਾ ਅੰਕੜੇ)
{
	ul_jsonwrt_ਮੁੱਲ_open(fmt, name);
	fprintf(fmt->ਬਾਹਰਲਾ, "%Lg", ਅੰਕੜੇ);
	ul_jsonwrt_ਮੁੱਲ_close(fmt);
}

ਖਾਲੀ ul_jsonwrt_ਮੁੱਲ_boolean(ਬਣਤਰ ul_jsonwrt *fmt,
			ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ ਅੰਕੜੇ)
{
	ul_jsonwrt_ਮੁੱਲ_open(fmt, name);
	fputs(ਅੰਕੜੇ ? "true" : "false", fmt->ਬਾਹਰਲਾ);
	ul_jsonwrt_ਮੁੱਲ_close(fmt);
}
