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
ساكن ثابت حرف *skip_ansi_escape(ثابت حرف *p, ثابت حرف *end)
{
	ثابت حرف *q;

	إذا (p >= end || (unsigned حرف) *p != '\033' || p + 1 >= end)
		أرجع عدم;

	q = p + 1;
	إذا (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		طالما (q < end && (unsigned حرف) *q >= 0x30
				&& (unsigned حرف) *q <= 0x3f)
			q++;
		طالما (q < end && (unsigned حرف) *q >= 0x20
				&& (unsigned حرف) *q <= 0x2f)
			q++;
		إذا (q < end && (unsigned حرف) *q >= 0x40
				&& (unsigned حرف) *q <= 0x7e)
			أرجع q + 1;
	} وإلا إذا (*q && strchr("()*+", *q) && q + 1 < end)
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
	ثابت حرف *p = buf, *last = buf;
	size_t width = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, حجم(st));
#endif
	إذا (p && *p && bufsz)
		last = p + (bufsz - 1);

	طالما (p && *p && p <= last) {
		إذا (*p == '\033') {
			ثابت حرف *e = skip_ansi_escape(p, last + 1);

			p = e ? e : p + 1;
			استمر;
		}
		إذا (iscntrl((unsigned حرف) *p)) {
			p++;
			استمر;
		}
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		إذا (len == 0)
			اكسر;
		إذا (len > 0 && iswprint(wc)) {
			صحيح x = wcwidth(wc);
			إذا (x > 0)
				width += x;
		} وإلا إذا (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#وإلا
		إذا (isprint((unsigned حرف) *p))
			width++;
		p++;
#endif
	}

	أرجع width;
}

size_t mbs_width(ثابت حرف *s)
{
	إذا (!s || !*s)
		أرجع 0;
	أرجع mbs_nwidth(s, strlen(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
size_t mbs_safe_nwidth(ثابت حرف *buf, size_t bufsz, size_t *sz)
{
	ثابت حرف *p = buf, *last = buf;
	size_t width = 0, bytes = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, حجم(st));
#endif
	إذا (p && *p && bufsz)
		last = p + (bufsz - 1);

	طالما (p && *p && p <= last) {
		إذا ((p < last && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned حرف) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		}
#ifdef HAVE_WIDECHAR
		وإلا {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			إذا (len == 0)
				اكسر;

			إذا (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				إذا (isprint((unsigned حرف) *p))
					width += 1, bytes += 1;
				وإلا
					width += 4, bytes += 4;

			} وإلا إذا (!iswprint(wc)) {
				width += len * 4;	/* hex encode whole sequence */
				bytes += len * 4;
			} وإلا {
				width += wcwidth(wc);	/* number of cells */
				bytes += len;		/* number of bytes */
			}
			p += len;
		}
#وإلا
		وإلا إذا (!isprint((unsigned حرف) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		} وإلا {
			width++, bytes++;
			p++;
		}
#endif
	}

	إذا (sz)
		*sz = bytes;
	أرجع width;
}

size_t mbs_safe_width(ثابت حرف *s)
{
	إذا (!s || !*s)
		أرجع 0;
	أرجع mbs_safe_nwidth(s, strlen(s), عدم);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
حرف *mbs_safe_encode_to_buffer(ثابت حرف *s, size_t *width, حرف *buf, size_t bufsiz, ثابت حرف *safechars)
{
	ثابت حرف *p = s;
	حرف *r;
	صحيح rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, حجم(st));
#endif
	إذا (!sz || !buf || !bufsiz)
		أرجع عدم;

	r = buf;
	rsz = (صحيح) bufsiz;
	*width = 0;

	طالما (p && *p) {
		إذا (safechars && strchr(safechars, *p)) {
			إذا (rsz < 2)
				اكسر;
			*r++ = *p++;
			rsz--;
			استمر;
		}

		إذا ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned حرف) *p)) {
			إذا (rsz < 5)
				اكسر;
			snprintf(r, rsz, "\\x%02x", (unsigned حرف) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
			p++;
		}
#ifdef HAVE_WIDECHAR
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
				إذا (!isprint((unsigned حرف) *p)) {
					إذا (rsz < 5)
						اكسر;
					snprintf(r, rsz, "\\x%02x", (unsigned حرف) *p);
					r += 4;
					rsz -= 4;
					*width += 4;
				} وإلا {
					إذا (rsz < 2)
						اكسر;
					(*width)++;
					*r++ = *p;
					rsz--;
				}
			} وإلا إذا (!iswprint(wc)) {
				size_t i;
				لكل (i = 0; i < len; i++) {
					إذا (rsz < 5)
						اكسر;
					snprintf(r, rsz, "\\x%02x", (unsigned حرف) p[i]);
					r += 4;
					rsz -= 4;
					*width += 4;
				}
				إذا (i < len)
					اكسر;
			} وإلا {
				إذا (rsz < (صحيح)len + 1)
					اكسر;
				memcpy(r, p, len);
				r += len;
				rsz -= len;
				*width += wcwidth(wc);
			}
			p += len;
		}
#وإلا
		وإلا إذا (!isprint((unsigned حرف) *p)) {
			إذا (rsz < 5)
				اكسر;
			snprintf(r, rsz, "\\x%02x", (unsigned حرف) *p);
			p++;
			r += 4;
			rsz -= 4;
			*width += 4;
		} وإلا {
			إذا (rsz < 2)
				اكسر;
			*r++ = *p++;
			rsz--;
			(*width)++;
		}
#endif
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
حرف *mbs_invalid_encode_to_buffer(ثابت حرف *s, size_t *width, حرف *buf, size_t bufsiz)
{
	ثابت حرف *p = s;
	حرف *r;
	صحيح rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, حجم(st));
#endif
	إذا (!sz || !buf || !bufsiz)
		أرجع عدم;

	r = buf;
	rsz = (صحيح) bufsiz;
	*width = 0;

	طالما (p && *p) {
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#وإلا
		size_t len = 1;
#endif

		إذا (len == 0)
			اكسر;		/* end of string */

		إذا (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			إذا (!isprint((unsigned حرف) *p)) {
				إذا (rsz < 5)
					اكسر;
				snprintf(r, rsz, "\\x%02x", (unsigned حرف) *p);
				r += 4;
				rsz -= 4;
				*width += 4;
			} وإلا {
				إذا (rsz < 2)
					اكسر;
				(*width)++;
				*r++ = *p;
				rsz--;
			}
		} وإلا إذا (*p == '\\' && *(p + 1) == 'x') {
			إذا (rsz < 5)
				اكسر;
			snprintf(r, rsz, "\\x%02x", (unsigned حرف) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
		} وإلا {
			إذا (rsz < (صحيح)len + 1)
				اكسر;
			r = mempcpy(r, p, len);
			rsz -= len;
			*width += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	أرجع buf;
}

/*
 * Guess size
 */
size_t mbs_safe_encode_size(size_t bytes)
{
	أرجع (bytes * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_safe_decode_size(ثابت حرف *p)
{
	size_t bytes = 0;

	طالما (p && *p) {
		إذا (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		وإلا
			p++;
		bytes++;
	}
	أرجع bytes;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
حرف *mbs_safe_encode(ثابت حرف *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	حرف *buf, *ret = عدم;

	إذا (!sz)
		أرجع عدم;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	إذا (buf)
		ret = mbs_safe_encode_to_buffer(s, width, buf, bufsz, عدم);
	إذا (!ret)
		حرر(buf);
	أرجع ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
حرف *mbs_invalid_encode(ثابت حرف *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	حرف *buf, *ret = عدم;

	إذا (!sz)
		أرجع عدم;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	إذا (buf)
		ret = mbs_invalid_encode_to_buffer(s, width, buf, bufsz);
	إذا (!ret)
		حرر(buf);
	أرجع ret;
}

#ifdef HAVE_WIDECHAR

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
wc_truncate (wchar_t *wc, size_t width)
{
  size_t cells = 0;
  صحيح next_cells = 0;

  طالما (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      إذا (after_escape)
        {
          wc = after_escape;
          استمر;
        }

      next_cells = wcwidth (*wc);
      إذا (next_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          next_cells = 1;
        }
      إذا (cells + next_cells > width)
        اكسر;

      cells += next_cells;
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
      إذا (ret > (INT_MAX - nwidth)) /* overflow */
        أرجع -1;
      ret += nwidth;
    }

  أرجع ret;
}
#endif /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
size_t
mbs_truncate(حرف *str, size_t *width)
{
	ssize_t bytes = strlen(str);
#ifdef HAVE_WIDECHAR
	ssize_t sz = mbstowcs(عدم, str, 0);
	wchar_t *wcs = عدم;

	إذا (sz == (ssize_t) -1)
		اذهب done;

	wcs = calloc(1, (sz + 1) * حجم(wchar_t));
	إذا (!wcs)
		اذهب done;

	إذا (!mbstowcs(wcs, str, sz))
		اذهب done;
	*width = wc_truncate(wcs, *width);
	bytes = wcstombs(str, wcs, bytes);
done:
	حرر(wcs);
#وإلا
	{
		ثابت حرف *p = str;
		ثابت حرف *end = str + bytes;
		size_t cells = 0;

		طالما (p < end) {
			ثابت حرف *after_escape = skip_ansi_escape(p, end);

			إذا (after_escape) {
				p = after_escape;
				استمر;
			}
			إذا (cells >= *width)
				اكسر;
			p++;
			cells++;
		}
		bytes = p - str;
		*width = cells;
	}
#endif
	إذا (bytes >= 0)
		str[bytes] = '\0';
	أرجع bytes;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

ساكن حرف*
mbs_align_pad (حرف *dest, ثابت حرف* dest_end, size_t n_spaces, صحيح padchar)
{
  لكل (/* nothing */; n_spaces && (dest < dest_end); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  أرجع dest;
}

size_t
mbsalign (ثابت حرف *src, حرف *dest, size_t dest_size,
          size_t *width, mbs_align_t align, صحيح flags)
{
	أرجع mbsalign_with_padding(src, dest, dest_size, width, align, flags, ' ');
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
mbsalign_with_padding (ثابت حرف *src, حرف *dest, size_t dest_size,
	               size_t *width, mbs_align_t align,
#ifdef HAVE_WIDECHAR
		       صحيح flags,
#وإلا
		       صحيح flags __attribute__((__unused__)),
#endif
		       صحيح padchar)
{
  size_t ret = -1;
  size_t src_size = strlen (src) + 1;
  حرف *newstr = عدم;
  wchar_t *str_wc = عدم;
  ثابت حرف *str_to_print = src;
  size_t n_cols = src_size - 1;
  size_t n_used_bytes = n_cols; /* Not including NUL */
  size_t n_spaces = 0, space_کيري;

#ifdef HAVE_WIDECHAR
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
      str_wc = malloc (src_chars * حجم (wchar_t));
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
          n_cols = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  إذا (wc_enabled && (conversion || (n_cols > *width)))
    {
        إذا (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_size = wcstombs(عدم, str_wc, 0) + 1;
          }
        newstr = malloc (src_size);
        إذا (newstr == عدم)
        {
          إذا (flags & MBA_UNIBYTE_FALLBACK)
            اذهب mbsalign_unibyte;
          وإلا
            اذهب mbsalign_cleanup;
        }
        str_to_print = newstr;
        n_cols = wc_truncate (str_wc, *width);
        n_used_bytes = wcstombs (newstr, str_wc, src_size);
    }

mbsalign_unibyte:
#endif

  إذا (n_cols > *width) /* Unibyte truncation required.  */
    {
      n_cols = *width;
      n_used_bytes = n_cols;
    }

  إذا (*width > n_cols) /* Padding required.  */
    n_spaces = *width - n_cols;

  /* indicate to caller how many cells needed (not including padding).  */
  *width = n_cols;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_used_bytes + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  إذا (dest_size != 0)
    {
      حرف *dest_end = dest + dest_size - 1;
      size_t start_spaces;
      size_t end_spaces;

      بدل (align)
        {
        حالة MBS_ALIGN_CENTER:
          start_spaces = n_spaces / 2 + n_spaces % 2;
          end_spaces = n_spaces / 2;
          اكسر;
        حالة MBS_ALIGN_کيري:
          start_spaces = 0;
          end_spaces = n_spaces;
          اكسر;
        حالة MBS_ALIGN_کانن:
          start_spaces = n_spaces;
          end_spaces = 0;
          اكسر;
	افتراضي:
	  abort();
        }

      dest = mbs_align_pad (dest, dest_end, start_spaces, padchar);
      space_کيري = dest_end - dest;
      dest = mempcpy (dest, str_to_print, min (n_used_bytes, space_کيري));
      mbs_align_pad (dest, dest_end, end_spaces, padchar);
    }
#ifdef HAVE_WIDECHAR
mbsalign_cleanup:
#endif
  حرر (str_wc);
  حرر (newstr);

  أرجع ret;
}
