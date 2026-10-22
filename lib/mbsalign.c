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
#شامل <stdlib.h>
#شامل <string.h>
#شامل <stdio.h>
#شامل <stdbool.h>
#شامل <limits.h>
#شامل <ctype.h>

#شامل "c.h"
#شامل "mbsalign.h"
#شامل "strutils.h"
#شامل "widechar.h"

/* Return the byte after a complete ANSI escape sequence, or NULL. */
ایستا ثابت نویسه *skip_ansi_escape(ثابت نویسه *p, ثابت نویسه *end)
{
	ثابت نویسه *q;

	اگر (p >= end || (unsigned نویسه) *p != '\033' || p + 1 >= end)
		بازگشت پوچ;

	q = p + 1;
	اگر (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		وقتی (q < end && (unsigned نویسه) *q >= 0x30
				&& (unsigned نویسه) *q <= 0x3f)
			q++;
		وقتی (q < end && (unsigned نویسه) *q >= 0x20
				&& (unsigned نویسه) *q <= 0x2f)
			q++;
		اگر (q < end && (unsigned نویسه) *q >= 0x40
				&& (unsigned نویسه) *q <= 0x7e)
			بازگشت q + 1;
	} وگرنه اگر (*q && strchr("()*+", *q) && q + 1 < end)
		بازگشت q + 2;

	بازگشت پوچ;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
size_t mbs_nwidth(ثابت نویسه *buf, size_t bufsz)
{
	ثابت نویسه *p = buf, *last = buf;
	size_t width = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, اندازه(st));
#endif
	اگر (p && *p && bufsz)
		last = p + (bufsz - 1);

	وقتی (p && *p && p <= last) {
		اگر (*p == '\033') {
			ثابت نویسه *e = skip_ansi_escape(p, last + 1);

			p = e ? e : p + 1;
			ادامه;
		}
		اگر (iscntrl((unsigned نویسه) *p)) {
			p++;
			ادامه;
		}
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		اگر (len == 0)
			بشکن;
		اگر (len > 0 && iswprint(wc)) {
			صحیح x = wcwidth(wc);
			اگر (x > 0)
				width += x;
		} وگرنه اگر (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#وگرنه
		اگر (isprint((unsigned نویسه) *p))
			width++;
		p++;
#endif
	}

	بازگشت width;
}

size_t mbs_width(ثابت نویسه *s)
{
	اگر (!s || !*s)
		بازگشت 0;
	بازگشت mbs_nwidth(s, strlen(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
size_t mbs_safe_nwidth(ثابت نویسه *buf, size_t bufsz, size_t *sz)
{
	ثابت نویسه *p = buf, *last = buf;
	size_t width = 0, bytes = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, اندازه(st));
#endif
	اگر (p && *p && bufsz)
		last = p + (bufsz - 1);

	وقتی (p && *p && p <= last) {
		اگر ((p < last && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned نویسه) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		}
#ifdef HAVE_WIDECHAR
		وگرنه {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			اگر (len == 0)
				بشکن;

			اگر (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				اگر (isprint((unsigned نویسه) *p))
					width += 1, bytes += 1;
				وگرنه
					width += 4, bytes += 4;

			} وگرنه اگر (!iswprint(wc)) {
				width += len * 4;	/* hex encode whole sequence */
				bytes += len * 4;
			} وگرنه {
				width += wcwidth(wc);	/* number of cells */
				bytes += len;		/* number of bytes */
			}
			p += len;
		}
#وگرنه
		وگرنه اگر (!isprint((unsigned نویسه) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		} وگرنه {
			width++, bytes++;
			p++;
		}
#endif
	}

	اگر (sz)
		*sz = bytes;
	بازگشت width;
}

size_t mbs_safe_width(ثابت نویسه *s)
{
	اگر (!s || !*s)
		بازگشت 0;
	بازگشت mbs_safe_nwidth(s, strlen(s), پوچ);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
نویسه *mbs_safe_encode_to_buffer(ثابت نویسه *s, size_t *width, نویسه *buf, size_t bufsiz, ثابت نویسه *safechars)
{
	ثابت نویسه *p = s;
	نویسه *r;
	صحیح rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, اندازه(st));
#endif
	اگر (!sz || !buf || !bufsiz)
		بازگشت پوچ;

	r = buf;
	rsz = (صحیح) bufsiz;
	*width = 0;

	وقتی (p && *p) {
		اگر (safechars && strchr(safechars, *p)) {
			اگر (rsz < 2)
				بشکن;
			*r++ = *p++;
			rsz--;
			ادامه;
		}

		اگر ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned نویسه) *p)) {
			اگر (rsz < 5)
				بشکن;
			snprintf(r, rsz, "\\x%02x", (unsigned نویسه) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
			p++;
		}
#ifdef HAVE_WIDECHAR
		وگرنه {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			اگر (len == 0)
				بشکن;		/* end of string */

			اگر (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				اگر (!isprint((unsigned نویسه) *p)) {
					اگر (rsz < 5)
						بشکن;
					snprintf(r, rsz, "\\x%02x", (unsigned نویسه) *p);
					r += 4;
					rsz -= 4;
					*width += 4;
				} وگرنه {
					اگر (rsz < 2)
						بشکن;
					(*width)++;
					*r++ = *p;
					rsz--;
				}
			} وگرنه اگر (!iswprint(wc)) {
				size_t i;
				برای (i = 0; i < len; i++) {
					اگر (rsz < 5)
						بشکن;
					snprintf(r, rsz, "\\x%02x", (unsigned نویسه) p[i]);
					r += 4;
					rsz -= 4;
					*width += 4;
				}
				اگر (i < len)
					بشکن;
			} وگرنه {
				اگر (rsz < (صحیح)len + 1)
					بشکن;
				memcpy(r, p, len);
				r += len;
				rsz -= len;
				*width += wcwidth(wc);
			}
			p += len;
		}
#وگرنه
		وگرنه اگر (!isprint((unsigned نویسه) *p)) {
			اگر (rsz < 5)
				بشکن;
			snprintf(r, rsz, "\\x%02x", (unsigned نویسه) *p);
			p++;
			r += 4;
			rsz -= 4;
			*width += 4;
		} وگرنه {
			اگر (rsz < 2)
				بشکن;
			*r++ = *p++;
			rsz--;
			(*width)++;
		}
#endif
	}

	*r = '\0';
	بازگشت buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
نویسه *mbs_invalid_encode_to_buffer(ثابت نویسه *s, size_t *width, نویسه *buf, size_t bufsiz)
{
	ثابت نویسه *p = s;
	نویسه *r;
	صحیح rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, اندازه(st));
#endif
	اگر (!sz || !buf || !bufsiz)
		بازگشت پوچ;

	r = buf;
	rsz = (صحیح) bufsiz;
	*width = 0;

	وقتی (p && *p) {
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#وگرنه
		size_t len = 1;
#endif

		اگر (len == 0)
			بشکن;		/* end of string */

		اگر (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			اگر (!isprint((unsigned نویسه) *p)) {
				اگر (rsz < 5)
					بشکن;
				snprintf(r, rsz, "\\x%02x", (unsigned نویسه) *p);
				r += 4;
				rsz -= 4;
				*width += 4;
			} وگرنه {
				اگر (rsz < 2)
					بشکن;
				(*width)++;
				*r++ = *p;
				rsz--;
			}
		} وگرنه اگر (*p == '\\' && *(p + 1) == 'x') {
			اگر (rsz < 5)
				بشکن;
			snprintf(r, rsz, "\\x%02x", (unsigned نویسه) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
		} وگرنه {
			اگر (rsz < (صحیح)len + 1)
				بشکن;
			r = mempcpy(r, p, len);
			rsz -= len;
			*width += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	بازگشت buf;
}

/*
 * Guess size
 */
size_t mbs_safe_encode_size(size_t bytes)
{
	بازگشت (bytes * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_safe_decode_size(ثابت نویسه *p)
{
	size_t bytes = 0;

	وقتی (p && *p) {
		اگر (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		وگرنه
			p++;
		bytes++;
	}
	بازگشت bytes;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
نویسه *mbs_safe_encode(ثابت نویسه *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	نویسه *buf, *ret = پوچ;

	اگر (!sz)
		بازگشت پوچ;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	اگر (buf)
		ret = mbs_safe_encode_to_buffer(s, width, buf, bufsz, پوچ);
	اگر (!ret)
		آزاد(buf);
	بازگشت ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
نویسه *mbs_invalid_encode(ثابت نویسه *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	نویسه *buf, *ret = پوچ;

	اگر (!sz)
		بازگشت پوچ;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	اگر (buf)
		ret = mbs_invalid_encode_to_buffer(s, width, buf, bufsz);
	اگر (!ret)
		آزاد(buf);
	بازگشت ret;
}

#ifdef HAVE_WIDECHAR

ایستا bool
wc_ensure_printable (wchar_t *wchars)
{
  bool replaced = false;
  wchar_t *wc = wchars;
  وقتی (*wc)
    {
      اگر (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  بازگشت replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
ایستا wchar_t *
wc_skip_ansi_escape (wchar_t *wc)
{
  wchar_t *p = wc + 1;

  اگر (*wc != L'\033' || !*p)
    بازگشت پوچ;

  اگر (*p == L'[')
    {
      p++;
      وقتی (*p >= 0x30 && *p <= 0x3f)
        p++;
      وقتی (*p >= 0x20 && *p <= 0x2f)
        p++;
      اگر (*p >= 0x40 && *p <= 0x7e)
        بازگشت p + 1;
    }
  وگرنه اگر ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    بازگشت p + 2;

  بازگشت پوچ;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

ایستا size_t
wc_truncate (wchar_t *wc, size_t width)
{
  size_t cells = 0;
  صحیح next_cells = 0;

  وقتی (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      اگر (after_escape)
        {
          wc = after_escape;
          ادامه;
        }

      next_cells = wcwidth (*wc);
      اگر (next_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          next_cells = 1;
        }
      اگر (cells + next_cells > width)
        بشکن;

      cells += next_cells;
      wc++;
    }
  *wc = L'\0';
  بازگشت cells;
}

ایستا صحیح
rpl_wcswidth (ثابت wchar_t *s, size_t n)
{
  صحیح ret = 0;

  وقتی (n-- > 0 && *s != L'\0')
    {
      صحیح nwidth = wcwidth (*s++);
      اگر (nwidth == -1)             /* non printable */
        بازگشت -1;
      اگر (ret > (INT_MAX - nwidth)) /* overflow */
        بازگشت -1;
      ret += nwidth;
    }

  بازگشت ret;
}
#endif /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
size_t
mbs_truncate(نویسه *str, size_t *width)
{
	ssize_t bytes = strlen(str);
#ifdef HAVE_WIDECHAR
	ssize_t sz = mbstowcs(پوچ, str, 0);
	wchar_t *wcs = پوچ;

	اگر (sz == (ssize_t) -1)
		برو done;

	wcs = calloc(1, (sz + 1) * اندازه(wchar_t));
	اگر (!wcs)
		برو done;

	اگر (!mbstowcs(wcs, str, sz))
		برو done;
	*width = wc_truncate(wcs, *width);
	bytes = wcstombs(str, wcs, bytes);
done:
	آزاد(wcs);
#وگرنه
	{
		ثابت نویسه *p = str;
		ثابت نویسه *end = str + bytes;
		size_t cells = 0;

		وقتی (p < end) {
			ثابت نویسه *after_escape = skip_ansi_escape(p, end);

			اگر (after_escape) {
				p = after_escape;
				ادامه;
			}
			اگر (cells >= *width)
				بشکن;
			p++;
			cells++;
		}
		bytes = p - str;
		*width = cells;
	}
#endif
	اگر (bytes >= 0)
		str[bytes] = '\0';
	بازگشت bytes;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

ایستا نویسه*
mbs_align_pad (نویسه *dest, ثابت نویسه* dest_end, size_t n_spaces, صحیح padchar)
{
  برای (/* nothing */; n_spaces && (dest < dest_end); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  بازگشت dest;
}

size_t
mbsalign (ثابت نویسه *src, نویسه *dest, size_t dest_size,
          size_t *width, mbs_align_t align, صحیح flags)
{
	بازگشت mbsalign_with_padding(src, dest, dest_size, width, align, flags, ' ');
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
mbsalign_with_padding (ثابت نویسه *src, نویسه *dest, size_t dest_size,
	               size_t *width, mbs_align_t align,
#ifdef HAVE_WIDECHAR
		       صحیح flags,
#وگرنه
		       صحیح flags __attribute__((__unused__)),
#endif
		       صحیح padchar)
{
  size_t ret = -1;
  size_t src_size = strlen (src) + 1;
  نویسه *newstr = پوچ;
  wchar_t *str_wc = پوچ;
  ثابت نویسه *str_to_print = src;
  size_t n_cols = src_size - 1;
  size_t n_used_bytes = n_cols; /* Not including NUL */
  size_t n_spaces = 0, space_چپ;

#ifdef HAVE_WIDECHAR
  bool conversion = false;
  bool wc_enabled = false;

  /* In multi-byte locales convert to wide characters
     to allow easy truncation. Also determine number
     of screen columns used.  */
  اگر (MB_CUR_MAX > 1)
    {
      size_t src_chars = mbstowcs (پوچ, src, 0);
      اگر (src_chars == (size_t) -1)
        {
          اگر (flags & MBA_UNIBYTE_FALLBACK)
            برو mbsalign_unibyte;
          وگرنه
            برو mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = malloc (src_chars * اندازه (wchar_t));
      اگر (str_wc == پوچ)
        {
          اگر (flags & MBA_UNIBYTE_FALLBACK)
            برو mbsalign_unibyte;
          وگرنه
            برو mbsalign_cleanup;
        }
      اگر (mbstowcs (str_wc, src, src_chars) != 0)
        {
          str_wc[src_chars - 1] = L'\0';
          wc_enabled = true;
          conversion = wc_ensure_printable (str_wc);
          n_cols = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  اگر (wc_enabled && (conversion || (n_cols > *width)))
    {
        اگر (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_size = wcstombs(پوچ, str_wc, 0) + 1;
          }
        newstr = malloc (src_size);
        اگر (newstr == پوچ)
        {
          اگر (flags & MBA_UNIBYTE_FALLBACK)
            برو mbsalign_unibyte;
          وگرنه
            برو mbsalign_cleanup;
        }
        str_to_print = newstr;
        n_cols = wc_truncate (str_wc, *width);
        n_used_bytes = wcstombs (newstr, str_wc, src_size);
    }

mbsalign_unibyte:
#endif

  اگر (n_cols > *width) /* Unibyte truncation required.  */
    {
      n_cols = *width;
      n_used_bytes = n_cols;
    }

  اگر (*width > n_cols) /* Padding required.  */
    n_spaces = *width - n_cols;

  /* indicate to caller how many cells needed (not including padding).  */
  *width = n_cols;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_used_bytes + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  اگر (dest_size != 0)
    {
      نویسه *dest_end = dest + dest_size - 1;
      size_t start_spaces;
      size_t end_spaces;

      انتخاب (align)
        {
        حالت MBS_ALIGN_CENTER:
          start_spaces = n_spaces / 2 + n_spaces % 2;
          end_spaces = n_spaces / 2;
          بشکن;
        حالت MBS_ALIGN_چپ:
          start_spaces = 0;
          end_spaces = n_spaces;
          بشکن;
        حالت MBS_ALIGN_راست:
          start_spaces = n_spaces;
          end_spaces = 0;
          بشکن;
	پیشفرض:
	  abort();
        }

      dest = mbs_align_pad (dest, dest_end, start_spaces, padchar);
      space_چپ = dest_end - dest;
      dest = mempcpy (dest, str_to_print, min (n_used_bytes, space_چپ));
      mbs_align_pad (dest, dest_end, end_spaces, padchar);
    }
#ifdef HAVE_WIDECHAR
mbsalign_cleanup:
#endif
  آزاد (str_wc);
  آزاد (newstr);

  بازگشت ret;
}
