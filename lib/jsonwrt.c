/*
 * JSON output formatting functions.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#समावेश <stdio.h>
#समावेश <inttypes.h>
#समावेश <ctype.h>
#समावेश <cctype.h>

#समावेश "c.h"
#समावेश "jsonwrt.h"

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
अटल व्योम fputs_quoted_case_json(स्थिर अक्षर *तथ्य, खाता *out, पूर्णांक dir, माप_प्रकार आकार)
{
	स्थिर अक्षर *p;

	ख_अक्षर_दो('"', out);
	क्रम (p = तथ्य; p && *p && (!आकार || p < तथ्य + आकार); p++) {

		स्थिर अचिन्हित पूर्णांक c = (अचिन्हित पूर्णांक) *p;

		/* From http://www.json.org
		 *
		 * The double-quote and backslashes would break out a string or
		 * init an escape sequence if not escaped.
		 *
		 * Note that single-quotes and forward slashes, while they're
		 * in the JSON spec, don't break double-quoted strings.
		 */
		अगर (c == '"' || c == '\\') {
			ख_अक्षर_दो('\\', out);
			ख_अक्षर_दो(c, out);
			जारी;
		}

		/* All non-control characters OK; do the case swap as required. */
		अगर (c >= 0x20) {
			/*
			 * Don't use locale sensitive ctype.h functions for regular
			 * ASCII chars, because for example with Turkish locale
			 * (aka LANG=tr_TR.UTF-8) toupper('I') returns 'I'.
			 */
			अगर (c <= 127)
				ख_अक्षर_दो(dir ==  1 ? c_toupper(c) :
				      dir == -1 ? c_tolower(c) : *p, out);
			अन्यथा
				ख_अक्षर_दो(dir ==  1 ? बड़े(c) :
				      dir == -1 ? छोटे(c) : *p, out);
			जारी;
		}

		/* In addition, all chars under ' ' break Node's/V8/Chrome's, and
		 * Firefox's JSON.parse function
		 */
		चयन (c) {
			/* Handle short-hand cases to reduce output size.  C
			 * has most of the same stuff here, so if there's an
			 * "Escape for C" function somewhere in the STL, we
			 * should probably be using it.
			 */
			हाल '\b':
				ख_माला_दो("\\b", out);
				अवरोध;
			हाल '\t':
				ख_माला_दो("\\t", out);
				अवरोध;
			हाल '\n':
				ख_माला_दो("\\n", out);
				अवरोध;
			हाल '\f':
				ख_माला_दो("\\f", out);
				अवरोध;
			हाल '\r':
				ख_माला_दो("\\r", out);
				अवरोध;
			शेष:
				/* Other assorted control characters */
				ख_लिखो(out, "\\u00%02x", c);
				अवरोध;
		}
	}
	ख_अक्षर_दो('"', out);
}

#घोषणा fputs_quoted_json(_d, _o)       fputs_quoted_case_json(_d, _o, 0, 0)
#घोषणा fputs_quoted_json_upper(_d, _o) fputs_quoted_case_json(_d, _o, 1, 0)
#घोषणा fputs_quoted_json_lower(_d, _o) fputs_quoted_case_json(_d, _o, -1, 0)

व्योम ul_jsonwrt_init(काष्ठा ul_jsonwrt *fmt, खाता *out, पूर्णांक indent, क्रमागत ul_json_format json_format)
{
	fmt->out = out;
	fmt->indent = indent;
	fmt->json_format = json_format;
	fmt->after_close = 0;
}

पूर्णांक ul_jsonwrt_is_ready(काष्ठा ul_jsonwrt *fmt)
{
	वापस fmt->out == शून्य ? 0 : 1;
}

व्योम ul_jsonwrt_indent(काष्ठा ul_jsonwrt *fmt)
{
	पूर्णांक i;

	क्रम (i = 0; i < fmt->indent; i++)
		ख_माला_दो("   ", fmt->out);
}

अटल व्योम print_name(काष्ठा ul_jsonwrt *fmt, स्थिर अक्षर *name)
{
	/* Values on the top level are not members of an array or object, so
	 * they are never comma separated; ul_jsonwrt_close() has already
	 * terminated the previous one by a newline. */
	अगर (fmt->after_close && (fmt->indent > 0 || name)) {
		अगर (fmt->json_format == UL_JSON_पंक्ति
		    || fmt->json_format == UL_JSON_COMPACT)
			ख_माला_दो(",", fmt->out);
		अन्यथा
			ख_माला_दो(name ? ",\n" : ",", fmt->out);
	}

	चयन (fmt->json_format) {
	हाल UL_JSON_पंक्ति:
	हाल UL_JSON_COMPACT:
		अवरोध;
	हाल UL_JSON_PRETTY:
	शेष:
		अगर (name || !fmt->after_close)
			ul_jsonwrt_indent(fmt);
		अवरोध;
	}
	अगर (name)
		fputs_quoted_json_lower(name, fmt->out);
}

व्योम ul_jsonwrt_open(काष्ठा ul_jsonwrt *fmt, स्थिर अक्षर *name, पूर्णांक type)
{
	स्थिर अक्षर *s = "";

	print_name(fmt, name);

	चयन (type) {
	हाल UL_JSON_OBJECT:
		अगर (fmt->json_format == UL_JSON_पंक्ति)
			s = "{";
		अन्यथा अगर (fmt->json_format == UL_JSON_COMPACT) {
			/* At nesting depth 2, put the object opens on a new line
			 * for readability; otherwise keep fully compact (no newline). */
			अगर (fmt->indent == 2)
				s = name ? ":{" : "\n{";
			अन्यथा
				s = "{";
		} अन्यथा
			s = name ? ": {\n" : "{\n";
		fmt->indent++;
		अवरोध;
	हाल UL_JSON_ARRAY:
		अगर (fmt->json_format == UL_JSON_पंक्ति)
			s = "[";
		अन्यथा अगर (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":[" : "[";
		अन्यथा
			s = name ? ": [\n" : "[\n";
		fmt->indent++;
		अवरोध;
	हाल UL_JSON_मूल्य:
		अगर (fmt->json_format == UL_JSON_पंक्ति)
			s = name ? ":" : "";
		अन्यथा अगर (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":" : "";
		अन्यथा
			s = name ? ": " : " ";
		अवरोध;
	}
	ख_माला_दो(s, fmt->out);
	fmt->after_close = 0;
}

व्योम ul_jsonwrt_empty(काष्ठा ul_jsonwrt *fmt, स्थिर अक्षर *name, पूर्णांक type)
{
	स्थिर अक्षर *s = "";

	print_name(fmt, name);

	चयन (type) {
	हाल UL_JSON_OBJECT:
		अगर (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": {}" : "{}";
		अन्यथा
			s = name ? ":{}" : "{}";
		अवरोध;
	हाल UL_JSON_ARRAY:
		अगर (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": []" : "[]";
		अन्यथा
			s = name ? ":[]" : "[]";
		अवरोध;
	हाल UL_JSON_मूल्य:
		अगर (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": null" : "null";
		अन्यथा
			s = name ? ":null" : "null";
		अवरोध;
	}
	ख_माला_दो(s, fmt->out);
	fmt->after_close = 1;
}

व्योम ul_jsonwrt_close(काष्ठा ul_jsonwrt *fmt, पूर्णांक type)
{
	अक्षर endchr;

	निश्चित(fmt->indent > 0);

	चयन (type) {
	हाल UL_JSON_OBJECT:
		fmt->indent--;
		endchr = '}';
		अवरोध;
	हाल UL_JSON_ARRAY:
		fmt->indent--;
		endchr = ']';
		अवरोध;
	हाल UL_JSON_मूल्य:
		fmt->after_close = 1;
		वापस;
	शेष:
		fmt->after_close = 1;
		वापस;
	}

	/* In COMPACT, when closing a direct child of root (indent==1),
	 * insert a newline so the closing bracket starts on its own line. */
	अगर (fmt->json_format == UL_JSON_COMPACT && fmt->indent == 1)
		ख_अक्षर_दो('\n', fmt->out);

	चयन (fmt->json_format) {
	हाल UL_JSON_पंक्ति:
	हाल UL_JSON_COMPACT:
		अवरोध;
	हाल UL_JSON_PRETTY:
	शेष:
		ख_माला_दो("\n", fmt->out);
		ul_jsonwrt_indent(fmt);
		अवरोध;
	}

	ख_अक्षर_दो(endchr, fmt->out);

	/* Terminate the value on the top level. The writer may be re-initialized
	 * between the values (for example libsmartcols does it for every
	 * scols_table_print_range() call), so the terminator has to be printed
	 * by the value itself rather than by the next one (see print_name()). */
	अगर (fmt->indent == 0)
		ख_अक्षर_दो('\n', fmt->out);

	fmt->after_close = 1;
}


व्योम ul_jsonwrt_flush(काष्ठा ul_jsonwrt *fmt)
{
	ख_साफ(fmt->out);
}

व्योम ul_jsonwrt_मूल्य_raw(काष्ठा ul_jsonwrt *fmt,
			स्थिर अक्षर *name, स्थिर अक्षर *तथ्य)
{
	ul_jsonwrt_मूल्य_open(fmt, name);
	अगर (तथ्य && *तथ्य)
		ख_माला_दो(तथ्य, fmt->out);
	अन्यथा
		ख_माला_दो("null", fmt->out);
	ul_jsonwrt_मूल्य_close(fmt);
}

व्योम ul_jsonwrt_मूल्य_s(काष्ठा ul_jsonwrt *fmt,
			स्थिर अक्षर *name, स्थिर अक्षर *तथ्य)
{
	ul_jsonwrt_मूल्य_open(fmt, name);
	अगर (तथ्य && *तथ्य)
		fputs_quoted_json(तथ्य, fmt->out);
	अन्यथा
		ख_माला_दो("null", fmt->out);
	ul_jsonwrt_मूल्य_close(fmt);
}

व्योम ul_jsonwrt_मूल्य_s_sized(काष्ठा ul_jsonwrt *fmt,
			      स्थिर अक्षर *name, स्थिर अक्षर *तथ्य, माप_प्रकार आकार)
{
	ul_jsonwrt_मूल्य_open(fmt, name);
	अगर (तथ्य && *तथ्य)
		fputs_quoted_case_json(तथ्य, fmt->out, 0, आकार);
	अन्यथा
		ख_माला_दो("null", fmt->out);
	ul_jsonwrt_मूल्य_close(fmt);
}

व्योम ul_jsonwrt_मूल्य_u64(काष्ठा ul_jsonwrt *fmt,
			स्थिर अक्षर *name, uint64_t तथ्य)
{
	ul_jsonwrt_मूल्य_open(fmt, name);
	ख_लिखो(fmt->out, "%"PRIu64, तथ्य);
	ul_jsonwrt_मूल्य_close(fmt);
}

व्योम ul_jsonwrt_मूल्य_double(काष्ठा ul_jsonwrt *fmt,
			स्थिर अक्षर *name, दीर्घ द्विगुन तथ्य)
{
	ul_jsonwrt_मूल्य_open(fmt, name);
	ख_लिखो(fmt->out, "%Lg", तथ्य);
	ul_jsonwrt_मूल्य_close(fmt);
}

व्योम ul_jsonwrt_मूल्य_boolean(काष्ठा ul_jsonwrt *fmt,
			स्थिर अक्षर *name, पूर्णांक तथ्य)
{
	ul_jsonwrt_मूल्य_open(fmt, name);
	ख_माला_दो(तथ्य ? "true" : "false", fmt->out);
	ul_jsonwrt_मूल्य_close(fmt);
}
