/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#شامل "buffer.h"
#شامل "mbsalign.h"
#شامل "strutils.h"

تهی ul_buffer_reset_data(ساختار ul_buffer *buf)
{
	اگر (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	اگر (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * اندازه(نویسه *));
}

ایستا درونخطی نویسه *ul_buffer_free_data_common(ساختار ul_buffer *buf, bool steal)
{
	نویسه *t = پوچ;

	أكد(buf);

	اگر (steal)
		t = buf->begin;
	وگرنه
		آزاد(buf->begin);
	buf->begin = پوچ;
	buf->end = پوچ;
	buf->sz = 0;

	آزاد(buf->ptrs);
	buf->ptrs = پوچ;
	buf->nptrs = 0;

	آزاد(buf->encoded);
	buf->encoded = پوچ;
	buf->encoded_sz = 0;

	بازگشت t;
}

تهی ul_buffer_free_data(ساختار ul_buffer *buf)
{
	ul_buffer_free_data_common(buf, false);
}

نویسه *ul_buffer_steal_string(ساختار ul_buffer *buf)
{
	اگر (ul_buffer_is_empty(buf)) {
		ul_buffer_free_data(buf);
		بازگشت پوچ;
	}

	بازگشت ul_buffer_free_data_common(buf, true);
}

تهی ul_buffer_set_chunksize(ساختار ul_buffer *buf, size_t sz)
{
	buf->chunksize = sz;
}

صحیح ul_buffer_is_empty(ساختار ul_buffer *buf)
{
	بازگشت buf->begin == buf->end;
}

صحیح ul_buffer_save_pointer(ساختار ul_buffer *buf, unsigned short ptr_idx)
{
	اگر (ptr_idx >= buf->nptrs) {
		نویسه **tmp = reallocarray(buf->ptrs, ptr_idx + 1, اندازه(نویسه *));

		اگر (!tmp)
			بازگشت -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	بازگشت 0;
}


نویسه *ul_buffer_get_pointer(ساختار ul_buffer *buf, unsigned short ptr_idx)
{
	اگر (ptr_idx < buf->nptrs)
		بازگشت buf->ptrs[ptr_idx];
	بازگشت پوچ;
}

/* returns length from begin to the pointer */
size_t ul_buffer_get_pointer_length(ساختار ul_buffer *buf, unsigned short ptr_idx)
{
	نویسه *ptr = ul_buffer_get_pointer(buf, ptr_idx);

	اگر (ptr && ptr > buf->begin)
		بازگشت ptr - buf->begin;
	بازگشت 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_buffer_get_safe_pointer_width(ساختار ul_buffer *buf, unsigned short ptr_idx)
{
	size_t len = ul_buffer_get_pointer_length(buf, ptr_idx);

	اگر (!len)
		بازگشت 0;

	بازگشت mbs_safe_nwidth(buf->begin, len, پوچ);
}

تهی ul_buffer_refer_string(ساختار ul_buffer *buf, نویسه *str)
{
	اگر (buf->sz)
		ul_buffer_free_data(buf);
	buf->begin = str;
	buf->sz = str ? strlen(str) : 0;
	buf->end = buf->begin ? buf->begin + buf->sz : buf->begin;
}

صحیح ul_buffer_alloc_data(ساختار ul_buffer *buf, size_t sz)
{
	نویسه *tmp;
	size_t len = 0;

	أكد(buf);

	اگر (sz <= buf->sz)
		بازگشت 0;

	اگر (buf->end && buf->begin)
		len = buf->end - buf->begin;

	اگر (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	اگر (!tmp)
		بازگشت -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	memset(buf->end, '\0', sz - len);

	بازگشت 0;
}

صحیح ul_buffer_append_data(ساختار ul_buffer *buf, ثابت نویسه *data, size_t sz)
{
	size_t maxsz = 0;

	اگر (!buf)
		بازگشت -EINVAL;
	اگر (!data)
		بازگشت 0;

	اگر (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	اگر (maxsz <= sz + 1) {
		صحیح rc = ul_buffer_alloc_data(buf, buf->sz + sz + 1);
		اگر (rc)
			بازگشت rc;
	}
	اگر (!buf->end)
		بازگشت -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, data, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	بازگشت 0;
}

صحیح ul_buffer_append_char(ساختار ul_buffer *buf, ثابت نویسه c)
{
	بازگشت ul_buffer_append_data(buf, &c, 1);
}

صحیح ul_buffer_append_string(ساختار ul_buffer *buf, ثابت نویسه *str)
{
	اگر (!str)
		بازگشت 0;

	بازگشت ul_buffer_append_data(buf, str, strlen(str));
}

صحیح ul_buffer_append_ntimes(ساختار ul_buffer *buf, size_t n, ثابت نویسه *str)
{
	size_t i;
	size_t len = strlen(str);

	برای (i = 0; len && i < n; i++) {
		صحیح rc = ul_buffer_append_data(buf, str, len);
		اگر (rc)
			بازگشت rc;
	}
	بازگشت 0;
}

صحیح ul_buffer_appendf(ساختار ul_buffer *buf, ثابت نویسه *format, ...)
{
	va_list ap;
	صحیح res;

	va_start(ap, format);
	res = ul_buffer_appendvf(buf, format, ap);
	va_end(ap);

	بازگشت res;
}

صحیح ul_buffer_appendvf(ساختار ul_buffer *buf, ثابت نویسه *format, va_list ap)
{
	نویسه *val;
	صحیح sz;
	صحیح res;

	sz = vasprintf(&val, format, ap);
	اگر (sz < 0)
		بازگشت -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_buffer_append_data(buf, val, sz);
	آزاد(val);

	بازگشت res;
}

صحیح ul_buffer_set_data(ساختار ul_buffer *buf, ثابت نویسه *data, size_t sz)
{
	ul_buffer_reset_data(buf);
	بازگشت ul_buffer_append_data(buf, data, sz);
}

نویسه *ul_buffer_get_data(ساختار ul_buffer *buf, size_t *sz, size_t *width)
{
	اگر (sz)
		*sz = buf->end - buf->begin;
	اگر (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	بازگشت buf->begin;
}

نویسه *ul_buffer_get_string(ساختار ul_buffer *buf, size_t *sz, size_t *width)
{
	نویسه *ret;

	ret = ul_buffer_get_data(buf, sz, width);

	/* data in buffer is already zero-terminated */
	اگر (sz)
		*sz = *sz + 1;

	بازگشت ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_buffer_get_bufsiz(ساختار ul_buffer *buf)
{
	بازگشت buf->sz;
}

size_t ul_buffer_get_datasiz(ساختار ul_buffer *buf)
{
	بازگشت buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
نویسه *ul_buffer_get_safe_data(ساختار ul_buffer *buf, size_t *sz, size_t *width, ثابت نویسه *safechars)
{
	نویسه *data = ul_buffer_get_data(buf, پوچ, پوچ);
	size_t encsz, wsz = 0;
	نویسه *res = پوچ;

	اگر (!data)
		برو nothing;

	encsz = mbs_safe_encode_size(buf->sz) + 1;
	اگر (encsz > buf->encoded_sz) {
		نویسه *tmp = realloc(buf->encoded, encsz);
		اگر (!tmp)
			برو nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_buffer(data, &wsz, buf->encoded, buf->encoded_sz, safechars);
	اگر (!res || !wsz || wsz == (size_t) -1)
		برو nothing;

	اگر (width)
		*width = wsz;
	اگر (sz)
		*sz = strlen(res);
	بازگشت res;
nothing:
	اگر (width)
		*width = 0;
	اگر (sz)
		*sz = 0;
	بازگشت پوچ;
}


#ifdef TEST_PROGRAM_BUFFER

شمارش {
	PTR_AAA  = 0,
	PTR_BBB,
};

صحیح اصلی(تهی)
{
	ساختار ul_buffer buf = UL_INIT_BUFFER;
	نویسه *str;
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

	str = ul_buffer_get_data(&buf, &sz, پوچ);
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
	str = ul_buffer_get_data(&buf, &sz, پوچ);
	printf("data [%zu] '%s'\n", sz, str);
	printf(" pointer data len: AAA=%zu\n", ul_buffer_get_pointer_length(&buf, PTR_AAA));

	ul_buffer_free_data(&buf);
	str = strdup("foo");
	ul_buffer_refer_string(&buf, str);
	ul_buffer_append_data(&buf, ",", 1);
	ul_buffer_append_string(&buf, "bar");
	str = ul_buffer_get_data(&buf, &sz, پوچ);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_buffer_get_data(&buf, &sz, پوچ);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_append_char(&buf, 'x');
	{
		نویسه c = 'y';
		ul_buffer_append_char(&buf, c);
	}
	{
		ثابت نویسه c = 'z';
		ul_buffer_append_char(&buf, c);
	}
	str = ul_buffer_get_data(&buf, &sz, پوچ);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);

	بازگشت EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BUFFER */
