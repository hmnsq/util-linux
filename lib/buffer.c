/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ضمن "buffer.h"
#ضمن "mbsalign.h"
#ضمن "strutils.h"

فراغ ul_بفر_reset_معلومات(بنية ul_بفر *buf)
{
	إذا (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->پڇاڙي = buf->begin;

	إذا (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * حجم(حرف *));
}

ساكن مضمن حرف *ul_بفر_free_معلومات_common(بنية ul_بفر *buf, bool steal)
{
	حرف *t = عدم;

	أكد(buf);

	إذا (steal)
		t = buf->begin;
	وإلا
		حرر(buf->begin);
	buf->begin = عدم;
	buf->پڇاڙي = عدم;
	buf->sz = 0;

	حرر(buf->ptrs);
	buf->ptrs = عدم;
	buf->nptrs = 0;

	حرر(buf->encoded);
	buf->encoded = عدم;
	buf->encoded_sz = 0;

	أرجع t;
}

فراغ ul_بفر_free_معلومات(بنية ul_بفر *buf)
{
	ul_بفر_free_معلومات_common(buf, false);
}

حرف *ul_بفر_steal_string(بنية ul_بفر *buf)
{
	إذا (ul_بفر_is_خالي_جايون(buf)) {
		ul_بفر_free_معلومات(buf);
		أرجع عدم;
	}

	أرجع ul_بفر_free_معلومات_common(buf, true);
}

فراغ ul_بفر_سيٽ_chunksize(بنية ul_بفر *buf, size_t sz)
{
	buf->chunksize = sz;
}

صحيح ul_بفر_is_خالي_جايون(بنية ul_بفر *buf)
{
	أرجع buf->begin == buf->پڇاڙي;
}

صحيح ul_بفر_save_اشارو(بنية ul_بفر *buf, بنانشان ننڍو ptr_idx)
{
	إذا (ptr_idx >= buf->nptrs) {
		حرف **tmp = reallocarray(buf->ptrs, ptr_idx + 1, حجم(حرف *));

		إذا (!tmp)
			أرجع -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->پڇاڙي;
	أرجع 0;
}


حرف *ul_بفر_get_اشارو(بنية ul_بفر *buf, بنانشان ننڍو ptr_idx)
{
	إذا (ptr_idx < buf->nptrs)
		أرجع buf->ptrs[ptr_idx];
	أرجع عدم;
}

/* returns length from begin to the pointer */
size_t ul_بفر_get_اشارو_ڊيگهائي(بنية ul_بفر *buf, بنانشان ننڍو ptr_idx)
{
	حرف *ptr = ul_بفر_get_اشارو(buf, ptr_idx);

	إذا (ptr && ptr > buf->begin)
		أرجع ptr - buf->begin;
	أرجع 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_بفر_get_محفوظ_اشارو_ويڪر(بنية ul_بفر *buf, بنانشان ننڍو ptr_idx)
{
	size_t len = ul_بفر_get_اشارو_ڊيگهائي(buf, ptr_idx);

	إذا (!len)
		أرجع 0;

	أرجع mbs_محفوظ_nwidth(buf->begin, len, عدم);
}

فراغ ul_بفر_refer_string(بنية ul_بفر *buf, حرف *str)
{
	إذا (buf->sz)
		ul_بفر_free_معلومات(buf);
	buf->begin = str;
	buf->sz = str ? لڙي_ڊيگهه(str) : 0;
	buf->پڇاڙي = buf->begin ? buf->begin + buf->sz : buf->begin;
}

صحيح ul_بفر_alloc_معلومات(بنية ul_بفر *buf, size_t sz)
{
	حرف *tmp;
	size_t len = 0;

	أكد(buf);

	إذا (sz <= buf->sz)
		أرجع 0;

	إذا (buf->پڇاڙي && buf->begin)
		len = buf->پڇاڙي - buf->begin;

	إذا (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	إذا (!tmp)
		أرجع -ENOMEM;

	buf->begin = tmp;
	buf->پڇاڙي = buf->begin + len;
	buf->sz = sz;

	memset(buf->پڇاڙي, '\0', sz - len);

	أرجع 0;
}

صحيح ul_بفر_append_معلومات(بنية ul_بفر *buf, ثابت حرف *معلومات, size_t sz)
{
	size_t maxsz = 0;

	إذا (!buf)
		أرجع -EINVAL;
	إذا (!معلومات)
		أرجع 0;

	إذا (buf->begin && buf->پڇاڙي)
		maxsz = buf->sz - (buf->پڇاڙي - buf->begin);
	إذا (maxsz <= sz + 1) {
		صحيح rc = ul_بفر_alloc_معلومات(buf, buf->sz + sz + 1);
		إذا (rc)
			أرجع rc;
	}
	إذا (!buf->پڇاڙي)
		أرجع -EINVAL;	/* make static analyzers happy */

	buf->پڇاڙي = mempcpy(buf->پڇاڙي, معلومات, sz);
	*buf->پڇاڙي = '\0';	/* make sure it's terminated */
	أرجع 0;
}

صحيح ul_بفر_append_char(بنية ul_بفر *buf, ثابت حرف c)
{
	أرجع ul_بفر_append_معلومات(buf, &c, 1);
}

صحيح ul_بفر_append_string(بنية ul_بفر *buf, ثابت حرف *str)
{
	إذا (!str)
		أرجع 0;

	أرجع ul_بفر_append_معلومات(buf, str, لڙي_ڊيگهه(str));
}

صحيح ul_بفر_append_ntimes(بنية ul_بفر *buf, size_t n, ثابت حرف *str)
{
	size_t i;
	size_t len = لڙي_ڊيگهه(str);

	لكل (i = 0; len && i < n; i++) {
		صحيح rc = ul_بفر_append_معلومات(buf, str, len);
		إذا (rc)
			أرجع rc;
	}
	أرجع 0;
}

صحيح ul_بفر_appendf(بنية ul_بفر *buf, ثابت حرف *format, ...)
{
	va_list ap;
	صحيح res;

	va_start(ap, format);
	res = ul_بفر_appendvf(buf, format, ap);
	va_end(ap);

	أرجع res;
}

صحيح ul_بفر_appendvf(بنية ul_بفر *buf, ثابت حرف *format, va_list ap)
{
	حرف *val;
	صحيح sz;
	صحيح res;

	sz = vasprintf(&val, format, ap);
	إذا (sz < 0)
		أرجع -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_بفر_append_معلومات(buf, val, sz);
	حرر(val);

	أرجع res;
}

صحيح ul_بفر_سيٽ_معلومات(بنية ul_بفر *buf, ثابت حرف *معلومات, size_t sz)
{
	ul_بفر_reset_معلومات(buf);
	أرجع ul_بفر_append_معلومات(buf, معلومات, sz);
}

حرف *ul_بفر_get_معلومات(بنية ul_بفر *buf, size_t *sz, size_t *ويڪر)
{
	إذا (sz)
		*sz = buf->پڇاڙي - buf->begin;
	إذا (ويڪر)
		*ويڪر = buf->begin && *buf->begin ? mbs_ويڪر(buf->begin) : 0;
	أرجع buf->begin;
}

حرف *ul_بفر_get_string(بنية ul_بفر *buf, size_t *sz, size_t *ويڪر)
{
	حرف *ret;

	ret = ul_بفر_get_معلومات(buf, sz, ويڪر);

	/* data in buffer is already zero-terminated */
	إذا (sz)
		*sz = *sz + 1;

	أرجع ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_بفر_get_bufsiz(بنية ul_بفر *buf)
{
	أرجع buf->sz;
}

size_t ul_بفر_get_datasiz(بنية ul_بفر *buf)
{
	أرجع buf->پڇاڙي - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
حرف *ul_بفر_get_محفوظ_معلومات(بنية ul_بفر *buf, size_t *sz, size_t *ويڪر, ثابت حرف *safechars)
{
	حرف *معلومات = ul_بفر_get_معلومات(buf, عدم, عدم);
	size_t encsz, wsz = 0;
	حرف *res = عدم;

	إذا (!معلومات)
		اذهب nothing;

	encsz = mbs_محفوظ_encode_سائيز(buf->sz) + 1;
	إذا (encsz > buf->encoded_sz) {
		حرف *tmp = realloc(buf->encoded, encsz);
		إذا (!tmp)
			اذهب nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_محفوظ_encode_منزل_بفر(معلومات, &wsz, buf->encoded, buf->encoded_sz, safechars);
	إذا (!res || !wsz || wsz == (size_t) -1)
		اذهب nothing;

	إذا (ويڪر)
		*ويڪر = wsz;
	إذا (sz)
		*sz = لڙي_ڊيگهه(res);
	أرجع res;
nothing:
	إذا (ويڪر)
		*ويڪر = 0;
	إذا (sz)
		*sz = 0;
	أرجع عدم;
}


#جيڪڏهن_وصف_ٿيل TEST_پروگرام_بفر

تعداد {
	PTR_AAA  = 0,
	PTR_BBB,
};

صحيح مکيه(فراغ)
{
	بنية ul_بفر buf = UL_INIT_بفر;
	حرف *str;
	size_t sz = 0;

	ul_بفر_سيٽ_chunksize(&buf, 16);

	ul_بفر_append_string(&buf, "AAA");
	ul_بفر_append_معلومات(&buf, "=", 1);
	ul_بفر_append_string(&buf, "aaa");
	ul_بفر_save_اشارو(&buf, PTR_AAA);

	ul_بفر_append_معلومات(&buf, ",", 1);
	ul_بفر_append_string(&buf, "BBB");
	ul_بفر_append_string(&buf, "=");
	ul_بفر_append_string(&buf, "bbb");
	ul_بفر_save_اشارو(&buf, PTR_BBB);

	str = ul_بفر_get_معلومات(&buf, &sz, عدم);
	لکو("data [%zu] '%s'\n", sz, str);

	لکو(" pointer data len: AAA=%zu, BBB=%zu\n",
			ul_بفر_get_اشارو_ڊيگهائي(&buf, PTR_AAA),
			ul_بفر_get_اشارو_ڊيگهائي(&buf, PTR_BBB));
	لکو(" pointer data width: AAA=%zu, BBB=%zu\n",
			ul_بفر_get_محفوظ_اشارو_ويڪر(&buf, PTR_AAA),
			ul_بفر_get_محفوظ_اشارو_ويڪر(&buf, PTR_BBB));

	ul_بفر_reset_معلومات(&buf);
	ul_بفر_append_string(&buf, "This is really long string to test the buffer function.");
	ul_بفر_save_اشارو(&buf, PTR_AAA);
	ul_بفر_append_string(&buf, " YES!");
	str = ul_بفر_get_معلومات(&buf, &sz, عدم);
	لکو("data [%zu] '%s'\n", sz, str);
	لکو(" pointer data len: AAA=%zu\n", ul_بفر_get_اشارو_ڊيگهائي(&buf, PTR_AAA));

	ul_بفر_free_معلومات(&buf);
	str = strdup("foo");
	ul_بفر_refer_string(&buf, str);
	ul_بفر_append_معلومات(&buf, ",", 1);
	ul_بفر_append_string(&buf, "bar");
	str = ul_بفر_get_معلومات(&buf, &sz, عدم);
	لکو("data [%zu] '%s'\n", sz, str);

	ul_بفر_free_معلومات(&buf);
	ul_بفر_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_بفر_get_معلومات(&buf, &sz, عدم);
	لکو("data [%zu] '%s'\n", sz, str);

	ul_بفر_free_معلومات(&buf);
	ul_بفر_append_char(&buf, 'x');
	{
		حرف c = 'y';
		ul_بفر_append_char(&buf, c);
	}
	{
		ثابت حرف c = 'z';
		ul_بفر_append_char(&buf, c);
	}
	str = ul_بفر_get_معلومات(&buf, &sz, عدم);
	لکو("data [%zu] '%s'\n", sz, str);

	ul_بفر_free_معلومات(&buf);

	أرجع EXIT_SUCCESS;
}
#جيڪڏهن_پورو /* TEST_PROGRAM_BUFFER */
