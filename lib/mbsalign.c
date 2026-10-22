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
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <ctype.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "mbsalign.h"
#અન્તર્ભાવ "strutils.h"
#અન્તર્ભાવ "widechar.h"

/* Return the byte after a complete ANSI escape sequence, or NULL. */
સ્થિર અચલ અક્ષર *skip_ansi_escape(અચલ અક્ષર *p, અચલ અક્ષર *end)
{
	અચલ અક્ષર *q;

	જો (p >= end || (unsigned અક્ષર) *p != '\033' || p + 1 >= end)
		પાછા ખાલી;

	q = p + 1;
	જો (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		જ્યારે (q < end && (unsigned અક્ષર) *q >= 0x30
				&& (unsigned અક્ષર) *q <= 0x3f)
			q++;
		જ્યારે (q < end && (unsigned અક્ષર) *q >= 0x20
				&& (unsigned અક્ષર) *q <= 0x2f)
			q++;
		જો (q < end && (unsigned અક્ષર) *q >= 0x40
				&& (unsigned અક્ષર) *q <= 0x7e)
			પાછા q + 1;
	} નહીંતર જો (*q && strchr("()*+", *q) && q + 1 < end)
		પાછા q + 2;

	પાછા ખાલી;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
size_t mbs_nwidth(અચલ અક્ષર *buf, size_t bufsz)
{
	અચલ અક્ષર *p = buf, *last = buf;
	size_t width = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, કદ(st));
#endif
	જો (p && *p && bufsz)
		last = p + (bufsz - 1);

	જ્યારે (p && *p && p <= last) {
		જો (*p == '\033') {
			અચલ અક્ષર *e = skip_ansi_escape(p, last + 1);

			p = e ? e : p + 1;
			ચાલુ;
		}
		જો (iscntrl((unsigned અક્ષર) *p)) {
			p++;
			ચાલુ;
		}
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		જો (len == 0)
			તોડો;
		જો (len > 0 && iswprint(wc)) {
			પૂર્ણાંક x = wcwidth(wc);
			જો (x > 0)
				width += x;
		} નહીંતર જો (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#નહીંતર
		જો (isprint((unsigned અક્ષર) *p))
			width++;
		p++;
#endif
	}

	પાછા width;
}

size_t mbs_width(અચલ અક્ષર *s)
{
	જો (!s || !*s)
		પાછા 0;
	પાછા mbs_nwidth(s, strlen(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
size_t mbs_safe_nwidth(અચલ અક્ષર *buf, size_t bufsz, size_t *sz)
{
	અચલ અક્ષર *p = buf, *last = buf;
	size_t width = 0, bytes = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, કદ(st));
#endif
	જો (p && *p && bufsz)
		last = p + (bufsz - 1);

	જ્યારે (p && *p && p <= last) {
		જો ((p < last && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned અક્ષર) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		}
#ifdef HAVE_WIDECHAR
		નહીંતર {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			જો (len == 0)
				તોડો;

			જો (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				જો (isprint((unsigned અક્ષર) *p))
					width += 1, bytes += 1;
				નહીંતર
					width += 4, bytes += 4;

			} નહીંતર જો (!iswprint(wc)) {
				width += len * 4;	/* hex encode whole sequence */
				bytes += len * 4;
			} નહીંતર {
				width += wcwidth(wc);	/* number of cells */
				bytes += len;		/* number of bytes */
			}
			p += len;
		}
#નહીંતર
		નહીંતર જો (!isprint((unsigned અક્ષર) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		} નહીંતર {
			width++, bytes++;
			p++;
		}
#endif
	}

	જો (sz)
		*sz = bytes;
	પાછા width;
}

size_t mbs_safe_width(અચલ અક્ષર *s)
{
	જો (!s || !*s)
		પાછા 0;
	પાછા mbs_safe_nwidth(s, strlen(s), ખાલી);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
અક્ષર *mbs_safe_encode_to_buffer(અચલ અક્ષર *s, size_t *width, અક્ષર *buf, size_t bufsiz, અચલ અક્ષર *safechars)
{
	અચલ અક્ષર *p = s;
	અક્ષર *r;
	પૂર્ણાંક rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, કદ(st));
#endif
	જો (!sz || !buf || !bufsiz)
		પાછા ખાલી;

	r = buf;
	rsz = (પૂર્ણાંક) bufsiz;
	*width = 0;

	જ્યારે (p && *p) {
		જો (safechars && strchr(safechars, *p)) {
			જો (rsz < 2)
				તોડો;
			*r++ = *p++;
			rsz--;
			ચાલુ;
		}

		જો ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned અક્ષર) *p)) {
			જો (rsz < 5)
				તોડો;
			snprintf(r, rsz, "\\x%02x", (unsigned અક્ષર) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
			p++;
		}
#ifdef HAVE_WIDECHAR
		નહીંતર {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			જો (len == 0)
				તોડો;		/* end of string */

			જો (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				જો (!isprint((unsigned અક્ષર) *p)) {
					જો (rsz < 5)
						તોડો;
					snprintf(r, rsz, "\\x%02x", (unsigned અક્ષર) *p);
					r += 4;
					rsz -= 4;
					*width += 4;
				} નહીંતર {
					જો (rsz < 2)
						તોડો;
					(*width)++;
					*r++ = *p;
					rsz--;
				}
			} નહીંતર જો (!iswprint(wc)) {
				size_t i;
				માટે (i = 0; i < len; i++) {
					જો (rsz < 5)
						તોડો;
					snprintf(r, rsz, "\\x%02x", (unsigned અક્ષર) p[i]);
					r += 4;
					rsz -= 4;
					*width += 4;
				}
				જો (i < len)
					તોડો;
			} નહીંતર {
				જો (rsz < (પૂર્ણાંક)len + 1)
					તોડો;
				memcpy(r, p, len);
				r += len;
				rsz -= len;
				*width += wcwidth(wc);
			}
			p += len;
		}
#નહીંતર
		નહીંતર જો (!isprint((unsigned અક્ષર) *p)) {
			જો (rsz < 5)
				તોડો;
			snprintf(r, rsz, "\\x%02x", (unsigned અક્ષર) *p);
			p++;
			r += 4;
			rsz -= 4;
			*width += 4;
		} નહીંતર {
			જો (rsz < 2)
				તોડો;
			*r++ = *p++;
			rsz--;
			(*width)++;
		}
#endif
	}

	*r = '\0';
	પાછા buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
અક્ષર *mbs_invalid_encode_to_buffer(અચલ અક્ષર *s, size_t *width, અક્ષર *buf, size_t bufsiz)
{
	અચલ અક્ષર *p = s;
	અક્ષર *r;
	પૂર્ણાંક rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, કદ(st));
#endif
	જો (!sz || !buf || !bufsiz)
		પાછા ખાલી;

	r = buf;
	rsz = (પૂર્ણાંક) bufsiz;
	*width = 0;

	જ્યારે (p && *p) {
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#નહીંતર
		size_t len = 1;
#endif

		જો (len == 0)
			તોડો;		/* end of string */

		જો (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			જો (!isprint((unsigned અક્ષર) *p)) {
				જો (rsz < 5)
					તોડો;
				snprintf(r, rsz, "\\x%02x", (unsigned અક્ષર) *p);
				r += 4;
				rsz -= 4;
				*width += 4;
			} નહીંતર {
				જો (rsz < 2)
					તોડો;
				(*width)++;
				*r++ = *p;
				rsz--;
			}
		} નહીંતર જો (*p == '\\' && *(p + 1) == 'x') {
			જો (rsz < 5)
				તોડો;
			snprintf(r, rsz, "\\x%02x", (unsigned અક્ષર) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
		} નહીંતર {
			જો (rsz < (પૂર્ણાંક)len + 1)
				તોડો;
			r = mempcpy(r, p, len);
			rsz -= len;
			*width += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	પાછા buf;
}

/*
 * Guess size
 */
size_t mbs_safe_encode_size(size_t bytes)
{
	પાછા (bytes * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_safe_decode_size(અચલ અક્ષર *p)
{
	size_t bytes = 0;

	જ્યારે (p && *p) {
		જો (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		નહીંતર
			p++;
		bytes++;
	}
	પાછા bytes;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
અક્ષર *mbs_safe_encode(અચલ અક્ષર *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	અક્ષર *buf, *ret = ખાલી;

	જો (!sz)
		પાછા ખાલી;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	જો (buf)
		ret = mbs_safe_encode_to_buffer(s, width, buf, bufsz, ખાલી);
	જો (!ret)
		મુક્ત(buf);
	પાછા ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
અક્ષર *mbs_invalid_encode(અચલ અક્ષર *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	અક્ષર *buf, *ret = ખાલી;

	જો (!sz)
		પાછા ખાલી;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	જો (buf)
		ret = mbs_invalid_encode_to_buffer(s, width, buf, bufsz);
	જો (!ret)
		મુક્ત(buf);
	પાછા ret;
}

#ifdef HAVE_WIDECHAR

સ્થિર bool
wc_ensure_printable (wchar_t *wchars)
{
  bool replaced = false;
  wchar_t *wc = wchars;
  જ્યારે (*wc)
    {
      જો (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  પાછા replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
સ્થિર wchar_t *
wc_skip_ansi_escape (wchar_t *wc)
{
  wchar_t *p = wc + 1;

  જો (*wc != L'\033' || !*p)
    પાછા ખાલી;

  જો (*p == L'[')
    {
      p++;
      જ્યારે (*p >= 0x30 && *p <= 0x3f)
        p++;
      જ્યારે (*p >= 0x20 && *p <= 0x2f)
        p++;
      જો (*p >= 0x40 && *p <= 0x7e)
        પાછા p + 1;
    }
  નહીંતર જો ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    પાછા p + 2;

  પાછા ખાલી;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

સ્થિર size_t
wc_truncate (wchar_t *wc, size_t width)
{
  size_t cells = 0;
  પૂર્ણાંક next_cells = 0;

  જ્યારે (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      જો (after_escape)
        {
          wc = after_escape;
          ચાલુ;
        }

      next_cells = wcwidth (*wc);
      જો (next_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          next_cells = 1;
        }
      જો (cells + next_cells > width)
        તોડો;

      cells += next_cells;
      wc++;
    }
  *wc = L'\0';
  પાછા cells;
}

સ્થિર પૂર્ણાંક
rpl_wcswidth (અચલ wchar_t *s, size_t n)
{
  પૂર્ણાંક ret = 0;

  જ્યારે (n-- > 0 && *s != L'\0')
    {
      પૂર્ણાંક nwidth = wcwidth (*s++);
      જો (nwidth == -1)             /* non printable */
        પાછા -1;
      જો (ret > (INT_MAX - nwidth)) /* overflow */
        પાછા -1;
      ret += nwidth;
    }

  પાછા ret;
}
#endif /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
size_t
mbs_truncate(અક્ષર *str, size_t *width)
{
	ssize_t bytes = strlen(str);
#ifdef HAVE_WIDECHAR
	ssize_t sz = mbstowcs(ખાલી, str, 0);
	wchar_t *wcs = ખાલી;

	જો (sz == (ssize_t) -1)
		જાઓ done;

	wcs = calloc(1, (sz + 1) * કદ(wchar_t));
	જો (!wcs)
		જાઓ done;

	જો (!mbstowcs(wcs, str, sz))
		જાઓ done;
	*width = wc_truncate(wcs, *width);
	bytes = wcstombs(str, wcs, bytes);
done:
	મુક્ત(wcs);
#નહીંતર
	{
		અચલ અક્ષર *p = str;
		અચલ અક્ષર *end = str + bytes;
		size_t cells = 0;

		જ્યારે (p < end) {
			અચલ અક્ષર *after_escape = skip_ansi_escape(p, end);

			જો (after_escape) {
				p = after_escape;
				ચાલુ;
			}
			જો (cells >= *width)
				તોડો;
			p++;
			cells++;
		}
		bytes = p - str;
		*width = cells;
	}
#endif
	જો (bytes >= 0)
		str[bytes] = '\0';
	પાછા bytes;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

સ્થિર અક્ષર*
mbs_align_pad (અક્ષર *dest, અચલ અક્ષર* dest_end, size_t n_spaces, પૂર્ણાંક padchar)
{
  માટે (/* nothing */; n_spaces && (dest < dest_end); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  પાછા dest;
}

size_t
mbsalign (અચલ અક્ષર *src, અક્ષર *dest, size_t dest_size,
          size_t *width, mbs_align_t align, પૂર્ણાંક flags)
{
	પાછા mbsalign_with_padding(src, dest, dest_size, width, align, flags, ' ');
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
mbsalign_with_padding (અચલ અક્ષર *src, અક્ષર *dest, size_t dest_size,
	               size_t *width, mbs_align_t align,
#ifdef HAVE_WIDECHAR
		       પૂર્ણાંક flags,
#નહીંતર
		       પૂર્ણાંક flags __attribute__((__unused__)),
#endif
		       પૂર્ણાંક padchar)
{
  size_t ret = -1;
  size_t src_size = strlen (src) + 1;
  અક્ષર *newstr = ખાલી;
  wchar_t *str_wc = ખાલી;
  અચલ અક્ષર *str_to_print = src;
  size_t n_cols = src_size - 1;
  size_t n_used_bytes = n_cols; /* Not including NUL */
  size_t n_spaces = 0, space_ડાબું;

#ifdef HAVE_WIDECHAR
  bool conversion = false;
  bool wc_enabled = false;

  /* In multi-byte locales convert to wide characters
     to allow easy truncation. Also determine number
     of screen columns used.  */
  જો (MB_CUR_MAX > 1)
    {
      size_t src_chars = mbstowcs (ખાલી, src, 0);
      જો (src_chars == (size_t) -1)
        {
          જો (flags & MBA_UNIBYTE_FALLBACK)
            જાઓ mbsalign_unibyte;
          નહીંતર
            જાઓ mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = malloc (src_chars * કદ (wchar_t));
      જો (str_wc == ખાલી)
        {
          જો (flags & MBA_UNIBYTE_FALLBACK)
            જાઓ mbsalign_unibyte;
          નહીંતર
            જાઓ mbsalign_cleanup;
        }
      જો (mbstowcs (str_wc, src, src_chars) != 0)
        {
          str_wc[src_chars - 1] = L'\0';
          wc_enabled = true;
          conversion = wc_ensure_printable (str_wc);
          n_cols = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  જો (wc_enabled && (conversion || (n_cols > *width)))
    {
        જો (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_size = wcstombs(ખાલી, str_wc, 0) + 1;
          }
        newstr = malloc (src_size);
        જો (newstr == ખાલી)
        {
          જો (flags & MBA_UNIBYTE_FALLBACK)
            જાઓ mbsalign_unibyte;
          નહીંતર
            જાઓ mbsalign_cleanup;
        }
        str_to_print = newstr;
        n_cols = wc_truncate (str_wc, *width);
        n_used_bytes = wcstombs (newstr, str_wc, src_size);
    }

mbsalign_unibyte:
#endif

  જો (n_cols > *width) /* Unibyte truncation required.  */
    {
      n_cols = *width;
      n_used_bytes = n_cols;
    }

  જો (*width > n_cols) /* Padding required.  */
    n_spaces = *width - n_cols;

  /* indicate to caller how many cells needed (not including padding).  */
  *width = n_cols;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_used_bytes + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  જો (dest_size != 0)
    {
      અક્ષર *dest_end = dest + dest_size - 1;
      size_t start_spaces;
      size_t end_spaces;

      પસંદગી (align)
        {
        સ્થિતિ MBS_ALIGN_CENTER:
          start_spaces = n_spaces / 2 + n_spaces % 2;
          end_spaces = n_spaces / 2;
          તોડો;
        સ્થિતિ MBS_ALIGN_ડાબું:
          start_spaces = 0;
          end_spaces = n_spaces;
          તોડો;
        સ્થિતિ MBS_ALIGN_જમણું:
          start_spaces = n_spaces;
          end_spaces = 0;
          તોડો;
	મૂળભૂત:
	  abort();
        }

      dest = mbs_align_pad (dest, dest_end, start_spaces, padchar);
      space_ડાબું = dest_end - dest;
      dest = mempcpy (dest, str_to_print, min (n_used_bytes, space_ડાબું));
      mbs_align_pad (dest, dest_end, end_spaces, padchar);
    }
#ifdef HAVE_WIDECHAR
mbsalign_cleanup:
#endif
  મુક્ત (str_wc);
  મુક્ત (newstr);

  પાછા ret;
}
