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
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <ctype.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "mbsalign.h"
#അന്തര്ഭാവ "strutils.h"
#അന്തര്ഭാവ "widechar.h"

/* Return the byte after a complete ANSI escape sequence, or NULL. */
സ്ഥിതം സ്ഥിരം അക്ഷരം *skip_ansi_escape(സ്ഥിരം അക്ഷരം *p, സ്ഥിരം അക്ഷരം *end)
{
	സ്ഥിരം അക്ഷരം *q;

	എങ്കിൽ (p >= end || (unsigned അക്ഷരം) *p != '\033' || p + 1 >= end)
		തിരികെ ശൂന്യ;

	q = p + 1;
	എങ്കിൽ (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		ആകുമ്പോൾ (q < end && (unsigned അക്ഷരം) *q >= 0x30
				&& (unsigned അക്ഷരം) *q <= 0x3f)
			q++;
		ആകുമ്പോൾ (q < end && (unsigned അക്ഷരം) *q >= 0x20
				&& (unsigned അക്ഷരം) *q <= 0x2f)
			q++;
		എങ്കിൽ (q < end && (unsigned അക്ഷരം) *q >= 0x40
				&& (unsigned അക്ഷരം) *q <= 0x7e)
			തിരികെ q + 1;
	} അല്ലെങ്കിൽ എങ്കിൽ (*q && strchr("()*+", *q) && q + 1 < end)
		തിരികെ q + 2;

	തിരികെ ശൂന്യ;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
size_t mbs_nwidth(സ്ഥിരം അക്ഷരം *buf, size_t bufsz)
{
	സ്ഥിരം അക്ഷരം *p = buf, *last = buf;
	size_t width = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, വലുപ്പം(st));
#endif
	എങ്കിൽ (p && *p && bufsz)
		last = p + (bufsz - 1);

	ആകുമ്പോൾ (p && *p && p <= last) {
		എങ്കിൽ (*p == '\033') {
			സ്ഥിരം അക്ഷരം *e = skip_ansi_escape(p, last + 1);

			p = e ? e : p + 1;
			തുടരുക;
		}
		എങ്കിൽ (iscntrl((unsigned അക്ഷരം) *p)) {
			p++;
			തുടരുക;
		}
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		എങ്കിൽ (len == 0)
			നിർത്തുക;
		എങ്കിൽ (len > 0 && iswprint(wc)) {
			പൂർണ്ണസംഖ്യ x = wcwidth(wc);
			എങ്കിൽ (x > 0)
				width += x;
		} അല്ലെങ്കിൽ എങ്കിൽ (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#അല്ലെങ്കിൽ
		എങ്കിൽ (isprint((unsigned അക്ഷരം) *p))
			width++;
		p++;
#endif
	}

	തിരികെ width;
}

size_t mbs_width(സ്ഥിരം അക്ഷരം *s)
{
	എങ്കിൽ (!s || !*s)
		തിരികെ 0;
	തിരികെ mbs_nwidth(s, strlen(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
size_t mbs_safe_nwidth(സ്ഥിരം അക്ഷരം *buf, size_t bufsz, size_t *sz)
{
	സ്ഥിരം അക്ഷരം *p = buf, *last = buf;
	size_t width = 0, bytes = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, വലുപ്പം(st));
#endif
	എങ്കിൽ (p && *p && bufsz)
		last = p + (bufsz - 1);

	ആകുമ്പോൾ (p && *p && p <= last) {
		എങ്കിൽ ((p < last && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned അക്ഷരം) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		}
#ifdef HAVE_WIDECHAR
		അല്ലെങ്കിൽ {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			എങ്കിൽ (len == 0)
				നിർത്തുക;

			എങ്കിൽ (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				എങ്കിൽ (isprint((unsigned അക്ഷരം) *p))
					width += 1, bytes += 1;
				അല്ലെങ്കിൽ
					width += 4, bytes += 4;

			} അല്ലെങ്കിൽ എങ്കിൽ (!iswprint(wc)) {
				width += len * 4;	/* hex encode whole sequence */
				bytes += len * 4;
			} അല്ലെങ്കിൽ {
				width += wcwidth(wc);	/* number of cells */
				bytes += len;		/* number of bytes */
			}
			p += len;
		}
#അല്ലെങ്കിൽ
		അല്ലെങ്കിൽ എങ്കിൽ (!isprint((unsigned അക്ഷരം) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		} അല്ലെങ്കിൽ {
			width++, bytes++;
			p++;
		}
#endif
	}

	എങ്കിൽ (sz)
		*sz = bytes;
	തിരികെ width;
}

size_t mbs_safe_width(സ്ഥിരം അക്ഷരം *s)
{
	എങ്കിൽ (!s || !*s)
		തിരികെ 0;
	തിരികെ mbs_safe_nwidth(s, strlen(s), ശൂന്യ);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
അക്ഷരം *mbs_safe_encode_to_buffer(സ്ഥിരം അക്ഷരം *s, size_t *width, അക്ഷരം *buf, size_t bufsiz, സ്ഥിരം അക്ഷരം *safechars)
{
	സ്ഥിരം അക്ഷരം *p = s;
	അക്ഷരം *r;
	പൂർണ്ണസംഖ്യ rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, വലുപ്പം(st));
#endif
	എങ്കിൽ (!sz || !buf || !bufsiz)
		തിരികെ ശൂന്യ;

	r = buf;
	rsz = (പൂർണ്ണസംഖ്യ) bufsiz;
	*width = 0;

	ആകുമ്പോൾ (p && *p) {
		എങ്കിൽ (safechars && strchr(safechars, *p)) {
			എങ്കിൽ (rsz < 2)
				നിർത്തുക;
			*r++ = *p++;
			rsz--;
			തുടരുക;
		}

		എങ്കിൽ ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned അക്ഷരം) *p)) {
			എങ്കിൽ (rsz < 5)
				നിർത്തുക;
			snprintf(r, rsz, "\\x%02x", (unsigned അക്ഷരം) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
			p++;
		}
#ifdef HAVE_WIDECHAR
		അല്ലെങ്കിൽ {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			എങ്കിൽ (len == 0)
				നിർത്തുക;		/* end of string */

			എങ്കിൽ (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				എങ്കിൽ (!isprint((unsigned അക്ഷരം) *p)) {
					എങ്കിൽ (rsz < 5)
						നിർത്തുക;
					snprintf(r, rsz, "\\x%02x", (unsigned അക്ഷരം) *p);
					r += 4;
					rsz -= 4;
					*width += 4;
				} അല്ലെങ്കിൽ {
					എങ്കിൽ (rsz < 2)
						നിർത്തുക;
					(*width)++;
					*r++ = *p;
					rsz--;
				}
			} അല്ലെങ്കിൽ എങ്കിൽ (!iswprint(wc)) {
				size_t i;
				വേണ്ടി (i = 0; i < len; i++) {
					എങ്കിൽ (rsz < 5)
						നിർത്തുക;
					snprintf(r, rsz, "\\x%02x", (unsigned അക്ഷരം) p[i]);
					r += 4;
					rsz -= 4;
					*width += 4;
				}
				എങ്കിൽ (i < len)
					നിർത്തുക;
			} അല്ലെങ്കിൽ {
				എങ്കിൽ (rsz < (പൂർണ്ണസംഖ്യ)len + 1)
					നിർത്തുക;
				memcpy(r, p, len);
				r += len;
				rsz -= len;
				*width += wcwidth(wc);
			}
			p += len;
		}
#അല്ലെങ്കിൽ
		അല്ലെങ്കിൽ എങ്കിൽ (!isprint((unsigned അക്ഷരം) *p)) {
			എങ്കിൽ (rsz < 5)
				നിർത്തുക;
			snprintf(r, rsz, "\\x%02x", (unsigned അക്ഷരം) *p);
			p++;
			r += 4;
			rsz -= 4;
			*width += 4;
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (rsz < 2)
				നിർത്തുക;
			*r++ = *p++;
			rsz--;
			(*width)++;
		}
#endif
	}

	*r = '\0';
	തിരികെ buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
അക്ഷരം *mbs_invalid_encode_to_buffer(സ്ഥിരം അക്ഷരം *s, size_t *width, അക്ഷരം *buf, size_t bufsiz)
{
	സ്ഥിരം അക്ഷരം *p = s;
	അക്ഷരം *r;
	പൂർണ്ണസംഖ്യ rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, വലുപ്പം(st));
#endif
	എങ്കിൽ (!sz || !buf || !bufsiz)
		തിരികെ ശൂന്യ;

	r = buf;
	rsz = (പൂർണ്ണസംഖ്യ) bufsiz;
	*width = 0;

	ആകുമ്പോൾ (p && *p) {
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#അല്ലെങ്കിൽ
		size_t len = 1;
#endif

		എങ്കിൽ (len == 0)
			നിർത്തുക;		/* end of string */

		എങ്കിൽ (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			എങ്കിൽ (!isprint((unsigned അക്ഷരം) *p)) {
				എങ്കിൽ (rsz < 5)
					നിർത്തുക;
				snprintf(r, rsz, "\\x%02x", (unsigned അക്ഷരം) *p);
				r += 4;
				rsz -= 4;
				*width += 4;
			} അല്ലെങ്കിൽ {
				എങ്കിൽ (rsz < 2)
					നിർത്തുക;
				(*width)++;
				*r++ = *p;
				rsz--;
			}
		} അല്ലെങ്കിൽ എങ്കിൽ (*p == '\\' && *(p + 1) == 'x') {
			എങ്കിൽ (rsz < 5)
				നിർത്തുക;
			snprintf(r, rsz, "\\x%02x", (unsigned അക്ഷരം) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (rsz < (പൂർണ്ണസംഖ്യ)len + 1)
				നിർത്തുക;
			r = mempcpy(r, p, len);
			rsz -= len;
			*width += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	തിരികെ buf;
}

/*
 * Guess size
 */
size_t mbs_safe_encode_size(size_t bytes)
{
	തിരികെ (bytes * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_safe_decode_size(സ്ഥിരം അക്ഷരം *p)
{
	size_t bytes = 0;

	ആകുമ്പോൾ (p && *p) {
		എങ്കിൽ (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		അല്ലെങ്കിൽ
			p++;
		bytes++;
	}
	തിരികെ bytes;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
അക്ഷരം *mbs_safe_encode(സ്ഥിരം അക്ഷരം *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	അക്ഷരം *buf, *ret = ശൂന്യ;

	എങ്കിൽ (!sz)
		തിരികെ ശൂന്യ;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	എങ്കിൽ (buf)
		ret = mbs_safe_encode_to_buffer(s, width, buf, bufsz, ശൂന്യ);
	എങ്കിൽ (!ret)
		സ്വതന്ത്രം(buf);
	തിരികെ ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
അക്ഷരം *mbs_invalid_encode(സ്ഥിരം അക്ഷരം *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	അക്ഷരം *buf, *ret = ശൂന്യ;

	എങ്കിൽ (!sz)
		തിരികെ ശൂന്യ;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	എങ്കിൽ (buf)
		ret = mbs_invalid_encode_to_buffer(s, width, buf, bufsz);
	എങ്കിൽ (!ret)
		സ്വതന്ത്രം(buf);
	തിരികെ ret;
}

#ifdef HAVE_WIDECHAR

സ്ഥിതം bool
wc_ensure_printable (wchar_t *wchars)
{
  bool replaced = false;
  wchar_t *wc = wchars;
  ആകുമ്പോൾ (*wc)
    {
      എങ്കിൽ (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  തിരികെ replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
സ്ഥിതം wchar_t *
wc_skip_ansi_escape (wchar_t *wc)
{
  wchar_t *p = wc + 1;

  എങ്കിൽ (*wc != L'\033' || !*p)
    തിരികെ ശൂന്യ;

  എങ്കിൽ (*p == L'[')
    {
      p++;
      ആകുമ്പോൾ (*p >= 0x30 && *p <= 0x3f)
        p++;
      ആകുമ്പോൾ (*p >= 0x20 && *p <= 0x2f)
        p++;
      എങ്കിൽ (*p >= 0x40 && *p <= 0x7e)
        തിരികെ p + 1;
    }
  അല്ലെങ്കിൽ എങ്കിൽ ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    തിരികെ p + 2;

  തിരികെ ശൂന്യ;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

സ്ഥിതം size_t
wc_truncate (wchar_t *wc, size_t width)
{
  size_t cells = 0;
  പൂർണ്ണസംഖ്യ next_cells = 0;

  ആകുമ്പോൾ (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      എങ്കിൽ (after_escape)
        {
          wc = after_escape;
          തുടരുക;
        }

      next_cells = wcwidth (*wc);
      എങ്കിൽ (next_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          next_cells = 1;
        }
      എങ്കിൽ (cells + next_cells > width)
        നിർത്തുക;

      cells += next_cells;
      wc++;
    }
  *wc = L'\0';
  തിരികെ cells;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
rpl_wcswidth (സ്ഥിരം wchar_t *s, size_t n)
{
  പൂർണ്ണസംഖ്യ ret = 0;

  ആകുമ്പോൾ (n-- > 0 && *s != L'\0')
    {
      പൂർണ്ണസംഖ്യ nwidth = wcwidth (*s++);
      എങ്കിൽ (nwidth == -1)             /* non printable */
        തിരികെ -1;
      എങ്കിൽ (ret > (INT_MAX - nwidth)) /* overflow */
        തിരികെ -1;
      ret += nwidth;
    }

  തിരികെ ret;
}
#endif /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
size_t
mbs_truncate(അക്ഷരം *str, size_t *width)
{
	ssize_t bytes = strlen(str);
#ifdef HAVE_WIDECHAR
	ssize_t sz = mbstowcs(ശൂന്യ, str, 0);
	wchar_t *wcs = ശൂന്യ;

	എങ്കിൽ (sz == (ssize_t) -1)
		പോകുക done;

	wcs = calloc(1, (sz + 1) * വലുപ്പം(wchar_t));
	എങ്കിൽ (!wcs)
		പോകുക done;

	എങ്കിൽ (!mbstowcs(wcs, str, sz))
		പോകുക done;
	*width = wc_truncate(wcs, *width);
	bytes = wcstombs(str, wcs, bytes);
done:
	സ്വതന്ത്രം(wcs);
#അല്ലെങ്കിൽ
	{
		സ്ഥിരം അക്ഷരം *p = str;
		സ്ഥിരം അക്ഷരം *end = str + bytes;
		size_t cells = 0;

		ആകുമ്പോൾ (p < end) {
			സ്ഥിരം അക്ഷരം *after_escape = skip_ansi_escape(p, end);

			എങ്കിൽ (after_escape) {
				p = after_escape;
				തുടരുക;
			}
			എങ്കിൽ (cells >= *width)
				നിർത്തുക;
			p++;
			cells++;
		}
		bytes = p - str;
		*width = cells;
	}
#endif
	എങ്കിൽ (bytes >= 0)
		str[bytes] = '\0';
	തിരികെ bytes;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

സ്ഥിതം അക്ഷരം*
mbs_align_pad (അക്ഷരം *dest, സ്ഥിരം അക്ഷരം* dest_end, size_t n_spaces, പൂർണ്ണസംഖ്യ padchar)
{
  വേണ്ടി (/* nothing */; n_spaces && (dest < dest_end); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  തിരികെ dest;
}

size_t
mbsalign (സ്ഥിരം അക്ഷരം *src, അക്ഷരം *dest, size_t dest_size,
          size_t *width, mbs_align_t align, പൂർണ്ണസംഖ്യ flags)
{
	തിരികെ mbsalign_with_padding(src, dest, dest_size, width, align, flags, ' ');
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
mbsalign_with_padding (സ്ഥിരം അക്ഷരം *src, അക്ഷരം *dest, size_t dest_size,
	               size_t *width, mbs_align_t align,
#ifdef HAVE_WIDECHAR
		       പൂർണ്ണസംഖ്യ flags,
#അല്ലെങ്കിൽ
		       പൂർണ്ണസംഖ്യ flags __attribute__((__unused__)),
#endif
		       പൂർണ്ണസംഖ്യ padchar)
{
  size_t ret = -1;
  size_t src_size = strlen (src) + 1;
  അക്ഷരം *newstr = ശൂന്യ;
  wchar_t *str_wc = ശൂന്യ;
  സ്ഥിരം അക്ഷരം *str_to_print = src;
  size_t n_cols = src_size - 1;
  size_t n_used_bytes = n_cols; /* Not including NUL */
  size_t n_spaces = 0, space_ഇടത്;

#ifdef HAVE_WIDECHAR
  bool conversion = false;
  bool wc_enabled = false;

  /* In multi-byte locales convert to wide characters
     to allow easy truncation. Also determine number
     of screen columns used.  */
  എങ്കിൽ (MB_CUR_MAX > 1)
    {
      size_t src_chars = mbstowcs (ശൂന്യ, src, 0);
      എങ്കിൽ (src_chars == (size_t) -1)
        {
          എങ്കിൽ (flags & MBA_UNIBYTE_FALLBACK)
            പോകുക mbsalign_unibyte;
          അല്ലെങ്കിൽ
            പോകുക mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = malloc (src_chars * വലുപ്പം (wchar_t));
      എങ്കിൽ (str_wc == ശൂന്യ)
        {
          എങ്കിൽ (flags & MBA_UNIBYTE_FALLBACK)
            പോകുക mbsalign_unibyte;
          അല്ലെങ്കിൽ
            പോകുക mbsalign_cleanup;
        }
      എങ്കിൽ (mbstowcs (str_wc, src, src_chars) != 0)
        {
          str_wc[src_chars - 1] = L'\0';
          wc_enabled = true;
          conversion = wc_ensure_printable (str_wc);
          n_cols = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  എങ്കിൽ (wc_enabled && (conversion || (n_cols > *width)))
    {
        എങ്കിൽ (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_size = wcstombs(ശൂന്യ, str_wc, 0) + 1;
          }
        newstr = malloc (src_size);
        എങ്കിൽ (newstr == ശൂന്യ)
        {
          എങ്കിൽ (flags & MBA_UNIBYTE_FALLBACK)
            പോകുക mbsalign_unibyte;
          അല്ലെങ്കിൽ
            പോകുക mbsalign_cleanup;
        }
        str_to_print = newstr;
        n_cols = wc_truncate (str_wc, *width);
        n_used_bytes = wcstombs (newstr, str_wc, src_size);
    }

mbsalign_unibyte:
#endif

  എങ്കിൽ (n_cols > *width) /* Unibyte truncation required.  */
    {
      n_cols = *width;
      n_used_bytes = n_cols;
    }

  എങ്കിൽ (*width > n_cols) /* Padding required.  */
    n_spaces = *width - n_cols;

  /* indicate to caller how many cells needed (not including padding).  */
  *width = n_cols;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_used_bytes + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  എങ്കിൽ (dest_size != 0)
    {
      അക്ഷരം *dest_end = dest + dest_size - 1;
      size_t start_spaces;
      size_t end_spaces;

      തിരഞ്ഞെടുപ്പ് (align)
        {
        അവസ്ഥ MBS_ALIGN_CENTER:
          start_spaces = n_spaces / 2 + n_spaces % 2;
          end_spaces = n_spaces / 2;
          നിർത്തുക;
        അവസ്ഥ MBS_ALIGN_ഇടത്:
          start_spaces = 0;
          end_spaces = n_spaces;
          നിർത്തുക;
        അവസ്ഥ MBS_ALIGN_വലത്:
          start_spaces = n_spaces;
          end_spaces = 0;
          നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
	  abort();
        }

      dest = mbs_align_pad (dest, dest_end, start_spaces, padchar);
      space_ഇടത് = dest_end - dest;
      dest = mempcpy (dest, str_to_print, min (n_used_bytes, space_ഇടത്));
      mbs_align_pad (dest, dest_end, end_spaces, padchar);
    }
#ifdef HAVE_WIDECHAR
mbsalign_cleanup:
#endif
  സ്വതന്ത്രം (str_wc);
  സ്വതന്ത്രം (newstr);

  തിരികെ ret;
}
