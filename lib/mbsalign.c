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
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "mbsalign.h"
#ଅନ୍ତର୍ଭାଵ "strutils.h"
#ଅନ୍ତର୍ଭାଵ "widechar.h"

/* Return the byte after a complete ANSI escape sequence, or NULL. */
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *skip_ansi_escape(ଅଚଳ ଅକ୍ଷର *p, ଅଚଳ ଅକ୍ଷର *end)
{
	ଅଚଳ ଅକ୍ଷର *q;

	ଯଦି (p >= end || (unsigned ଅକ୍ଷର) *p != '\033' || p + 1 >= end)
		ଫେରନ୍ତୁ ଖାଲି;

	q = p + 1;
	ଯଦି (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		ଯେତେବେଳେ (q < end && (unsigned ଅକ୍ଷର) *q >= 0x30
				&& (unsigned ଅକ୍ଷର) *q <= 0x3f)
			q++;
		ଯେତେବେଳେ (q < end && (unsigned ଅକ୍ଷର) *q >= 0x20
				&& (unsigned ଅକ୍ଷର) *q <= 0x2f)
			q++;
		ଯଦି (q < end && (unsigned ଅକ୍ଷର) *q >= 0x40
				&& (unsigned ଅକ୍ଷର) *q <= 0x7e)
			ଫେରନ୍ତୁ q + 1;
	} ଅନ୍ୟଥା ଯଦି (*q && strchr("()*+", *q) && q + 1 < end)
		ଫେରନ୍ତୁ q + 2;

	ଫେରନ୍ତୁ ଖାଲି;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
size_t mbs_nwidth(ଅଚଳ ଅକ୍ଷର *buf, size_t bufsz)
{
	ଅଚଳ ଅକ୍ଷର *p = buf, *last = buf;
	size_t width = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, ଆକାର(st));
#endif
	ଯଦି (p && *p && bufsz)
		last = p + (bufsz - 1);

	ଯେତେବେଳେ (p && *p && p <= last) {
		ଯଦି (*p == '\033') {
			ଅଚଳ ଅକ୍ଷର *e = skip_ansi_escape(p, last + 1);

			p = e ? e : p + 1;
			ଜାରି;
		}
		ଯଦି (iscntrl((unsigned ଅକ୍ଷର) *p)) {
			p++;
			ଜାରି;
		}
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		ଯଦି (len == 0)
			ଭାଙ୍ଗ;
		ଯଦି (len > 0 && iswprint(wc)) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା x = wcwidth(wc);
			ଯଦି (x > 0)
				width += x;
		} ଅନ୍ୟଥା ଯଦି (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#ଅନ୍ୟଥା
		ଯଦି (isprint((unsigned ଅକ୍ଷର) *p))
			width++;
		p++;
#endif
	}

	ଫେରନ୍ତୁ width;
}

size_t mbs_width(ଅଚଳ ଅକ୍ଷର *s)
{
	ଯଦି (!s || !*s)
		ଫେରନ୍ତୁ 0;
	ଫେରନ୍ତୁ mbs_nwidth(s, strlen(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
size_t mbs_safe_nwidth(ଅଚଳ ଅକ୍ଷର *buf, size_t bufsz, size_t *sz)
{
	ଅଚଳ ଅକ୍ଷର *p = buf, *last = buf;
	size_t width = 0, bytes = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, ଆକାର(st));
#endif
	ଯଦି (p && *p && bufsz)
		last = p + (bufsz - 1);

	ଯେତେବେଳେ (p && *p && p <= last) {
		ଯଦି ((p < last && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned ଅକ୍ଷର) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		}
#ifdef HAVE_WIDECHAR
		ଅନ୍ୟଥା {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			ଯଦି (len == 0)
				ଭାଙ୍ଗ;

			ଯଦି (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				ଯଦି (isprint((unsigned ଅକ୍ଷର) *p))
					width += 1, bytes += 1;
				ଅନ୍ୟଥା
					width += 4, bytes += 4;

			} ଅନ୍ୟଥା ଯଦି (!iswprint(wc)) {
				width += len * 4;	/* hex encode whole sequence */
				bytes += len * 4;
			} ଅନ୍ୟଥା {
				width += wcwidth(wc);	/* number of cells */
				bytes += len;		/* number of bytes */
			}
			p += len;
		}
#ଅନ୍ୟଥା
		ଅନ୍ୟଥା ଯଦି (!isprint((unsigned ଅକ୍ଷର) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		} ଅନ୍ୟଥା {
			width++, bytes++;
			p++;
		}
#endif
	}

	ଯଦି (sz)
		*sz = bytes;
	ଫେରନ୍ତୁ width;
}

size_t mbs_safe_width(ଅଚଳ ଅକ୍ଷର *s)
{
	ଯଦି (!s || !*s)
		ଫେରନ୍ତୁ 0;
	ଫେରନ୍ତୁ mbs_safe_nwidth(s, strlen(s), ଖାଲି);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
ଅକ୍ଷର *mbs_safe_encode_to_buffer(ଅଚଳ ଅକ୍ଷର *s, size_t *width, ଅକ୍ଷର *buf, size_t bufsiz, ଅଚଳ ଅକ୍ଷର *safechars)
{
	ଅଚଳ ଅକ୍ଷର *p = s;
	ଅକ୍ଷର *r;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, ଆକାର(st));
#endif
	ଯଦି (!sz || !buf || !bufsiz)
		ଫେରନ୍ତୁ ଖାଲି;

	r = buf;
	rsz = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) bufsiz;
	*width = 0;

	ଯେତେବେଳେ (p && *p) {
		ଯଦି (safechars && strchr(safechars, *p)) {
			ଯଦି (rsz < 2)
				ଭାଙ୍ଗ;
			*r++ = *p++;
			rsz--;
			ଜାରି;
		}

		ଯଦି ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned ଅକ୍ଷର) *p)) {
			ଯଦି (rsz < 5)
				ଭାଙ୍ଗ;
			snprintf(r, rsz, "\\x%02x", (unsigned ଅକ୍ଷର) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
			p++;
		}
#ifdef HAVE_WIDECHAR
		ଅନ୍ୟଥା {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			ଯଦି (len == 0)
				ଭାଙ୍ଗ;		/* end of string */

			ଯଦି (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				ଯଦି (!isprint((unsigned ଅକ୍ଷର) *p)) {
					ଯଦି (rsz < 5)
						ଭାଙ୍ଗ;
					snprintf(r, rsz, "\\x%02x", (unsigned ଅକ୍ଷର) *p);
					r += 4;
					rsz -= 4;
					*width += 4;
				} ଅନ୍ୟଥା {
					ଯଦି (rsz < 2)
						ଭାଙ୍ଗ;
					(*width)++;
					*r++ = *p;
					rsz--;
				}
			} ଅନ୍ୟଥା ଯଦି (!iswprint(wc)) {
				size_t i;
				ପାଇଁ (i = 0; i < len; i++) {
					ଯଦି (rsz < 5)
						ଭାଙ୍ଗ;
					snprintf(r, rsz, "\\x%02x", (unsigned ଅକ୍ଷର) p[i]);
					r += 4;
					rsz -= 4;
					*width += 4;
				}
				ଯଦି (i < len)
					ଭାଙ୍ଗ;
			} ଅନ୍ୟଥା {
				ଯଦି (rsz < (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)len + 1)
					ଭାଙ୍ଗ;
				memcpy(r, p, len);
				r += len;
				rsz -= len;
				*width += wcwidth(wc);
			}
			p += len;
		}
#ଅନ୍ୟଥା
		ଅନ୍ୟଥା ଯଦି (!isprint((unsigned ଅକ୍ଷର) *p)) {
			ଯଦି (rsz < 5)
				ଭାଙ୍ଗ;
			snprintf(r, rsz, "\\x%02x", (unsigned ଅକ୍ଷର) *p);
			p++;
			r += 4;
			rsz -= 4;
			*width += 4;
		} ଅନ୍ୟଥା {
			ଯଦି (rsz < 2)
				ଭାଙ୍ଗ;
			*r++ = *p++;
			rsz--;
			(*width)++;
		}
#endif
	}

	*r = '\0';
	ଫେରନ୍ତୁ buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
ଅକ୍ଷର *mbs_invalid_encode_to_buffer(ଅଚଳ ଅକ୍ଷର *s, size_t *width, ଅକ୍ଷର *buf, size_t bufsiz)
{
	ଅଚଳ ଅକ୍ଷର *p = s;
	ଅକ୍ଷର *r;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, ଆକାର(st));
#endif
	ଯଦି (!sz || !buf || !bufsiz)
		ଫେରନ୍ତୁ ଖାଲି;

	r = buf;
	rsz = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) bufsiz;
	*width = 0;

	ଯେତେବେଳେ (p && *p) {
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#ଅନ୍ୟଥା
		size_t len = 1;
#endif

		ଯଦି (len == 0)
			ଭାଙ୍ଗ;		/* end of string */

		ଯଦି (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			ଯଦି (!isprint((unsigned ଅକ୍ଷର) *p)) {
				ଯଦି (rsz < 5)
					ଭାଙ୍ଗ;
				snprintf(r, rsz, "\\x%02x", (unsigned ଅକ୍ଷର) *p);
				r += 4;
				rsz -= 4;
				*width += 4;
			} ଅନ୍ୟଥା {
				ଯଦି (rsz < 2)
					ଭାଙ୍ଗ;
				(*width)++;
				*r++ = *p;
				rsz--;
			}
		} ଅନ୍ୟଥା ଯଦି (*p == '\\' && *(p + 1) == 'x') {
			ଯଦି (rsz < 5)
				ଭାଙ୍ଗ;
			snprintf(r, rsz, "\\x%02x", (unsigned ଅକ୍ଷର) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
		} ଅନ୍ୟଥା {
			ଯଦି (rsz < (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)len + 1)
				ଭାଙ୍ଗ;
			r = mempcpy(r, p, len);
			rsz -= len;
			*width += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	ଫେରନ୍ତୁ buf;
}

/*
 * Guess size
 */
size_t mbs_safe_encode_size(size_t bytes)
{
	ଫେରନ୍ତୁ (bytes * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_safe_decode_size(ଅଚଳ ଅକ୍ଷର *p)
{
	size_t bytes = 0;

	ଯେତେବେଳେ (p && *p) {
		ଯଦି (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		ଅନ୍ୟଥା
			p++;
		bytes++;
	}
	ଫେରନ୍ତୁ bytes;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
ଅକ୍ଷର *mbs_safe_encode(ଅଚଳ ଅକ୍ଷର *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	ଅକ୍ଷର *buf, *ret = ଖାଲି;

	ଯଦି (!sz)
		ଫେରନ୍ତୁ ଖାଲି;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	ଯଦି (buf)
		ret = mbs_safe_encode_to_buffer(s, width, buf, bufsz, ଖାଲି);
	ଯଦି (!ret)
		ମୁକ୍ତ(buf);
	ଫେରନ୍ତୁ ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
ଅକ୍ଷର *mbs_invalid_encode(ଅଚଳ ଅକ୍ଷର *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	ଅକ୍ଷର *buf, *ret = ଖାଲି;

	ଯଦି (!sz)
		ଫେରନ୍ତୁ ଖାଲି;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	ଯଦି (buf)
		ret = mbs_invalid_encode_to_buffer(s, width, buf, bufsz);
	ଯଦି (!ret)
		ମୁକ୍ତ(buf);
	ଫେରନ୍ତୁ ret;
}

#ifdef HAVE_WIDECHAR

ସ୍ଥିର bool
wc_ensure_printable (wchar_t *wchars)
{
  bool replaced = false;
  wchar_t *wc = wchars;
  ଯେତେବେଳେ (*wc)
    {
      ଯଦି (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  ଫେରନ୍ତୁ replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
ସ୍ଥିର wchar_t *
wc_skip_ansi_escape (wchar_t *wc)
{
  wchar_t *p = wc + 1;

  ଯଦି (*wc != L'\033' || !*p)
    ଫେରନ୍ତୁ ଖାଲି;

  ଯଦି (*p == L'[')
    {
      p++;
      ଯେତେବେଳେ (*p >= 0x30 && *p <= 0x3f)
        p++;
      ଯେତେବେଳେ (*p >= 0x20 && *p <= 0x2f)
        p++;
      ଯଦି (*p >= 0x40 && *p <= 0x7e)
        ଫେରନ୍ତୁ p + 1;
    }
  ଅନ୍ୟଥା ଯଦି ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    ଫେରନ୍ତୁ p + 2;

  ଫେରନ୍ତୁ ଖାଲି;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

ସ୍ଥିର size_t
wc_truncate (wchar_t *wc, size_t width)
{
  size_t cells = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା next_cells = 0;

  ଯେତେବେଳେ (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      ଯଦି (after_escape)
        {
          wc = after_escape;
          ଜାରି;
        }

      next_cells = wcwidth (*wc);
      ଯଦି (next_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          next_cells = 1;
        }
      ଯଦି (cells + next_cells > width)
        ଭାଙ୍ଗ;

      cells += next_cells;
      wc++;
    }
  *wc = L'\0';
  ଫେରନ୍ତୁ cells;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
rpl_wcswidth (ଅଚଳ wchar_t *s, size_t n)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

  ଯେତେବେଳେ (n-- > 0 && *s != L'\0')
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nwidth = wcwidth (*s++);
      ଯଦି (nwidth == -1)             /* non printable */
        ଫେରନ୍ତୁ -1;
      ଯଦି (ret > (INT_MAX - nwidth)) /* overflow */
        ଫେରନ୍ତୁ -1;
      ret += nwidth;
    }

  ଫେରନ୍ତୁ ret;
}
#endif /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
size_t
mbs_truncate(ଅକ୍ଷର *str, size_t *width)
{
	ssize_t bytes = strlen(str);
#ifdef HAVE_WIDECHAR
	ssize_t sz = mbstowcs(ଖାଲି, str, 0);
	wchar_t *wcs = ଖାଲି;

	ଯଦି (sz == (ssize_t) -1)
		ଯାଅ done;

	wcs = calloc(1, (sz + 1) * ଆକାର(wchar_t));
	ଯଦି (!wcs)
		ଯାଅ done;

	ଯଦି (!mbstowcs(wcs, str, sz))
		ଯାଅ done;
	*width = wc_truncate(wcs, *width);
	bytes = wcstombs(str, wcs, bytes);
done:
	ମୁକ୍ତ(wcs);
#ଅନ୍ୟଥା
	{
		ଅଚଳ ଅକ୍ଷର *p = str;
		ଅଚଳ ଅକ୍ଷର *end = str + bytes;
		size_t cells = 0;

		ଯେତେବେଳେ (p < end) {
			ଅଚଳ ଅକ୍ଷର *after_escape = skip_ansi_escape(p, end);

			ଯଦି (after_escape) {
				p = after_escape;
				ଜାରି;
			}
			ଯଦି (cells >= *width)
				ଭାଙ୍ଗ;
			p++;
			cells++;
		}
		bytes = p - str;
		*width = cells;
	}
#endif
	ଯଦି (bytes >= 0)
		str[bytes] = '\0';
	ଫେରନ୍ତୁ bytes;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

ସ୍ଥିର ଅକ୍ଷର*
mbs_align_pad (ଅକ୍ଷର *dest, ଅଚଳ ଅକ୍ଷର* dest_end, size_t n_spaces, ପୂର୍ଣ୍ଣସଂଖ୍ୟା padchar)
{
  ପାଇଁ (/* nothing */; n_spaces && (dest < dest_end); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  ଫେରନ୍ତୁ dest;
}

size_t
mbsalign (ଅଚଳ ଅକ୍ଷର *src, ଅକ୍ଷର *dest, size_t dest_size,
          size_t *width, mbs_align_t align, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ଫେରନ୍ତୁ mbsalign_with_padding(src, dest, dest_size, width, align, flags, ' ');
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
mbsalign_with_padding (ଅଚଳ ଅକ୍ଷର *src, ଅକ୍ଷର *dest, size_t dest_size,
	               size_t *width, mbs_align_t align,
#ifdef HAVE_WIDECHAR
		       ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,
#ଅନ୍ୟଥା
		       ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags __attribute__((__unused__)),
#endif
		       ପୂର୍ଣ୍ଣସଂଖ୍ୟା padchar)
{
  size_t ret = -1;
  size_t src_size = strlen (src) + 1;
  ଅକ୍ଷର *newstr = ଖାଲି;
  wchar_t *str_wc = ଖାଲି;
  ଅଚଳ ଅକ୍ଷର *str_to_print = src;
  size_t n_cols = src_size - 1;
  size_t n_used_bytes = n_cols; /* Not including NUL */
  size_t n_spaces = 0, space_ବାମ;

#ifdef HAVE_WIDECHAR
  bool conversion = false;
  bool wc_enabled = false;

  /* In multi-byte locales convert to wide characters
     to allow easy truncation. Also determine number
     of screen columns used.  */
  ଯଦି (MB_CUR_MAX > 1)
    {
      size_t src_chars = mbstowcs (ଖାଲି, src, 0);
      ଯଦି (src_chars == (size_t) -1)
        {
          ଯଦି (flags & MBA_UNIBYTE_FALLBACK)
            ଯାଅ mbsalign_unibyte;
          ଅନ୍ୟଥା
            ଯାଅ mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = malloc (src_chars * ଆକାର (wchar_t));
      ଯଦି (str_wc == ଖାଲି)
        {
          ଯଦି (flags & MBA_UNIBYTE_FALLBACK)
            ଯାଅ mbsalign_unibyte;
          ଅନ୍ୟଥା
            ଯାଅ mbsalign_cleanup;
        }
      ଯଦି (mbstowcs (str_wc, src, src_chars) != 0)
        {
          str_wc[src_chars - 1] = L'\0';
          wc_enabled = true;
          conversion = wc_ensure_printable (str_wc);
          n_cols = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  ଯଦି (wc_enabled && (conversion || (n_cols > *width)))
    {
        ଯଦି (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_size = wcstombs(ଖାଲି, str_wc, 0) + 1;
          }
        newstr = malloc (src_size);
        ଯଦି (newstr == ଖାଲି)
        {
          ଯଦି (flags & MBA_UNIBYTE_FALLBACK)
            ଯାଅ mbsalign_unibyte;
          ଅନ୍ୟଥା
            ଯାଅ mbsalign_cleanup;
        }
        str_to_print = newstr;
        n_cols = wc_truncate (str_wc, *width);
        n_used_bytes = wcstombs (newstr, str_wc, src_size);
    }

mbsalign_unibyte:
#endif

  ଯଦି (n_cols > *width) /* Unibyte truncation required.  */
    {
      n_cols = *width;
      n_used_bytes = n_cols;
    }

  ଯଦି (*width > n_cols) /* Padding required.  */
    n_spaces = *width - n_cols;

  /* indicate to caller how many cells needed (not including padding).  */
  *width = n_cols;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_used_bytes + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  ଯଦି (dest_size != 0)
    {
      ଅକ୍ଷର *dest_end = dest + dest_size - 1;
      size_t start_spaces;
      size_t end_spaces;

      ବାଛ (align)
        {
        ସ୍ଥିତି MBS_ALIGN_CENTER:
          start_spaces = n_spaces / 2 + n_spaces % 2;
          end_spaces = n_spaces / 2;
          ଭାଙ୍ଗ;
        ସ୍ଥିତି MBS_ALIGN_ବାମ:
          start_spaces = 0;
          end_spaces = n_spaces;
          ଭାଙ୍ଗ;
        ସ୍ଥିତି MBS_ALIGN_ଡାହାଣ:
          start_spaces = n_spaces;
          end_spaces = 0;
          ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  abort();
        }

      dest = mbs_align_pad (dest, dest_end, start_spaces, padchar);
      space_ବାମ = dest_end - dest;
      dest = mempcpy (dest, str_to_print, min (n_used_bytes, space_ବାମ));
      mbs_align_pad (dest, dest_end, end_spaces, padchar);
    }
#ifdef HAVE_WIDECHAR
mbsalign_cleanup:
#endif
  ମୁକ୍ତ (str_wc);
  ମୁକ୍ତ (newstr);

  ଫେରନ୍ତୁ ret;
}
