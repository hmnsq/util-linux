/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ضمن "buffer.h"
#ضمن "mbsalign.h"
#ضمن "strutils.h"

فراغ ul_buffer_reset_data(بنية ul_buffer *buf)
{
	ܐܢ (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	ܐܢ (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * حجم(حرف *));
}

ساكن مضمن حرف *ul_buffer_free_data_common(بنية ul_buffer *buf, bool steal)
{
	حرف *t = عدم;

	أكد(buf);

	ܐܢ (steal)
		t = buf->begin;
	ܐܚܪܢܐ
		حرر(buf->begin);
	buf->begin = عدم;
	buf->end = عدم;
	buf->sz = 0;

	حرر(buf->ptrs);
	buf->ptrs = عدم;
	buf->nptrs = 0;

	حرر(buf->encoded);
	buf->encoded = عدم;
	buf->encoded_sz = 0;

	ܦܢܐ t;
}

فراغ ul_buffer_free_data(بنية ul_buffer *buf)
{
	ul_buffer_free_data_common(buf, false);
}

حرف *ul_buffer_steal_string(بنية ul_buffer *buf)
{
	ܐܢ (ul_buffer_is_empty(buf)) {
		ul_buffer_free_data(buf);
		ܦܢܐ عدم;
	}

	ܦܢܐ ul_buffer_free_data_common(buf, true);
}

فراغ ul_buffer_set_chunksize(بنية ul_buffer *buf, size_t sz)
{
	buf->chunksize = sz;
}

صحيح ul_buffer_is_empty(بنية ul_buffer *buf)
{
	ܦܢܐ buf->begin == buf->end;
}

صحيح ul_buffer_save_pointer(بنية ul_buffer *buf, unsigned short ptr_idx)
{
	ܐܢ (ptr_idx >= buf->nptrs) {
		حرف **tmp = reallocarray(buf->ptrs, ptr_idx + 1, حجم(حرف *));

		ܐܢ (!tmp)
			ܦܢܐ -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	ܦܢܐ 0;
}


حرف *ul_buffer_get_pointer(بنية ul_buffer *buf, unsigned short ptr_idx)
{
	ܐܢ (ptr_idx < buf->nptrs)
		ܦܢܐ buf->ptrs[ptr_idx];
	ܦܢܐ عدم;
}

/* returns length from begin to the pointer */
size_t ul_buffer_get_pointer_length(بنية ul_buffer *buf, unsigned short ptr_idx)
{
	حرف *ptr = ul_buffer_get_pointer(buf, ptr_idx);

	ܐܢ (ptr && ptr > buf->begin)
		ܦܢܐ ptr - buf->begin;
	ܦܢܐ 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_buffer_get_safe_pointer_width(بنية ul_buffer *buf, unsigned short ptr_idx)
{
	size_t len = ul_buffer_get_pointer_length(buf, ptr_idx);

	ܐܢ (!len)
		ܦܢܐ 0;

	ܦܢܐ mbs_safe_nwidth(buf->begin, len, عدم);
}

فراغ ul_buffer_refer_string(بنية ul_buffer *buf, حرف *str)
{
	ܐܢ (buf->sz)
		ul_buffer_free_data(buf);
	buf->begin = str;
	buf->sz = str ? strlen(str) : 0;
	buf->end = buf->begin ? buf->begin + buf->sz : buf->begin;
}

صحيح ul_buffer_alloc_data(بنية ul_buffer *buf, size_t sz)
{
	حرف *tmp;
	size_t len = 0;

	أكد(buf);

	ܐܢ (sz <= buf->sz)
		ܦܢܐ 0;

	ܐܢ (buf->end && buf->begin)
		len = buf->end - buf->begin;

	ܐܢ (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	ܐܢ (!tmp)
		ܦܢܐ -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	memset(buf->end, '\0', sz - len);

	ܦܢܐ 0;
}

صحيح ul_buffer_append_data(بنية ul_buffer *buf, ثابت حرف *data, size_t sz)
{
	size_t maxsz = 0;

	ܐܢ (!buf)
		ܦܢܐ -EINVAL;
	ܐܢ (!data)
		ܦܢܐ 0;

	ܐܢ (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	ܐܢ (maxsz <= sz + 1) {
		صحيح rc = ul_buffer_alloc_data(buf, buf->sz + sz + 1);
		ܐܢ (rc)
			ܦܢܐ rc;
	}
	ܐܢ (!buf->end)
		ܦܢܐ -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, data, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	ܦܢܐ 0;
}

صحيح ul_buffer_append_char(بنية ul_buffer *buf, ثابت حرف c)
{
	ܦܢܐ ul_buffer_append_data(buf, &c, 1);
}

صحيح ul_buffer_append_string(بنية ul_buffer *buf, ثابت حرف *str)
{
	ܐܢ (!str)
		ܦܢܐ 0;

	ܦܢܐ ul_buffer_append_data(buf, str, strlen(str));
}

صحيح ul_buffer_append_ntimes(بنية ul_buffer *buf, size_t n, ثابت حرف *str)
{
	size_t i;
	size_t len = strlen(str);

	لكل (i = 0; len && i < n; i++) {
		صحيح rc = ul_buffer_append_data(buf, str, len);
		ܐܢ (rc)
			ܦܢܐ rc;
	}
	ܦܢܐ 0;
}

صحيح ul_buffer_appendf(بنية ul_buffer *buf, ثابت حرف *format, ...)
{
	va_list ap;
	صحيح res;

	va_start(ap, format);
	res = ul_buffer_appendvf(buf, format, ap);
	va_end(ap);

	ܦܢܐ res;
}

صحيح ul_buffer_appendvf(بنية ul_buffer *buf, ثابت حرف *format, va_list ap)
{
	حرف *val;
	صحيح sz;
	صحيح res;

	sz = vasprintf(&val, format, ap);
	ܐܢ (sz < 0)
		ܦܢܐ -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_buffer_append_data(buf, val, sz);
	حرر(val);

	ܦܢܐ res;
}

صحيح ul_buffer_set_data(بنية ul_buffer *buf, ثابت حرف *data, size_t sz)
{
	ul_buffer_reset_data(buf);
	ܦܢܐ ul_buffer_append_data(buf, data, sz);
}

حرف *ul_buffer_get_data(بنية ul_buffer *buf, size_t *sz, size_t *width)
{
	ܐܢ (sz)
		*sz = buf->end - buf->begin;
	ܐܢ (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	ܦܢܐ buf->begin;
}

حرف *ul_buffer_get_string(بنية ul_buffer *buf, size_t *sz, size_t *width)
{
	حرف *ret;

	ret = ul_buffer_get_data(buf, sz, width);

	/* data in buffer is already zero-terminated */
	ܐܢ (sz)
		*sz = *sz + 1;

	ܦܢܐ ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_buffer_get_bufsiz(بنية ul_buffer *buf)
{
	ܦܢܐ buf->sz;
}

size_t ul_buffer_get_datasiz(بنية ul_buffer *buf)
{
	ܦܢܐ buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
حرف *ul_buffer_get_safe_data(بنية ul_buffer *buf, size_t *sz, size_t *width, ثابت حرف *safechars)
{
	حرف *data = ul_buffer_get_data(buf, عدم, عدم);
	size_t encsz, wsz = 0;
	حرف *res = عدم;

	ܐܢ (!data)
		اذهب nothing;

	encsz = mbs_safe_encode_size(buf->sz) + 1;
	ܐܢ (encsz > buf->encoded_sz) {
		حرف *tmp = realloc(buf->encoded, encsz);
		ܐܢ (!tmp)
			اذهب nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_buffer(data, &wsz, buf->encoded, buf->encoded_sz, safechars);
	ܐܢ (!res || !wsz || wsz == (size_t) -1)
		اذهب nothing;

	ܐܢ (width)
		*width = wsz;
	ܐܢ (sz)
		*sz = strlen(res);
	ܦܢܐ res;
nothing:
	ܐܢ (width)
		*width = 0;
	ܐܢ (sz)
		*sz = 0;
	ܦܢܐ عدم;
}


#ifdef TEST_PROGRAM_BUFFER

تعداد {
	PTR_AAA  = 0,
	PTR_BBB,
};

صحيح main(فراغ)
{
	بنية ul_buffer buf = UL_INIT_BUFFER;
	حرف *str;
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

	str = ul_buffer_get_data(&buf, &sz, عدم);
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
	str = ul_buffer_get_data(&buf, &sz, عدم);
	printf("data [%zu] '%s'\n", sz, str);
	printf(" pointer data len: AAA=%zu\n", ul_buffer_get_pointer_length(&buf, PTR_AAA));

	ul_buffer_free_data(&buf);
	str = strdup("foo");
	ul_buffer_refer_string(&buf, str);
	ul_buffer_append_data(&buf, ",", 1);
	ul_buffer_append_string(&buf, "bar");
	str = ul_buffer_get_data(&buf, &sz, عدم);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_buffer_get_data(&buf, &sz, عدم);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_append_char(&buf, 'x');
	{
		حرف c = 'y';
		ul_buffer_append_char(&buf, c);
	}
	{
		ثابت حرف c = 'z';
		ul_buffer_append_char(&buf, c);
	}
	str = ul_buffer_get_data(&buf, &sz, عدم);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);

	ܦܢܐ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BUFFER */
