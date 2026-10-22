/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#include "buffer.h"
#include "mbsalign.h"
#include "strutils.h"

void ul_buffer_reset_data(struct ul_buffer *buf)
{
	nếu (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	nếu (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * sizeof(char *));
}

static inline char *ul_buffer_free_data_common(struct ul_buffer *buf, bool steal)
{
	char *t = NULL;

	assert(buf);

	nếu (steal)
		t = buf->begin;
	không_thì
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

	trả_về t;
}

void ul_buffer_free_data(struct ul_buffer *buf)
{
	ul_buffer_free_data_common(buf, false);
}

char *ul_buffer_steal_string(struct ul_buffer *buf)
{
	nếu (ul_buffer_is_empty(buf)) {
		ul_buffer_free_data(buf);
		trả_về NULL;
	}

	trả_về ul_buffer_free_data_common(buf, true);
}

void ul_buffer_set_chunksize(struct ul_buffer *buf, size_t sz)
{
	buf->chunksize = sz;
}

số_nguyên ul_buffer_is_empty(struct ul_buffer *buf)
{
	trả_về buf->begin == buf->end;
}

số_nguyên ul_buffer_save_pointer(struct ul_buffer *buf, unsigned short ptr_idx)
{
	nếu (ptr_idx >= buf->nptrs) {
		char **tmp = reallocarray(buf->ptrs, ptr_idx + 1, sizeof(char *));

		nếu (!tmp)
			trả_về -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	trả_về 0;
}


char *ul_buffer_get_pointer(struct ul_buffer *buf, unsigned short ptr_idx)
{
	nếu (ptr_idx < buf->nptrs)
		trả_về buf->ptrs[ptr_idx];
	trả_về NULL;
}

/* returns length from begin to the pointer */
size_t ul_buffer_get_pointer_length(struct ul_buffer *buf, unsigned short ptr_idx)
{
	char *ptr = ul_buffer_get_pointer(buf, ptr_idx);

	nếu (ptr && ptr > buf->begin)
		trả_về ptr - buf->begin;
	trả_về 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_buffer_get_safe_pointer_width(struct ul_buffer *buf, unsigned short ptr_idx)
{
	size_t len = ul_buffer_get_pointer_length(buf, ptr_idx);

	nếu (!len)
		trả_về 0;

	trả_về mbs_safe_nwidth(buf->begin, len, NULL);
}

void ul_buffer_refer_string(struct ul_buffer *buf, char *str)
{
	nếu (buf->sz)
		ul_buffer_free_data(buf);
	buf->begin = str;
	buf->sz = str ? strlen(str) : 0;
	buf->end = buf->begin ? buf->begin + buf->sz : buf->begin;
}

số_nguyên ul_buffer_alloc_data(struct ul_buffer *buf, size_t sz)
{
	char *tmp;
	size_t len = 0;

	assert(buf);

	nếu (sz <= buf->sz)
		trả_về 0;

	nếu (buf->end && buf->begin)
		len = buf->end - buf->begin;

	nếu (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	nếu (!tmp)
		trả_về -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	memset(buf->end, '\0', sz - len);

	trả_về 0;
}

số_nguyên ul_buffer_append_data(struct ul_buffer *buf, const char *data, size_t sz)
{
	size_t maxsz = 0;

	nếu (!buf)
		trả_về -EINVAL;
	nếu (!data)
		trả_về 0;

	nếu (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	nếu (maxsz <= sz + 1) {
		số_nguyên rc = ul_buffer_alloc_data(buf, buf->sz + sz + 1);
		nếu (rc)
			trả_về rc;
	}
	nếu (!buf->end)
		trả_về -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, data, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	trả_về 0;
}

số_nguyên ul_buffer_append_char(struct ul_buffer *buf, const char c)
{
	trả_về ul_buffer_append_data(buf, &c, 1);
}

số_nguyên ul_buffer_append_string(struct ul_buffer *buf, const char *str)
{
	nếu (!str)
		trả_về 0;

	trả_về ul_buffer_append_data(buf, str, strlen(str));
}

số_nguyên ul_buffer_append_ntimes(struct ul_buffer *buf, size_t n, const char *str)
{
	size_t i;
	size_t len = strlen(str);

	cho (i = 0; len && i < n; i++) {
		số_nguyên rc = ul_buffer_append_data(buf, str, len);
		nếu (rc)
			trả_về rc;
	}
	trả_về 0;
}

số_nguyên ul_buffer_appendf(struct ul_buffer *buf, const char *format, ...)
{
	va_list ap;
	số_nguyên res;

	va_start(ap, format);
	res = ul_buffer_appendvf(buf, format, ap);
	va_end(ap);

	trả_về res;
}

số_nguyên ul_buffer_appendvf(struct ul_buffer *buf, const char *format, va_list ap)
{
	char *val;
	số_nguyên sz;
	số_nguyên res;

	sz = vasprintf(&val, format, ap);
	nếu (sz < 0)
		trả_về -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_buffer_append_data(buf, val, sz);
	free(val);

	trả_về res;
}

số_nguyên ul_buffer_set_data(struct ul_buffer *buf, const char *data, size_t sz)
{
	ul_buffer_reset_data(buf);
	trả_về ul_buffer_append_data(buf, data, sz);
}

char *ul_buffer_get_data(struct ul_buffer *buf, size_t *sz, size_t *width)
{
	nếu (sz)
		*sz = buf->end - buf->begin;
	nếu (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	trả_về buf->begin;
}

char *ul_buffer_get_string(struct ul_buffer *buf, size_t *sz, size_t *width)
{
	char *ret;

	ret = ul_buffer_get_data(buf, sz, width);

	/* data in buffer is already zero-terminated */
	nếu (sz)
		*sz = *sz + 1;

	trả_về ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_buffer_get_bufsiz(struct ul_buffer *buf)
{
	trả_về buf->sz;
}

size_t ul_buffer_get_datasiz(struct ul_buffer *buf)
{
	trả_về buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
char *ul_buffer_get_safe_data(struct ul_buffer *buf, size_t *sz, size_t *width, const char *safechars)
{
	char *data = ul_buffer_get_data(buf, NULL, NULL);
	size_t encsz, wsz = 0;
	char *res = NULL;

	nếu (!data)
		goto nothing;

	encsz = mbs_safe_encode_size(buf->sz) + 1;
	nếu (encsz > buf->encoded_sz) {
		char *tmp = realloc(buf->encoded, encsz);
		nếu (!tmp)
			goto nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_buffer(data, &wsz, buf->encoded, buf->encoded_sz, safechars);
	nếu (!res || !wsz || wsz == (size_t) -1)
		goto nothing;

	nếu (width)
		*width = wsz;
	nếu (sz)
		*sz = strlen(res);
	trả_về res;
nothing:
	nếu (width)
		*width = 0;
	nếu (sz)
		*sz = 0;
	trả_về NULL;
}


#ifdef TEST_PROGRAM_BUFFER

enum {
	PTR_AAA  = 0,
	PTR_BBB,
};

số_nguyên main(void)
{
	struct ul_buffer buf = UL_INIT_BUFFER;
	char *str;
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
		char c = 'y';
		ul_buffer_append_char(&buf, c);
	}
	{
		const char c = 'z';
		ul_buffer_append_char(&buf, c);
	}
	str = ul_buffer_get_data(&buf, &sz, NULL);
	printf("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);

	trả_về EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BUFFER */
