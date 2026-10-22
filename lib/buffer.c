/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ਅਨ੍ਤਰ੍ਭਾਵ "buffer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "mbsalign.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "strutils.h"

ਖਾਲੀ ul_ਬਫਰ_reset_ਅੰਕੜੇ(ਬਣਤਰ ul_ਬਫਰ *buf)
{
	ਜੇ (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->ਸਿਰਾ = buf->begin;

	ਜੇ (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * ਆਕਾਰ(ਅੱਖਰ *));
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਅੱਖਰ *ul_ਬਫਰ_free_ਅੰਕੜੇ_common(ਬਣਤਰ ul_ਬਫਰ *buf, bool steal)
{
	ਅੱਖਰ *t = ਨਿਰਾ;

	ਪ੍ਰਤਿਜ੍ਞਾ(buf);

	ਜੇ (steal)
		t = buf->begin;
	ਨਹੀਂਤਾਂ
		ਮੁਕਤ(buf->begin);
	buf->begin = ਨਿਰਾ;
	buf->ਸਿਰਾ = ਨਿਰਾ;
	buf->sz = 0;

	ਮੁਕਤ(buf->ptrs);
	buf->ptrs = ਨਿਰਾ;
	buf->nptrs = 0;

	ਮੁਕਤ(buf->encoded);
	buf->encoded = ਨਿਰਾ;
	buf->encoded_sz = 0;

	ਵਾਪਸ t;
}

ਖਾਲੀ ul_ਬਫਰ_free_ਅੰਕੜੇ(ਬਣਤਰ ul_ਬਫਰ *buf)
{
	ul_ਬਫਰ_free_ਅੰਕੜੇ_common(buf, false);
}

ਅੱਖਰ *ul_ਬਫਰ_steal_string(ਬਣਤਰ ul_ਬਫਰ *buf)
{
	ਜੇ (ul_ਬਫਰ_is_ਖਾਲੀ_ਥਾਵਾਂ(buf)) {
		ul_ਬਫਰ_free_ਅੰਕੜੇ(buf);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ ul_ਬਫਰ_free_ਅੰਕੜੇ_common(buf, true);
}

ਖਾਲੀ ul_ਬਫਰ_ਸੈੱਟ_chunksize(ਬਣਤਰ ul_ਬਫਰ *buf, size_t sz)
{
	buf->chunksize = sz;
}

ਪੂਰਨ ul_ਬਫਰ_is_ਖਾਲੀ_ਥਾਵਾਂ(ਬਣਤਰ ul_ਬਫਰ *buf)
{
	ਵਾਪਸ buf->begin == buf->ਸਿਰਾ;
}

ਪੂਰਨ ul_ਬਫਰ_save_ਸੂਚਕ(ਬਣਤਰ ul_ਬਫਰ *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ptr_idx)
{
	ਜੇ (ptr_idx >= buf->nptrs) {
		ਅੱਖਰ **tmp = reallocarray(buf->ptrs, ptr_idx + 1, ਆਕਾਰ(ਅੱਖਰ *));

		ਜੇ (!tmp)
			ਵਾਪਸ -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->ਸਿਰਾ;
	ਵਾਪਸ 0;
}


ਅੱਖਰ *ul_ਬਫਰ_get_ਸੂਚਕ(ਬਣਤਰ ul_ਬਫਰ *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ptr_idx)
{
	ਜੇ (ptr_idx < buf->nptrs)
		ਵਾਪਸ buf->ptrs[ptr_idx];
	ਵਾਪਸ ਨਿਰਾ;
}

/* returns length from begin to the pointer */
size_t ul_ਬਫਰ_get_ਸੂਚਕ_ਲੰਮਾਈ(ਬਣਤਰ ul_ਬਫਰ *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ptr_idx)
{
	ਅੱਖਰ *ptr = ul_ਬਫਰ_get_ਸੂਚਕ(buf, ptr_idx);

	ਜੇ (ptr && ptr > buf->begin)
		ਵਾਪਸ ptr - buf->begin;
	ਵਾਪਸ 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_ਬਫਰ_get_ਸੁਰੱਖਿਅਤ_ਸੂਚਕ_ਚੌੜਾਈ(ਬਣਤਰ ul_ਬਫਰ *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ptr_idx)
{
	size_t len = ul_ਬਫਰ_get_ਸੂਚਕ_ਲੰਮਾਈ(buf, ptr_idx);

	ਜੇ (!len)
		ਵਾਪਸ 0;

	ਵਾਪਸ mbs_ਸੁਰੱਖਿਅਤ_nwidth(buf->begin, len, ਨਿਰਾ);
}

ਖਾਲੀ ul_ਬਫਰ_refer_string(ਬਣਤਰ ul_ਬਫਰ *buf, ਅੱਖਰ *str)
{
	ਜੇ (buf->sz)
		ul_ਬਫਰ_free_ਅੰਕੜੇ(buf);
	buf->begin = str;
	buf->sz = str ? ਲੜੀ_ਲੰਬਾਈ(str) : 0;
	buf->ਸਿਰਾ = buf->begin ? buf->begin + buf->sz : buf->begin;
}

ਪੂਰਨ ul_ਬਫਰ_alloc_ਅੰਕੜੇ(ਬਣਤਰ ul_ਬਫਰ *buf, size_t sz)
{
	ਅੱਖਰ *tmp;
	size_t len = 0;

	ਪ੍ਰਤਿਜ੍ਞਾ(buf);

	ਜੇ (sz <= buf->sz)
		ਵਾਪਸ 0;

	ਜੇ (buf->ਸਿਰਾ && buf->begin)
		len = buf->ਸਿਰਾ - buf->begin;

	ਜੇ (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	ਜੇ (!tmp)
		ਵਾਪਸ -ENOMEM;

	buf->begin = tmp;
	buf->ਸਿਰਾ = buf->begin + len;
	buf->sz = sz;

	memset(buf->ਸਿਰਾ, '\0', sz - len);

	ਵਾਪਸ 0;
}

ਪੂਰਨ ul_ਬਫਰ_append_ਅੰਕੜੇ(ਬਣਤਰ ul_ਬਫਰ *buf, ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ, size_t sz)
{
	size_t maxsz = 0;

	ਜੇ (!buf)
		ਵਾਪਸ -EINVAL;
	ਜੇ (!ਅੰਕੜੇ)
		ਵਾਪਸ 0;

	ਜੇ (buf->begin && buf->ਸਿਰਾ)
		maxsz = buf->sz - (buf->ਸਿਰਾ - buf->begin);
	ਜੇ (maxsz <= sz + 1) {
		ਪੂਰਨ rc = ul_ਬਫਰ_alloc_ਅੰਕੜੇ(buf, buf->sz + sz + 1);
		ਜੇ (rc)
			ਵਾਪਸ rc;
	}
	ਜੇ (!buf->ਸਿਰਾ)
		ਵਾਪਸ -EINVAL;	/* make static analyzers happy */

	buf->ਸਿਰਾ = mempcpy(buf->ਸਿਰਾ, ਅੰਕੜੇ, sz);
	*buf->ਸਿਰਾ = '\0';	/* make sure it's terminated */
	ਵਾਪਸ 0;
}

ਪੂਰਨ ul_ਬਫਰ_append_char(ਬਣਤਰ ul_ਬਫਰ *buf, ਅਚਲ ਅੱਖਰ c)
{
	ਵਾਪਸ ul_ਬਫਰ_append_ਅੰਕੜੇ(buf, &c, 1);
}

ਪੂਰਨ ul_ਬਫਰ_append_string(ਬਣਤਰ ul_ਬਫਰ *buf, ਅਚਲ ਅੱਖਰ *str)
{
	ਜੇ (!str)
		ਵਾਪਸ 0;

	ਵਾਪਸ ul_ਬਫਰ_append_ਅੰਕੜੇ(buf, str, ਲੜੀ_ਲੰਬਾਈ(str));
}

ਪੂਰਨ ul_ਬਫਰ_append_ntimes(ਬਣਤਰ ul_ਬਫਰ *buf, size_t n, ਅਚਲ ਅੱਖਰ *str)
{
	size_t i;
	size_t len = ਲੜੀ_ਲੰਬਾਈ(str);

	ਲਈ (i = 0; len && i < n; i++) {
		ਪੂਰਨ rc = ul_ਬਫਰ_append_ਅੰਕੜੇ(buf, str, len);
		ਜੇ (rc)
			ਵਾਪਸ rc;
	}
	ਵਾਪਸ 0;
}

ਪੂਰਨ ul_ਬਫਰ_appendf(ਬਣਤਰ ul_ਬਫਰ *buf, ਅਚਲ ਅੱਖਰ *format, ...)
{
	va_list ap;
	ਪੂਰਨ res;

	va_start(ap, format);
	res = ul_ਬਫਰ_appendvf(buf, format, ap);
	va_end(ap);

	ਵਾਪਸ res;
}

ਪੂਰਨ ul_ਬਫਰ_appendvf(ਬਣਤਰ ul_ਬਫਰ *buf, ਅਚਲ ਅੱਖਰ *format, va_list ap)
{
	ਅੱਖਰ *val;
	ਪੂਰਨ sz;
	ਪੂਰਨ res;

	sz = vasprintf(&val, format, ap);
	ਜੇ (sz < 0)
		ਵਾਪਸ -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_ਬਫਰ_append_ਅੰਕੜੇ(buf, val, sz);
	ਮੁਕਤ(val);

	ਵਾਪਸ res;
}

ਪੂਰਨ ul_ਬਫਰ_ਸੈੱਟ_ਅੰਕੜੇ(ਬਣਤਰ ul_ਬਫਰ *buf, ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ, size_t sz)
{
	ul_ਬਫਰ_reset_ਅੰਕੜੇ(buf);
	ਵਾਪਸ ul_ਬਫਰ_append_ਅੰਕੜੇ(buf, ਅੰਕੜੇ, sz);
}

ਅੱਖਰ *ul_ਬਫਰ_get_ਅੰਕੜੇ(ਬਣਤਰ ul_ਬਫਰ *buf, size_t *sz, size_t *ਚੌੜਾਈ)
{
	ਜੇ (sz)
		*sz = buf->ਸਿਰਾ - buf->begin;
	ਜੇ (ਚੌੜਾਈ)
		*ਚੌੜਾਈ = buf->begin && *buf->begin ? mbs_ਚੌੜਾਈ(buf->begin) : 0;
	ਵਾਪਸ buf->begin;
}

ਅੱਖਰ *ul_ਬਫਰ_get_string(ਬਣਤਰ ul_ਬਫਰ *buf, size_t *sz, size_t *ਚੌੜਾਈ)
{
	ਅੱਖਰ *ret;

	ret = ul_ਬਫਰ_get_ਅੰਕੜੇ(buf, sz, ਚੌੜਾਈ);

	/* data in buffer is already zero-terminated */
	ਜੇ (sz)
		*sz = *sz + 1;

	ਵਾਪਸ ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_ਬਫਰ_get_bufsiz(ਬਣਤਰ ul_ਬਫਰ *buf)
{
	ਵਾਪਸ buf->sz;
}

size_t ul_ਬਫਰ_get_datasiz(ਬਣਤਰ ul_ਬਫਰ *buf)
{
	ਵਾਪਸ buf->ਸਿਰਾ - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
ਅੱਖਰ *ul_ਬਫਰ_get_ਸੁਰੱਖਿਅਤ_ਅੰਕੜੇ(ਬਣਤਰ ul_ਬਫਰ *buf, size_t *sz, size_t *ਚੌੜਾਈ, ਅਚਲ ਅੱਖਰ *safechars)
{
	ਅੱਖਰ *ਅੰਕੜੇ = ul_ਬਫਰ_get_ਅੰਕੜੇ(buf, ਨਿਰਾ, ਨਿਰਾ);
	size_t encsz, wsz = 0;
	ਅੱਖਰ *res = ਨਿਰਾ;

	ਜੇ (!ਅੰਕੜੇ)
		ਜਾਓ nothing;

	encsz = mbs_ਸੁਰੱਖਿਅਤ_encode_ਮਾਪ(buf->sz) + 1;
	ਜੇ (encsz > buf->encoded_sz) {
		ਅੱਖਰ *tmp = realloc(buf->encoded, encsz);
		ਜੇ (!tmp)
			ਜਾਓ nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_ਸੁਰੱਖਿਅਤ_encode_ਮੰਜ਼ਿਲ_ਬਫਰ(ਅੰਕੜੇ, &wsz, buf->encoded, buf->encoded_sz, safechars);
	ਜੇ (!res || !wsz || wsz == (size_t) -1)
		ਜਾਓ nothing;

	ਜੇ (ਚੌੜਾਈ)
		*ਚੌੜਾਈ = wsz;
	ਜੇ (sz)
		*sz = ਲੜੀ_ਲੰਬਾਈ(res);
	ਵਾਪਸ res;
nothing:
	ਜੇ (ਚੌੜਾਈ)
		*ਚੌੜਾਈ = 0;
	ਜੇ (sz)
		*sz = 0;
	ਵਾਪਸ ਨਿਰਾ;
}


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_ਬਫਰ

ਗਿਣਤੀ {
	PTR_AAA  = 0,
	PTR_BBB,
};

ਪੂਰਨ ਮੁੱਖ(ਖਾਲੀ)
{
	ਬਣਤਰ ul_ਬਫਰ buf = UL_INIT_ਬਫਰ;
	ਅੱਖਰ *str;
	size_t sz = 0;

	ul_ਬਫਰ_ਸੈੱਟ_chunksize(&buf, 16);

	ul_ਬਫਰ_append_string(&buf, "AAA");
	ul_ਬਫਰ_append_ਅੰਕੜੇ(&buf, "=", 1);
	ul_ਬਫਰ_append_string(&buf, "aaa");
	ul_ਬਫਰ_save_ਸੂਚਕ(&buf, PTR_AAA);

	ul_ਬਫਰ_append_ਅੰਕੜੇ(&buf, ",", 1);
	ul_ਬਫਰ_append_string(&buf, "BBB");
	ul_ਬਫਰ_append_string(&buf, "=");
	ul_ਬਫਰ_append_string(&buf, "bbb");
	ul_ਬਫਰ_save_ਸੂਚਕ(&buf, PTR_BBB);

	str = ul_ਬਫਰ_get_ਅੰਕੜੇ(&buf, &sz, ਨਿਰਾ);
	ਲਿਖੋ("data [%zu] '%s'\n", sz, str);

	ਲਿਖੋ(" pointer data len: AAA=%zu, BBB=%zu\n",
			ul_ਬਫਰ_get_ਸੂਚਕ_ਲੰਮਾਈ(&buf, PTR_AAA),
			ul_ਬਫਰ_get_ਸੂਚਕ_ਲੰਮਾਈ(&buf, PTR_BBB));
	ਲਿਖੋ(" pointer data width: AAA=%zu, BBB=%zu\n",
			ul_ਬਫਰ_get_ਸੁਰੱਖਿਅਤ_ਸੂਚਕ_ਚੌੜਾਈ(&buf, PTR_AAA),
			ul_ਬਫਰ_get_ਸੁਰੱਖਿਅਤ_ਸੂਚਕ_ਚੌੜਾਈ(&buf, PTR_BBB));

	ul_ਬਫਰ_reset_ਅੰਕੜੇ(&buf);
	ul_ਬਫਰ_append_string(&buf, "This is really long string to test the buffer function.");
	ul_ਬਫਰ_save_ਸੂਚਕ(&buf, PTR_AAA);
	ul_ਬਫਰ_append_string(&buf, " YES!");
	str = ul_ਬਫਰ_get_ਅੰਕੜੇ(&buf, &sz, ਨਿਰਾ);
	ਲਿਖੋ("data [%zu] '%s'\n", sz, str);
	ਲਿਖੋ(" pointer data len: AAA=%zu\n", ul_ਬਫਰ_get_ਸੂਚਕ_ਲੰਮਾਈ(&buf, PTR_AAA));

	ul_ਬਫਰ_free_ਅੰਕੜੇ(&buf);
	str = strdup("foo");
	ul_ਬਫਰ_refer_string(&buf, str);
	ul_ਬਫਰ_append_ਅੰਕੜੇ(&buf, ",", 1);
	ul_ਬਫਰ_append_string(&buf, "bar");
	str = ul_ਬਫਰ_get_ਅੰਕੜੇ(&buf, &sz, ਨਿਰਾ);
	ਲਿਖੋ("data [%zu] '%s'\n", sz, str);

	ul_ਬਫਰ_free_ਅੰਕੜੇ(&buf);
	ul_ਬਫਰ_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_ਬਫਰ_get_ਅੰਕੜੇ(&buf, &sz, ਨਿਰਾ);
	ਲਿਖੋ("data [%zu] '%s'\n", sz, str);

	ul_ਬਫਰ_free_ਅੰਕੜੇ(&buf);
	ul_ਬਫਰ_append_char(&buf, 'x');
	{
		ਅੱਖਰ c = 'y';
		ul_ਬਫਰ_append_char(&buf, c);
	}
	{
		ਅਚਲ ਅੱਖਰ c = 'z';
		ul_ਬਫਰ_append_char(&buf, c);
	}
	str = ul_ਬਫਰ_get_ਅੰਕੜੇ(&buf, &sz, ਨਿਰਾ);
	ਲਿਖੋ("data [%zu] '%s'\n", sz, str);

	ul_ਬਫਰ_free_ਅੰਕੜੇ(&buf);

	ਵਾਪਸ EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ /* TEST_PROGRAM_BUFFER */
