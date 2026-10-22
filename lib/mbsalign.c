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
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <stdio.h>
#समावेश <stdbool.h>
#समावेश <limits.h>
#समावेश <ctype.h>

#समावेश "c.h"
#समावेश "mbsalign.h"
#समावेश "strutils.h"
#समावेश "widechar.h"

/* Return the byte after a complete ANSI escape sequence, or NULL. */
अटल स्थिर अक्षर *skip_ansi_escape(स्थिर अक्षर *p, स्थिर अक्षर *end)
{
	स्थिर अक्षर *q;

	अगर (p >= end || (अचिन्हित अक्षर) *p != '\033' || p + 1 >= end)
		वापस शून्य;

	q = p + 1;
	अगर (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		जबतक (q < end && (अचिन्हित अक्षर) *q >= 0x30
				&& (अचिन्हित अक्षर) *q <= 0x3f)
			q++;
		जबतक (q < end && (अचिन्हित अक्षर) *q >= 0x20
				&& (अचिन्हित अक्षर) *q <= 0x2f)
			q++;
		अगर (q < end && (अचिन्हित अक्षर) *q >= 0x40
				&& (अचिन्हित अक्षर) *q <= 0x7e)
			वापस q + 1;
	} अन्यथा अगर (*q && म_अक्षर("()*+", *q) && q + 1 < end)
		वापस q + 2;

	वापस शून्य;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
माप_प्रकार mbs_nwidth(स्थिर अक्षर *buf, माप_प्रकार bufsz)
{
	स्थिर अक्षर *p = buf, *last = buf;
	माप_प्रकार width = 0;

#अगर_घोषित HAVE_WIDECHAR
	mbstate_t st;
	स_रखो(&st, 0, माप(st));
#पूर्ण_अगर
	अगर (p && *p && bufsz)
		last = p + (bufsz - 1);

	जबतक (p && *p && p <= last) {
		अगर (*p == '\033') {
			स्थिर अक्षर *e = skip_ansi_escape(p, last + 1);

			p = e ? e : p + 1;
			जारी;
		}
		अगर (है_नियंत्रण((अचिन्हित अक्षर) *p)) {
			p++;
			जारी;
		}
#अगर_घोषित HAVE_WIDECHAR
		ब_अक्षर_प्रकार wc;
		माप_प्रकार len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		अगर (len == 0)
			अवरोध;
		अगर (len > 0 && iswprint(wc)) {
			पूर्णांक x = wcwidth(wc);
			अगर (x > 0)
				width += x;
		} अन्यथा अगर (len == (माप_प्रकार) -1 || len == (माप_प्रकार) -2)
			len = 1;
		p += len;
#अन्यथा
		अगर (है_छाप((अचिन्हित अक्षर) *p))
			width++;
		p++;
#पूर्ण_अगर
	}

	वापस width;
}

माप_प्रकार mbs_width(स्थिर अक्षर *s)
{
	अगर (!s || !*s)
		वापस 0;
	वापस mbs_nwidth(s, म_माप(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
माप_प्रकार mbs_safe_nwidth(स्थिर अक्षर *buf, माप_प्रकार bufsz, माप_प्रकार *sz)
{
	स्थिर अक्षर *p = buf, *last = buf;
	माप_प्रकार width = 0, bytes = 0;

#अगर_घोषित HAVE_WIDECHAR
	mbstate_t st;
	स_रखो(&st, 0, माप(st));
#पूर्ण_अगर
	अगर (p && *p && bufsz)
		last = p + (bufsz - 1);

	जबतक (p && *p && p <= last) {
		अगर ((p < last && *p == '\\' && *(p + 1) == 'x')
		    || है_नियंत्रण((अचिन्हित अक्षर) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		}
#अगर_घोषित HAVE_WIDECHAR
		अन्यथा {
			ब_अक्षर_प्रकार wc;
			माप_प्रकार len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			अगर (len == 0)
				अवरोध;

			अगर (len == (माप_प्रकार) -1 || len == (माप_प्रकार) -2) {
				len = 1;
				अगर (है_छाप((अचिन्हित अक्षर) *p))
					width += 1, bytes += 1;
				अन्यथा
					width += 4, bytes += 4;

			} अन्यथा अगर (!iswprint(wc)) {
				width += len * 4;	/* hex encode whole sequence */
				bytes += len * 4;
			} अन्यथा {
				width += wcwidth(wc);	/* number of cells */
				bytes += len;		/* number of bytes */
			}
			p += len;
		}
#अन्यथा
		अन्यथा अगर (!है_छाप((अचिन्हित अक्षर) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		} अन्यथा {
			width++, bytes++;
			p++;
		}
#पूर्ण_अगर
	}

	अगर (sz)
		*sz = bytes;
	वापस width;
}

माप_प्रकार mbs_safe_width(स्थिर अक्षर *s)
{
	अगर (!s || !*s)
		वापस 0;
	वापस mbs_safe_nwidth(s, म_माप(s), शून्य);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
अक्षर *mbs_safe_encode_to_बफर(स्थिर अक्षर *s, माप_प्रकार *width, अक्षर *buf, माप_प्रकार bufsiz, स्थिर अक्षर *safechars)
{
	स्थिर अक्षर *p = s;
	अक्षर *r;
	पूर्णांक rsz;
	माप_प्रकार sz = s ? म_माप(s) : 0;

#अगर_घोषित HAVE_WIDECHAR
	mbstate_t st;
	स_रखो(&st, 0, माप(st));
#पूर्ण_अगर
	अगर (!sz || !buf || !bufsiz)
		वापस शून्य;

	r = buf;
	rsz = (पूर्णांक) bufsiz;
	*width = 0;

	जबतक (p && *p) {
		अगर (safechars && म_अक्षर(safechars, *p)) {
			अगर (rsz < 2)
				अवरोध;
			*r++ = *p++;
			rsz--;
			जारी;
		}

		अगर ((*p == '\\' && *(p + 1) == 'x')
		    || है_नियंत्रण((अचिन्हित अक्षर) *p)) {
			अगर (rsz < 5)
				अवरोध;
			snprintf(r, rsz, "\\x%02x", (अचिन्हित अक्षर) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
			p++;
		}
#अगर_घोषित HAVE_WIDECHAR
		अन्यथा {
			ब_अक्षर_प्रकार wc;
			माप_प्रकार len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			अगर (len == 0)
				अवरोध;		/* end of string */

			अगर (len == (माप_प्रकार) -1 || len == (माप_प्रकार) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				अगर (!है_छाप((अचिन्हित अक्षर) *p)) {
					अगर (rsz < 5)
						अवरोध;
					snprintf(r, rsz, "\\x%02x", (अचिन्हित अक्षर) *p);
					r += 4;
					rsz -= 4;
					*width += 4;
				} अन्यथा {
					अगर (rsz < 2)
						अवरोध;
					(*width)++;
					*r++ = *p;
					rsz--;
				}
			} अन्यथा अगर (!iswprint(wc)) {
				माप_प्रकार i;
				क्रम (i = 0; i < len; i++) {
					अगर (rsz < 5)
						अवरोध;
					snprintf(r, rsz, "\\x%02x", (अचिन्हित अक्षर) p[i]);
					r += 4;
					rsz -= 4;
					*width += 4;
				}
				अगर (i < len)
					अवरोध;
			} अन्यथा {
				अगर (rsz < (पूर्णांक)len + 1)
					अवरोध;
				स_नकल(r, p, len);
				r += len;
				rsz -= len;
				*width += wcwidth(wc);
			}
			p += len;
		}
#अन्यथा
		अन्यथा अगर (!है_छाप((अचिन्हित अक्षर) *p)) {
			अगर (rsz < 5)
				अवरोध;
			snprintf(r, rsz, "\\x%02x", (अचिन्हित अक्षर) *p);
			p++;
			r += 4;
			rsz -= 4;
			*width += 4;
		} अन्यथा {
			अगर (rsz < 2)
				अवरोध;
			*r++ = *p++;
			rsz--;
			(*width)++;
		}
#पूर्ण_अगर
	}

	*r = '\0';
	वापस buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
अक्षर *mbs_invalid_encode_to_बफर(स्थिर अक्षर *s, माप_प्रकार *width, अक्षर *buf, माप_प्रकार bufsiz)
{
	स्थिर अक्षर *p = s;
	अक्षर *r;
	पूर्णांक rsz;
	माप_प्रकार sz = s ? म_माप(s) : 0;

#अगर_घोषित HAVE_WIDECHAR
	mbstate_t st;
	स_रखो(&st, 0, माप(st));
#पूर्ण_अगर
	अगर (!sz || !buf || !bufsiz)
		वापस शून्य;

	r = buf;
	rsz = (पूर्णांक) bufsiz;
	*width = 0;

	जबतक (p && *p) {
#अगर_घोषित HAVE_WIDECHAR
		ब_अक्षर_प्रकार wc;
		माप_प्रकार len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#अन्यथा
		माप_प्रकार len = 1;
#पूर्ण_अगर

		अगर (len == 0)
			अवरोध;		/* end of string */

		अगर (len == (माप_प्रकार) -1 || len == (माप_प्रकार) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			अगर (!है_छाप((अचिन्हित अक्षर) *p)) {
				अगर (rsz < 5)
					अवरोध;
				snprintf(r, rsz, "\\x%02x", (अचिन्हित अक्षर) *p);
				r += 4;
				rsz -= 4;
				*width += 4;
			} अन्यथा {
				अगर (rsz < 2)
					अवरोध;
				(*width)++;
				*r++ = *p;
				rsz--;
			}
		} अन्यथा अगर (*p == '\\' && *(p + 1) == 'x') {
			अगर (rsz < 5)
				अवरोध;
			snprintf(r, rsz, "\\x%02x", (अचिन्हित अक्षर) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
		} अन्यथा {
			अगर (rsz < (पूर्णांक)len + 1)
				अवरोध;
			r = mempcpy(r, p, len);
			rsz -= len;
			*width += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	वापस buf;
}

/*
 * Guess size
 */
माप_प्रकार mbs_safe_encode_आकार(माप_प्रकार bytes)
{
	वापस (bytes * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
माप_प्रकार mbs_safe_decode_आकार(स्थिर अक्षर *p)
{
	माप_प्रकार bytes = 0;

	जबतक (p && *p) {
		अगर (*p == '\\' && *(p + 1) == 'x' &&
		    है_षष्ठादशक(*(p + 2)) && है_षष्ठादशक(*(p + 3)))
			p += 4;
		अन्यथा
			p++;
		bytes++;
	}
	वापस bytes;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
अक्षर *mbs_safe_encode(स्थिर अक्षर *s, माप_प्रकार *width)
{
	माप_प्रकार sz = s ? म_माप(s) : 0;
	माप_प्रकार bufsz;
	अक्षर *buf, *ret = शून्य;

	अगर (!sz)
		वापस शून्य;
	bufsz = mbs_safe_encode_आकार(sz);
	buf = दो_स्मृति(bufsz);
	अगर (buf)
		ret = mbs_safe_encode_to_बफर(s, width, buf, bufsz, शून्य);
	अगर (!ret)
		मुक्त(buf);
	वापस ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
अक्षर *mbs_invalid_encode(स्थिर अक्षर *s, माप_प्रकार *width)
{
	माप_प्रकार sz = s ? म_माप(s) : 0;
	माप_प्रकार bufsz;
	अक्षर *buf, *ret = शून्य;

	अगर (!sz)
		वापस शून्य;
	bufsz = mbs_safe_encode_आकार(sz);
	buf = दो_स्मृति(bufsz);
	अगर (buf)
		ret = mbs_invalid_encode_to_बफर(s, width, buf, bufsz);
	अगर (!ret)
		मुक्त(buf);
	वापस ret;
}

#अगर_घोषित HAVE_WIDECHAR

अटल bool
wc_ensure_printable (ब_अक्षर_प्रकार *wchars)
{
  bool replaced = false;
  ब_अक्षर_प्रकार *wc = wchars;
  जबतक (*wc)
    {
      अगर (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  वापस replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
अटल ब_अक्षर_प्रकार *
wc_skip_ansi_escape (ब_अक्षर_प्रकार *wc)
{
  ब_अक्षर_प्रकार *p = wc + 1;

  अगर (*wc != L'\033' || !*p)
    वापस शून्य;

  अगर (*p == L'[')
    {
      p++;
      जबतक (*p >= 0x30 && *p <= 0x3f)
        p++;
      जबतक (*p >= 0x20 && *p <= 0x2f)
        p++;
      अगर (*p >= 0x40 && *p <= 0x7e)
        वापस p + 1;
    }
  अन्यथा अगर ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    वापस p + 2;

  वापस शून्य;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

अटल माप_प्रकार
wc_truncate (ब_अक्षर_प्रकार *wc, माप_प्रकार width)
{
  माप_प्रकार cells = 0;
  पूर्णांक अग्र_cells = 0;

  जबतक (*wc)
    {
      ब_अक्षर_प्रकार *after_escape = wc_skip_ansi_escape (wc);
      अगर (after_escape)
        {
          wc = after_escape;
          जारी;
        }

      अग्र_cells = wcwidth (*wc);
      अगर (अग्र_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          अग्र_cells = 1;
        }
      अगर (cells + अग्र_cells > width)
        अवरोध;

      cells += अग्र_cells;
      wc++;
    }
  *wc = L'\0';
  वापस cells;
}

अटल पूर्णांक
rpl_wcswidth (स्थिर ब_अक्षर_प्रकार *s, माप_प्रकार n)
{
  पूर्णांक ret = 0;

  जबतक (n-- > 0 && *s != L'\0')
    {
      पूर्णांक nwidth = wcwidth (*s++);
      अगर (nwidth == -1)             /* non printable */
        वापस -1;
      अगर (ret > (पूर्णांक_उच्च - nwidth)) /* overflow */
        वापस -1;
      ret += nwidth;
    }

  वापस ret;
}
#पूर्ण_अगर /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
माप_प्रकार
mbs_truncate(अक्षर *str, माप_प्रकार *width)
{
	ssize_t bytes = म_माप(str);
#अगर_घोषित HAVE_WIDECHAR
	ssize_t sz = mbstowcs(शून्य, str, 0);
	ब_अक्षर_प्रकार *wcs = शून्य;

	अगर (sz == (ssize_t) -1)
		जाओ done;

	wcs = सुस्मृति(1, (sz + 1) * माप(ब_अक्षर_प्रकार));
	अगर (!wcs)
		जाओ done;

	अगर (!mbstowcs(wcs, str, sz))
		जाओ done;
	*width = wc_truncate(wcs, *width);
	bytes = wcstombs(str, wcs, bytes);
done:
	मुक्त(wcs);
#अन्यथा
	{
		स्थिर अक्षर *p = str;
		स्थिर अक्षर *end = str + bytes;
		माप_प्रकार cells = 0;

		जबतक (p < end) {
			स्थिर अक्षर *after_escape = skip_ansi_escape(p, end);

			अगर (after_escape) {
				p = after_escape;
				जारी;
			}
			अगर (cells >= *width)
				अवरोध;
			p++;
			cells++;
		}
		bytes = p - str;
		*width = cells;
	}
#पूर्ण_अगर
	अगर (bytes >= 0)
		str[bytes] = '\0';
	वापस bytes;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

अटल अक्षर*
mbs_align_pad (अक्षर *dest, स्थिर अक्षर* dest_end, माप_प्रकार n_spaces, पूर्णांक padchar)
{
  क्रम (/* nothing */; n_spaces && (dest < dest_end); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  वापस dest;
}

माप_प्रकार
mbsalign (स्थिर अक्षर *src, अक्षर *dest, माप_प्रकार dest_आकार,
          माप_प्रकार *width, mbs_align_t align, पूर्णांक flags)
{
	वापस mbsalign_with_padding(src, dest, dest_आकार, width, align, flags, ' ');
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
mbsalign_with_padding (स्थिर अक्षर *src, अक्षर *dest, माप_प्रकार dest_आकार,
	               माप_प्रकार *width, mbs_align_t align,
#अगर_घोषित HAVE_WIDECHAR
		       पूर्णांक flags,
#अन्यथा
		       पूर्णांक flags __attribute__((__unused__)),
#पूर्ण_अगर
		       पूर्णांक padchar)
{
  माप_प्रकार ret = -1;
  माप_प्रकार src_आकार = म_माप (src) + 1;
  अक्षर *newstr = शून्य;
  ब_अक्षर_प्रकार *str_wc = शून्य;
  स्थिर अक्षर *str_to_print = src;
  माप_प्रकार n_cols = src_आकार - 1;
  माप_प्रकार n_used_bytes = n_cols; /* Not including NUL */
  माप_प्रकार n_spaces = 0, space_बायाँ;

#अगर_घोषित HAVE_WIDECHAR
  bool conversion = false;
  bool wc_enabled = false;

  /* In multi-byte locales convert to wide characters
     to allow easy truncation. Also determine number
     of screen columns used.  */
  अगर (MB_CUR_MAX > 1)
    {
      माप_प्रकार src_chars = mbstowcs (शून्य, src, 0);
      अगर (src_chars == (माप_प्रकार) -1)
        {
          अगर (flags & MBA_UNIBYTE_FALLBACK)
            जाओ mbsalign_unibyte;
          अन्यथा
            जाओ mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = दो_स्मृति (src_chars * माप (ब_अक्षर_प्रकार));
      अगर (str_wc == शून्य)
        {
          अगर (flags & MBA_UNIBYTE_FALLBACK)
            जाओ mbsalign_unibyte;
          अन्यथा
            जाओ mbsalign_cleanup;
        }
      अगर (mbstowcs (str_wc, src, src_chars) != 0)
        {
          str_wc[src_chars - 1] = L'\0';
          wc_enabled = true;
          conversion = wc_ensure_printable (str_wc);
          n_cols = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  अगर (wc_enabled && (conversion || (n_cols > *width)))
    {
        अगर (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_आकार = wcstombs(शून्य, str_wc, 0) + 1;
          }
        newstr = दो_स्मृति (src_आकार);
        अगर (newstr == शून्य)
        {
          अगर (flags & MBA_UNIBYTE_FALLBACK)
            जाओ mbsalign_unibyte;
          अन्यथा
            जाओ mbsalign_cleanup;
        }
        str_to_print = newstr;
        n_cols = wc_truncate (str_wc, *width);
        n_used_bytes = wcstombs (newstr, str_wc, src_आकार);
    }

mbsalign_unibyte:
#पूर्ण_अगर

  अगर (n_cols > *width) /* Unibyte truncation required.  */
    {
      n_cols = *width;
      n_used_bytes = n_cols;
    }

  अगर (*width > n_cols) /* Padding required.  */
    n_spaces = *width - n_cols;

  /* indicate to caller how many cells needed (not including padding).  */
  *width = n_cols;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_used_bytes + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  अगर (dest_आकार != 0)
    {
      अक्षर *dest_end = dest + dest_आकार - 1;
      माप_प्रकार start_spaces;
      माप_प्रकार end_spaces;

      चयन (align)
        {
        हाल MBS_ALIGN_CENTER:
          start_spaces = n_spaces / 2 + n_spaces % 2;
          end_spaces = n_spaces / 2;
          अवरोध;
        हाल MBS_ALIGN_बायाँ:
          start_spaces = 0;
          end_spaces = n_spaces;
          अवरोध;
        हाल MBS_ALIGN_दायाँ:
          start_spaces = n_spaces;
          end_spaces = 0;
          अवरोध;
	शेष:
	  पात();
        }

      dest = mbs_align_pad (dest, dest_end, start_spaces, padchar);
      space_बायाँ = dest_end - dest;
      dest = mempcpy (dest, str_to_print, min (n_used_bytes, space_बायाँ));
      mbs_align_pad (dest, dest_end, end_spaces, padchar);
    }
#अगर_घोषित HAVE_WIDECHAR
mbsalign_cleanup:
#पूर्ण_अगर
  मुक्त (str_wc);
  मुक्त (newstr);

  वापस ret;
}
