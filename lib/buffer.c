/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ಅನ್ತರ್ಭಾವ "buffer.h"
#ಅನ್ತರ್ಭಾವ "mbsalign.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"

ಶೂನ್ಯ ul_ಬಫರ್_reset_ದತ್ತಾಂಶ(ರಚನೆ ul_ಬಫರ್ *buf)
{
	ಆದರೆ (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->ಕೊನೆ = buf->begin;

	ಆದರೆ (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * ಗಾತ್ರ(ಅಕ್ಷರ *));
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಅಕ್ಷರ *ul_ಬಫರ್_free_ದತ್ತಾಂಶ_common(ರಚನೆ ul_ಬಫರ್ *buf, bool steal)
{
	ಅಕ್ಷರ *t = ಖಾಲಿ;

	ಪ್ರತಿಜ್ಞಾ(buf);

	ಆದರೆ (steal)
		t = buf->begin;
	ಇಲ್ಲದಿದ್ದರೆ
		ಬಿಡುಗಡೆ(buf->begin);
	buf->begin = ಖಾಲಿ;
	buf->ಕೊನೆ = ಖಾಲಿ;
	buf->sz = 0;

	ಬಿಡುಗಡೆ(buf->ptrs);
	buf->ptrs = ಖಾಲಿ;
	buf->nptrs = 0;

	ಬಿಡುಗಡೆ(buf->encoded);
	buf->encoded = ಖಾಲಿ;
	buf->encoded_sz = 0;

	ಹಿಂತಿರುಗಿ t;
}

ಶೂನ್ಯ ul_ಬಫರ್_free_ದತ್ತಾಂಶ(ರಚನೆ ul_ಬಫರ್ *buf)
{
	ul_ಬಫರ್_free_ದತ್ತಾಂಶ_common(buf, false);
}

ಅಕ್ಷರ *ul_ಬಫರ್_steal_string(ರಚನೆ ul_ಬಫರ್ *buf)
{
	ಆದರೆ (ul_ಬಫರ್_is_ಖಾಲಿಗಳು(buf)) {
		ul_ಬಫರ್_free_ದತ್ತಾಂಶ(buf);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ ul_ಬಫರ್_free_ದತ್ತಾಂಶ_common(buf, true);
}

ಶೂನ್ಯ ul_ಬಫರ್_ಗಣ_chunksize(ರಚನೆ ul_ಬಫರ್ *buf, size_t sz)
{
	buf->chunksize = sz;
}

ಪೂರ್ಣಾಂಕ ul_ಬಫರ್_is_ಖಾಲಿಗಳು(ರಚನೆ ul_ಬಫರ್ *buf)
{
	ಹಿಂತಿರುಗಿ buf->begin == buf->ಕೊನೆ;
}

ಪೂರ್ಣಾಂಕ ul_ಬಫರ್_save_ಸೂಚಕ(ರಚನೆ ul_ಬಫರ್ *buf, ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ptr_idx)
{
	ಆದರೆ (ptr_idx >= buf->nptrs) {
		ಅಕ್ಷರ **tmp = reallocarray(buf->ptrs, ptr_idx + 1, ಗಾತ್ರ(ಅಕ್ಷರ *));

		ಆದರೆ (!tmp)
			ಹಿಂತಿರುಗಿ -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->ಕೊನೆ;
	ಹಿಂತಿರುಗಿ 0;
}


ಅಕ್ಷರ *ul_ಬಫರ್_get_ಸೂಚಕ(ರಚನೆ ul_ಬಫರ್ *buf, ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ptr_idx)
{
	ಆದರೆ (ptr_idx < buf->nptrs)
		ಹಿಂತಿರುಗಿ buf->ptrs[ptr_idx];
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/* returns length from begin to the pointer */
size_t ul_ಬಫರ್_get_ಸೂಚಕ_ಉದ್ದಳತೆ(ರಚನೆ ul_ಬಫರ್ *buf, ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ptr_idx)
{
	ಅಕ್ಷರ *ptr = ul_ಬಫರ್_get_ಸೂಚಕ(buf, ptr_idx);

	ಆದರೆ (ptr && ptr > buf->begin)
		ಹಿಂತಿರುಗಿ ptr - buf->begin;
	ಹಿಂತಿರುಗಿ 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_ಬಫರ್_get_ಸುರಕ್ಷಿತ_ಸೂಚಕ_ಅಗಲ(ರಚನೆ ul_ಬಫರ್ *buf, ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ptr_idx)
{
	size_t len = ul_ಬಫರ್_get_ಸೂಚಕ_ಉದ್ದಳತೆ(buf, ptr_idx);

	ಆದರೆ (!len)
		ಹಿಂತಿರುಗಿ 0;

	ಹಿಂತಿರುಗಿ mbs_ಸುರಕ್ಷಿತ_nwidth(buf->begin, len, ಖಾಲಿ);
}

ಶೂನ್ಯ ul_ಬಫರ್_refer_string(ರಚನೆ ul_ಬಫರ್ *buf, ಅಕ್ಷರ *str)
{
	ಆದರೆ (buf->sz)
		ul_ಬಫರ್_free_ದತ್ತಾಂಶ(buf);
	buf->begin = str;
	buf->sz = str ? ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str) : 0;
	buf->ಕೊನೆ = buf->begin ? buf->begin + buf->sz : buf->begin;
}

ಪೂರ್ಣಾಂಕ ul_ಬಫರ್_alloc_ದತ್ತಾಂಶ(ರಚನೆ ul_ಬಫರ್ *buf, size_t sz)
{
	ಅಕ್ಷರ *tmp;
	size_t len = 0;

	ಪ್ರತಿಜ್ಞಾ(buf);

	ಆದರೆ (sz <= buf->sz)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (buf->ಕೊನೆ && buf->begin)
		len = buf->ಕೊನೆ - buf->begin;

	ಆದರೆ (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	ಆದರೆ (!tmp)
		ಹಿಂತಿರುಗಿ -ENOMEM;

	buf->begin = tmp;
	buf->ಕೊನೆ = buf->begin + len;
	buf->sz = sz;

	memset(buf->ಕೊನೆ, '\0', sz - len);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ ul_ಬಫರ್_append_ದತ್ತಾಂಶ(ರಚನೆ ul_ಬಫರ್ *buf, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ, size_t sz)
{
	size_t maxsz = 0;

	ಆದರೆ (!buf)
		ಹಿಂತಿರುಗಿ -EINVAL;
	ಆದರೆ (!ದತ್ತಾಂಶ)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (buf->begin && buf->ಕೊನೆ)
		maxsz = buf->sz - (buf->ಕೊನೆ - buf->begin);
	ಆದರೆ (maxsz <= sz + 1) {
		ಪೂರ್ಣಾಂಕ rc = ul_ಬಫರ್_alloc_ದತ್ತಾಂಶ(buf, buf->sz + sz + 1);
		ಆದರೆ (rc)
			ಹಿಂತಿರುಗಿ rc;
	}
	ಆದರೆ (!buf->ಕೊನೆ)
		ಹಿಂತಿರುಗಿ -EINVAL;	/* make static analyzers happy */

	buf->ಕೊನೆ = mempcpy(buf->ಕೊನೆ, ದತ್ತಾಂಶ, sz);
	*buf->ಕೊನೆ = '\0';	/* make sure it's terminated */
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ ul_ಬಫರ್_append_char(ರಚನೆ ul_ಬಫರ್ *buf, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ c)
{
	ಹಿಂತಿರುಗಿ ul_ಬಫರ್_append_ದತ್ತಾಂಶ(buf, &c, 1);
}

ಪೂರ್ಣಾಂಕ ul_ಬಫರ್_append_string(ರಚನೆ ul_ಬಫರ್ *buf, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	ಆದರೆ (!str)
		ಹಿಂತಿರುಗಿ 0;

	ಹಿಂತಿರುಗಿ ul_ಬಫರ್_append_ದತ್ತಾಂಶ(buf, str, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str));
}

ಪೂರ್ಣಾಂಕ ul_ಬಫರ್_append_ntimes(ರಚನೆ ul_ಬಫರ್ *buf, size_t n, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	size_t i;
	size_t len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str);

	ಗಾಗಿ (i = 0; len && i < n; i++) {
		ಪೂರ್ಣಾಂಕ rc = ul_ಬಫರ್_append_ದತ್ತಾಂಶ(buf, str, len);
		ಆದರೆ (rc)
			ಹಿಂತಿರುಗಿ rc;
	}
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ ul_ಬಫರ್_appendf(ರಚನೆ ul_ಬಫರ್ *buf, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *format, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಂಕ res;

	va_start(ap, format);
	res = ul_ಬಫರ್_appendvf(buf, format, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ res;
}

ಪೂರ್ಣಾಂಕ ul_ಬಫರ್_appendvf(ರಚನೆ ul_ಬಫರ್ *buf, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *format, va_list ap)
{
	ಅಕ್ಷರ *val;
	ಪೂರ್ಣಾಂಕ sz;
	ಪೂರ್ಣಾಂಕ res;

	sz = vasprintf(&val, format, ap);
	ಆದರೆ (sz < 0)
		ಹಿಂತಿರುಗಿ -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_ಬಫರ್_append_ದತ್ತಾಂಶ(buf, val, sz);
	ಬಿಡುಗಡೆ(val);

	ಹಿಂತಿರುಗಿ res;
}

ಪೂರ್ಣಾಂಕ ul_ಬಫರ್_ಗಣ_ದತ್ತಾಂಶ(ರಚನೆ ul_ಬಫರ್ *buf, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ, size_t sz)
{
	ul_ಬಫರ್_reset_ದತ್ತಾಂಶ(buf);
	ಹಿಂತಿರುಗಿ ul_ಬಫರ್_append_ದತ್ತಾಂಶ(buf, ದತ್ತಾಂಶ, sz);
}

ಅಕ್ಷರ *ul_ಬಫರ್_get_ದತ್ತಾಂಶ(ರಚನೆ ul_ಬಫರ್ *buf, size_t *sz, size_t *ಅಗಲ)
{
	ಆದರೆ (sz)
		*sz = buf->ಕೊನೆ - buf->begin;
	ಆದರೆ (ಅಗಲ)
		*ಅಗಲ = buf->begin && *buf->begin ? mbs_ಅಗಲ(buf->begin) : 0;
	ಹಿಂತಿರುಗಿ buf->begin;
}

ಅಕ್ಷರ *ul_ಬಫರ್_get_string(ರಚನೆ ul_ಬಫರ್ *buf, size_t *sz, size_t *ಅಗಲ)
{
	ಅಕ್ಷರ *ret;

	ret = ul_ಬಫರ್_get_ದತ್ತಾಂಶ(buf, sz, ಅಗಲ);

	/* data in buffer is already zero-terminated */
	ಆದರೆ (sz)
		*sz = *sz + 1;

	ಹಿಂತಿರುಗಿ ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_ಬಫರ್_get_bufsiz(ರಚನೆ ul_ಬಫರ್ *buf)
{
	ಹಿಂತಿರುಗಿ buf->sz;
}

size_t ul_ಬಫರ್_get_datasiz(ರಚನೆ ul_ಬಫರ್ *buf)
{
	ಹಿಂತಿರುಗಿ buf->ಕೊನೆ - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
ಅಕ್ಷರ *ul_ಬಫರ್_get_ಸುರಕ್ಷಿತ_ದತ್ತಾಂಶ(ರಚನೆ ul_ಬಫರ್ *buf, size_t *sz, size_t *ಅಗಲ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *safechars)
{
	ಅಕ್ಷರ *ದತ್ತಾಂಶ = ul_ಬಫರ್_get_ದತ್ತಾಂಶ(buf, ಖಾಲಿ, ಖಾಲಿ);
	size_t encsz, wsz = 0;
	ಅಕ್ಷರ *res = ಖಾಲಿ;

	ಆದರೆ (!ದತ್ತಾಂಶ)
		ಹೋಗು nothing;

	encsz = mbs_ಸುರಕ್ಷಿತ_encode_ಗಾತ್ರ(buf->sz) + 1;
	ಆದರೆ (encsz > buf->encoded_sz) {
		ಅಕ್ಷರ *tmp = realloc(buf->encoded, encsz);
		ಆದರೆ (!tmp)
			ಹೋಗು nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_ಸುರಕ್ಷಿತ_encode_ಗಮ್ಯ_ಬಫರ್(ದತ್ತಾಂಶ, &wsz, buf->encoded, buf->encoded_sz, safechars);
	ಆದರೆ (!res || !wsz || wsz == (size_t) -1)
		ಹೋಗು nothing;

	ಆದರೆ (ಅಗಲ)
		*ಅಗಲ = wsz;
	ಆದರೆ (sz)
		*sz = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(res);
	ಹಿಂತಿರುಗಿ res;
nothing:
	ಆದರೆ (ಅಗಲ)
		*ಅಗಲ = 0;
	ಆದರೆ (sz)
		*sz = 0;
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_ಬಫರ್

ಎಣಿಕೆ {
	PTR_AAA  = 0,
	PTR_BBB,
};

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಶೂನ್ಯ)
{
	ರಚನೆ ul_ಬಫರ್ buf = UL_INIT_ಬಫರ್;
	ಅಕ್ಷರ *str;
	size_t sz = 0;

	ul_ಬಫರ್_ಗಣ_chunksize(&buf, 16);

	ul_ಬಫರ್_append_string(&buf, "AAA");
	ul_ಬಫರ್_append_ದತ್ತಾಂಶ(&buf, "=", 1);
	ul_ಬಫರ್_append_string(&buf, "aaa");
	ul_ಬಫರ್_save_ಸೂಚಕ(&buf, PTR_AAA);

	ul_ಬಫರ್_append_ದತ್ತಾಂಶ(&buf, ",", 1);
	ul_ಬಫರ್_append_string(&buf, "BBB");
	ul_ಬಫರ್_append_string(&buf, "=");
	ul_ಬಫರ್_append_string(&buf, "bbb");
	ul_ಬಫರ್_save_ಸೂಚಕ(&buf, PTR_BBB);

	str = ul_ಬಫರ್_get_ದತ್ತಾಂಶ(&buf, &sz, ಖಾಲಿ);
	ಬರೆ("data [%zu] '%s'\n", sz, str);

	ಬರೆ(" pointer data len: AAA=%zu, BBB=%zu\n",
			ul_ಬಫರ್_get_ಸೂಚಕ_ಉದ್ದಳತೆ(&buf, PTR_AAA),
			ul_ಬಫರ್_get_ಸೂಚಕ_ಉದ್ದಳತೆ(&buf, PTR_BBB));
	ಬರೆ(" pointer data width: AAA=%zu, BBB=%zu\n",
			ul_ಬಫರ್_get_ಸುರಕ್ಷಿತ_ಸೂಚಕ_ಅಗಲ(&buf, PTR_AAA),
			ul_ಬಫರ್_get_ಸುರಕ್ಷಿತ_ಸೂಚಕ_ಅಗಲ(&buf, PTR_BBB));

	ul_ಬಫರ್_reset_ದತ್ತಾಂಶ(&buf);
	ul_ಬಫರ್_append_string(&buf, "This is really long string to test the buffer function.");
	ul_ಬಫರ್_save_ಸೂಚಕ(&buf, PTR_AAA);
	ul_ಬಫರ್_append_string(&buf, " YES!");
	str = ul_ಬಫರ್_get_ದತ್ತಾಂಶ(&buf, &sz, ಖಾಲಿ);
	ಬರೆ("data [%zu] '%s'\n", sz, str);
	ಬರೆ(" pointer data len: AAA=%zu\n", ul_ಬಫರ್_get_ಸೂಚಕ_ಉದ್ದಳತೆ(&buf, PTR_AAA));

	ul_ಬಫರ್_free_ದತ್ತಾಂಶ(&buf);
	str = strdup("foo");
	ul_ಬಫರ್_refer_string(&buf, str);
	ul_ಬಫರ್_append_ದತ್ತಾಂಶ(&buf, ",", 1);
	ul_ಬಫರ್_append_string(&buf, "bar");
	str = ul_ಬಫರ್_get_ದತ್ತಾಂಶ(&buf, &sz, ಖಾಲಿ);
	ಬರೆ("data [%zu] '%s'\n", sz, str);

	ul_ಬಫರ್_free_ದತ್ತಾಂಶ(&buf);
	ul_ಬಫರ್_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_ಬಫರ್_get_ದತ್ತಾಂಶ(&buf, &sz, ಖಾಲಿ);
	ಬರೆ("data [%zu] '%s'\n", sz, str);

	ul_ಬಫರ್_free_ದತ್ತಾಂಶ(&buf);
	ul_ಬಫರ್_append_char(&buf, 'x');
	{
		ಅಕ್ಷರ c = 'y';
		ul_ಬಫರ್_append_char(&buf, c);
	}
	{
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ c = 'z';
		ul_ಬಫರ್_append_char(&buf, c);
	}
	str = ul_ಬಫರ್_get_ದತ್ತಾಂಶ(&buf, &sz, ಖಾಲಿ);
	ಬರೆ("data [%zu] '%s'\n", sz, str);

	ul_ಬಫರ್_free_ದತ್ತಾಂಶ(&buf);

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ /* TEST_PROGRAM_BUFFER */
