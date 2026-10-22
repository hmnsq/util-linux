/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#অন্তর্ভাব "buffer.h"
#অন্তর্ভাব "mbsalign.h"
#অন্তর্ভাব "strutils.h"

শূন্য ul_বাফার_reset_তথ্য(কাঠামো ul_বাফার *buf)
{
	যদি (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	যদি (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * আকার(অক্ষর *));
}

স্থির অন্তঃরেখ অক্ষর *ul_বাফার_free_তথ্য_common(কাঠামো ul_বাফার *buf, bool steal)
{
	অক্ষর *t = ফাঁকা;

	প্রতিজ্ঞা(buf);

	যদি (steal)
		t = buf->begin;
	অন্যথা
		মুক্ত(buf->begin);
	buf->begin = ফাঁকা;
	buf->end = ফাঁকা;
	buf->sz = 0;

	মুক্ত(buf->ptrs);
	buf->ptrs = ফাঁকা;
	buf->nptrs = 0;

	মুক্ত(buf->encoded);
	buf->encoded = ফাঁকা;
	buf->encoded_sz = 0;

	ফেরত t;
}

শূন্য ul_বাফার_free_তথ্য(কাঠামো ul_বাফার *buf)
{
	ul_বাফার_free_তথ্য_common(buf, false);
}

অক্ষর *ul_বাফার_steal_string(কাঠামো ul_বাফার *buf)
{
	যদি (ul_বাফার_is_empty(buf)) {
		ul_বাফার_free_তথ্য(buf);
		ফেরত ফাঁকা;
	}

	ফেরত ul_বাফার_free_তথ্য_common(buf, true);
}

শূন্য ul_বাফার_set_chunksize(কাঠামো ul_বাফার *buf, size_t sz)
{
	buf->chunksize = sz;
}

পূর্ণসংখ্যা ul_বাফার_is_empty(কাঠামো ul_বাফার *buf)
{
	ফেরত buf->begin == buf->end;
}

পূর্ণসংখ্যা ul_বাফার_save_pointer(কাঠামো ul_বাফার *buf, unsigned short ptr_idx)
{
	যদি (ptr_idx >= buf->nptrs) {
		অক্ষর **tmp = reallocarray(buf->ptrs, ptr_idx + 1, আকার(অক্ষর *));

		যদি (!tmp)
			ফেরত -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	ফেরত 0;
}


অক্ষর *ul_বাফার_get_pointer(কাঠামো ul_বাফার *buf, unsigned short ptr_idx)
{
	যদি (ptr_idx < buf->nptrs)
		ফেরত buf->ptrs[ptr_idx];
	ফেরত ফাঁকা;
}

/* returns length from begin to the pointer */
size_t ul_বাফার_get_pointer_দৈর্ঘ্য(কাঠামো ul_বাফার *buf, unsigned short ptr_idx)
{
	অক্ষর *ptr = ul_বাফার_get_pointer(buf, ptr_idx);

	যদি (ptr && ptr > buf->begin)
		ফেরত ptr - buf->begin;
	ফেরত 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_বাফার_get_safe_pointer_width(কাঠামো ul_বাফার *buf, unsigned short ptr_idx)
{
	size_t len = ul_বাফার_get_pointer_দৈর্ঘ্য(buf, ptr_idx);

	যদি (!len)
		ফেরত 0;

	ফেরত mbs_safe_nwidth(buf->begin, len, ফাঁকা);
}

শূন্য ul_বাফার_refer_string(কাঠামো ul_বাফার *buf, অক্ষর *str)
{
	যদি (buf->sz)
		ul_বাফার_free_তথ্য(buf);
	buf->begin = str;
	buf->sz = str ? strlen(str) : 0;
	buf->end = buf->begin ? buf->begin + buf->sz : buf->begin;
}

পূর্ণসংখ্যা ul_বাফার_alloc_তথ্য(কাঠামো ul_বাফার *buf, size_t sz)
{
	অক্ষর *tmp;
	size_t len = 0;

	প্রতিজ্ঞা(buf);

	যদি (sz <= buf->sz)
		ফেরত 0;

	যদি (buf->end && buf->begin)
		len = buf->end - buf->begin;

	যদি (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	যদি (!tmp)
		ফেরত -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	memset(buf->end, '\0', sz - len);

	ফেরত 0;
}

পূর্ণসংখ্যা ul_বাফার_append_তথ্য(কাঠামো ul_বাফার *buf, ধ্রুবক অক্ষর *তথ্য, size_t sz)
{
	size_t maxsz = 0;

	যদি (!buf)
		ফেরত -EINVAL;
	যদি (!তথ্য)
		ফেরত 0;

	যদি (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	যদি (maxsz <= sz + 1) {
		পূর্ণসংখ্যা rc = ul_বাফার_alloc_তথ্য(buf, buf->sz + sz + 1);
		যদি (rc)
			ফেরত rc;
	}
	যদি (!buf->end)
		ফেরত -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, তথ্য, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	ফেরত 0;
}

পূর্ণসংখ্যা ul_বাফার_append_char(কাঠামো ul_বাফার *buf, ধ্রুবক অক্ষর c)
{
	ফেরত ul_বাফার_append_তথ্য(buf, &c, 1);
}

পূর্ণসংখ্যা ul_বাফার_append_string(কাঠামো ul_বাফার *buf, ধ্রুবক অক্ষর *str)
{
	যদি (!str)
		ফেরত 0;

	ফেরত ul_বাফার_append_তথ্য(buf, str, strlen(str));
}

পূর্ণসংখ্যা ul_বাফার_append_ntimes(কাঠামো ul_বাফার *buf, size_t n, ধ্রুবক অক্ষর *str)
{
	size_t i;
	size_t len = strlen(str);

	জন্য (i = 0; len && i < n; i++) {
		পূর্ণসংখ্যা rc = ul_বাফার_append_তথ্য(buf, str, len);
		যদি (rc)
			ফেরত rc;
	}
	ফেরত 0;
}

পূর্ণসংখ্যা ul_বাফার_appendf(কাঠামো ul_বাফার *buf, ধ্রুবক অক্ষর *format, ...)
{
	va_list ap;
	পূর্ণসংখ্যা res;

	va_start(ap, format);
	res = ul_বাফার_appendvf(buf, format, ap);
	va_end(ap);

	ফেরত res;
}

পূর্ণসংখ্যা ul_বাফার_appendvf(কাঠামো ul_বাফার *buf, ধ্রুবক অক্ষর *format, va_list ap)
{
	অক্ষর *val;
	পূর্ণসংখ্যা sz;
	পূর্ণসংখ্যা res;

	sz = vasprintf(&val, format, ap);
	যদি (sz < 0)
		ফেরত -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_বাফার_append_তথ্য(buf, val, sz);
	মুক্ত(val);

	ফেরত res;
}

পূর্ণসংখ্যা ul_বাফার_set_তথ্য(কাঠামো ul_বাফার *buf, ধ্রুবক অক্ষর *তথ্য, size_t sz)
{
	ul_বাফার_reset_তথ্য(buf);
	ফেরত ul_বাফার_append_তথ্য(buf, তথ্য, sz);
}

অক্ষর *ul_বাফার_get_তথ্য(কাঠামো ul_বাফার *buf, size_t *sz, size_t *width)
{
	যদি (sz)
		*sz = buf->end - buf->begin;
	যদি (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	ফেরত buf->begin;
}

অক্ষর *ul_বাফার_get_string(কাঠামো ul_বাফার *buf, size_t *sz, size_t *width)
{
	অক্ষর *ret;

	ret = ul_বাফার_get_তথ্য(buf, sz, width);

	/* data in buffer is already zero-terminated */
	যদি (sz)
		*sz = *sz + 1;

	ফেরত ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_বাফার_get_bufsiz(কাঠামো ul_বাফার *buf)
{
	ফেরত buf->sz;
}

size_t ul_বাফার_get_datasiz(কাঠামো ul_বাফার *buf)
{
	ফেরত buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
অক্ষর *ul_বাফার_get_safe_তথ্য(কাঠামো ul_বাফার *buf, size_t *sz, size_t *width, ধ্রুবক অক্ষর *safechars)
{
	অক্ষর *তথ্য = ul_বাফার_get_তথ্য(buf, ফাঁকা, ফাঁকা);
	size_t encsz, wsz = 0;
	অক্ষর *res = ফাঁকা;

	যদি (!তথ্য)
		যাও nothing;

	encsz = mbs_safe_encode_আকার(buf->sz) + 1;
	যদি (encsz > buf->encoded_sz) {
		অক্ষর *tmp = realloc(buf->encoded, encsz);
		যদি (!tmp)
			যাও nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_বাফার(তথ্য, &wsz, buf->encoded, buf->encoded_sz, safechars);
	যদি (!res || !wsz || wsz == (size_t) -1)
		যাও nothing;

	যদি (width)
		*width = wsz;
	যদি (sz)
		*sz = strlen(res);
	ফেরত res;
nothing:
	যদি (width)
		*width = 0;
	যদি (sz)
		*sz = 0;
	ফেরত ফাঁকা;
}


#ifdef TEST_PROGRAM_বাফার

গণনা {
	PTR_AAA  = 0,
	PTR_BBB,
};

পূর্ণসংখ্যা প্রধান(শূন্য)
{
	কাঠামো ul_বাফার buf = UL_INIT_বাফার;
	অক্ষর *str;
	size_t sz = 0;

	ul_বাফার_set_chunksize(&buf, 16);

	ul_বাফার_append_string(&buf, "AAA");
	ul_বাফার_append_তথ্য(&buf, "=", 1);
	ul_বাফার_append_string(&buf, "aaa");
	ul_বাফার_save_pointer(&buf, PTR_AAA);

	ul_বাফার_append_তথ্য(&buf, ",", 1);
	ul_বাফার_append_string(&buf, "BBB");
	ul_বাফার_append_string(&buf, "=");
	ul_বাফার_append_string(&buf, "bbb");
	ul_বাফার_save_pointer(&buf, PTR_BBB);

	str = ul_বাফার_get_তথ্য(&buf, &sz, ফাঁকা);
	printf("data [%zu] '%s'\n", sz, str);

	printf(" pointer data len: AAA=%zu, BBB=%zu\n",
			ul_বাফার_get_pointer_দৈর্ঘ্য(&buf, PTR_AAA),
			ul_বাফার_get_pointer_দৈর্ঘ্য(&buf, PTR_BBB));
	printf(" pointer data width: AAA=%zu, BBB=%zu\n",
			ul_বাফার_get_safe_pointer_width(&buf, PTR_AAA),
			ul_বাফার_get_safe_pointer_width(&buf, PTR_BBB));

	ul_বাফার_reset_তথ্য(&buf);
	ul_বাফার_append_string(&buf, "This is really long string to test the buffer function.");
	ul_বাফার_save_pointer(&buf, PTR_AAA);
	ul_বাফার_append_string(&buf, " YES!");
	str = ul_বাফার_get_তথ্য(&buf, &sz, ফাঁকা);
	printf("data [%zu] '%s'\n", sz, str);
	printf(" pointer data len: AAA=%zu\n", ul_বাফার_get_pointer_দৈর্ঘ্য(&buf, PTR_AAA));

	ul_বাফার_free_তথ্য(&buf);
	str = strdup("foo");
	ul_বাফার_refer_string(&buf, str);
	ul_বাফার_append_তথ্য(&buf, ",", 1);
	ul_বাফার_append_string(&buf, "bar");
	str = ul_বাফার_get_তথ্য(&buf, &sz, ফাঁকা);
	printf("data [%zu] '%s'\n", sz, str);

	ul_বাফার_free_তথ্য(&buf);
	ul_বাফার_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_বাফার_get_তথ্য(&buf, &sz, ফাঁকা);
	printf("data [%zu] '%s'\n", sz, str);

	ul_বাফার_free_তথ্য(&buf);
	ul_বাফার_append_char(&buf, 'x');
	{
		অক্ষর c = 'y';
		ul_বাফার_append_char(&buf, c);
	}
	{
		ধ্রুবক অক্ষর c = 'z';
		ul_বাফার_append_char(&buf, c);
	}
	str = ul_বাফার_get_তথ্য(&buf, &sz, ফাঁকা);
	printf("data [%zu] '%s'\n", sz, str);

	ul_বাফার_free_তথ্য(&buf);

	ফেরত EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BUFFER */
