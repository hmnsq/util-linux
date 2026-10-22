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
	если (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	если (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * sizeof(char *));
}

static inline char *ul_buffer_free_data_common(struct ul_buffer *buf, bool steal)
{
	char *t = NULL;

	assert(buf);

	если (steal)
		t = buf->begin;
	иначе
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

	вернуть t;
}

void ul_buffer_free_data(struct ul_buffer *buf)
{
	ul_buffer_free_data_common(buf, false);
}

char *ul_buffer_steal_string(struct ul_buffer *buf)
{
	если (ul_buffer_is_empty(buf)) {
		ul_buffer_free_data(buf);
		вернуть NULL;
	}

	вернуть ul_buffer_free_data_common(buf, true);
}

void ul_buffer_set_chunksize(struct ul_buffer *buf, size_t sz)
{
	buf->chunksize = sz;
}

целое ul_buffer_is_empty(struct ul_buffer *buf)
{
	вернуть buf->begin == buf->end;
}

целое ul_buffer_save_pointer(struct ul_buffer *buf, unsigned short ptr_idx)
{
	если (ptr_idx >= buf->nptrs) {
		char **tmp = reallocarray(buf->ptrs, ptr_idx + 1, sizeof(char *));

		если (!tmp)
			вернуть -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	вернуть 0;
}


char *ul_buffer_get_pointer(struct ul_buffer *buf, unsigned short ptr_idx)
{
	если (ptr_idx < buf->nptrs)
		вернуть buf->ptrs[ptr_idx];
	вернуть NULL;
}

/* returns length from begin to the pointer */
size_t ul_buffer_get_pointer_length(struct ul_buffer *buf, unsigned short ptr_idx)
{
	char *ptr = ul_buffer_get_pointer(buf, ptr_idx);

	если (ptr && ptr > buf->begin)
		вернуть ptr - buf->begin;
	вернуть 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_buffer_get_safe_pointer_width(struct ul_buffer *buf, unsigned short ptr_idx)
{
	size_t len = ul_buffer_get_pointer_length(buf, ptr_idx);

	если (!len)
		вернуть 0;

	вернуть mbs_safe_nwidth(buf->begin, len, NULL);
}

void ul_buffer_refer_string(struct ul_buffer *buf, char *str)
{
	если (buf->sz)
		ul_buffer_free_data(buf);
	buf->begin = str;
	buf->sz = str ? strlen(str) : 0;
	buf->end = buf->begin ? buf->begin + buf->sz : buf->begin;
}

целое ul_buffer_alloc_data(struct ul_buffer *buf, size_t sz)
{
	char *tmp;
	size_t len = 0;

	assert(buf);

	если (sz <= buf->sz)
		вернуть 0;

	если (buf->end && buf->begin)
		len = buf->end - buf->begin;

	если (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	если (!tmp)
		вернуть -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	memset(buf->end, '\0', sz - len);

	вернуть 0;
}

целое ul_buffer_append_data(struct ul_buffer *buf, const char *data, size_t sz)
{
	size_t maxsz = 0;

	если (!buf)
		вернуть -EINVAL;
	если (!data)
		вернуть 0;

	если (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	если (maxsz <= sz + 1) {
		целое rc = ul_buffer_alloc_data(buf, buf->sz + sz + 1);
		если (rc)
			вернуть rc;
	}
	если (!buf->end)
		вернуть -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, data, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	вернуть 0;
}

целое ul_buffer_append_char(struct ul_buffer *buf, const char c)
{
	вернуть ul_buffer_append_data(buf, &c, 1);
}

целое ul_buffer_append_string(struct ul_buffer *buf, const char *str)
{
	если (!str)
		вернуть 0;

	вернуть ul_buffer_append_data(buf, str, strlen(str));
}

целое ul_buffer_append_ntimes(struct ul_buffer *buf, size_t n, const char *str)
{
	size_t i;
	size_t len = strlen(str);

	для (i = 0; len && i < n; i++) {
		целое rc = ul_buffer_append_data(buf, str, len);
		если (rc)
			вернуть rc;
	}
	вернуть 0;
}

целое ul_buffer_appendf(struct ul_buffer *buf, const char *format, ...)
{
	va_list ap;
	целое res;

	va_start(ap, format);
	res = ul_buffer_appendvf(buf, format, ap);
	va_end(ap);

	вернуть res;
}

целое ul_buffer_appendvf(struct ul_buffer *buf, const char *format, va_list ap)
{
	char *val;
	целое sz;
	целое res;

	sz = vasprintf(&val, format, ap);
	если (sz < 0)
		вернуть -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_buffer_append_data(buf, val, sz);
	free(val);

	вернуть res;
}

целое ul_buffer_set_data(struct ul_buffer *buf, const char *data, size_t sz)
{
	ul_buffer_reset_data(buf);
	вернуть ul_buffer_append_data(buf, data, sz);
}

char *ul_buffer_get_data(struct ul_buffer *buf, size_t *sz, size_t *width)
{
	если (sz)
		*sz = buf->end - buf->begin;
	если (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	вернуть buf->begin;
}

char *ul_buffer_get_string(struct ul_buffer *buf, size_t *sz, size_t *width)
{
	char *ret;

	ret = ul_buffer_get_data(buf, sz, width);

	/* data in buffer is already zero-terminated */
	если (sz)
		*sz = *sz + 1;

	вернуть ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_buffer_get_bufsiz(struct ul_buffer *buf)
{
	вернуть buf->sz;
}

size_t ul_buffer_get_datasiz(struct ul_buffer *buf)
{
	вернуть buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
char *ul_buffer_get_safe_data(struct ul_buffer *buf, size_t *sz, size_t *width, const char *safechars)
{
	char *data = ul_buffer_get_data(buf, NULL, NULL);
	size_t encsz, wsz = 0;
	char *res = NULL;

	если (!data)
		goto nothing;

	encsz = mbs_safe_encode_size(buf->sz) + 1;
	если (encsz > buf->encoded_sz) {
		char *tmp = realloc(buf->encoded, encsz);
		если (!tmp)
			goto nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_buffer(data, &wsz, buf->encoded, buf->encoded_sz, safechars);
	если (!res || !wsz || wsz == (size_t) -1)
		goto nothing;

	если (width)
		*width = wsz;
	если (sz)
		*sz = strlen(res);
	вернуть res;
nothing:
	если (width)
		*width = 0;
	если (sz)
		*sz = 0;
	вернуть NULL;
}


#ifdef TEST_PROGRAM_BUFFER

enum {
	PTR_AAA  = 0,
	PTR_BBB,
};

целое основной(void)
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

	вернуть EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BUFFER */
