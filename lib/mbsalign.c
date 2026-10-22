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
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "mbsalign.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"
#ಅನ್ತರ್ಭಾವ "widechar.h"

/* Return the byte after a complete ANSI escape sequence, or NULL. */
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *skip_ansi_escape(ಧ್ರುವ ವರ್ಣ *p, ಧ್ರುವ ವರ್ಣ *end)
{
	ಧ್ರುವ ವರ್ಣ *q;

	ಯದಿ (p >= end || (unsigned ವರ್ಣ) *p != '\033' || p + 1 >= end)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	q = p + 1;
	ಯದಿ (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		ಯಾವತ್ (q < end && (unsigned ವರ್ಣ) *q >= 0x30
				&& (unsigned ವರ್ಣ) *q <= 0x3f)
			q++;
		ಯಾವತ್ (q < end && (unsigned ವರ್ಣ) *q >= 0x20
				&& (unsigned ವರ್ಣ) *q <= 0x2f)
			q++;
		ಯದಿ (q < end && (unsigned ವರ್ಣ) *q >= 0x40
				&& (unsigned ವರ್ಣ) *q <= 0x7e)
			ಪ್ರತ್ಯಾಗಮ q + 1;
	} ಅನ್ಯಥಾ ಯದಿ (*q && strchr("()*+", *q) && q + 1 < end)
		ಪ್ರತ್ಯಾಗಮ q + 2;

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
size_t mbs_nwidth(ಧ್ರುವ ವರ್ಣ *buf, size_t bufsz)
{
	ಧ್ರುವ ವರ್ಣ *p = buf, *last = buf;
	size_t width = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, ಪರಿಮಾಣ(st));
#endif
	ಯದಿ (p && *p && bufsz)
		last = p + (bufsz - 1);

	ಯಾವತ್ (p && *p && p <= last) {
		ಯದಿ (*p == '\033') {
			ಧ್ರುವ ವರ್ಣ *e = skip_ansi_escape(p, last + 1);

			p = e ? e : p + 1;
			ಅನುವರ್ತ;
		}
		ಯದಿ (iscntrl((unsigned ವರ್ಣ) *p)) {
			p++;
			ಅನುವರ್ತ;
		}
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		ಯದಿ (len == 0)
			ಭಙ್ಗ;
		ಯದಿ (len > 0 && iswprint(wc)) {
			ಪೂರ್ಣಾಙ್ಕ x = wcwidth(wc);
			ಯದಿ (x > 0)
				width += x;
		} ಅನ್ಯಥಾ ಯದಿ (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#ಅನ್ಯಥಾ
		ಯದಿ (isprint((unsigned ವರ್ಣ) *p))
			width++;
		p++;
#endif
	}

	ಪ್ರತ್ಯಾಗಮ width;
}

size_t mbs_width(ಧ್ರುವ ವರ್ಣ *s)
{
	ಯದಿ (!s || !*s)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಪ್ರತ್ಯಾಗಮ mbs_nwidth(s, strlen(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
size_t mbs_safe_nwidth(ಧ್ರುವ ವರ್ಣ *buf, size_t bufsz, size_t *sz)
{
	ಧ್ರುವ ವರ್ಣ *p = buf, *last = buf;
	size_t width = 0, bytes = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, ಪರಿಮಾಣ(st));
#endif
	ಯದಿ (p && *p && bufsz)
		last = p + (bufsz - 1);

	ಯಾವತ್ (p && *p && p <= last) {
		ಯದಿ ((p < last && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned ವರ್ಣ) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		}
#ifdef HAVE_WIDECHAR
		ಅನ್ಯಥಾ {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			ಯದಿ (len == 0)
				ಭಙ್ಗ;

			ಯದಿ (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				ಯದಿ (isprint((unsigned ವರ್ಣ) *p))
					width += 1, bytes += 1;
				ಅನ್ಯಥಾ
					width += 4, bytes += 4;

			} ಅನ್ಯಥಾ ಯದಿ (!iswprint(wc)) {
				width += len * 4;	/* hex encode whole sequence */
				bytes += len * 4;
			} ಅನ್ಯಥಾ {
				width += wcwidth(wc);	/* number of cells */
				bytes += len;		/* number of bytes */
			}
			p += len;
		}
#ಅನ್ಯಥಾ
		ಅನ್ಯಥಾ ಯದಿ (!isprint((unsigned ವರ್ಣ) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		} ಅನ್ಯಥಾ {
			width++, bytes++;
			p++;
		}
#endif
	}

	ಯದಿ (sz)
		*sz = bytes;
	ಪ್ರತ್ಯಾಗಮ width;
}

size_t mbs_safe_width(ಧ್ರುವ ವರ್ಣ *s)
{
	ಯದಿ (!s || !*s)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಪ್ರತ್ಯಾಗಮ mbs_safe_nwidth(s, strlen(s), ರಿಕ್ತ);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
ವರ್ಣ *mbs_safe_encode_to_buffer(ಧ್ರುವ ವರ್ಣ *s, size_t *width, ವರ್ಣ *buf, size_t bufsiz, ಧ್ರುವ ವರ್ಣ *safechars)
{
	ಧ್ರುವ ವರ್ಣ *p = s;
	ವರ್ಣ *r;
	ಪೂರ್ಣಾಙ್ಕ rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, ಪರಿಮಾಣ(st));
#endif
	ಯದಿ (!sz || !buf || !bufsiz)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	r = buf;
	rsz = (ಪೂರ್ಣಾಙ್ಕ) bufsiz;
	*width = 0;

	ಯಾವತ್ (p && *p) {
		ಯದಿ (safechars && strchr(safechars, *p)) {
			ಯದಿ (rsz < 2)
				ಭಙ್ಗ;
			*r++ = *p++;
			rsz--;
			ಅನುವರ್ತ;
		}

		ಯದಿ ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned ವರ್ಣ) *p)) {
			ಯದಿ (rsz < 5)
				ಭಙ್ಗ;
			snprintf(r, rsz, "\\x%02x", (unsigned ವರ್ಣ) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
			p++;
		}
#ifdef HAVE_WIDECHAR
		ಅನ್ಯಥಾ {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			ಯದಿ (len == 0)
				ಭಙ್ಗ;		/* end of string */

			ಯದಿ (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				ಯದಿ (!isprint((unsigned ವರ್ಣ) *p)) {
					ಯದಿ (rsz < 5)
						ಭಙ್ಗ;
					snprintf(r, rsz, "\\x%02x", (unsigned ವರ್ಣ) *p);
					r += 4;
					rsz -= 4;
					*width += 4;
				} ಅನ್ಯಥಾ {
					ಯದಿ (rsz < 2)
						ಭಙ್ಗ;
					(*width)++;
					*r++ = *p;
					rsz--;
				}
			} ಅನ್ಯಥಾ ಯದಿ (!iswprint(wc)) {
				size_t i;
				ಪ್ರತಿ (i = 0; i < len; i++) {
					ಯದಿ (rsz < 5)
						ಭಙ್ಗ;
					snprintf(r, rsz, "\\x%02x", (unsigned ವರ್ಣ) p[i]);
					r += 4;
					rsz -= 4;
					*width += 4;
				}
				ಯದಿ (i < len)
					ಭಙ್ಗ;
			} ಅನ್ಯಥಾ {
				ಯದಿ (rsz < (ಪೂರ್ಣಾಙ್ಕ)len + 1)
					ಭಙ್ಗ;
				memcpy(r, p, len);
				r += len;
				rsz -= len;
				*width += wcwidth(wc);
			}
			p += len;
		}
#ಅನ್ಯಥಾ
		ಅನ್ಯಥಾ ಯದಿ (!isprint((unsigned ವರ್ಣ) *p)) {
			ಯದಿ (rsz < 5)
				ಭಙ್ಗ;
			snprintf(r, rsz, "\\x%02x", (unsigned ವರ್ಣ) *p);
			p++;
			r += 4;
			rsz -= 4;
			*width += 4;
		} ಅನ್ಯಥಾ {
			ಯದಿ (rsz < 2)
				ಭಙ್ಗ;
			*r++ = *p++;
			rsz--;
			(*width)++;
		}
#endif
	}

	*r = '\0';
	ಪ್ರತ್ಯಾಗಮ buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
ವರ್ಣ *mbs_invalid_encode_to_buffer(ಧ್ರುವ ವರ್ಣ *s, size_t *width, ವರ್ಣ *buf, size_t bufsiz)
{
	ಧ್ರುವ ವರ್ಣ *p = s;
	ವರ್ಣ *r;
	ಪೂರ್ಣಾಙ್ಕ rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, ಪರಿಮಾಣ(st));
#endif
	ಯದಿ (!sz || !buf || !bufsiz)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	r = buf;
	rsz = (ಪೂರ್ಣಾಙ್ಕ) bufsiz;
	*width = 0;

	ಯಾವತ್ (p && *p) {
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#ಅನ್ಯಥಾ
		size_t len = 1;
#endif

		ಯದಿ (len == 0)
			ಭಙ್ಗ;		/* end of string */

		ಯದಿ (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			ಯದಿ (!isprint((unsigned ವರ್ಣ) *p)) {
				ಯದಿ (rsz < 5)
					ಭಙ್ಗ;
				snprintf(r, rsz, "\\x%02x", (unsigned ವರ್ಣ) *p);
				r += 4;
				rsz -= 4;
				*width += 4;
			} ಅನ್ಯಥಾ {
				ಯದಿ (rsz < 2)
					ಭಙ್ಗ;
				(*width)++;
				*r++ = *p;
				rsz--;
			}
		} ಅನ್ಯಥಾ ಯದಿ (*p == '\\' && *(p + 1) == 'x') {
			ಯದಿ (rsz < 5)
				ಭಙ್ಗ;
			snprintf(r, rsz, "\\x%02x", (unsigned ವರ್ಣ) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
		} ಅನ್ಯಥಾ {
			ಯದಿ (rsz < (ಪೂರ್ಣಾಙ್ಕ)len + 1)
				ಭಙ್ಗ;
			r = mempcpy(r, p, len);
			rsz -= len;
			*width += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	ಪ್ರತ್ಯಾಗಮ buf;
}

/*
 * Guess size
 */
size_t mbs_safe_encode_size(size_t bytes)
{
	ಪ್ರತ್ಯಾಗಮ (bytes * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_safe_decode_size(ಧ್ರುವ ವರ್ಣ *p)
{
	size_t bytes = 0;

	ಯಾವತ್ (p && *p) {
		ಯದಿ (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		ಅನ್ಯಥಾ
			p++;
		bytes++;
	}
	ಪ್ರತ್ಯಾಗಮ bytes;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
ವರ್ಣ *mbs_safe_encode(ಧ್ರುವ ವರ್ಣ *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	ವರ್ಣ *buf, *ret = ರಿಕ್ತ;

	ಯದಿ (!sz)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	ಯದಿ (buf)
		ret = mbs_safe_encode_to_buffer(s, width, buf, bufsz, ರಿಕ್ತ);
	ಯದಿ (!ret)
		ಮುಕ್ತ(buf);
	ಪ್ರತ್ಯಾಗಮ ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
ವರ್ಣ *mbs_invalid_encode(ಧ್ರುವ ವರ್ಣ *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	ವರ್ಣ *buf, *ret = ರಿಕ್ತ;

	ಯದಿ (!sz)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	ಯದಿ (buf)
		ret = mbs_invalid_encode_to_buffer(s, width, buf, bufsz);
	ಯದಿ (!ret)
		ಮುಕ್ತ(buf);
	ಪ್ರತ್ಯಾಗಮ ret;
}

#ifdef HAVE_WIDECHAR

ಸ್ಥಿರ bool
wc_ensure_printable (wchar_t *wchars)
{
  bool replaced = false;
  wchar_t *wc = wchars;
  ಯಾವತ್ (*wc)
    {
      ಯದಿ (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  ಪ್ರತ್ಯಾಗಮ replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
ಸ್ಥಿರ wchar_t *
wc_skip_ansi_escape (wchar_t *wc)
{
  wchar_t *p = wc + 1;

  ಯದಿ (*wc != L'\033' || !*p)
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

  ಯದಿ (*p == L'[')
    {
      p++;
      ಯಾವತ್ (*p >= 0x30 && *p <= 0x3f)
        p++;
      ಯಾವತ್ (*p >= 0x20 && *p <= 0x2f)
        p++;
      ಯದಿ (*p >= 0x40 && *p <= 0x7e)
        ಪ್ರತ್ಯಾಗಮ p + 1;
    }
  ಅನ್ಯಥಾ ಯದಿ ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    ಪ್ರತ್ಯಾಗಮ p + 2;

  ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

ಸ್ಥಿರ size_t
wc_truncate (wchar_t *wc, size_t width)
{
  size_t cells = 0;
  ಪೂರ್ಣಾಙ್ಕ next_cells = 0;

  ಯಾವತ್ (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      ಯದಿ (after_escape)
        {
          wc = after_escape;
          ಅನುವರ್ತ;
        }

      next_cells = wcwidth (*wc);
      ಯದಿ (next_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          next_cells = 1;
        }
      ಯದಿ (cells + next_cells > width)
        ಭಙ್ಗ;

      cells += next_cells;
      wc++;
    }
  *wc = L'\0';
  ಪ್ರತ್ಯಾಗಮ cells;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
rpl_wcswidth (ಧ್ರುವ wchar_t *s, size_t n)
{
  ಪೂರ್ಣಾಙ್ಕ ret = 0;

  ಯಾವತ್ (n-- > 0 && *s != L'\0')
    {
      ಪೂರ್ಣಾಙ್ಕ nwidth = wcwidth (*s++);
      ಯದಿ (nwidth == -1)             /* non printable */
        ಪ್ರತ್ಯಾಗಮ -1;
      ಯದಿ (ret > (INT_MAX - nwidth)) /* overflow */
        ಪ್ರತ್ಯಾಗಮ -1;
      ret += nwidth;
    }

  ಪ್ರತ್ಯಾಗಮ ret;
}
#endif /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
size_t
mbs_truncate(ವರ್ಣ *str, size_t *width)
{
	ssize_t bytes = strlen(str);
#ifdef HAVE_WIDECHAR
	ssize_t sz = mbstowcs(ರಿಕ್ತ, str, 0);
	wchar_t *wcs = ರಿಕ್ತ;

	ಯದಿ (sz == (ssize_t) -1)
		ಗಚ್ಛ done;

	wcs = calloc(1, (sz + 1) * ಪರಿಮಾಣ(wchar_t));
	ಯದಿ (!wcs)
		ಗಚ್ಛ done;

	ಯದಿ (!mbstowcs(wcs, str, sz))
		ಗಚ್ಛ done;
	*width = wc_truncate(wcs, *width);
	bytes = wcstombs(str, wcs, bytes);
done:
	ಮುಕ್ತ(wcs);
#ಅನ್ಯಥಾ
	{
		ಧ್ರುವ ವರ್ಣ *p = str;
		ಧ್ರುವ ವರ್ಣ *end = str + bytes;
		size_t cells = 0;

		ಯಾವತ್ (p < end) {
			ಧ್ರುವ ವರ್ಣ *after_escape = skip_ansi_escape(p, end);

			ಯದಿ (after_escape) {
				p = after_escape;
				ಅನುವರ್ತ;
			}
			ಯದಿ (cells >= *width)
				ಭಙ್ಗ;
			p++;
			cells++;
		}
		bytes = p - str;
		*width = cells;
	}
#endif
	ಯದಿ (bytes >= 0)
		str[bytes] = '\0';
	ಪ್ರತ್ಯಾಗಮ bytes;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

ಸ್ಥಿರ ವರ್ಣ*
mbs_align_pad (ವರ್ಣ *dest, ಧ್ರುವ ವರ್ಣ* dest_end, size_t n_spaces, ಪೂರ್ಣಾಙ್ಕ padchar)
{
  ಪ್ರತಿ (/* nothing */; n_spaces && (dest < dest_end); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  ಪ್ರತ್ಯಾಗಮ dest;
}

size_t
mbsalign (ಧ್ರುವ ವರ್ಣ *src, ವರ್ಣ *dest, size_t dest_size,
          size_t *width, mbs_align_t align, ಪೂರ್ಣಾಙ್ಕ flags)
{
	ಪ್ರತ್ಯಾಗಮ mbsalign_with_padding(src, dest, dest_size, width, align, flags, ' ');
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
mbsalign_with_padding (ಧ್ರುವ ವರ್ಣ *src, ವರ್ಣ *dest, size_t dest_size,
	               size_t *width, mbs_align_t align,
#ifdef HAVE_WIDECHAR
		       ಪೂರ್ಣಾಙ್ಕ flags,
#ಅನ್ಯಥಾ
		       ಪೂರ್ಣಾಙ್ಕ flags __attribute__((__unused__)),
#endif
		       ಪೂರ್ಣಾಙ್ಕ padchar)
{
  size_t ret = -1;
  size_t src_size = strlen (src) + 1;
  ವರ್ಣ *newstr = ರಿಕ್ತ;
  wchar_t *str_wc = ರಿಕ್ತ;
  ಧ್ರುವ ವರ್ಣ *str_to_print = src;
  size_t n_cols = src_size - 1;
  size_t n_used_bytes = n_cols; /* Not including NUL */
  size_t n_spaces = 0, space_ಎಡ;

#ifdef HAVE_WIDECHAR
  bool conversion = false;
  bool wc_enabled = false;

  /* In multi-byte locales convert to wide characters
     to allow easy truncation. Also determine number
     of screen columns used.  */
  ಯದಿ (MB_CUR_MAX > 1)
    {
      size_t src_chars = mbstowcs (ರಿಕ್ತ, src, 0);
      ಯದಿ (src_chars == (size_t) -1)
        {
          ಯದಿ (flags & MBA_UNIBYTE_FALLBACK)
            ಗಚ್ಛ mbsalign_unibyte;
          ಅನ್ಯಥಾ
            ಗಚ್ಛ mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = malloc (src_chars * ಪರಿಮಾಣ (wchar_t));
      ಯದಿ (str_wc == ರಿಕ್ತ)
        {
          ಯದಿ (flags & MBA_UNIBYTE_FALLBACK)
            ಗಚ್ಛ mbsalign_unibyte;
          ಅನ್ಯಥಾ
            ಗಚ್ಛ mbsalign_cleanup;
        }
      ಯದಿ (mbstowcs (str_wc, src, src_chars) != 0)
        {
          str_wc[src_chars - 1] = L'\0';
          wc_enabled = true;
          conversion = wc_ensure_printable (str_wc);
          n_cols = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  ಯದಿ (wc_enabled && (conversion || (n_cols > *width)))
    {
        ಯದಿ (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_size = wcstombs(ರಿಕ್ತ, str_wc, 0) + 1;
          }
        newstr = malloc (src_size);
        ಯದಿ (newstr == ರಿಕ್ತ)
        {
          ಯದಿ (flags & MBA_UNIBYTE_FALLBACK)
            ಗಚ್ಛ mbsalign_unibyte;
          ಅನ್ಯಥಾ
            ಗಚ್ಛ mbsalign_cleanup;
        }
        str_to_print = newstr;
        n_cols = wc_truncate (str_wc, *width);
        n_used_bytes = wcstombs (newstr, str_wc, src_size);
    }

mbsalign_unibyte:
#endif

  ಯದಿ (n_cols > *width) /* Unibyte truncation required.  */
    {
      n_cols = *width;
      n_used_bytes = n_cols;
    }

  ಯದಿ (*width > n_cols) /* Padding required.  */
    n_spaces = *width - n_cols;

  /* indicate to caller how many cells needed (not including padding).  */
  *width = n_cols;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_used_bytes + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  ಯದಿ (dest_size != 0)
    {
      ವರ್ಣ *dest_end = dest + dest_size - 1;
      size_t start_spaces;
      size_t end_spaces;

      ವಿಕಲ್ಪ (align)
        {
        ಪಕ್ಷ MBS_ALIGN_CENTER:
          start_spaces = n_spaces / 2 + n_spaces % 2;
          end_spaces = n_spaces / 2;
          ಭಙ್ಗ;
        ಪಕ್ಷ MBS_ALIGN_ಎಡ:
          start_spaces = 0;
          end_spaces = n_spaces;
          ಭಙ್ಗ;
        ಪಕ್ಷ MBS_ALIGN_ಬಲ:
          start_spaces = n_spaces;
          end_spaces = 0;
          ಭಙ್ಗ;
	ಸ್ವಭಾವ:
	  abort();
        }

      dest = mbs_align_pad (dest, dest_end, start_spaces, padchar);
      space_ಎಡ = dest_end - dest;
      dest = mempcpy (dest, str_to_print, min (n_used_bytes, space_ಎಡ));
      mbs_align_pad (dest, dest_end, end_spaces, padchar);
    }
#ifdef HAVE_WIDECHAR
mbsalign_cleanup:
#endif
  ಮುಕ್ತ (str_wc);
  ಮುಕ್ತ (newstr);

  ಪ್ರತ್ಯಾಗಮ ret;
}
