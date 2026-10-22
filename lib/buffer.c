/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ಅನ್ತರ್ಭಾವ "buffer.h"
#ಅನ್ತರ್ಭಾವ "mbsalign.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"

ಶೂನ್ಯ ul_buffer_reset_data(ಸಂರಚನಾ ul_buffer *buf)
{
	ಯದಿ (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	ಯದಿ (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * ಪರಿಮಾಣ(ವರ್ಣ *));
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ವರ್ಣ *ul_buffer_free_data_common(ಸಂರಚನಾ ul_buffer *buf, bool steal)
{
	ವರ್ಣ *t = ರಿಕ್ತ;

	ಪ್ರತಿಜ್ಞಾ(buf);

	ಯದಿ (steal)
		t = buf->begin;
	ಅನ್ಯಥಾ
		ಮುಕ್ತ(buf->begin);
	buf->begin = ರಿಕ್ತ;
	buf->end = ರಿಕ್ತ;
	buf->sz = 0;

	ಮುಕ್ತ(buf->ptrs);
	buf->ptrs = ರಿಕ್ತ;
	buf->nptrs = 0;

	ಮುಕ್ತ(buf->encoded);
	buf->encoded = ರಿಕ್ತ;
	buf->encoded_sz = 0;

	ಪ್ರತ್ಯಾಗಮ t;
}

ಶೂನ್ಯ ul_buffer_free_data(ಸಂರಚನಾ ul_buffer *buf)
{
	ul_buffer_free_data_common(buf, false);
}

ವರ್ಣ *ul_buffer_steal_string(ಸಂರಚನಾ ul_buffer *buf)
{
	ಯದಿ (ul_buffer_is_empty(buf)) {
		ul_buffer_free_data(buf);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ ul_buffer_free_data_common(buf, true);
}

ಶೂನ್ಯ ul_buffer_set_chunksize(ಸಂರಚನಾ ul_buffer *buf, size_t sz)
{
	buf->chunksize = sz;
}

ಪೂರ್ಣಾಙ್ಕ ul_buffer_is_empty(ಸಂರಚನಾ ul_buffer *buf)
{
	ಪ್ರತ್ಯಾಗಮ buf->begin == buf->end;
}

ಪೂರ್ಣಾಙ್ಕ ul_buffer_save_pointer(ಸಂರಚನಾ ul_buffer *buf, unsigned short ptr_idx)
{
	ಯದಿ (ptr_idx >= buf->nptrs) {
		ವರ್ಣ **tmp = reallocarray(buf->ptrs, ptr_idx + 1, ಪರಿಮಾಣ(ವರ್ಣ *));

		ಯದಿ (!tmp)
			ಪ್ರತ್ಯಾಗಮ -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	ಪ್ರತ್ಯಾಗಮ 0;
}


ವರ್ಣ *ul_buffer_get_pointer(ಸಂರಚನಾ ul_buffer *buf, unsigned short ptr_idx)
{
	ಯದಿ (ptr_idx < buf->nptrs)
		ಪ್ರತ್ಯಾಗಮ buf->ptrs[ptr_idx];
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/* returns length from begin to the pointer */
size_t ul_buffer_get_pointer_length(ಸಂರಚನಾ ul_buffer *buf, unsigned short ptr_idx)
{
	ವರ್ಣ *ptr = ul_buffer_get_pointer(buf, ptr_idx);

	ಯದಿ (ptr && ptr > buf->begin)
		ಪ್ರತ್ಯಾಗಮ ptr - buf->begin;
	ಪ್ರತ್ಯಾಗಮ 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_buffer_get_safe_pointer_width(ಸಂರಚನಾ ul_buffer *buf, unsigned short ptr_idx)
{
	size_t len = ul_buffer_get_pointer_length(buf, ptr_idx);

	ಯದಿ (!len)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಪ್ರತ್ಯಾಗಮ mbs_safe_nwidth(buf->begin, len, ರಿಕ್ತ);
}

ಶೂನ್ಯ ul_buffer_refer_string(ಸಂರಚನಾ ul_buffer *buf, ವರ್ಣ *str)
{
	ಯದಿ (buf->sz)
		ul_buffer_free_data(buf);
	buf->begin = str;
	buf->sz = str ? strlen(str) : 0;
	buf->end = buf->begin ? buf->begin + buf->sz : buf->begin;
}

ಪೂರ್ಣಾಙ್ಕ ul_buffer_alloc_data(ಸಂರಚನಾ ul_buffer *buf, size_t sz)
{
	ವರ್ಣ *tmp;
	size_t len = 0;

	ಪ್ರತಿಜ್ಞಾ(buf);

	ಯದಿ (sz <= buf->sz)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (buf->end && buf->begin)
		len = buf->end - buf->begin;

	ಯದಿ (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	ಯದಿ (!tmp)
		ಪ್ರತ್ಯಾಗಮ -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	memset(buf->end, '\0', sz - len);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ ul_buffer_append_data(ಸಂರಚನಾ ul_buffer *buf, ಧ್ರುವ ವರ್ಣ *data, size_t sz)
{
	size_t maxsz = 0;

	ಯದಿ (!buf)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	ಯದಿ (!data)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	ಯದಿ (maxsz <= sz + 1) {
		ಪೂರ್ಣಾಙ್ಕ rc = ul_buffer_alloc_data(buf, buf->sz + sz + 1);
		ಯದಿ (rc)
			ಪ್ರತ್ಯಾಗಮ rc;
	}
	ಯದಿ (!buf->end)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, data, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ ul_buffer_append_char(ಸಂರಚನಾ ul_buffer *buf, ಧ್ರುವ ವರ್ಣ c)
{
	ಪ್ರತ್ಯಾಗಮ ul_buffer_append_data(buf, &c, 1);
}

ಪೂರ್ಣಾಙ್ಕ ul_buffer_append_string(ಸಂರಚನಾ ul_buffer *buf, ಧ್ರುವ ವರ್ಣ *str)
{
	ಯದಿ (!str)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಪ್ರತ್ಯಾಗಮ ul_buffer_append_data(buf, str, strlen(str));
}

ಪೂರ್ಣಾಙ್ಕ ul_buffer_append_ntimes(ಸಂರಚನಾ ul_buffer *buf, size_t n, ಧ್ರುವ ವರ್ಣ *str)
{
	size_t i;
	size_t len = strlen(str);

	ಪ್ರತಿ (i = 0; len && i < n; i++) {
		ಪೂರ್ಣಾಙ್ಕ rc = ul_buffer_append_data(buf, str, len);
		ಯದಿ (rc)
			ಪ್ರತ್ಯಾಗಮ rc;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ ul_buffer_appendf(ಸಂರಚನಾ ul_buffer *buf, ಧ್ರುವ ವರ್ಣ *format, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಙ್ಕ res;

	va_start(ap, format);
	res = ul_buffer_appendvf(buf, format, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ res;
}

ಪೂರ್ಣಾಙ್ಕ ul_buffer_appendvf(ಸಂರಚನಾ ul_buffer *buf, ಧ್ರುವ ವರ್ಣ *format, va_list ap)
{
	ವರ್ಣ *val;
	ಪೂರ್ಣಾಙ್ಕ sz;
	ಪೂರ್ಣಾಙ್ಕ res;

	sz = vasprintf(&val, format, ap);
	ಯದಿ (sz < 0)
		ಪ್ರತ್ಯಾಗಮ -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_buffer_append_data(buf, val, sz);
	ಮುಕ್ತ(val);

	ಪ್ರತ್ಯಾಗಮ res;
}

ಪೂರ್ಣಾಙ್ಕ ul_buffer_set_data(ಸಂರಚನಾ ul_buffer *buf, ಧ್ರುವ ವರ್ಣ *data, size_t sz)
{
	ul_buffer_reset_data(buf);
	ಪ್ರತ್ಯಾಗಮ ul_buffer_append_data(buf, data, sz);
}

ವರ್ಣ *ul_buffer_get_data(ಸಂರಚನಾ ul_buffer *buf, size_t *sz, size_t *width)
{
	ಯದಿ (sz)
		*sz = buf->end - buf->begin;
	ಯದಿ (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	ಪ್ರತ್ಯಾಗಮ buf->begin;
}

ವರ್ಣ *ul_buffer_get_string(ಸಂರಚನಾ ul_buffer *buf, size_t *sz, size_t *width)
{
	ವರ್ಣ *ret;

	ret = ul_buffer_get_data(buf, sz, width);

	/* data in buffer is already zero-terminated */
	ಯದಿ (sz)
		*sz = *sz + 1;

	ಪ್ರತ್ಯಾಗಮ ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_buffer_get_bufsiz(ಸಂರಚನಾ ul_buffer *buf)
{
	ಪ್ರತ್ಯಾಗಮ buf->sz;
}

size_t ul_buffer_get_datasiz(ಸಂರಚನಾ ul_buffer *buf)
{
	ಪ್ರತ್ಯಾಗಮ buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
ವರ್ಣ *ul_buffer_get_safe_data(ಸಂರಚನಾ ul_buffer *buf, size_t *sz, size_t *width, ಧ್ರುವ ವರ್ಣ *safechars)
{
	ವರ್ಣ *data = ul_buffer_get_data(buf, ರಿಕ್ತ, ರಿಕ್ತ);
	size_t encsz, wsz = 0;
	ವರ್ಣ *res = ರಿಕ್ತ;

	ಯದಿ (!data)
		ಗಚ್ಛ nothing;

	encsz = mbs_safe_encode_size(buf->sz) + 1;
	ಯದಿ (encsz > buf->encoded_sz) {
		ವರ್ಣ *tmp = realloc(buf->encoded, encsz);
		ಯದಿ (!tmp)
			ಗಚ್ಛ nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_buffer(data, &wsz, buf->encoded, buf->encoded_sz, safechars);
	ಯದಿ (!res || !wsz || wsz == (size_t) -1)
		ಗಚ್ಛ nothing;

	ಯದಿ (width)
		*width = wsz;
	ಯದಿ (sz)
		*sz = strlen(res);
	ಪ್ರತ್ಯಾಗಮ res;
nothing:
	ಯದಿ (width)
		*width = 0;
	ಯದಿ (sz)
		*sz = 0;
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}


#ifdef TEST_PROGRAM_BUFFER

ಗಣನಾ {
	PTR_AAA  = 0,
	PTR_BBB,
};

ಪೂರ್ಣಾಙ್ಕ main(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ ul_buffer buf = UL_INIT_BUFFER;
	ವರ್ಣ *str;
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

	str = ul_buffer_get_data(&buf, &sz, ರಿಕ್ತ);
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
	str = ul_buffer_get_data(&buf, &sz, ರಿಕ್ತ);
	printf("data [%zu] '%s'\n", sz, str);
	printf(" pointer data len: AAA=%zu\n", ul_buffer_get_pointer_length(&buf, PTR_AAA));

	ul_buffer_free_data(&buf);
	str = strdup("foo");
	ul_buffer_refer_string(&buf, str);
	ul_buffer_append_data(&buf, ",", 1);
	ul_buffer_append_string(&buf, "bar");
	str = ul_buffer_get_data(&buf, &sz, ರಿಕ್ತ);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_buffer_get_data(&buf, &sz, ರಿಕ್ತ);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_append_char(&buf, 'x');
	{
		ವರ್ಣ c = 'y';
		ul_buffer_append_char(&buf, c);
	}
	{
		ಧ್ರುವ ವರ್ಣ c = 'z';
		ul_buffer_append_char(&buf, c);
	}
	str = ul_buffer_get_data(&buf, &sz, ರಿಕ್ತ);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);

	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BUFFER */
