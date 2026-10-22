/*
 * JSON output formatting functions.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <inttypes.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <cctype.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "jsonwrt.h"

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
స్థిర శూన్య fputs_quoted_case_json(స్థిరరాశి అక్షరం *దత్తాంశం, FILE *బహిర్గతం, పూర్ణాంకం dir, size_t కొలత)
{
	స్థిరరాశి అక్షరం *p;

	fputc('"', బహిర్గతం);
	కొరకు (p = దత్తాంశం; p && *p && (!కొలత || p < దత్తాంశం + కొలత); p++) {

		స్థిరరాశి గుర్తులేని పూర్ణాంకం c = (గుర్తులేని పూర్ణాంకం) *p;

		/* From http://www.json.org
		 *
		 * The double-quote and backslashes would break out a string or
		 * init an escape sequence if not escaped.
		 *
		 * Note that single-quotes and forward slashes, while they're
		 * in the JSON spec, don't break double-quoted strings.
		 */
		ఒకవేళ (c == '"' || c == '\\') {
			fputc('\\', బహిర్గతం);
			fputc(c, బహిర్గతం);
			కొనసాగు;
		}

		/* All non-control characters OK; do the case swap as required. */
		ఒకవేళ (c >= 0x20) {
			/*
			 * Don't use locale sensitive ctype.h functions for regular
			 * ASCII chars, because for example with Turkish locale
			 * (aka LANG=tr_TR.UTF-8) toupper('I') returns 'I'.
			 */
			ఒకవేళ (c <= 127)
				fputc(dir ==  1 ? c_toupper(c) :
				      dir == -1 ? c_tolower(c) : *p, బహిర్గతం);
			లేకపోతే
				fputc(dir ==  1 ? పెద్ద_అక్షరం(c) :
				      dir == -1 ? tolower(c) : *p, బహిర్గతం);
			కొనసాగు;
		}

		/* In addition, all chars under ' ' break Node's/V8/Chrome's, and
		 * Firefox's JSON.parse function
		 */
		ఎంపిక (c) {
			/* Handle short-hand cases to reduce output size.  C
			 * has most of the same stuff here, so if there's an
			 * "Escape for C" function somewhere in the STL, we
			 * should probably be using it.
			 */
			స్థితి '\b':
				fputs("\\b", బహిర్గతం);
				ఆపు;
			స్థితి '\t':
				fputs("\\t", బహిర్గతం);
				ఆపు;
			స్థితి '\n':
				fputs("\\n", బహిర్గతం);
				ఆపు;
			స్థితి '\f':
				fputs("\\f", బహిర్గతం);
				ఆపు;
			స్థితి '\r':
				fputs("\\r", బహిర్గతం);
				ఆపు;
			అప్రమేయ:
				/* Other assorted control characters */
				fprintf(బహిర్గతం, "\\u00%02x", c);
				ఆపు;
		}
	}
	fputc('"', బహిర్గతం);
}

#నిర్వచించు fputs_quoted_json(_d, _o)       fputs_quoted_case_json(_d, _o, 0, 0)
#నిర్వచించు fputs_quoted_json_పై(_d, _o) fputs_quoted_case_json(_d, _o, 1, 0)
#నిర్వచించు fputs_quoted_json_క్రింది(_d, _o) fputs_quoted_case_json(_d, _o, -1, 0)

శూన్య ul_jsonwrt_init(నిర్మాణం ul_jsonwrt *fmt, FILE *బహిర్గతం, పూర్ణాంకం indent, గణన ul_json_format json_format)
{
	fmt->బహిర్గతం = బహిర్గతం;
	fmt->indent = indent;
	fmt->json_format = json_format;
	fmt->after_close = 0;
}

పూర్ణాంకం ul_jsonwrt_is_ready(నిర్మాణం ul_jsonwrt *fmt)
{
	తిరిగి fmt->బహిర్గతం == ఖాళీ ? 0 : 1;
}

శూన్య ul_jsonwrt_indent(నిర్మాణం ul_jsonwrt *fmt)
{
	పూర్ణాంకం i;

	కొరకు (i = 0; i < fmt->indent; i++)
		fputs("   ", fmt->బహిర్గతం);
}

స్థిర శూన్య print_name(నిర్మాణం ul_jsonwrt *fmt, స్థిరరాశి అక్షరం *name)
{
	/* Values on the top level are not members of an array or object, so
	 * they are never comma separated; ul_jsonwrt_close() has already
	 * terminated the previous one by a newline. */
	ఒకవేళ (fmt->after_close && (fmt->indent > 0 || name)) {
		ఒకవేళ (fmt->json_format == UL_JSON_పంక్తి
		    || fmt->json_format == UL_JSON_COMPACT)
			fputs(",", fmt->బహిర్గతం);
		లేకపోతే
			fputs(name ? ",\n" : ",", fmt->బహిర్గతం);
	}

	ఎంపిక (fmt->json_format) {
	స్థితి UL_JSON_పంక్తి:
	స్థితి UL_JSON_COMPACT:
		ఆపు;
	స్థితి UL_JSON_PRETTY:
	అప్రమేయ:
		ఒకవేళ (name || !fmt->after_close)
			ul_jsonwrt_indent(fmt);
		ఆపు;
	}
	ఒకవేళ (name)
		fputs_quoted_json_క్రింది(name, fmt->బహిర్గతం);
}

శూన్య ul_jsonwrt_open(నిర్మాణం ul_jsonwrt *fmt, స్థిరరాశి అక్షరం *name, పూర్ణాంకం type)
{
	స్థిరరాశి అక్షరం *s = "";

	print_name(fmt, name);

	ఎంపిక (type) {
	స్థితి UL_JSON_OBJECT:
		ఒకవేళ (fmt->json_format == UL_JSON_పంక్తి)
			s = "{";
		లేకపోతే ఒకవేళ (fmt->json_format == UL_JSON_COMPACT) {
			/* At nesting depth 2, put the object opens on a new line
			 * for readability; otherwise keep fully compact (no newline). */
			ఒకవేళ (fmt->indent == 2)
				s = name ? ":{" : "\n{";
			లేకపోతే
				s = "{";
		} లేకపోతే
			s = name ? ": {\n" : "{\n";
		fmt->indent++;
		ఆపు;
	స్థితి UL_JSON_ARRAY:
		ఒకవేళ (fmt->json_format == UL_JSON_పంక్తి)
			s = "[";
		లేకపోతే ఒకవేళ (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":[" : "[";
		లేకపోతే
			s = name ? ": [\n" : "[\n";
		fmt->indent++;
		ఆపు;
	స్థితి UL_JSON_విలువ:
		ఒకవేళ (fmt->json_format == UL_JSON_పంక్తి)
			s = name ? ":" : "";
		లేకపోతే ఒకవేళ (fmt->json_format == UL_JSON_COMPACT)
			s = name ? ":" : "";
		లేకపోతే
			s = name ? ": " : " ";
		ఆపు;
	}
	fputs(s, fmt->బహిర్గతం);
	fmt->after_close = 0;
}

శూన్య ul_jsonwrt_ఖాళీలు(నిర్మాణం ul_jsonwrt *fmt, స్థిరరాశి అక్షరం *name, పూర్ణాంకం type)
{
	స్థిరరాశి అక్షరం *s = "";

	print_name(fmt, name);

	ఎంపిక (type) {
	స్థితి UL_JSON_OBJECT:
		ఒకవేళ (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": {}" : "{}";
		లేకపోతే
			s = name ? ":{}" : "{}";
		ఆపు;
	స్థితి UL_JSON_ARRAY:
		ఒకవేళ (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": []" : "[]";
		లేకపోతే
			s = name ? ":[]" : "[]";
		ఆపు;
	స్థితి UL_JSON_విలువ:
		ఒకవేళ (fmt->json_format == UL_JSON_PRETTY)
			s = name ? ": null" : "null";
		లేకపోతే
			s = name ? ":null" : "null";
		ఆపు;
	}
	fputs(s, fmt->బహిర్గతం);
	fmt->after_close = 1;
}

శూన్య ul_jsonwrt_close(నిర్మాణం ul_jsonwrt *fmt, పూర్ణాంకం type)
{
	అక్షరం endchr;

	ప్రతిజ్ఞా(fmt->indent > 0);

	ఎంపిక (type) {
	స్థితి UL_JSON_OBJECT:
		fmt->indent--;
		endchr = '}';
		ఆపు;
	స్థితి UL_JSON_ARRAY:
		fmt->indent--;
		endchr = ']';
		ఆపు;
	స్థితి UL_JSON_విలువ:
		fmt->after_close = 1;
		తిరిగి;
	అప్రమేయ:
		fmt->after_close = 1;
		తిరిగి;
	}

	/* In COMPACT, when closing a direct child of root (indent==1),
	 * insert a newline so the closing bracket starts on its own line. */
	ఒకవేళ (fmt->json_format == UL_JSON_COMPACT && fmt->indent == 1)
		fputc('\n', fmt->బహిర్గతం);

	ఎంపిక (fmt->json_format) {
	స్థితి UL_JSON_పంక్తి:
	స్థితి UL_JSON_COMPACT:
		ఆపు;
	స్థితి UL_JSON_PRETTY:
	అప్రమేయ:
		fputs("\n", fmt->బహిర్గతం);
		ul_jsonwrt_indent(fmt);
		ఆపు;
	}

	fputc(endchr, fmt->బహిర్గతం);

	/* Terminate the value on the top level. The writer may be re-initialized
	 * between the values (for example libsmartcols does it for every
	 * scols_table_print_range() call), so the terminator has to be printed
	 * by the value itself rather than by the next one (see print_name()). */
	ఒకవేళ (fmt->indent == 0)
		fputc('\n', fmt->బహిర్గతం);

	fmt->after_close = 1;
}


శూన్య ul_jsonwrt_flush(నిర్మాణం ul_jsonwrt *fmt)
{
	fflush(fmt->బహిర్గతం);
}

శూన్య ul_jsonwrt_విలువ_raw(నిర్మాణం ul_jsonwrt *fmt,
			స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *దత్తాంశం)
{
	ul_jsonwrt_విలువ_open(fmt, name);
	ఒకవేళ (దత్తాంశం && *దత్తాంశం)
		fputs(దత్తాంశం, fmt->బహిర్గతం);
	లేకపోతే
		fputs("null", fmt->బహిర్గతం);
	ul_jsonwrt_విలువ_close(fmt);
}

శూన్య ul_jsonwrt_విలువ_s(నిర్మాణం ul_jsonwrt *fmt,
			స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *దత్తాంశం)
{
	ul_jsonwrt_విలువ_open(fmt, name);
	ఒకవేళ (దత్తాంశం && *దత్తాంశం)
		fputs_quoted_json(దత్తాంశం, fmt->బహిర్గతం);
	లేకపోతే
		fputs("null", fmt->బహిర్గతం);
	ul_jsonwrt_విలువ_close(fmt);
}

శూన్య ul_jsonwrt_విలువ_s_sized(నిర్మాణం ul_jsonwrt *fmt,
			      స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *దత్తాంశం, size_t కొలత)
{
	ul_jsonwrt_విలువ_open(fmt, name);
	ఒకవేళ (దత్తాంశం && *దత్తాంశం)
		fputs_quoted_case_json(దత్తాంశం, fmt->బహిర్గతం, 0, కొలత);
	లేకపోతే
		fputs("null", fmt->బహిర్గతం);
	ul_jsonwrt_విలువ_close(fmt);
}

శూన్య ul_jsonwrt_విలువ_u64(నిర్మాణం ul_jsonwrt *fmt,
			స్థిరరాశి అక్షరం *name, uint64_t దత్తాంశం)
{
	ul_jsonwrt_విలువ_open(fmt, name);
	fprintf(fmt->బహిర్గతం, "%"PRIu64, దత్తాంశం);
	ul_jsonwrt_విలువ_close(fmt);
}

శూన్య ul_jsonwrt_విలువ_double(నిర్మాణం ul_jsonwrt *fmt,
			స్థిరరాశి అక్షరం *name, దీర్ఘ ద్విగుణ దత్తాంశం)
{
	ul_jsonwrt_విలువ_open(fmt, name);
	fprintf(fmt->బహిర్గతం, "%Lg", దత్తాంశం);
	ul_jsonwrt_విలువ_close(fmt);
}

శూన్య ul_jsonwrt_విలువ_boolean(నిర్మాణం ul_jsonwrt *fmt,
			స్థిరరాశి అక్షరం *name, పూర్ణాంకం దత్తాంశం)
{
	ul_jsonwrt_విలువ_open(fmt, name);
	fputs(దత్తాంశం ? "true" : "false", fmt->బహిర్గతం);
	ul_jsonwrt_విలువ_close(fmt);
}
