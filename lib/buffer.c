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
	ако (buf->begin)
		memset(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	ако (buf->ptrs && buf->nptrs)
		memset(buf->ptrs, 0, buf->nptrs * sizeof(char *));
}

static inline char *ul_buffer_free_data_common(struct ul_buffer *buf, bool steal)
{
	char *t = NULL;

	assert(buf);

	ако (steal)
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

	върни t;
}

void ul_buffer_free_data(struct ul_buffer *buf)
{
	ul_buffer_free_data_common(buf, false);
}

char *ul_buffer_steal_string(struct ul_buffer *buf)
{
	ако (ul_buffer_is_empty(buf)) {
		ul_buffer_free_data(buf);
		върни NULL;
	}

	върни ul_buffer_free_data_common(buf, true);
}

void ul_buffer_set_chunksize(struct ul_buffer *buf, size_t sz)
{
	buf->chunksize = sz;
}

цяло ul_buffer_is_empty(struct ul_buffer *buf)
{
	върни buf->begin == buf->end;
}

цяло ul_buffer_save_pointer(struct ul_buffer *buf, unsigned short ptr_idx)
{
	ако (ptr_idx >= buf->nptrs) {
		char **tmp = reallocarray(buf->ptrs, ptr_idx + 1, sizeof(char *));

		ако (!tmp)
			върни -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	върни 0;
}


char *ul_buffer_get_pointer(struct ul_buffer *buf, unsigned short ptr_idx)
{
	ако (ptr_idx < buf->nptrs)
		върни buf->ptrs[ptr_idx];
	върни NULL;
}

/* returns length from begin to the pointer */
size_t ul_buffer_get_pointer_length(struct ul_buffer *buf, unsigned short ptr_idx)
{
	char *ptr = ul_buffer_get_pointer(buf, ptr_idx);

	ако (ptr && ptr > buf->begin)
		върни ptr - buf->begin;
	върни 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
size_t ul_buffer_get_safe_pointer_width(struct ul_buffer *buf, unsigned short ptr_idx)
{
	size_t len = ul_buffer_get_pointer_length(buf, ptr_idx);

	ако (!len)
		върни 0;

	върни mbs_safe_nwidth(buf->begin, len, NULL);
}

void ul_buffer_refer_string(struct ul_buffer *buf, char *str)
{
	ако (buf->sz)
		ul_buffer_free_data(buf);
	buf->begin = str;
	buf->sz = str ? strlen(str) : 0;
	buf->end = buf->begin ? buf->begin + buf->sz : buf->begin;
}

цяло ul_buffer_alloc_data(struct ul_buffer *buf, size_t sz)
{
	char *tmp;
	size_t len = 0;

	assert(buf);

	ако (sz <= buf->sz)
		върни 0;

	ако (buf->end && buf->begin)
		len = buf->end - buf->begin;

	ако (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = realloc(buf->begin, sz);
	ако (!tmp)
		върни -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	memset(buf->end, '\0', sz - len);

	върни 0;
}

цяло ul_buffer_append_data(struct ul_buffer *buf, const char *data, size_t sz)
{
	size_t maxsz = 0;

	ако (!buf)
		върни -EINVAL;
	ако (!data)
		върни 0;

	ако (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	ако (maxsz <= sz + 1) {
		цяло rc = ul_buffer_alloc_data(buf, buf->sz + sz + 1);
		ако (rc)
			върни rc;
	}
	ако (!buf->end)
		върни -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, data, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	върни 0;
}

цяло ul_buffer_append_char(struct ul_buffer *buf, const char c)
{
	върни ul_buffer_append_data(buf, &c, 1);
}

цяло ul_buffer_append_string(struct ul_buffer *buf, const char *str)
{
	ако (!str)
		върни 0;

	върни ul_buffer_append_data(buf, str, strlen(str));
}

цяло ul_buffer_append_ntimes(struct ul_buffer *buf, size_t n, const char *str)
{
	size_t i;
	size_t len = strlen(str);

	за (i = 0; len && i < n; i++) {
		цяло rc = ul_buffer_append_data(buf, str, len);
		ако (rc)
			върни rc;
	}
	върни 0;
}

цяло ul_buffer_appendf(struct ul_buffer *buf, const char *format, ...)
{
	va_list ap;
	цяло res;

	va_start(ap, format);
	res = ul_buffer_appendvf(buf, format, ap);
	va_end(ap);

	върни res;
}

цяло ul_buffer_appendvf(struct ul_buffer *buf, const char *format, va_list ap)
{
	char *val;
	цяло sz;
	цяло res;

	sz = vasprintf(&val, format, ap);
	ако (sz < 0)
		върни -errno;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_buffer_append_data(buf, val, sz);
	free(val);

	върни res;
}

цяло ul_buffer_set_data(struct ul_buffer *buf, const char *data, size_t sz)
{
	ul_buffer_reset_data(buf);
	върни ul_buffer_append_data(buf, data, sz);
}

char *ul_buffer_get_data(struct ul_buffer *buf, size_t *sz, size_t *width)
{
	ако (sz)
		*sz = buf->end - buf->begin;
	ако (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	върни buf->begin;
}

char *ul_buffer_get_string(struct ul_buffer *buf, size_t *sz, size_t *width)
{
	char *ret;

	ret = ul_buffer_get_data(buf, sz, width);

	/* data in buffer is already zero-terminated */
	ако (sz)
		*sz = *sz + 1;

	върни ret;
}

/* size of allocated area (!= size of stored data */
size_t ul_buffer_get_bufsiz(struct ul_buffer *buf)
{
	върни buf->sz;
}

size_t ul_buffer_get_datasiz(struct ul_buffer *buf)
{
	върни buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
char *ul_buffer_get_safe_data(struct ul_buffer *buf, size_t *sz, size_t *width, const char *safechars)
{
	char *data = ul_buffer_get_data(buf, NULL, NULL);
	size_t encsz, wsz = 0;
	char *res = NULL;

	ако (!data)
		goto nothing;

	encsz = mbs_safe_encode_size(buf->sz) + 1;
	ако (encsz > buf->encoded_sz) {
		char *tmp = realloc(buf->encoded, encsz);
		ако (!tmp)
			goto nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_buffer(data, &wsz, buf->encoded, buf->encoded_sz, safechars);
	ако (!res || !wsz || wsz == (size_t) -1)
		goto nothing;

	ако (width)
		*width = wsz;
	ако (sz)
		*sz = strlen(res);
	върни res;
nothing:
	ако (width)
		*width = 0;
	ако (sz)
		*sz = 0;
	върни NULL;
}


#ifdef TEST_PROGRAM_BUFFER

enum {
	PTR_AAA  = 0,
	PTR_BBB,
};

цяло main(void)
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

	върни EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BUFFER */
