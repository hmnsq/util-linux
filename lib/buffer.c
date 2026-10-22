/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#شامل "buffer.h"
#شامل "mbsalign.h"
#شامل "strutils.h"

خالی ul_بفر_reset_ڈیٹا(ساخت ul_بفر *buf)
{
	اگر (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->انتہا = buf->begin;

	اگر (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * حجم(حرف *));
}

ساکن اندرونی حرف *ul_بفر_free_ڈیٹا_common(ساخت ul_بفر *buf, bool steal)
{
	حرف *t = تہی;

	أكد(buf);

	اگر (steal)
		t = buf->begin;
	ورنہ
		آزاد(buf->begin);
	buf->begin = تہی;
	buf->انتہا = تہی;
	buf->sz = 0;

	آزاد(buf->ptrs);
	buf->ptrs = تہی;
	buf->nptrs = 0;

	آزاد(buf->encoded);
	buf->encoded = تہی;
	buf->encoded_sz = 0;

	واپس t;
}

خالی ul_بفر_free_ڈیٹا(ساخت ul_بفر *buf)
{
	ul_بفر_free_ڈیٹا_common(buf, false);
}

حرف *ul_بفر_steal_string(ساخت ul_بفر *buf)
{
	اگر (ul_بفر_is_خالی_خانے(buf)) {
		ul_بفر_free_ڈیٹا(buf);
		واپس تہی;
	}

	واپس ul_بفر_free_ڈیٹا_common(buf, true);
}

خالی ul_بفر_سیٹ_chunksize(ساخت ul_بفر *buf, size_t sz)
{
	buf->chunksize = sz;
}

صحیح ul_بفر_is_خالی_خانے(ساخت ul_بفر *buf)
{
	واپس buf->begin == buf->انتہا;
}

صحیح ul_بفر_save_اشارہ_گر(ساخت ul_بفر *buf, غیرعلامتی مختصر ptr_idx)
{
	اگر (ptr_idx >= buf->nptrs) {
		حرف **tmp = reallocarray(buf->ptrs, ptr_idx + 1, حجم(حرف *));

		اگر (!tmp)
			واپس -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->انتہا;
	واپس 0;
}


حرف *ul_بفر_get_اشارہ_گر(ساخت ul_بفر *buf, غیرعلامتی مختصر ptr_idx)
{
	اگر (ptr_idx < buf->nptrs)
		واپس buf->ptrs[ptr_idx];
	واپس تہی;
}

/* returns length from begin to the pointer */
size_t ul_بفر_get_اشارہ_گر_لمبائی(ساخت ul_بفر *buf, غیرعلامتی مختصر ptr_idx)
{
	حرف *ptr = ul_بفر_get_اشارہ_گر(buf, ptr_idx);

	اگر (ptr && ptr > buf->begin)
		واپس ptr - buf->begin;
	واپس 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_بفر_get_محفوظ_اشارہ_گر_چوڑائی(ساخت ul_بفر *buf, غیرعلامتی مختصر ptr_idx)
{
	size_t len = ul_بفر_get_اشارہ_گر_لمبائی(buf, ptr_idx);

	اگر (!len)
		واپس 0;

	واپس mbs_محفوظ_nwidth(buf->begin, len, تہی);
}

خالی ul_بفر_refer_string(ساخت ul_بفر *buf, حرف *str)
{
	اگر (buf->sz)
		ul_بفر_free_ڈیٹا(buf);
	buf->begin = str;
	buf->sz = str ? لڑی_لمبائی(str) : 0;
	buf->انتہا = buf->begin ? buf->begin + buf->sz : buf->begin;
}

صحیح ul_بفر_alloc_ڈیٹا(ساخت ul_بفر *buf, size_t sz)
{
	حرف *tmp;
	size_t len = 0;

	أكد(buf);

	اگر (sz <= buf->sz)
		واپس 0;

	اگر (buf->انتہا && buf->begin)
		len = buf->انتہا - buf->begin;

	اگر (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	اگر (!tmp)
		واپس -ENOMEM;

	buf->begin = tmp;
	buf->انتہا = buf->begin + len;
	buf->sz = sz;

	memset(buf->انتہا, '\0', sz - len);

	واپس 0;
}

صحیح ul_بفر_append_ڈیٹا(ساخت ul_بفر *buf, ثابت حرف *ڈیٹا, size_t sz)
{
	size_t maxsz = 0;

	اگر (!buf)
		واپس -EINVAL;
	اگر (!ڈیٹا)
		واپس 0;

	اگر (buf->begin && buf->انتہا)
		maxsz = buf->sz - (buf->انتہا - buf->begin);
	اگر (maxsz <= sz + 1) {
		صحیح rc = ul_بفر_alloc_ڈیٹا(buf, buf->sz + sz + 1);
		اگر (rc)
			واپس rc;
	}
	اگر (!buf->انتہا)
		واپس -EINVAL;	/* make static analyzers happy */

	buf->انتہا = mempcpy(buf->انتہا, ڈیٹا, sz);
	*buf->انتہا = '\0';	/* make sure it's terminated */
	واپس 0;
}

صحیح ul_بفر_append_char(ساخت ul_بفر *buf, ثابت حرف c)
{
	واپس ul_بفر_append_ڈیٹا(buf, &c, 1);
}

صحیح ul_بفر_append_string(ساخت ul_بفر *buf, ثابت حرف *str)
{
	اگر (!str)
		واپس 0;

	واپس ul_بفر_append_ڈیٹا(buf, str, لڑی_لمبائی(str));
}

صحیح ul_بفر_append_ntimes(ساخت ul_بفر *buf, size_t n, ثابت حرف *str)
{
	size_t i;
	size_t len = لڑی_لمبائی(str);

	برائے (i = 0; len && i < n; i++) {
		صحیح rc = ul_بفر_append_ڈیٹا(buf, str, len);
		اگر (rc)
			واپس rc;
	}
	واپس 0;
}

صحیح ul_بفر_appendf(ساخت ul_بفر *buf, ثابت حرف *format, ...)
{
	va_list ap;
	صحیح res;

	va_start(ap, format);
	res = ul_بفر_appendvf(buf, format, ap);
	va_end(ap);

	واپس res;
}

صحیح ul_بفر_appendvf(ساخت ul_بفر *buf, ثابت حرف *format, va_list ap)
{
	حرف *val;
	صحیح sz;
	صحیح res;

	sz = vasprintf(&val, format, ap);
	اگر (sz < 0)
		واپس -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_بفر_append_ڈیٹا(buf, val, sz);
	آزاد(val);

	واپس res;
}

صحیح ul_بفر_سیٹ_ڈیٹا(ساخت ul_بفر *buf, ثابت حرف *ڈیٹا, size_t sz)
{
	ul_بفر_reset_ڈیٹا(buf);
	واپس ul_بفر_append_ڈیٹا(buf, ڈیٹا, sz);
}

حرف *ul_بفر_get_ڈیٹا(ساخت ul_بفر *buf, size_t *sz, size_t *چوڑائی)
{
	اگر (sz)
		*sz = buf->انتہا - buf->begin;
	اگر (چوڑائی)
		*چوڑائی = buf->begin && *buf->begin ? mbs_چوڑائی(buf->begin) : 0;
	واپس buf->begin;
}

حرف *ul_بفر_get_string(ساخت ul_بفر *buf, size_t *sz, size_t *چوڑائی)
{
	حرف *ret;

	ret = ul_بفر_get_ڈیٹا(buf, sz, چوڑائی);

	/* data in buffer is already zero-terminated */
	اگر (sz)
		*sz = *sz + 1;

	واپس ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_بفر_get_bufsiz(ساخت ul_بفر *buf)
{
	واپس buf->sz;
}

size_t ul_بفر_get_datasiz(ساخت ul_بفر *buf)
{
	واپس buf->انتہا - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
حرف *ul_بفر_get_محفوظ_ڈیٹا(ساخت ul_بفر *buf, size_t *sz, size_t *چوڑائی, ثابت حرف *safechars)
{
	حرف *ڈیٹا = ul_بفر_get_ڈیٹا(buf, تہی, تہی);
	size_t encsz, wsz = 0;
	حرف *res = تہی;

	اگر (!ڈیٹا)
		جاؤ nothing;

	encsz = mbs_محفوظ_encode_سائز(buf->sz) + 1;
	اگر (encsz > buf->encoded_sz) {
		حرف *tmp = realloc(buf->encoded, encsz);
		اگر (!tmp)
			جاؤ nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_محفوظ_encode_مقصد_بفر(ڈیٹا, &wsz, buf->encoded, buf->encoded_sz, safechars);
	اگر (!res || !wsz || wsz == (size_t) -1)
		جاؤ nothing;

	اگر (چوڑائی)
		*چوڑائی = wsz;
	اگر (sz)
		*sz = لڑی_لمبائی(res);
	واپس res;
nothing:
	اگر (چوڑائی)
		*چوڑائی = 0;
	اگر (sz)
		*sz = 0;
	واپس تہی;
}


#اگر_تعریف_شدہ TEST_پروگرام_بفر

شمار {
	PTR_AAA  = 0,
	PTR_BBB,
};

صحیح مین(خالی)
{
	ساخت ul_بفر buf = UL_INIT_بفر;
	حرف *str;
	size_t sz = 0;

	ul_بفر_سیٹ_chunksize(&buf, 16);

	ul_بفر_append_string(&buf, "AAA");
	ul_بفر_append_ڈیٹا(&buf, "=", 1);
	ul_بفر_append_string(&buf, "aaa");
	ul_بفر_save_اشارہ_گر(&buf, PTR_AAA);

	ul_بفر_append_ڈیٹا(&buf, ",", 1);
	ul_بفر_append_string(&buf, "BBB");
	ul_بفر_append_string(&buf, "=");
	ul_بفر_append_string(&buf, "bbb");
	ul_بفر_save_اشارہ_گر(&buf, PTR_BBB);

	str = ul_بفر_get_ڈیٹا(&buf, &sz, تہی);
	لکھو("data [%zu] '%s'\n", sz, str);

	لکھو(" pointer data len: AAA=%zu, BBB=%zu\n",
			ul_بفر_get_اشارہ_گر_لمبائی(&buf, PTR_AAA),
			ul_بفر_get_اشارہ_گر_لمبائی(&buf, PTR_BBB));
	لکھو(" pointer data width: AAA=%zu, BBB=%zu\n",
			ul_بفر_get_محفوظ_اشارہ_گر_چوڑائی(&buf, PTR_AAA),
			ul_بفر_get_محفوظ_اشارہ_گر_چوڑائی(&buf, PTR_BBB));

	ul_بفر_reset_ڈیٹا(&buf);
	ul_بفر_append_string(&buf, "This is really long string to test the buffer function.");
	ul_بفر_save_اشارہ_گر(&buf, PTR_AAA);
	ul_بفر_append_string(&buf, " YES!");
	str = ul_بفر_get_ڈیٹا(&buf, &sz, تہی);
	لکھو("data [%zu] '%s'\n", sz, str);
	لکھو(" pointer data len: AAA=%zu\n", ul_بفر_get_اشارہ_گر_لمبائی(&buf, PTR_AAA));

	ul_بفر_free_ڈیٹا(&buf);
	str = strdup("foo");
	ul_بفر_refer_string(&buf, str);
	ul_بفر_append_ڈیٹا(&buf, ",", 1);
	ul_بفر_append_string(&buf, "bar");
	str = ul_بفر_get_ڈیٹا(&buf, &sz, تہی);
	لکھو("data [%zu] '%s'\n", sz, str);

	ul_بفر_free_ڈیٹا(&buf);
	ul_بفر_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_بفر_get_ڈیٹا(&buf, &sz, تہی);
	لکھو("data [%zu] '%s'\n", sz, str);

	ul_بفر_free_ڈیٹا(&buf);
	ul_بفر_append_char(&buf, 'x');
	{
		حرف c = 'y';
		ul_بفر_append_char(&buf, c);
	}
	{
		ثابت حرف c = 'z';
		ul_بفر_append_char(&buf, c);
	}
	str = ul_بفر_get_ڈیٹا(&buf, &sz, تہی);
	لکھو("data [%zu] '%s'\n", sz, str);

	ul_بفر_free_ڈیٹا(&buf);

	واپس EXIT_SUCCESS;
}
#اگر_ختم /* TEST_PROGRAM_BUFFER */
