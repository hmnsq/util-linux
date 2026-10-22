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
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <errno.h>
#ضمن <stdio.h>

#ضمن "mbsalign.h"
#ضمن "mbsedit.h"

بنية mbs_editor *mbs_new_edit(حرف *buf, size_t bufsz, size_t ncells)
{
	بنية mbs_editor *edit = calloc(1, حجم(*edit));

	אם (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = strlen(buf);
	}
	החזר edit;
}

حرف *mbs_free_edit(بنية mbs_editor *edit)
{
	حرف *ret = edit ? edit->buf : عدم;

	حرر(edit);
	החזר ret;
}

ساكن size_t mbs_next(ثابت حرف *str, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	אם (!str || !*str)
		החזר 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, عدم);
	*ncells = wcwidth(wc);
	החזר n;
#אחרת
	אם (!str || !*str)
		החזר 0;
	*ncells = 1;
	החזר 1;
#endif
}

ساكن size_t mbs_prev(ثابت حرف *start, ثابت حرف *end, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc = 0;
	ثابت حرف *p, *prev;
	size_t n = 0;

	אם (!start || !end || start == end || !*start)
		החזר 0;

	prev = p = start;
	כלעוד (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, عدم);
		prev = p;

		אם (n == (size_t) -1 || n == (size_t) -2)
			p++;
		אחרת
			p += n;
	}

	אם (prev == end)
		החזר 0;
	*ncells = wcwidth(wc);
	החזר n;
#אחרת
	אם (!start || !end || start == end || !*start)
		החזר 0;
	*ncells = 1;
	החזר 1;
#endif
}

صحيح mbs_edit_goto(بنية mbs_editor *edit, صحيح where)
{
	بدل (where) {
	حالة MBS_EDIT_LEFT:
		אם (edit->cursor == 0)
			החזר 1;
		אחרת {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			אם (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		اكسر;
	حالة MBS_EDIT_RIGHT:
		אם (edit->cursor_cells >= edit->cur_cells)
			החזר 1;
		אחרת {
			size_t n, cells;
			n = mbs_next(edit->buf + edit->cursor, &cells);
			אם (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		اكسر;
	حالة MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		اكسر;
	حالة MBS_EDIT_END:
		edit->cursor = edit->cur_bytes;
		edit->cursor_cells = edit->cur_cells;
		اكسر;
	افتراضي:
		החזר -EINVAL;
	}

	החזר 0;
}

/* Remove next MB from @str, returns number of removed bytes */
ساكن size_t remove_next(حرف *str, size_t *ncells)
{
	/* all in bytes! */
	size_t bytes, move_bytes, n;

	n          = mbs_next(str, ncells);
	bytes      = strlen(str);
	move_bytes = bytes - n;

	memmove(str, str + n, move_bytes);
	str[bytes - n] = '\0';
	החזר n;
}

ساكن size_t mbs_insert(حرف *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, bytes;
	حرف *in;

#ifdef HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	in = malloc(MB_CUR_MAX);
	אם (!in)
		החזר -1;

	n = wctomb(in, wc);
	אם (n == (size_t) -1)
		اذهب out;
	*ncells = wcwidth(wc);
#אחרת
	*ncells = 1;
	in = (حرف *) &c;
#endif
	bytes       = strlen(str);

	memmove(str + n, str, bytes);
	memcpy(str, in, n);
	str[bytes + n] = '\0';
#ifdef HAVE_WIDECHAR
out:
	حرر(in);
#endif
	החזר n;
}

ساكن صحيح mbs_edit_remove(بنية mbs_editor *edit)
{
	size_t n, ncells;

	אם (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		החזר 1;

	n = remove_next(edit->buf + edit->cursor, &ncells);
	אם (n == (size_t)-1)
		החזר 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	החזר 0;
}

صحيح mbs_edit_delete(بنية mbs_editor *edit)
{
	אם (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_LEFT) == 1)
		החזר 1;

	החזר mbs_edit_remove(edit);
}

صحيح mbs_edit_backspace(بنية mbs_editor *edit)
{
	אם (mbs_edit_goto(edit, MBS_EDIT_LEFT) == 0)
		החזר mbs_edit_remove(edit);
	החזר 1;
}

صحيح mbs_edit_insert(بنية mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	אם (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		החזר 1;

	n = mbs_insert(edit->buf + edit->cursor, c, &ncells);
	אם (n == (size_t)-1)
		החזר 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	החזר 0;
}
