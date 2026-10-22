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

	ако (p >= end || (unsigned char) *p != '\033' || p + 1 >= end)
		върни NULL;

	q = p + 1;
	ако (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		докато (q < end && (unsigned char) *q >= 0x30
				&& (unsigned char) *q <= 0x3f)
			q++;
		докато (q < end && (unsigned char) *q >= 0x20
				&& (unsigned char) *q <= 0x2f)
			q++;
		ако (q < end && (unsigned char) *q >= 0x40
				&& (unsigned char) *q <= 0x7e)
			върни q + 1;
	} иначе ако (*q && strchr("()*+", *q) && q + 1 < end)
		върни q + 2;

	върни NULL;
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
	ако (p && *p && bufsz)
		last = p + (bufsz - 1);

	докато (p && *p && p <= last) {
		ако (*p == '\033') {
			const char *e = skip_ansi_escape(p, last + 1);

			p = e ? e : p + 1;
			continue;
		}
		ако (iscntrl((unsigned char) *p)) {
			p++;
			continue;
		}
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		ако (len == 0)
			break;
		ако (len > 0 && iswprint(wc)) {
			цяло x = wcwidth(wc);
			ако (x > 0)
				width += x;
		} иначе ако (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#иначе
		ако (isprint((unsigned char) *p))
			width++;
		p++;
#endif
	}

	върни width;
}

size_t mbs_width(const char *s)
{
	ако (!s || !*s)
		върни 0;
	върни mbs_nwidth(s, strlen(s));
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
	ако (p && *p && bufsz)
		last = p + (bufsz - 1);

	докато (p && *p && p <= last) {
		ако ((p < last && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned char) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		}
#ifdef HAVE_WIDECHAR
		иначе {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			ако (len == 0)
				break;

			ако (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				ако (isprint((unsigned char) *p))
					width += 1, bytes += 1;
				иначе
					width += 4, bytes += 4;

			} иначе ако (!iswprint(wc)) {
				width += len * 4;	/* hex encode whole sequence */
				bytes += len * 4;
			} иначе {
				width += wcwidth(wc);	/* number of cells */
				bytes += len;		/* number of bytes */
			}
			p += len;
		}
#иначе
		иначе ако (!isprint((unsigned char) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		} иначе {
			width++, bytes++;
			p++;
		}
#endif
	}

	ако (sz)
		*sz = bytes;
	върни width;
}

size_t mbs_safe_width(const char *s)
{
	ако (!s || !*s)
		върни 0;
	върни mbs_safe_nwidth(s, strlen(s), NULL);
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
	цяло rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, sizeof(st));
#endif
	ако (!sz || !buf || !bufsiz)
		върни NULL;

	r = buf;
	rsz = (цяло) bufsiz;
	*width = 0;

	докато (p && *p) {
		ако (safechars && strchr(safechars, *p)) {
			ако (rsz < 2)
				break;
			*r++ = *p++;
			rsz--;
			continue;
		}

		ако ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned char) *p)) {
			ако (rsz < 5)
				break;
			snprintf(r, rsz, "\\x%02x", (unsigned char) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
			p++;
		}
#ifdef HAVE_WIDECHAR
		иначе {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			ако (len == 0)
				break;		/* end of string */

			ако (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				ако (!isprint((unsigned char) *p)) {
					ако (rsz < 5)
						break;
					snprintf(r, rsz, "\\x%02x", (unsigned char) *p);
					r += 4;
					rsz -= 4;
					*width += 4;
				} иначе {
					ако (rsz < 2)
						break;
					(*width)++;
					*r++ = *p;
					rsz--;
				}
			} иначе ако (!iswprint(wc)) {
				size_t i;
				за (i = 0; i < len; i++) {
					ако (rsz < 5)
						break;
					snprintf(r, rsz, "\\x%02x", (unsigned char) p[i]);
					r += 4;
					rsz -= 4;
					*width += 4;
				}
				ако (i < len)
					break;
			} иначе {
				ако (rsz < (цяло)len + 1)
					break;
				memcpy(r, p, len);
				r += len;
				rsz -= len;
				*width += wcwidth(wc);
			}
			p += len;
		}
#иначе
		иначе ако (!isprint((unsigned char) *p)) {
			ако (rsz < 5)
				break;
			snprintf(r, rsz, "\\x%02x", (unsigned char) *p);
			p++;
			r += 4;
			rsz -= 4;
			*width += 4;
		} иначе {
			ако (rsz < 2)
				break;
			*r++ = *p++;
			rsz--;
			(*width)++;
		}
#endif
	}

	*r = '\0';
	върни buf;
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
	цяло rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, sizeof(st));
#endif
	ако (!sz || !buf || !bufsiz)
		върни NULL;

	r = buf;
	rsz = (цяло) bufsiz;
	*width = 0;

	докато (p && *p) {
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#иначе
		size_t len = 1;
#endif

		ако (len == 0)
			break;		/* end of string */

		ако (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			ако (!isprint((unsigned char) *p)) {
				ако (rsz < 5)
					break;
				snprintf(r, rsz, "\\x%02x", (unsigned char) *p);
				r += 4;
				rsz -= 4;
				*width += 4;
			} иначе {
				ако (rsz < 2)
					break;
				(*width)++;
				*r++ = *p;
				rsz--;
			}
		} иначе ако (*p == '\\' && *(p + 1) == 'x') {
			ако (rsz < 5)
				break;
			snprintf(r, rsz, "\\x%02x", (unsigned char) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
		} иначе {
			ако (rsz < (цяло)len + 1)
				break;
			r = mempcpy(r, p, len);
			rsz -= len;
			*width += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	върни buf;
}

/*
 * Guess size
 */
size_t mbs_safe_encode_size(size_t bytes)
{
	върни (bytes * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_safe_decode_size(const char *p)
{
	size_t bytes = 0;

	докато (p && *p) {
		ако (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		иначе
			p++;
		bytes++;
	}
	върни bytes;
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

	ако (!sz)
		върни NULL;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	ако (buf)
		ret = mbs_safe_encode_to_buffer(s, width, buf, bufsz, NULL);
	ако (!ret)
		free(buf);
	върни ret;
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

	ако (!sz)
		върни NULL;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	ако (buf)
		ret = mbs_invalid_encode_to_buffer(s, width, buf, bufsz);
	ако (!ret)
		free(buf);
	върни ret;
}

#ifdef HAVE_WIDECHAR

static bool
wc_ensure_printable (wchar_t *wchars)
{
  bool replaced = false;
  wchar_t *wc = wchars;
  докато (*wc)
    {
      ако (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  върни replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
static wchar_t *
wc_skip_ansi_escape (wchar_t *wc)
{
  wchar_t *p = wc + 1;

  ако (*wc != L'\033' || !*p)
    върни NULL;

  ако (*p == L'[')
    {
      p++;
      докато (*p >= 0x30 && *p <= 0x3f)
        p++;
      докато (*p >= 0x20 && *p <= 0x2f)
        p++;
      ако (*p >= 0x40 && *p <= 0x7e)
        върни p + 1;
    }
  иначе ако ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    върни p + 2;

  върни NULL;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

static size_t
wc_truncate (wchar_t *wc, size_t width)
{
  size_t cells = 0;
  цяло next_cells = 0;

  докато (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      ако (after_escape)
        {
          wc = after_escape;
          continue;
        }

      next_cells = wcwidth (*wc);
      ако (next_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          next_cells = 1;
        }
      ако (cells + next_cells > width)
        break;

      cells += next_cells;
      wc++;
    }
  *wc = L'\0';
  върни cells;
}

static цяло
rpl_wcswidth (const wchar_t *s, size_t n)
{
  цяло ret = 0;

  докато (n-- > 0 && *s != L'\0')
    {
      цяло nwidth = wcwidth (*s++);
      ако (nwidth == -1)             /* non printable */
        върни -1;
      ако (ret > (INT_MAX - nwidth)) /* overflow */
        върни -1;
      ret += nwidth;
    }

  върни ret;
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

	ако (sz == (ssize_t) -1)
		goto done;

	wcs = calloc(1, (sz + 1) * sizeof(wchar_t));
	ако (!wcs)
		goto done;

	ако (!mbstowcs(wcs, str, sz))
		goto done;
	*width = wc_truncate(wcs, *width);
	bytes = wcstombs(str, wcs, bytes);
done:
	free(wcs);
#иначе
	{
		const char *p = str;
		const char *end = str + bytes;
		size_t cells = 0;

		докато (p < end) {
			const char *after_escape = skip_ansi_escape(p, end);

			ако (after_escape) {
				p = after_escape;
				continue;
			}
			ако (cells >= *width)
				break;
			p++;
			cells++;
		}
		bytes = p - str;
		*width = cells;
	}
#endif
	ако (bytes >= 0)
		str[bytes] = '\0';
	върни bytes;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

static char*
mbs_align_pad (char *dest, const char* dest_end, size_t n_spaces, цяло padchar)
{
  за (/* nothing */; n_spaces && (dest < dest_end); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  върни dest;
}

size_t
mbsalign (const char *src, char *dest, size_t dest_size,
          size_t *width, mbs_align_t align, цяло flags)
{
	върни mbsalign_with_padding(src, dest, dest_size, width, align, flags, ' ');
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
		       цяло flags,
#иначе
		       цяло flags __attribute__((__unused__)),
#endif
		       цяло padchar)
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
  ако (MB_CUR_MAX > 1)
    {
      size_t src_chars = mbstowcs (NULL, src, 0);
      ако (src_chars == (size_t) -1)
        {
          ако (flags & MBA_UNIBYTE_FALLBACK)
            goto mbsalign_unibyte;
          иначе
            goto mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = malloc (src_chars * sizeof (wchar_t));
      ако (str_wc == NULL)
        {
          ако (flags & MBA_UNIBYTE_FALLBACK)
            goto mbsalign_unibyte;
          иначе
            goto mbsalign_cleanup;
        }
      ако (mbstowcs (str_wc, src, src_chars) != 0)
        {
          str_wc[src_chars - 1] = L'\0';
          wc_enabled = true;
          conversion = wc_ensure_printable (str_wc);
          n_cols = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  ако (wc_enabled && (conversion || (n_cols > *width)))
    {
        ако (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_size = wcstombs(NULL, str_wc, 0) + 1;
          }
        newstr = malloc (src_size);
        ако (newstr == NULL)
        {
          ако (flags & MBA_UNIBYTE_FALLBACK)
            goto mbsalign_unibyte;
          иначе
            goto mbsalign_cleanup;
        }
        str_to_print = newstr;
        n_cols = wc_truncate (str_wc, *width);
        n_used_bytes = wcstombs (newstr, str_wc, src_size);
    }

mbsalign_unibyte:
#endif

  ако (n_cols > *width) /* Unibyte truncation required.  */
    {
      n_cols = *width;
      n_used_bytes = n_cols;
    }

  ако (*width > n_cols) /* Padding required.  */
    n_spaces = *width - n_cols;

  /* indicate to caller how many cells needed (not including padding).  */
  *width = n_cols;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_used_bytes + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  ако (dest_size != 0)
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

  върни ret;
}
