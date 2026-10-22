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
स्थिर ध्रुव वर्ण *skip_ansi_escape(ध्रुव वर्ण *p, ध्रुव वर्ण *अन्तः)
{
	ध्रुव वर्ण *q;

	यदि (p >= अन्तः || (अचिह्नित वर्ण) *p != '\033' || p + 1 >= अन्तः)
		प्रत्यागम रिक्त;

	q = p + 1;
	यदि (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		यावत् (q < अन्तः && (अचिह्नित वर्ण) *q >= 0x30
				&& (अचिह्नित वर्ण) *q <= 0x3f)
			q++;
		यावत् (q < अन्तः && (अचिह्नित वर्ण) *q >= 0x20
				&& (अचिह्नित वर्ण) *q <= 0x2f)
			q++;
		यदि (q < अन्तः && (अचिह्नित वर्ण) *q >= 0x40
				&& (अचिह्नित वर्ण) *q <= 0x7e)
			प्रत्यागम q + 1;
	} अन्यथा यदि (*q && strchr("()*+", *q) && q + 1 < अन्तः)
		प्रत्यागम q + 2;

	प्रत्यागम रिक्त;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
size_t mbs_nwidth(ध्रुव वर्ण *buf, size_t bufsz)
{
	ध्रुव वर्ण *p = buf, *चरमः = buf;
	size_t विस्तारः = 0;

#यदि_परिभाषितम् HAVE_WIDECHAR
	mbstate_t st;
	स्मृतिं_पूरय(&st, 0, परिमाण(st));
#यदि_अन्तः
	यदि (p && *p && bufsz)
		चरमः = p + (bufsz - 1);

	यावत् (p && *p && p <= चरमः) {
		यदि (*p == '\033') {
			ध्रुव वर्ण *e = skip_ansi_escape(p, चरमः + 1);

			p = e ? e : p + 1;
			अनुवर्त;
		}
		यदि (iscntrl((अचिह्नित वर्ण) *p)) {
			p++;
			अनुवर्त;
		}
#यदि_परिभाषितम् HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		यदि (len == 0)
			भङ्ग;
		यदि (len > 0 && iswprint(wc)) {
			पूर्णाङ्क x = wcwidth(wc);
			यदि (x > 0)
				विस्तारः += x;
		} अन्यथा यदि (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#अन्यथा
		यदि (isprint((अचिह्नित वर्ण) *p))
			विस्तारः++;
		p++;
#यदि_अन्तः
	}

	प्रत्यागम विस्तारः;
}

size_t mbs_विस्तारः(ध्रुव वर्ण *s)
{
	यदि (!s || !*s)
		प्रत्यागम 0;
	प्रत्यागम mbs_nwidth(s, वाक्यदैर्घ्यम्(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
size_t mbs_safe_nwidth(ध्रुव वर्ण *buf, size_t bufsz, size_t *sz)
{
	ध्रुव वर्ण *p = buf, *चरमः = buf;
	size_t विस्तारः = 0, अष्टकानि = 0;

#यदि_परिभाषितम् HAVE_WIDECHAR
	mbstate_t st;
	स्मृतिं_पूरय(&st, 0, परिमाण(st));
#यदि_अन्तः
	यदि (p && *p && bufsz)
		चरमः = p + (bufsz - 1);

	यावत् (p && *p && p <= चरमः) {
		यदि ((p < चरमः && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((अचिह्नित वर्ण) *p)) {
			विस्तारः += 4, अष्टकानि += 4;		/* *p encoded to \x?? */
			p++;
		}
#यदि_परिभाषितम् HAVE_WIDECHAR
		अन्यथा {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			यदि (len == 0)
				भङ्ग;

			यदि (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				यदि (isprint((अचिह्नित वर्ण) *p))
					विस्तारः += 1, अष्टकानि += 1;
				अन्यथा
					विस्तारः += 4, अष्टकानि += 4;

			} अन्यथा यदि (!iswprint(wc)) {
				विस्तारः += len * 4;	/* hex encode whole sequence */
				अष्टकानि += len * 4;
			} अन्यथा {
				विस्तारः += wcwidth(wc);	/* number of cells */
				अष्टकानि += len;		/* number of bytes */
			}
			p += len;
		}
#अन्यथा
		अन्यथा यदि (!isprint((अचिह्नित वर्ण) *p)) {
			विस्तारः += 4, अष्टकानि += 4;		/* *p encoded to \x?? */
			p++;
		} अन्यथा {
			विस्तारः++, अष्टकानि++;
			p++;
		}
#यदि_अन्तः
	}

	यदि (sz)
		*sz = अष्टकानि;
	प्रत्यागम विस्तारः;
}

size_t mbs_safe_विस्तारः(ध्रुव वर्ण *s)
{
	यदि (!s || !*s)
		प्रत्यागम 0;
	प्रत्यागम mbs_safe_nwidth(s, वाक्यदैर्घ्यम्(s), रिक्त);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
वर्ण *mbs_safe_encode_गन्तव्यम्_बफर(ध्रुव वर्ण *s, size_t *विस्तारः, वर्ण *buf, size_t bufsiz, ध्रुव वर्ण *safechars)
{
	ध्रुव वर्ण *p = s;
	वर्ण *r;
	पूर्णाङ्क rsz;
	size_t sz = s ? वाक्यदैर्घ्यम्(s) : 0;

#यदि_परिभाषितम् HAVE_WIDECHAR
	mbstate_t st;
	स्मृतिं_पूरय(&st, 0, परिमाण(st));
#यदि_अन्तः
	यदि (!sz || !buf || !bufsiz)
		प्रत्यागम रिक्त;

	r = buf;
	rsz = (पूर्णाङ्क) bufsiz;
	*विस्तारः = 0;

	यावत् (p && *p) {
		यदि (safechars && strchr(safechars, *p)) {
			यदि (rsz < 2)
				भङ्ग;
			*r++ = *p++;
			rsz--;
			अनुवर्त;
		}

		यदि ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((अचिह्नित वर्ण) *p)) {
			यदि (rsz < 5)
				भङ्ग;
			snprintf(r, rsz, "\\x%02x", (अचिह्नित वर्ण) *p);
			r += 4;
			rsz -= 4;
			*विस्तारः += 4;
			p++;
		}
#यदि_परिभाषितम् HAVE_WIDECHAR
		अन्यथा {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			यदि (len == 0)
				भङ्ग;		/* end of string */

			यदि (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				यदि (!isprint((अचिह्नित वर्ण) *p)) {
					यदि (rsz < 5)
						भङ्ग;
					snprintf(r, rsz, "\\x%02x", (अचिह्नित वर्ण) *p);
					r += 4;
					rsz -= 4;
					*विस्तारः += 4;
				} अन्यथा {
					यदि (rsz < 2)
						भङ्ग;
					(*विस्तारः)++;
					*r++ = *p;
					rsz--;
				}
			} अन्यथा यदि (!iswprint(wc)) {
				size_t i;
				प्रति (i = 0; i < len; i++) {
					यदि (rsz < 5)
						भङ्ग;
					snprintf(r, rsz, "\\x%02x", (अचिह्नित वर्ण) p[i]);
					r += 4;
					rsz -= 4;
					*विस्तारः += 4;
				}
				यदि (i < len)
					भङ्ग;
			} अन्यथा {
				यदि (rsz < (पूर्णाङ्क)len + 1)
					भङ्ग;
				स्मृतिं_प्रतिलिख(r, p, len);
				r += len;
				rsz -= len;
				*विस्तारः += wcwidth(wc);
			}
			p += len;
		}
#अन्यथा
		अन्यथा यदि (!isprint((अचिह्नित वर्ण) *p)) {
			यदि (rsz < 5)
				भङ्ग;
			snprintf(r, rsz, "\\x%02x", (अचिह्नित वर्ण) *p);
			p++;
			r += 4;
			rsz -= 4;
			*विस्तारः += 4;
		} अन्यथा {
			यदि (rsz < 2)
				भङ्ग;
			*r++ = *p++;
			rsz--;
			(*विस्तारः)++;
		}
#यदि_अन्तः
	}

	*r = '\0';
	प्रत्यागम buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
वर्ण *mbs_invalid_encode_गन्तव्यम्_बफर(ध्रुव वर्ण *s, size_t *विस्तारः, वर्ण *buf, size_t bufsiz)
{
	ध्रुव वर्ण *p = s;
	वर्ण *r;
	पूर्णाङ्क rsz;
	size_t sz = s ? वाक्यदैर्घ्यम्(s) : 0;

#यदि_परिभाषितम् HAVE_WIDECHAR
	mbstate_t st;
	स्मृतिं_पूरय(&st, 0, परिमाण(st));
#यदि_अन्तः
	यदि (!sz || !buf || !bufsiz)
		प्रत्यागम रिक्त;

	r = buf;
	rsz = (पूर्णाङ्क) bufsiz;
	*विस्तारः = 0;

	यावत् (p && *p) {
#यदि_परिभाषितम् HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#अन्यथा
		size_t len = 1;
#यदि_अन्तः

		यदि (len == 0)
			भङ्ग;		/* end of string */

		यदि (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			यदि (!isprint((अचिह्नित वर्ण) *p)) {
				यदि (rsz < 5)
					भङ्ग;
				snprintf(r, rsz, "\\x%02x", (अचिह्नित वर्ण) *p);
				r += 4;
				rsz -= 4;
				*विस्तारः += 4;
			} अन्यथा {
				यदि (rsz < 2)
					भङ्ग;
				(*विस्तारः)++;
				*r++ = *p;
				rsz--;
			}
		} अन्यथा यदि (*p == '\\' && *(p + 1) == 'x') {
			यदि (rsz < 5)
				भङ्ग;
			snprintf(r, rsz, "\\x%02x", (अचिह्नित वर्ण) *p);
			r += 4;
			rsz -= 4;
			*विस्तारः += 4;
		} अन्यथा {
			यदि (rsz < (पूर्णाङ्क)len + 1)
				भङ्ग;
			r = mempcpy(r, p, len);
			rsz -= len;
			*विस्तारः += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	प्रत्यागम buf;
}

/*
 * Guess size
 */
size_t mbs_safe_encode_आकार(size_t अष्टकानि)
{
	प्रत्यागम (अष्टकानि * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_safe_decode_आकार(ध्रुव वर्ण *p)
{
	size_t अष्टकानि = 0;

	यावत् (p && *p) {
		यदि (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		अन्यथा
			p++;
		अष्टकानि++;
	}
	प्रत्यागम अष्टकानि;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
वर्ण *mbs_safe_encode(ध्रुव वर्ण *s, size_t *विस्तारः)
{
	size_t sz = s ? वाक्यदैर्घ्यम्(s) : 0;
	size_t bufsz;
	वर्ण *buf, *ret = रिक्त;

	यदि (!sz)
		प्रत्यागम रिक्त;
	bufsz = mbs_safe_encode_आकार(sz);
	buf = स्मृतिं_देहि(bufsz);
	यदि (buf)
		ret = mbs_safe_encode_गन्तव्यम्_बफर(s, विस्तारः, buf, bufsz, रिक्त);
	यदि (!ret)
		मुक्त(buf);
	प्रत्यागम ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
वर्ण *mbs_invalid_encode(ध्रुव वर्ण *s, size_t *विस्तारः)
{
	size_t sz = s ? वाक्यदैर्घ्यम्(s) : 0;
	size_t bufsz;
	वर्ण *buf, *ret = रिक्त;

	यदि (!sz)
		प्रत्यागम रिक्त;
	bufsz = mbs_safe_encode_आकार(sz);
	buf = स्मृतिं_देहि(bufsz);
	यदि (buf)
		ret = mbs_invalid_encode_गन्तव्यम्_बफर(s, विस्तारः, buf, bufsz);
	यदि (!ret)
		मुक्त(buf);
	प्रत्यागम ret;
}

#यदि_परिभाषितम् HAVE_WIDECHAR

स्थिर bool
wc_ensure_printable (wchar_t *wchars)
{
  bool replaced = false;
  wchar_t *wc = wchars;
  यावत् (*wc)
    {
      यदि (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  प्रत्यागम replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
स्थिर wchar_t *
wc_skip_ansi_escape (wchar_t *wc)
{
  wchar_t *p = wc + 1;

  यदि (*wc != L'\033' || !*p)
    प्रत्यागम रिक्त;

  यदि (*p == L'[')
    {
      p++;
      यावत् (*p >= 0x30 && *p <= 0x3f)
        p++;
      यावत् (*p >= 0x20 && *p <= 0x2f)
        p++;
      यदि (*p >= 0x40 && *p <= 0x7e)
        प्रत्यागम p + 1;
    }
  अन्यथा यदि ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    प्रत्यागम p + 2;

  प्रत्यागम रिक्त;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

स्थिर size_t
wc_truncate (wchar_t *wc, size_t विस्तारः)
{
  size_t cells = 0;
  पूर्णाङ्क अग्र_cells = 0;

  यावत् (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      यदि (after_escape)
        {
          wc = after_escape;
          अनुवर्त;
        }

      अग्र_cells = wcwidth (*wc);
      यदि (अग्र_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          अग्र_cells = 1;
        }
      यदि (cells + अग्र_cells > विस्तारः)
        भङ्ग;

      cells += अग्र_cells;
      wc++;
    }
  *wc = L'\0';
  प्रत्यागम cells;
}

स्थिर पूर्णाङ्क
rpl_wcswidth (ध्रुव wchar_t *s, size_t n)
{
  पूर्णाङ्क ret = 0;

  यावत् (n-- > 0 && *s != L'\0')
    {
      पूर्णाङ्क nwidth = wcwidth (*s++);
      यदि (nwidth == -1)             /* non printable */
        प्रत्यागम -1;
      यदि (ret > (अधिकतम_पूर्णाङ्कः - nwidth)) /*अतिप्रवाहः*/
        प्रत्यागम -1;
      ret += nwidth;
    }

  प्रत्यागम ret;
}
#यदि_अन्तः /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
size_t
mbs_truncate(वर्ण *str, size_t *विस्तारः)
{
	ssize_t अष्टकानि = वाक्यदैर्घ्यम्(str);
#यदि_परिभाषितम् HAVE_WIDECHAR
	ssize_t sz = mbstowcs(रिक्त, str, 0);
	wchar_t *wcs = रिक्त;

	यदि (sz == (ssize_t) -1)
		गच्छ समाप्तम्;

	wcs = शून्यस्मृतिं_देहि(1, (sz + 1) * परिमाण(wchar_t));
	यदि (!wcs)
		गच्छ समाप्तम्;

	यदि (!mbstowcs(wcs, str, sz))
		गच्छ समाप्तम्;
	*विस्तारः = wc_truncate(wcs, *विस्तारः);
	अष्टकानि = wcstombs(str, wcs, अष्टकानि);
समाप्तम्:
	मुक्त(wcs);
#अन्यथा
	{
		ध्रुव वर्ण *p = str;
		ध्रुव वर्ण *अन्तः = str + अष्टकानि;
		size_t cells = 0;

		यावत् (p < अन्तः) {
			ध्रुव वर्ण *after_escape = skip_ansi_escape(p, अन्तः);

			यदि (after_escape) {
				p = after_escape;
				अनुवर्त;
			}
			यदि (cells >= *विस्तारः)
				भङ्ग;
			p++;
			cells++;
		}
		अष्टकानि = p - str;
		*विस्तारः = cells;
	}
#यदि_अन्तः
	यदि (अष्टकानि >= 0)
		str[अष्टकानि] = '\0';
	प्रत्यागम अष्टकानि;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

स्थिर वर्ण*
mbs_align_pad (वर्ण *dest, ध्रुव वर्ण* dest_अन्तः, size_t n_spaces, पूर्णाङ्क padchar)
{
  प्रति (/* nothing */; n_spaces && (dest < dest_अन्तः); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  प्रत्यागम dest;
}

size_t
mbsalign (ध्रुव वर्ण *src, वर्ण *dest, size_t dest_आकार,
          size_t *विस्तारः, mbs_align_t align, पूर्णाङ्क flags)
{
	प्रत्यागम mbsalign_with_padding(src, dest, dest_आकार, विस्तारः, align, flags, ' ');
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

size_t
mbsalign_with_padding (ध्रुव वर्ण *src, वर्ण *dest, size_t dest_आकार,
	               size_t *विस्तारः, mbs_align_t align,
#यदि_परिभाषितम् HAVE_WIDECHAR
		       पूर्णाङ्क flags,
#अन्यथा
		       पूर्णाङ्क flags __attribute__((__unused__)),
#यदि_अन्तः
		       पूर्णाङ्क padchar)
{
  size_t ret = -1;
  size_t src_आकार = वाक्यदैर्घ्यम् (src) + 1;
  वर्ण *newstr = रिक्त;
  wchar_t *str_wc = रिक्त;
  ध्रुव वर्ण *str_गन्तव्यम्_print = src;
  size_t n_स्तम्भसङ्ख्या = src_आकार - 1;
  size_t n_प्रयुक्तम्_अष्टकानि = n_स्तम्भसङ्ख्या; /* Not including NUL */
  size_t n_spaces = 0, space_वामः;

#यदि_परिभाषितम् HAVE_WIDECHAR
  bool conversion = false;
  bool wc_enabled = false;

  /* In multi-byte locales convert to wide characters
     to allow easy truncation. Also determine number
     of screen columns used.  */
  यदि (MB_CUR_MAX > 1)
    {
      size_t src_chars = mbstowcs (रिक्त, src, 0);
      यदि (src_chars == (size_t) -1)
        {
          यदि (flags & MBA_UNIBYTE_FALLBACK)
            गच्छ mbsalign_unibyte;
          अन्यथा
            गच्छ mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = स्मृतिं_देहि (src_chars * परिमाण (wchar_t));
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
          n_स्तम्भसङ्ख्या = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  यदि (wc_enabled && (conversion || (n_स्तम्भसङ्ख्या > *विस्तारः)))
    {
        यदि (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_आकार = wcstombs(रिक्त, str_wc, 0) + 1;
          }
        newstr = स्मृतिं_देहि (src_आकार);
        यदि (newstr == रिक्त)
        {
          यदि (flags & MBA_UNIBYTE_FALLBACK)
            गच्छ mbsalign_unibyte;
          अन्यथा
            गच्छ mbsalign_cleanup;
        }
        str_गन्तव्यम्_print = newstr;
        n_स्तम्भसङ्ख्या = wc_truncate (str_wc, *विस्तारः);
        n_प्रयुक्तम्_अष्टकानि = wcstombs (newstr, str_wc, src_आकार);
    }

mbsalign_unibyte:
#यदि_अन्तः

  यदि (n_स्तम्भसङ्ख्या > *विस्तारः) /* Unibyte truncation required.  */
    {
      n_स्तम्भसङ्ख्या = *विस्तारः;
      n_प्रयुक्तम्_अष्टकानि = n_स्तम्भसङ्ख्या;
    }

  यदि (*विस्तारः > n_स्तम्भसङ्ख्या) /* Padding required.  */
    n_spaces = *विस्तारः - n_स्तम्भसङ्ख्या;

  /* indicate to caller how many cells needed (not including padding).  */
  *विस्तारः = n_स्तम्भसङ्ख्या;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_प्रयुक्तम्_अष्टकानि + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  यदि (dest_आकार != 0)
    {
      वर्ण *dest_अन्तः = dest + dest_आकार - 1;
      size_t start_spaces;
      size_t अन्तः_spaces;

      विकल्प (align)
        {
        पक्ष MBS_ALIGN_CENTER:
          start_spaces = n_spaces / 2 + n_spaces % 2;
          अन्तः_spaces = n_spaces / 2;
          भङ्ग;
        पक्ष MBS_ALIGN_वामः:
          start_spaces = 0;
          अन्तः_spaces = n_spaces;
          भङ्ग;
        पक्ष MBS_ALIGN_दक्षिणः:
          start_spaces = n_spaces;
          अन्तः_spaces = 0;
          भङ्ग;
	स्वभाव:
	  abort();
        }

      dest = mbs_align_pad (dest, dest_अन्तः, start_spaces, padchar);
      space_वामः = dest_अन्तः - dest;
      dest = mempcpy (dest, str_गन्तव्यम्_print, min (n_प्रयुक्तम्_अष्टकानि, space_वामः));
      mbs_align_pad (dest, dest_अन्तः, अन्तः_spaces, padchar);
    }
#यदि_परिभाषितम् HAVE_WIDECHAR
mbsalign_cleanup:
#यदि_अन्तः
  मुक्त (str_wc);
  मुक्त (newstr);

  प्रत्यागम ret;
}
