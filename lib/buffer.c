/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#అన్తర్భావ "buffer.h"
#అన్తర్భావ "mbsalign.h"
#అన్తర్భావ "strutils.h"

శూన్య ul_బఫరు_reset_దత్తాంశం(నిర్మాణం ul_బఫరు *buf)
{
	ఒకవేళ (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->చివర = buf->begin;

	ఒకవేళ (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * పరిమాణం(అక్షరం *));
}

స్థిర అంతఃవరుస అక్షరం *ul_బఫరు_free_దత్తాంశం_common(నిర్మాణం ul_బఫరు *buf, bool steal)
{
	అక్షరం *t = ఖాళీ;

	ప్రతిజ్ఞా(buf);

	ఒకవేళ (steal)
		t = buf->begin;
	లేకపోతే
		విడుదల(buf->begin);
	buf->begin = ఖాళీ;
	buf->చివర = ఖాళీ;
	buf->sz = 0;

	విడుదల(buf->ptrs);
	buf->ptrs = ఖాళీ;
	buf->nptrs = 0;

	విడుదల(buf->encoded);
	buf->encoded = ఖాళీ;
	buf->encoded_sz = 0;

	తిరిగి t;
}

శూన్య ul_బఫరు_free_దత్తాంశం(నిర్మాణం ul_బఫరు *buf)
{
	ul_బఫరు_free_దత్తాంశం_common(buf, false);
}

అక్షరం *ul_బఫరు_steal_string(నిర్మాణం ul_బఫరు *buf)
{
	ఒకవేళ (ul_బఫరు_is_ఖాళీలు(buf)) {
		ul_బఫరు_free_దత్తాంశం(buf);
		తిరిగి ఖాళీ;
	}

	తిరిగి ul_బఫరు_free_దత్తాంశం_common(buf, true);
}

శూన్య ul_బఫరు_సమితి_chunksize(నిర్మాణం ul_బఫరు *buf, size_t sz)
{
	buf->chunksize = sz;
}

పూర్ణాంకం ul_బఫరు_is_ఖాళీలు(నిర్మాణం ul_బఫరు *buf)
{
	తిరిగి buf->begin == buf->చివర;
}

పూర్ణాంకం ul_బఫరు_save_సూచిక(నిర్మాణం ul_బఫరు *buf, గుర్తులేని పొట్టి ptr_idx)
{
	ఒకవేళ (ptr_idx >= buf->nptrs) {
		అక్షరం **tmp = reallocarray(buf->ptrs, ptr_idx + 1, పరిమాణం(అక్షరం *));

		ఒకవేళ (!tmp)
			తిరిగి -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->చివర;
	తిరిగి 0;
}


అక్షరం *ul_బఫరు_get_సూచిక(నిర్మాణం ul_బఫరు *buf, గుర్తులేని పొట్టి ptr_idx)
{
	ఒకవేళ (ptr_idx < buf->nptrs)
		తిరిగి buf->ptrs[ptr_idx];
	తిరిగి ఖాళీ;
}

/* returns length from begin to the pointer */
size_t ul_బఫరు_get_సూచిక_నిడివి(నిర్మాణం ul_బఫరు *buf, గుర్తులేని పొట్టి ptr_idx)
{
	అక్షరం *ptr = ul_బఫరు_get_సూచిక(buf, ptr_idx);

	ఒకవేళ (ptr && ptr > buf->begin)
		తిరిగి ptr - buf->begin;
	తిరిగి 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_బఫరు_get_సురక్షిత_సూచిక_వెడల్పు(నిర్మాణం ul_బఫరు *buf, గుర్తులేని పొట్టి ptr_idx)
{
	size_t len = ul_బఫరు_get_సూచిక_నిడివి(buf, ptr_idx);

	ఒకవేళ (!len)
		తిరిగి 0;

	తిరిగి mbs_సురక్షిత_nwidth(buf->begin, len, ఖాళీ);
}

శూన్య ul_బఫరు_refer_string(నిర్మాణం ul_బఫరు *buf, అక్షరం *str)
{
	ఒకవేళ (buf->sz)
		ul_బఫరు_free_దత్తాంశం(buf);
	buf->begin = str;
	buf->sz = str ? అక్షరశ్రేణి_పొడవు(str) : 0;
	buf->చివర = buf->begin ? buf->begin + buf->sz : buf->begin;
}

పూర్ణాంకం ul_బఫరు_alloc_దత్తాంశం(నిర్మాణం ul_బఫరు *buf, size_t sz)
{
	అక్షరం *tmp;
	size_t len = 0;

	ప్రతిజ్ఞా(buf);

	ఒకవేళ (sz <= buf->sz)
		తిరిగి 0;

	ఒకవేళ (buf->చివర && buf->begin)
		len = buf->చివర - buf->begin;

	ఒకవేళ (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	ఒకవేళ (!tmp)
		తిరిగి -ENOMEM;

	buf->begin = tmp;
	buf->చివర = buf->begin + len;
	buf->sz = sz;

	memset(buf->చివర, '\0', sz - len);

	తిరిగి 0;
}

పూర్ణాంకం ul_బఫరు_append_దత్తాంశం(నిర్మాణం ul_బఫరు *buf, స్థిరరాశి అక్షరం *దత్తాంశం, size_t sz)
{
	size_t maxsz = 0;

	ఒకవేళ (!buf)
		తిరిగి -EINVAL;
	ఒకవేళ (!దత్తాంశం)
		తిరిగి 0;

	ఒకవేళ (buf->begin && buf->చివర)
		maxsz = buf->sz - (buf->చివర - buf->begin);
	ఒకవేళ (maxsz <= sz + 1) {
		పూర్ణాంకం rc = ul_బఫరు_alloc_దత్తాంశం(buf, buf->sz + sz + 1);
		ఒకవేళ (rc)
			తిరిగి rc;
	}
	ఒకవేళ (!buf->చివర)
		తిరిగి -EINVAL;	/* make static analyzers happy */

	buf->చివర = mempcpy(buf->చివర, దత్తాంశం, sz);
	*buf->చివర = '\0';	/* make sure it's terminated */
	తిరిగి 0;
}

పూర్ణాంకం ul_బఫరు_append_char(నిర్మాణం ul_బఫరు *buf, స్థిరరాశి అక్షరం c)
{
	తిరిగి ul_బఫరు_append_దత్తాంశం(buf, &c, 1);
}

పూర్ణాంకం ul_బఫరు_append_string(నిర్మాణం ul_బఫరు *buf, స్థిరరాశి అక్షరం *str)
{
	ఒకవేళ (!str)
		తిరిగి 0;

	తిరిగి ul_బఫరు_append_దత్తాంశం(buf, str, అక్షరశ్రేణి_పొడవు(str));
}

పూర్ణాంకం ul_బఫరు_append_ntimes(నిర్మాణం ul_బఫరు *buf, size_t n, స్థిరరాశి అక్షరం *str)
{
	size_t i;
	size_t len = అక్షరశ్రేణి_పొడవు(str);

	కొరకు (i = 0; len && i < n; i++) {
		పూర్ణాంకం rc = ul_బఫరు_append_దత్తాంశం(buf, str, len);
		ఒకవేళ (rc)
			తిరిగి rc;
	}
	తిరిగి 0;
}

పూర్ణాంకం ul_బఫరు_appendf(నిర్మాణం ul_బఫరు *buf, స్థిరరాశి అక్షరం *format, ...)
{
	va_list ap;
	పూర్ణాంకం res;

	va_start(ap, format);
	res = ul_బఫరు_appendvf(buf, format, ap);
	va_end(ap);

	తిరిగి res;
}

పూర్ణాంకం ul_బఫరు_appendvf(నిర్మాణం ul_బఫరు *buf, స్థిరరాశి అక్షరం *format, va_list ap)
{
	అక్షరం *val;
	పూర్ణాంకం sz;
	పూర్ణాంకం res;

	sz = vasprintf(&val, format, ap);
	ఒకవేళ (sz < 0)
		తిరిగి -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_బఫరు_append_దత్తాంశం(buf, val, sz);
	విడుదల(val);

	తిరిగి res;
}

పూర్ణాంకం ul_బఫరు_సమితి_దత్తాంశం(నిర్మాణం ul_బఫరు *buf, స్థిరరాశి అక్షరం *దత్తాంశం, size_t sz)
{
	ul_బఫరు_reset_దత్తాంశం(buf);
	తిరిగి ul_బఫరు_append_దత్తాంశం(buf, దత్తాంశం, sz);
}

అక్షరం *ul_బఫరు_get_దత్తాంశం(నిర్మాణం ul_బఫరు *buf, size_t *sz, size_t *వెడల్పు)
{
	ఒకవేళ (sz)
		*sz = buf->చివర - buf->begin;
	ఒకవేళ (వెడల్పు)
		*వెడల్పు = buf->begin && *buf->begin ? mbs_వెడల్పు(buf->begin) : 0;
	తిరిగి buf->begin;
}

అక్షరం *ul_బఫరు_get_string(నిర్మాణం ul_బఫరు *buf, size_t *sz, size_t *వెడల్పు)
{
	అక్షరం *ret;

	ret = ul_బఫరు_get_దత్తాంశం(buf, sz, వెడల్పు);

	/* data in buffer is already zero-terminated */
	ఒకవేళ (sz)
		*sz = *sz + 1;

	తిరిగి ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_బఫరు_get_bufsiz(నిర్మాణం ul_బఫరు *buf)
{
	తిరిగి buf->sz;
}

size_t ul_బఫరు_get_datasiz(నిర్మాణం ul_బఫరు *buf)
{
	తిరిగి buf->చివర - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
అక్షరం *ul_బఫరు_get_సురక్షిత_దత్తాంశం(నిర్మాణం ul_బఫరు *buf, size_t *sz, size_t *వెడల్పు, స్థిరరాశి అక్షరం *safechars)
{
	అక్షరం *దత్తాంశం = ul_బఫరు_get_దత్తాంశం(buf, ఖాళీ, ఖాళీ);
	size_t encsz, wsz = 0;
	అక్షరం *res = ఖాళీ;

	ఒకవేళ (!దత్తాంశం)
		వెళ్ళు nothing;

	encsz = mbs_సురక్షిత_encode_కొలత(buf->sz) + 1;
	ఒకవేళ (encsz > buf->encoded_sz) {
		అక్షరం *tmp = realloc(buf->encoded, encsz);
		ఒకవేళ (!tmp)
			వెళ్ళు nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_సురక్షిత_encode_గమ్యం_బఫరు(దత్తాంశం, &wsz, buf->encoded, buf->encoded_sz, safechars);
	ఒకవేళ (!res || !wsz || wsz == (size_t) -1)
		వెళ్ళు nothing;

	ఒకవేళ (వెడల్పు)
		*వెడల్పు = wsz;
	ఒకవేళ (sz)
		*sz = అక్షరశ్రేణి_పొడవు(res);
	తిరిగి res;
nothing:
	ఒకవేళ (వెడల్పు)
		*వెడల్పు = 0;
	ఒకవేళ (sz)
		*sz = 0;
	తిరిగి ఖాళీ;
}


#నిర్వచితమైతే TEST_కార్యక్రమం_బఫరు

గణన {
	PTR_AAA  = 0,
	PTR_BBB,
};

పూర్ణాంకం ప్రధాన(శూన్య)
{
	నిర్మాణం ul_బఫరు buf = UL_INIT_బఫరు;
	అక్షరం *str;
	size_t sz = 0;

	ul_బఫరు_సమితి_chunksize(&buf, 16);

	ul_బఫరు_append_string(&buf, "AAA");
	ul_బఫరు_append_దత్తాంశం(&buf, "=", 1);
	ul_బఫరు_append_string(&buf, "aaa");
	ul_బఫరు_save_సూచిక(&buf, PTR_AAA);

	ul_బఫరు_append_దత్తాంశం(&buf, ",", 1);
	ul_బఫరు_append_string(&buf, "BBB");
	ul_బఫరు_append_string(&buf, "=");
	ul_బఫరు_append_string(&buf, "bbb");
	ul_బఫరు_save_సూచిక(&buf, PTR_BBB);

	str = ul_బఫరు_get_దత్తాంశం(&buf, &sz, ఖాళీ);
	రాయి("data [%zu] '%s'\n", sz, str);

	రాయి(" pointer data len: AAA=%zu, BBB=%zu\n",
			ul_బఫరు_get_సూచిక_నిడివి(&buf, PTR_AAA),
			ul_బఫరు_get_సూచిక_నిడివి(&buf, PTR_BBB));
	రాయి(" pointer data width: AAA=%zu, BBB=%zu\n",
			ul_బఫరు_get_సురక్షిత_సూచిక_వెడల్పు(&buf, PTR_AAA),
			ul_బఫరు_get_సురక్షిత_సూచిక_వెడల్పు(&buf, PTR_BBB));

	ul_బఫరు_reset_దత్తాంశం(&buf);
	ul_బఫరు_append_string(&buf, "This is really long string to test the buffer function.");
	ul_బఫరు_save_సూచిక(&buf, PTR_AAA);
	ul_బఫరు_append_string(&buf, " YES!");
	str = ul_బఫరు_get_దత్తాంశం(&buf, &sz, ఖాళీ);
	రాయి("data [%zu] '%s'\n", sz, str);
	రాయి(" pointer data len: AAA=%zu\n", ul_బఫరు_get_సూచిక_నిడివి(&buf, PTR_AAA));

	ul_బఫరు_free_దత్తాంశం(&buf);
	str = strdup("foo");
	ul_బఫరు_refer_string(&buf, str);
	ul_బఫరు_append_దత్తాంశం(&buf, ",", 1);
	ul_బఫరు_append_string(&buf, "bar");
	str = ul_బఫరు_get_దత్తాంశం(&buf, &sz, ఖాళీ);
	రాయి("data [%zu] '%s'\n", sz, str);

	ul_బఫరు_free_దత్తాంశం(&buf);
	ul_బఫరు_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_బఫరు_get_దత్తాంశం(&buf, &sz, ఖాళీ);
	రాయి("data [%zu] '%s'\n", sz, str);

	ul_బఫరు_free_దత్తాంశం(&buf);
	ul_బఫరు_append_char(&buf, 'x');
	{
		అక్షరం c = 'y';
		ul_బఫరు_append_char(&buf, c);
	}
	{
		స్థిరరాశి అక్షరం c = 'z';
		ul_బఫరు_append_char(&buf, c);
	}
	str = ul_బఫరు_get_దత్తాంశం(&buf, &sz, ఖాళీ);
	రాయి("data [%zu] '%s'\n", sz, str);

	ul_బఫరు_free_దత్తాంశం(&buf);

	తిరిగి EXIT_SUCCESS;
}
#అయితే_ముగింపు /* TEST_PROGRAM_BUFFER */
