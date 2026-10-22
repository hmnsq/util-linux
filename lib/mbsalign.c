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
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <ctype.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "mbsalign.h"
#அந்தர்பாவ "strutils.h"
#அந்தர்பாவ "widechar.h"

/* Return the byte after a complete ANSI escape sequence, or NULL. */
நிலையான மாறிலி எழுத்து *skip_ansi_escape(மாறிலி எழுத்து *p, மாறிலி எழுத்து *end)
{
	மாறிலி எழுத்து *q;

	எனில் (p >= end || (unsigned எழுத்து) *p != '\033' || p + 1 >= end)
		திரும்பு வெற்று;

	q = p + 1;
	எனில் (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		போது (q < end && (unsigned எழுத்து) *q >= 0x30
				&& (unsigned எழுத்து) *q <= 0x3f)
			q++;
		போது (q < end && (unsigned எழுத்து) *q >= 0x20
				&& (unsigned எழுத்து) *q <= 0x2f)
			q++;
		எனில் (q < end && (unsigned எழுத்து) *q >= 0x40
				&& (unsigned எழுத்து) *q <= 0x7e)
			திரும்பு q + 1;
	} இல்லையெனில் எனில் (*q && strchr("()*+", *q) && q + 1 < end)
		திரும்பு q + 2;

	திரும்பு வெற்று;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
size_t mbs_nwidth(மாறிலி எழுத்து *buf, size_t bufsz)
{
	மாறிலி எழுத்து *p = buf, *last = buf;
	size_t width = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, அளவு(st));
#endif
	எனில் (p && *p && bufsz)
		last = p + (bufsz - 1);

	போது (p && *p && p <= last) {
		எனில் (*p == '\033') {
			மாறிலி எழுத்து *e = skip_ansi_escape(p, last + 1);

			p = e ? e : p + 1;
			தொடர்;
		}
		எனில் (iscntrl((unsigned எழுத்து) *p)) {
			p++;
			தொடர்;
		}
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		எனில் (len == 0)
			நிறுத்து;
		எனில் (len > 0 && iswprint(wc)) {
			முழு x = wcwidth(wc);
			எனில் (x > 0)
				width += x;
		} இல்லையெனில் எனில் (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#இல்லையெனில்
		எனில் (isprint((unsigned எழுத்து) *p))
			width++;
		p++;
#endif
	}

	திரும்பு width;
}

size_t mbs_width(மாறிலி எழுத்து *s)
{
	எனில் (!s || !*s)
		திரும்பு 0;
	திரும்பு mbs_nwidth(s, strlen(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
size_t mbs_safe_nwidth(மாறிலி எழுத்து *buf, size_t bufsz, size_t *sz)
{
	மாறிலி எழுத்து *p = buf, *last = buf;
	size_t width = 0, bytes = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, அளவு(st));
#endif
	எனில் (p && *p && bufsz)
		last = p + (bufsz - 1);

	போது (p && *p && p <= last) {
		எனில் ((p < last && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned எழுத்து) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		}
#ifdef HAVE_WIDECHAR
		இல்லையெனில் {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			எனில் (len == 0)
				நிறுத்து;

			எனில் (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				எனில் (isprint((unsigned எழுத்து) *p))
					width += 1, bytes += 1;
				இல்லையெனில்
					width += 4, bytes += 4;

			} இல்லையெனில் எனில் (!iswprint(wc)) {
				width += len * 4;	/* hex encode whole sequence */
				bytes += len * 4;
			} இல்லையெனில் {
				width += wcwidth(wc);	/* number of cells */
				bytes += len;		/* number of bytes */
			}
			p += len;
		}
#இல்லையெனில்
		இல்லையெனில் எனில் (!isprint((unsigned எழுத்து) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		} இல்லையெனில் {
			width++, bytes++;
			p++;
		}
#endif
	}

	எனில் (sz)
		*sz = bytes;
	திரும்பு width;
}

size_t mbs_safe_width(மாறிலி எழுத்து *s)
{
	எனில் (!s || !*s)
		திரும்பு 0;
	திரும்பு mbs_safe_nwidth(s, strlen(s), வெற்று);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
எழுத்து *mbs_safe_encode_to_buffer(மாறிலி எழுத்து *s, size_t *width, எழுத்து *buf, size_t bufsiz, மாறிலி எழுத்து *safechars)
{
	மாறிலி எழுத்து *p = s;
	எழுத்து *r;
	முழு rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, அளவு(st));
#endif
	எனில் (!sz || !buf || !bufsiz)
		திரும்பு வெற்று;

	r = buf;
	rsz = (முழு) bufsiz;
	*width = 0;

	போது (p && *p) {
		எனில் (safechars && strchr(safechars, *p)) {
			எனில் (rsz < 2)
				நிறுத்து;
			*r++ = *p++;
			rsz--;
			தொடர்;
		}

		எனில் ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned எழுத்து) *p)) {
			எனில் (rsz < 5)
				நிறுத்து;
			snprintf(r, rsz, "\\x%02x", (unsigned எழுத்து) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
			p++;
		}
#ifdef HAVE_WIDECHAR
		இல்லையெனில் {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			எனில் (len == 0)
				நிறுத்து;		/* end of string */

			எனில் (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				எனில் (!isprint((unsigned எழுத்து) *p)) {
					எனில் (rsz < 5)
						நிறுத்து;
					snprintf(r, rsz, "\\x%02x", (unsigned எழுத்து) *p);
					r += 4;
					rsz -= 4;
					*width += 4;
				} இல்லையெனில் {
					எனில் (rsz < 2)
						நிறுத்து;
					(*width)++;
					*r++ = *p;
					rsz--;
				}
			} இல்லையெனில் எனில் (!iswprint(wc)) {
				size_t i;
				க்கு (i = 0; i < len; i++) {
					எனில் (rsz < 5)
						நிறுத்து;
					snprintf(r, rsz, "\\x%02x", (unsigned எழுத்து) p[i]);
					r += 4;
					rsz -= 4;
					*width += 4;
				}
				எனில் (i < len)
					நிறுத்து;
			} இல்லையெனில் {
				எனில் (rsz < (முழு)len + 1)
					நிறுத்து;
				memcpy(r, p, len);
				r += len;
				rsz -= len;
				*width += wcwidth(wc);
			}
			p += len;
		}
#இல்லையெனில்
		இல்லையெனில் எனில் (!isprint((unsigned எழுத்து) *p)) {
			எனில் (rsz < 5)
				நிறுத்து;
			snprintf(r, rsz, "\\x%02x", (unsigned எழுத்து) *p);
			p++;
			r += 4;
			rsz -= 4;
			*width += 4;
		} இல்லையெனில் {
			எனில் (rsz < 2)
				நிறுத்து;
			*r++ = *p++;
			rsz--;
			(*width)++;
		}
#endif
	}

	*r = '\0';
	திரும்பு buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
எழுத்து *mbs_invalid_encode_to_buffer(மாறிலி எழுத்து *s, size_t *width, எழுத்து *buf, size_t bufsiz)
{
	மாறிலி எழுத்து *p = s;
	எழுத்து *r;
	முழு rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, அளவு(st));
#endif
	எனில் (!sz || !buf || !bufsiz)
		திரும்பு வெற்று;

	r = buf;
	rsz = (முழு) bufsiz;
	*width = 0;

	போது (p && *p) {
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#இல்லையெனில்
		size_t len = 1;
#endif

		எனில் (len == 0)
			நிறுத்து;		/* end of string */

		எனில் (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			எனில் (!isprint((unsigned எழுத்து) *p)) {
				எனில் (rsz < 5)
					நிறுத்து;
				snprintf(r, rsz, "\\x%02x", (unsigned எழுத்து) *p);
				r += 4;
				rsz -= 4;
				*width += 4;
			} இல்லையெனில் {
				எனில் (rsz < 2)
					நிறுத்து;
				(*width)++;
				*r++ = *p;
				rsz--;
			}
		} இல்லையெனில் எனில் (*p == '\\' && *(p + 1) == 'x') {
			எனில் (rsz < 5)
				நிறுத்து;
			snprintf(r, rsz, "\\x%02x", (unsigned எழுத்து) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
		} இல்லையெனில் {
			எனில் (rsz < (முழு)len + 1)
				நிறுத்து;
			r = mempcpy(r, p, len);
			rsz -= len;
			*width += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	திரும்பு buf;
}

/*
 * Guess size
 */
size_t mbs_safe_encode_size(size_t bytes)
{
	திரும்பு (bytes * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_safe_decode_size(மாறிலி எழுத்து *p)
{
	size_t bytes = 0;

	போது (p && *p) {
		எனில் (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		இல்லையெனில்
			p++;
		bytes++;
	}
	திரும்பு bytes;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
எழுத்து *mbs_safe_encode(மாறிலி எழுத்து *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	எழுத்து *buf, *ret = வெற்று;

	எனில் (!sz)
		திரும்பு வெற்று;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	எனில் (buf)
		ret = mbs_safe_encode_to_buffer(s, width, buf, bufsz, வெற்று);
	எனில் (!ret)
		விடுவி(buf);
	திரும்பு ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
எழுத்து *mbs_invalid_encode(மாறிலி எழுத்து *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	எழுத்து *buf, *ret = வெற்று;

	எனில் (!sz)
		திரும்பு வெற்று;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	எனில் (buf)
		ret = mbs_invalid_encode_to_buffer(s, width, buf, bufsz);
	எனில் (!ret)
		விடுவி(buf);
	திரும்பு ret;
}

#ifdef HAVE_WIDECHAR

நிலையான bool
wc_ensure_printable (wchar_t *wchars)
{
  bool replaced = false;
  wchar_t *wc = wchars;
  போது (*wc)
    {
      எனில் (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  திரும்பு replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
நிலையான wchar_t *
wc_skip_ansi_escape (wchar_t *wc)
{
  wchar_t *p = wc + 1;

  எனில் (*wc != L'\033' || !*p)
    திரும்பு வெற்று;

  எனில் (*p == L'[')
    {
      p++;
      போது (*p >= 0x30 && *p <= 0x3f)
        p++;
      போது (*p >= 0x20 && *p <= 0x2f)
        p++;
      எனில் (*p >= 0x40 && *p <= 0x7e)
        திரும்பு p + 1;
    }
  இல்லையெனில் எனில் ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    திரும்பு p + 2;

  திரும்பு வெற்று;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

நிலையான size_t
wc_truncate (wchar_t *wc, size_t width)
{
  size_t cells = 0;
  முழு next_cells = 0;

  போது (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      எனில் (after_escape)
        {
          wc = after_escape;
          தொடர்;
        }

      next_cells = wcwidth (*wc);
      எனில் (next_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          next_cells = 1;
        }
      எனில் (cells + next_cells > width)
        நிறுத்து;

      cells += next_cells;
      wc++;
    }
  *wc = L'\0';
  திரும்பு cells;
}

நிலையான முழு
rpl_wcswidth (மாறிலி wchar_t *s, size_t n)
{
  முழு ret = 0;

  போது (n-- > 0 && *s != L'\0')
    {
      முழு nwidth = wcwidth (*s++);
      எனில் (nwidth == -1)             /* non printable */
        திரும்பு -1;
      எனில் (ret > (INT_MAX - nwidth)) /* overflow */
        திரும்பு -1;
      ret += nwidth;
    }

  திரும்பு ret;
}
#endif /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
size_t
mbs_truncate(எழுத்து *str, size_t *width)
{
	ssize_t bytes = strlen(str);
#ifdef HAVE_WIDECHAR
	ssize_t sz = mbstowcs(வெற்று, str, 0);
	wchar_t *wcs = வெற்று;

	எனில் (sz == (ssize_t) -1)
		செல் done;

	wcs = calloc(1, (sz + 1) * அளவு(wchar_t));
	எனில் (!wcs)
		செல் done;

	எனில் (!mbstowcs(wcs, str, sz))
		செல் done;
	*width = wc_truncate(wcs, *width);
	bytes = wcstombs(str, wcs, bytes);
done:
	விடுவி(wcs);
#இல்லையெனில்
	{
		மாறிலி எழுத்து *p = str;
		மாறிலி எழுத்து *end = str + bytes;
		size_t cells = 0;

		போது (p < end) {
			மாறிலி எழுத்து *after_escape = skip_ansi_escape(p, end);

			எனில் (after_escape) {
				p = after_escape;
				தொடர்;
			}
			எனில் (cells >= *width)
				நிறுத்து;
			p++;
			cells++;
		}
		bytes = p - str;
		*width = cells;
	}
#endif
	எனில் (bytes >= 0)
		str[bytes] = '\0';
	திரும்பு bytes;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

நிலையான எழுத்து*
mbs_align_pad (எழுத்து *dest, மாறிலி எழுத்து* dest_end, size_t n_spaces, முழு padchar)
{
  க்கு (/* nothing */; n_spaces && (dest < dest_end); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  திரும்பு dest;
}

size_t
mbsalign (மாறிலி எழுத்து *src, எழுத்து *dest, size_t dest_size,
          size_t *width, mbs_align_t align, முழு flags)
{
	திரும்பு mbsalign_with_padding(src, dest, dest_size, width, align, flags, ' ');
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
mbsalign_with_padding (மாறிலி எழுத்து *src, எழுத்து *dest, size_t dest_size,
	               size_t *width, mbs_align_t align,
#ifdef HAVE_WIDECHAR
		       முழு flags,
#இல்லையெனில்
		       முழு flags __attribute__((__unused__)),
#endif
		       முழு padchar)
{
  size_t ret = -1;
  size_t src_size = strlen (src) + 1;
  எழுத்து *newstr = வெற்று;
  wchar_t *str_wc = வெற்று;
  மாறிலி எழுத்து *str_to_print = src;
  size_t n_cols = src_size - 1;
  size_t n_used_bytes = n_cols; /* Not including NUL */
  size_t n_spaces = 0, space_இடது;

#ifdef HAVE_WIDECHAR
  bool conversion = false;
  bool wc_enabled = false;

  /* In multi-byte locales convert to wide characters
     to allow easy truncation. Also determine number
     of screen columns used.  */
  எனில் (MB_CUR_MAX > 1)
    {
      size_t src_chars = mbstowcs (வெற்று, src, 0);
      எனில் (src_chars == (size_t) -1)
        {
          எனில் (flags & MBA_UNIBYTE_FALLBACK)
            செல் mbsalign_unibyte;
          இல்லையெனில்
            செல் mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = malloc (src_chars * அளவு (wchar_t));
      எனில் (str_wc == வெற்று)
        {
          எனில் (flags & MBA_UNIBYTE_FALLBACK)
            செல் mbsalign_unibyte;
          இல்லையெனில்
            செல் mbsalign_cleanup;
        }
      எனில் (mbstowcs (str_wc, src, src_chars) != 0)
        {
          str_wc[src_chars - 1] = L'\0';
          wc_enabled = true;
          conversion = wc_ensure_printable (str_wc);
          n_cols = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  எனில் (wc_enabled && (conversion || (n_cols > *width)))
    {
        எனில் (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_size = wcstombs(வெற்று, str_wc, 0) + 1;
          }
        newstr = malloc (src_size);
        எனில் (newstr == வெற்று)
        {
          எனில் (flags & MBA_UNIBYTE_FALLBACK)
            செல் mbsalign_unibyte;
          இல்லையெனில்
            செல் mbsalign_cleanup;
        }
        str_to_print = newstr;
        n_cols = wc_truncate (str_wc, *width);
        n_used_bytes = wcstombs (newstr, str_wc, src_size);
    }

mbsalign_unibyte:
#endif

  எனில் (n_cols > *width) /* Unibyte truncation required.  */
    {
      n_cols = *width;
      n_used_bytes = n_cols;
    }

  எனில் (*width > n_cols) /* Padding required.  */
    n_spaces = *width - n_cols;

  /* indicate to caller how many cells needed (not including padding).  */
  *width = n_cols;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_used_bytes + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  எனில் (dest_size != 0)
    {
      எழுத்து *dest_end = dest + dest_size - 1;
      size_t start_spaces;
      size_t end_spaces;

      தேர்வு (align)
        {
        நிலை MBS_ALIGN_CENTER:
          start_spaces = n_spaces / 2 + n_spaces % 2;
          end_spaces = n_spaces / 2;
          நிறுத்து;
        நிலை MBS_ALIGN_இடது:
          start_spaces = 0;
          end_spaces = n_spaces;
          நிறுத்து;
        நிலை MBS_ALIGN_வலது:
          start_spaces = n_spaces;
          end_spaces = 0;
          நிறுத்து;
	இயல்பு:
	  abort();
        }

      dest = mbs_align_pad (dest, dest_end, start_spaces, padchar);
      space_இடது = dest_end - dest;
      dest = mempcpy (dest, str_to_print, min (n_used_bytes, space_இடது));
      mbs_align_pad (dest, dest_end, end_spaces, padchar);
    }
#ifdef HAVE_WIDECHAR
mbsalign_cleanup:
#endif
  விடுவி (str_wc);
  விடுவி (newstr);

  திரும்பு ret;
}
