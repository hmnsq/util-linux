/*
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Very simple multibyte buffer editor. Allows to maintain the current
 * position in the string, add and remove chars on the current position.
 *
 * This file may be distributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2017 Karel Zak <kzak@redhat.com>
 */
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <stdio.h>

#அந்தர்பாவ "mbsalign.h"
#அந்தர்பாவ "mbsedit.h"

கட்டமைப்பு mbs_editor *mbs_new_edit(எழுத்து *buf, size_t bufsz, size_t ncells)
{
	கட்டமைப்பு mbs_editor *edit = calloc(1, அளவு(*edit));

	எனில் (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = strlen(buf);
	}
	திரும்பு edit;
}

எழுத்து *mbs_free_edit(கட்டமைப்பு mbs_editor *edit)
{
	எழுத்து *ret = edit ? edit->buf : வெற்று;

	விடுவி(edit);
	திரும்பு ret;
}

நிலையான size_t mbs_next(மாறிலி எழுத்து *str, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	எனில் (!str || !*str)
		திரும்பு 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, வெற்று);
	*ncells = wcwidth(wc);
	திரும்பு n;
#இல்லையெனில்
	எனில் (!str || !*str)
		திரும்பு 0;
	*ncells = 1;
	திரும்பு 1;
#endif
}

நிலையான size_t mbs_prev(மாறிலி எழுத்து *start, மாறிலி எழுத்து *end, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc = 0;
	மாறிலி எழுத்து *p, *prev;
	size_t n = 0;

	எனில் (!start || !end || start == end || !*start)
		திரும்பு 0;

	prev = p = start;
	போது (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, வெற்று);
		prev = p;

		எனில் (n == (size_t) -1 || n == (size_t) -2)
			p++;
		இல்லையெனில்
			p += n;
	}

	எனில் (prev == end)
		திரும்பு 0;
	*ncells = wcwidth(wc);
	திரும்பு n;
#இல்லையெனில்
	எனில் (!start || !end || start == end || !*start)
		திரும்பு 0;
	*ncells = 1;
	திரும்பு 1;
#endif
}

முழு mbs_edit_goto(கட்டமைப்பு mbs_editor *edit, முழு where)
{
	தேர்வு (where) {
	நிலை MBS_EDIT_இடது:
		எனில் (edit->cursor == 0)
			திரும்பு 1;
		இல்லையெனில் {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			எனில் (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		நிறுத்து;
	நிலை MBS_EDIT_வலது:
		எனில் (edit->cursor_cells >= edit->cur_cells)
			திரும்பு 1;
		இல்லையெனில் {
			size_t n, cells;
			n = mbs_next(edit->buf + edit->cursor, &cells);
			எனில் (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		நிறுத்து;
	நிலை MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		நிறுத்து;
	நிலை MBS_EDIT_END:
		edit->cursor = edit->cur_bytes;
		edit->cursor_cells = edit->cur_cells;
		நிறுத்து;
	இயல்பு:
		திரும்பு -EINVAL;
	}

	திரும்பு 0;
}

/* Remove next MB from @str, returns number of removed bytes */
நிலையான size_t remove_next(எழுத்து *str, size_t *ncells)
{
	/* all in bytes! */
	size_t bytes, move_bytes, n;

	n          = mbs_next(str, ncells);
	bytes      = strlen(str);
	move_bytes = bytes - n;

	memmove(str, str + n, move_bytes);
	str[bytes - n] = '\0';
	திரும்பு n;
}

நிலையான size_t mbs_செருகு(எழுத்து *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, bytes;
	எழுத்து *in;

#ifdef HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	in = malloc(MB_CUR_MAX);
	எனில் (!in)
		திரும்பு -1;

	n = wctomb(in, wc);
	எனில் (n == (size_t) -1)
		செல் out;
	*ncells = wcwidth(wc);
#இல்லையெனில்
	*ncells = 1;
	in = (எழுத்து *) &c;
#endif
	bytes       = strlen(str);

	memmove(str + n, str, bytes);
	memcpy(str, in, n);
	str[bytes + n] = '\0';
#ifdef HAVE_WIDECHAR
out:
	விடுவி(in);
#endif
	திரும்பு n;
}

நிலையான முழு mbs_edit_remove(கட்டமைப்பு mbs_editor *edit)
{
	size_t n, ncells;

	எனில் (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		திரும்பு 1;

	n = remove_next(edit->buf + edit->cursor, &ncells);
	எனில் (n == (size_t)-1)
		திரும்பு 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	திரும்பு 0;
}

முழு mbs_edit_delete(கட்டமைப்பு mbs_editor *edit)
{
	எனில் (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_இடது) == 1)
		திரும்பு 1;

	திரும்பு mbs_edit_remove(edit);
}

முழு mbs_edit_backspace(கட்டமைப்பு mbs_editor *edit)
{
	எனில் (mbs_edit_goto(edit, MBS_EDIT_இடது) == 0)
		திரும்பு mbs_edit_remove(edit);
	திரும்பு 1;
}

முழு mbs_edit_செருகு(கட்டமைப்பு mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	எனில் (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		திரும்பு 1;

	n = mbs_செருகு(edit->buf + edit->cursor, c, &ncells);
	எனில் (n == (size_t)-1)
		திரும்பு 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	திரும்பு 0;
}
