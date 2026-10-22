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
स्थिर शून्य fputs_quoted_case_json(स्थिरांक अक्षर *data, खाता *out, पूर्णांक dir, माप_प्रकार size)
{
	स्थिरांक अक्षर *p;

	ख_अक्षर_दो('"', out);
	साठी (p = data; p && *p && (!size || p < data + size); p++) {

		स्थिरांक अचिन्हित पूर्णांक c = (अचिन्हित पूर्णांक) *p;

		/* From http://www.json.org
		 *
		 * The double-quote and backslashes would break out a string or
		 * init an escape sequence if not escaped.
		 *
		 * Note that single-quotes and forward slashes, while they're
		 * in the JSON spec, don't break double-quoted strings.
		 */
		जर (c == '"' || c == '\\') {
			ख_अक्षर_दो('\\', out);
			ख_अक्षर_दो(c, out);
			सुरूठेवा;
		}

		/* All non-control characters OK; do the case swap as required. */
		जर (c >= 0x20) {
			/*
			 * Don't use locale sensitive ctype.h functions for regular
			 * ASCII chars, because for example with Turkish locale
			 * (aka LANG=tr_TR.UTF-8) toupper('I') returns 'I'.
			 */
			जर (c <= 127)
				ख_अक्षर_दो(dir ==  1 ? c_toupper(c) :
				      dir == -1 ? c_tolower(c) : *p, out);
			अन्यथा
				ख_अक्षर_दो(dir ==  1 ? बड़े(c) :
				      dir == -1 ? छोटे(c) : *p, out);
			सुरूठेवा;
		}

		/* In addition, all chars under ' ' break Node's/V8/Chrome's, and
		 * Firefox's JSON.parse function
		 */
		निवड (c) {
			/* Handle short-hand cases to reduce output size.  C
			 * has most of the same stuff here, so if there's an
			 * "Escape for C" function somewhere in the STL, we
			 * should probably be using it.
			 */
			स्थिती '\b':
				ख_माला_दो("\\b", out);
				थांबा;
			स्थिती '\t':
				ख_माला_दो("\\t", out);
				थांबा;
			स्थिती '\n':
				ख_माला_दो("\\n", out);
				थांबा;
			स्थिती '\f':
				ख_माला_दो("\\f", out);
				थांबा;
			स्थिती '\r':
				ख_माला_दो("\\r", out);
				थांबा;
			पूर्वनिर्धारित:
				/* Other assorted control characters */
				ख_लिखो(out, "\\u00%02x", c);
				थांबा;
		}
	}
	ख_अक्षर_दो('"', out);
}

#define fputs_quoted_json(_d, _o)       fputs_quoted_case_json(_d, _o, 0, 0)
#define fputs_quoted_json_upper(_d, _o) fputs_quoted_case_json(_d, _o, 1, 0)
#define fputs_quoted_json_lower(_d, _o) fputs_quoted_case_json(_d, _o, -1, 0)

शून्य ul_jsonwrt_init(संरचना ul_jsonwrt *fmt, खाता *out, पूर्णांक indent, गणना ul_json_format json_format)
{
	fmt->out = out;
	fmt->indent = indent;
	fmt->json_format = json_format;
	fmt->after_close = 0;
}

पूर्णांक ul_jsonwrt_is_ready(संरचना ul_jsonwrt *fmt)
{
	परत fmt->out == रिक्त ? 0 : 1;
}

शून्य ul_jsonwrt_indent(संरचना ul_jsonwrt *fmt)
{
	पूर्णांक i;

	साठी (i = 0; i < fmt->indent; i++)
		ख_माला_दो("   ", fmt->out);
}

स्थिर शून्य print_name(संरचना ul_jsonwrt *fmt, स्थिरांक अक्षर *name)
{
	/* Values on the top level are not members of an array or object, so
	 * they are never comma separated; ul_jsonwrt_close() has already
	 * terminated the previous one by a newline. */
	जर (fmt->after_close && (fmt->indent > 0 || name)) {
		जर (fmt->json_format == UL_JSON_ओळ
		    || fmt->json_format == UL_JSON_COMPACT)
			ख_माला_दो(",", fmt->out);
		अन्यथा
			ख_माला_दो(name ? ",\n" : ",", fmt->out);
	}

	निवड (fmt->json_format) {
	स्थिती UL_JSON_ओळ:
	स्थिती UL_JSON_COMPACT:
		थांबा;
	स्थिती UL_JSON_PRETTY:
	पूर्वनिर्धारित:
		जर (name || !fmt->after_close)
			ul_jsonwrt_indent(fmt);
		थांबा;
	}
	जर (name)
		fputs_quoted_json_lower(name, fmt->out);
}

शून्य ul_jsonwrt_open(संरचना ul_jsonwrt *fmt, स्थिरांक अक्षर *name, पूर्णांक type)
{
	स्थिरांक अक्षर *s = "";

	print_name(fmt, name);

	निवड (type) {
	स्थिती UL_JSON_OBJECT:
		जर (fmt->json_format == UL_JSON_ओळ)
			s = "{";
		अन्यथा जर (fmt->json_format == UL_JSON_COMPACT) {
			/* At nesting depth 2, put the object opens on a new line
			 * for readability; otherwise keep fully compact (no newline). */
			जर (fmt->indent == 2)
				s = name ? ":{" : "\n{";
			अन्यथा
				s = "{";
		} अन्यथा
			s = name ? ": {\n" : "{\n";
		fmt->indent++;
		थांबा;
	स्थिती UL_JSON_ARRAY:
		जर (fmt->json_format == UL_JSON_ओळ)
			s = "[";
		अन्यथा जर (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":[" : "[";
		अन्यथा
			s = name ? ": [\n" : "[\n";
		fmt->indent++;
		थांबा;
	स्थिती UL_JSON_मूल्य:
		जर (fmt->json_format == UL_JSON_ओळ)
			s = name ? ":" : "";
		अन्यथा जर (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":" : "";
		अन्यथा
			s = name ? ": " : " ";
		थांबा;
	}
	ख_माला_दो(s, fmt->out);
	fmt->after_close = 0;
}

शून्य ul_jsonwrt_empty(संरचना ul_jsonwrt *fmt, स्थिरांक अक्षर *name, पूर्णांक type)
{
	स्थिरांक अक्षर *s = "";

	print_name(fmt, name);

	निवड (type) {
	स्थिती UL_JSON_OBJECT:
		जर (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": {}" : "{}";
		अन्यथा
			s = name ? ":{}" : "{}";
		थांबा;
	स्थिती UL_JSON_ARRAY:
		जर (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": []" : "[]";
		अन्यथा
			s = name ? ":[]" : "[]";
		थांबा;
	स्थिती UL_JSON_मूल्य:
		जर (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": null" : "null";
		अन्यथा
			s = name ? ":null" : "null";
		थांबा;
	}
	ख_माला_दो(s, fmt->out);
	fmt->after_close = 1;
}

शून्य ul_jsonwrt_close(संरचना ul_jsonwrt *fmt, पूर्णांक type)
{
	अक्षर endchr;

	प्रतिज्ञा(fmt->indent > 0);

	निवड (type) {
	स्थिती UL_JSON_OBJECT:
		fmt->indent--;
		endchr = '}';
		थांबा;
	स्थिती UL_JSON_ARRAY:
		fmt->indent--;
		endchr = ']';
		थांबा;
	स्थिती UL_JSON_मूल्य:
		fmt->after_close = 1;
		परत;
	पूर्वनिर्धारित:
		fmt->after_close = 1;
		परत;
	}

	/* In COMPACT, when closing a direct child of root (indent==1),
	 * insert a newline so the closing bracket starts on its own line. */
	जर (fmt->json_format == UL_JSON_COMPACT && fmt->indent == 1)
		ख_अक्षर_दो('\n', fmt->out);

	निवड (fmt->json_format) {
	स्थिती UL_JSON_ओळ:
	स्थिती UL_JSON_COMPACT:
		थांबा;
	स्थिती UL_JSON_PRETTY:
	पूर्वनिर्धारित:
		ख_माला_दो("\n", fmt->out);
		ul_jsonwrt_indent(fmt);
		थांबा;
	}

	ख_अक्षर_दो(endchr, fmt->out);

	/* Terminate the value on the top level. The writer may be re-initialized
	 * between the values (for example libsmartcols does it for every
	 * scols_table_print_range() call), so the terminator has to be printed
	 * by the value itself rather than by the next one (see print_name()). */
	जर (fmt->indent == 0)
		ख_अक्षर_दो('\n', fmt->out);

	fmt->after_close = 1;
}


शून्य ul_jsonwrt_flush(संरचना ul_jsonwrt *fmt)
{
	ख_साफ(fmt->out);
}

शून्य ul_jsonwrt_मूल्य_raw(संरचना ul_jsonwrt *fmt,
			स्थिरांक अक्षर *name, स्थिरांक अक्षर *data)
{
	ul_jsonwrt_मूल्य_open(fmt, name);
	जर (data && *data)
		ख_माला_दो(data, fmt->out);
	अन्यथा
		ख_माला_दो("null", fmt->out);
	ul_jsonwrt_मूल्य_close(fmt);
}

शून्य ul_jsonwrt_मूल्य_s(संरचना ul_jsonwrt *fmt,
			स्थिरांक अक्षर *name, स्थिरांक अक्षर *data)
{
	ul_jsonwrt_मूल्य_open(fmt, name);
	जर (data && *data)
		fputs_quoted_json(data, fmt->out);
	अन्यथा
		ख_माला_दो("null", fmt->out);
	ul_jsonwrt_मूल्य_close(fmt);
}

शून्य ul_jsonwrt_मूल्य_s_sized(संरचना ul_jsonwrt *fmt,
			      स्थिरांक अक्षर *name, स्थिरांक अक्षर *data, माप_प्रकार size)
{
	ul_jsonwrt_मूल्य_open(fmt, name);
	जर (data && *data)
		fputs_quoted_case_json(data, fmt->out, 0, size);
	अन्यथा
		ख_माला_दो("null", fmt->out);
	ul_jsonwrt_मूल्य_close(fmt);
}

शून्य ul_jsonwrt_मूल्य_u64(संरचना ul_jsonwrt *fmt,
			स्थिरांक अक्षर *name, uint64_t data)
{
	ul_jsonwrt_मूल्य_open(fmt, name);
	ख_लिखो(fmt->out, "%"PRIu64, data);
	ul_jsonwrt_मूल्य_close(fmt);
}

शून्य ul_jsonwrt_मूल्य_double(संरचना ul_jsonwrt *fmt,
			स्थिरांक अक्षर *name, दीर्घ द्विगुण data)
{
	ul_jsonwrt_मूल्य_open(fmt, name);
	ख_लिखो(fmt->out, "%Lg", data);
	ul_jsonwrt_मूल्य_close(fmt);
}

शून्य ul_jsonwrt_मूल्य_boolean(संरचना ul_jsonwrt *fmt,
			स्थिरांक अक्षर *name, पूर्णांक data)
{
	ul_jsonwrt_मूल्य_open(fmt, name);
	ख_माला_दो(data ? "true" : "false", fmt->out);
	ul_jsonwrt_मूल्य_close(fmt);
}
