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
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <stdio.h>
#ضمن <stdbool.h>
#ضمن <limits.h>
#ضمن <ctype.h>

#ضمن "c.h"
#ضمن "mbsalign.h"
#ضمن "strutils.h"
#ضمن "widechar.h"

/* Return the byte after a complete ANSI escape sequence, or NULL. */
ساكن ثابت حرف *skip_ansi_escape(ثابت حرف *p, ثابت حرف *پای_ټکی)
{
	ثابت حرف *q;

	إذا (p >= پای_ټکی || (بینښه حرف) *p != '\033' || p + 1 >= پای_ټکی)
		أرجع عدم;

	q = p + 1;
	إذا (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		طالما (q < پای_ټکی && (بینښه حرف) *q >= 0x30
				&& (بینښه حرف) *q <= 0x3f)
			q++;
		طالما (q < پای_ټکی && (بینښه حرف) *q >= 0x20
				&& (بینښه حرف) *q <= 0x2f)
			q++;
		إذا (q < پای_ټکی && (بینښه حرف) *q >= 0x40
				&& (بینښه حرف) *q <= 0x7e)
			أرجع q + 1;
	} وإلا إذا (*q && strchr("()*+", *q) && q + 1 < پای_ټکی)
		أرجع q + 2;

	أرجع عدم;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
size_t mbs_nwidth(ثابت حرف *buf, size_t bufsz)
{
	ثابت حرف *p = buf, *وروستی = buf;
	size_t پلنوالی = 0;

#که_تعریف_شوی HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, حجم(st));
#د_که_پای
	إذا (p && *p && bufsz)
		وروستی = p + (bufsz - 1);

	طالما (p && *p && p <= وروستی) {
		إذا (*p == '\033') {
			ثابت حرف *e = skip_ansi_escape(p, وروستی + 1);

			p = e ? e : p + 1;
			استمر;
		}
		إذا (iscntrl((بینښه حرف) *p)) {
			p++;
			استمر;
		}
#که_تعریف_شوی HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		إذا (len == 0)
			اكسر;
		إذا (len > 0 && iswprint(wc)) {
			صحيح x = wcwidth(wc);
			إذا (x > 0)
				پلنوالی += x;
		} وإلا إذا (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#وإلا
		إذا (isprint((بینښه حرف) *p))
			پلنوالی++;
		p++;
#د_که_پای
	}

	أرجع پلنوالی;
}

size_t mbs_پلنوالی(ثابت حرف *s)
{
	إذا (!s || !*s)
		أرجع 0;
	أرجع mbs_nwidth(s, مزي_اوږدوالی(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
size_t mbs_خوندي_nwidth(ثابت حرف *buf, size_t bufsz, size_t *sz)
{
	ثابت حرف *p = buf, *وروستی = buf;
	size_t پلنوالی = 0, بایټونه = 0;

#که_تعریف_شوی HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, حجم(st));
#د_که_پای
	إذا (p && *p && bufsz)
		وروستی = p + (bufsz - 1);

	طالما (p && *p && p <= وروستی) {
		إذا ((p < وروستی && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((بینښه حرف) *p)) {
			پلنوالی += 4, بایټونه += 4;		/* *p encoded to \x?? */
			p++;
		}
#که_تعریف_شوی HAVE_WIDECHAR
		وإلا {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			إذا (len == 0)
				اكسر;

			إذا (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				إذا (isprint((بینښه حرف) *p))
					پلنوالی += 1, بایټونه += 1;
				وإلا
					پلنوالی += 4, بایټونه += 4;

			} وإلا إذا (!iswprint(wc)) {
				پلنوالی += len * 4;	/* hex encode whole sequence */
				بایټونه += len * 4;
			} وإلا {
				پلنوالی += wcwidth(wc);	/* number of cells */
				بایټونه += len;		/* number of bytes */
			}
			p += len;
		}
#وإلا
		وإلا إذا (!isprint((بینښه حرف) *p)) {
			پلنوالی += 4, بایټونه += 4;		/* *p encoded to \x?? */
			p++;
		} وإلا {
			پلنوالی++, بایټونه++;
			p++;
		}
#د_که_پای
	}

	إذا (sz)
		*sz = بایټونه;
	أرجع پلنوالی;
}

size_t mbs_خوندي_پلنوالی(ثابت حرف *s)
{
	إذا (!s || !*s)
		أرجع 0;
	أرجع mbs_خوندي_nwidth(s, مزي_اوږدوالی(s), عدم);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
حرف *mbs_خوندي_encode_منزل_بفر(ثابت حرف *s, size_t *پلنوالی, حرف *buf, size_t bufsiz, ثابت حرف *safechars)
{
	ثابت حرف *p = s;
	حرف *r;
	صحيح rsz;
	size_t sz = s ? مزي_اوږدوالی(s) : 0;

#که_تعریف_شوی HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, حجم(st));
#د_که_پای
	إذا (!sz || !buf || !bufsiz)
		أرجع عدم;

	r = buf;
	rsz = (صحيح) bufsiz;
	*پلنوالی = 0;

	طالما (p && *p) {
		إذا (safechars && strchr(safechars, *p)) {
			إذا (rsz < 2)
				اكسر;
			*r++ = *p++;
			rsz--;
			استمر;
		}

		إذا ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((بینښه حرف) *p)) {
			إذا (rsz < 5)
				اكسر;
			snprintf(r, rsz, "\\x%02x", (بینښه حرف) *p);
			r += 4;
			rsz -= 4;
			*پلنوالی += 4;
			p++;
		}
#که_تعریف_شوی HAVE_WIDECHAR
		وإلا {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			إذا (len == 0)
				اكسر;		/* end of string */

			إذا (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				إذا (!isprint((بینښه حرف) *p)) {
					إذا (rsz < 5)
						اكسر;
					snprintf(r, rsz, "\\x%02x", (بینښه حرف) *p);
					r += 4;
					rsz -= 4;
					*پلنوالی += 4;
				} وإلا {
					إذا (rsz < 2)
						اكسر;
					(*پلنوالی)++;
					*r++ = *p;
					rsz--;
				}
			} وإلا إذا (!iswprint(wc)) {
				size_t i;
				لكل (i = 0; i < len; i++) {
					إذا (rsz < 5)
						اكسر;
					snprintf(r, rsz, "\\x%02x", (بینښه حرف) p[i]);
					r += 4;
					rsz -= 4;
					*پلنوالی += 4;
				}
				إذا (i < len)
					اكسر;
			} وإلا {
				إذا (rsz < (صحيح)len + 1)
					اكسر;
				حافظه_نقل(r, p, len);
				r += len;
				rsz -= len;
				*پلنوالی += wcwidth(wc);
			}
			p += len;
		}
#وإلا
		وإلا إذا (!isprint((بینښه حرف) *p)) {
			إذا (rsz < 5)
				اكسر;
			snprintf(r, rsz, "\\x%02x", (بینښه حرف) *p);
			p++;
			r += 4;
			rsz -= 4;
			*پلنوالی += 4;
		} وإلا {
			إذا (rsz < 2)
				اكسر;
			*r++ = *p++;
			rsz--;
			(*پلنوالی)++;
		}
#د_که_پای
	}

	*r = '\0';
	أرجع buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
حرف *mbs_invalid_encode_منزل_بفر(ثابت حرف *s, size_t *پلنوالی, حرف *buf, size_t bufsiz)
{
	ثابت حرف *p = s;
	حرف *r;
	صحيح rsz;
	size_t sz = s ? مزي_اوږدوالی(s) : 0;

#که_تعریف_شوی HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, حجم(st));
#د_که_پای
	إذا (!sz || !buf || !bufsiz)
		أرجع عدم;

	r = buf;
	rsz = (صحيح) bufsiz;
	*پلنوالی = 0;

	طالما (p && *p) {
#که_تعریف_شوی HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#وإلا
		size_t len = 1;
#د_که_پای

		إذا (len == 0)
			اكسر;		/* end of string */

		إذا (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			إذا (!isprint((بینښه حرف) *p)) {
				إذا (rsz < 5)
					اكسر;
				snprintf(r, rsz, "\\x%02x", (بینښه حرف) *p);
				r += 4;
				rsz -= 4;
				*پلنوالی += 4;
			} وإلا {
				إذا (rsz < 2)
					اكسر;
				(*پلنوالی)++;
				*r++ = *p;
				rsz--;
			}
		} وإلا إذا (*p == '\\' && *(p + 1) == 'x') {
			إذا (rsz < 5)
				اكسر;
			snprintf(r, rsz, "\\x%02x", (بینښه حرف) *p);
			r += 4;
			rsz -= 4;
			*پلنوالی += 4;
		} وإلا {
			إذا (rsz < (صحيح)len + 1)
				اكسر;
			r = mempcpy(r, p, len);
			rsz -= len;
			*پلنوالی += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	أرجع buf;
}

/*
 * Guess size
 */
size_t mbs_خوندي_encode_اندازه(size_t بایټونه)
{
	أرجع (بایټونه * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_خوندي_decode_اندازه(ثابت حرف *p)
{
	size_t بایټونه = 0;

	طالما (p && *p) {
		إذا (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		وإلا
			p++;
		بایټونه++;
	}
	أرجع بایټونه;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
حرف *mbs_خوندي_encode(ثابت حرف *s, size_t *پلنوالی)
{
	size_t sz = s ? مزي_اوږدوالی(s) : 0;
	size_t bufsz;
	حرف *buf, *ret = عدم;

	إذا (!sz)
		أرجع عدم;
	bufsz = mbs_خوندي_encode_اندازه(sz);
	buf = حافظه_ورکړه(bufsz);
	إذا (buf)
		ret = mbs_خوندي_encode_منزل_بفر(s, پلنوالی, buf, bufsz, عدم);
	إذا (!ret)
		حرر(buf);
	أرجع ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
حرف *mbs_invalid_encode(ثابت حرف *s, size_t *پلنوالی)
{
	size_t sz = s ? مزي_اوږدوالی(s) : 0;
	size_t bufsz;
	حرف *buf, *ret = عدم;

	إذا (!sz)
		أرجع عدم;
	bufsz = mbs_خوندي_encode_اندازه(sz);
	buf = حافظه_ورکړه(bufsz);
	إذا (buf)
		ret = mbs_invalid_encode_منزل_بفر(s, پلنوالی, buf, bufsz);
	إذا (!ret)
		حرر(buf);
	أرجع ret;
}

#که_تعریف_شوی HAVE_WIDECHAR

ساكن bool
wc_ensure_printable (wchar_t *wchars)
{
  bool replaced = false;
  wchar_t *wc = wchars;
  طالما (*wc)
    {
      إذا (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  أرجع replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
ساكن wchar_t *
wc_skip_ansi_escape (wchar_t *wc)
{
  wchar_t *p = wc + 1;

  إذا (*wc != L'\033' || !*p)
    أرجع عدم;

  إذا (*p == L'[')
    {
      p++;
      طالما (*p >= 0x30 && *p <= 0x3f)
        p++;
      طالما (*p >= 0x20 && *p <= 0x2f)
        p++;
      إذا (*p >= 0x40 && *p <= 0x7e)
        أرجع p + 1;
    }
  وإلا إذا ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    أرجع p + 2;

  أرجع عدم;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

ساكن size_t
wc_truncate (wchar_t *wc, size_t پلنوالی)
{
  size_t cells = 0;
  صحيح بل_cells = 0;

  طالما (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      إذا (after_escape)
        {
          wc = after_escape;
          استمر;
        }

      بل_cells = wcwidth (*wc);
      إذا (بل_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          بل_cells = 1;
        }
      إذا (cells + بل_cells > پلنوالی)
        اكسر;

      cells += بل_cells;
      wc++;
    }
  *wc = L'\0';
  أرجع cells;
}

ساكن صحيح
rpl_wcswidth (ثابت wchar_t *s, size_t n)
{
  صحيح ret = 0;

  طالما (n-- > 0 && *s != L'\0')
    {
      صحيح nwidth = wcwidth (*s++);
      إذا (nwidth == -1)             /* non printable */
        أرجع -1;
      إذا (ret > (لوی_عدد - nwidth)) /*ډکېدنه*/
        أرجع -1;
      ret += nwidth;
    }

  أرجع ret;
}
#د_که_پای /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
size_t
mbs_truncate(حرف *str, size_t *پلنوالی)
{
	ssize_t بایټونه = مزي_اوږدوالی(str);
#که_تعریف_شوی HAVE_WIDECHAR
	ssize_t sz = mbstowcs(عدم, str, 0);
	wchar_t *wcs = عدم;

	إذا (sz == (ssize_t) -1)
		اذهب بشپړ;

	wcs = حافظه_صفر_ورکړه(1, (sz + 1) * حجم(wchar_t));
	إذا (!wcs)
		اذهب بشپړ;

	إذا (!mbstowcs(wcs, str, sz))
		اذهب بشپړ;
	*پلنوالی = wc_truncate(wcs, *پلنوالی);
	بایټونه = wcstombs(str, wcs, بایټونه);
بشپړ:
	حرر(wcs);
#وإلا
	{
		ثابت حرف *p = str;
		ثابت حرف *پای_ټکی = str + بایټونه;
		size_t cells = 0;

		طالما (p < پای_ټکی) {
			ثابت حرف *after_escape = skip_ansi_escape(p, پای_ټکی);

			إذا (after_escape) {
				p = after_escape;
				استمر;
			}
			إذا (cells >= *پلنوالی)
				اكسر;
			p++;
			cells++;
		}
		بایټونه = p - str;
		*پلنوالی = cells;
	}
#د_که_پای
	إذا (بایټونه >= 0)
		str[بایټونه] = '\0';
	أرجع بایټونه;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

ساكن حرف*
mbs_align_pad (حرف *dest, ثابت حرف* dest_پای_ټکی, size_t n_spaces, صحيح padchar)
{
  لكل (/* nothing */; n_spaces && (dest < dest_پای_ټکی); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  أرجع dest;
}

size_t
mbsalign (ثابت حرف *src, حرف *dest, size_t dest_اندازه,
          size_t *پلنوالی, mbs_align_t align, صحيح flags)
{
	أرجع mbsalign_with_padding(src, dest, dest_اندازه, پلنوالی, align, flags, ' ');
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
mbsalign_with_padding (ثابت حرف *src, حرف *dest, size_t dest_اندازه,
	               size_t *پلنوالی, mbs_align_t align,
#که_تعریف_شوی HAVE_WIDECHAR
		       صحيح flags,
#وإلا
		       صحيح flags __attribute__((__unused__)),
#د_که_پای
		       صحيح padchar)
{
  size_t ret = -1;
  size_t src_اندازه = مزي_اوږدوالی (src) + 1;
  حرف *newstr = عدم;
  wchar_t *str_wc = عدم;
  ثابت حرف *str_منزل_print = src;
  size_t n_ستون_شمېر = src_اندازه - 1;
  size_t n_کارول_شوی_بایټونه = n_ستون_شمېر; /* Not including NUL */
  size_t n_spaces = 0, space_کیڼ;

#که_تعریف_شوی HAVE_WIDECHAR
  bool conversion = false;
  bool wc_enabled = false;

  /* In multi-byte locales convert to wide characters
     to allow easy truncation. Also determine number
     of screen columns used.  */
  إذا (MB_CUR_MAX > 1)
    {
      size_t src_chars = mbstowcs (عدم, src, 0);
      إذا (src_chars == (size_t) -1)
        {
          إذا (flags & MBA_UNIBYTE_FALLBACK)
            اذهب mbsalign_unibyte;
          وإلا
            اذهب mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = حافظه_ورکړه (src_chars * حجم (wchar_t));
      إذا (str_wc == عدم)
        {
          إذا (flags & MBA_UNIBYTE_FALLBACK)
            اذهب mbsalign_unibyte;
          وإلا
            اذهب mbsalign_cleanup;
        }
      إذا (mbstowcs (str_wc, src, src_chars) != 0)
        {
          str_wc[src_chars - 1] = L'\0';
          wc_enabled = true;
          conversion = wc_ensure_printable (str_wc);
          n_ستون_شمېر = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  إذا (wc_enabled && (conversion || (n_ستون_شمېر > *پلنوالی)))
    {
        إذا (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_اندازه = wcstombs(عدم, str_wc, 0) + 1;
          }
        newstr = حافظه_ورکړه (src_اندازه);
        إذا (newstr == عدم)
        {
          إذا (flags & MBA_UNIBYTE_FALLBACK)
            اذهب mbsalign_unibyte;
          وإلا
            اذهب mbsalign_cleanup;
        }
        str_منزل_print = newstr;
        n_ستون_شمېر = wc_truncate (str_wc, *پلنوالی);
        n_کارول_شوی_بایټونه = wcstombs (newstr, str_wc, src_اندازه);
    }

mbsalign_unibyte:
#د_که_پای

  إذا (n_ستون_شمېر > *پلنوالی) /* Unibyte truncation required.  */
    {
      n_ستون_شمېر = *پلنوالی;
      n_کارول_شوی_بایټونه = n_ستون_شمېر;
    }

  إذا (*پلنوالی > n_ستون_شمېر) /* Padding required.  */
    n_spaces = *پلنوالی - n_ستون_شمېر;

  /* indicate to caller how many cells needed (not including padding).  */
  *پلنوالی = n_ستون_شمېر;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_کارول_شوی_بایټونه + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  إذا (dest_اندازه != 0)
    {
      حرف *dest_پای_ټکی = dest + dest_اندازه - 1;
      size_t پیل_spaces;
      size_t پای_ټکی_spaces;

      بدل (align)
        {
        حالة MBS_ALIGN_CENTER:
          پیل_spaces = n_spaces / 2 + n_spaces % 2;
          پای_ټکی_spaces = n_spaces / 2;
          اكسر;
        حالة MBS_ALIGN_کیڼ:
          پیل_spaces = 0;
          پای_ټکی_spaces = n_spaces;
          اكسر;
        حالة MBS_ALIGN_ښی:
          پیل_spaces = n_spaces;
          پای_ټکی_spaces = 0;
          اكسر;
	افتراضي:
	  abort();
        }

      dest = mbs_align_pad (dest, dest_پای_ټکی, پیل_spaces, padchar);
      space_کیڼ = dest_پای_ټکی - dest;
      dest = mempcpy (dest, str_منزل_print, min (n_کارول_شوی_بایټونه, space_کیڼ));
      mbs_align_pad (dest, dest_پای_ټکی, پای_ټکی_spaces, padchar);
    }
#که_تعریف_شوی HAVE_WIDECHAR
mbsalign_cleanup:
#د_که_پای
  حرر (str_wc);
  حرر (newstr);

  أرجع ret;
}
