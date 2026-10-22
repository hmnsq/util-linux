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
static const char *skip_ansi_escape(const char *p, const char *end)
{
	const char *q;

	nếu (p >= end || (unsigned char) *p != '\033' || p + 1 >= end)
		trả_về NULL;

	q = p + 1;
	nếu (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		trong_khi (q < end && (unsigned char) *q >= 0x30
				&& (unsigned char) *q <= 0x3f)
			q++;
		trong_khi (q < end && (unsigned char) *q >= 0x20
				&& (unsigned char) *q <= 0x2f)
			q++;
		nếu (q < end && (unsigned char) *q >= 0x40
				&& (unsigned char) *q <= 0x7e)
			trả_về q + 1;
	} không_thì nếu (*q && strchr("()*+", *q) && q + 1 < end)
		trả_về q + 2;

	trả_về NULL;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
size_t mbs_nwidth(const char *buf, size_t bufsz)
{
	const char *p = buf, *last = buf;
	size_t width = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, sizeof(st));
#endif
	nếu (p && *p && bufsz)
		last = p + (bufsz - 1);

	trong_khi (p && *p && p <= last) {
		nếu (*p == '\033') {
			const char *e = skip_ansi_escape(p, last + 1);

			p = e ? e : p + 1;
			continue;
		}
		nếu (iscntrl((unsigned char) *p)) {
			p++;
			continue;
		}
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		nếu (len == 0)
			break;
		nếu (len > 0 && iswprint(wc)) {
			số_nguyên x = wcwidth(wc);
			nếu (x > 0)
				width += x;
		} không_thì nếu (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#không_thì
		nếu (isprint((unsigned char) *p))
			width++;
		p++;
#endif
	}

	trả_về width;
}

size_t mbs_width(const char *s)
{
	nếu (!s || !*s)
		trả_về 0;
	trả_về mbs_nwidth(s, strlen(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
size_t mbs_safe_nwidth(const char *buf, size_t bufsz, size_t *sz)
{
	const char *p = buf, *last = buf;
	size_t width = 0, bytes = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, sizeof(st));
#endif
	nếu (p && *p && bufsz)
		last = p + (bufsz - 1);

	trong_khi (p && *p && p <= last) {
		nếu ((p < last && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned char) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		}
#ifdef HAVE_WIDECHAR
		không_thì {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			nếu (len == 0)
				break;

			nếu (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				nếu (isprint((unsigned char) *p))
					width += 1, bytes += 1;
				không_thì
					width += 4, bytes += 4;

			} không_thì nếu (!iswprint(wc)) {
				width += len * 4;	/* hex encode whole sequence */
				bytes += len * 4;
			} không_thì {
				width += wcwidth(wc);	/* number of cells */
				bytes += len;		/* number of bytes */
			}
			p += len;
		}
#không_thì
		không_thì nếu (!isprint((unsigned char) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		} không_thì {
			width++, bytes++;
			p++;
		}
#endif
	}

	nếu (sz)
		*sz = bytes;
	trả_về width;
}

size_t mbs_safe_width(const char *s)
{
	nếu (!s || !*s)
		trả_về 0;
	trả_về mbs_safe_nwidth(s, strlen(s), NULL);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
char *mbs_safe_encode_to_buffer(const char *s, size_t *width, char *buf, size_t bufsiz, const char *safechars)
{
	const char *p = s;
	char *r;
	số_nguyên rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, sizeof(st));
#endif
	nếu (!sz || !buf || !bufsiz)
		trả_về NULL;

	r = buf;
	rsz = (số_nguyên) bufsiz;
	*width = 0;

	trong_khi (p && *p) {
		nếu (safechars && strchr(safechars, *p)) {
			nếu (rsz < 2)
				break;
			*r++ = *p++;
			rsz--;
			continue;
		}

		nếu ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned char) *p)) {
			nếu (rsz < 5)
				break;
			snprintf(r, rsz, "\\x%02x", (unsigned char) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
			p++;
		}
#ifdef HAVE_WIDECHAR
		không_thì {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			nếu (len == 0)
				break;		/* end of string */

			nếu (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				nếu (!isprint((unsigned char) *p)) {
					nếu (rsz < 5)
						break;
					snprintf(r, rsz, "\\x%02x", (unsigned char) *p);
					r += 4;
					rsz -= 4;
					*width += 4;
				} không_thì {
					nếu (rsz < 2)
						break;
					(*width)++;
					*r++ = *p;
					rsz--;
				}
			} không_thì nếu (!iswprint(wc)) {
				size_t i;
				cho (i = 0; i < len; i++) {
					nếu (rsz < 5)
						break;
					snprintf(r, rsz, "\\x%02x", (unsigned char) p[i]);
					r += 4;
					rsz -= 4;
					*width += 4;
				}
				nếu (i < len)
					break;
			} không_thì {
				nếu (rsz < (số_nguyên)len + 1)
					break;
				memcpy(r, p, len);
				r += len;
				rsz -= len;
				*width += wcwidth(wc);
			}
			p += len;
		}
#không_thì
		không_thì nếu (!isprint((unsigned char) *p)) {
			nếu (rsz < 5)
				break;
			snprintf(r, rsz, "\\x%02x", (unsigned char) *p);
			p++;
			r += 4;
			rsz -= 4;
			*width += 4;
		} không_thì {
			nếu (rsz < 2)
				break;
			*r++ = *p++;
			rsz--;
			(*width)++;
		}
#endif
	}

	*r = '\0';
	trả_về buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
char *mbs_invalid_encode_to_buffer(const char *s, size_t *width, char *buf, size_t bufsiz)
{
	const char *p = s;
	char *r;
	số_nguyên rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, sizeof(st));
#endif
	nếu (!sz || !buf || !bufsiz)
		trả_về NULL;

	r = buf;
	rsz = (số_nguyên) bufsiz;
	*width = 0;

	trong_khi (p && *p) {
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#không_thì
		size_t len = 1;
#endif

		nếu (len == 0)
			break;		/* end of string */

		nếu (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			nếu (!isprint((unsigned char) *p)) {
				nếu (rsz < 5)
					break;
				snprintf(r, rsz, "\\x%02x", (unsigned char) *p);
				r += 4;
				rsz -= 4;
				*width += 4;
			} không_thì {
				nếu (rsz < 2)
					break;
				(*width)++;
				*r++ = *p;
				rsz--;
			}
		} không_thì nếu (*p == '\\' && *(p + 1) == 'x') {
			nếu (rsz < 5)
				break;
			snprintf(r, rsz, "\\x%02x", (unsigned char) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
		} không_thì {
			nếu (rsz < (số_nguyên)len + 1)
				break;
			r = mempcpy(r, p, len);
			rsz -= len;
			*width += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	trả_về buf;
}

/*
 * Guess size
 */
size_t mbs_safe_encode_size(size_t bytes)
{
	trả_về (bytes * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_safe_decode_size(const char *p)
{
	size_t bytes = 0;

	trong_khi (p && *p) {
		nếu (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		không_thì
			p++;
		bytes++;
	}
	trả_về bytes;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
char *mbs_safe_encode(const char *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	char *buf, *ret = NULL;

	nếu (!sz)
		trả_về NULL;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	nếu (buf)
		ret = mbs_safe_encode_to_buffer(s, width, buf, bufsz, NULL);
	nếu (!ret)
		free(buf);
	trả_về ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
char *mbs_invalid_encode(const char *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	char *buf, *ret = NULL;

	nếu (!sz)
		trả_về NULL;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	nếu (buf)
		ret = mbs_invalid_encode_to_buffer(s, width, buf, bufsz);
	nếu (!ret)
		free(buf);
	trả_về ret;
}

#ifdef HAVE_WIDECHAR

static bool
wc_ensure_printable (wchar_t *wchars)
{
  bool replaced = false;
  wchar_t *wc = wchars;
  trong_khi (*wc)
    {
      nếu (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  trả_về replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
static wchar_t *
wc_skip_ansi_escape (wchar_t *wc)
{
  wchar_t *p = wc + 1;

  nếu (*wc != L'\033' || !*p)
    trả_về NULL;

  nếu (*p == L'[')
    {
      p++;
      trong_khi (*p >= 0x30 && *p <= 0x3f)
        p++;
      trong_khi (*p >= 0x20 && *p <= 0x2f)
        p++;
      nếu (*p >= 0x40 && *p <= 0x7e)
        trả_về p + 1;
    }
  không_thì nếu ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    trả_về p + 2;

  trả_về NULL;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

static size_t
wc_truncate (wchar_t *wc, size_t width)
{
  size_t cells = 0;
  số_nguyên next_cells = 0;

  trong_khi (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      nếu (after_escape)
        {
          wc = after_escape;
          continue;
        }

      next_cells = wcwidth (*wc);
      nếu (next_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          next_cells = 1;
        }
      nếu (cells + next_cells > width)
        break;

      cells += next_cells;
      wc++;
    }
  *wc = L'\0';
  trả_về cells;
}

static số_nguyên
rpl_wcswidth (const wchar_t *s, size_t n)
{
  số_nguyên ret = 0;

  trong_khi (n-- > 0 && *s != L'\0')
    {
      số_nguyên nwidth = wcwidth (*s++);
      nếu (nwidth == -1)             /* non printable */
        trả_về -1;
      nếu (ret > (INT_MAX - nwidth)) /* overflow */
        trả_về -1;
      ret += nwidth;
    }

  trả_về ret;
}
#endif /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
size_t
mbs_truncate(char *str, size_t *width)
{
	ssize_t bytes = strlen(str);
#ifdef HAVE_WIDECHAR
	ssize_t sz = mbstowcs(NULL, str, 0);
	wchar_t *wcs = NULL;

	nếu (sz == (ssize_t) -1)
		goto done;

	wcs = calloc(1, (sz + 1) * sizeof(wchar_t));
	nếu (!wcs)
		goto done;

	nếu (!mbstowcs(wcs, str, sz))
		goto done;
	*width = wc_truncate(wcs, *width);
	bytes = wcstombs(str, wcs, bytes);
done:
	free(wcs);
#không_thì
	{
		const char *p = str;
		const char *end = str + bytes;
		size_t cells = 0;

		trong_khi (p < end) {
			const char *after_escape = skip_ansi_escape(p, end);

			nếu (after_escape) {
				p = after_escape;
				continue;
			}
			nếu (cells >= *width)
				break;
			p++;
			cells++;
		}
		bytes = p - str;
		*width = cells;
	}
#endif
	nếu (bytes >= 0)
		str[bytes] = '\0';
	trả_về bytes;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

static char*
mbs_align_pad (char *dest, const char* dest_end, size_t n_spaces, số_nguyên padchar)
{
  cho (/* nothing */; n_spaces && (dest < dest_end); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  trả_về dest;
}

size_t
mbsalign (const char *src, char *dest, size_t dest_size,
          size_t *width, mbs_align_t align, số_nguyên flags)
{
	trả_về mbsalign_with_padding(src, dest, dest_size, width, align, flags, ' ');
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
mbsalign_with_padding (const char *src, char *dest, size_t dest_size,
	               size_t *width, mbs_align_t align,
#ifdef HAVE_WIDECHAR
		       số_nguyên flags,
#không_thì
		       số_nguyên flags __attribute__((__unused__)),
#endif
		       số_nguyên padchar)
{
  size_t ret = -1;
  size_t src_size = strlen (src) + 1;
  char *newstr = NULL;
  wchar_t *str_wc = NULL;
  const char *str_to_print = src;
  size_t n_cols = src_size - 1;
  size_t n_used_bytes = n_cols; /* Not including NUL */
  size_t n_spaces = 0, space_left;

#ifdef HAVE_WIDECHAR
  bool conversion = false;
  bool wc_enabled = false;

  /* In multi-byte locales convert to wide characters
     to allow easy truncation. Also determine number
     of screen columns used.  */
  nếu (MB_CUR_MAX > 1)
    {
      size_t src_chars = mbstowcs (NULL, src, 0);
      nếu (src_chars == (size_t) -1)
        {
          nếu (flags & MBA_UNIBYTE_FALLBACK)
            goto mbsalign_unibyte;
          không_thì
            goto mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = malloc (src_chars * sizeof (wchar_t));
      nếu (str_wc == NULL)
        {
          nếu (flags & MBA_UNIBYTE_FALLBACK)
            goto mbsalign_unibyte;
          không_thì
            goto mbsalign_cleanup;
        }
      nếu (mbstowcs (str_wc, src, src_chars) != 0)
        {
          str_wc[src_chars - 1] = L'\0';
          wc_enabled = true;
          conversion = wc_ensure_printable (str_wc);
          n_cols = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  nếu (wc_enabled && (conversion || (n_cols > *width)))
    {
        nếu (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_size = wcstombs(NULL, str_wc, 0) + 1;
          }
        newstr = malloc (src_size);
        nếu (newstr == NULL)
        {
          nếu (flags & MBA_UNIBYTE_FALLBACK)
            goto mbsalign_unibyte;
          không_thì
            goto mbsalign_cleanup;
        }
        str_to_print = newstr;
        n_cols = wc_truncate (str_wc, *width);
        n_used_bytes = wcstombs (newstr, str_wc, src_size);
    }

mbsalign_unibyte:
#endif

  nếu (n_cols > *width) /* Unibyte truncation required.  */
    {
      n_cols = *width;
      n_used_bytes = n_cols;
    }

  nếu (*width > n_cols) /* Padding required.  */
    n_spaces = *width - n_cols;

  /* indicate to caller how many cells needed (not including padding).  */
  *width = n_cols;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_used_bytes + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  nếu (dest_size != 0)
    {
      char *dest_end = dest + dest_size - 1;
      size_t start_spaces;
      size_t end_spaces;

      switch (align)
        {
        case MBS_ALIGN_CENTER:
          start_spaces = n_spaces / 2 + n_spaces % 2;
          end_spaces = n_spaces / 2;
          break;
        case MBS_ALIGN_LEFT:
          start_spaces = 0;
          end_spaces = n_spaces;
          break;
        case MBS_ALIGN_RIGHT:
          start_spaces = n_spaces;
          end_spaces = 0;
          break;
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

  trả_về ret;
}
