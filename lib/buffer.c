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
	إذا (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	إذا (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * حجم(حرف *));
}

ساكن مضمن حرف *ul_buffer_free_data_common(بنية ul_buffer *buf, bool steal)
{
	حرف *t = عدم;

	أكد(buf);

	إذا (steal)
		t = buf->begin;
	وإلا
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

	أرجع t;
}

فراغ ul_buffer_free_data(بنية ul_buffer *buf)
{
	ul_buffer_free_data_common(buf, false);
}

حرف *ul_buffer_steal_string(بنية ul_buffer *buf)
{
	إذا (ul_buffer_is_empty(buf)) {
		ul_buffer_free_data(buf);
		أرجع عدم;
	}

	أرجع ul_buffer_free_data_common(buf, true);
}

فراغ ul_buffer_set_chunksize(بنية ul_buffer *buf, size_t sz)
{
	buf->chunksize = sz;
}

صحيح ul_buffer_is_empty(بنية ul_buffer *buf)
{
	أرجع buf->begin == buf->end;
}

صحيح ul_buffer_save_pointer(بنية ul_buffer *buf, unsigned short ptr_idx)
{
	إذا (ptr_idx >= buf->nptrs) {
		حرف **tmp = reallocarray(buf->ptrs, ptr_idx + 1, حجم(حرف *));

		إذا (!tmp)
			أرجع -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	أرجع 0;
}


حرف *ul_buffer_get_pointer(بنية ul_buffer *buf, unsigned short ptr_idx)
{
	إذا (ptr_idx < buf->nptrs)
		أرجع buf->ptrs[ptr_idx];
	أرجع عدم;
}

/* returns length from begin to the pointer */
size_t ul_buffer_get_pointer_length(بنية ul_buffer *buf, unsigned short ptr_idx)
{
	حرف *ptr = ul_buffer_get_pointer(buf, ptr_idx);

	إذا (ptr && ptr > buf->begin)
		أرجع ptr - buf->begin;
	أرجع 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_buffer_get_safe_pointer_width(بنية ul_buffer *buf, unsigned short ptr_idx)
{
	size_t len = ul_buffer_get_pointer_length(buf, ptr_idx);

	إذا (!len)
		أرجع 0;

	أرجع mbs_safe_nwidth(buf->begin, len, عدم);
}

فراغ ul_buffer_refer_string(بنية ul_buffer *buf, حرف *str)
{
	إذا (buf->sz)
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

	إذا (sz <= buf->sz)
		أرجع 0;

	إذا (buf->end && buf->begin)
		len = buf->end - buf->begin;

	إذا (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	إذا (!tmp)
		أرجع -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	memset(buf->end, '\0', sz - len);

	أرجع 0;
}

صحيح ul_buffer_append_data(بنية ul_buffer *buf, ثابت حرف *data, size_t sz)
{
	size_t maxsz = 0;

	إذا (!buf)
		أرجع -EINVAL;
	إذا (!data)
		أرجع 0;

	إذا (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	إذا (maxsz <= sz + 1) {
		صحيح rc = ul_buffer_alloc_data(buf, buf->sz + sz + 1);
		إذا (rc)
			أرجع rc;
	}
	إذا (!buf->end)
		أرجع -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, data, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	أرجع 0;
}

صحيح ul_buffer_append_char(بنية ul_buffer *buf, ثابت حرف c)
{
	أرجع ul_buffer_append_data(buf, &c, 1);
}

صحيح ul_buffer_append_string(بنية ul_buffer *buf, ثابت حرف *str)
{
	إذا (!str)
		أرجع 0;

	أرجع ul_buffer_append_data(buf, str, strlen(str));
}

صحيح ul_buffer_append_ntimes(بنية ul_buffer *buf, size_t n, ثابت حرف *str)
{
	size_t i;
	size_t len = strlen(str);

	لكل (i = 0; len && i < n; i++) {
		صحيح rc = ul_buffer_append_data(buf, str, len);
		إذا (rc)
			أرجع rc;
	}
	أرجع 0;
}

صحيح ul_buffer_appendf(بنية ul_buffer *buf, ثابت حرف *format, ...)
{
	va_list ap;
	صحيح res;

	va_start(ap, format);
	res = ul_buffer_appendvf(buf, format, ap);
	va_end(ap);

	أرجع res;
}

صحيح ul_buffer_appendvf(بنية ul_buffer *buf, ثابت حرف *format, va_list ap)
{
	حرف *val;
	صحيح sz;
	صحيح res;

	sz = vasprintf(&val, format, ap);
	إذا (sz < 0)
		أرجع -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_buffer_append_data(buf, val, sz);
	حرر(val);

	أرجع res;
}

صحيح ul_buffer_set_data(بنية ul_buffer *buf, ثابت حرف *data, size_t sz)
{
	ul_buffer_reset_data(buf);
	أرجع ul_buffer_append_data(buf, data, sz);
}

حرف *ul_buffer_get_data(بنية ul_buffer *buf, size_t *sz, size_t *width)
{
	إذا (sz)
		*sz = buf->end - buf->begin;
	إذا (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	أرجع buf->begin;
}

حرف *ul_buffer_get_string(بنية ul_buffer *buf, size_t *sz, size_t *width)
{
	حرف *ret;

	ret = ul_buffer_get_data(buf, sz, width);

	/* data in buffer is already zero-terminated */
	إذا (sz)
		*sz = *sz + 1;

	أرجع ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_buffer_get_bufsiz(بنية ul_buffer *buf)
{
	أرجع buf->sz;
}

size_t ul_buffer_get_datasiz(بنية ul_buffer *buf)
{
	أرجع buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
حرف *ul_buffer_get_safe_data(بنية ul_buffer *buf, size_t *sz, size_t *width, ثابت حرف *safechars)
{
	حرف *data = ul_buffer_get_data(buf, عدم, عدم);
	size_t encsz, wsz = 0;
	حرف *res = عدم;

	إذا (!data)
		اذهب nothing;

	encsz = mbs_safe_encode_size(buf->sz) + 1;
	إذا (encsz > buf->encoded_sz) {
		حرف *tmp = realloc(buf->encoded, encsz);
		إذا (!tmp)
			اذهب nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_buffer(data, &wsz, buf->encoded, buf->encoded_sz, safechars);
	إذا (!res || !wsz || wsz == (size_t) -1)
		اذهب nothing;

	إذا (width)
		*width = wsz;
	إذا (sz)
		*sz = strlen(res);
	أرجع res;
nothing:
	إذا (width)
		*width = 0;
	إذا (sz)
		*sz = 0;
	أرجع عدم;
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

	أرجع EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BUFFER */
