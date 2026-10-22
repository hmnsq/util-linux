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
#شامل <stdlib.h>
#شامل <string.h>
#شامل <errno.h>
#شامل <stdio.h>

#شامل "mbsalign.h"
#شامل "mbsedit.h"

ساختار mbs_editor *mbs_new_edit(نویسه *buf, size_t bufsz, size_t ncells)
{
	ساختار mbs_editor *edit = calloc(1, اندازه(*edit));

	اگر (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = strlen(buf);
	}
	بازگشت edit;
}

نویسه *mbs_free_edit(ساختار mbs_editor *edit)
{
	نویسه *ret = edit ? edit->buf : پوچ;

	آزاد(edit);
	بازگشت ret;
}

ایستا size_t mbs_next(ثابت نویسه *str, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	اگر (!str || !*str)
		بازگشت 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, پوچ);
	*ncells = wcwidth(wc);
	بازگشت n;
#وگرنه
	اگر (!str || !*str)
		بازگشت 0;
	*ncells = 1;
	بازگشت 1;
#endif
}

ایستا size_t mbs_prev(ثابت نویسه *start, ثابت نویسه *end, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc = 0;
	ثابت نویسه *p, *prev;
	size_t n = 0;

	اگر (!start || !end || start == end || !*start)
		بازگشت 0;

	prev = p = start;
	وقتی (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, پوچ);
		prev = p;

		اگر (n == (size_t) -1 || n == (size_t) -2)
			p++;
		وگرنه
			p += n;
	}

	اگر (prev == end)
		بازگشت 0;
	*ncells = wcwidth(wc);
	بازگشت n;
#وگرنه
	اگر (!start || !end || start == end || !*start)
		بازگشت 0;
	*ncells = 1;
	بازگشت 1;
#endif
}

صحیح mbs_edit_goto(ساختار mbs_editor *edit, صحیح where)
{
	انتخاب (where) {
	حالت MBS_EDIT_چپ:
		اگر (edit->cursor == 0)
			بازگشت 1;
		وگرنه {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			اگر (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		بشکن;
	حالت MBS_EDIT_راست:
		اگر (edit->cursor_cells >= edit->cur_cells)
			بازگشت 1;
		وگرنه {
			size_t n, cells;
			n = mbs_next(edit->buf + edit->cursor, &cells);
			اگر (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		بشکن;
	حالت MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		بشکن;
	حالت MBS_EDIT_END:
		edit->cursor = edit->cur_bytes;
		edit->cursor_cells = edit->cur_cells;
		بشکن;
	پیشفرض:
		بازگشت -EINVAL;
	}

	بازگشت 0;
}

/* Remove next MB from @str, returns number of removed bytes */
ایستا size_t remove_next(نویسه *str, size_t *ncells)
{
	/* all in bytes! */
	size_t bytes, move_bytes, n;

	n          = mbs_next(str, ncells);
	bytes      = strlen(str);
	move_bytes = bytes - n;

	memmove(str, str + n, move_bytes);
	str[bytes - n] = '\0';
	بازگشت n;
}

ایستا size_t mbs_درج(نویسه *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, bytes;
	نویسه *in;

#ifdef HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	in = malloc(MB_CUR_MAX);
	اگر (!in)
		بازگشت -1;

	n = wctomb(in, wc);
	اگر (n == (size_t) -1)
		برو out;
	*ncells = wcwidth(wc);
#وگرنه
	*ncells = 1;
	in = (نویسه *) &c;
#endif
	bytes       = strlen(str);

	memmove(str + n, str, bytes);
	memcpy(str, in, n);
	str[bytes + n] = '\0';
#ifdef HAVE_WIDECHAR
out:
	آزاد(in);
#endif
	بازگشت n;
}

ایستا صحیح mbs_edit_remove(ساختار mbs_editor *edit)
{
	size_t n, ncells;

	اگر (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		بازگشت 1;

	n = remove_next(edit->buf + edit->cursor, &ncells);
	اگر (n == (size_t)-1)
		بازگشت 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	بازگشت 0;
}

صحیح mbs_edit_delete(ساختار mbs_editor *edit)
{
	اگر (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_چپ) == 1)
		بازگشت 1;

	بازگشت mbs_edit_remove(edit);
}

صحیح mbs_edit_backspace(ساختار mbs_editor *edit)
{
	اگر (mbs_edit_goto(edit, MBS_EDIT_چپ) == 0)
		بازگشت mbs_edit_remove(edit);
	بازگشت 1;
}

صحیح mbs_edit_درج(ساختار mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	اگر (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		بازگشت 1;

	n = mbs_درج(edit->buf + edit->cursor, c, &ncells);
	اگر (n == (size_t)-1)
		بازگشت 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	بازگشت 0;
}
