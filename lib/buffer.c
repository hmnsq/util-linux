/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#समावेश "buffer.h"
#समावेश "mbsalign.h"
#समावेश "strutils.h"

व्योम ul_बफर_reset_तथ्य(काष्ठा ul_बफर *buf)
{
	अगर (buf->begin)
		स_रखो(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	अगर (buf->ptrs && buf->nptrs)
		स_रखो(buf->ptrs, 0, buf->nptrs * माप(अक्षर *));
}

अटल अंतरभूत अक्षर *ul_बफर_free_तथ्य_common(काष्ठा ul_बफर *buf, bool steal)
{
	अक्षर *t = शून्य;

	निश्चित(buf);

	अगर (steal)
		t = buf->begin;
	अन्यथा
		मुक्त(buf->begin);
	buf->begin = शून्य;
	buf->end = शून्य;
	buf->sz = 0;

	मुक्त(buf->ptrs);
	buf->ptrs = शून्य;
	buf->nptrs = 0;

	मुक्त(buf->encoded);
	buf->encoded = शून्य;
	buf->encoded_sz = 0;

	वापस t;
}

व्योम ul_बफर_free_तथ्य(काष्ठा ul_बफर *buf)
{
	ul_बफर_free_तथ्य_common(buf, false);
}

अक्षर *ul_बफर_steal_string(काष्ठा ul_बफर *buf)
{
	अगर (ul_बफर_is_empty(buf)) {
		ul_बफर_free_तथ्य(buf);
		वापस शून्य;
	}

	वापस ul_बफर_free_तथ्य_common(buf, true);
}

व्योम ul_बफर_set_chunksize(काष्ठा ul_बफर *buf, माप_प्रकार sz)
{
	buf->chunksize = sz;
}

पूर्णांक ul_बफर_is_empty(काष्ठा ul_बफर *buf)
{
	वापस buf->begin == buf->end;
}

पूर्णांक ul_बफर_save_pointer(काष्ठा ul_बफर *buf, अचिन्हित लघु ptr_idx)
{
	अगर (ptr_idx >= buf->nptrs) {
		अक्षर **tmp = reallocarray(buf->ptrs, ptr_idx + 1, माप(अक्षर *));

		अगर (!tmp)
			वापस -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	वापस 0;
}


अक्षर *ul_बफर_get_pointer(काष्ठा ul_बफर *buf, अचिन्हित लघु ptr_idx)
{
	अगर (ptr_idx < buf->nptrs)
		वापस buf->ptrs[ptr_idx];
	वापस शून्य;
}

/* returns length from begin to the pointer */
माप_प्रकार ul_बफर_get_pointer_लंबाई(काष्ठा ul_बफर *buf, अचिन्हित लघु ptr_idx)
{
	अक्षर *ptr = ul_बफर_get_pointer(buf, ptr_idx);

	अगर (ptr && ptr > buf->begin)
		वापस ptr - buf->begin;
	वापस 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
माप_प्रकार ul_बफर_get_safe_pointer_width(काष्ठा ul_बफर *buf, अचिन्हित लघु ptr_idx)
{
	माप_प्रकार len = ul_बफर_get_pointer_लंबाई(buf, ptr_idx);

	अगर (!len)
		वापस 0;

	वापस mbs_safe_nwidth(buf->begin, len, शून्य);
}

व्योम ul_बफर_refer_string(काष्ठा ul_बफर *buf, अक्षर *str)
{
	अगर (buf->sz)
		ul_बफर_free_तथ्य(buf);
	buf->begin = str;
	buf->sz = str ? म_माप(str) : 0;
	buf->end = buf->begin ? buf->begin + buf->sz : buf->begin;
}

पूर्णांक ul_बफर_alloc_तथ्य(काष्ठा ul_बफर *buf, माप_प्रकार sz)
{
	अक्षर *tmp;
	माप_प्रकार len = 0;

	निश्चित(buf);

	अगर (sz <= buf->sz)
		वापस 0;

	अगर (buf->end && buf->begin)
		len = buf->end - buf->begin;

	अगर (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = पुनः_स्मृति(buf->begin, sz);
	अगर (!tmp)
		वापस -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	स_रखो(buf->end, '\0', sz - len);

	वापस 0;
}

पूर्णांक ul_बफर_append_तथ्य(काष्ठा ul_बफर *buf, स्थिर अक्षर *तथ्य, माप_प्रकार sz)
{
	माप_प्रकार maxsz = 0;

	अगर (!buf)
		वापस -EINVAL;
	अगर (!तथ्य)
		वापस 0;

	अगर (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	अगर (maxsz <= sz + 1) {
		पूर्णांक rc = ul_बफर_alloc_तथ्य(buf, buf->sz + sz + 1);
		अगर (rc)
			वापस rc;
	}
	अगर (!buf->end)
		वापस -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, तथ्य, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	वापस 0;
}

पूर्णांक ul_बफर_append_char(काष्ठा ul_बफर *buf, स्थिर अक्षर c)
{
	वापस ul_बफर_append_तथ्य(buf, &c, 1);
}

पूर्णांक ul_बफर_append_string(काष्ठा ul_बफर *buf, स्थिर अक्षर *str)
{
	अगर (!str)
		वापस 0;

	वापस ul_बफर_append_तथ्य(buf, str, म_माप(str));
}

पूर्णांक ul_बफर_append_ntimes(काष्ठा ul_बफर *buf, माप_प्रकार n, स्थिर अक्षर *str)
{
	माप_प्रकार i;
	माप_प्रकार len = म_माप(str);

	क्रम (i = 0; len && i < n; i++) {
		पूर्णांक rc = ul_बफर_append_तथ्य(buf, str, len);
		अगर (rc)
			वापस rc;
	}
	वापस 0;
}

पूर्णांक ul_बफर_appendf(काष्ठा ul_बफर *buf, स्थिर अक्षर *format, ...)
{
	बहु_सूची ap;
	पूर्णांक res;

	बहु_शुरू(ap, format);
	res = ul_बफर_appendvf(buf, format, ap);
	बहु_पूर्ण(ap);

	वापस res;
}

पूर्णांक ul_बफर_appendvf(काष्ठा ul_बफर *buf, स्थिर अक्षर *format, बहु_सूची ap)
{
	अक्षर *val;
	पूर्णांक sz;
	पूर्णांक res;

	sz = vasprintf(&val, format, ap);
	अगर (sz < 0)
		वापस -त्रुटि_सं;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_बफर_append_तथ्य(buf, val, sz);
	मुक्त(val);

	वापस res;
}

पूर्णांक ul_बफर_set_तथ्य(काष्ठा ul_बफर *buf, स्थिर अक्षर *तथ्य, माप_प्रकार sz)
{
	ul_बफर_reset_तथ्य(buf);
	वापस ul_बफर_append_तथ्य(buf, तथ्य, sz);
}

अक्षर *ul_बफर_get_तथ्य(काष्ठा ul_बफर *buf, माप_प्रकार *sz, माप_प्रकार *width)
{
	अगर (sz)
		*sz = buf->end - buf->begin;
	अगर (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	वापस buf->begin;
}

अक्षर *ul_बफर_get_string(काष्ठा ul_बफर *buf, माप_प्रकार *sz, माप_प्रकार *width)
{
	अक्षर *ret;

	ret = ul_बफर_get_तथ्य(buf, sz, width);

	/* data in buffer is already zero-terminated */
	अगर (sz)
		*sz = *sz + 1;

	वापस ret;
}

/* size of allocated area (!= size of stored data */
माप_प्रकार ul_बफर_get_bufsiz(काष्ठा ul_बफर *buf)
{
	वापस buf->sz;
}

माप_प्रकार ul_बफर_get_datasiz(काष्ठा ul_बफर *buf)
{
	वापस buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
अक्षर *ul_बफर_get_safe_तथ्य(काष्ठा ul_बफर *buf, माप_प्रकार *sz, माप_प्रकार *width, स्थिर अक्षर *safechars)
{
	अक्षर *तथ्य = ul_बफर_get_तथ्य(buf, शून्य, शून्य);
	माप_प्रकार encsz, wsz = 0;
	अक्षर *res = शून्य;

	अगर (!तथ्य)
		जाओ nothing;

	encsz = mbs_safe_encode_आकार(buf->sz) + 1;
	अगर (encsz > buf->encoded_sz) {
		अक्षर *tmp = पुनः_स्मृति(buf->encoded, encsz);
		अगर (!tmp)
			जाओ nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_बफर(तथ्य, &wsz, buf->encoded, buf->encoded_sz, safechars);
	अगर (!res || !wsz || wsz == (माप_प्रकार) -1)
		जाओ nothing;

	अगर (width)
		*width = wsz;
	अगर (sz)
		*sz = म_माप(res);
	वापस res;
nothing:
	अगर (width)
		*width = 0;
	अगर (sz)
		*sz = 0;
	वापस शून्य;
}


#अगर_घोषित TEST_PROGRAM_बफर

क्रमागत {
	PTR_AAA  = 0,
	PTR_BBB,
};

पूर्णांक मुख्य(व्योम)
{
	काष्ठा ul_बफर buf = UL_INIT_बफर;
	अक्षर *str;
	माप_प्रकार sz = 0;

	ul_बफर_set_chunksize(&buf, 16);

	ul_बफर_append_string(&buf, "AAA");
	ul_बफर_append_तथ्य(&buf, "=", 1);
	ul_बफर_append_string(&buf, "aaa");
	ul_बफर_save_pointer(&buf, PTR_AAA);

	ul_बफर_append_तथ्य(&buf, ",", 1);
	ul_बफर_append_string(&buf, "BBB");
	ul_बफर_append_string(&buf, "=");
	ul_बफर_append_string(&buf, "bbb");
	ul_बफर_save_pointer(&buf, PTR_BBB);

	str = ul_बफर_get_तथ्य(&buf, &sz, शून्य);
	म_लिखो("data [%zu] '%s'\n", sz, str);

	म_लिखो(" pointer data len: AAA=%zu, BBB=%zu\n",
			ul_बफर_get_pointer_लंबाई(&buf, PTR_AAA),
			ul_बफर_get_pointer_लंबाई(&buf, PTR_BBB));
	म_लिखो(" pointer data width: AAA=%zu, BBB=%zu\n",
			ul_बफर_get_safe_pointer_width(&buf, PTR_AAA),
			ul_बफर_get_safe_pointer_width(&buf, PTR_BBB));

	ul_बफर_reset_तथ्य(&buf);
	ul_बफर_append_string(&buf, "This is really long string to test the buffer function.");
	ul_बफर_save_pointer(&buf, PTR_AAA);
	ul_बफर_append_string(&buf, " YES!");
	str = ul_बफर_get_तथ्य(&buf, &sz, शून्य);
	म_लिखो("data [%zu] '%s'\n", sz, str);
	म_लिखो(" pointer data len: AAA=%zu\n", ul_बफर_get_pointer_लंबाई(&buf, PTR_AAA));

	ul_बफर_free_तथ्य(&buf);
	str = strdup("foo");
	ul_बफर_refer_string(&buf, str);
	ul_बफर_append_तथ्य(&buf, ",", 1);
	ul_बफर_append_string(&buf, "bar");
	str = ul_बफर_get_तथ्य(&buf, &sz, शून्य);
	म_लिखो("data [%zu] '%s'\n", sz, str);

	ul_बफर_free_तथ्य(&buf);
	ul_बफर_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_बफर_get_तथ्य(&buf, &sz, शून्य);
	म_लिखो("data [%zu] '%s'\n", sz, str);

	ul_बफर_free_तथ्य(&buf);
	ul_बफर_append_char(&buf, 'x');
	{
		अक्षर c = 'y';
		ul_बफर_append_char(&buf, c);
	}
	{
		स्थिर अक्षर c = 'z';
		ul_बफर_append_char(&buf, c);
	}
	str = ul_बफर_get_तथ्य(&buf, &sz, शून्य);
	म_लिखो("data [%zu] '%s'\n", sz, str);

	ul_बफर_free_तथ्य(&buf);

	वापस निकास_सफल;
}
#पूर्ण_अगर /* TEST_PROGRAM_BUFFER */
