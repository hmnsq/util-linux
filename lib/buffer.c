/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#அந்தர்பாவ "buffer.h"
#அந்தர்பாவ "mbsalign.h"
#அந்தர்பாவ "strutils.h"

வெறுமை ul_buffer_reset_data(கட்டமைப்பு ul_buffer *buf)
{
	எனில் (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	எனில் (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * அளவு(எழுத்து *));
}

நிலையான உள்வரி எழுத்து *ul_buffer_free_data_common(கட்டமைப்பு ul_buffer *buf, bool steal)
{
	எழுத்து *t = வெற்று;

	ப்ரதிஜ்ஞா(buf);

	எனில் (steal)
		t = buf->begin;
	இல்லையெனில்
		விடுவி(buf->begin);
	buf->begin = வெற்று;
	buf->end = வெற்று;
	buf->sz = 0;

	விடுவி(buf->ptrs);
	buf->ptrs = வெற்று;
	buf->nptrs = 0;

	விடுவி(buf->encoded);
	buf->encoded = வெற்று;
	buf->encoded_sz = 0;

	திரும்பு t;
}

வெறுமை ul_buffer_free_data(கட்டமைப்பு ul_buffer *buf)
{
	ul_buffer_free_data_common(buf, false);
}

எழுத்து *ul_buffer_steal_string(கட்டமைப்பு ul_buffer *buf)
{
	எனில் (ul_buffer_is_empty(buf)) {
		ul_buffer_free_data(buf);
		திரும்பு வெற்று;
	}

	திரும்பு ul_buffer_free_data_common(buf, true);
}

வெறுமை ul_buffer_set_chunksize(கட்டமைப்பு ul_buffer *buf, size_t sz)
{
	buf->chunksize = sz;
}

முழு ul_buffer_is_empty(கட்டமைப்பு ul_buffer *buf)
{
	திரும்பு buf->begin == buf->end;
}

முழு ul_buffer_save_pointer(கட்டமைப்பு ul_buffer *buf, unsigned short ptr_idx)
{
	எனில் (ptr_idx >= buf->nptrs) {
		எழுத்து **tmp = reallocarray(buf->ptrs, ptr_idx + 1, அளவு(எழுத்து *));

		எனில் (!tmp)
			திரும்பு -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	திரும்பு 0;
}


எழுத்து *ul_buffer_get_pointer(கட்டமைப்பு ul_buffer *buf, unsigned short ptr_idx)
{
	எனில் (ptr_idx < buf->nptrs)
		திரும்பு buf->ptrs[ptr_idx];
	திரும்பு வெற்று;
}

/* returns length from begin to the pointer */
size_t ul_buffer_get_pointer_length(கட்டமைப்பு ul_buffer *buf, unsigned short ptr_idx)
{
	எழுத்து *ptr = ul_buffer_get_pointer(buf, ptr_idx);

	எனில் (ptr && ptr > buf->begin)
		திரும்பு ptr - buf->begin;
	திரும்பு 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_buffer_get_safe_pointer_width(கட்டமைப்பு ul_buffer *buf, unsigned short ptr_idx)
{
	size_t len = ul_buffer_get_pointer_length(buf, ptr_idx);

	எனில் (!len)
		திரும்பு 0;

	திரும்பு mbs_safe_nwidth(buf->begin, len, வெற்று);
}

வெறுமை ul_buffer_refer_string(கட்டமைப்பு ul_buffer *buf, எழுத்து *str)
{
	எனில் (buf->sz)
		ul_buffer_free_data(buf);
	buf->begin = str;
	buf->sz = str ? strlen(str) : 0;
	buf->end = buf->begin ? buf->begin + buf->sz : buf->begin;
}

முழு ul_buffer_alloc_data(கட்டமைப்பு ul_buffer *buf, size_t sz)
{
	எழுத்து *tmp;
	size_t len = 0;

	ப்ரதிஜ்ஞா(buf);

	எனில் (sz <= buf->sz)
		திரும்பு 0;

	எனில் (buf->end && buf->begin)
		len = buf->end - buf->begin;

	எனில் (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	எனில் (!tmp)
		திரும்பு -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	memset(buf->end, '\0', sz - len);

	திரும்பு 0;
}

முழு ul_buffer_append_data(கட்டமைப்பு ul_buffer *buf, மாறிலி எழுத்து *data, size_t sz)
{
	size_t maxsz = 0;

	எனில் (!buf)
		திரும்பு -EINVAL;
	எனில் (!data)
		திரும்பு 0;

	எனில் (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	எனில் (maxsz <= sz + 1) {
		முழு rc = ul_buffer_alloc_data(buf, buf->sz + sz + 1);
		எனில் (rc)
			திரும்பு rc;
	}
	எனில் (!buf->end)
		திரும்பு -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, data, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	திரும்பு 0;
}

முழு ul_buffer_append_char(கட்டமைப்பு ul_buffer *buf, மாறிலி எழுத்து c)
{
	திரும்பு ul_buffer_append_data(buf, &c, 1);
}

முழு ul_buffer_append_string(கட்டமைப்பு ul_buffer *buf, மாறிலி எழுத்து *str)
{
	எனில் (!str)
		திரும்பு 0;

	திரும்பு ul_buffer_append_data(buf, str, strlen(str));
}

முழு ul_buffer_append_ntimes(கட்டமைப்பு ul_buffer *buf, size_t n, மாறிலி எழுத்து *str)
{
	size_t i;
	size_t len = strlen(str);

	க்கு (i = 0; len && i < n; i++) {
		முழு rc = ul_buffer_append_data(buf, str, len);
		எனில் (rc)
			திரும்பு rc;
	}
	திரும்பு 0;
}

முழு ul_buffer_appendf(கட்டமைப்பு ul_buffer *buf, மாறிலி எழுத்து *format, ...)
{
	va_list ap;
	முழு res;

	va_start(ap, format);
	res = ul_buffer_appendvf(buf, format, ap);
	va_end(ap);

	திரும்பு res;
}

முழு ul_buffer_appendvf(கட்டமைப்பு ul_buffer *buf, மாறிலி எழுத்து *format, va_list ap)
{
	எழுத்து *val;
	முழு sz;
	முழு res;

	sz = vasprintf(&val, format, ap);
	எனில் (sz < 0)
		திரும்பு -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_buffer_append_data(buf, val, sz);
	விடுவி(val);

	திரும்பு res;
}

முழு ul_buffer_set_data(கட்டமைப்பு ul_buffer *buf, மாறிலி எழுத்து *data, size_t sz)
{
	ul_buffer_reset_data(buf);
	திரும்பு ul_buffer_append_data(buf, data, sz);
}

எழுத்து *ul_buffer_get_data(கட்டமைப்பு ul_buffer *buf, size_t *sz, size_t *width)
{
	எனில் (sz)
		*sz = buf->end - buf->begin;
	எனில் (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	திரும்பு buf->begin;
}

எழுத்து *ul_buffer_get_string(கட்டமைப்பு ul_buffer *buf, size_t *sz, size_t *width)
{
	எழுத்து *ret;

	ret = ul_buffer_get_data(buf, sz, width);

	/* data in buffer is already zero-terminated */
	எனில் (sz)
		*sz = *sz + 1;

	திரும்பு ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_buffer_get_bufsiz(கட்டமைப்பு ul_buffer *buf)
{
	திரும்பு buf->sz;
}

size_t ul_buffer_get_datasiz(கட்டமைப்பு ul_buffer *buf)
{
	திரும்பு buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
எழுத்து *ul_buffer_get_safe_data(கட்டமைப்பு ul_buffer *buf, size_t *sz, size_t *width, மாறிலி எழுத்து *safechars)
{
	எழுத்து *data = ul_buffer_get_data(buf, வெற்று, வெற்று);
	size_t encsz, wsz = 0;
	எழுத்து *res = வெற்று;

	எனில் (!data)
		செல் nothing;

	encsz = mbs_safe_encode_size(buf->sz) + 1;
	எனில் (encsz > buf->encoded_sz) {
		எழுத்து *tmp = realloc(buf->encoded, encsz);
		எனில் (!tmp)
			செல் nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_buffer(data, &wsz, buf->encoded, buf->encoded_sz, safechars);
	எனில் (!res || !wsz || wsz == (size_t) -1)
		செல் nothing;

	எனில் (width)
		*width = wsz;
	எனில் (sz)
		*sz = strlen(res);
	திரும்பு res;
nothing:
	எனில் (width)
		*width = 0;
	எனில் (sz)
		*sz = 0;
	திரும்பு வெற்று;
}


#ifdef TEST_PROGRAM_BUFFER

எண்ணிக்கை {
	PTR_AAA  = 0,
	PTR_BBB,
};

முழு முதன்மை(வெறுமை)
{
	கட்டமைப்பு ul_buffer buf = UL_INIT_BUFFER;
	எழுத்து *str;
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

	str = ul_buffer_get_data(&buf, &sz, வெற்று);
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
	str = ul_buffer_get_data(&buf, &sz, வெற்று);
	printf("data [%zu] '%s'\n", sz, str);
	printf(" pointer data len: AAA=%zu\n", ul_buffer_get_pointer_length(&buf, PTR_AAA));

	ul_buffer_free_data(&buf);
	str = strdup("foo");
	ul_buffer_refer_string(&buf, str);
	ul_buffer_append_data(&buf, ",", 1);
	ul_buffer_append_string(&buf, "bar");
	str = ul_buffer_get_data(&buf, &sz, வெற்று);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_buffer_get_data(&buf, &sz, வெற்று);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_append_char(&buf, 'x');
	{
		எழுத்து c = 'y';
		ul_buffer_append_char(&buf, c);
	}
	{
		மாறிலி எழுத்து c = 'z';
		ul_buffer_append_char(&buf, c);
	}
	str = ul_buffer_get_data(&buf, &sz, வெற்று);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);

	திரும்பு EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BUFFER */
