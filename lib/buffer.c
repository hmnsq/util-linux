/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#अन्तर्भाव "buffer.h"
#अन्तर्भाव "mbsalign.h"
#अन्तर्भाव "strutils.h"

शून्य ul_buffer_reset_data(संरचना ul_buffer *buf)
{
	जर (buf->begin)
		स_रखो(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	जर (buf->ptrs && buf->nptrs)
		स_रखो(buf->ptrs, 0, buf->nptrs * आकार(अक्षर *));
}

स्थिर ओळीत अक्षर *ul_buffer_free_data_common(संरचना ul_buffer *buf, bool steal)
{
	अक्षर *t = रिक्त;

	प्रतिज्ञा(buf);

	जर (steal)
		t = buf->begin;
	अन्यथा
		मुक्त(buf->begin);
	buf->begin = रिक्त;
	buf->end = रिक्त;
	buf->sz = 0;

	मुक्त(buf->ptrs);
	buf->ptrs = रिक्त;
	buf->nptrs = 0;

	मुक्त(buf->encoded);
	buf->encoded = रिक्त;
	buf->encoded_sz = 0;

	परत t;
}

शून्य ul_buffer_free_data(संरचना ul_buffer *buf)
{
	ul_buffer_free_data_common(buf, false);
}

अक्षर *ul_buffer_steal_string(संरचना ul_buffer *buf)
{
	जर (ul_buffer_is_empty(buf)) {
		ul_buffer_free_data(buf);
		परत रिक्त;
	}

	परत ul_buffer_free_data_common(buf, true);
}

शून्य ul_buffer_set_chunksize(संरचना ul_buffer *buf, माप_प्रकार sz)
{
	buf->chunksize = sz;
}

पूर्णांक ul_buffer_is_empty(संरचना ul_buffer *buf)
{
	परत buf->begin == buf->end;
}

पूर्णांक ul_buffer_save_pointer(संरचना ul_buffer *buf, अचिन्हित लघु ptr_idx)
{
	जर (ptr_idx >= buf->nptrs) {
		अक्षर **tmp = reallocarray(buf->ptrs, ptr_idx + 1, आकार(अक्षर *));

		जर (!tmp)
			परत -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	परत 0;
}


अक्षर *ul_buffer_get_pointer(संरचना ul_buffer *buf, अचिन्हित लघु ptr_idx)
{
	जर (ptr_idx < buf->nptrs)
		परत buf->ptrs[ptr_idx];
	परत रिक्त;
}

/* returns length from begin to the pointer */
माप_प्रकार ul_buffer_get_pointer_length(संरचना ul_buffer *buf, अचिन्हित लघु ptr_idx)
{
	अक्षर *ptr = ul_buffer_get_pointer(buf, ptr_idx);

	जर (ptr && ptr > buf->begin)
		परत ptr - buf->begin;
	परत 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
माप_प्रकार ul_buffer_get_safe_pointer_width(संरचना ul_buffer *buf, अचिन्हित लघु ptr_idx)
{
	माप_प्रकार len = ul_buffer_get_pointer_length(buf, ptr_idx);

	जर (!len)
		परत 0;

	परत mbs_safe_nwidth(buf->begin, len, रिक्त);
}

शून्य ul_buffer_refer_string(संरचना ul_buffer *buf, अक्षर *str)
{
	जर (buf->sz)
		ul_buffer_free_data(buf);
	buf->begin = str;
	buf->sz = str ? म_माप(str) : 0;
	buf->end = buf->begin ? buf->begin + buf->sz : buf->begin;
}

पूर्णांक ul_buffer_alloc_data(संरचना ul_buffer *buf, माप_प्रकार sz)
{
	अक्षर *tmp;
	माप_प्रकार len = 0;

	प्रतिज्ञा(buf);

	जर (sz <= buf->sz)
		परत 0;

	जर (buf->end && buf->begin)
		len = buf->end - buf->begin;

	जर (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = पुनः_स्मृति(buf->begin, sz);
	जर (!tmp)
		परत -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	स_रखो(buf->end, '\0', sz - len);

	परत 0;
}

पूर्णांक ul_buffer_append_data(संरचना ul_buffer *buf, स्थिरांक अक्षर *data, माप_प्रकार sz)
{
	माप_प्रकार maxsz = 0;

	जर (!buf)
		परत -EINVAL;
	जर (!data)
		परत 0;

	जर (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	जर (maxsz <= sz + 1) {
		पूर्णांक rc = ul_buffer_alloc_data(buf, buf->sz + sz + 1);
		जर (rc)
			परत rc;
	}
	जर (!buf->end)
		परत -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, data, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	परत 0;
}

पूर्णांक ul_buffer_append_char(संरचना ul_buffer *buf, स्थिरांक अक्षर c)
{
	परत ul_buffer_append_data(buf, &c, 1);
}

पूर्णांक ul_buffer_append_string(संरचना ul_buffer *buf, स्थिरांक अक्षर *str)
{
	जर (!str)
		परत 0;

	परत ul_buffer_append_data(buf, str, म_माप(str));
}

पूर्णांक ul_buffer_append_ntimes(संरचना ul_buffer *buf, माप_प्रकार n, स्थिरांक अक्षर *str)
{
	माप_प्रकार i;
	माप_प्रकार len = म_माप(str);

	साठी (i = 0; len && i < n; i++) {
		पूर्णांक rc = ul_buffer_append_data(buf, str, len);
		जर (rc)
			परत rc;
	}
	परत 0;
}

पूर्णांक ul_buffer_appendf(संरचना ul_buffer *buf, स्थिरांक अक्षर *format, ...)
{
	बहु_सूची ap;
	पूर्णांक res;

	बहु_शुरू(ap, format);
	res = ul_buffer_appendvf(buf, format, ap);
	बहु_पूर्ण(ap);

	परत res;
}

पूर्णांक ul_buffer_appendvf(संरचना ul_buffer *buf, स्थिरांक अक्षर *format, बहु_सूची ap)
{
	अक्षर *val;
	पूर्णांक sz;
	पूर्णांक res;

	sz = vasprintf(&val, format, ap);
	जर (sz < 0)
		परत -त्रुटि_सं;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_buffer_append_data(buf, val, sz);
	मुक्त(val);

	परत res;
}

पूर्णांक ul_buffer_set_data(संरचना ul_buffer *buf, स्थिरांक अक्षर *data, माप_प्रकार sz)
{
	ul_buffer_reset_data(buf);
	परत ul_buffer_append_data(buf, data, sz);
}

अक्षर *ul_buffer_get_data(संरचना ul_buffer *buf, माप_प्रकार *sz, माप_प्रकार *width)
{
	जर (sz)
		*sz = buf->end - buf->begin;
	जर (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	परत buf->begin;
}

अक्षर *ul_buffer_get_string(संरचना ul_buffer *buf, माप_प्रकार *sz, माप_प्रकार *width)
{
	अक्षर *ret;

	ret = ul_buffer_get_data(buf, sz, width);

	/* data in buffer is already zero-terminated */
	जर (sz)
		*sz = *sz + 1;

	परत ret;
}

/* size of allocated area (!= size of stored data */
माप_प्रकार ul_buffer_get_bufsiz(संरचना ul_buffer *buf)
{
	परत buf->sz;
}

माप_प्रकार ul_buffer_get_datasiz(संरचना ul_buffer *buf)
{
	परत buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
अक्षर *ul_buffer_get_safe_data(संरचना ul_buffer *buf, माप_प्रकार *sz, माप_प्रकार *width, स्थिरांक अक्षर *safechars)
{
	अक्षर *data = ul_buffer_get_data(buf, रिक्त, रिक्त);
	माप_प्रकार encsz, wsz = 0;
	अक्षर *res = रिक्त;

	जर (!data)
		जा nothing;

	encsz = mbs_safe_encode_size(buf->sz) + 1;
	जर (encsz > buf->encoded_sz) {
		अक्षर *tmp = पुनः_स्मृति(buf->encoded, encsz);
		जर (!tmp)
			जा nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_buffer(data, &wsz, buf->encoded, buf->encoded_sz, safechars);
	जर (!res || !wsz || wsz == (माप_प्रकार) -1)
		जा nothing;

	जर (width)
		*width = wsz;
	जर (sz)
		*sz = म_माप(res);
	परत res;
nothing:
	जर (width)
		*width = 0;
	जर (sz)
		*sz = 0;
	परत रिक्त;
}


#ifdef TEST_PROGRAM_BUFFER

गणना {
	PTR_AAA  = 0,
	PTR_BBB,
};

पूर्णांक main(शून्य)
{
	संरचना ul_buffer buf = UL_INIT_BUFFER;
	अक्षर *str;
	माप_प्रकार sz = 0;

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

	str = ul_buffer_get_data(&buf, &sz, रिक्त);
	म_लिखो("data [%zu] '%s'\n", sz, str);

	म_लिखो(" pointer data len: AAA=%zu, BBB=%zu\n",
			ul_buffer_get_pointer_length(&buf, PTR_AAA),
			ul_buffer_get_pointer_length(&buf, PTR_BBB));
	म_लिखो(" pointer data width: AAA=%zu, BBB=%zu\n",
			ul_buffer_get_safe_pointer_width(&buf, PTR_AAA),
			ul_buffer_get_safe_pointer_width(&buf, PTR_BBB));

	ul_buffer_reset_data(&buf);
	ul_buffer_append_string(&buf, "This is really long string to test the buffer function.");
	ul_buffer_save_pointer(&buf, PTR_AAA);
	ul_buffer_append_string(&buf, " YES!");
	str = ul_buffer_get_data(&buf, &sz, रिक्त);
	म_लिखो("data [%zu] '%s'\n", sz, str);
	म_लिखो(" pointer data len: AAA=%zu\n", ul_buffer_get_pointer_length(&buf, PTR_AAA));

	ul_buffer_free_data(&buf);
	str = strdup("foo");
	ul_buffer_refer_string(&buf, str);
	ul_buffer_append_data(&buf, ",", 1);
	ul_buffer_append_string(&buf, "bar");
	str = ul_buffer_get_data(&buf, &sz, रिक्त);
	म_लिखो("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_appendf(&buf, "a%se%d", "bcd", 10);
	str = ul_buffer_get_data(&buf, &sz, रिक्त);
	म_लिखो("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);
	ul_buffer_append_char(&buf, 'x');
	{
		अक्षर c = 'y';
		ul_buffer_append_char(&buf, c);
	}
	{
		स्थिरांक अक्षर c = 'z';
		ul_buffer_append_char(&buf, c);
	}
	str = ul_buffer_get_data(&buf, &sz, रिक्त);
	म_लिखो("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);

	परत निकास_सफल;
}
#endif /* TEST_PROGRAM_BUFFER */
