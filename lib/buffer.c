/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ضمن "buffer.h"
#ضمن "mbsalign.h"
#ضمن "strutils.h"

فراغ ul_میانگیر_reset_داده(بنية ul_میانگیر *buf)
{
	إذا (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->انتها = buf->begin;

	إذا (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * حجم(حرف *));
}

ساكن مضمن حرف *ul_میانگیر_free_داده_common(بنية ul_میانگیر *buf, bool steal)
{
	حرف *t = عدم;

	أكد(buf);

	إذا (steal)
		t = buf->begin;
	وإلا
		حرر(buf->begin);
	buf->begin = عدم;
	buf->انتها = عدم;
	buf->sz = 0;

	حرر(buf->ptrs);
	buf->ptrs = عدم;
	buf->nptrs = 0;

	حرر(buf->encoded);
	buf->encoded = عدم;
	buf->encoded_sz = 0;

	أرجع t;
}

فراغ ul_میانگیر_free_داده(بنية ul_میانگیر *buf)
{
	ul_میانگیر_free_داده_common(buf, false);
}

حرف *ul_میانگیر_steal_string(بنية ul_میانگیر *buf)
{
	إذا (ul_میانگیر_is_خالی(buf)) {
		ul_میانگیر_free_داده(buf);
		أرجع عدم;
	}

	أرجع ul_میانگیر_free_داده_common(buf, true);
}

فراغ ul_میانگیر_مجموعه_chunksize(بنية ul_میانگیر *buf, size_t sz)
{
	buf->chunksize = sz;
}

صحيح ul_میانگیر_is_خالی(بنية ul_میانگیر *buf)
{
	أرجع buf->begin == buf->انتها;
}

صحيح ul_میانگیر_save_اشاره_گر(بنية ul_میانگیر *buf, بیعلامت کوتاه ptr_idx)
{
	إذا (ptr_idx >= buf->nptrs) {
		حرف **tmp = reallocarray(buf->ptrs, ptr_idx + 1, حجم(حرف *));

		إذا (!tmp)
			أرجع -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->انتها;
	أرجع 0;
}


حرف *ul_میانگیر_get_اشاره_گر(بنية ul_میانگیر *buf, بیعلامت کوتاه ptr_idx)
{
	إذا (ptr_idx < buf->nptrs)
		أرجع buf->ptrs[ptr_idx];
	أرجع عدم;
}

/* returns length from begin to the pointer */
size_t ul_میانگیر_get_اشاره_گر_درازی(بنية ul_میانگیر *buf, بیعلامت کوتاه ptr_idx)
{
	حرف *ptr = ul_میانگیر_get_اشاره_گر(buf, ptr_idx);

	إذا (ptr && ptr > buf->begin)
		أرجع ptr - buf->begin;
	أرجع 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_میانگیر_get_امن_اشاره_گر_عرض(بنية ul_میانگیر *buf, بیعلامت کوتاه ptr_idx)
{
	size_t len = ul_میانگیر_get_اشاره_گر_درازی(buf, ptr_idx);

	إذا (!len)
		أرجع 0;

	أرجع mbs_امن_nwidth(buf->begin, len, عدم);
}

فراغ ul_میانگیر_refer_string(بنية ul_میانگیر *buf, حرف *str)
{
	إذا (buf->sz)
		ul_میانگیر_free_داده(buf);
	buf->begin = str;
	buf->sz = str ? طول_رشته(str) : 0;
	buf->انتها = buf->begin ? buf->begin + buf->sz : buf->begin;
}

صحيح ul_میانگیر_alloc_داده(بنية ul_میانگیر *buf, size_t sz)
{
	حرف *tmp;
	size_t len = 0;

	أكد(buf);

	إذا (sz <= buf->sz)
		أرجع 0;

	إذا (buf->انتها && buf->begin)
		len = buf->انتها - buf->begin;

	إذا (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	إذا (!tmp)
		أرجع -ENOMEM;

	buf->begin = tmp;
	buf->انتها = buf->begin + len;
	buf->sz = sz;

	memset(buf->انتها, '\0', sz - len);

	أرجع 0;
}

صحيح ul_میانگیر_append_داده(بنية ul_میانگیر *buf, ثابت حرف *داده, size_t sz)
{
	size_t maxsz = 0;

	إذا (!buf)
		أرجع -EINVAL;
	إذا (!داده)
		أرجع 0;

	إذا (buf->begin && buf->انتها)
		maxsz = buf->sz - (buf->انتها - buf->begin);
	إذا (maxsz <= sz + 1) {
		صحيح rc = ul_میانگیر_alloc_داده(buf, buf->sz + sz + 1);
		إذا (rc)
			أرجع rc;
	}
	إذا (!buf->انتها)
		أرجع -EINVAL;	/* make static analyzers happy */

	buf->انتها = mempcpy(buf->انتها, داده, sz);
	*buf->انتها = '\0';	/* make sure it's terminated */
	أرجع 0;
}

صحيح ul_میانگیر_append_char(بنية ul_میانگیر *buf, ثابت حرف c)
{
	أرجع ul_میانگیر_append_داده(buf, &c, 1);
}

صحيح ul_میانگیر_append_string(بنية ul_میانگیر *buf, ثابت حرف *str)
{
	إذا (!str)
		أرجع 0;

	أرجع ul_میانگیر_append_داده(buf, str, طول_رشته(str));
}

صحيح ul_میانگیر_append_ntimes(بنية ul_میانگیر *buf, size_t n, ثابت حرف *str)
{
	size_t i;
	size_t len = طول_رشته(str);

	لكل (i = 0; len && i < n; i++) {
		صحيح rc = ul_میانگیر_append_داده(buf, str, len);
		إذا (rc)
			أرجع rc;
	}
	أرجع 0;
}

صحيح ul_میانگیر_appendf(بنية ul_میانگیر *buf, ثابت حرف *format, ...)
{
	va_list ap;
	صحيح res;

	va_start(ap, format);
	res = ul_میانگیر_appendvf(buf, format, ap);
	va_end(ap);

	أرجع res;
}

صحيح ul_میانگیر_appendvf(بنية ul_میانگیر *buf, ثابت حرف *format, va_list ap)
{
	حرف *val;
	صحيح sz;
	صحيح res;

	sz = vasprintf(&val, format, ap);
	إذا (sz < 0)
		أرجع -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_میانگیر_append_داده(buf, val, sz);
	حرر(val);

	أرجع res;
}

صحيح ul_میانگیر_مجموعه_داده(بنية ul_میانگیر *buf, ثابت حرف *داده, size_t sz)
{
	ul_میانگیر_reset_داده(buf);
	أرجع ul_میانگیر_append_داده(buf, داده, sz);
}

حرف *ul_میانگیر_get_داده(بنية ul_میانگیر *buf, size_t *sz, size_t *عرض)
{
	إذا (sz)
		*sz = buf->انتها - buf->begin;
	إذا (عرض)
		*عرض = buf->begin && *buf->begin ? mbs_عرض(buf->begin) : 0;
	أرجع buf->begin;
}

حرف *ul_میانگیر_get_string(بنية ul_میانگیر *buf, size_t *sz, size_t *عرض)
{
	حرف *ret;

	ret = ul_میانگیر_get_داده(buf, sz, عرض);

	/* data in buffer is already zero-terminated */
	إذا (sz)
		*sz = *sz + 1;

	أرجع ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_میانگیر_get_bufsiz(بنية ul_میانگیر *buf)
{
	أرجع buf->sz;
}

size_t ul_میانگیر_get_datasiz(بنية ul_میانگیر *buf)
{
	أرجع buf->انتها - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
حرف *ul_میانگیر_get_امن_داده(بنية ul_میانگیر *buf, size_t *sz, size_t *عرض, ثابت حرف *safechars)
{
	حرف *داده = ul_میانگیر_get_داده(buf, عدم, عدم);
	size_t encsz, wsz = 0;
	حرف *res = عدم;

	إذا (!داده)
		اذهب nothing;

	encsz = mbs_امن_encode_اندازه(buf->sz) + 1;
	إذا (encsz > buf->encoded_sz) {
		حرف *tmp = realloc(buf->encoded, encsz);
		إذا (!tmp)
			اذهب nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_امن_encode_مقصد_میانگیر(داده, &wsz, buf->encoded, buf->encoded_sz, safechars);
	إذا (!res || !wsz || wsz == (size_t) -1)
		اذهب nothing;

	إذا (عرض)
		*عرض = wsz;
	إذا (sz)
		*sz = طول_رشته(res);
	أرجع res;
nothing:
	إذا (عرض)
		*عرض = 0;
	إذا (sz)
		*sz = 0;
	أرجع عدم;
}


#اگر_تعریف_شده TEST_برنامه_میانگیر

تعداد {
	PTR_AAA  = 0,
	PTR_BBB,
};

صحيح اصلی(فراغ)
{
	بنية ul_میانگیر buf = UL_INIT_میانگیر;
	حرف *str;
	size_t sz = 0;

	ul_میانگیر_مجموعه_chunksize(&buf, 16);

	ul_میانگیر_append_string(&buf, "AAA");
	ul_میانگیر_append_داده(&buf, "=", 1);
	ul_میانگیر_append_string(&buf, "aaa");
	ul_میانگیر_save_اشاره_گر(&buf, PTR_AAA);

	ul_میانگیر_append_داده(&buf, ",", 1);
	ul_میانگیر_append_string(&buf, "BBB");
	ul_میانگیر_append_string(&buf, "=");
	ul_میانگیر_append_string(&buf, "bbb");
	ul_میانگیر_save_اشاره_گر(&buf, PTR_BBB);

	str = ul_میانگیر_get_داده(&buf, &sz, عدم);
	چاپ_کن("data [%zu] '%s'\n", sz, str);

	چاپ_کن(" pointer data len: AAA=%zu, BBB=%zu\n",
			ul_میانگیر_get_اشاره_گر_درازی(&buf, PTR_AAA),
			ul_میانگیر_get_اشاره_گر_درازی(&buf, PTR_BBB));
	چاپ_کن(" pointer data width: AAA=%zu, BBB=%zu\n",
			ul_میانگیر_get_امن_اشاره_گر_عرض(&buf, PTR_AAA),
			ul_میانگیر_get_امن_اشاره_گر_عرض(&buf, PTR_BBB));

	ul_میانگیر_reset_داده(&buf);
	ul_میانگیر_append_string(&buf, "This is really long string to test the buffer function.");
	ul_میانگیر_save_اشاره_گر(&buf, PTR_AAA);
	ul_میانگیر_append_string(&buf, " YES!");
	str = ul_میانگیر_get_داده(&buf, &sz, عدم);
	چاپ_کن("data [%zu] '%s'\n", sz, str);
	چاپ_کن(" pointer data len: AAA=%zu\n", ul_میانگیر_get_اشاره_گر_درازی(&buf, PTR_AAA));

	ul_میانگیر_free_داده(&buf);
	str = strdup("foo");
	ul_میانگیر_refer_string(&buf, str);
	ul_میانگیر_append_داده(&buf, ",", 1);
	ul_میانگیر_append_string(&buf, "bar");
	str = ul_میانگیر_get_داده(&buf, &sz, عدم);
	چاپ_کن("data [%zu] '%s'\n", sz, str);

	ul_میانگیر_free_داده(&buf);
	ul_میانگیر_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_میانگیر_get_داده(&buf, &sz, عدم);
	چاپ_کن("data [%zu] '%s'\n", sz, str);

	ul_میانگیر_free_داده(&buf);
	ul_میانگیر_append_char(&buf, 'x');
	{
		حرف c = 'y';
		ul_میانگیر_append_char(&buf, c);
	}
	{
		ثابت حرف c = 'z';
		ul_میانگیر_append_char(&buf, c);
	}
	str = ul_میانگیر_get_داده(&buf, &sz, عدم);
	چاپ_کن("data [%zu] '%s'\n", sz, str);

	ul_میانگیر_free_داده(&buf);

	أرجع EXIT_SUCCESS;
}
#پایان_اگر /* TEST_PROGRAM_BUFFER */
