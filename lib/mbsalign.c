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
#شامل <stdlib.h>
#شامل <string.h>
#شامل <stdio.h>
#شامل <stdbool.h>
#شامل <limits.h>
#شامل <ctype.h>

#شامل "c.h"
#شامل "mbsalign.h"
#شامل "strutils.h"
#شامل "widechar.h"

/* Return the byte after a complete ANSI escape sequence, or NULL. */
ساکن ثابت حرف *skip_ansi_escape(ثابت حرف *p, ثابت حرف *انتہا)
{
	ثابت حرف *q;

	اگر (p >= انتہا || (غیرعلامتی حرف) *p != '\033' || p + 1 >= انتہا)
		واپس تہی;

	q = p + 1;
	اگر (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		جب (q < انتہا && (غیرعلامتی حرف) *q >= 0x30
				&& (غیرعلامتی حرف) *q <= 0x3f)
			q++;
		جب (q < انتہا && (غیرعلامتی حرف) *q >= 0x20
				&& (غیرعلامتی حرف) *q <= 0x2f)
			q++;
		اگر (q < انتہا && (غیرعلامتی حرف) *q >= 0x40
				&& (غیرعلامتی حرف) *q <= 0x7e)
			واپس q + 1;
	} ورنہ اگر (*q && strchr("()*+", *q) && q + 1 < انتہا)
		واپس q + 2;

	واپس تہی;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
size_t mbs_nwidth(ثابت حرف *buf, size_t bufsz)
{
	ثابت حرف *p = buf, *آخری = buf;
	size_t چوڑائی = 0;

#اگر_تعریف_شدہ HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, حجم(st));
#اگر_ختم
	اگر (p && *p && bufsz)
		آخری = p + (bufsz - 1);

	جب (p && *p && p <= آخری) {
		اگر (*p == '\033') {
			ثابت حرف *e = skip_ansi_escape(p, آخری + 1);

			p = e ? e : p + 1;
			جاری;
		}
		اگر (iscntrl((غیرعلامتی حرف) *p)) {
			p++;
			جاری;
		}
#اگر_تعریف_شدہ HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		اگر (len == 0)
			توڑو;
		اگر (len > 0 && iswprint(wc)) {
			صحیح x = wcwidth(wc);
			اگر (x > 0)
				چوڑائی += x;
		} ورنہ اگر (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#ورنہ
		اگر (isprint((غیرعلامتی حرف) *p))
			چوڑائی++;
		p++;
#اگر_ختم
	}

	واپس چوڑائی;
}

size_t mbs_چوڑائی(ثابت حرف *s)
{
	اگر (!s || !*s)
		واپس 0;
	واپس mbs_nwidth(s, لڑی_لمبائی(s));
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
	ثابت حرف *p = buf, *آخری = buf;
	size_t چوڑائی = 0, بائٹیں = 0;

#اگر_تعریف_شدہ HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, حجم(st));
#اگر_ختم
	اگر (p && *p && bufsz)
		آخری = p + (bufsz - 1);

	جب (p && *p && p <= آخری) {
		اگر ((p < آخری && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((غیرعلامتی حرف) *p)) {
			چوڑائی += 4, بائٹیں += 4;		/* *p encoded to \x?? */
			p++;
		}
#اگر_تعریف_شدہ HAVE_WIDECHAR
		ورنہ {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			اگر (len == 0)
				توڑو;

			اگر (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				اگر (isprint((غیرعلامتی حرف) *p))
					چوڑائی += 1, بائٹیں += 1;
				ورنہ
					چوڑائی += 4, بائٹیں += 4;

			} ورنہ اگر (!iswprint(wc)) {
				چوڑائی += len * 4;	/* hex encode whole sequence */
				بائٹیں += len * 4;
			} ورنہ {
				چوڑائی += wcwidth(wc);	/* number of cells */
				بائٹیں += len;		/* number of bytes */
			}
			p += len;
		}
#ورنہ
		ورنہ اگر (!isprint((غیرعلامتی حرف) *p)) {
			چوڑائی += 4, بائٹیں += 4;		/* *p encoded to \x?? */
			p++;
		} ورنہ {
			چوڑائی++, بائٹیں++;
			p++;
		}
#اگر_ختم
	}

	اگر (sz)
		*sz = بائٹیں;
	واپس چوڑائی;
}

size_t mbs_محفوظ_چوڑائی(ثابت حرف *s)
{
	اگر (!s || !*s)
		واپس 0;
	واپس mbs_محفوظ_nwidth(s, لڑی_لمبائی(s), تہی);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
حرف *mbs_محفوظ_encode_مقصد_بفر(ثابت حرف *s, size_t *چوڑائی, حرف *buf, size_t bufsiz, ثابت حرف *safechars)
{
	ثابت حرف *p = s;
	حرف *r;
	صحیح rsz;
	size_t sz = s ? لڑی_لمبائی(s) : 0;

#اگر_تعریف_شدہ HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, حجم(st));
#اگر_ختم
	اگر (!sz || !buf || !bufsiz)
		واپس تہی;

	r = buf;
	rsz = (صحیح) bufsiz;
	*چوڑائی = 0;

	جب (p && *p) {
		اگر (safechars && strchr(safechars, *p)) {
			اگر (rsz < 2)
				توڑو;
			*r++ = *p++;
			rsz--;
			جاری;
		}

		اگر ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((غیرعلامتی حرف) *p)) {
			اگر (rsz < 5)
				توڑو;
			snprintf(r, rsz, "\\x%02x", (غیرعلامتی حرف) *p);
			r += 4;
			rsz -= 4;
			*چوڑائی += 4;
			p++;
		}
#اگر_تعریف_شدہ HAVE_WIDECHAR
		ورنہ {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			اگر (len == 0)
				توڑو;		/* end of string */

			اگر (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				اگر (!isprint((غیرعلامتی حرف) *p)) {
					اگر (rsz < 5)
						توڑو;
					snprintf(r, rsz, "\\x%02x", (غیرعلامتی حرف) *p);
					r += 4;
					rsz -= 4;
					*چوڑائی += 4;
				} ورنہ {
					اگر (rsz < 2)
						توڑو;
					(*چوڑائی)++;
					*r++ = *p;
					rsz--;
				}
			} ورنہ اگر (!iswprint(wc)) {
				size_t i;
				برائے (i = 0; i < len; i++) {
					اگر (rsz < 5)
						توڑو;
					snprintf(r, rsz, "\\x%02x", (غیرعلامتی حرف) p[i]);
					r += 4;
					rsz -= 4;
					*چوڑائی += 4;
				}
				اگر (i < len)
					توڑو;
			} ورنہ {
				اگر (rsz < (صحیح)len + 1)
					توڑو;
				حافظہ_نقل(r, p, len);
				r += len;
				rsz -= len;
				*چوڑائی += wcwidth(wc);
			}
			p += len;
		}
#ورنہ
		ورنہ اگر (!isprint((غیرعلامتی حرف) *p)) {
			اگر (rsz < 5)
				توڑو;
			snprintf(r, rsz, "\\x%02x", (غیرعلامتی حرف) *p);
			p++;
			r += 4;
			rsz -= 4;
			*چوڑائی += 4;
		} ورنہ {
			اگر (rsz < 2)
				توڑو;
			*r++ = *p++;
			rsz--;
			(*چوڑائی)++;
		}
#اگر_ختم
	}

	*r = '\0';
	واپس buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
حرف *mbs_invalid_encode_مقصد_بفر(ثابت حرف *s, size_t *چوڑائی, حرف *buf, size_t bufsiz)
{
	ثابت حرف *p = s;
	حرف *r;
	صحیح rsz;
	size_t sz = s ? لڑی_لمبائی(s) : 0;

#اگر_تعریف_شدہ HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, حجم(st));
#اگر_ختم
	اگر (!sz || !buf || !bufsiz)
		واپس تہی;

	r = buf;
	rsz = (صحیح) bufsiz;
	*چوڑائی = 0;

	جب (p && *p) {
#اگر_تعریف_شدہ HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#ورنہ
		size_t len = 1;
#اگر_ختم

		اگر (len == 0)
			توڑو;		/* end of string */

		اگر (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			اگر (!isprint((غیرعلامتی حرف) *p)) {
				اگر (rsz < 5)
					توڑو;
				snprintf(r, rsz, "\\x%02x", (غیرعلامتی حرف) *p);
				r += 4;
				rsz -= 4;
				*چوڑائی += 4;
			} ورنہ {
				اگر (rsz < 2)
					توڑو;
				(*چوڑائی)++;
				*r++ = *p;
				rsz--;
			}
		} ورنہ اگر (*p == '\\' && *(p + 1) == 'x') {
			اگر (rsz < 5)
				توڑو;
			snprintf(r, rsz, "\\x%02x", (غیرعلامتی حرف) *p);
			r += 4;
			rsz -= 4;
			*چوڑائی += 4;
		} ورنہ {
			اگر (rsz < (صحیح)len + 1)
				توڑو;
			r = mempcpy(r, p, len);
			rsz -= len;
			*چوڑائی += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	واپس buf;
}

/*
 * Guess size
 */
size_t mbs_محفوظ_encode_سائز(size_t بائٹیں)
{
	واپس (بائٹیں * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_محفوظ_decode_سائز(ثابت حرف *p)
{
	size_t بائٹیں = 0;

	جب (p && *p) {
		اگر (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		ورنہ
			p++;
		بائٹیں++;
	}
	واپس بائٹیں;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
حرف *mbs_محفوظ_encode(ثابت حرف *s, size_t *چوڑائی)
{
	size_t sz = s ? لڑی_لمبائی(s) : 0;
	size_t bufsz;
	حرف *buf, *ret = تہی;

	اگر (!sz)
		واپس تہی;
	bufsz = mbs_محفوظ_encode_سائز(sz);
	buf = حافظہ_دو(bufsz);
	اگر (buf)
		ret = mbs_محفوظ_encode_مقصد_بفر(s, چوڑائی, buf, bufsz, تہی);
	اگر (!ret)
		آزاد(buf);
	واپس ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
حرف *mbs_invalid_encode(ثابت حرف *s, size_t *چوڑائی)
{
	size_t sz = s ? لڑی_لمبائی(s) : 0;
	size_t bufsz;
	حرف *buf, *ret = تہی;

	اگر (!sz)
		واپس تہی;
	bufsz = mbs_محفوظ_encode_سائز(sz);
	buf = حافظہ_دو(bufsz);
	اگر (buf)
		ret = mbs_invalid_encode_مقصد_بفر(s, چوڑائی, buf, bufsz);
	اگر (!ret)
		آزاد(buf);
	واپس ret;
}

#اگر_تعریف_شدہ HAVE_WIDECHAR

ساکن bool
wc_ensure_printable (wchar_t *wchars)
{
  bool replaced = false;
  wchar_t *wc = wchars;
  جب (*wc)
    {
      اگر (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  واپس replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
ساکن wchar_t *
wc_skip_ansi_escape (wchar_t *wc)
{
  wchar_t *p = wc + 1;

  اگر (*wc != L'\033' || !*p)
    واپس تہی;

  اگر (*p == L'[')
    {
      p++;
      جب (*p >= 0x30 && *p <= 0x3f)
        p++;
      جب (*p >= 0x20 && *p <= 0x2f)
        p++;
      اگر (*p >= 0x40 && *p <= 0x7e)
        واپس p + 1;
    }
  ورنہ اگر ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    واپس p + 2;

  واپس تہی;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

ساکن size_t
wc_truncate (wchar_t *wc, size_t چوڑائی)
{
  size_t cells = 0;
  صحیح اگلا_cells = 0;

  جب (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      اگر (after_escape)
        {
          wc = after_escape;
          جاری;
        }

      اگلا_cells = wcwidth (*wc);
      اگر (اگلا_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          اگلا_cells = 1;
        }
      اگر (cells + اگلا_cells > چوڑائی)
        توڑو;

      cells += اگلا_cells;
      wc++;
    }
  *wc = L'\0';
  واپس cells;
}

ساکن صحیح
rpl_wcswidth (ثابت wchar_t *s, size_t n)
{
  صحیح ret = 0;

  جب (n-- > 0 && *s != L'\0')
    {
      صحیح nwidth = wcwidth (*s++);
      اگر (nwidth == -1)             /* non printable */
        واپس -1;
      اگر (ret > (زیادہ_سے_زیادہ_عدد - nwidth)) /*حد_سے_تجاوز*/
        واپس -1;
      ret += nwidth;
    }

  واپس ret;
}
#اگر_ختم /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
size_t
mbs_truncate(حرف *str, size_t *چوڑائی)
{
	ssize_t بائٹیں = لڑی_لمبائی(str);
#اگر_تعریف_شدہ HAVE_WIDECHAR
	ssize_t sz = mbstowcs(تہی, str, 0);
	wchar_t *wcs = تہی;

	اگر (sz == (ssize_t) -1)
		جاؤ مکمل;

	wcs = صفر_حافظہ_دو(1, (sz + 1) * حجم(wchar_t));
	اگر (!wcs)
		جاؤ مکمل;

	اگر (!mbstowcs(wcs, str, sz))
		جاؤ مکمل;
	*چوڑائی = wc_truncate(wcs, *چوڑائی);
	بائٹیں = wcstombs(str, wcs, بائٹیں);
مکمل:
	آزاد(wcs);
#ورنہ
	{
		ثابت حرف *p = str;
		ثابت حرف *انتہا = str + بائٹیں;
		size_t cells = 0;

		جب (p < انتہا) {
			ثابت حرف *after_escape = skip_ansi_escape(p, انتہا);

			اگر (after_escape) {
				p = after_escape;
				جاری;
			}
			اگر (cells >= *چوڑائی)
				توڑو;
			p++;
			cells++;
		}
		بائٹیں = p - str;
		*چوڑائی = cells;
	}
#اگر_ختم
	اگر (بائٹیں >= 0)
		str[بائٹیں] = '\0';
	واپس بائٹیں;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

ساکن حرف*
mbs_align_pad (حرف *dest, ثابت حرف* dest_انتہا, size_t n_spaces, صحیح padchar)
{
  برائے (/* nothing */; n_spaces && (dest < dest_انتہا); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  واپس dest;
}

size_t
mbsalign (ثابت حرف *src, حرف *dest, size_t dest_سائز,
          size_t *چوڑائی, mbs_align_t align, صحیح flags)
{
	واپس mbsalign_with_padding(src, dest, dest_سائز, چوڑائی, align, flags, ' ');
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
mbsalign_with_padding (ثابت حرف *src, حرف *dest, size_t dest_سائز,
	               size_t *چوڑائی, mbs_align_t align,
#اگر_تعریف_شدہ HAVE_WIDECHAR
		       صحیح flags,
#ورنہ
		       صحیح flags __attribute__((__unused__)),
#اگر_ختم
		       صحیح padchar)
{
  size_t ret = -1;
  size_t src_سائز = لڑی_لمبائی (src) + 1;
  حرف *newstr = تہی;
  wchar_t *str_wc = تہی;
  ثابت حرف *str_مقصد_print = src;
  size_t n_ستون_تعداد = src_سائز - 1;
  size_t n_مستعمل_بائٹیں = n_ستون_تعداد; /* Not including NUL */
  size_t n_spaces = 0, space_بایاں;

#اگر_تعریف_شدہ HAVE_WIDECHAR
  bool conversion = false;
  bool wc_enabled = false;

  /* In multi-byte locales convert to wide characters
     to allow easy truncation. Also determine number
     of screen columns used.  */
  اگر (MB_CUR_MAX > 1)
    {
      size_t src_chars = mbstowcs (تہی, src, 0);
      اگر (src_chars == (size_t) -1)
        {
          اگر (flags & MBA_UNIBYTE_FALLBACK)
            جاؤ mbsalign_unibyte;
          ورنہ
            جاؤ mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = حافظہ_دو (src_chars * حجم (wchar_t));
      اگر (str_wc == تہی)
        {
          اگر (flags & MBA_UNIBYTE_FALLBACK)
            جاؤ mbsalign_unibyte;
          ورنہ
            جاؤ mbsalign_cleanup;
        }
      اگر (mbstowcs (str_wc, src, src_chars) != 0)
        {
          str_wc[src_chars - 1] = L'\0';
          wc_enabled = true;
          conversion = wc_ensure_printable (str_wc);
          n_ستون_تعداد = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  اگر (wc_enabled && (conversion || (n_ستون_تعداد > *چوڑائی)))
    {
        اگر (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_سائز = wcstombs(تہی, str_wc, 0) + 1;
          }
        newstr = حافظہ_دو (src_سائز);
        اگر (newstr == تہی)
        {
          اگر (flags & MBA_UNIBYTE_FALLBACK)
            جاؤ mbsalign_unibyte;
          ورنہ
            جاؤ mbsalign_cleanup;
        }
        str_مقصد_print = newstr;
        n_ستون_تعداد = wc_truncate (str_wc, *چوڑائی);
        n_مستعمل_بائٹیں = wcstombs (newstr, str_wc, src_سائز);
    }

mbsalign_unibyte:
#اگر_ختم

  اگر (n_ستون_تعداد > *چوڑائی) /* Unibyte truncation required.  */
    {
      n_ستون_تعداد = *چوڑائی;
      n_مستعمل_بائٹیں = n_ستون_تعداد;
    }

  اگر (*چوڑائی > n_ستون_تعداد) /* Padding required.  */
    n_spaces = *چوڑائی - n_ستون_تعداد;

  /* indicate to caller how many cells needed (not including padding).  */
  *چوڑائی = n_ستون_تعداد;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_مستعمل_بائٹیں + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  اگر (dest_سائز != 0)
    {
      حرف *dest_انتہا = dest + dest_سائز - 1;
      size_t آغاز_spaces;
      size_t انتہا_spaces;

      انتخاب (align)
        {
        حالت MBS_ALIGN_CENTER:
          آغاز_spaces = n_spaces / 2 + n_spaces % 2;
          انتہا_spaces = n_spaces / 2;
          توڑو;
        حالت MBS_ALIGN_بایاں:
          آغاز_spaces = 0;
          انتہا_spaces = n_spaces;
          توڑو;
        حالت MBS_ALIGN_دایاں:
          آغاز_spaces = n_spaces;
          انتہا_spaces = 0;
          توڑو;
	طےشدہ:
	  abort();
        }

      dest = mbs_align_pad (dest, dest_انتہا, آغاز_spaces, padchar);
      space_بایاں = dest_انتہا - dest;
      dest = mempcpy (dest, str_مقصد_print, min (n_مستعمل_بائٹیں, space_بایاں));
      mbs_align_pad (dest, dest_انتہا, انتہا_spaces, padchar);
    }
#اگر_تعریف_شدہ HAVE_WIDECHAR
mbsalign_cleanup:
#اگر_ختم
  آزاد (str_wc);
  آزاد (newstr);

  واپس ret;
}
