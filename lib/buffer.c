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
	もし (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	もし (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * sizeof(char *));
}

static inline char *ul_buffer_free_data_common(struct ul_buffer *buf, bool steal)
{
	char *t = NULL;

	assert(buf);

	もし (steal)
		t = buf->begin;
	否则
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

	戻る t;
}

void ul_buffer_free_data(struct ul_buffer *buf)
{
	ul_buffer_free_data_common(buf, false);
}

char *ul_buffer_steal_string(struct ul_buffer *buf)
{
	もし (ul_buffer_is_empty(buf)) {
		ul_buffer_free_data(buf);
		戻る NULL;
	}

	戻る ul_buffer_free_data_common(buf, true);
}

void ul_buffer_set_chunksize(struct ul_buffer *buf, size_t sz)
{
	buf->chunksize = sz;
}

整数 ul_buffer_is_empty(struct ul_buffer *buf)
{
	戻る buf->begin == buf->end;
}

整数 ul_buffer_save_pointer(struct ul_buffer *buf, unsigned short ptr_idx)
{
	もし (ptr_idx >= buf->nptrs) {
		char **tmp = reallocarray(buf->ptrs, ptr_idx + 1, sizeof(char *));

		もし (!tmp)
			戻る -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	戻る 0;
}


char *ul_buffer_get_pointer(struct ul_buffer *buf, unsigned short ptr_idx)
{
	もし (ptr_idx < buf->nptrs)
		戻る buf->ptrs[ptr_idx];
	戻る NULL;
}

/* returns length from begin to the pointer */
size_t ul_buffer_get_pointer_length(struct ul_buffer *buf, unsigned short ptr_idx)
{
	char *ptr = ul_buffer_get_pointer(buf, ptr_idx);

	もし (ptr && ptr > buf->begin)
		戻る ptr - buf->begin;
	戻る 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_buffer_get_safe_pointer_width(struct ul_buffer *buf, unsigned short ptr_idx)
{
	size_t len = ul_buffer_get_pointer_length(buf, ptr_idx);

	もし (!len)
		戻る 0;

	戻る mbs_safe_nwidth(buf->begin, len, NULL);
}

void ul_buffer_refer_string(struct ul_buffer *buf, char *str)
{
	もし (buf->sz)
		ul_buffer_free_data(buf);
	buf->begin = str;
	buf->sz = str ? strlen(str) : 0;
	buf->end = buf->begin ? buf->begin + buf->sz : buf->begin;
}

整数 ul_buffer_alloc_data(struct ul_buffer *buf, size_t sz)
{
	char *tmp;
	size_t len = 0;

	assert(buf);

	もし (sz <= buf->sz)
		戻る 0;

	もし (buf->end && buf->begin)
		len = buf->end - buf->begin;

	もし (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	もし (!tmp)
		戻る -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	memset(buf->end, '\0', sz - len);

	戻る 0;
}

整数 ul_buffer_append_data(struct ul_buffer *buf, const char *data, size_t sz)
{
	size_t maxsz = 0;

	もし (!buf)
		戻る -EINVAL;
	もし (!data)
		戻る 0;

	もし (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	もし (maxsz <= sz + 1) {
		整数 rc = ul_buffer_alloc_data(buf, buf->sz + sz + 1);
		もし (rc)
			戻る rc;
	}
	もし (!buf->end)
		戻る -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, data, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	戻る 0;
}

整数 ul_buffer_append_char(struct ul_buffer *buf, const char c)
{
	戻る ul_buffer_append_data(buf, &c, 1);
}

整数 ul_buffer_append_string(struct ul_buffer *buf, const char *str)
{
	もし (!str)
		戻る 0;

	戻る ul_buffer_append_data(buf, str, strlen(str));
}

整数 ul_buffer_append_ntimes(struct ul_buffer *buf, size_t n, const char *str)
{
	size_t i;
	size_t len = strlen(str);

	ために (i = 0; len && i < n; i++) {
		整数 rc = ul_buffer_append_data(buf, str, len);
		もし (rc)
			戻る rc;
	}
	戻る 0;
}

整数 ul_buffer_appendf(struct ul_buffer *buf, const char *format, ...)
{
	va_list ap;
	整数 res;

	va_start(ap, format);
	res = ul_buffer_appendvf(buf, format, ap);
	va_end(ap);

	戻る res;
}

整数 ul_buffer_appendvf(struct ul_buffer *buf, const char *format, va_list ap)
{
	char *val;
	整数 sz;
	整数 res;

	sz = vasprintf(&val, format, ap);
	もし (sz < 0)
		戻る -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_buffer_append_data(buf, val, sz);
	free(val);

	戻る res;
}

整数 ul_buffer_set_data(struct ul_buffer *buf, const char *data, size_t sz)
{
	ul_buffer_reset_data(buf);
	戻る ul_buffer_append_data(buf, data, sz);
}

char *ul_buffer_get_data(struct ul_buffer *buf, size_t *sz, size_t *width)
{
	もし (sz)
		*sz = buf->end - buf->begin;
	もし (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	戻る buf->begin;
}

char *ul_buffer_get_string(struct ul_buffer *buf, size_t *sz, size_t *width)
{
	char *ret;

	ret = ul_buffer_get_data(buf, sz, width);

	/* data in buffer is already zero-terminated */
	もし (sz)
		*sz = *sz + 1;

	戻る ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_buffer_get_bufsiz(struct ul_buffer *buf)
{
	戻る buf->sz;
}

size_t ul_buffer_get_datasiz(struct ul_buffer *buf)
{
	戻る buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
char *ul_buffer_get_safe_data(struct ul_buffer *buf, size_t *sz, size_t *width, const char *safechars)
{
	char *data = ul_buffer_get_data(buf, NULL, NULL);
	size_t encsz, wsz = 0;
	char *res = NULL;

	もし (!data)
		goto nothing;

	encsz = mbs_safe_encode_size(buf->sz) + 1;
	もし (encsz > buf->encoded_sz) {
		char *tmp = realloc(buf->encoded, encsz);
		もし (!tmp)
			goto nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_buffer(data, &wsz, buf->encoded, buf->encoded_sz, safechars);
	もし (!res || !wsz || wsz == (size_t) -1)
		goto nothing;

	もし (width)
		*width = wsz;
	もし (sz)
		*sz = strlen(res);
	戻る res;
nothing:
	もし (width)
		*width = 0;
	もし (sz)
		*sz = 0;
	戻る NULL;
}


#ifdef TEST_PROGRAM_BUFFER

enum {
	PTR_AAA  = 0,
	PTR_BBB,
};

整数 メインページ(void)
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

	戻る EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BUFFER */
