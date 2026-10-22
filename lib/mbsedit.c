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

	ܐܢ (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = strlen(buf);
	}
	ܦܢܐ edit;
}

حرف *mbs_free_edit(بنية mbs_editor *edit)
{
	حرف *ret = edit ? edit->buf : عدم;

	حرر(edit);
	ܦܢܐ ret;
}

ساكن size_t mbs_next(ثابت حرف *str, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	ܐܢ (!str || !*str)
		ܦܢܐ 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, عدم);
	*ncells = wcwidth(wc);
	ܦܢܐ n;
#ܐܚܪܢܐ
	ܐܢ (!str || !*str)
		ܦܢܐ 0;
	*ncells = 1;
	ܦܢܐ 1;
#endif
}

ساكن size_t mbs_prev(ثابت حرف *start, ثابت حرف *end, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc = 0;
	ثابت حرف *p, *prev;
	size_t n = 0;

	ܐܢ (!start || !end || start == end || !*start)
		ܦܢܐ 0;

	prev = p = start;
	ܟܡܐ (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, عدم);
		prev = p;

		ܐܢ (n == (size_t) -1 || n == (size_t) -2)
			p++;
		ܐܚܪܢܐ
			p += n;
	}

	ܐܢ (prev == end)
		ܦܢܐ 0;
	*ncells = wcwidth(wc);
	ܦܢܐ n;
#ܐܚܪܢܐ
	ܐܢ (!start || !end || start == end || !*start)
		ܦܢܐ 0;
	*ncells = 1;
	ܦܢܐ 1;
#endif
}

صحيح mbs_edit_goto(بنية mbs_editor *edit, صحيح where)
{
	بدل (where) {
	حالة MBS_EDIT_LEFT:
		ܐܢ (edit->cursor == 0)
			ܦܢܐ 1;
		ܐܚܪܢܐ {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			ܐܢ (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		اكسر;
	حالة MBS_EDIT_RIGHT:
		ܐܢ (edit->cursor_cells >= edit->cur_cells)
			ܦܢܐ 1;
		ܐܚܪܢܐ {
			size_t n, cells;
			n = mbs_next(edit->buf + edit->cursor, &cells);
			ܐܢ (n) {
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
		ܦܢܐ -EINVAL;
	}

	ܦܢܐ 0;
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
	ܦܢܐ n;
}

ساكن size_t mbs_insert(حرف *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, bytes;
	حرف *in;

#ifdef HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	in = malloc(MB_CUR_MAX);
	ܐܢ (!in)
		ܦܢܐ -1;

	n = wctomb(in, wc);
	ܐܢ (n == (size_t) -1)
		اذهب out;
	*ncells = wcwidth(wc);
#ܐܚܪܢܐ
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
	ܦܢܐ n;
}

ساكن صحيح mbs_edit_remove(بنية mbs_editor *edit)
{
	size_t n, ncells;

	ܐܢ (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		ܦܢܐ 1;

	n = remove_next(edit->buf + edit->cursor, &ncells);
	ܐܢ (n == (size_t)-1)
		ܦܢܐ 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	ܦܢܐ 0;
}

صحيح mbs_edit_delete(بنية mbs_editor *edit)
{
	ܐܢ (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_LEFT) == 1)
		ܦܢܐ 1;

	ܦܢܐ mbs_edit_remove(edit);
}

صحيح mbs_edit_backspace(بنية mbs_editor *edit)
{
	ܐܢ (mbs_edit_goto(edit, MBS_EDIT_LEFT) == 0)
		ܦܢܐ mbs_edit_remove(edit);
	ܦܢܐ 1;
}

صحيح mbs_edit_insert(بنية mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	ܐܢ (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		ܦܢܐ 1;

	n = mbs_insert(edit->buf + edit->cursor, c, &ncells);
	ܐܢ (n == (size_t)-1)
		ܦܢܐ 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	ܦܢܐ 0;
}
