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
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <limits.h>
#অন্তর্ভাৱ <ctype.h>

#অন্তর্ভাৱ "c.h"
#অন্তর্ভাৱ "mbsalign.h"
#অন্তর্ভাৱ "strutils.h"
#অন্তর্ভাৱ "widechar.h"

/* Return the byte after a complete ANSI escape sequence, or NULL. */
স্থির ধ্রুৱ ৱর্ণ *skip_ansi_escape(ধ্রুৱ ৱর্ণ *p, ধ্রুৱ ৱর্ণ *end)
{
	ধ্রুৱ ৱর্ণ *q;

	যদি (p >= end || (unsigned ৱর্ণ) *p != '\033' || p + 1 >= end)
		ঘূৰাই রিক্ত;

	q = p + 1;
	যদি (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		যেতিয়ালৈ (q < end && (unsigned ৱর্ণ) *q >= 0x30
				&& (unsigned ৱর্ণ) *q <= 0x3f)
			q++;
		যেতিয়ালৈ (q < end && (unsigned ৱর্ণ) *q >= 0x20
				&& (unsigned ৱর্ণ) *q <= 0x2f)
			q++;
		যদি (q < end && (unsigned ৱর্ণ) *q >= 0x40
				&& (unsigned ৱর্ণ) *q <= 0x7e)
			ঘূৰাই q + 1;
	} অন্যথা যদি (*q && strchr("()*+", *q) && q + 1 < end)
		ঘূৰাই q + 2;

	ঘূৰাই রিক্ত;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
size_t mbs_nwidth(ধ্রুৱ ৱর্ণ *buf, size_t bufsz)
{
	ধ্রুৱ ৱর্ণ *p = buf, *last = buf;
	size_t width = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, পরিমাণ(st));
#endif
	যদি (p && *p && bufsz)
		last = p + (bufsz - 1);

	যেতিয়ালৈ (p && *p && p <= last) {
		যদি (*p == '\033') {
			ধ্রুৱ ৱর্ণ *e = skip_ansi_escape(p, last + 1);

			p = e ? e : p + 1;
			অনুৱর্ত;
		}
		যদি (iscntrl((unsigned ৱর্ণ) *p)) {
			p++;
			অনুৱর্ত;
		}
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		যদি (len == 0)
			ভঙ্গ;
		যদি (len > 0 && iswprint(wc)) {
			পূর্ণাঙ্ক x = wcwidth(wc);
			যদি (x > 0)
				width += x;
		} অন্যথা যদি (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#অন্যথা
		যদি (isprint((unsigned ৱর্ণ) *p))
			width++;
		p++;
#endif
	}

	ঘূৰাই width;
}

size_t mbs_width(ধ্রুৱ ৱর্ণ *s)
{
	যদি (!s || !*s)
		ঘূৰাই 0;
	ঘূৰাই mbs_nwidth(s, strlen(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
size_t mbs_safe_nwidth(ধ্রুৱ ৱর্ণ *buf, size_t bufsz, size_t *sz)
{
	ধ্রুৱ ৱর্ণ *p = buf, *last = buf;
	size_t width = 0, bytes = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, পরিমাণ(st));
#endif
	যদি (p && *p && bufsz)
		last = p + (bufsz - 1);

	যেতিয়ালৈ (p && *p && p <= last) {
		যদি ((p < last && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned ৱর্ণ) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		}
#ifdef HAVE_WIDECHAR
		অন্যথা {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			যদি (len == 0)
				ভঙ্গ;

			যদি (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				যদি (isprint((unsigned ৱর্ণ) *p))
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
		অন্যথা যদি (!isprint((unsigned ৱর্ণ) *p)) {
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
	ঘূৰাই width;
}

size_t mbs_safe_width(ধ্রুৱ ৱর্ণ *s)
{
	যদি (!s || !*s)
		ঘূৰাই 0;
	ঘূৰাই mbs_safe_nwidth(s, strlen(s), রিক্ত);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
ৱর্ণ *mbs_safe_encode_to_buffer(ধ্রুৱ ৱর্ণ *s, size_t *width, ৱর্ণ *buf, size_t bufsiz, ধ্রুৱ ৱর্ণ *safechars)
{
	ধ্রুৱ ৱর্ণ *p = s;
	ৱর্ণ *r;
	পূর্ণাঙ্ক rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, পরিমাণ(st));
#endif
	যদি (!sz || !buf || !bufsiz)
		ঘূৰাই রিক্ত;

	r = buf;
	rsz = (পূর্ণাঙ্ক) bufsiz;
	*width = 0;

	যেতিয়ালৈ (p && *p) {
		যদি (safechars && strchr(safechars, *p)) {
			যদি (rsz < 2)
				ভঙ্গ;
			*r++ = *p++;
			rsz--;
			অনুৱর্ত;
		}

		যদি ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned ৱর্ণ) *p)) {
			যদি (rsz < 5)
				ভঙ্গ;
			snprintf(r, rsz, "\\x%02x", (unsigned ৱর্ণ) *p);
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
				ভঙ্গ;		/* end of string */

			যদি (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				যদি (!isprint((unsigned ৱর্ণ) *p)) {
					যদি (rsz < 5)
						ভঙ্গ;
					snprintf(r, rsz, "\\x%02x", (unsigned ৱর্ণ) *p);
					r += 4;
					rsz -= 4;
					*width += 4;
				} অন্যথা {
					যদি (rsz < 2)
						ভঙ্গ;
					(*width)++;
					*r++ = *p;
					rsz--;
				}
			} অন্যথা যদি (!iswprint(wc)) {
				size_t i;
				বাবে (i = 0; i < len; i++) {
					যদি (rsz < 5)
						ভঙ্গ;
					snprintf(r, rsz, "\\x%02x", (unsigned ৱর্ণ) p[i]);
					r += 4;
					rsz -= 4;
					*width += 4;
				}
				যদি (i < len)
					ভঙ্গ;
			} অন্যথা {
				যদি (rsz < (পূর্ণাঙ্ক)len + 1)
					ভঙ্গ;
				memcpy(r, p, len);
				r += len;
				rsz -= len;
				*width += wcwidth(wc);
			}
			p += len;
		}
#অন্যথা
		অন্যথা যদি (!isprint((unsigned ৱর্ণ) *p)) {
			যদি (rsz < 5)
				ভঙ্গ;
			snprintf(r, rsz, "\\x%02x", (unsigned ৱর্ণ) *p);
			p++;
			r += 4;
			rsz -= 4;
			*width += 4;
		} অন্যথা {
			যদি (rsz < 2)
				ভঙ্গ;
			*r++ = *p++;
			rsz--;
			(*width)++;
		}
#endif
	}

	*r = '\0';
	ঘূৰাই buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
ৱর্ণ *mbs_invalid_encode_to_buffer(ধ্রুৱ ৱর্ণ *s, size_t *width, ৱর্ণ *buf, size_t bufsiz)
{
	ধ্রুৱ ৱর্ণ *p = s;
	ৱর্ণ *r;
	পূর্ণাঙ্ক rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, পরিমাণ(st));
#endif
	যদি (!sz || !buf || !bufsiz)
		ঘূৰাই রিক্ত;

	r = buf;
	rsz = (পূর্ণাঙ্ক) bufsiz;
	*width = 0;

	যেতিয়ালৈ (p && *p) {
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#অন্যথা
		size_t len = 1;
#endif

		যদি (len == 0)
			ভঙ্গ;		/* end of string */

		যদি (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			যদি (!isprint((unsigned ৱর্ণ) *p)) {
				যদি (rsz < 5)
					ভঙ্গ;
				snprintf(r, rsz, "\\x%02x", (unsigned ৱর্ণ) *p);
				r += 4;
				rsz -= 4;
				*width += 4;
			} অন্যথা {
				যদি (rsz < 2)
					ভঙ্গ;
				(*width)++;
				*r++ = *p;
				rsz--;
			}
		} অন্যথা যদি (*p == '\\' && *(p + 1) == 'x') {
			যদি (rsz < 5)
				ভঙ্গ;
			snprintf(r, rsz, "\\x%02x", (unsigned ৱর্ণ) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
		} অন্যথা {
			যদি (rsz < (পূর্ণাঙ্ক)len + 1)
				ভঙ্গ;
			r = mempcpy(r, p, len);
			rsz -= len;
			*width += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	ঘূৰাই buf;
}

/*
 * Guess size
 */
size_t mbs_safe_encode_size(size_t bytes)
{
	ঘূৰাই (bytes * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_safe_decode_size(ধ্রুৱ ৱর্ণ *p)
{
	size_t bytes = 0;

	যেতিয়ালৈ (p && *p) {
		যদি (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		অন্যথা
			p++;
		bytes++;
	}
	ঘূৰাই bytes;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
ৱর্ণ *mbs_safe_encode(ধ্রুৱ ৱর্ণ *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	ৱর্ণ *buf, *ret = রিক্ত;

	যদি (!sz)
		ঘূৰাই রিক্ত;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	যদি (buf)
		ret = mbs_safe_encode_to_buffer(s, width, buf, bufsz, রিক্ত);
	যদি (!ret)
		মুক্ত(buf);
	ঘূৰাই ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
ৱর্ণ *mbs_invalid_encode(ধ্রুৱ ৱর্ণ *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	ৱর্ণ *buf, *ret = রিক্ত;

	যদি (!sz)
		ঘূৰাই রিক্ত;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	যদি (buf)
		ret = mbs_invalid_encode_to_buffer(s, width, buf, bufsz);
	যদি (!ret)
		মুক্ত(buf);
	ঘূৰাই ret;
}

#ifdef HAVE_WIDECHAR

স্থির bool
wc_ensure_printable (wchar_t *wchars)
{
  bool replaced = false;
  wchar_t *wc = wchars;
  যেতিয়ালৈ (*wc)
    {
      যদি (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  ঘূৰাই replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
স্থির wchar_t *
wc_skip_ansi_escape (wchar_t *wc)
{
  wchar_t *p = wc + 1;

  যদি (*wc != L'\033' || !*p)
    ঘূৰাই রিক্ত;

  যদি (*p == L'[')
    {
      p++;
      যেতিয়ালৈ (*p >= 0x30 && *p <= 0x3f)
        p++;
      যেতিয়ালৈ (*p >= 0x20 && *p <= 0x2f)
        p++;
      যদি (*p >= 0x40 && *p <= 0x7e)
        ঘূৰাই p + 1;
    }
  অন্যথা যদি ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    ঘূৰাই p + 2;

  ঘূৰাই রিক্ত;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

স্থির size_t
wc_truncate (wchar_t *wc, size_t width)
{
  size_t cells = 0;
  পূর্ণাঙ্ক next_cells = 0;

  যেতিয়ালৈ (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      যদি (after_escape)
        {
          wc = after_escape;
          অনুৱর্ত;
        }

      next_cells = wcwidth (*wc);
      যদি (next_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          next_cells = 1;
        }
      যদি (cells + next_cells > width)
        ভঙ্গ;

      cells += next_cells;
      wc++;
    }
  *wc = L'\0';
  ঘূৰাই cells;
}

স্থির পূর্ণাঙ্ক
rpl_wcswidth (ধ্রুৱ wchar_t *s, size_t n)
{
  পূর্ণাঙ্ক ret = 0;

  যেতিয়ালৈ (n-- > 0 && *s != L'\0')
    {
      পূর্ণাঙ্ক nwidth = wcwidth (*s++);
      যদি (nwidth == -1)             /* non printable */
        ঘূৰাই -1;
      যদি (ret > (INT_MAX - nwidth)) /* overflow */
        ঘূৰাই -1;
      ret += nwidth;
    }

  ঘূৰাই ret;
}
#endif /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
size_t
mbs_truncate(ৱর্ণ *str, size_t *width)
{
	ssize_t bytes = strlen(str);
#ifdef HAVE_WIDECHAR
	ssize_t sz = mbstowcs(রিক্ত, str, 0);
	wchar_t *wcs = রিক্ত;

	যদি (sz == (ssize_t) -1)
		গচ্ছ done;

	wcs = calloc(1, (sz + 1) * পরিমাণ(wchar_t));
	যদি (!wcs)
		গচ্ছ done;

	যদি (!mbstowcs(wcs, str, sz))
		গচ্ছ done;
	*width = wc_truncate(wcs, *width);
	bytes = wcstombs(str, wcs, bytes);
done:
	মুক্ত(wcs);
#অন্যথা
	{
		ধ্রুৱ ৱর্ণ *p = str;
		ধ্রুৱ ৱর্ণ *end = str + bytes;
		size_t cells = 0;

		যেতিয়ালৈ (p < end) {
			ধ্রুৱ ৱর্ণ *after_escape = skip_ansi_escape(p, end);

			যদি (after_escape) {
				p = after_escape;
				অনুৱর্ত;
			}
			যদি (cells >= *width)
				ভঙ্গ;
			p++;
			cells++;
		}
		bytes = p - str;
		*width = cells;
	}
#endif
	যদি (bytes >= 0)
		str[bytes] = '\0';
	ঘূৰাই bytes;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

স্থির ৱর্ণ*
mbs_align_pad (ৱর্ণ *dest, ধ্রুৱ ৱর্ণ* dest_end, size_t n_spaces, পূর্ণাঙ্ক padchar)
{
  বাবে (/* nothing */; n_spaces && (dest < dest_end); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  ঘূৰাই dest;
}

size_t
mbsalign (ধ্রুৱ ৱর্ণ *src, ৱর্ণ *dest, size_t dest_size,
          size_t *width, mbs_align_t align, পূর্ণাঙ্ক flags)
{
	ঘূৰাই mbsalign_with_padding(src, dest, dest_size, width, align, flags, ' ');
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
mbsalign_with_padding (ধ্রুৱ ৱর্ণ *src, ৱর্ণ *dest, size_t dest_size,
	               size_t *width, mbs_align_t align,
#ifdef HAVE_WIDECHAR
		       পূর্ণাঙ্ক flags,
#অন্যথা
		       পূর্ণাঙ্ক flags __attribute__((__unused__)),
#endif
		       পূর্ণাঙ্ক padchar)
{
  size_t ret = -1;
  size_t src_size = strlen (src) + 1;
  ৱর্ণ *newstr = রিক্ত;
  wchar_t *str_wc = রিক্ত;
  ধ্রুৱ ৱর্ণ *str_to_print = src;
  size_t n_cols = src_size - 1;
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
      size_t src_chars = mbstowcs (রিক্ত, src, 0);
      যদি (src_chars == (size_t) -1)
        {
          যদি (flags & MBA_UNIBYTE_FALLBACK)
            গচ্ছ mbsalign_unibyte;
          অন্যথা
            গচ্ছ mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = malloc (src_chars * পরিমাণ (wchar_t));
      যদি (str_wc == রিক্ত)
        {
          যদি (flags & MBA_UNIBYTE_FALLBACK)
            গচ্ছ mbsalign_unibyte;
          অন্যথা
            গচ্ছ mbsalign_cleanup;
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
            src_size = wcstombs(রিক্ত, str_wc, 0) + 1;
          }
        newstr = malloc (src_size);
        যদি (newstr == রিক্ত)
        {
          যদি (flags & MBA_UNIBYTE_FALLBACK)
            গচ্ছ mbsalign_unibyte;
          অন্যথা
            গচ্ছ mbsalign_cleanup;
        }
        str_to_print = newstr;
        n_cols = wc_truncate (str_wc, *width);
        n_used_bytes = wcstombs (newstr, str_wc, src_size);
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
  যদি (dest_size != 0)
    {
      ৱর্ণ *dest_end = dest + dest_size - 1;
      size_t start_spaces;
      size_t end_spaces;

      ৱিকল্প (align)
        {
        পক্ষ MBS_ALIGN_CENTER:
          start_spaces = n_spaces / 2 + n_spaces % 2;
          end_spaces = n_spaces / 2;
          ভঙ্গ;
        পক্ষ MBS_ALIGN_বাম:
          start_spaces = 0;
          end_spaces = n_spaces;
          ভঙ্গ;
        পক্ষ MBS_ALIGN_ডান:
          start_spaces = n_spaces;
          end_spaces = 0;
          ভঙ্গ;
	স্ৱভাৱ:
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

  ঘূৰাই ret;
}
