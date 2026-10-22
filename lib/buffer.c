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
	यदि (buf->begin)
		स_रखो(buf->begin, 0, buf->sz);
	buf->end = buf->begin;

	यदि (buf->ptrs && buf->nptrs)
		स_रखो(buf->ptrs, 0, buf->nptrs * परिमाण(वर्ण *));
}

स्थिर अन्तःस्थ वर्ण *ul_buffer_free_data_common(संरचना ul_buffer *buf, bool steal)
{
	वर्ण *t = रिक्त;

	प्रतिज्ञा(buf);

	यदि (steal)
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

	फर्काऊ t;
}

शून्य ul_buffer_free_data(संरचना ul_buffer *buf)
{
	ul_buffer_free_data_common(buf, false);
}

वर्ण *ul_buffer_steal_string(संरचना ul_buffer *buf)
{
	यदि (ul_buffer_is_empty(buf)) {
		ul_buffer_free_data(buf);
		फर्काऊ रिक्त;
	}

	फर्काऊ ul_buffer_free_data_common(buf, true);
}

शून्य ul_buffer_set_chunksize(संरचना ul_buffer *buf, माप_प्रकार sz)
{
	buf->chunksize = sz;
}

पूर्णाङ्क ul_buffer_is_empty(संरचना ul_buffer *buf)
{
	फर्काऊ buf->begin == buf->end;
}

पूर्णाङ्क ul_buffer_save_pointer(संरचना ul_buffer *buf, अचिन्हित लघु ptr_idx)
{
	यदि (ptr_idx >= buf->nptrs) {
		वर्ण **tmp = reallocarray(buf->ptrs, ptr_idx + 1, परिमाण(वर्ण *));

		यदि (!tmp)
			फर्काऊ -EINVAL;
		buf->ptrs = tmp;
		buf->nptrs = ptr_idx + 1;
	}

	buf->ptrs[ptr_idx] = buf->end;
	फर्काऊ 0;
}


वर्ण *ul_buffer_get_pointer(संरचना ul_buffer *buf, अचिन्हित लघु ptr_idx)
{
	यदि (ptr_idx < buf->nptrs)
		फर्काऊ buf->ptrs[ptr_idx];
	फर्काऊ रिक्त;
}

/* returns length from begin to the pointer */
माप_प्रकार ul_buffer_get_pointer_length(संरचना ul_buffer *buf, अचिन्हित लघु ptr_idx)
{
	वर्ण *ptr = ul_buffer_get_pointer(buf, ptr_idx);

	यदि (ptr && ptr > buf->begin)
		फर्काऊ ptr - buf->begin;
	फर्काऊ 0;
}

/* returns width of data in safe encoding (from the begin to the pointer) */
माप_प्रकार ul_buffer_get_safe_pointer_width(संरचना ul_buffer *buf, अचिन्हित लघु ptr_idx)
{
	माप_प्रकार len = ul_buffer_get_pointer_length(buf, ptr_idx);

	यदि (!len)
		फर्काऊ 0;

	फर्काऊ mbs_safe_nwidth(buf->begin, len, रिक्त);
}

शून्य ul_buffer_refer_string(संरचना ul_buffer *buf, वर्ण *str)
{
	यदि (buf->sz)
		ul_buffer_free_data(buf);
	buf->begin = str;
	buf->sz = str ? म_माप(str) : 0;
	buf->end = buf->begin ? buf->begin + buf->sz : buf->begin;
}

पूर्णाङ्क ul_buffer_alloc_data(संरचना ul_buffer *buf, माप_प्रकार sz)
{
	वर्ण *tmp;
	माप_प्रकार len = 0;

	प्रतिज्ञा(buf);

	यदि (sz <= buf->sz)
		फर्काऊ 0;

	यदि (buf->end && buf->begin)
		len = buf->end - buf->begin;

	यदि (buf->chunksize)
		sz = ((sz + buf->chunksize) / buf->chunksize) * buf->chunksize + 1;

	tmp = पुनः_स्मृति(buf->begin, sz);
	यदि (!tmp)
		फर्काऊ -ENOMEM;

	buf->begin = tmp;
	buf->end = buf->begin + len;
	buf->sz = sz;

	स_रखो(buf->end, '\0', sz - len);

	फर्काऊ 0;
}

पूर्णाङ्क ul_buffer_append_data(संरचना ul_buffer *buf, अचर वर्ण *data, माप_प्रकार sz)
{
	माप_प्रकार maxsz = 0;

	यदि (!buf)
		फर्काऊ -EINVAL;
	यदि (!data)
		फर्काऊ 0;

	यदि (buf->begin && buf->end)
		maxsz = buf->sz - (buf->end - buf->begin);
	यदि (maxsz <= sz + 1) {
		पूर्णाङ्क rc = ul_buffer_alloc_data(buf, buf->sz + sz + 1);
		यदि (rc)
			फर्काऊ rc;
	}
	यदि (!buf->end)
		फर्काऊ -EINVAL;	/* make static analyzers happy */

	buf->end = mempcpy(buf->end, data, sz);
	*buf->end = '\0';	/* make sure it's terminated */
	फर्काऊ 0;
}

पूर्णाङ्क ul_buffer_append_char(संरचना ul_buffer *buf, अचर वर्ण c)
{
	फर्काऊ ul_buffer_append_data(buf, &c, 1);
}

पूर्णाङ्क ul_buffer_append_string(संरचना ul_buffer *buf, अचर वर्ण *str)
{
	यदि (!str)
		फर्काऊ 0;

	फर्काऊ ul_buffer_append_data(buf, str, म_माप(str));
}

पूर्णाङ्क ul_buffer_append_ntimes(संरचना ul_buffer *buf, माप_प्रकार n, अचर वर्ण *str)
{
	माप_प्रकार i;
	माप_प्रकार len = म_माप(str);

	लागि (i = 0; len && i < n; i++) {
		पूर्णाङ्क rc = ul_buffer_append_data(buf, str, len);
		यदि (rc)
			फर्काऊ rc;
	}
	फर्काऊ 0;
}

पूर्णाङ्क ul_buffer_appendf(संरचना ul_buffer *buf, अचर वर्ण *format, ...)
{
	बहु_सूची ap;
	पूर्णाङ्क res;

	बहु_शुरू(ap, format);
	res = ul_buffer_appendvf(buf, format, ap);
	बहु_पूर्ण(ap);

	फर्काऊ res;
}

पूर्णाङ्क ul_buffer_appendvf(संरचना ul_buffer *buf, अचर वर्ण *format, बहु_सूची ap)
{
	वर्ण *val;
	पूर्णाङ्क sz;
	पूर्णाङ्क res;

	sz = vasprintf(&val, format, ap);
	यदि (sz < 0)
		फर्काऊ -त्रुटि_सं;

	/* Like strlen(), sz doesn't include the last null byte. */
	res = ul_buffer_append_data(buf, val, sz);
	मुक्त(val);

	फर्काऊ res;
}

पूर्णाङ्क ul_buffer_set_data(संरचना ul_buffer *buf, अचर वर्ण *data, माप_प्रकार sz)
{
	ul_buffer_reset_data(buf);
	फर्काऊ ul_buffer_append_data(buf, data, sz);
}

वर्ण *ul_buffer_get_data(संरचना ul_buffer *buf, माप_प्रकार *sz, माप_प्रकार *width)
{
	यदि (sz)
		*sz = buf->end - buf->begin;
	यदि (width)
		*width = buf->begin && *buf->begin ? mbs_width(buf->begin) : 0;
	फर्काऊ buf->begin;
}

वर्ण *ul_buffer_get_string(संरचना ul_buffer *buf, माप_प्रकार *sz, माप_प्रकार *width)
{
	वर्ण *ret;

	ret = ul_buffer_get_data(buf, sz, width);

	/* data in buffer is already zero-terminated */
	यदि (sz)
		*sz = *sz + 1;

	फर्काऊ ret;
}

/* size of allocated area (!= size of stored data */
माप_प्रकार ul_buffer_get_bufsiz(संरचना ul_buffer *buf)
{
	फर्काऊ buf->sz;
}

माप_प्रकार ul_buffer_get_datasiz(संरचना ul_buffer *buf)
{
	फर्काऊ buf->end - buf->begin;
}

/* encode data by mbs_safe_encode() to avoid control and non-printable chars */
वर्ण *ul_buffer_get_safe_data(संरचना ul_buffer *buf, माप_प्रकार *sz, माप_प्रकार *width, अचर वर्ण *safechars)
{
	वर्ण *data = ul_buffer_get_data(buf, रिक्त, रिक्त);
	माप_प्रकार encsz, wsz = 0;
	वर्ण *res = रिक्त;

	यदि (!data)
		गच्छ nothing;

	encsz = mbs_safe_encode_size(buf->sz) + 1;
	यदि (encsz > buf->encoded_sz) {
		वर्ण *tmp = पुनः_स्मृति(buf->encoded, encsz);
		यदि (!tmp)
			गच्छ nothing;
		buf->encoded = tmp;
		buf->encoded_sz = encsz;
	}

	res = mbs_safe_encode_to_buffer(data, &wsz, buf->encoded, buf->encoded_sz, safechars);
	यदि (!res || !wsz || wsz == (माप_प्रकार) -1)
		गच्छ nothing;

	यदि (width)
		*width = wsz;
	यदि (sz)
		*sz = म_माप(res);
	फर्काऊ res;
nothing:
	यदि (width)
		*width = 0;
	यदि (sz)
		*sz = 0;
	फर्काऊ रिक्त;
}


#ifdef TEST_PROGRAM_BUFFER

गणना {
	PTR_AAA  = 0,
	PTR_BBB,
};

पूर्णाङ्क main(शून्य)
{
	संरचना ul_buffer buf = UL_INIT_BUFFER;
	वर्ण *str;
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
		वर्ण c = 'y';
		ul_buffer_append_char(&buf, c);
	}
	{
		अचर वर्ण c = 'z';
		ul_buffer_append_char(&buf, c);
	}
	str = ul_buffer_get_data(&buf, &sz, रिक्त);
	म_लिखो("data [%zu] '%s'\n", sz, str);

	ul_buffer_free_data(&buf);

	फर्काऊ निकास_सफल;
}
#endif /* TEST_PROGRAM_BUFFER */
