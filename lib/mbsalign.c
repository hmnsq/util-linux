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
ساكن ثابت حرف *skip_ansi_escape(ثابت حرف *p, ثابت حرف *پڇاڙي)
{
	ثابت حرف *q;

	إذا (p >= پڇاڙي || (بنانشان حرف) *p != '\033' || p + 1 >= پڇاڙي)
		أرجع عدم;

	q = p + 1;
	إذا (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		طالما (q < پڇاڙي && (بنانشان حرف) *q >= 0x30
				&& (بنانشان حرف) *q <= 0x3f)
			q++;
		طالما (q < پڇاڙي && (بنانشان حرف) *q >= 0x20
				&& (بنانشان حرف) *q <= 0x2f)
			q++;
		إذا (q < پڇاڙي && (بنانشان حرف) *q >= 0x40
				&& (بنانشان حرف) *q <= 0x7e)
			أرجع q + 1;
	} وإلا إذا (*q && strchr("()*+", *q) && q + 1 < پڇاڙي)
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
	ثابت حرف *p = buf, *آخري = buf;
	size_t ويڪر = 0;

#جيڪڏهن_وصف_ٿيل HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, حجم(st));
#جيڪڏهن_پورو
	إذا (p && *p && bufsz)
		آخري = p + (bufsz - 1);

	طالما (p && *p && p <= آخري) {
		إذا (*p == '\033') {
			ثابت حرف *e = skip_ansi_escape(p, آخري + 1);

			p = e ? e : p + 1;
			استمر;
		}
		إذا (iscntrl((بنانشان حرف) *p)) {
			p++;
			استمر;
		}
#جيڪڏهن_وصف_ٿيل HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		إذا (len == 0)
			اكسر;
		إذا (len > 0 && iswprint(wc)) {
			صحيح x = wcwidth(wc);
			إذا (x > 0)
				ويڪر += x;
		} وإلا إذا (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#وإلا
		إذا (isprint((بنانشان حرف) *p))
			ويڪر++;
		p++;
#جيڪڏهن_پورو
	}

	أرجع ويڪر;
}

size_t mbs_ويڪر(ثابت حرف *s)
{
	إذا (!s || !*s)
		أرجع 0;
	أرجع mbs_nwidth(s, لڙي_ڊيگهه(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
size_t mbs_محفوظ_nwidth(ثابت حرف *buf, size_t bufsz, size_t *sz)
{
	ثابت حرف *p = buf, *آخري = buf;
	size_t ويڪر = 0, بائيٽون = 0;

#جيڪڏهن_وصف_ٿيل HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, حجم(st));
#جيڪڏهن_پورو
	إذا (p && *p && bufsz)
		آخري = p + (bufsz - 1);

	طالما (p && *p && p <= آخري) {
		إذا ((p < آخري && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((بنانشان حرف) *p)) {
			ويڪر += 4, بائيٽون += 4;		/* *p encoded to \x?? */
			p++;
		}
#جيڪڏهن_وصف_ٿيل HAVE_WIDECHAR
		وإلا {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			إذا (len == 0)
				اكسر;

			إذا (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				إذا (isprint((بنانشان حرف) *p))
					ويڪر += 1, بائيٽون += 1;
				وإلا
					ويڪر += 4, بائيٽون += 4;

			} وإلا إذا (!iswprint(wc)) {
				ويڪر += len * 4;	/* hex encode whole sequence */
				بائيٽون += len * 4;
			} وإلا {
				ويڪر += wcwidth(wc);	/* number of cells */
				بائيٽون += len;		/* number of bytes */
			}
			p += len;
		}
#وإلا
		وإلا إذا (!isprint((بنانشان حرف) *p)) {
			ويڪر += 4, بائيٽون += 4;		/* *p encoded to \x?? */
			p++;
		} وإلا {
			ويڪر++, بائيٽون++;
			p++;
		}
#جيڪڏهن_پورو
	}

	إذا (sz)
		*sz = بائيٽون;
	أرجع ويڪر;
}

size_t mbs_محفوظ_ويڪر(ثابت حرف *s)
{
	إذا (!s || !*s)
		أرجع 0;
	أرجع mbs_محفوظ_nwidth(s, لڙي_ڊيگهه(s), عدم);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
حرف *mbs_محفوظ_encode_منزل_بفر(ثابت حرف *s, size_t *ويڪر, حرف *buf, size_t bufsiz, ثابت حرف *safechars)
{
	ثابت حرف *p = s;
	حرف *r;
	صحيح rsz;
	size_t sz = s ? لڙي_ڊيگهه(s) : 0;

#جيڪڏهن_وصف_ٿيل HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, حجم(st));
#جيڪڏهن_پورو
	إذا (!sz || !buf || !bufsiz)
		أرجع عدم;

	r = buf;
	rsz = (صحيح) bufsiz;
	*ويڪر = 0;

	طالما (p && *p) {
		إذا (safechars && strchr(safechars, *p)) {
			إذا (rsz < 2)
				اكسر;
			*r++ = *p++;
			rsz--;
			استمر;
		}

		إذا ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((بنانشان حرف) *p)) {
			إذا (rsz < 5)
				اكسر;
			snprintf(r, rsz, "\\x%02x", (بنانشان حرف) *p);
			r += 4;
			rsz -= 4;
			*ويڪر += 4;
			p++;
		}
#جيڪڏهن_وصف_ٿيل HAVE_WIDECHAR
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
				إذا (!isprint((بنانشان حرف) *p)) {
					إذا (rsz < 5)
						اكسر;
					snprintf(r, rsz, "\\x%02x", (بنانشان حرف) *p);
					r += 4;
					rsz -= 4;
					*ويڪر += 4;
				} وإلا {
					إذا (rsz < 2)
						اكسر;
					(*ويڪر)++;
					*r++ = *p;
					rsz--;
				}
			} وإلا إذا (!iswprint(wc)) {
				size_t i;
				لكل (i = 0; i < len; i++) {
					إذا (rsz < 5)
						اكسر;
					snprintf(r, rsz, "\\x%02x", (بنانشان حرف) p[i]);
					r += 4;
					rsz -= 4;
					*ويڪر += 4;
				}
				إذا (i < len)
					اكسر;
			} وإلا {
				إذا (rsz < (صحيح)len + 1)
					اكسر;
				يادگيري_نقل(r, p, len);
				r += len;
				rsz -= len;
				*ويڪر += wcwidth(wc);
			}
			p += len;
		}
#وإلا
		وإلا إذا (!isprint((بنانشان حرف) *p)) {
			إذا (rsz < 5)
				اكسر;
			snprintf(r, rsz, "\\x%02x", (بنانشان حرف) *p);
			p++;
			r += 4;
			rsz -= 4;
			*ويڪر += 4;
		} وإلا {
			إذا (rsz < 2)
				اكسر;
			*r++ = *p++;
			rsz--;
			(*ويڪر)++;
		}
#جيڪڏهن_پورو
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
حرف *mbs_invalid_encode_منزل_بفر(ثابت حرف *s, size_t *ويڪر, حرف *buf, size_t bufsiz)
{
	ثابت حرف *p = s;
	حرف *r;
	صحيح rsz;
	size_t sz = s ? لڙي_ڊيگهه(s) : 0;

#جيڪڏهن_وصف_ٿيل HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, حجم(st));
#جيڪڏهن_پورو
	إذا (!sz || !buf || !bufsiz)
		أرجع عدم;

	r = buf;
	rsz = (صحيح) bufsiz;
	*ويڪر = 0;

	طالما (p && *p) {
#جيڪڏهن_وصف_ٿيل HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#وإلا
		size_t len = 1;
#جيڪڏهن_پورو

		إذا (len == 0)
			اكسر;		/* end of string */

		إذا (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			إذا (!isprint((بنانشان حرف) *p)) {
				إذا (rsz < 5)
					اكسر;
				snprintf(r, rsz, "\\x%02x", (بنانشان حرف) *p);
				r += 4;
				rsz -= 4;
				*ويڪر += 4;
			} وإلا {
				إذا (rsz < 2)
					اكسر;
				(*ويڪر)++;
				*r++ = *p;
				rsz--;
			}
		} وإلا إذا (*p == '\\' && *(p + 1) == 'x') {
			إذا (rsz < 5)
				اكسر;
			snprintf(r, rsz, "\\x%02x", (بنانشان حرف) *p);
			r += 4;
			rsz -= 4;
			*ويڪر += 4;
		} وإلا {
			إذا (rsz < (صحيح)len + 1)
				اكسر;
			r = mempcpy(r, p, len);
			rsz -= len;
			*ويڪر += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	أرجع buf;
}

/*
 * Guess size
 */
size_t mbs_محفوظ_encode_سائيز(size_t بائيٽون)
{
	أرجع (بائيٽون * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_محفوظ_decode_سائيز(ثابت حرف *p)
{
	size_t بائيٽون = 0;

	طالما (p && *p) {
		إذا (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		وإلا
			p++;
		بائيٽون++;
	}
	أرجع بائيٽون;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
حرف *mbs_محفوظ_encode(ثابت حرف *s, size_t *ويڪر)
{
	size_t sz = s ? لڙي_ڊيگهه(s) : 0;
	size_t bufsz;
	حرف *buf, *ret = عدم;

	إذا (!sz)
		أرجع عدم;
	bufsz = mbs_محفوظ_encode_سائيز(sz);
	buf = يادگيري_ڏيو(bufsz);
	إذا (buf)
		ret = mbs_محفوظ_encode_منزل_بفر(s, ويڪر, buf, bufsz, عدم);
	إذا (!ret)
		حرر(buf);
	أرجع ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
حرف *mbs_invalid_encode(ثابت حرف *s, size_t *ويڪر)
{
	size_t sz = s ? لڙي_ڊيگهه(s) : 0;
	size_t bufsz;
	حرف *buf, *ret = عدم;

	إذا (!sz)
		أرجع عدم;
	bufsz = mbs_محفوظ_encode_سائيز(sz);
	buf = يادگيري_ڏيو(bufsz);
	إذا (buf)
		ret = mbs_invalid_encode_منزل_بفر(s, ويڪر, buf, bufsz);
	إذا (!ret)
		حرر(buf);
	أرجع ret;
}

#جيڪڏهن_وصف_ٿيل HAVE_WIDECHAR

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
wc_truncate (wchar_t *wc, size_t ويڪر)
{
  size_t cells = 0;
  صحيح ايندڙ_cells = 0;

  طالما (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      إذا (after_escape)
        {
          wc = after_escape;
          استمر;
        }

      ايندڙ_cells = wcwidth (*wc);
      إذا (ايندڙ_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          ايندڙ_cells = 1;
        }
      إذا (cells + ايندڙ_cells > ويڪر)
        اكسر;

      cells += ايندڙ_cells;
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
      إذا (ret > (وڌ_کان_وڌ_عدد - nwidth)) /*حد_کان_مٿي*/
        أرجع -1;
      ret += nwidth;
    }

  أرجع ret;
}
#جيڪڏهن_پورو /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
size_t
mbs_truncate(حرف *str, size_t *ويڪر)
{
	ssize_t بائيٽون = لڙي_ڊيگهه(str);
#جيڪڏهن_وصف_ٿيل HAVE_WIDECHAR
	ssize_t sz = mbstowcs(عدم, str, 0);
	wchar_t *wcs = عدم;

	إذا (sz == (ssize_t) -1)
		اذهب پورو;

	wcs = صفر_يادگيري_ڏيو(1, (sz + 1) * حجم(wchar_t));
	إذا (!wcs)
		اذهب پورو;

	إذا (!mbstowcs(wcs, str, sz))
		اذهب پورو;
	*ويڪر = wc_truncate(wcs, *ويڪر);
	بائيٽون = wcstombs(str, wcs, بائيٽون);
پورو:
	حرر(wcs);
#وإلا
	{
		ثابت حرف *p = str;
		ثابت حرف *پڇاڙي = str + بائيٽون;
		size_t cells = 0;

		طالما (p < پڇاڙي) {
			ثابت حرف *after_escape = skip_ansi_escape(p, پڇاڙي);

			إذا (after_escape) {
				p = after_escape;
				استمر;
			}
			إذا (cells >= *ويڪر)
				اكسر;
			p++;
			cells++;
		}
		بائيٽون = p - str;
		*ويڪر = cells;
	}
#جيڪڏهن_پورو
	إذا (بائيٽون >= 0)
		str[بائيٽون] = '\0';
	أرجع بائيٽون;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

ساكن حرف*
mbs_align_pad (حرف *dest, ثابت حرف* dest_پڇاڙي, size_t n_spaces, صحيح padchar)
{
  لكل (/* nothing */; n_spaces && (dest < dest_پڇاڙي); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  أرجع dest;
}

size_t
mbsalign (ثابت حرف *src, حرف *dest, size_t dest_سائيز,
          size_t *ويڪر, mbs_align_t align, صحيح flags)
{
	أرجع mbsalign_with_padding(src, dest, dest_سائيز, ويڪر, align, flags, ' ');
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
mbsalign_with_padding (ثابت حرف *src, حرف *dest, size_t dest_سائيز,
	               size_t *ويڪر, mbs_align_t align,
#جيڪڏهن_وصف_ٿيل HAVE_WIDECHAR
		       صحيح flags,
#وإلا
		       صحيح flags __attribute__((__unused__)),
#جيڪڏهن_پورو
		       صحيح padchar)
{
  size_t ret = -1;
  size_t src_سائيز = لڙي_ڊيگهه (src) + 1;
  حرف *newstr = عدم;
  wchar_t *str_wc = عدم;
  ثابت حرف *str_منزل_print = src;
  size_t n_کانن_ڳڻپ = src_سائيز - 1;
  size_t n_استعمال_ٿيل_بائيٽون = n_کانن_ڳڻپ; /* Not including NUL */
  size_t n_spaces = 0, space_کاٻو;

#جيڪڏهن_وصف_ٿيل HAVE_WIDECHAR
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
      str_wc = يادگيري_ڏيو (src_chars * حجم (wchar_t));
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
          n_کانن_ڳڻپ = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  إذا (wc_enabled && (conversion || (n_کانن_ڳڻپ > *ويڪر)))
    {
        إذا (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_سائيز = wcstombs(عدم, str_wc, 0) + 1;
          }
        newstr = يادگيري_ڏيو (src_سائيز);
        إذا (newstr == عدم)
        {
          إذا (flags & MBA_UNIBYTE_FALLBACK)
            اذهب mbsalign_unibyte;
          وإلا
            اذهب mbsalign_cleanup;
        }
        str_منزل_print = newstr;
        n_کانن_ڳڻپ = wc_truncate (str_wc, *ويڪر);
        n_استعمال_ٿيل_بائيٽون = wcstombs (newstr, str_wc, src_سائيز);
    }

mbsalign_unibyte:
#جيڪڏهن_پورو

  إذا (n_کانن_ڳڻپ > *ويڪر) /* Unibyte truncation required.  */
    {
      n_کانن_ڳڻپ = *ويڪر;
      n_استعمال_ٿيل_بائيٽون = n_کانن_ڳڻپ;
    }

  إذا (*ويڪر > n_کانن_ڳڻپ) /* Padding required.  */
    n_spaces = *ويڪر - n_کانن_ڳڻپ;

  /* indicate to caller how many cells needed (not including padding).  */
  *ويڪر = n_کانن_ڳڻپ;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_استعمال_ٿيل_بائيٽون + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  إذا (dest_سائيز != 0)
    {
      حرف *dest_پڇاڙي = dest + dest_سائيز - 1;
      size_t شروعات_spaces;
      size_t پڇاڙي_spaces;

      بدل (align)
        {
        حالة MBS_ALIGN_CENTER:
          شروعات_spaces = n_spaces / 2 + n_spaces % 2;
          پڇاڙي_spaces = n_spaces / 2;
          اكسر;
        حالة MBS_ALIGN_کاٻو:
          شروعات_spaces = 0;
          پڇاڙي_spaces = n_spaces;
          اكسر;
        حالة MBS_ALIGN_ساڄو:
          شروعات_spaces = n_spaces;
          پڇاڙي_spaces = 0;
          اكسر;
	افتراضي:
	  abort();
        }

      dest = mbs_align_pad (dest, dest_پڇاڙي, شروعات_spaces, padchar);
      space_کاٻو = dest_پڇاڙي - dest;
      dest = mempcpy (dest, str_منزل_print, min (n_استعمال_ٿيل_بائيٽون, space_کاٻو));
      mbs_align_pad (dest, dest_پڇاڙي, پڇاڙي_spaces, padchar);
    }
#جيڪڏهن_وصف_ٿيل HAVE_WIDECHAR
mbsalign_cleanup:
#جيڪڏهن_پورو
  حرر (str_wc);
  حرر (newstr);

  أرجع ret;
}
