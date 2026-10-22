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
स्थिर अचर वर्ण *skip_ansi_escape(अचर वर्ण *p, अचर वर्ण *end)
{
	अचर वर्ण *q;

	यदि (p >= end || (अचिन्हित वर्ण) *p != '\033' || p + 1 >= end)
		फर्काऊ रिक्त;

	q = p + 1;
	यदि (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		जबसम्म (q < end && (अचिन्हित वर्ण) *q >= 0x30
				&& (अचिन्हित वर्ण) *q <= 0x3f)
			q++;
		जबसम्म (q < end && (अचिन्हित वर्ण) *q >= 0x20
				&& (अचिन्हित वर्ण) *q <= 0x2f)
			q++;
		यदि (q < end && (अचिन्हित वर्ण) *q >= 0x40
				&& (अचिन्हित वर्ण) *q <= 0x7e)
			फर्काऊ q + 1;
	} अन्यथा यदि (*q && म_अक्षर("()*+", *q) && q + 1 < end)
		फर्काऊ q + 2;

	फर्काऊ रिक्त;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
माप_प्रकार mbs_nwidth(अचर वर्ण *buf, माप_प्रकार bufsz)
{
	अचर वर्ण *p = buf, *last = buf;
	माप_प्रकार width = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	स_रखो(&st, 0, परिमाण(st));
#endif
	यदि (p && *p && bufsz)
		last = p + (bufsz - 1);

	जबसम्म (p && *p && p <= last) {
		यदि (*p == '\033') {
			अचर वर्ण *e = skip_ansi_escape(p, last + 1);

			p = e ? e : p + 1;
			जारी;
		}
		यदि (है_नियंत्रण((अचिन्हित वर्ण) *p)) {
			p++;
			जारी;
		}
#ifdef HAVE_WIDECHAR
		ब_अक्षर_प्रकार wc;
		माप_प्रकार len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		यदि (len == 0)
			रोक;
		यदि (len > 0 && iswprint(wc)) {
			पूर्णाङ्क x = wcwidth(wc);
			यदि (x > 0)
				width += x;
		} अन्यथा यदि (len == (माप_प्रकार) -1 || len == (माप_प्रकार) -2)
			len = 1;
		p += len;
#अन्यथा
		यदि (है_छाप((अचिन्हित वर्ण) *p))
			width++;
		p++;
#endif
	}

	फर्काऊ width;
}

माप_प्रकार mbs_width(अचर वर्ण *s)
{
	यदि (!s || !*s)
		फर्काऊ 0;
	फर्काऊ mbs_nwidth(s, म_माप(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
माप_प्रकार mbs_safe_nwidth(अचर वर्ण *buf, माप_प्रकार bufsz, माप_प्रकार *sz)
{
	अचर वर्ण *p = buf, *last = buf;
	माप_प्रकार width = 0, bytes = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	स_रखो(&st, 0, परिमाण(st));
#endif
	यदि (p && *p && bufsz)
		last = p + (bufsz - 1);

	जबसम्म (p && *p && p <= last) {
		यदि ((p < last && *p == '\\' && *(p + 1) == 'x')
		    || है_नियंत्रण((अचिन्हित वर्ण) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		}
#ifdef HAVE_WIDECHAR
		अन्यथा {
			ब_अक्षर_प्रकार wc;
			माप_प्रकार len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			यदि (len == 0)
				रोक;

			यदि (len == (माप_प्रकार) -1 || len == (माप_प्रकार) -2) {
				len = 1;
				यदि (है_छाप((अचिन्हित वर्ण) *p))
					width += 1, bytes += 1;
				अन्यथा
					width += 4, bytes += 4;

			} अन्यथा यदि (!iswprint(wc)) {
				width += len * 4;	/* hex encode whole sequence */
				bytes += len * 4;
			} अन्यथा {
				width += wcwidth(wc);	/* number of cells */
				bytes += len;		/* number of bytes */
			}
			p += len;
		}
#अन्यथा
		अन्यथा यदि (!है_छाप((अचिन्हित वर्ण) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		} अन्यथा {
			width++, bytes++;
			p++;
		}
#endif
	}

	यदि (sz)
		*sz = bytes;
	फर्काऊ width;
}

माप_प्रकार mbs_safe_width(अचर वर्ण *s)
{
	यदि (!s || !*s)
		फर्काऊ 0;
	फर्काऊ mbs_safe_nwidth(s, म_माप(s), रिक्त);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
वर्ण *mbs_safe_encode_to_buffer(अचर वर्ण *s, माप_प्रकार *width, वर्ण *buf, माप_प्रकार bufsiz, अचर वर्ण *safechars)
{
	अचर वर्ण *p = s;
	वर्ण *r;
	पूर्णाङ्क rsz;
	माप_प्रकार sz = s ? म_माप(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	स_रखो(&st, 0, परिमाण(st));
#endif
	यदि (!sz || !buf || !bufsiz)
		फर्काऊ रिक्त;

	r = buf;
	rsz = (पूर्णाङ्क) bufsiz;
	*width = 0;

	जबसम्म (p && *p) {
		यदि (safechars && म_अक्षर(safechars, *p)) {
			यदि (rsz < 2)
				रोक;
			*r++ = *p++;
			rsz--;
			जारी;
		}

		यदि ((*p == '\\' && *(p + 1) == 'x')
		    || है_नियंत्रण((अचिन्हित वर्ण) *p)) {
			यदि (rsz < 5)
				रोक;
			snprintf(r, rsz, "\\x%02x", (अचिन्हित वर्ण) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
			p++;
		}
#ifdef HAVE_WIDECHAR
		अन्यथा {
			ब_अक्षर_प्रकार wc;
			माप_प्रकार len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			यदि (len == 0)
				रोक;		/* end of string */

			यदि (len == (माप_प्रकार) -1 || len == (माप_प्रकार) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				यदि (!है_छाप((अचिन्हित वर्ण) *p)) {
					यदि (rsz < 5)
						रोक;
					snprintf(r, rsz, "\\x%02x", (अचिन्हित वर्ण) *p);
					r += 4;
					rsz -= 4;
					*width += 4;
				} अन्यथा {
					यदि (rsz < 2)
						रोक;
					(*width)++;
					*r++ = *p;
					rsz--;
				}
			} अन्यथा यदि (!iswprint(wc)) {
				माप_प्रकार i;
				लागि (i = 0; i < len; i++) {
					यदि (rsz < 5)
						रोक;
					snprintf(r, rsz, "\\x%02x", (अचिन्हित वर्ण) p[i]);
					r += 4;
					rsz -= 4;
					*width += 4;
				}
				यदि (i < len)
					रोक;
			} अन्यथा {
				यदि (rsz < (पूर्णाङ्क)len + 1)
					रोक;
				स_नकल(r, p, len);
				r += len;
				rsz -= len;
				*width += wcwidth(wc);
			}
			p += len;
		}
#अन्यथा
		अन्यथा यदि (!है_छाप((अचिन्हित वर्ण) *p)) {
			यदि (rsz < 5)
				रोक;
			snprintf(r, rsz, "\\x%02x", (अचिन्हित वर्ण) *p);
			p++;
			r += 4;
			rsz -= 4;
			*width += 4;
		} अन्यथा {
			यदि (rsz < 2)
				रोक;
			*r++ = *p++;
			rsz--;
			(*width)++;
		}
#endif
	}

	*r = '\0';
	फर्काऊ buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
वर्ण *mbs_invalid_encode_to_buffer(अचर वर्ण *s, माप_प्रकार *width, वर्ण *buf, माप_प्रकार bufsiz)
{
	अचर वर्ण *p = s;
	वर्ण *r;
	पूर्णाङ्क rsz;
	माप_प्रकार sz = s ? म_माप(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	स_रखो(&st, 0, परिमाण(st));
#endif
	यदि (!sz || !buf || !bufsiz)
		फर्काऊ रिक्त;

	r = buf;
	rsz = (पूर्णाङ्क) bufsiz;
	*width = 0;

	जबसम्म (p && *p) {
#ifdef HAVE_WIDECHAR
		ब_अक्षर_प्रकार wc;
		माप_प्रकार len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#अन्यथा
		माप_प्रकार len = 1;
#endif

		यदि (len == 0)
			रोक;		/* end of string */

		यदि (len == (माप_प्रकार) -1 || len == (माप_प्रकार) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			यदि (!है_छाप((अचिन्हित वर्ण) *p)) {
				यदि (rsz < 5)
					रोक;
				snprintf(r, rsz, "\\x%02x", (अचिन्हित वर्ण) *p);
				r += 4;
				rsz -= 4;
				*width += 4;
			} अन्यथा {
				यदि (rsz < 2)
					रोक;
				(*width)++;
				*r++ = *p;
				rsz--;
			}
		} अन्यथा यदि (*p == '\\' && *(p + 1) == 'x') {
			यदि (rsz < 5)
				रोक;
			snprintf(r, rsz, "\\x%02x", (अचिन्हित वर्ण) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
		} अन्यथा {
			यदि (rsz < (पूर्णाङ्क)len + 1)
				रोक;
			r = mempcpy(r, p, len);
			rsz -= len;
			*width += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	फर्काऊ buf;
}

/*
 * Guess size
 */
माप_प्रकार mbs_safe_encode_size(माप_प्रकार bytes)
{
	फर्काऊ (bytes * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
माप_प्रकार mbs_safe_decode_size(अचर वर्ण *p)
{
	माप_प्रकार bytes = 0;

	जबसम्म (p && *p) {
		यदि (*p == '\\' && *(p + 1) == 'x' &&
		    है_षष्ठादशक(*(p + 2)) && है_षष्ठादशक(*(p + 3)))
			p += 4;
		अन्यथा
			p++;
		bytes++;
	}
	फर्काऊ bytes;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
वर्ण *mbs_safe_encode(अचर वर्ण *s, माप_प्रकार *width)
{
	माप_प्रकार sz = s ? म_माप(s) : 0;
	माप_प्रकार bufsz;
	वर्ण *buf, *ret = रिक्त;

	यदि (!sz)
		फर्काऊ रिक्त;
	bufsz = mbs_safe_encode_size(sz);
	buf = दो_स्मृति(bufsz);
	यदि (buf)
		ret = mbs_safe_encode_to_buffer(s, width, buf, bufsz, रिक्त);
	यदि (!ret)
		मुक्त(buf);
	फर्काऊ ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
वर्ण *mbs_invalid_encode(अचर वर्ण *s, माप_प्रकार *width)
{
	माप_प्रकार sz = s ? म_माप(s) : 0;
	माप_प्रकार bufsz;
	वर्ण *buf, *ret = रिक्त;

	यदि (!sz)
		फर्काऊ रिक्त;
	bufsz = mbs_safe_encode_size(sz);
	buf = दो_स्मृति(bufsz);
	यदि (buf)
		ret = mbs_invalid_encode_to_buffer(s, width, buf, bufsz);
	यदि (!ret)
		मुक्त(buf);
	फर्काऊ ret;
}

#ifdef HAVE_WIDECHAR

स्थिर bool
wc_ensure_printable (ब_अक्षर_प्रकार *wchars)
{
  bool replaced = false;
  ब_अक्षर_प्रकार *wc = wchars;
  जबसम्म (*wc)
    {
      यदि (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  फर्काऊ replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
स्थिर ब_अक्षर_प्रकार *
wc_skip_ansi_escape (ब_अक्षर_प्रकार *wc)
{
  ब_अक्षर_प्रकार *p = wc + 1;

  यदि (*wc != L'\033' || !*p)
    फर्काऊ रिक्त;

  यदि (*p == L'[')
    {
      p++;
      जबसम्म (*p >= 0x30 && *p <= 0x3f)
        p++;
      जबसम्म (*p >= 0x20 && *p <= 0x2f)
        p++;
      यदि (*p >= 0x40 && *p <= 0x7e)
        फर्काऊ p + 1;
    }
  अन्यथा यदि ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    फर्काऊ p + 2;

  फर्काऊ रिक्त;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

स्थिर माप_प्रकार
wc_truncate (ब_अक्षर_प्रकार *wc, माप_प्रकार width)
{
  माप_प्रकार cells = 0;
  पूर्णाङ्क next_cells = 0;

  जबसम्म (*wc)
    {
      ब_अक्षर_प्रकार *after_escape = wc_skip_ansi_escape (wc);
      यदि (after_escape)
        {
          wc = after_escape;
          जारी;
        }

      next_cells = wcwidth (*wc);
      यदि (next_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          next_cells = 1;
        }
      यदि (cells + next_cells > width)
        रोक;

      cells += next_cells;
      wc++;
    }
  *wc = L'\0';
  फर्काऊ cells;
}

स्थिर पूर्णाङ्क
rpl_wcswidth (अचर ब_अक्षर_प्रकार *s, माप_प्रकार n)
{
  पूर्णाङ्क ret = 0;

  जबसम्म (n-- > 0 && *s != L'\0')
    {
      पूर्णाङ्क nwidth = wcwidth (*s++);
      यदि (nwidth == -1)             /* non printable */
        फर्काऊ -1;
      यदि (ret > (पूर्णांक_उच्च - nwidth)) /* overflow */
        फर्काऊ -1;
      ret += nwidth;
    }

  फर्काऊ ret;
}
#endif /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
माप_प्रकार
mbs_truncate(वर्ण *str, माप_प्रकार *width)
{
	ssize_t bytes = म_माप(str);
#ifdef HAVE_WIDECHAR
	ssize_t sz = mbstowcs(रिक्त, str, 0);
	ब_अक्षर_प्रकार *wcs = रिक्त;

	यदि (sz == (ssize_t) -1)
		गच्छ done;

	wcs = सुस्मृति(1, (sz + 1) * परिमाण(ब_अक्षर_प्रकार));
	यदि (!wcs)
		गच्छ done;

	यदि (!mbstowcs(wcs, str, sz))
		गच्छ done;
	*width = wc_truncate(wcs, *width);
	bytes = wcstombs(str, wcs, bytes);
done:
	मुक्त(wcs);
#अन्यथा
	{
		अचर वर्ण *p = str;
		अचर वर्ण *end = str + bytes;
		माप_प्रकार cells = 0;

		जबसम्म (p < end) {
			अचर वर्ण *after_escape = skip_ansi_escape(p, end);

			यदि (after_escape) {
				p = after_escape;
				जारी;
			}
			यदि (cells >= *width)
				रोक;
			p++;
			cells++;
		}
		bytes = p - str;
		*width = cells;
	}
#endif
	यदि (bytes >= 0)
		str[bytes] = '\0';
	फर्काऊ bytes;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

स्थिर वर्ण*
mbs_align_pad (वर्ण *dest, अचर वर्ण* dest_end, माप_प्रकार n_spaces, पूर्णाङ्क padchar)
{
  लागि (/* nothing */; n_spaces && (dest < dest_end); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  फर्काऊ dest;
}

माप_प्रकार
mbsalign (अचर वर्ण *src, वर्ण *dest, माप_प्रकार dest_size,
          माप_प्रकार *width, mbs_align_t align, पूर्णाङ्क flags)
{
	फर्काऊ mbsalign_with_padding(src, dest, dest_size, width, align, flags, ' ');
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
mbsalign_with_padding (अचर वर्ण *src, वर्ण *dest, माप_प्रकार dest_size,
	               माप_प्रकार *width, mbs_align_t align,
#ifdef HAVE_WIDECHAR
		       पूर्णाङ्क flags,
#अन्यथा
		       पूर्णाङ्क flags __attribute__((__unused__)),
#endif
		       पूर्णाङ्क padchar)
{
  माप_प्रकार ret = -1;
  माप_प्रकार src_size = म_माप (src) + 1;
  वर्ण *newstr = रिक्त;
  ब_अक्षर_प्रकार *str_wc = रिक्त;
  अचर वर्ण *str_to_print = src;
  माप_प्रकार n_cols = src_size - 1;
  माप_प्रकार n_used_bytes = n_cols; /* Not including NUL */
  माप_प्रकार n_spaces = 0, space_बायाँ;

#ifdef HAVE_WIDECHAR
  bool conversion = false;
  bool wc_enabled = false;

  /* In multi-byte locales convert to wide characters
     to allow easy truncation. Also determine number
     of screen columns used.  */
  यदि (MB_CUR_MAX > 1)
    {
      माप_प्रकार src_chars = mbstowcs (रिक्त, src, 0);
      यदि (src_chars == (माप_प्रकार) -1)
        {
          यदि (flags & MBA_UNIBYTE_FALLBACK)
            गच्छ mbsalign_unibyte;
          अन्यथा
            गच्छ mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = दो_स्मृति (src_chars * परिमाण (ब_अक्षर_प्रकार));
      यदि (str_wc == रिक्त)
        {
          यदि (flags & MBA_UNIBYTE_FALLBACK)
            गच्छ mbsalign_unibyte;
          अन्यथा
            गच्छ mbsalign_cleanup;
        }
      यदि (mbstowcs (str_wc, src, src_chars) != 0)
        {
          str_wc[src_chars - 1] = L'\0';
          wc_enabled = true;
          conversion = wc_ensure_printable (str_wc);
          n_cols = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  यदि (wc_enabled && (conversion || (n_cols > *width)))
    {
        यदि (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_size = wcstombs(रिक्त, str_wc, 0) + 1;
          }
        newstr = दो_स्मृति (src_size);
        यदि (newstr == रिक्त)
        {
          यदि (flags & MBA_UNIBYTE_FALLBACK)
            गच्छ mbsalign_unibyte;
          अन्यथा
            गच्छ mbsalign_cleanup;
        }
        str_to_print = newstr;
        n_cols = wc_truncate (str_wc, *width);
        n_used_bytes = wcstombs (newstr, str_wc, src_size);
    }

mbsalign_unibyte:
#endif

  यदि (n_cols > *width) /* Unibyte truncation required.  */
    {
      n_cols = *width;
      n_used_bytes = n_cols;
    }

  यदि (*width > n_cols) /* Padding required.  */
    n_spaces = *width - n_cols;

  /* indicate to caller how many cells needed (not including padding).  */
  *width = n_cols;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_used_bytes + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  यदि (dest_size != 0)
    {
      वर्ण *dest_end = dest + dest_size - 1;
      माप_प्रकार start_spaces;
      माप_प्रकार end_spaces;

      विकल्प (align)
        {
        पक्ष MBS_ALIGN_CENTER:
          start_spaces = n_spaces / 2 + n_spaces % 2;
          end_spaces = n_spaces / 2;
          रोक;
        पक्ष MBS_ALIGN_बायाँ:
          start_spaces = 0;
          end_spaces = n_spaces;
          रोक;
        पक्ष MBS_ALIGN_दायाँ:
          start_spaces = n_spaces;
          end_spaces = 0;
          रोक;
	स्वभाव:
	  पात();
        }

      dest = mbs_align_pad (dest, dest_end, start_spaces, padchar);
      space_बायाँ = dest_end - dest;
      dest = mempcpy (dest, str_to_print, min (n_used_bytes, space_बायाँ));
      mbs_align_pad (dest, dest_end, end_spaces, padchar);
    }
#ifdef HAVE_WIDECHAR
mbsalign_cleanup:
#endif
  मुक्त (str_wc);
  मुक्त (newstr);

  फर्काऊ ret;
}
