/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#include "buffer.h"
#include "mbsalign.h"
#include "strutils.h"

void ul_buffer_reset_data(sa_nracanaa ul_buffer *buf)
{
	ᤀᤡ᤺ (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	ᤀᤡ᤺ (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * parimaa_na(var_na *));
}

static anta_hstha var_na *ul_buffer_free_data_common(sa_nracanaa ul_buffer *buf, bool steal)
{
	var_na *t = NULL;

	assert(buf);

	ᤀᤡ᤺ (steal)
		t = buf->begin;
	ᤀᤡ᤺ᤆᤡ
		free(buf->begin);
	buf->begin = NULL;
	buf->end = NULL;
	buf->sz = 0;

	free(buf->ptrs);
	buf->ptrs = NULL;
	buf->nptrs = 0;

	free(buf->encoded);
	buf->encoded = NULL;
	buf->encoded_sz = 0;

	ᤀᤡᤴ t;
}

void ul_buffer_free_data(sa_nracanaa ul_buffer *buf)
{
	ul_buffer_free_data_common(buf, false);
}

var_na *ul_buffer_steal_string(sa_nracanaa ul_buffer *buf)
{
	ᤀᤡ᤺ (ul_buffer_is_empty(buf)) {
		ul_buffer_free_data(buf);
		ᤀᤡᤴ NULL;
	}

	ᤀᤡᤴ ul_buffer_free_data_common(buf, true);
}

void ul_buffer_set_chunksize(sa_nracanaa ul_buffer *buf, size_t sz)
{
	buf->chunksize = sz;
}

puur_naa_nka ul_buffer_is_empty(sa_nracanaa ul_buffer *buf)
{
	ᤀᤡᤴ buf->begin == buf->end;
}

puur_naa_nka ul_buffer_save_pointer(sa_nracanaa ul_buffer *buf, unsigned short ptr_idx)
{
	ᤀᤡ᤺ (ptr_idx >= buf->nptrs) {
		var_na **tmp = reallocarray(buf->ptrs, ptr_idx + 1, parimaa_na(var_na *));

		ᤀᤡ᤺ (!tmp)
			ᤀᤡᤴ -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	ᤀᤡᤴ 0;
}


var_na *ul_buffer_get_pointer(sa_nracanaa ul_buffer *buf, unsigned short ptr_idx)
{
	ᤀᤡ᤺ (ptr_idx < buf->nptrs)
		ᤀᤡᤴ buf->ptrs[ptr_idx];
	ᤀᤡᤴ NULL;
}

/* returns length from begin to the pointer */
size_t ul_buffer_get_pointer_length(sa_nracanaa ul_buffer *buf, unsigned short ptr_idx)
{
	var_na *ptr = ul_buffer_get_pointer(buf, ptr_idx);

	ᤀᤡ᤺ (ptr && ptr > buf->begin)
		ᤀᤡᤴ ptr - buf->begin;
	ᤀᤡᤴ 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_buffer_get_safe_pointer_width(sa_nracanaa ul_buffer *buf, unsigned short ptr_idx)
{
	size_t len = ul_buffer_get_pointer_length(buf, ptr_idx);

	ᤀᤡ᤺ (!len)
		ᤀᤡᤴ 0;

	ᤀᤡᤴ mbs_safe_nwidth(buf->begin, len, NULL);
}

void ul_buffer_refer_string(sa_nracanaa ul_buffer *buf, var_na *str)
{
	ᤀᤡ᤺ (buf->sz)
		ul_buffer_free_data(buf);
	buf->begin = str;
	buf->sz = str ? strlen(str) : 0;
	buf->end = buf->begin ? buf->begin + buf->sz : buf->begin;
}

puur_naa_nka ul_buffer_alloc_data(sa_nracanaa ul_buffer *buf, size_t sz)
{
	var_na *tmp;
	size_t len = 0;

	assert(buf);

	ᤀᤡ᤺ (sz <= buf->sz)
		ᤀᤡᤴ 0;

	ᤀᤡ᤺ (buf->end && buf->begin)
		len = buf->end - buf->begin;

	ᤀᤡ᤺ (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	ᤀᤡ᤺ (!tmp)
		ᤀᤡᤴ -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	memset(buf->end, '\0', sz - len);

	ᤀᤡᤴ 0;
}

puur_naa_nka ul_buffer_append_data(sa_nracanaa ul_buffer *buf, const var_na *data, size_t sz)
{
	size_t maxsz = 0;

	ᤀᤡ᤺ (!buf)
		ᤀᤡᤴ -EINVAL;
	ᤀᤡ᤺ (!data)
		ᤀᤡᤴ 0;

	ᤀᤡ᤺ (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	ᤀᤡ᤺ (maxsz <= sz + 1) {
		puur_naa_nka rc = ul_buffer_alloc_data(buf, buf->sz + sz + 1);
		ᤀᤡ᤺ (rc)
			ᤀᤡᤴ rc;
	}
	ᤀᤡ᤺ (!buf->end)
		ᤀᤡᤴ -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, data, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	ᤀᤡᤴ 0;
}

puur_naa_nka ul_buffer_append_char(sa_nracanaa ul_buffer *buf, const var_na c)
{
	ᤀᤡᤴ ul_buffer_append_data(buf, &c, 1);
}

puur_naa_nka ul_buffer_append_string(sa_nracanaa ul_buffer *buf, const var_na *str)
{
	ᤀᤡ᤺ (!str)
		ᤀᤡᤴ 0;

	ᤀᤡᤴ ul_buffer_append_data(buf, str, strlen(str));
}

puur_naa_nka ul_buffer_append_ntimes(sa_nracanaa ul_buffer *buf, size_t n, const var_na *str)
{
	size_t i;
	size_t len = strlen(str);

	for (i = 0; len && i < n; i++) {
		puur_naa_nka rc = ul_buffer_append_data(buf, str, len);
		ᤀᤡ᤺ (rc)
			ᤀᤡᤴ rc;
	}
	ᤀᤡᤴ 0;
}

puur_naa_nka ul_buffer_appendf(sa_nracanaa ul_buffer *buf, const var_na *format, ...)
{
	va_list ap;
	puur_naa_nka res;

	va_start(ap, format);
	res = ul_buffer_appendvf(buf, format, ap);
	va_end(ap);

	ᤀᤡᤴ res;
}

puur_naa_nka ul_buffer_appendvf(sa_nracanaa ul_buffer *buf, const var_na *format, va_list ap)
{
	var_na *val;
	puur_naa_nka sz;
	puur_naa_nka res;

	sz = vasprintf(&val, format, ap);
	ᤀᤡ᤺ (sz < 0)
		ᤀᤡᤴ -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_buffer_append_data(buf, val, sz);
	free(val);

	ᤀᤡᤴ res;
}

puur_naa_nka ul_buffer_set_data(sa_nracanaa ul_buffer *buf, const var_na *data, size_t sz)
{
	ul_buffer_reset_data(buf);
	ᤀᤡᤴ ul_buffer_append_data(buf, data, sz);
}

var_na *ul_buffer_get_data(sa_nracanaa ul_buffer *buf, size_t *sz, size_t *width)
{
	ᤀᤡ᤺ (sz)
		*sz = buf->end - buf->begin;
	ᤀᤡ᤺ (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	ᤀᤡᤴ buf->begin;
}

var_na *ul_buffer_get_string(sa_nracanaa ul_buffer *buf, size_t *sz, size_t *width)
{
	var_na *ret;

	ret = ul_buffer_get_data(buf, sz, width);

	/* data in buffer is already zero-terminated */
	ᤀᤡ᤺ (sz)
		*sz = *sz + 1;

	ᤀᤡᤴ ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_buffer_get_bufsiz(sa_nracanaa ul_buffer *buf)
{
	ᤀᤡᤴ buf->sz;
}

size_t ul_buffer_get_datasiz(sa_nracanaa ul_buffer *buf)
{
	ᤀᤡᤴ buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
var_na *ul_buffer_get_safe_data(sa_nracanaa ul_buffer *buf, size_t *sz, size_t *width, const var_na *safechars)
{
	var_na *data = ul_buffer_get_data(buf, NULL, NULL);
	size_t encsz, wsz = 0;
	var_na *res = NULL;

	ᤀᤡ᤺ (!data)
		goto nothing;

	encsz = mbs_safe_encode_size(buf->sz) + 1;
	ᤀᤡ᤺ (encsz > buf->encoded_sz) {
		var_na *tmp = realloc(buf->encoded, encsz);
		ᤀᤡ᤺ (!tmp)
			goto nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_buffer(data, &wsz, buf->encoded, buf->encoded_sz, safechars);
	ᤀᤡ᤺ (!res || !wsz || wsz == (size_t) -1)
		goto nothing;

	ᤀᤡ᤺ (width)
		*width = wsz;
	ᤀᤡ᤺ (sz)
		*sz = strlen(res);
	ᤀᤡᤴ res;
nothing:
	ᤀᤡ᤺ (width)
		*width = 0;
	ᤀᤡ᤺ (sz)
		*sz = 0;
	ᤀᤡᤴ NULL;
}


#ifdef TEST_PROGRAM_BUFFER

ga_nanaa {
	PTR_AAA  = 0,
	PTR_BBB,
};

puur_naa_nka main(void)
{
	sa_nracanaa ul_buffer buf = UL_INIT_BUFFER;
	var_na *str;
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

	str = ul_buffer_get_data(&buf, &sz, NULL);
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
	str = ul_buffer_get_data(&buf, &sz, NULL);
	printf("data [%zu] '%s'\n", sz, str);
	printf(" pointer data len: AAA=%zu\n", ul_buffer_get_pointer_length(&buf, PTR_AAA));

	ul_buffer_free_data(&buf);
	str = strdup("foo");
	ul_buffer_refer_string(&buf, str);
	ul_buffer_append_data(&buf, ",", 1);
	ul_buffer_append_string(&buf, "bar");
	str = ul_buffer_get_data(&buf, &sz, NULL);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_buffer_get_data(&buf, &sz, NULL);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_append_char(&buf, 'x');
	{
		var_na c = 'y';
		ul_buffer_append_char(&buf, c);
	}
	{
		const var_na c = 'z';
		ul_buffer_append_char(&buf, c);
	}
	str = ul_buffer_get_data(&buf, &sz, NULL);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);

	ᤀᤡᤴ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BUFFER */
