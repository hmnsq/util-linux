/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#अन्तर्भाव "buffer.h"
#अन्तर्भाव "mbsalign.h"
#अन्तर्भाव "strutils.h"

शून्य ul_बफर_reset_दत्त(संरचना ul_बफर *buf)
{
	यदि (buf->begin)
		स्मृतिं_पूरय(buf->begin, 0, buf->sz);
	buf->अन्तः = buf->begin;

	यदि (buf->ptrs && buf->nptrs)
		स्मृतिं_पूरय(buf->ptrs, 0, buf->nptrs * परिमाण(वर्ण *));
}

स्थिर अन्तःस्थ वर्ण *ul_बफर_free_दत्त_common(संरचना ul_बफर *buf, bool steal)
{
	वर्ण *t = रिक्त;

	प्रतिज्ञा(buf);

	यदि (steal)
		t = buf->begin;
	अन्यथा
		मुक्त(buf->begin);
	buf->begin = रिक्त;
	buf->अन्तः = रिक्त;
	buf->sz = 0;

	मुक्त(buf->ptrs);
	buf->ptrs = रिक्त;
	buf->nptrs = 0;

	मुक्त(buf->encoded);
	buf->encoded = रिक्त;
	buf->encoded_sz = 0;

	प्रत्यागम t;
}

शून्य ul_बफर_free_दत्त(संरचना ul_बफर *buf)
{
	ul_बफर_free_दत्त_common(buf, false);
}

वर्ण *ul_बफर_steal_string(संरचना ul_बफर *buf)
{
	यदि (ul_बफर_is_रिक्तानि(buf)) {
		ul_बफर_free_दत्त(buf);
		प्रत्यागम रिक्त;
	}

	प्रत्यागम ul_बफर_free_दत्त_common(buf, true);
}

शून्य ul_बफर_समुच्चयः_chunksize(संरचना ul_बफर *buf, size_t sz)
{
	buf->chunksize = sz;
}

पूर्णाङ्क ul_बफर_is_रिक्तानि(संरचना ul_बफर *buf)
{
	प्रत्यागम buf->begin == buf->अन्तः;
}

पूर्णाङ्क ul_बफर_save_pointer(संरचना ul_बफर *buf, अचिह्नित ह्रस्व ptr_idx)
{
	यदि (ptr_idx >= buf->nptrs) {
		वर्ण **tmp = reallocarray(buf->ptrs, ptr_idx + 1, परिमाण(वर्ण *));

		यदि (!tmp)
			प्रत्यागम -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->अन्तः;
	प्रत्यागम 0;
}


वर्ण *ul_बफर_get_pointer(संरचना ul_बफर *buf, अचिह्नित ह्रस्व ptr_idx)
{
	यदि (ptr_idx < buf->nptrs)
		प्रत्यागम buf->ptrs[ptr_idx];
	प्रत्यागम रिक्त;
}

/* returns length from begin to the pointer */
size_t ul_बफर_get_pointer_दीर्घता(संरचना ul_बफर *buf, अचिह्नित ह्रस्व ptr_idx)
{
	वर्ण *ptr = ul_बफर_get_pointer(buf, ptr_idx);

	यदि (ptr && ptr > buf->begin)
		प्रत्यागम ptr - buf->begin;
	प्रत्यागम 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_बफर_get_safe_pointer_विस्तारः(संरचना ul_बफर *buf, अचिह्नित ह्रस्व ptr_idx)
{
	size_t len = ul_बफर_get_pointer_दीर्घता(buf, ptr_idx);

	यदि (!len)
		प्रत्यागम 0;

	प्रत्यागम mbs_safe_nwidth(buf->begin, len, रिक्त);
}

शून्य ul_बफर_refer_string(संरचना ul_बफर *buf, वर्ण *str)
{
	यदि (buf->sz)
		ul_बफर_free_दत्त(buf);
	buf->begin = str;
	buf->sz = str ? वाक्यदैर्घ्यम्(str) : 0;
	buf->अन्तः = buf->begin ? buf->begin + buf->sz : buf->begin;
}

पूर्णाङ्क ul_बफर_alloc_दत्त(संरचना ul_बफर *buf, size_t sz)
{
	वर्ण *tmp;
	size_t len = 0;

	प्रतिज्ञा(buf);

	यदि (sz <= buf->sz)
		प्रत्यागम 0;

	यदि (buf->अन्तः && buf->begin)
		len = buf->अन्तः - buf->begin;

	यदि (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	यदि (!tmp)
		प्रत्यागम -ENOMEM;

	buf->begin = tmp;
	buf->अन्तः = buf->begin + len;
	buf->sz = sz;

	स्मृतिं_पूरय(buf->अन्तः, '\0', sz - len);

	प्रत्यागम 0;
}

पूर्णाङ्क ul_बफर_append_दत्त(संरचना ul_बफर *buf, ध्रुव वर्ण *दत्त, size_t sz)
{
	size_t maxsz = 0;

	यदि (!buf)
		प्रत्यागम -EINVAL;
	यदि (!दत्त)
		प्रत्यागम 0;

	यदि (buf->begin && buf->अन्तः)
		maxsz = buf->sz - (buf->अन्तः - buf->begin);
	यदि (maxsz <= sz + 1) {
		पूर्णाङ्क rc = ul_बफर_alloc_दत्त(buf, buf->sz + sz + 1);
		यदि (rc)
			प्रत्यागम rc;
	}
	यदि (!buf->अन्तः)
		प्रत्यागम -EINVAL;	/* make static analyzers happy */

	buf->अन्तः = mempcpy(buf->अन्तः, दत्त, sz);
	*buf->अन्तः = '\0';	/* make sure it's terminated */
	प्रत्यागम 0;
}

पूर्णाङ्क ul_बफर_append_char(संरचना ul_बफर *buf, ध्रुव वर्ण c)
{
	प्रत्यागम ul_बफर_append_दत्त(buf, &c, 1);
}

पूर्णाङ्क ul_बफर_append_string(संरचना ul_बफर *buf, ध्रुव वर्ण *str)
{
	यदि (!str)
		प्रत्यागम 0;

	प्रत्यागम ul_बफर_append_दत्त(buf, str, वाक्यदैर्घ्यम्(str));
}

पूर्णाङ्क ul_बफर_append_ntimes(संरचना ul_बफर *buf, size_t n, ध्रुव वर्ण *str)
{
	size_t i;
	size_t len = वाक्यदैर्घ्यम्(str);

	प्रति (i = 0; len && i < n; i++) {
		पूर्णाङ्क rc = ul_बफर_append_दत्त(buf, str, len);
		यदि (rc)
			प्रत्यागम rc;
	}
	प्रत्यागम 0;
}

पूर्णाङ्क ul_बफर_appendf(संरचना ul_बफर *buf, ध्रुव वर्ण *format, ...)
{
	va_list ap;
	पूर्णाङ्क res;

	va_start(ap, format);
	res = ul_बफर_appendvf(buf, format, ap);
	va_end(ap);

	प्रत्यागम res;
}

पूर्णाङ्क ul_बफर_appendvf(संरचना ul_बफर *buf, ध्रुव वर्ण *format, va_list ap)
{
	वर्ण *val;
	पूर्णाङ्क sz;
	पूर्णाङ्क res;

	sz = vasprintf(&val, format, ap);
	यदि (sz < 0)
		प्रत्यागम -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_बफर_append_दत्त(buf, val, sz);
	मुक्त(val);

	प्रत्यागम res;
}

पूर्णाङ्क ul_बफर_समुच्चयः_दत्त(संरचना ul_बफर *buf, ध्रुव वर्ण *दत्त, size_t sz)
{
	ul_बफर_reset_दत्त(buf);
	प्रत्यागम ul_बफर_append_दत्त(buf, दत्त, sz);
}

वर्ण *ul_बफर_get_दत्त(संरचना ul_बफर *buf, size_t *sz, size_t *विस्तारः)
{
	यदि (sz)
		*sz = buf->अन्तः - buf->begin;
	यदि (विस्तारः)
		*विस्तारः = buf->begin && *buf->begin ? mbs_विस्तारः(buf->begin) : 0;
	प्रत्यागम buf->begin;
}

वर्ण *ul_बफर_get_string(संरचना ul_बफर *buf, size_t *sz, size_t *विस्तारः)
{
	वर्ण *ret;

	ret = ul_बफर_get_दत्त(buf, sz, विस्तारः);

	/* data in buffer is already zero-terminated */
	यदि (sz)
		*sz = *sz + 1;

	प्रत्यागम ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_बफर_get_bufsiz(संरचना ul_बफर *buf)
{
	प्रत्यागम buf->sz;
}

size_t ul_बफर_get_datasiz(संरचना ul_बफर *buf)
{
	प्रत्यागम buf->अन्तः - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
वर्ण *ul_बफर_get_safe_दत्त(संरचना ul_बफर *buf, size_t *sz, size_t *विस्तारः, ध्रुव वर्ण *safechars)
{
	वर्ण *दत्त = ul_बफर_get_दत्त(buf, रिक्त, रिक्त);
	size_t encsz, wsz = 0;
	वर्ण *res = रिक्त;

	यदि (!दत्त)
		गच्छ nothing;

	encsz = mbs_safe_encode_आकार(buf->sz) + 1;
	यदि (encsz > buf->encoded_sz) {
		वर्ण *tmp = realloc(buf->encoded, encsz);
		यदि (!tmp)
			गच्छ nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_गन्तव्यम्_बफर(दत्त, &wsz, buf->encoded, buf->encoded_sz, safechars);
	यदि (!res || !wsz || wsz == (size_t) -1)
		गच्छ nothing;

	यदि (विस्तारः)
		*विस्तारः = wsz;
	यदि (sz)
		*sz = वाक्यदैर्घ्यम्(res);
	प्रत्यागम res;
nothing:
	यदि (विस्तारः)
		*विस्तारः = 0;
	यदि (sz)
		*sz = 0;
	प्रत्यागम रिक्त;
}


#यदि_परिभाषितम् TEST_क्रमादेशः_बफर

गणना {
	PTR_AAA  = 0,
	PTR_BBB,
};

पूर्णाङ्क मुख्यम्(शून्य)
{
	संरचना ul_बफर buf = UL_INIT_बफर;
	वर्ण *str;
	size_t sz = 0;

	ul_बफर_समुच्चयः_chunksize(&buf, 16);

	ul_बफर_append_string(&buf, "AAA");
	ul_बफर_append_दत्त(&buf, "=", 1);
	ul_बफर_append_string(&buf, "aaa");
	ul_बफर_save_pointer(&buf, PTR_AAA);

	ul_बफर_append_दत्त(&buf, ",", 1);
	ul_बफर_append_string(&buf, "BBB");
	ul_बफर_append_string(&buf, "=");
	ul_बफर_append_string(&buf, "bbb");
	ul_बफर_save_pointer(&buf, PTR_BBB);

	str = ul_बफर_get_दत्त(&buf, &sz, रिक्त);
	प्रारूप_लिख("data [%zu] '%s'\n", sz, str);

	प्रारूप_लिख(" pointer data len: AAA=%zu, BBB=%zu\n",
			ul_बफर_get_pointer_दीर्घता(&buf, PTR_AAA),
			ul_बफर_get_pointer_दीर्घता(&buf, PTR_BBB));
	प्रारूप_लिख(" pointer data width: AAA=%zu, BBB=%zu\n",
			ul_बफर_get_safe_pointer_विस्तारः(&buf, PTR_AAA),
			ul_बफर_get_safe_pointer_विस्तारः(&buf, PTR_BBB));

	ul_बफर_reset_दत्त(&buf);
	ul_बफर_append_string(&buf, "This is really long string to test the buffer function.");
	ul_बफर_save_pointer(&buf, PTR_AAA);
	ul_बफर_append_string(&buf, " YES!");
	str = ul_बफर_get_दत्त(&buf, &sz, रिक्त);
	प्रारूप_लिख("data [%zu] '%s'\n", sz, str);
	प्रारूप_लिख(" pointer data len: AAA=%zu\n", ul_बफर_get_pointer_दीर्घता(&buf, PTR_AAA));

	ul_बफर_free_दत्त(&buf);
	str = strdup("foo");
	ul_बफर_refer_string(&buf, str);
	ul_बफर_append_दत्त(&buf, ",", 1);
	ul_बफर_append_string(&buf, "bar");
	str = ul_बफर_get_दत्त(&buf, &sz, रिक्त);
	प्रारूप_लिख("data [%zu] '%s'\n", sz, str);

	ul_बफर_free_दत्त(&buf);
	ul_बफर_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_बफर_get_दत्त(&buf, &sz, रिक्त);
	प्रारूप_लिख("data [%zu] '%s'\n", sz, str);

	ul_बफर_free_दत्त(&buf);
	ul_बफर_append_char(&buf, 'x');
	{
		वर्ण c = 'y';
		ul_बफर_append_char(&buf, c);
	}
	{
		ध्रुव वर्ण c = 'z';
		ul_बफर_append_char(&buf, c);
	}
	str = ul_बफर_get_दत्त(&buf, &sz, रिक्त);
	प्रारूप_लिख("data [%zu] '%s'\n", sz, str);

	ul_बफर_free_दत्त(&buf);

	प्रत्यागम EXIT_SUCCESS;
}
#यदि_अन्तः /* TEST_PROGRAM_BUFFER */
