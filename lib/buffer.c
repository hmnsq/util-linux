/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#અન્તર્ભાવ "buffer.h"
#અન્તર્ભાવ "mbsalign.h"
#અન્તર્ભાવ "strutils.h"

શૂન્ય ul_buffer_reset_data(સંરચના ul_buffer *buf)
{
	જો (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	જો (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * કદ(અક્ષર *));
}

સ્થિર અંતઃરેખ અક્ષર *ul_buffer_free_data_common(સંરચના ul_buffer *buf, bool steal)
{
	અક્ષર *t = ખાલી;

	પ્રતિજ્ઞા(buf);

	જો (steal)
		t = buf->begin;
	નહીંતર
		મુક્ત(buf->begin);
	buf->begin = ખાલી;
	buf->end = ખાલી;
	buf->sz = 0;

	મુક્ત(buf->ptrs);
	buf->ptrs = ખાલી;
	buf->nptrs = 0;

	મુક્ત(buf->encoded);
	buf->encoded = ખાલી;
	buf->encoded_sz = 0;

	પાછા t;
}

શૂન્ય ul_buffer_free_data(સંરચના ul_buffer *buf)
{
	ul_buffer_free_data_common(buf, false);
}

અક્ષર *ul_buffer_steal_string(સંરચના ul_buffer *buf)
{
	જો (ul_buffer_is_empty(buf)) {
		ul_buffer_free_data(buf);
		પાછા ખાલી;
	}

	પાછા ul_buffer_free_data_common(buf, true);
}

શૂન્ય ul_buffer_set_chunksize(સંરચના ul_buffer *buf, size_t sz)
{
	buf->chunksize = sz;
}

પૂર્ણાંક ul_buffer_is_empty(સંરચના ul_buffer *buf)
{
	પાછા buf->begin == buf->end;
}

પૂર્ણાંક ul_buffer_save_pointer(સંરચના ul_buffer *buf, unsigned short ptr_idx)
{
	જો (ptr_idx >= buf->nptrs) {
		અક્ષર **tmp = reallocarray(buf->ptrs, ptr_idx + 1, કદ(અક્ષર *));

		જો (!tmp)
			પાછા -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	પાછા 0;
}


અક્ષર *ul_buffer_get_pointer(સંરચના ul_buffer *buf, unsigned short ptr_idx)
{
	જો (ptr_idx < buf->nptrs)
		પાછા buf->ptrs[ptr_idx];
	પાછા ખાલી;
}

/* returns length from begin to the pointer */
size_t ul_buffer_get_pointer_length(સંરચના ul_buffer *buf, unsigned short ptr_idx)
{
	અક્ષર *ptr = ul_buffer_get_pointer(buf, ptr_idx);

	જો (ptr && ptr > buf->begin)
		પાછા ptr - buf->begin;
	પાછા 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_buffer_get_safe_pointer_width(સંરચના ul_buffer *buf, unsigned short ptr_idx)
{
	size_t len = ul_buffer_get_pointer_length(buf, ptr_idx);

	જો (!len)
		પાછા 0;

	પાછા mbs_safe_nwidth(buf->begin, len, ખાલી);
}

શૂન્ય ul_buffer_refer_string(સંરચના ul_buffer *buf, અક્ષર *str)
{
	જો (buf->sz)
		ul_buffer_free_data(buf);
	buf->begin = str;
	buf->sz = str ? strlen(str) : 0;
	buf->end = buf->begin ? buf->begin + buf->sz : buf->begin;
}

પૂર્ણાંક ul_buffer_alloc_data(સંરચના ul_buffer *buf, size_t sz)
{
	અક્ષર *tmp;
	size_t len = 0;

	પ્રતિજ્ઞા(buf);

	જો (sz <= buf->sz)
		પાછા 0;

	જો (buf->end && buf->begin)
		len = buf->end - buf->begin;

	જો (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	જો (!tmp)
		પાછા -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	memset(buf->end, '\0', sz - len);

	પાછા 0;
}

પૂર્ણાંક ul_buffer_append_data(સંરચના ul_buffer *buf, અચલ અક્ષર *data, size_t sz)
{
	size_t maxsz = 0;

	જો (!buf)
		પાછા -EINVAL;
	જો (!data)
		પાછા 0;

	જો (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	જો (maxsz <= sz + 1) {
		પૂર્ણાંક rc = ul_buffer_alloc_data(buf, buf->sz + sz + 1);
		જો (rc)
			પાછા rc;
	}
	જો (!buf->end)
		પાછા -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, data, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	પાછા 0;
}

પૂર્ણાંક ul_buffer_append_char(સંરચના ul_buffer *buf, અચલ અક્ષર c)
{
	પાછા ul_buffer_append_data(buf, &c, 1);
}

પૂર્ણાંક ul_buffer_append_string(સંરચના ul_buffer *buf, અચલ અક્ષર *str)
{
	જો (!str)
		પાછા 0;

	પાછા ul_buffer_append_data(buf, str, strlen(str));
}

પૂર્ણાંક ul_buffer_append_ntimes(સંરચના ul_buffer *buf, size_t n, અચલ અક્ષર *str)
{
	size_t i;
	size_t len = strlen(str);

	માટે (i = 0; len && i < n; i++) {
		પૂર્ણાંક rc = ul_buffer_append_data(buf, str, len);
		જો (rc)
			પાછા rc;
	}
	પાછા 0;
}

પૂર્ણાંક ul_buffer_appendf(સંરચના ul_buffer *buf, અચલ અક્ષર *format, ...)
{
	va_list ap;
	પૂર્ણાંક res;

	va_start(ap, format);
	res = ul_buffer_appendvf(buf, format, ap);
	va_end(ap);

	પાછા res;
}

પૂર્ણાંક ul_buffer_appendvf(સંરચના ul_buffer *buf, અચલ અક્ષર *format, va_list ap)
{
	અક્ષર *val;
	પૂર્ણાંક sz;
	પૂર્ણાંક res;

	sz = vasprintf(&val, format, ap);
	જો (sz < 0)
		પાછા -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_buffer_append_data(buf, val, sz);
	મુક્ત(val);

	પાછા res;
}

પૂર્ણાંક ul_buffer_set_data(સંરચના ul_buffer *buf, અચલ અક્ષર *data, size_t sz)
{
	ul_buffer_reset_data(buf);
	પાછા ul_buffer_append_data(buf, data, sz);
}

અક્ષર *ul_buffer_get_data(સંરચના ul_buffer *buf, size_t *sz, size_t *width)
{
	જો (sz)
		*sz = buf->end - buf->begin;
	જો (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	પાછા buf->begin;
}

અક્ષર *ul_buffer_get_string(સંરચના ul_buffer *buf, size_t *sz, size_t *width)
{
	અક્ષર *ret;

	ret = ul_buffer_get_data(buf, sz, width);

	/* data in buffer is already zero-terminated */
	જો (sz)
		*sz = *sz + 1;

	પાછા ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_buffer_get_bufsiz(સંરચના ul_buffer *buf)
{
	પાછા buf->sz;
}

size_t ul_buffer_get_datasiz(સંરચના ul_buffer *buf)
{
	પાછા buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
અક્ષર *ul_buffer_get_safe_data(સંરચના ul_buffer *buf, size_t *sz, size_t *width, અચલ અક્ષર *safechars)
{
	અક્ષર *data = ul_buffer_get_data(buf, ખાલી, ખાલી);
	size_t encsz, wsz = 0;
	અક્ષર *res = ખાલી;

	જો (!data)
		જાઓ nothing;

	encsz = mbs_safe_encode_size(buf->sz) + 1;
	જો (encsz > buf->encoded_sz) {
		અક્ષર *tmp = realloc(buf->encoded, encsz);
		જો (!tmp)
			જાઓ nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_buffer(data, &wsz, buf->encoded, buf->encoded_sz, safechars);
	જો (!res || !wsz || wsz == (size_t) -1)
		જાઓ nothing;

	જો (width)
		*width = wsz;
	જો (sz)
		*sz = strlen(res);
	પાછા res;
nothing:
	જો (width)
		*width = 0;
	જો (sz)
		*sz = 0;
	પાછા ખાલી;
}


#ifdef TEST_PROGRAM_BUFFER

ગણના {
	PTR_AAA  = 0,
	PTR_BBB,
};

પૂર્ણાંક મુખ્ય(શૂન્ય)
{
	સંરચના ul_buffer buf = UL_INIT_BUFFER;
	અક્ષર *str;
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

	str = ul_buffer_get_data(&buf, &sz, ખાલી);
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
	str = ul_buffer_get_data(&buf, &sz, ખાલી);
	printf("data [%zu] '%s'\n", sz, str);
	printf(" pointer data len: AAA=%zu\n", ul_buffer_get_pointer_length(&buf, PTR_AAA));

	ul_buffer_free_data(&buf);
	str = strdup("foo");
	ul_buffer_refer_string(&buf, str);
	ul_buffer_append_data(&buf, ",", 1);
	ul_buffer_append_string(&buf, "bar");
	str = ul_buffer_get_data(&buf, &sz, ખાલી);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_buffer_get_data(&buf, &sz, ખાલી);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_append_char(&buf, 'x');
	{
		અક્ષર c = 'y';
		ul_buffer_append_char(&buf, c);
	}
	{
		અચલ અક્ષર c = 'z';
		ul_buffer_append_char(&buf, c);
	}
	str = ul_buffer_get_data(&buf, &sz, ખાલી);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);

	પાછા EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BUFFER */
