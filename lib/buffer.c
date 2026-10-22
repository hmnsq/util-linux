/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ଅନ୍ତର୍ଭାଵ "buffer.h"
#ଅନ୍ତର୍ଭାଵ "mbsalign.h"
#ଅନ୍ତର୍ଭାଵ "strutils.h"

ଶୂନ୍ୟ ul_buffer_reset_data(ସଂରଚନା ul_buffer *buf)
{
	ଯଦି (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	ଯଦି (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * ଆକାର(ଅକ୍ଷର *));
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଅକ୍ଷର *ul_buffer_free_data_common(ସଂରଚନା ul_buffer *buf, bool steal)
{
	ଅକ୍ଷର *t = ଖାଲି;

	ପ୍ରତିଜ୍ଞା(buf);

	ଯଦି (steal)
		t = buf->begin;
	ଅନ୍ୟଥା
		ମୁକ୍ତ(buf->begin);
	buf->begin = ଖାଲି;
	buf->end = ଖାଲି;
	buf->sz = 0;

	ମୁକ୍ତ(buf->ptrs);
	buf->ptrs = ଖାଲି;
	buf->nptrs = 0;

	ମୁକ୍ତ(buf->encoded);
	buf->encoded = ଖାଲି;
	buf->encoded_sz = 0;

	ଫେରନ୍ତୁ t;
}

ଶୂନ୍ୟ ul_buffer_free_data(ସଂରଚନା ul_buffer *buf)
{
	ul_buffer_free_data_common(buf, false);
}

ଅକ୍ଷର *ul_buffer_steal_string(ସଂରଚନା ul_buffer *buf)
{
	ଯଦି (ul_buffer_is_empty(buf)) {
		ul_buffer_free_data(buf);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ ul_buffer_free_data_common(buf, true);
}

ଶୂନ୍ୟ ul_buffer_set_chunksize(ସଂରଚନା ul_buffer *buf, size_t sz)
{
	buf->chunksize = sz;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_buffer_is_empty(ସଂରଚନା ul_buffer *buf)
{
	ଫେରନ୍ତୁ buf->begin == buf->end;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_buffer_save_pointer(ସଂରଚନା ul_buffer *buf, unsigned short ptr_idx)
{
	ଯଦି (ptr_idx >= buf->nptrs) {
		ଅକ୍ଷର **tmp = reallocarray(buf->ptrs, ptr_idx + 1, ଆକାର(ଅକ୍ଷର *));

		ଯଦି (!tmp)
			ଫେରନ୍ତୁ -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	ଫେରନ୍ତୁ 0;
}


ଅକ୍ଷର *ul_buffer_get_pointer(ସଂରଚନା ul_buffer *buf, unsigned short ptr_idx)
{
	ଯଦି (ptr_idx < buf->nptrs)
		ଫେରନ୍ତୁ buf->ptrs[ptr_idx];
	ଫେରନ୍ତୁ ଖାଲି;
}

/* returns length from begin to the pointer */
size_t ul_buffer_get_pointer_length(ସଂରଚନା ul_buffer *buf, unsigned short ptr_idx)
{
	ଅକ୍ଷର *ptr = ul_buffer_get_pointer(buf, ptr_idx);

	ଯଦି (ptr && ptr > buf->begin)
		ଫେରନ୍ତୁ ptr - buf->begin;
	ଫେରନ୍ତୁ 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_buffer_get_safe_pointer_width(ସଂରଚନା ul_buffer *buf, unsigned short ptr_idx)
{
	size_t len = ul_buffer_get_pointer_length(buf, ptr_idx);

	ଯଦି (!len)
		ଫେରନ୍ତୁ 0;

	ଫେରନ୍ତୁ mbs_safe_nwidth(buf->begin, len, ଖାଲି);
}

ଶୂନ୍ୟ ul_buffer_refer_string(ସଂରଚନା ul_buffer *buf, ଅକ୍ଷର *str)
{
	ଯଦି (buf->sz)
		ul_buffer_free_data(buf);
	buf->begin = str;
	buf->sz = str ? strlen(str) : 0;
	buf->end = buf->begin ? buf->begin + buf->sz : buf->begin;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_buffer_alloc_data(ସଂରଚନା ul_buffer *buf, size_t sz)
{
	ଅକ୍ଷର *tmp;
	size_t len = 0;

	ପ୍ରତିଜ୍ଞା(buf);

	ଯଦି (sz <= buf->sz)
		ଫେରନ୍ତୁ 0;

	ଯଦି (buf->end && buf->begin)
		len = buf->end - buf->begin;

	ଯଦି (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	ଯଦି (!tmp)
		ଫେରନ୍ତୁ -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	memset(buf->end, '\0', sz - len);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_buffer_append_data(ସଂରଚନା ul_buffer *buf, ଅଚଳ ଅକ୍ଷର *data, size_t sz)
{
	size_t maxsz = 0;

	ଯଦି (!buf)
		ଫେରନ୍ତୁ -EINVAL;
	ଯଦି (!data)
		ଫେରନ୍ତୁ 0;

	ଯଦି (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	ଯଦି (maxsz <= sz + 1) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = ul_buffer_alloc_data(buf, buf->sz + sz + 1);
		ଯଦି (rc)
			ଫେରନ୍ତୁ rc;
	}
	ଯଦି (!buf->end)
		ଫେରନ୍ତୁ -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, data, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_buffer_append_char(ସଂରଚନା ul_buffer *buf, ଅଚଳ ଅକ୍ଷର c)
{
	ଫେରନ୍ତୁ ul_buffer_append_data(buf, &c, 1);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_buffer_append_string(ସଂରଚନା ul_buffer *buf, ଅଚଳ ଅକ୍ଷର *str)
{
	ଯଦି (!str)
		ଫେରନ୍ତୁ 0;

	ଫେରନ୍ତୁ ul_buffer_append_data(buf, str, strlen(str));
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_buffer_append_ntimes(ସଂରଚନା ul_buffer *buf, size_t n, ଅଚଳ ଅକ୍ଷର *str)
{
	size_t i;
	size_t len = strlen(str);

	ପାଇଁ (i = 0; len && i < n; i++) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = ul_buffer_append_data(buf, str, len);
		ଯଦି (rc)
			ଫେରନ୍ତୁ rc;
	}
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_buffer_appendf(ସଂରଚନା ul_buffer *buf, ଅଚଳ ଅକ୍ଷର *format, ...)
{
	va_list ap;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;

	va_start(ap, format);
	res = ul_buffer_appendvf(buf, format, ap);
	va_end(ap);

	ଫେରନ୍ତୁ res;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_buffer_appendvf(ସଂରଚନା ul_buffer *buf, ଅଚଳ ଅକ୍ଷର *format, va_list ap)
{
	ଅକ୍ଷର *val;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;

	sz = vasprintf(&val, format, ap);
	ଯଦି (sz < 0)
		ଫେରନ୍ତୁ -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_buffer_append_data(buf, val, sz);
	ମୁକ୍ତ(val);

	ଫେରନ୍ତୁ res;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_buffer_set_data(ସଂରଚନା ul_buffer *buf, ଅଚଳ ଅକ୍ଷର *data, size_t sz)
{
	ul_buffer_reset_data(buf);
	ଫେରନ୍ତୁ ul_buffer_append_data(buf, data, sz);
}

ଅକ୍ଷର *ul_buffer_get_data(ସଂରଚନା ul_buffer *buf, size_t *sz, size_t *width)
{
	ଯଦି (sz)
		*sz = buf->end - buf->begin;
	ଯଦି (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	ଫେରନ୍ତୁ buf->begin;
}

ଅକ୍ଷର *ul_buffer_get_string(ସଂରଚନା ul_buffer *buf, size_t *sz, size_t *width)
{
	ଅକ୍ଷର *ret;

	ret = ul_buffer_get_data(buf, sz, width);

	/* data in buffer is already zero-terminated */
	ଯଦି (sz)
		*sz = *sz + 1;

	ଫେରନ୍ତୁ ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_buffer_get_bufsiz(ସଂରଚନା ul_buffer *buf)
{
	ଫେରନ୍ତୁ buf->sz;
}

size_t ul_buffer_get_datasiz(ସଂରଚନା ul_buffer *buf)
{
	ଫେରନ୍ତୁ buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
ଅକ୍ଷର *ul_buffer_get_safe_data(ସଂରଚନା ul_buffer *buf, size_t *sz, size_t *width, ଅଚଳ ଅକ୍ଷର *safechars)
{
	ଅକ୍ଷର *data = ul_buffer_get_data(buf, ଖାଲି, ଖାଲି);
	size_t encsz, wsz = 0;
	ଅକ୍ଷର *res = ଖାଲି;

	ଯଦି (!data)
		ଯାଅ nothing;

	encsz = mbs_safe_encode_size(buf->sz) + 1;
	ଯଦି (encsz > buf->encoded_sz) {
		ଅକ୍ଷର *tmp = realloc(buf->encoded, encsz);
		ଯଦି (!tmp)
			ଯାଅ nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_buffer(data, &wsz, buf->encoded, buf->encoded_sz, safechars);
	ଯଦି (!res || !wsz || wsz == (size_t) -1)
		ଯାଅ nothing;

	ଯଦି (width)
		*width = wsz;
	ଯଦି (sz)
		*sz = strlen(res);
	ଫେରନ୍ତୁ res;
nothing:
	ଯଦି (width)
		*width = 0;
	ଯଦି (sz)
		*sz = 0;
	ଫେରନ୍ତୁ ଖାଲି;
}


#ifdef TEST_PROGRAM_BUFFER

ଗଣନା {
	PTR_AAA  = 0,
	PTR_BBB,
};

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ଶୂନ୍ୟ)
{
	ସଂରଚନା ul_buffer buf = UL_INIT_BUFFER;
	ଅକ୍ଷର *str;
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

	str = ul_buffer_get_data(&buf, &sz, ଖାଲି);
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
	str = ul_buffer_get_data(&buf, &sz, ଖାଲି);
	printf("data [%zu] '%s'\n", sz, str);
	printf(" pointer data len: AAA=%zu\n", ul_buffer_get_pointer_length(&buf, PTR_AAA));

	ul_buffer_free_data(&buf);
	str = strdup("foo");
	ul_buffer_refer_string(&buf, str);
	ul_buffer_append_data(&buf, ",", 1);
	ul_buffer_append_string(&buf, "bar");
	str = ul_buffer_get_data(&buf, &sz, ଖାଲି);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_buffer_get_data(&buf, &sz, ଖାଲି);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_append_char(&buf, 'x');
	{
		ଅକ୍ଷର c = 'y';
		ul_buffer_append_char(&buf, c);
	}
	{
		ଅଚଳ ଅକ୍ଷର c = 'z';
		ul_buffer_append_char(&buf, c);
	}
	str = ul_buffer_get_data(&buf, &sz, ଖାଲି);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BUFFER */
