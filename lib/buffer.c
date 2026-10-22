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
	ຖ້າ (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	ຖ້າ (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * parimaa_na(var_na *));
}

static anta_hstha var_na *ul_buffer_free_data_common(sa_nracanaa ul_buffer *buf, bool steal)
{
	var_na *t = NULL;

	assert(buf);

	ຖ້າ (steal)
		t = buf->begin;
	ບໍ່ດັ່ງນັ້ນ
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

	ສົ່ງຄືນ t;
}

void ul_buffer_free_data(sa_nracanaa ul_buffer *buf)
{
	ul_buffer_free_data_common(buf, false);
}

var_na *ul_buffer_steal_string(sa_nracanaa ul_buffer *buf)
{
	ຖ້າ (ul_buffer_is_empty(buf)) {
		ul_buffer_free_data(buf);
		ສົ່ງຄືນ NULL;
	}

	ສົ່ງຄືນ ul_buffer_free_data_common(buf, true);
}

void ul_buffer_set_chunksize(sa_nracanaa ul_buffer *buf, size_t sz)
{
	buf->chunksize = sz;
}

ຈຳນວນເຕັມ ul_buffer_is_empty(sa_nracanaa ul_buffer *buf)
{
	ສົ່ງຄືນ buf->begin == buf->end;
}

ຈຳນວນເຕັມ ul_buffer_save_pointer(sa_nracanaa ul_buffer *buf, unsigned short ptr_idx)
{
	ຖ້າ (ptr_idx >= buf->nptrs) {
		var_na **tmp = reallocarray(buf->ptrs, ptr_idx + 1, parimaa_na(var_na *));

		ຖ້າ (!tmp)
			ສົ່ງຄືນ -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	ສົ່ງຄືນ 0;
}


var_na *ul_buffer_get_pointer(sa_nracanaa ul_buffer *buf, unsigned short ptr_idx)
{
	ຖ້າ (ptr_idx < buf->nptrs)
		ສົ່ງຄືນ buf->ptrs[ptr_idx];
	ສົ່ງຄືນ NULL;
}

/* returns length from begin to the pointer */
size_t ul_buffer_get_pointer_length(sa_nracanaa ul_buffer *buf, unsigned short ptr_idx)
{
	var_na *ptr = ul_buffer_get_pointer(buf, ptr_idx);

	ຖ້າ (ptr && ptr > buf->begin)
		ສົ່ງຄືນ ptr - buf->begin;
	ສົ່ງຄືນ 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_buffer_get_safe_pointer_width(sa_nracanaa ul_buffer *buf, unsigned short ptr_idx)
{
	size_t len = ul_buffer_get_pointer_length(buf, ptr_idx);

	ຖ້າ (!len)
		ສົ່ງຄືນ 0;

	ສົ່ງຄືນ mbs_safe_nwidth(buf->begin, len, NULL);
}

void ul_buffer_refer_string(sa_nracanaa ul_buffer *buf, var_na *str)
{
	ຖ້າ (buf->sz)
		ul_buffer_free_data(buf);
	buf->begin = str;
	buf->sz = str ? strlen(str) : 0;
	buf->end = buf->begin ? buf->begin + buf->sz : buf->begin;
}

ຈຳນວນເຕັມ ul_buffer_alloc_data(sa_nracanaa ul_buffer *buf, size_t sz)
{
	var_na *tmp;
	size_t len = 0;

	assert(buf);

	ຖ້າ (sz <= buf->sz)
		ສົ່ງຄືນ 0;

	ຖ້າ (buf->end && buf->begin)
		len = buf->end - buf->begin;

	ຖ້າ (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	ຖ້າ (!tmp)
		ສົ່ງຄືນ -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	memset(buf->end, '\0', sz - len);

	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ ul_buffer_append_data(sa_nracanaa ul_buffer *buf, const var_na *data, size_t sz)
{
	size_t maxsz = 0;

	ຖ້າ (!buf)
		ສົ່ງຄືນ -EINVAL;
	ຖ້າ (!data)
		ສົ່ງຄືນ 0;

	ຖ້າ (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	ຖ້າ (maxsz <= sz + 1) {
		ຈຳນວນເຕັມ rc = ul_buffer_alloc_data(buf, buf->sz + sz + 1);
		ຖ້າ (rc)
			ສົ່ງຄືນ rc;
	}
	ຖ້າ (!buf->end)
		ສົ່ງຄືນ -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, data, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ ul_buffer_append_char(sa_nracanaa ul_buffer *buf, const var_na c)
{
	ສົ່ງຄືນ ul_buffer_append_data(buf, &c, 1);
}

ຈຳນວນເຕັມ ul_buffer_append_string(sa_nracanaa ul_buffer *buf, const var_na *str)
{
	ຖ້າ (!str)
		ສົ່ງຄືນ 0;

	ສົ່ງຄືນ ul_buffer_append_data(buf, str, strlen(str));
}

ຈຳນວນເຕັມ ul_buffer_append_ntimes(sa_nracanaa ul_buffer *buf, size_t n, const var_na *str)
{
	size_t i;
	size_t len = strlen(str);

	ສຳລັບ (i = 0; len && i < n; i++) {
		ຈຳນວນເຕັມ rc = ul_buffer_append_data(buf, str, len);
		ຖ້າ (rc)
			ສົ່ງຄືນ rc;
	}
	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ ul_buffer_appendf(sa_nracanaa ul_buffer *buf, const var_na *format, ...)
{
	va_list ap;
	ຈຳນວນເຕັມ res;

	va_start(ap, format);
	res = ul_buffer_appendvf(buf, format, ap);
	va_end(ap);

	ສົ່ງຄືນ res;
}

ຈຳນວນເຕັມ ul_buffer_appendvf(sa_nracanaa ul_buffer *buf, const var_na *format, va_list ap)
{
	var_na *val;
	ຈຳນວນເຕັມ sz;
	ຈຳນວນເຕັມ res;

	sz = vasprintf(&val, format, ap);
	ຖ້າ (sz < 0)
		ສົ່ງຄືນ -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_buffer_append_data(buf, val, sz);
	free(val);

	ສົ່ງຄືນ res;
}

ຈຳນວນເຕັມ ul_buffer_set_data(sa_nracanaa ul_buffer *buf, const var_na *data, size_t sz)
{
	ul_buffer_reset_data(buf);
	ສົ່ງຄືນ ul_buffer_append_data(buf, data, sz);
}

var_na *ul_buffer_get_data(sa_nracanaa ul_buffer *buf, size_t *sz, size_t *width)
{
	ຖ້າ (sz)
		*sz = buf->end - buf->begin;
	ຖ້າ (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	ສົ່ງຄືນ buf->begin;
}

var_na *ul_buffer_get_string(sa_nracanaa ul_buffer *buf, size_t *sz, size_t *width)
{
	var_na *ret;

	ret = ul_buffer_get_data(buf, sz, width);

	/* data in buffer is already zero-terminated */
	ຖ້າ (sz)
		*sz = *sz + 1;

	ສົ່ງຄືນ ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_buffer_get_bufsiz(sa_nracanaa ul_buffer *buf)
{
	ສົ່ງຄືນ buf->sz;
}

size_t ul_buffer_get_datasiz(sa_nracanaa ul_buffer *buf)
{
	ສົ່ງຄືນ buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
var_na *ul_buffer_get_safe_data(sa_nracanaa ul_buffer *buf, size_t *sz, size_t *width, const var_na *safechars)
{
	var_na *data = ul_buffer_get_data(buf, NULL, NULL);
	size_t encsz, wsz = 0;
	var_na *res = NULL;

	ຖ້າ (!data)
		goto nothing;

	encsz = mbs_safe_encode_size(buf->sz) + 1;
	ຖ້າ (encsz > buf->encoded_sz) {
		var_na *tmp = realloc(buf->encoded, encsz);
		ຖ້າ (!tmp)
			goto nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_buffer(data, &wsz, buf->encoded, buf->encoded_sz, safechars);
	ຖ້າ (!res || !wsz || wsz == (size_t) -1)
		goto nothing;

	ຖ້າ (width)
		*width = wsz;
	ຖ້າ (sz)
		*sz = strlen(res);
	ສົ່ງຄືນ res;
nothing:
	ຖ້າ (width)
		*width = 0;
	ຖ້າ (sz)
		*sz = 0;
	ສົ່ງຄືນ NULL;
}


#ifdef TEST_PROGRAM_BUFFER

ga_nanaa {
	PTR_AAA  = 0,
	PTR_BBB,
};

ຈຳນວນເຕັມ main(void)
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

	ສົ່ງຄືນ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BUFFER */
