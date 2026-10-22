/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#अन्तर्भाव "buffer.h"
#अन्तर्भाव "mbsalign.h"
#अन्तर्भाव "strutils.h"

शून्य ul_buffer_reset_data(संरचना ul_buffer *buf)
{
	जइ (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	जइ (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * परिमाण(वर्ण *));
}

स्थिर अन्तःस्थ वर्ण *ul_buffer_free_data_common(संरचना ul_buffer *buf, bool steal)
{
	वर्ण *t = रिक्त;

	प्रतिज्ञा(buf);

	जइ (steal)
		t = buf->begin;
	अण्णहा
		मुक्त(buf->begin);
	buf->begin = रिक्त;
	buf->end = रिक्त;
	buf->sz = 0;

	मुक्त(buf->ptrs);
	buf->ptrs = रिक्त;
	buf->nptrs = 0;

	मुक्त(buf->encoded);
	buf->encoded = रिक्त;
	buf->encoded_sz = 0;

	प्रत्यागम t;
}

शून्य ul_buffer_free_data(संरचना ul_buffer *buf)
{
	ul_buffer_free_data_common(buf, false);
}

वर्ण *ul_buffer_steal_string(संरचना ul_buffer *buf)
{
	जइ (ul_buffer_is_empty(buf)) {
		ul_buffer_free_data(buf);
		प्रत्यागम रिक्त;
	}

	प्रत्यागम ul_buffer_free_data_common(buf, true);
}

शून्य ul_buffer_set_chunksize(संरचना ul_buffer *buf, size_t sz)
{
	buf->chunksize = sz;
}

पूर्णाङ्क ul_buffer_is_empty(संरचना ul_buffer *buf)
{
	प्रत्यागम buf->begin == buf->end;
}

पूर्णाङ्क ul_buffer_save_pointer(संरचना ul_buffer *buf, unsigned short ptr_idx)
{
	जइ (ptr_idx >= buf->nptrs) {
		वर्ण **tmp = reallocarray(buf->ptrs, ptr_idx + 1, परिमाण(वर्ण *));

		जइ (!tmp)
			प्रत्यागम -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	प्रत्यागम 0;
}


वर्ण *ul_buffer_get_pointer(संरचना ul_buffer *buf, unsigned short ptr_idx)
{
	जइ (ptr_idx < buf->nptrs)
		प्रत्यागम buf->ptrs[ptr_idx];
	प्रत्यागम रिक्त;
}

/* returns length from begin to the pointer */
size_t ul_buffer_get_pointer_length(संरचना ul_buffer *buf, unsigned short ptr_idx)
{
	वर्ण *ptr = ul_buffer_get_pointer(buf, ptr_idx);

	जइ (ptr && ptr > buf->begin)
		प्रत्यागम ptr - buf->begin;
	प्रत्यागम 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_buffer_get_safe_pointer_width(संरचना ul_buffer *buf, unsigned short ptr_idx)
{
	size_t len = ul_buffer_get_pointer_length(buf, ptr_idx);

	जइ (!len)
		प्रत्यागम 0;

	प्रत्यागम mbs_safe_nwidth(buf->begin, len, रिक्त);
}

शून्य ul_buffer_refer_string(संरचना ul_buffer *buf, वर्ण *str)
{
	जइ (buf->sz)
		ul_buffer_free_data(buf);
	buf->begin = str;
	buf->sz = str ? strlen(str) : 0;
	buf->end = buf->begin ? buf->begin + buf->sz : buf->begin;
}

पूर्णाङ्क ul_buffer_alloc_data(संरचना ul_buffer *buf, size_t sz)
{
	वर्ण *tmp;
	size_t len = 0;

	प्रतिज्ञा(buf);

	जइ (sz <= buf->sz)
		प्रत्यागम 0;

	जइ (buf->end && buf->begin)
		len = buf->end - buf->begin;

	जइ (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	जइ (!tmp)
		प्रत्यागम -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	memset(buf->end, '\0', sz - len);

	प्रत्यागम 0;
}

पूर्णाङ्क ul_buffer_append_data(संरचना ul_buffer *buf, ध्रुव वर्ण *data, size_t sz)
{
	size_t maxsz = 0;

	जइ (!buf)
		प्रत्यागम -EINVAL;
	जइ (!data)
		प्रत्यागम 0;

	जइ (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	जइ (maxsz <= sz + 1) {
		पूर्णाङ्क rc = ul_buffer_alloc_data(buf, buf->sz + sz + 1);
		जइ (rc)
			प्रत्यागम rc;
	}
	जइ (!buf->end)
		प्रत्यागम -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, data, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	प्रत्यागम 0;
}

पूर्णाङ्क ul_buffer_append_char(संरचना ul_buffer *buf, ध्रुव वर्ण c)
{
	प्रत्यागम ul_buffer_append_data(buf, &c, 1);
}

पूर्णाङ्क ul_buffer_append_string(संरचना ul_buffer *buf, ध्रुव वर्ण *str)
{
	जइ (!str)
		प्रत्यागम 0;

	प्रत्यागम ul_buffer_append_data(buf, str, strlen(str));
}

पूर्णाङ्क ul_buffer_append_ntimes(संरचना ul_buffer *buf, size_t n, ध्रुव वर्ण *str)
{
	size_t i;
	size_t len = strlen(str);

	प्रति (i = 0; len && i < n; i++) {
		पूर्णाङ्क rc = ul_buffer_append_data(buf, str, len);
		जइ (rc)
			प्रत्यागम rc;
	}
	प्रत्यागम 0;
}

पूर्णाङ्क ul_buffer_appendf(संरचना ul_buffer *buf, ध्रुव वर्ण *format, ...)
{
	va_list ap;
	पूर्णाङ्क res;

	va_start(ap, format);
	res = ul_buffer_appendvf(buf, format, ap);
	va_end(ap);

	प्रत्यागम res;
}

पूर्णाङ्क ul_buffer_appendvf(संरचना ul_buffer *buf, ध्रुव वर्ण *format, va_list ap)
{
	वर्ण *val;
	पूर्णाङ्क sz;
	पूर्णाङ्क res;

	sz = vasprintf(&val, format, ap);
	जइ (sz < 0)
		प्रत्यागम -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_buffer_append_data(buf, val, sz);
	मुक्त(val);

	प्रत्यागम res;
}

पूर्णाङ्क ul_buffer_set_data(संरचना ul_buffer *buf, ध्रुव वर्ण *data, size_t sz)
{
	ul_buffer_reset_data(buf);
	प्रत्यागम ul_buffer_append_data(buf, data, sz);
}

वर्ण *ul_buffer_get_data(संरचना ul_buffer *buf, size_t *sz, size_t *width)
{
	जइ (sz)
		*sz = buf->end - buf->begin;
	जइ (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	प्रत्यागम buf->begin;
}

वर्ण *ul_buffer_get_string(संरचना ul_buffer *buf, size_t *sz, size_t *width)
{
	वर्ण *ret;

	ret = ul_buffer_get_data(buf, sz, width);

	/* data in buffer is already zero-terminated */
	जइ (sz)
		*sz = *sz + 1;

	प्रत्यागम ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_buffer_get_bufsiz(संरचना ul_buffer *buf)
{
	प्रत्यागम buf->sz;
}

size_t ul_buffer_get_datasiz(संरचना ul_buffer *buf)
{
	प्रत्यागम buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
वर्ण *ul_buffer_get_safe_data(संरचना ul_buffer *buf, size_t *sz, size_t *width, ध्रुव वर्ण *safechars)
{
	वर्ण *data = ul_buffer_get_data(buf, रिक्त, रिक्त);
	size_t encsz, wsz = 0;
	वर्ण *res = रिक्त;

	जइ (!data)
		गच्छ nothing;

	encsz = mbs_safe_encode_size(buf->sz) + 1;
	जइ (encsz > buf->encoded_sz) {
		वर्ण *tmp = realloc(buf->encoded, encsz);
		जइ (!tmp)
			गच्छ nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_buffer(data, &wsz, buf->encoded, buf->encoded_sz, safechars);
	जइ (!res || !wsz || wsz == (size_t) -1)
		गच्छ nothing;

	जइ (width)
		*width = wsz;
	जइ (sz)
		*sz = strlen(res);
	प्रत्यागम res;
nothing:
	जइ (width)
		*width = 0;
	जइ (sz)
		*sz = 0;
	प्रत्यागम रिक्त;
}


#ifdef TEST_PROGRAM_BUFFER

गणना {
	PTR_AAA  = 0,
	PTR_BBB,
};

पूर्णाङ्क main(शून्य)
{
	संरचना ul_buffer buf = UL_INIT_BUFFER;
	वर्ण *str;
	size_t sz = 0;

	ul_buffer_set_chunksize(&buf, 16);

	ul_buffer_append_string(&buf, "AAA");
	ul_buffer_append_data(&buf, "=", 1);
	ul_buffer_append_string(&buf, "aaa");
	ul_buffer_save_pointer(&buf, PTR_AAA);

	ul_buffer_append_data(&buf, ",", 1);
	ul_buffer_append_string(&buf, "BBB");
	ul_buffer_append_string(&buf, "=");
	ul_buffer_append_string(&buf, "bbb");
	ul_buffer_save_pointer(&buf, PTR_BBB);

	str = ul_buffer_get_data(&buf, &sz, रिक्त);
	printf("data [%zu] '%s'\n", sz, str);

	printf(" pointer data len: AAA=%zu, BBB=%zu\n",
			ul_buffer_get_pointer_length(&buf, PTR_AAA),
			ul_buffer_get_pointer_length(&buf, PTR_BBB));
	printf(" pointer data width: AAA=%zu, BBB=%zu\n",
			ul_buffer_get_safe_pointer_width(&buf, PTR_AAA),
			ul_buffer_get_safe_pointer_width(&buf, PTR_BBB));

	ul_buffer_reset_data(&buf);
	ul_buffer_append_string(&buf, "This is really long string to test the buffer function.");
	ul_buffer_save_pointer(&buf, PTR_AAA);
	ul_buffer_append_string(&buf, " YES!");
	str = ul_buffer_get_data(&buf, &sz, रिक्त);
	printf("data [%zu] '%s'\n", sz, str);
	printf(" pointer data len: AAA=%zu\n", ul_buffer_get_pointer_length(&buf, PTR_AAA));

	ul_buffer_free_data(&buf);
	str = strdup("foo");
	ul_buffer_refer_string(&buf, str);
	ul_buffer_append_data(&buf, ",", 1);
	ul_buffer_append_string(&buf, "bar");
	str = ul_buffer_get_data(&buf, &sz, रिक्त);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_buffer_get_data(&buf, &sz, रिक्त);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_append_char(&buf, 'x');
	{
		वर्ण c = 'y';
		ul_buffer_append_char(&buf, c);
	}
	{
		ध्रुव वर्ण c = 'z';
		ul_buffer_append_char(&buf, c);
	}
	str = ul_buffer_get_data(&buf, &sz, रिक्त);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);

	प्रत्यागम EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BUFFER */
