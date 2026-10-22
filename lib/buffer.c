/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#അന്തര്ഭാവ "buffer.h"
#അന്തര്ഭാവ "mbsalign.h"
#അന്തര്ഭാവ "strutils.h"

ശൂന്യം ul_buffer_reset_data(ഘടന ul_buffer *buf)
{
	എങ്കിൽ (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	എങ്കിൽ (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * വലുപ്പം(അക്ഷരം *));
}

സ്ഥിതം വരിയിൽ അക്ഷരം *ul_buffer_free_data_common(ഘടന ul_buffer *buf, bool steal)
{
	അക്ഷരം *t = ശൂന്യ;

	പ്രതിജ്ഞാ(buf);

	എങ്കിൽ (steal)
		t = buf->begin;
	അല്ലെങ്കിൽ
		സ്വതന്ത്രം(buf->begin);
	buf->begin = ശൂന്യ;
	buf->end = ശൂന്യ;
	buf->sz = 0;

	സ്വതന്ത്രം(buf->ptrs);
	buf->ptrs = ശൂന്യ;
	buf->nptrs = 0;

	സ്വതന്ത്രം(buf->encoded);
	buf->encoded = ശൂന്യ;
	buf->encoded_sz = 0;

	തിരികെ t;
}

ശൂന്യം ul_buffer_free_data(ഘടന ul_buffer *buf)
{
	ul_buffer_free_data_common(buf, false);
}

അക്ഷരം *ul_buffer_steal_string(ഘടന ul_buffer *buf)
{
	എങ്കിൽ (ul_buffer_is_empty(buf)) {
		ul_buffer_free_data(buf);
		തിരികെ ശൂന്യ;
	}

	തിരികെ ul_buffer_free_data_common(buf, true);
}

ശൂന്യം ul_buffer_set_chunksize(ഘടന ul_buffer *buf, size_t sz)
{
	buf->chunksize = sz;
}

പൂർണ്ണസംഖ്യ ul_buffer_is_empty(ഘടന ul_buffer *buf)
{
	തിരികെ buf->begin == buf->end;
}

പൂർണ്ണസംഖ്യ ul_buffer_save_pointer(ഘടന ul_buffer *buf, unsigned short ptr_idx)
{
	എങ്കിൽ (ptr_idx >= buf->nptrs) {
		അക്ഷരം **tmp = reallocarray(buf->ptrs, ptr_idx + 1, വലുപ്പം(അക്ഷരം *));

		എങ്കിൽ (!tmp)
			തിരികെ -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	തിരികെ 0;
}


അക്ഷരം *ul_buffer_get_pointer(ഘടന ul_buffer *buf, unsigned short ptr_idx)
{
	എങ്കിൽ (ptr_idx < buf->nptrs)
		തിരികെ buf->ptrs[ptr_idx];
	തിരികെ ശൂന്യ;
}

/* returns length from begin to the pointer */
size_t ul_buffer_get_pointer_length(ഘടന ul_buffer *buf, unsigned short ptr_idx)
{
	അക്ഷരം *ptr = ul_buffer_get_pointer(buf, ptr_idx);

	എങ്കിൽ (ptr && ptr > buf->begin)
		തിരികെ ptr - buf->begin;
	തിരികെ 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_buffer_get_safe_pointer_width(ഘടന ul_buffer *buf, unsigned short ptr_idx)
{
	size_t len = ul_buffer_get_pointer_length(buf, ptr_idx);

	എങ്കിൽ (!len)
		തിരികെ 0;

	തിരികെ mbs_safe_nwidth(buf->begin, len, ശൂന്യ);
}

ശൂന്യം ul_buffer_refer_string(ഘടന ul_buffer *buf, അക്ഷരം *str)
{
	എങ്കിൽ (buf->sz)
		ul_buffer_free_data(buf);
	buf->begin = str;
	buf->sz = str ? strlen(str) : 0;
	buf->end = buf->begin ? buf->begin + buf->sz : buf->begin;
}

പൂർണ്ണസംഖ്യ ul_buffer_alloc_data(ഘടന ul_buffer *buf, size_t sz)
{
	അക്ഷരം *tmp;
	size_t len = 0;

	പ്രതിജ്ഞാ(buf);

	എങ്കിൽ (sz <= buf->sz)
		തിരികെ 0;

	എങ്കിൽ (buf->end && buf->begin)
		len = buf->end - buf->begin;

	എങ്കിൽ (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	എങ്കിൽ (!tmp)
		തിരികെ -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	memset(buf->end, '\0', sz - len);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ ul_buffer_append_data(ഘടന ul_buffer *buf, സ്ഥിരം അക്ഷരം *data, size_t sz)
{
	size_t maxsz = 0;

	എങ്കിൽ (!buf)
		തിരികെ -EINVAL;
	എങ്കിൽ (!data)
		തിരികെ 0;

	എങ്കിൽ (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	എങ്കിൽ (maxsz <= sz + 1) {
		പൂർണ്ണസംഖ്യ rc = ul_buffer_alloc_data(buf, buf->sz + sz + 1);
		എങ്കിൽ (rc)
			തിരികെ rc;
	}
	എങ്കിൽ (!buf->end)
		തിരികെ -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, data, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ ul_buffer_append_char(ഘടന ul_buffer *buf, സ്ഥിരം അക്ഷരം c)
{
	തിരികെ ul_buffer_append_data(buf, &c, 1);
}

പൂർണ്ണസംഖ്യ ul_buffer_append_string(ഘടന ul_buffer *buf, സ്ഥിരം അക്ഷരം *str)
{
	എങ്കിൽ (!str)
		തിരികെ 0;

	തിരികെ ul_buffer_append_data(buf, str, strlen(str));
}

പൂർണ്ണസംഖ്യ ul_buffer_append_ntimes(ഘടന ul_buffer *buf, size_t n, സ്ഥിരം അക്ഷരം *str)
{
	size_t i;
	size_t len = strlen(str);

	വേണ്ടി (i = 0; len && i < n; i++) {
		പൂർണ്ണസംഖ്യ rc = ul_buffer_append_data(buf, str, len);
		എങ്കിൽ (rc)
			തിരികെ rc;
	}
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ ul_buffer_appendf(ഘടന ul_buffer *buf, സ്ഥിരം അക്ഷരം *format, ...)
{
	va_list ap;
	പൂർണ്ണസംഖ്യ res;

	va_start(ap, format);
	res = ul_buffer_appendvf(buf, format, ap);
	va_end(ap);

	തിരികെ res;
}

പൂർണ്ണസംഖ്യ ul_buffer_appendvf(ഘടന ul_buffer *buf, സ്ഥിരം അക്ഷരം *format, va_list ap)
{
	അക്ഷരം *val;
	പൂർണ്ണസംഖ്യ sz;
	പൂർണ്ണസംഖ്യ res;

	sz = vasprintf(&val, format, ap);
	എങ്കിൽ (sz < 0)
		തിരികെ -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_buffer_append_data(buf, val, sz);
	സ്വതന്ത്രം(val);

	തിരികെ res;
}

പൂർണ്ണസംഖ്യ ul_buffer_set_data(ഘടന ul_buffer *buf, സ്ഥിരം അക്ഷരം *data, size_t sz)
{
	ul_buffer_reset_data(buf);
	തിരികെ ul_buffer_append_data(buf, data, sz);
}

അക്ഷരം *ul_buffer_get_data(ഘടന ul_buffer *buf, size_t *sz, size_t *width)
{
	എങ്കിൽ (sz)
		*sz = buf->end - buf->begin;
	എങ്കിൽ (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	തിരികെ buf->begin;
}

അക്ഷരം *ul_buffer_get_string(ഘടന ul_buffer *buf, size_t *sz, size_t *width)
{
	അക്ഷരം *ret;

	ret = ul_buffer_get_data(buf, sz, width);

	/* data in buffer is already zero-terminated */
	എങ്കിൽ (sz)
		*sz = *sz + 1;

	തിരികെ ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_buffer_get_bufsiz(ഘടന ul_buffer *buf)
{
	തിരികെ buf->sz;
}

size_t ul_buffer_get_datasiz(ഘടന ul_buffer *buf)
{
	തിരികെ buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
അക്ഷരം *ul_buffer_get_safe_data(ഘടന ul_buffer *buf, size_t *sz, size_t *width, സ്ഥിരം അക്ഷരം *safechars)
{
	അക്ഷരം *data = ul_buffer_get_data(buf, ശൂന്യ, ശൂന്യ);
	size_t encsz, wsz = 0;
	അക്ഷരം *res = ശൂന്യ;

	എങ്കിൽ (!data)
		പോകുക nothing;

	encsz = mbs_safe_encode_size(buf->sz) + 1;
	എങ്കിൽ (encsz > buf->encoded_sz) {
		അക്ഷരം *tmp = realloc(buf->encoded, encsz);
		എങ്കിൽ (!tmp)
			പോകുക nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_buffer(data, &wsz, buf->encoded, buf->encoded_sz, safechars);
	എങ്കിൽ (!res || !wsz || wsz == (size_t) -1)
		പോകുക nothing;

	എങ്കിൽ (width)
		*width = wsz;
	എങ്കിൽ (sz)
		*sz = strlen(res);
	തിരികെ res;
nothing:
	എങ്കിൽ (width)
		*width = 0;
	എങ്കിൽ (sz)
		*sz = 0;
	തിരികെ ശൂന്യ;
}


#ifdef TEST_PROGRAM_BUFFER

എണ്ണൽ {
	PTR_AAA  = 0,
	PTR_BBB,
};

പൂർണ്ണസംഖ്യ പ്രധാന(ശൂന്യം)
{
	ഘടന ul_buffer buf = UL_INIT_BUFFER;
	അക്ഷരം *str;
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

	str = ul_buffer_get_data(&buf, &sz, ശൂന്യ);
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
	str = ul_buffer_get_data(&buf, &sz, ശൂന്യ);
	printf("data [%zu] '%s'\n", sz, str);
	printf(" pointer data len: AAA=%zu\n", ul_buffer_get_pointer_length(&buf, PTR_AAA));

	ul_buffer_free_data(&buf);
	str = strdup("foo");
	ul_buffer_refer_string(&buf, str);
	ul_buffer_append_data(&buf, ",", 1);
	ul_buffer_append_string(&buf, "bar");
	str = ul_buffer_get_data(&buf, &sz, ശൂന്യ);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_buffer_get_data(&buf, &sz, ശൂന്യ);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_append_char(&buf, 'x');
	{
		അക്ഷരം c = 'y';
		ul_buffer_append_char(&buf, c);
	}
	{
		സ്ഥിരം അക്ഷരം c = 'z';
		ul_buffer_append_char(&buf, c);
	}
	str = ul_buffer_get_data(&buf, &sz, ശൂന്യ);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);

	തിരികെ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BUFFER */
