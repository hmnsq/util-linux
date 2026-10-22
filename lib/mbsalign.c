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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <ctype.h>

#include "c.h"
#include "mbsalign.h"
#include "strutils.h"
#include "widechar.h"

/* Return the byte after a complete ANSI escape sequence, or NULL. */
static const var_na *skip_ansi_escape(const var_na *p, const var_na *end)
{
	const var_na *q;

	ᱡᱩᱫᱤ (p >= end || (unsigned var_na) *p != '\033' || p + 1 >= end)
		ᱨᱩᱣᱟᱹᱲ NULL;

	q = p + 1;
	ᱡᱩᱫᱤ (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		ᱡᱚᱠᱷᱚᱱ (q < end && (unsigned var_na) *q >= 0x30
				&& (unsigned var_na) *q <= 0x3f)
			q++;
		ᱡᱚᱠᱷᱚᱱ (q < end && (unsigned var_na) *q >= 0x20
				&& (unsigned var_na) *q <= 0x2f)
			q++;
		ᱡᱩᱫᱤ (q < end && (unsigned var_na) *q >= 0x40
				&& (unsigned var_na) *q <= 0x7e)
			ᱨᱩᱣᱟᱹᱲ q + 1;
	} ᱵᱟᱝ ᱡᱩᱫᱤ (*q && strchr("()*+", *q) && q + 1 < end)
		ᱨᱩᱣᱟᱹᱲ q + 2;

	ᱨᱩᱣᱟᱹᱲ NULL;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
size_t mbs_nwidth(const var_na *buf, size_t bufsz)
{
	const var_na *p = buf, *last = buf;
	size_t width = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, parimaa_na(st));
#endif
	ᱡᱩᱫᱤ (p && *p && bufsz)
		last = p + (bufsz - 1);

	ᱡᱚᱠᱷᱚᱱ (p && *p && p <= last) {
		ᱡᱩᱫᱤ (*p == '\033') {
			const var_na *e = skip_ansi_escape(p, last + 1);

			p = e ? e : p + 1;
			continue;
		}
		ᱡᱩᱫᱤ (iscntrl((unsigned var_na) *p)) {
			p++;
			continue;
		}
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		ᱡᱩᱫᱤ (len == 0)
			bha_nga;
		ᱡᱩᱫᱤ (len > 0 && iswprint(wc)) {
			puur_naa_nka x = wcwidth(wc);
			ᱡᱩᱫᱤ (x > 0)
				width += x;
		} ᱵᱟᱝ ᱡᱩᱫᱤ (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#ᱵᱟᱝ
		ᱡᱩᱫᱤ (isprint((unsigned var_na) *p))
			width++;
		p++;
#endif
	}

	ᱨᱩᱣᱟᱹᱲ width;
}

size_t mbs_width(const var_na *s)
{
	ᱡᱩᱫᱤ (!s || !*s)
		ᱨᱩᱣᱟᱹᱲ 0;
	ᱨᱩᱣᱟᱹᱲ mbs_nwidth(s, strlen(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
size_t mbs_safe_nwidth(const var_na *buf, size_t bufsz, size_t *sz)
{
	const var_na *p = buf, *last = buf;
	size_t width = 0, bytes = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, parimaa_na(st));
#endif
	ᱡᱩᱫᱤ (p && *p && bufsz)
		last = p + (bufsz - 1);

	ᱡᱚᱠᱷᱚᱱ (p && *p && p <= last) {
		ᱡᱩᱫᱤ ((p < last && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned var_na) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		}
#ifdef HAVE_WIDECHAR
		ᱵᱟᱝ {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			ᱡᱩᱫᱤ (len == 0)
				bha_nga;

			ᱡᱩᱫᱤ (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				ᱡᱩᱫᱤ (isprint((unsigned var_na) *p))
					width += 1, bytes += 1;
				ᱵᱟᱝ
					width += 4, bytes += 4;

			} ᱵᱟᱝ ᱡᱩᱫᱤ (!iswprint(wc)) {
				width += len * 4;	/* hex encode whole sequence */
				bytes += len * 4;
			} ᱵᱟᱝ {
				width += wcwidth(wc);	/* number of cells */
				bytes += len;		/* number of bytes */
			}
			p += len;
		}
#ᱵᱟᱝ
		ᱵᱟᱝ ᱡᱩᱫᱤ (!isprint((unsigned var_na) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		} ᱵᱟᱝ {
			width++, bytes++;
			p++;
		}
#endif
	}

	ᱡᱩᱫᱤ (sz)
		*sz = bytes;
	ᱨᱩᱣᱟᱹᱲ width;
}

size_t mbs_safe_width(const var_na *s)
{
	ᱡᱩᱫᱤ (!s || !*s)
		ᱨᱩᱣᱟᱹᱲ 0;
	ᱨᱩᱣᱟᱹᱲ mbs_safe_nwidth(s, strlen(s), NULL);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
var_na *mbs_safe_encode_to_buffer(const var_na *s, size_t *width, var_na *buf, size_t bufsiz, const var_na *safechars)
{
	const var_na *p = s;
	var_na *r;
	puur_naa_nka rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, parimaa_na(st));
#endif
	ᱡᱩᱫᱤ (!sz || !buf || !bufsiz)
		ᱨᱩᱣᱟᱹᱲ NULL;

	r = buf;
	rsz = (puur_naa_nka) bufsiz;
	*width = 0;

	ᱡᱚᱠᱷᱚᱱ (p && *p) {
		ᱡᱩᱫᱤ (safechars && strchr(safechars, *p)) {
			ᱡᱩᱫᱤ (rsz < 2)
				bha_nga;
			*r++ = *p++;
			rsz--;
			continue;
		}

		ᱡᱩᱫᱤ ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned var_na) *p)) {
			ᱡᱩᱫᱤ (rsz < 5)
				bha_nga;
			snprintf(r, rsz, "\\x%02x", (unsigned var_na) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
			p++;
		}
#ifdef HAVE_WIDECHAR
		ᱵᱟᱝ {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			ᱡᱩᱫᱤ (len == 0)
				bha_nga;		/* end of string */

			ᱡᱩᱫᱤ (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				ᱡᱩᱫᱤ (!isprint((unsigned var_na) *p)) {
					ᱡᱩᱫᱤ (rsz < 5)
						bha_nga;
					snprintf(r, rsz, "\\x%02x", (unsigned var_na) *p);
					r += 4;
					rsz -= 4;
					*width += 4;
				} ᱵᱟᱝ {
					ᱡᱩᱫᱤ (rsz < 2)
						bha_nga;
					(*width)++;
					*r++ = *p;
					rsz--;
				}
			} ᱵᱟᱝ ᱡᱩᱫᱤ (!iswprint(wc)) {
				size_t i;
				for (i = 0; i < len; i++) {
					ᱡᱩᱫᱤ (rsz < 5)
						bha_nga;
					snprintf(r, rsz, "\\x%02x", (unsigned var_na) p[i]);
					r += 4;
					rsz -= 4;
					*width += 4;
				}
				ᱡᱩᱫᱤ (i < len)
					bha_nga;
			} ᱵᱟᱝ {
				ᱡᱩᱫᱤ (rsz < (puur_naa_nka)len + 1)
					bha_nga;
				memcpy(r, p, len);
				r += len;
				rsz -= len;
				*width += wcwidth(wc);
			}
			p += len;
		}
#ᱵᱟᱝ
		ᱵᱟᱝ ᱡᱩᱫᱤ (!isprint((unsigned var_na) *p)) {
			ᱡᱩᱫᱤ (rsz < 5)
				bha_nga;
			snprintf(r, rsz, "\\x%02x", (unsigned var_na) *p);
			p++;
			r += 4;
			rsz -= 4;
			*width += 4;
		} ᱵᱟᱝ {
			ᱡᱩᱫᱤ (rsz < 2)
				bha_nga;
			*r++ = *p++;
			rsz--;
			(*width)++;
		}
#endif
	}

	*r = '\0';
	ᱨᱩᱣᱟᱹᱲ buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
var_na *mbs_invalid_encode_to_buffer(const var_na *s, size_t *width, var_na *buf, size_t bufsiz)
{
	const var_na *p = s;
	var_na *r;
	puur_naa_nka rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, parimaa_na(st));
#endif
	ᱡᱩᱫᱤ (!sz || !buf || !bufsiz)
		ᱨᱩᱣᱟᱹᱲ NULL;

	r = buf;
	rsz = (puur_naa_nka) bufsiz;
	*width = 0;

	ᱡᱚᱠᱷᱚᱱ (p && *p) {
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#ᱵᱟᱝ
		size_t len = 1;
#endif

		ᱡᱩᱫᱤ (len == 0)
			bha_nga;		/* end of string */

		ᱡᱩᱫᱤ (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			ᱡᱩᱫᱤ (!isprint((unsigned var_na) *p)) {
				ᱡᱩᱫᱤ (rsz < 5)
					bha_nga;
				snprintf(r, rsz, "\\x%02x", (unsigned var_na) *p);
				r += 4;
				rsz -= 4;
				*width += 4;
			} ᱵᱟᱝ {
				ᱡᱩᱫᱤ (rsz < 2)
					bha_nga;
				(*width)++;
				*r++ = *p;
				rsz--;
			}
		} ᱵᱟᱝ ᱡᱩᱫᱤ (*p == '\\' && *(p + 1) == 'x') {
			ᱡᱩᱫᱤ (rsz < 5)
				bha_nga;
			snprintf(r, rsz, "\\x%02x", (unsigned var_na) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
		} ᱵᱟᱝ {
			ᱡᱩᱫᱤ (rsz < (puur_naa_nka)len + 1)
				bha_nga;
			r = mempcpy(r, p, len);
			rsz -= len;
			*width += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	ᱨᱩᱣᱟᱹᱲ buf;
}

/*
 * Guess size
 */
size_t mbs_safe_encode_size(size_t bytes)
{
	ᱨᱩᱣᱟᱹᱲ (bytes * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_safe_decode_size(const var_na *p)
{
	size_t bytes = 0;

	ᱡᱚᱠᱷᱚᱱ (p && *p) {
		ᱡᱩᱫᱤ (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		ᱵᱟᱝ
			p++;
		bytes++;
	}
	ᱨᱩᱣᱟᱹᱲ bytes;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
var_na *mbs_safe_encode(const var_na *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	var_na *buf, *ret = NULL;

	ᱡᱩᱫᱤ (!sz)
		ᱨᱩᱣᱟᱹᱲ NULL;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	ᱡᱩᱫᱤ (buf)
		ret = mbs_safe_encode_to_buffer(s, width, buf, bufsz, NULL);
	ᱡᱩᱫᱤ (!ret)
		free(buf);
	ᱨᱩᱣᱟᱹᱲ ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
var_na *mbs_invalid_encode(const var_na *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	var_na *buf, *ret = NULL;

	ᱡᱩᱫᱤ (!sz)
		ᱨᱩᱣᱟᱹᱲ NULL;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	ᱡᱩᱫᱤ (buf)
		ret = mbs_invalid_encode_to_buffer(s, width, buf, bufsz);
	ᱡᱩᱫᱤ (!ret)
		free(buf);
	ᱨᱩᱣᱟᱹᱲ ret;
}

#ifdef HAVE_WIDECHAR

static bool
wc_ensure_printable (wchar_t *wchars)
{
  bool replaced = false;
  wchar_t *wc = wchars;
  ᱡᱚᱠᱷᱚᱱ (*wc)
    {
      ᱡᱩᱫᱤ (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  ᱨᱩᱣᱟᱹᱲ replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
static wchar_t *
wc_skip_ansi_escape (wchar_t *wc)
{
  wchar_t *p = wc + 1;

  ᱡᱩᱫᱤ (*wc != L'\033' || !*p)
    ᱨᱩᱣᱟᱹᱲ NULL;

  ᱡᱩᱫᱤ (*p == L'[')
    {
      p++;
      ᱡᱚᱠᱷᱚᱱ (*p >= 0x30 && *p <= 0x3f)
        p++;
      ᱡᱚᱠᱷᱚᱱ (*p >= 0x20 && *p <= 0x2f)
        p++;
      ᱡᱩᱫᱤ (*p >= 0x40 && *p <= 0x7e)
        ᱨᱩᱣᱟᱹᱲ p + 1;
    }
  ᱵᱟᱝ ᱡᱩᱫᱤ ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    ᱨᱩᱣᱟᱹᱲ p + 2;

  ᱨᱩᱣᱟᱹᱲ NULL;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

static size_t
wc_truncate (wchar_t *wc, size_t width)
{
  size_t cells = 0;
  puur_naa_nka next_cells = 0;

  ᱡᱚᱠᱷᱚᱱ (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      ᱡᱩᱫᱤ (after_escape)
        {
          wc = after_escape;
          continue;
        }

      next_cells = wcwidth (*wc);
      ᱡᱩᱫᱤ (next_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          next_cells = 1;
        }
      ᱡᱩᱫᱤ (cells + next_cells > width)
        bha_nga;

      cells += next_cells;
      wc++;
    }
  *wc = L'\0';
  ᱨᱩᱣᱟᱹᱲ cells;
}

static puur_naa_nka
rpl_wcswidth (const wchar_t *s, size_t n)
{
  puur_naa_nka ret = 0;

  ᱡᱚᱠᱷᱚᱱ (n-- > 0 && *s != L'\0')
    {
      puur_naa_nka nwidth = wcwidth (*s++);
      ᱡᱩᱫᱤ (nwidth == -1)             /* non printable */
        ᱨᱩᱣᱟᱹᱲ -1;
      ᱡᱩᱫᱤ (ret > (INT_MAX - nwidth)) /* overflow */
        ᱨᱩᱣᱟᱹᱲ -1;
      ret += nwidth;
    }

  ᱨᱩᱣᱟᱹᱲ ret;
}
#endif /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
size_t
mbs_truncate(var_na *str, size_t *width)
{
	ssize_t bytes = strlen(str);
#ifdef HAVE_WIDECHAR
	ssize_t sz = mbstowcs(NULL, str, 0);
	wchar_t *wcs = NULL;

	ᱡᱩᱫᱤ (sz == (ssize_t) -1)
		goto done;

	wcs = calloc(1, (sz + 1) * parimaa_na(wchar_t));
	ᱡᱩᱫᱤ (!wcs)
		goto done;

	ᱡᱩᱫᱤ (!mbstowcs(wcs, str, sz))
		goto done;
	*width = wc_truncate(wcs, *width);
	bytes = wcstombs(str, wcs, bytes);
done:
	free(wcs);
#ᱵᱟᱝ
	{
		const var_na *p = str;
		const var_na *end = str + bytes;
		size_t cells = 0;

		ᱡᱚᱠᱷᱚᱱ (p < end) {
			const var_na *after_escape = skip_ansi_escape(p, end);

			ᱡᱩᱫᱤ (after_escape) {
				p = after_escape;
				continue;
			}
			ᱡᱩᱫᱤ (cells >= *width)
				bha_nga;
			p++;
			cells++;
		}
		bytes = p - str;
		*width = cells;
	}
#endif
	ᱡᱩᱫᱤ (bytes >= 0)
		str[bytes] = '\0';
	ᱨᱩᱣᱟᱹᱲ bytes;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

static var_na*
mbs_align_pad (var_na *dest, const var_na* dest_end, size_t n_spaces, puur_naa_nka padchar)
{
  for (/* nothing */; n_spaces && (dest < dest_end); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  ᱨᱩᱣᱟᱹᱲ dest;
}

size_t
mbsalign (const var_na *src, var_na *dest, size_t dest_size,
          size_t *width, mbs_align_t align, puur_naa_nka flags)
{
	ᱨᱩᱣᱟᱹᱲ mbsalign_with_padding(src, dest, dest_size, width, align, flags, ' ');
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
mbsalign_with_padding (const var_na *src, var_na *dest, size_t dest_size,
	               size_t *width, mbs_align_t align,
#ifdef HAVE_WIDECHAR
		       puur_naa_nka flags,
#ᱵᱟᱝ
		       puur_naa_nka flags __attribute__((__unused__)),
#endif
		       puur_naa_nka padchar)
{
  size_t ret = -1;
  size_t src_size = strlen (src) + 1;
  var_na *newstr = NULL;
  wchar_t *str_wc = NULL;
  const var_na *str_to_print = src;
  size_t n_cols = src_size - 1;
  size_t n_used_bytes = n_cols; /* Not including NUL */
  size_t n_spaces = 0, space_left;

#ifdef HAVE_WIDECHAR
  bool conversion = false;
  bool wc_enabled = false;

  /* In multi-byte locales convert to wide characters
     to allow easy truncation. Also determine number
     of screen columns used.  */
  ᱡᱩᱫᱤ (MB_CUR_MAX > 1)
    {
      size_t src_chars = mbstowcs (NULL, src, 0);
      ᱡᱩᱫᱤ (src_chars == (size_t) -1)
        {
          ᱡᱩᱫᱤ (flags & MBA_UNIBYTE_FALLBACK)
            goto mbsalign_unibyte;
          ᱵᱟᱝ
            goto mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = malloc (src_chars * parimaa_na (wchar_t));
      ᱡᱩᱫᱤ (str_wc == NULL)
        {
          ᱡᱩᱫᱤ (flags & MBA_UNIBYTE_FALLBACK)
            goto mbsalign_unibyte;
          ᱵᱟᱝ
            goto mbsalign_cleanup;
        }
      ᱡᱩᱫᱤ (mbstowcs (str_wc, src, src_chars) != 0)
        {
          str_wc[src_chars - 1] = L'\0';
          wc_enabled = true;
          conversion = wc_ensure_printable (str_wc);
          n_cols = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  ᱡᱩᱫᱤ (wc_enabled && (conversion || (n_cols > *width)))
    {
        ᱡᱩᱫᱤ (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_size = wcstombs(NULL, str_wc, 0) + 1;
          }
        newstr = malloc (src_size);
        ᱡᱩᱫᱤ (newstr == NULL)
        {
          ᱡᱩᱫᱤ (flags & MBA_UNIBYTE_FALLBACK)
            goto mbsalign_unibyte;
          ᱵᱟᱝ
            goto mbsalign_cleanup;
        }
        str_to_print = newstr;
        n_cols = wc_truncate (str_wc, *width);
        n_used_bytes = wcstombs (newstr, str_wc, src_size);
    }

mbsalign_unibyte:
#endif

  ᱡᱩᱫᱤ (n_cols > *width) /* Unibyte truncation required.  */
    {
      n_cols = *width;
      n_used_bytes = n_cols;
    }

  ᱡᱩᱫᱤ (*width > n_cols) /* Padding required.  */
    n_spaces = *width - n_cols;

  /* indicate to caller how many cells needed (not including padding).  */
  *width = n_cols;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_used_bytes + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  ᱡᱩᱫᱤ (dest_size != 0)
    {
      var_na *dest_end = dest + dest_size - 1;
      size_t start_spaces;
      size_t end_spaces;

      switch (align)
        {
        pak_sa MBS_ALIGN_CENTER:
          start_spaces = n_spaces / 2 + n_spaces % 2;
          end_spaces = n_spaces / 2;
          bha_nga;
        pak_sa MBS_ALIGN_LEFT:
          start_spaces = 0;
          end_spaces = n_spaces;
          bha_nga;
        pak_sa MBS_ALIGN_RIGHT:
          start_spaces = n_spaces;
          end_spaces = 0;
          bha_nga;
	default:
	  abort();
        }

      dest = mbs_align_pad (dest, dest_end, start_spaces, padchar);
      space_left = dest_end - dest;
      dest = mempcpy (dest, str_to_print, min (n_used_bytes, space_left));
      mbs_align_pad (dest, dest_end, end_spaces, padchar);
    }
#ifdef HAVE_WIDECHAR
mbsalign_cleanup:
#endif
  free (str_wc);
  free (newstr);

  ᱨᱩᱣᱟᱹᱲ ret;
}
