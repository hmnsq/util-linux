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
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <ctype.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "mbsalign.h"
#অন্তর্ভাব "strutils.h"
#অন্তর্ভাব "widechar.h"

/* Return the byte after a complete ANSI escape sequence, or NULL. */
স্থির ধ্রুবক অক্ষর *skip_ansi_escape(ধ্রুবক অক্ষর *p, ধ্রুবক অক্ষর *end)
{
	ধ্রুবক অক্ষর *q;

	যদি (p >= end || (unsigned অক্ষর) *p != '\033' || p + 1 >= end)
		ফেরত ফাঁকা;

	q = p + 1;
	যদি (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		যখন (q < end && (unsigned অক্ষর) *q >= 0x30
				&& (unsigned অক্ষর) *q <= 0x3f)
			q++;
		যখন (q < end && (unsigned অক্ষর) *q >= 0x20
				&& (unsigned অক্ষর) *q <= 0x2f)
			q++;
		যদি (q < end && (unsigned অক্ষর) *q >= 0x40
				&& (unsigned অক্ষর) *q <= 0x7e)
			ফেরত q + 1;
	} অন্যথা যদি (*q && strchr("()*+", *q) && q + 1 < end)
		ফেরত q + 2;

	ফেরত ফাঁকা;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
size_t mbs_nwidth(ধ্রুবক অক্ষর *buf, size_t bufsz)
{
	ধ্রুবক অক্ষর *p = buf, *last = buf;
	size_t width = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, আকার(st));
#endif
	যদি (p && *p && bufsz)
		last = p + (bufsz - 1);

	যখন (p && *p && p <= last) {
		যদি (*p == '\033') {
			ধ্রুবক অক্ষর *e = skip_ansi_escape(p, last + 1);

			p = e ? e : p + 1;
			চালু;
		}
		যদি (iscntrl((unsigned অক্ষর) *p)) {
			p++;
			চালু;
		}
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		যদি (len == 0)
			বিরতি;
		যদি (len > 0 && iswprint(wc)) {
			পূর্ণসংখ্যা x = wcwidth(wc);
			যদি (x > 0)
				width += x;
		} অন্যথা যদি (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#অন্যথা
		যদি (isprint((unsigned অক্ষর) *p))
			width++;
		p++;
#endif
	}

	ফেরত width;
}

size_t mbs_width(ধ্রুবক অক্ষর *s)
{
	যদি (!s || !*s)
		ফেরত 0;
	ফেরত mbs_nwidth(s, strlen(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
size_t mbs_safe_nwidth(ধ্রুবক অক্ষর *buf, size_t bufsz, size_t *sz)
{
	ধ্রুবক অক্ষর *p = buf, *last = buf;
	size_t width = 0, bytes = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, আকার(st));
#endif
	যদি (p && *p && bufsz)
		last = p + (bufsz - 1);

	যখন (p && *p && p <= last) {
		যদি ((p < last && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned অক্ষর) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		}
#ifdef HAVE_WIDECHAR
		অন্যথা {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			যদি (len == 0)
				বিরতি;

			যদি (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				যদি (isprint((unsigned অক্ষর) *p))
					width += 1, bytes += 1;
				অন্যথা
					width += 4, bytes += 4;

			} অন্যথা যদি (!iswprint(wc)) {
				width += len * 4;	/* hex encode whole sequence */
				bytes += len * 4;
			} অন্যথা {
				width += wcwidth(wc);	/* number of cells */
				bytes += len;		/* number of bytes */
			}
			p += len;
		}
#অন্যথা
		অন্যথা যদি (!isprint((unsigned অক্ষর) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		} অন্যথা {
			width++, bytes++;
			p++;
		}
#endif
	}

	যদি (sz)
		*sz = bytes;
	ফেরত width;
}

size_t mbs_safe_width(ধ্রুবক অক্ষর *s)
{
	যদি (!s || !*s)
		ফেরত 0;
	ফেরত mbs_safe_nwidth(s, strlen(s), ফাঁকা);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
অক্ষর *mbs_safe_encode_to_বাফার(ধ্রুবক অক্ষর *s, size_t *width, অক্ষর *buf, size_t bufsiz, ধ্রুবক অক্ষর *safechars)
{
	ধ্রুবক অক্ষর *p = s;
	অক্ষর *r;
	পূর্ণসংখ্যা rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, আকার(st));
#endif
	যদি (!sz || !buf || !bufsiz)
		ফেরত ফাঁকা;

	r = buf;
	rsz = (পূর্ণসংখ্যা) bufsiz;
	*width = 0;

	যখন (p && *p) {
		যদি (safechars && strchr(safechars, *p)) {
			যদি (rsz < 2)
				বিরতি;
			*r++ = *p++;
			rsz--;
			চালু;
		}

		যদি ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned অক্ষর) *p)) {
			যদি (rsz < 5)
				বিরতি;
			snprintf(r, rsz, "\\x%02x", (unsigned অক্ষর) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
			p++;
		}
#ifdef HAVE_WIDECHAR
		অন্যথা {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			যদি (len == 0)
				বিরতি;		/* end of string */

			যদি (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				যদি (!isprint((unsigned অক্ষর) *p)) {
					যদি (rsz < 5)
						বিরতি;
					snprintf(r, rsz, "\\x%02x", (unsigned অক্ষর) *p);
					r += 4;
					rsz -= 4;
					*width += 4;
				} অন্যথা {
					যদি (rsz < 2)
						বিরতি;
					(*width)++;
					*r++ = *p;
					rsz--;
				}
			} অন্যথা যদি (!iswprint(wc)) {
				size_t i;
				জন্য (i = 0; i < len; i++) {
					যদি (rsz < 5)
						বিরতি;
					snprintf(r, rsz, "\\x%02x", (unsigned অক্ষর) p[i]);
					r += 4;
					rsz -= 4;
					*width += 4;
				}
				যদি (i < len)
					বিরতি;
			} অন্যথা {
				যদি (rsz < (পূর্ণসংখ্যা)len + 1)
					বিরতি;
				memcpy(r, p, len);
				r += len;
				rsz -= len;
				*width += wcwidth(wc);
			}
			p += len;
		}
#অন্যথা
		অন্যথা যদি (!isprint((unsigned অক্ষর) *p)) {
			যদি (rsz < 5)
				বিরতি;
			snprintf(r, rsz, "\\x%02x", (unsigned অক্ষর) *p);
			p++;
			r += 4;
			rsz -= 4;
			*width += 4;
		} অন্যথা {
			যদি (rsz < 2)
				বিরতি;
			*r++ = *p++;
			rsz--;
			(*width)++;
		}
#endif
	}

	*r = '\0';
	ফেরত buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
অক্ষর *mbs_invalid_encode_to_বাফার(ধ্রুবক অক্ষর *s, size_t *width, অক্ষর *buf, size_t bufsiz)
{
	ধ্রুবক অক্ষর *p = s;
	অক্ষর *r;
	পূর্ণসংখ্যা rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, আকার(st));
#endif
	যদি (!sz || !buf || !bufsiz)
		ফেরত ফাঁকা;

	r = buf;
	rsz = (পূর্ণসংখ্যা) bufsiz;
	*width = 0;

	যখন (p && *p) {
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#অন্যথা
		size_t len = 1;
#endif

		যদি (len == 0)
			বিরতি;		/* end of string */

		যদি (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			যদি (!isprint((unsigned অক্ষর) *p)) {
				যদি (rsz < 5)
					বিরতি;
				snprintf(r, rsz, "\\x%02x", (unsigned অক্ষর) *p);
				r += 4;
				rsz -= 4;
				*width += 4;
			} অন্যথা {
				যদি (rsz < 2)
					বিরতি;
				(*width)++;
				*r++ = *p;
				rsz--;
			}
		} অন্যথা যদি (*p == '\\' && *(p + 1) == 'x') {
			যদি (rsz < 5)
				বিরতি;
			snprintf(r, rsz, "\\x%02x", (unsigned অক্ষর) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
		} অন্যথা {
			যদি (rsz < (পূর্ণসংখ্যা)len + 1)
				বিরতি;
			r = mempcpy(r, p, len);
			rsz -= len;
			*width += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	ফেরত buf;
}

/*
 * Guess size
 */
size_t mbs_safe_encode_আকার(size_t bytes)
{
	ফেরত (bytes * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_safe_decode_আকার(ধ্রুবক অক্ষর *p)
{
	size_t bytes = 0;

	যখন (p && *p) {
		যদি (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		অন্যথা
			p++;
		bytes++;
	}
	ফেরত bytes;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
অক্ষর *mbs_safe_encode(ধ্রুবক অক্ষর *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	অক্ষর *buf, *ret = ফাঁকা;

	যদি (!sz)
		ফেরত ফাঁকা;
	bufsz = mbs_safe_encode_আকার(sz);
	buf = malloc(bufsz);
	যদি (buf)
		ret = mbs_safe_encode_to_বাফার(s, width, buf, bufsz, ফাঁকা);
	যদি (!ret)
		মুক্ত(buf);
	ফেরত ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
অক্ষর *mbs_invalid_encode(ধ্রুবক অক্ষর *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	অক্ষর *buf, *ret = ফাঁকা;

	যদি (!sz)
		ফেরত ফাঁকা;
	bufsz = mbs_safe_encode_আকার(sz);
	buf = malloc(bufsz);
	যদি (buf)
		ret = mbs_invalid_encode_to_বাফার(s, width, buf, bufsz);
	যদি (!ret)
		মুক্ত(buf);
	ফেরত ret;
}

#ifdef HAVE_WIDECHAR

স্থির bool
wc_ensure_printable (wchar_t *wchars)
{
  bool replaced = false;
  wchar_t *wc = wchars;
  যখন (*wc)
    {
      যদি (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  ফেরত replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
স্থির wchar_t *
wc_skip_ansi_escape (wchar_t *wc)
{
  wchar_t *p = wc + 1;

  যদি (*wc != L'\033' || !*p)
    ফেরত ফাঁকা;

  যদি (*p == L'[')
    {
      p++;
      যখন (*p >= 0x30 && *p <= 0x3f)
        p++;
      যখন (*p >= 0x20 && *p <= 0x2f)
        p++;
      যদি (*p >= 0x40 && *p <= 0x7e)
        ফেরত p + 1;
    }
  অন্যথা যদি ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    ফেরত p + 2;

  ফেরত ফাঁকা;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

স্থির size_t
wc_truncate (wchar_t *wc, size_t width)
{
  size_t cells = 0;
  পূর্ণসংখ্যা পরবর্তী_cells = 0;

  যখন (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      যদি (after_escape)
        {
          wc = after_escape;
          চালু;
        }

      পরবর্তী_cells = wcwidth (*wc);
      যদি (পরবর্তী_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          পরবর্তী_cells = 1;
        }
      যদি (cells + পরবর্তী_cells > width)
        বিরতি;

      cells += পরবর্তী_cells;
      wc++;
    }
  *wc = L'\0';
  ফেরত cells;
}

স্থির পূর্ণসংখ্যা
rpl_wcswidth (ধ্রুবক wchar_t *s, size_t n)
{
  পূর্ণসংখ্যা ret = 0;

  যখন (n-- > 0 && *s != L'\0')
    {
      পূর্ণসংখ্যা nwidth = wcwidth (*s++);
      যদি (nwidth == -1)             /* non printable */
        ফেরত -1;
      যদি (ret > (INT_MAX - nwidth)) /* overflow */
        ফেরত -1;
      ret += nwidth;
    }

  ফেরত ret;
}
#endif /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
size_t
mbs_truncate(অক্ষর *str, size_t *width)
{
	ssize_t bytes = strlen(str);
#ifdef HAVE_WIDECHAR
	ssize_t sz = mbstowcs(ফাঁকা, str, 0);
	wchar_t *wcs = ফাঁকা;

	যদি (sz == (ssize_t) -1)
		যাও done;

	wcs = calloc(1, (sz + 1) * আকার(wchar_t));
	যদি (!wcs)
		যাও done;

	যদি (!mbstowcs(wcs, str, sz))
		যাও done;
	*width = wc_truncate(wcs, *width);
	bytes = wcstombs(str, wcs, bytes);
done:
	মুক্ত(wcs);
#অন্যথা
	{
		ধ্রুবক অক্ষর *p = str;
		ধ্রুবক অক্ষর *end = str + bytes;
		size_t cells = 0;

		যখন (p < end) {
			ধ্রুবক অক্ষর *after_escape = skip_ansi_escape(p, end);

			যদি (after_escape) {
				p = after_escape;
				চালু;
			}
			যদি (cells >= *width)
				বিরতি;
			p++;
			cells++;
		}
		bytes = p - str;
		*width = cells;
	}
#endif
	যদি (bytes >= 0)
		str[bytes] = '\0';
	ফেরত bytes;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

স্থির অক্ষর*
mbs_align_pad (অক্ষর *dest, ধ্রুবক অক্ষর* dest_end, size_t n_spaces, পূর্ণসংখ্যা padchar)
{
  জন্য (/* nothing */; n_spaces && (dest < dest_end); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  ফেরত dest;
}

size_t
mbsalign (ধ্রুবক অক্ষর *src, অক্ষর *dest, size_t dest_আকার,
          size_t *width, mbs_align_t align, পূর্ণসংখ্যা flags)
{
	ফেরত mbsalign_with_padding(src, dest, dest_আকার, width, align, flags, ' ');
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
mbsalign_with_padding (ধ্রুবক অক্ষর *src, অক্ষর *dest, size_t dest_আকার,
	               size_t *width, mbs_align_t align,
#ifdef HAVE_WIDECHAR
		       পূর্ণসংখ্যা flags,
#অন্যথা
		       পূর্ণসংখ্যা flags __attribute__((__unused__)),
#endif
		       পূর্ণসংখ্যা padchar)
{
  size_t ret = -1;
  size_t src_আকার = strlen (src) + 1;
  অক্ষর *newstr = ফাঁকা;
  wchar_t *str_wc = ফাঁকা;
  ধ্রুবক অক্ষর *str_to_print = src;
  size_t n_cols = src_আকার - 1;
  size_t n_used_bytes = n_cols; /* Not including NUL */
  size_t n_spaces = 0, space_বাম;

#ifdef HAVE_WIDECHAR
  bool conversion = false;
  bool wc_enabled = false;

  /* In multi-byte locales convert to wide characters
     to allow easy truncation. Also determine number
     of screen columns used.  */
  যদি (MB_CUR_MAX > 1)
    {
      size_t src_chars = mbstowcs (ফাঁকা, src, 0);
      যদি (src_chars == (size_t) -1)
        {
          যদি (flags & MBA_UNIBYTE_FALLBACK)
            যাও mbsalign_unibyte;
          অন্যথা
            যাও mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = malloc (src_chars * আকার (wchar_t));
      যদি (str_wc == ফাঁকা)
        {
          যদি (flags & MBA_UNIBYTE_FALLBACK)
            যাও mbsalign_unibyte;
          অন্যথা
            যাও mbsalign_cleanup;
        }
      যদি (mbstowcs (str_wc, src, src_chars) != 0)
        {
          str_wc[src_chars - 1] = L'\0';
          wc_enabled = true;
          conversion = wc_ensure_printable (str_wc);
          n_cols = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  যদি (wc_enabled && (conversion || (n_cols > *width)))
    {
        যদি (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_আকার = wcstombs(ফাঁকা, str_wc, 0) + 1;
          }
        newstr = malloc (src_আকার);
        যদি (newstr == ফাঁকা)
        {
          যদি (flags & MBA_UNIBYTE_FALLBACK)
            যাও mbsalign_unibyte;
          অন্যথা
            যাও mbsalign_cleanup;
        }
        str_to_print = newstr;
        n_cols = wc_truncate (str_wc, *width);
        n_used_bytes = wcstombs (newstr, str_wc, src_আকার);
    }

mbsalign_unibyte:
#endif

  যদি (n_cols > *width) /* Unibyte truncation required.  */
    {
      n_cols = *width;
      n_used_bytes = n_cols;
    }

  যদি (*width > n_cols) /* Padding required.  */
    n_spaces = *width - n_cols;

  /* indicate to caller how many cells needed (not including padding).  */
  *width = n_cols;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_used_bytes + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  যদি (dest_আকার != 0)
    {
      অক্ষর *dest_end = dest + dest_আকার - 1;
      size_t start_spaces;
      size_t end_spaces;

      নির্বাচন (align)
        {
        ক্ষেত্র MBS_ALIGN_CENTER:
          start_spaces = n_spaces / 2 + n_spaces % 2;
          end_spaces = n_spaces / 2;
          বিরতি;
        ক্ষেত্র MBS_ALIGN_বাম:
          start_spaces = 0;
          end_spaces = n_spaces;
          বিরতি;
        ক্ষেত্র MBS_ALIGN_ডান:
          start_spaces = n_spaces;
          end_spaces = 0;
          বিরতি;
	পূর্বনির্ধারিত:
	  abort();
        }

      dest = mbs_align_pad (dest, dest_end, start_spaces, padchar);
      space_বাম = dest_end - dest;
      dest = mempcpy (dest, str_to_print, min (n_used_bytes, space_বাম));
      mbs_align_pad (dest, dest_end, end_spaces, padchar);
    }
#ifdef HAVE_WIDECHAR
mbsalign_cleanup:
#endif
  মুক্ত (str_wc);
  মুক্ত (newstr);

  ফেরত ret;
}
