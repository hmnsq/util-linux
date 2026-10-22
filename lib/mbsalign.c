/*
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Align/Truncate a string in a given screen width
 * Copyright (C) 2009-2010 Free Software Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Written by Pádraig Brady.
 */
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <ctype.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "mbsalign.h"
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "widechar.h"

/* Return the byte after a complete ANSI escape sequence, or NULL. */
स्थिर स्थिरांक अक्षर *skip_ansi_escape(स्थिरांक अक्षर *p, स्थिरांक अक्षर *end)
{
	स्थिरांक अक्षर *q;

	जर (p >= end || (अचिन्हित अक्षर) *p != '\033' || p + 1 >= end)
		परत रिक्त;

	q = p + 1;
	जर (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		पर्यंत (q < end && (अचिन्हित अक्षर) *q >= 0x30
				&& (अचिन्हित अक्षर) *q <= 0x3f)
			q++;
		पर्यंत (q < end && (अचिन्हित अक्षर) *q >= 0x20
				&& (अचिन्हित अक्षर) *q <= 0x2f)
			q++;
		जर (q < end && (अचिन्हित अक्षर) *q >= 0x40
				&& (अचिन्हित अक्षर) *q <= 0x7e)
			परत q + 1;
	} अन्यथा जर (*q && म_अक्षर("()*+", *q) && q + 1 < end)
		परत q + 2;

	परत रिक्त;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
माप_प्रकार mbs_nwidth(स्थिरांक अक्षर *buf, माप_प्रकार bufsz)
{
	स्थिरांक अक्षर *p = buf, *last = buf;
	माप_प्रकार width = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	स_रखो(&st, 0, आकार(st));
#endif
	जर (p && *p && bufsz)
		last = p + (bufsz - 1);

	पर्यंत (p && *p && p <= last) {
		जर (*p == '\033') {
			स्थिरांक अक्षर *e = skip_ansi_escape(p, last + 1);

			p = e ? e : p + 1;
			सुरूठेवा;
		}
		जर (है_नियंत्रण((अचिन्हित अक्षर) *p)) {
			p++;
			सुरूठेवा;
		}
#ifdef HAVE_WIDECHAR
		ब_अक्षर_प्रकार wc;
		माप_प्रकार len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		जर (len == 0)
			थांबा;
		जर (len > 0 && iswprint(wc)) {
			पूर्णांक x = wcwidth(wc);
			जर (x > 0)
				width += x;
		} अन्यथा जर (len == (माप_प्रकार) -1 || len == (माप_प्रकार) -2)
			len = 1;
		p += len;
#अन्यथा
		जर (है_छाप((अचिन्हित अक्षर) *p))
			width++;
		p++;
#endif
	}

	परत width;
}

माप_प्रकार mbs_width(स्थिरांक अक्षर *s)
{
	जर (!s || !*s)
		परत 0;
	परत mbs_nwidth(s, म_माप(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
माप_प्रकार mbs_safe_nwidth(स्थिरांक अक्षर *buf, माप_प्रकार bufsz, माप_प्रकार *sz)
{
	स्थिरांक अक्षर *p = buf, *last = buf;
	माप_प्रकार width = 0, bytes = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	स_रखो(&st, 0, आकार(st));
#endif
	जर (p && *p && bufsz)
		last = p + (bufsz - 1);

	पर्यंत (p && *p && p <= last) {
		जर ((p < last && *p == '\\' && *(p + 1) == 'x')
		    || है_नियंत्रण((अचिन्हित अक्षर) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		}
#ifdef HAVE_WIDECHAR
		अन्यथा {
			ब_अक्षर_प्रकार wc;
			माप_प्रकार len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			जर (len == 0)
				थांबा;

			जर (len == (माप_प्रकार) -1 || len == (माप_प्रकार) -2) {
				len = 1;
				जर (है_छाप((अचिन्हित अक्षर) *p))
					width += 1, bytes += 1;
				अन्यथा
					width += 4, bytes += 4;

			} अन्यथा जर (!iswprint(wc)) {
				width += len * 4;	/* hex encode whole sequence */
				bytes += len * 4;
			} अन्यथा {
				width += wcwidth(wc);	/* number of cells */
				bytes += len;		/* number of bytes */
			}
			p += len;
		}
#अन्यथा
		अन्यथा जर (!है_छाप((अचिन्हित अक्षर) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		} अन्यथा {
			width++, bytes++;
			p++;
		}
#endif
	}

	जर (sz)
		*sz = bytes;
	परत width;
}

माप_प्रकार mbs_safe_width(स्थिरांक अक्षर *s)
{
	जर (!s || !*s)
		परत 0;
	परत mbs_safe_nwidth(s, म_माप(s), रिक्त);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
अक्षर *mbs_safe_encode_to_buffer(स्थिरांक अक्षर *s, माप_प्रकार *width, अक्षर *buf, माप_प्रकार bufsiz, स्थिरांक अक्षर *safechars)
{
	स्थिरांक अक्षर *p = s;
	अक्षर *r;
	पूर्णांक rsz;
	माप_प्रकार sz = s ? म_माप(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	स_रखो(&st, 0, आकार(st));
#endif
	जर (!sz || !buf || !bufsiz)
		परत रिक्त;

	r = buf;
	rsz = (पूर्णांक) bufsiz;
	*width = 0;

	पर्यंत (p && *p) {
		जर (safechars && म_अक्षर(safechars, *p)) {
			जर (rsz < 2)
				थांबा;
			*r++ = *p++;
			rsz--;
			सुरूठेवा;
		}

		जर ((*p == '\\' && *(p + 1) == 'x')
		    || है_नियंत्रण((अचिन्हित अक्षर) *p)) {
			जर (rsz < 5)
				थांबा;
			snprintf(r, rsz, "\\x%02x", (अचिन्हित अक्षर) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
			p++;
		}
#ifdef HAVE_WIDECHAR
		अन्यथा {
			ब_अक्षर_प्रकार wc;
			माप_प्रकार len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			जर (len == 0)
				थांबा;		/* end of string */

			जर (len == (माप_प्रकार) -1 || len == (माप_प्रकार) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				जर (!है_छाप((अचिन्हित अक्षर) *p)) {
					जर (rsz < 5)
						थांबा;
					snprintf(r, rsz, "\\x%02x", (अचिन्हित अक्षर) *p);
					r += 4;
					rsz -= 4;
					*width += 4;
				} अन्यथा {
					जर (rsz < 2)
						थांबा;
					(*width)++;
					*r++ = *p;
					rsz--;
				}
			} अन्यथा जर (!iswprint(wc)) {
				माप_प्रकार i;
				साठी (i = 0; i < len; i++) {
					जर (rsz < 5)
						थांबा;
					snprintf(r, rsz, "\\x%02x", (अचिन्हित अक्षर) p[i]);
					r += 4;
					rsz -= 4;
					*width += 4;
				}
				जर (i < len)
					थांबा;
			} अन्यथा {
				जर (rsz < (पूर्णांक)len + 1)
					थांबा;
				स_नकल(r, p, len);
				r += len;
				rsz -= len;
				*width += wcwidth(wc);
			}
			p += len;
		}
#अन्यथा
		अन्यथा जर (!है_छाप((अचिन्हित अक्षर) *p)) {
			जर (rsz < 5)
				थांबा;
			snprintf(r, rsz, "\\x%02x", (अचिन्हित अक्षर) *p);
			p++;
			r += 4;
			rsz -= 4;
			*width += 4;
		} अन्यथा {
			जर (rsz < 2)
				थांबा;
			*r++ = *p++;
			rsz--;
			(*width)++;
		}
#endif
	}

	*r = '\0';
	परत buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
अक्षर *mbs_invalid_encode_to_buffer(स्थिरांक अक्षर *s, माप_प्रकार *width, अक्षर *buf, माप_प्रकार bufsiz)
{
	स्थिरांक अक्षर *p = s;
	अक्षर *r;
	पूर्णांक rsz;
	माप_प्रकार sz = s ? म_माप(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	स_रखो(&st, 0, आकार(st));
#endif
	जर (!sz || !buf || !bufsiz)
		परत रिक्त;

	r = buf;
	rsz = (पूर्णांक) bufsiz;
	*width = 0;

	पर्यंत (p && *p) {
#ifdef HAVE_WIDECHAR
		ब_अक्षर_प्रकार wc;
		माप_प्रकार len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#अन्यथा
		माप_प्रकार len = 1;
#endif

		जर (len == 0)
			थांबा;		/* end of string */

		जर (len == (माप_प्रकार) -1 || len == (माप_प्रकार) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			जर (!है_छाप((अचिन्हित अक्षर) *p)) {
				जर (rsz < 5)
					थांबा;
				snprintf(r, rsz, "\\x%02x", (अचिन्हित अक्षर) *p);
				r += 4;
				rsz -= 4;
				*width += 4;
			} अन्यथा {
				जर (rsz < 2)
					थांबा;
				(*width)++;
				*r++ = *p;
				rsz--;
			}
		} अन्यथा जर (*p == '\\' && *(p + 1) == 'x') {
			जर (rsz < 5)
				थांबा;
			snprintf(r, rsz, "\\x%02x", (अचिन्हित अक्षर) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
		} अन्यथा {
			जर (rsz < (पूर्णांक)len + 1)
				थांबा;
			r = mempcpy(r, p, len);
			rsz -= len;
			*width += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	परत buf;
}

/*
 * Guess size
 */
माप_प्रकार mbs_safe_encode_size(माप_प्रकार bytes)
{
	परत (bytes * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
माप_प्रकार mbs_safe_decode_size(स्थिरांक अक्षर *p)
{
	माप_प्रकार bytes = 0;

	पर्यंत (p && *p) {
		जर (*p == '\\' && *(p + 1) == 'x' &&
		    है_षष्ठादशक(*(p + 2)) && है_षष्ठादशक(*(p + 3)))
			p += 4;
		अन्यथा
			p++;
		bytes++;
	}
	परत bytes;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
अक्षर *mbs_safe_encode(स्थिरांक अक्षर *s, माप_प्रकार *width)
{
	माप_प्रकार sz = s ? म_माप(s) : 0;
	माप_प्रकार bufsz;
	अक्षर *buf, *ret = रिक्त;

	जर (!sz)
		परत रिक्त;
	bufsz = mbs_safe_encode_size(sz);
	buf = दो_स्मृति(bufsz);
	जर (buf)
		ret = mbs_safe_encode_to_buffer(s, width, buf, bufsz, रिक्त);
	जर (!ret)
		मुक्त(buf);
	परत ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
अक्षर *mbs_invalid_encode(स्थिरांक अक्षर *s, माप_प्रकार *width)
{
	माप_प्रकार sz = s ? म_माप(s) : 0;
	माप_प्रकार bufsz;
	अक्षर *buf, *ret = रिक्त;

	जर (!sz)
		परत रिक्त;
	bufsz = mbs_safe_encode_size(sz);
	buf = दो_स्मृति(bufsz);
	जर (buf)
		ret = mbs_invalid_encode_to_buffer(s, width, buf, bufsz);
	जर (!ret)
		मुक्त(buf);
	परत ret;
}

#ifdef HAVE_WIDECHAR

स्थिर bool
wc_ensure_printable (ब_अक्षर_प्रकार *wchars)
{
  bool replaced = false;
  ब_अक्षर_प्रकार *wc = wchars;
  पर्यंत (*wc)
    {
      जर (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  परत replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
स्थिर ब_अक्षर_प्रकार *
wc_skip_ansi_escape (ब_अक्षर_प्रकार *wc)
{
  ब_अक्षर_प्रकार *p = wc + 1;

  जर (*wc != L'\033' || !*p)
    परत रिक्त;

  जर (*p == L'[')
    {
      p++;
      पर्यंत (*p >= 0x30 && *p <= 0x3f)
        p++;
      पर्यंत (*p >= 0x20 && *p <= 0x2f)
        p++;
      जर (*p >= 0x40 && *p <= 0x7e)
        परत p + 1;
    }
  अन्यथा जर ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    परत p + 2;

  परत रिक्त;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

स्थिर माप_प्रकार
wc_truncate (ब_अक्षर_प्रकार *wc, माप_प्रकार width)
{
  माप_प्रकार cells = 0;
  पूर्णांक next_cells = 0;

  पर्यंत (*wc)
    {
      ब_अक्षर_प्रकार *after_escape = wc_skip_ansi_escape (wc);
      जर (after_escape)
        {
          wc = after_escape;
          सुरूठेवा;
        }

      next_cells = wcwidth (*wc);
      जर (next_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          next_cells = 1;
        }
      जर (cells + next_cells > width)
        थांबा;

      cells += next_cells;
      wc++;
    }
  *wc = L'\0';
  परत cells;
}

स्थिर पूर्णांक
rpl_wcswidth (स्थिरांक ब_अक्षर_प्रकार *s, माप_प्रकार n)
{
  पूर्णांक ret = 0;

  पर्यंत (n-- > 0 && *s != L'\0')
    {
      पूर्णांक nwidth = wcwidth (*s++);
      जर (nwidth == -1)             /* non printable */
        परत -1;
      जर (ret > (पूर्णांक_उच्च - nwidth)) /* overflow */
        परत -1;
      ret += nwidth;
    }

  परत ret;
}
#endif /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
माप_प्रकार
mbs_truncate(अक्षर *str, माप_प्रकार *width)
{
	ssize_t bytes = म_माप(str);
#ifdef HAVE_WIDECHAR
	ssize_t sz = mbstowcs(रिक्त, str, 0);
	ब_अक्षर_प्रकार *wcs = रिक्त;

	जर (sz == (ssize_t) -1)
		जा done;

	wcs = सुस्मृति(1, (sz + 1) * आकार(ब_अक्षर_प्रकार));
	जर (!wcs)
		जा done;

	जर (!mbstowcs(wcs, str, sz))
		जा done;
	*width = wc_truncate(wcs, *width);
	bytes = wcstombs(str, wcs, bytes);
done:
	मुक्त(wcs);
#अन्यथा
	{
		स्थिरांक अक्षर *p = str;
		स्थिरांक अक्षर *end = str + bytes;
		माप_प्रकार cells = 0;

		पर्यंत (p < end) {
			स्थिरांक अक्षर *after_escape = skip_ansi_escape(p, end);

			जर (after_escape) {
				p = after_escape;
				सुरूठेवा;
			}
			जर (cells >= *width)
				थांबा;
			p++;
			cells++;
		}
		bytes = p - str;
		*width = cells;
	}
#endif
	जर (bytes >= 0)
		str[bytes] = '\0';
	परत bytes;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

स्थिर अक्षर*
mbs_align_pad (अक्षर *dest, स्थिरांक अक्षर* dest_end, माप_प्रकार n_spaces, पूर्णांक padchar)
{
  साठी (/* nothing */; n_spaces && (dest < dest_end); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  परत dest;
}

माप_प्रकार
mbsalign (स्थिरांक अक्षर *src, अक्षर *dest, माप_प्रकार dest_size,
          माप_प्रकार *width, mbs_align_t align, पूर्णांक flags)
{
	परत mbsalign_with_padding(src, dest, dest_size, width, align, flags, ' ');
}

/* Align a string, SRC, in a field of *WIDTH columns, handling multi-byte
   characters; write the result into the DEST_SIZE-byte buffer, DEST.
   ALIGNMENT specifies whether to left- or right-justify or to center.
   If SRC requires more than *WIDTH columns, truncate it to fit.
   When centering, the number of trailing spaces may be one less than the
   number of leading spaces. The FLAGS parameter is unused at present.
   Return the length in bytes required for the final result, not counting
   the trailing NUL.  A return value of DEST_SIZE or larger means there
   wasn't enough space.  DEST will be NUL terminated in any case.
   Return (size_t) -1 upon error (invalid multi-byte sequence in SRC,
   or malloc failure), unless MBA_UNIBYTE_FALLBACK is specified.
   Update *WIDTH to indicate how many columns were used before padding.  */

माप_प्रकार
mbsalign_with_padding (स्थिरांक अक्षर *src, अक्षर *dest, माप_प्रकार dest_size,
	               माप_प्रकार *width, mbs_align_t align,
#ifdef HAVE_WIDECHAR
		       पूर्णांक flags,
#अन्यथा
		       पूर्णांक flags __attribute__((__unused__)),
#endif
		       पूर्णांक padchar)
{
  माप_प्रकार ret = -1;
  माप_प्रकार src_size = म_माप (src) + 1;
  अक्षर *newstr = रिक्त;
  ब_अक्षर_प्रकार *str_wc = रिक्त;
  स्थिरांक अक्षर *str_to_print = src;
  माप_प्रकार n_cols = src_size - 1;
  माप_प्रकार n_used_bytes = n_cols; /* Not including NUL */
  माप_प्रकार n_spaces = 0, space_डावा;

#ifdef HAVE_WIDECHAR
  bool conversion = false;
  bool wc_enabled = false;

  /* In multi-byte locales convert to wide characters
     to allow easy truncation. Also determine number
     of screen columns used.  */
  जर (MB_CUR_MAX > 1)
    {
      माप_प्रकार src_chars = mbstowcs (रिक्त, src, 0);
      जर (src_chars == (माप_प्रकार) -1)
        {
          जर (flags & MBA_UNIBYTE_FALLBACK)
            जा mbsalign_unibyte;
          अन्यथा
            जा mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = दो_स्मृति (src_chars * आकार (ब_अक्षर_प्रकार));
      जर (str_wc == रिक्त)
        {
          जर (flags & MBA_UNIBYTE_FALLBACK)
            जा mbsalign_unibyte;
          अन्यथा
            जा mbsalign_cleanup;
        }
      जर (mbstowcs (str_wc, src, src_chars) != 0)
        {
          str_wc[src_chars - 1] = L'\0';
          wc_enabled = true;
          conversion = wc_ensure_printable (str_wc);
          n_cols = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  जर (wc_enabled && (conversion || (n_cols > *width)))
    {
        जर (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_size = wcstombs(रिक्त, str_wc, 0) + 1;
          }
        newstr = दो_स्मृति (src_size);
        जर (newstr == रिक्त)
        {
          जर (flags & MBA_UNIBYTE_FALLBACK)
            जा mbsalign_unibyte;
          अन्यथा
            जा mbsalign_cleanup;
        }
        str_to_print = newstr;
        n_cols = wc_truncate (str_wc, *width);
        n_used_bytes = wcstombs (newstr, str_wc, src_size);
    }

mbsalign_unibyte:
#endif

  जर (n_cols > *width) /* Unibyte truncation required.  */
    {
      n_cols = *width;
      n_used_bytes = n_cols;
    }

  जर (*width > n_cols) /* Padding required.  */
    n_spaces = *width - n_cols;

  /* indicate to caller how many cells needed (not including padding).  */
  *width = n_cols;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_used_bytes + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  जर (dest_size != 0)
    {
      अक्षर *dest_end = dest + dest_size - 1;
      माप_प्रकार start_spaces;
      माप_प्रकार end_spaces;

      निवड (align)
        {
        स्थिती MBS_ALIGN_CENTER:
          start_spaces = n_spaces / 2 + n_spaces % 2;
          end_spaces = n_spaces / 2;
          थांबा;
        स्थिती MBS_ALIGN_डावा:
          start_spaces = 0;
          end_spaces = n_spaces;
          थांबा;
        स्थिती MBS_ALIGN_उजवा:
          start_spaces = n_spaces;
          end_spaces = 0;
          थांबा;
	पूर्वनिर्धारित:
	  पात();
        }

      dest = mbs_align_pad (dest, dest_end, start_spaces, padchar);
      space_डावा = dest_end - dest;
      dest = mempcpy (dest, str_to_print, min (n_used_bytes, space_डावा));
      mbs_align_pad (dest, dest_end, end_spaces, padchar);
    }
#ifdef HAVE_WIDECHAR
mbsalign_cleanup:
#endif
  मुक्त (str_wc);
  मुक्त (newstr);

  परत ret;
}
