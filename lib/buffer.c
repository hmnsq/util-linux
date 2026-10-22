/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#অন্তর্ভাৱ "buffer.h"
#অন্তর্ভাৱ "mbsalign.h"
#অন্তর্ভাৱ "strutils.h"

শূন্য ul_buffer_reset_data(সংরচনা ul_buffer *buf)
{
	যদি (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	যদি (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * পরিমাণ(ৱর্ণ *));
}

স্থির অন্তঃস্থ ৱর্ণ *ul_buffer_free_data_common(সংরচনা ul_buffer *buf, bool steal)
{
	ৱর্ণ *t = রিক্ত;

	প্রতিজ্ঞা(buf);

	যদি (steal)
		t = buf->begin;
	অন্যথা
		মুক্ত(buf->begin);
	buf->begin = রিক্ত;
	buf->end = রিক্ত;
	buf->sz = 0;

	মুক্ত(buf->ptrs);
	buf->ptrs = রিক্ত;
	buf->nptrs = 0;

	মুক্ত(buf->encoded);
	buf->encoded = রিক্ত;
	buf->encoded_sz = 0;

	ঘূৰাই t;
}

শূন্য ul_buffer_free_data(সংরচনা ul_buffer *buf)
{
	ul_buffer_free_data_common(buf, false);
}

ৱর্ণ *ul_buffer_steal_string(সংরচনা ul_buffer *buf)
{
	যদি (ul_buffer_is_empty(buf)) {
		ul_buffer_free_data(buf);
		ঘূৰাই রিক্ত;
	}

	ঘূৰাই ul_buffer_free_data_common(buf, true);
}

শূন্য ul_buffer_set_chunksize(সংরচনা ul_buffer *buf, size_t sz)
{
	buf->chunksize = sz;
}

পূর্ণাঙ্ক ul_buffer_is_empty(সংরচনা ul_buffer *buf)
{
	ঘূৰাই buf->begin == buf->end;
}

পূর্ণাঙ্ক ul_buffer_save_pointer(সংরচনা ul_buffer *buf, unsigned short ptr_idx)
{
	যদি (ptr_idx >= buf->nptrs) {
		ৱর্ণ **tmp = reallocarray(buf->ptrs, ptr_idx + 1, পরিমাণ(ৱর্ণ *));

		যদি (!tmp)
			ঘূৰাই -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	ঘূৰাই 0;
}


ৱর্ণ *ul_buffer_get_pointer(সংরচনা ul_buffer *buf, unsigned short ptr_idx)
{
	যদি (ptr_idx < buf->nptrs)
		ঘূৰাই buf->ptrs[ptr_idx];
	ঘূৰাই রিক্ত;
}

/* returns length from begin to the pointer */
size_t ul_buffer_get_pointer_length(সংরচনা ul_buffer *buf, unsigned short ptr_idx)
{
	ৱর্ণ *ptr = ul_buffer_get_pointer(buf, ptr_idx);

	যদি (ptr && ptr > buf->begin)
		ঘূৰাই ptr - buf->begin;
	ঘূৰাই 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_buffer_get_safe_pointer_width(সংরচনা ul_buffer *buf, unsigned short ptr_idx)
{
	size_t len = ul_buffer_get_pointer_length(buf, ptr_idx);

	যদি (!len)
		ঘূৰাই 0;

	ঘূৰাই mbs_safe_nwidth(buf->begin, len, রিক্ত);
}

শূন্য ul_buffer_refer_string(সংরচনা ul_buffer *buf, ৱর্ণ *str)
{
	যদি (buf->sz)
		ul_buffer_free_data(buf);
	buf->begin = str;
	buf->sz = str ? strlen(str) : 0;
	buf->end = buf->begin ? buf->begin + buf->sz : buf->begin;
}

পূর্ণাঙ্ক ul_buffer_alloc_data(সংরচনা ul_buffer *buf, size_t sz)
{
	ৱর্ণ *tmp;
	size_t len = 0;

	প্রতিজ্ঞা(buf);

	যদি (sz <= buf->sz)
		ঘূৰাই 0;

	যদি (buf->end && buf->begin)
		len = buf->end - buf->begin;

	যদি (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	যদি (!tmp)
		ঘূৰাই -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	memset(buf->end, '\0', sz - len);

	ঘূৰাই 0;
}

পূর্ণাঙ্ক ul_buffer_append_data(সংরচনা ul_buffer *buf, ধ্রুৱ ৱর্ণ *data, size_t sz)
{
	size_t maxsz = 0;

	যদি (!buf)
		ঘূৰাই -EINVAL;
	যদি (!data)
		ঘূৰাই 0;

	যদি (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	যদি (maxsz <= sz + 1) {
		পূর্ণাঙ্ক rc = ul_buffer_alloc_data(buf, buf->sz + sz + 1);
		যদি (rc)
			ঘূৰাই rc;
	}
	যদি (!buf->end)
		ঘূৰাই -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, data, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	ঘূৰাই 0;
}

পূর্ণাঙ্ক ul_buffer_append_char(সংরচনা ul_buffer *buf, ধ্রুৱ ৱর্ণ c)
{
	ঘূৰাই ul_buffer_append_data(buf, &c, 1);
}

পূর্ণাঙ্ক ul_buffer_append_string(সংরচনা ul_buffer *buf, ধ্রুৱ ৱর্ণ *str)
{
	যদি (!str)
		ঘূৰাই 0;

	ঘূৰাই ul_buffer_append_data(buf, str, strlen(str));
}

পূর্ণাঙ্ক ul_buffer_append_ntimes(সংরচনা ul_buffer *buf, size_t n, ধ্রুৱ ৱর্ণ *str)
{
	size_t i;
	size_t len = strlen(str);

	বাবে (i = 0; len && i < n; i++) {
		পূর্ণাঙ্ক rc = ul_buffer_append_data(buf, str, len);
		যদি (rc)
			ঘূৰাই rc;
	}
	ঘূৰাই 0;
}

পূর্ণাঙ্ক ul_buffer_appendf(সংরচনা ul_buffer *buf, ধ্রুৱ ৱর্ণ *format, ...)
{
	va_list ap;
	পূর্ণাঙ্ক res;

	va_start(ap, format);
	res = ul_buffer_appendvf(buf, format, ap);
	va_end(ap);

	ঘূৰাই res;
}

পূর্ণাঙ্ক ul_buffer_appendvf(সংরচনা ul_buffer *buf, ধ্রুৱ ৱর্ণ *format, va_list ap)
{
	ৱর্ণ *val;
	পূর্ণাঙ্ক sz;
	পূর্ণাঙ্ক res;

	sz = vasprintf(&val, format, ap);
	যদি (sz < 0)
		ঘূৰাই -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_buffer_append_data(buf, val, sz);
	মুক্ত(val);

	ঘূৰাই res;
}

পূর্ণাঙ্ক ul_buffer_set_data(সংরচনা ul_buffer *buf, ধ্রুৱ ৱর্ণ *data, size_t sz)
{
	ul_buffer_reset_data(buf);
	ঘূৰাই ul_buffer_append_data(buf, data, sz);
}

ৱর্ণ *ul_buffer_get_data(সংরচনা ul_buffer *buf, size_t *sz, size_t *width)
{
	যদি (sz)
		*sz = buf->end - buf->begin;
	যদি (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	ঘূৰাই buf->begin;
}

ৱর্ণ *ul_buffer_get_string(সংরচনা ul_buffer *buf, size_t *sz, size_t *width)
{
	ৱর্ণ *ret;

	ret = ul_buffer_get_data(buf, sz, width);

	/* data in buffer is already zero-terminated */
	যদি (sz)
		*sz = *sz + 1;

	ঘূৰাই ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_buffer_get_bufsiz(সংরচনা ul_buffer *buf)
{
	ঘূৰাই buf->sz;
}

size_t ul_buffer_get_datasiz(সংরচনা ul_buffer *buf)
{
	ঘূৰাই buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
ৱর্ণ *ul_buffer_get_safe_data(সংরচনা ul_buffer *buf, size_t *sz, size_t *width, ধ্রুৱ ৱর্ণ *safechars)
{
	ৱর্ণ *data = ul_buffer_get_data(buf, রিক্ত, রিক্ত);
	size_t encsz, wsz = 0;
	ৱর্ণ *res = রিক্ত;

	যদি (!data)
		গচ্ছ nothing;

	encsz = mbs_safe_encode_size(buf->sz) + 1;
	যদি (encsz > buf->encoded_sz) {
		ৱর্ণ *tmp = realloc(buf->encoded, encsz);
		যদি (!tmp)
			গচ্ছ nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_buffer(data, &wsz, buf->encoded, buf->encoded_sz, safechars);
	যদি (!res || !wsz || wsz == (size_t) -1)
		গচ্ছ nothing;

	যদি (width)
		*width = wsz;
	যদি (sz)
		*sz = strlen(res);
	ঘূৰাই res;
nothing:
	যদি (width)
		*width = 0;
	যদি (sz)
		*sz = 0;
	ঘূৰাই রিক্ত;
}


#ifdef TEST_PROGRAM_BUFFER

গণনা {
	PTR_AAA  = 0,
	PTR_BBB,
};

পূর্ণাঙ্ক main(শূন্য)
{
	সংরচনা ul_buffer buf = UL_INIT_BUFFER;
	ৱর্ণ *str;
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

	str = ul_buffer_get_data(&buf, &sz, রিক্ত);
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
	str = ul_buffer_get_data(&buf, &sz, রিক্ত);
	printf("data [%zu] '%s'\n", sz, str);
	printf(" pointer data len: AAA=%zu\n", ul_buffer_get_pointer_length(&buf, PTR_AAA));

	ul_buffer_free_data(&buf);
	str = strdup("foo");
	ul_buffer_refer_string(&buf, str);
	ul_buffer_append_data(&buf, ",", 1);
	ul_buffer_append_string(&buf, "bar");
	str = ul_buffer_get_data(&buf, &sz, রিক্ত);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_buffer_get_data(&buf, &sz, রিক্ত);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_append_char(&buf, 'x');
	{
		ৱর্ণ c = 'y';
		ul_buffer_append_char(&buf, c);
	}
	{
		ধ্রুৱ ৱর্ণ c = 'z';
		ul_buffer_append_char(&buf, c);
	}
	str = ul_buffer_get_data(&buf, &sz, রিক্ত);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);

	ঘূৰাই EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BUFFER */
