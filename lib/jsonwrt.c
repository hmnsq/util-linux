/*
 * JSON output formatting functions.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <inttypes.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <cctype.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "jsonwrt.h"

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
स्थिर शून्य fputs_quoted_case_json(ध्रुव वर्ण *data, खाता *out, पूर्णाङ्क dir, माप_प्रकार size)
{
	ध्रुव वर्ण *p;

	ख_अक्षर_दो('"', out);
	प्रति (p = data; p && *p && (!size || p < data + size); p++) {

		ध्रुव अचिन्हित पूर्णाङ्क c = (अचिन्हित पूर्णाङ्क) *p;

		/* From http://www.json.org
		 *
		 * The double-quote and backslashes would break out a string or
		 * init an escape sequence if not escaped.
		 *
		 * Note that single-quotes and forward slashes, while they're
		 * in the JSON spec, don't break double-quoted strings.
		 */
		यदि (c == '"' || c == '\\') {
			ख_अक्षर_दो('\\', out);
			ख_अक्षर_दो(c, out);
			अनुवर्त;
		}

		/* All non-control characters OK; do the case swap as required. */
		यदि (c >= 0x20) {
			/*
			 * Don't use locale sensitive ctype.h functions for regular
			 * ASCII chars, because for example with Turkish locale
			 * (aka LANG=tr_TR.UTF-8) toupper('I') returns 'I'.
			 */
			यदि (c <= 127)
				ख_अक्षर_दो(dir ==  1 ? c_toupper(c) :
				      dir == -1 ? c_tolower(c) : *p, out);
			अन्यथा
				ख_अक्षर_दो(dir ==  1 ? बड़े(c) :
				      dir == -1 ? छोटे(c) : *p, out);
			अनुवर्त;
		}

		/* In addition, all chars under ' ' break Node's/V8/Chrome's, and
		 * Firefox's JSON.parse function
		 */
		विकल्प (c) {
			/* Handle short-hand cases to reduce output size.  C
			 * has most of the same stuff here, so if there's an
			 * "Escape for C" function somewhere in the STL, we
			 * should probably be using it.
			 */
			पक्ष '\b':
				ख_माला_दो("\\b", out);
				भङ्ग;
			पक्ष '\t':
				ख_माला_दो("\\t", out);
				भङ्ग;
			पक्ष '\n':
				ख_माला_दो("\\n", out);
				भङ्ग;
			पक्ष '\f':
				ख_माला_दो("\\f", out);
				भङ्ग;
			पक्ष '\r':
				ख_माला_दो("\\r", out);
				भङ्ग;
			स्वभाव:
				/* Other assorted control characters */
				ख_लिखो(out, "\\u00%02x", c);
				भङ्ग;
		}
	}
	ख_अक्षर_दो('"', out);
}

#define fputs_quoted_json(_d, _o)       fputs_quoted_case_json(_d, _o, 0, 0)
#define fputs_quoted_json_upper(_d, _o) fputs_quoted_case_json(_d, _o, 1, 0)
#define fputs_quoted_json_lower(_d, _o) fputs_quoted_case_json(_d, _o, -1, 0)

शून्य ul_jsonwrt_init(संरचना ul_jsonwrt *fmt, खाता *out, पूर्णाङ्क indent, गणना ul_json_format json_format)
{
	fmt->out = out;
	fmt->indent = indent;
	fmt->json_format = json_format;
	fmt->after_close = 0;
}

पूर्णाङ्क ul_jsonwrt_is_ready(संरचना ul_jsonwrt *fmt)
{
	प्रत्यागम fmt->out == रिक्त ? 0 : 1;
}

शून्य ul_jsonwrt_indent(संरचना ul_jsonwrt *fmt)
{
	पूर्णाङ्क i;

	प्रति (i = 0; i < fmt->indent; i++)
		ख_माला_दो("   ", fmt->out);
}

स्थिर शून्य print_name(संरचना ul_jsonwrt *fmt, ध्रुव वर्ण *name)
{
	/* Values on the top level are not members of an array or object, so
	 * they are never comma separated; ul_jsonwrt_close() has already
	 * terminated the previous one by a newline. */
	यदि (fmt->after_close && (fmt->indent > 0 || name)) {
		यदि (fmt->json_format == UL_JSON_पंक्ति
		    || fmt->json_format == UL_JSON_COMPACT)
			ख_माला_दो(",", fmt->out);
		अन्यथा
			ख_माला_दो(name ? ",\n" : ",", fmt->out);
	}

	विकल्प (fmt->json_format) {
	पक्ष UL_JSON_पंक्ति:
	पक्ष UL_JSON_COMPACT:
		भङ्ग;
	पक्ष UL_JSON_PRETTY:
	स्वभाव:
		यदि (name || !fmt->after_close)
			ul_jsonwrt_indent(fmt);
		भङ्ग;
	}
	यदि (name)
		fputs_quoted_json_lower(name, fmt->out);
}

शून्य ul_jsonwrt_open(संरचना ul_jsonwrt *fmt, ध्रुव वर्ण *name, पूर्णाङ्क type)
{
	ध्रुव वर्ण *s = "";

	print_name(fmt, name);

	विकल्प (type) {
	पक्ष UL_JSON_OBJECT:
		यदि (fmt->json_format == UL_JSON_पंक्ति)
			s = "{";
		अन्यथा यदि (fmt->json_format == UL_JSON_COMPACT) {
			/* At nesting depth 2, put the object opens on a new line
			 * for readability; otherwise keep fully compact (no newline). */
			यदि (fmt->indent == 2)
				s = name ? ":{" : "\n{";
			अन्यथा
				s = "{";
		} अन्यथा
			s = name ? ": {\n" : "{\n";
		fmt->indent++;
		भङ्ग;
	पक्ष UL_JSON_ARRAY:
		यदि (fmt->json_format == UL_JSON_पंक्ति)
			s = "[";
		अन्यथा यदि (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":[" : "[";
		अन्यथा
			s = name ? ": [\n" : "[\n";
		fmt->indent++;
		भङ्ग;
	पक्ष UL_JSON_मूल्य:
		यदि (fmt->json_format == UL_JSON_पंक्ति)
			s = name ? ":" : "";
		अन्यथा यदि (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":" : "";
		अन्यथा
			s = name ? ": " : " ";
		भङ्ग;
	}
	ख_माला_दो(s, fmt->out);
	fmt->after_close = 0;
}

शून्य ul_jsonwrt_empty(संरचना ul_jsonwrt *fmt, ध्रुव वर्ण *name, पूर्णाङ्क type)
{
	ध्रुव वर्ण *s = "";

	print_name(fmt, name);

	विकल्प (type) {
	पक्ष UL_JSON_OBJECT:
		यदि (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": {}" : "{}";
		अन्यथा
			s = name ? ":{}" : "{}";
		भङ्ग;
	पक्ष UL_JSON_ARRAY:
		यदि (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": []" : "[]";
		अन्यथा
			s = name ? ":[]" : "[]";
		भङ्ग;
	पक्ष UL_JSON_मूल्य:
		यदि (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": null" : "null";
		अन्यथा
			s = name ? ":null" : "null";
		भङ्ग;
	}
	ख_माला_दो(s, fmt->out);
	fmt->after_close = 1;
}

शून्य ul_jsonwrt_close(संरचना ul_jsonwrt *fmt, पूर्णाङ्क type)
{
	वर्ण endchr;

	प्रतिज्ञा(fmt->indent > 0);

	विकल्प (type) {
	पक्ष UL_JSON_OBJECT:
		fmt->indent--;
		endchr = '}';
		भङ्ग;
	पक्ष UL_JSON_ARRAY:
		fmt->indent--;
		endchr = ']';
		भङ्ग;
	पक्ष UL_JSON_मूल्य:
		fmt->after_close = 1;
		प्रत्यागम;
	स्वभाव:
		fmt->after_close = 1;
		प्रत्यागम;
	}

	/* In COMPACT, when closing a direct child of root (indent==1),
	 * insert a newline so the closing bracket starts on its own line. */
	यदि (fmt->json_format == UL_JSON_COMPACT && fmt->indent == 1)
		ख_अक्षर_दो('\n', fmt->out);

	विकल्प (fmt->json_format) {
	पक्ष UL_JSON_पंक्ति:
	पक्ष UL_JSON_COMPACT:
		भङ्ग;
	पक्ष UL_JSON_PRETTY:
	स्वभाव:
		ख_माला_दो("\n", fmt->out);
		ul_jsonwrt_indent(fmt);
		भङ्ग;
	}

	ख_अक्षर_दो(endchr, fmt->out);

	/* Terminate the value on the top level. The writer may be re-initialized
	 * between the values (for example libsmartcols does it for every
	 * scols_table_print_range() call), so the terminator has to be printed
	 * by the value itself rather than by the next one (see print_name()). */
	यदि (fmt->indent == 0)
		ख_अक्षर_दो('\n', fmt->out);

	fmt->after_close = 1;
}


शून्य ul_jsonwrt_flush(संरचना ul_jsonwrt *fmt)
{
	ख_साफ(fmt->out);
}

शून्य ul_jsonwrt_मूल्य_raw(संरचना ul_jsonwrt *fmt,
			ध्रुव वर्ण *name, ध्रुव वर्ण *data)
{
	ul_jsonwrt_मूल्य_open(fmt, name);
	यदि (data && *data)
		ख_माला_दो(data, fmt->out);
	अन्यथा
		ख_माला_दो("null", fmt->out);
	ul_jsonwrt_मूल्य_close(fmt);
}

शून्य ul_jsonwrt_मूल्य_s(संरचना ul_jsonwrt *fmt,
			ध्रुव वर्ण *name, ध्रुव वर्ण *data)
{
	ul_jsonwrt_मूल्य_open(fmt, name);
	यदि (data && *data)
		fputs_quoted_json(data, fmt->out);
	अन्यथा
		ख_माला_दो("null", fmt->out);
	ul_jsonwrt_मूल्य_close(fmt);
}

शून्य ul_jsonwrt_मूल्य_s_sized(संरचना ul_jsonwrt *fmt,
			      ध्रुव वर्ण *name, ध्रुव वर्ण *data, माप_प्रकार size)
{
	ul_jsonwrt_मूल्य_open(fmt, name);
	यदि (data && *data)
		fputs_quoted_case_json(data, fmt->out, 0, size);
	अन्यथा
		ख_माला_दो("null", fmt->out);
	ul_jsonwrt_मूल्य_close(fmt);
}

शून्य ul_jsonwrt_मूल्य_u64(संरचना ul_jsonwrt *fmt,
			ध्रुव वर्ण *name, uint64_t data)
{
	ul_jsonwrt_मूल्य_open(fmt, name);
	ख_लिखो(fmt->out, "%"PRIu64, data);
	ul_jsonwrt_मूल्य_close(fmt);
}

शून्य ul_jsonwrt_मूल्य_double(संरचना ul_jsonwrt *fmt,
			ध्रुव वर्ण *name, दीर्घ द्विगुण data)
{
	ul_jsonwrt_मूल्य_open(fmt, name);
	ख_लिखो(fmt->out, "%Lg", data);
	ul_jsonwrt_मूल्य_close(fmt);
}

शून्य ul_jsonwrt_मूल्य_boolean(संरचना ul_jsonwrt *fmt,
			ध्रुव वर्ण *name, पूर्णाङ्क data)
{
	ul_jsonwrt_मूल्य_open(fmt, name);
	ख_माला_दो(data ? "true" : "false", fmt->out);
	ul_jsonwrt_मूल्य_close(fmt);
}
