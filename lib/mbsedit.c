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

	إذا (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = strlen(buf);
	}
	أرجع edit;
}

حرف *mbs_free_edit(بنية mbs_editor *edit)
{
	حرف *ret = edit ? edit->buf : عدم;

	حرر(edit);
	أرجع ret;
}

ساكن size_t mbs_next(ثابت حرف *str, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	إذا (!str || !*str)
		أرجع 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, عدم);
	*ncells = wcwidth(wc);
	أرجع n;
#وإلا
	إذا (!str || !*str)
		أرجع 0;
	*ncells = 1;
	أرجع 1;
#endif
}

ساكن size_t mbs_prev(ثابت حرف *start, ثابت حرف *end, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc = 0;
	ثابت حرف *p, *prev;
	size_t n = 0;

	إذا (!start || !end || start == end || !*start)
		أرجع 0;

	prev = p = start;
	طالما (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, عدم);
		prev = p;

		إذا (n == (size_t) -1 || n == (size_t) -2)
			p++;
		وإلا
			p += n;
	}

	إذا (prev == end)
		أرجع 0;
	*ncells = wcwidth(wc);
	أرجع n;
#وإلا
	إذا (!start || !end || start == end || !*start)
		أرجع 0;
	*ncells = 1;
	أرجع 1;
#endif
}

صحيح mbs_edit_goto(بنية mbs_editor *edit, صحيح where)
{
	بدل (where) {
	حالة MBS_EDIT_سول:
		إذا (edit->cursor == 0)
			أرجع 1;
		وإلا {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			إذا (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		اكسر;
	حالة MBS_EDIT_ئوڭ:
		إذا (edit->cursor_cells >= edit->cur_cells)
			أرجع 1;
		وإلا {
			size_t n, cells;
			n = mbs_next(edit->buf + edit->cursor, &cells);
			إذا (n) {
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
		أرجع -EINVAL;
	}

	أرجع 0;
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
	أرجع n;
}

ساكن size_t mbs_قىستۇرۇش(حرف *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, bytes;
	حرف *in;

#ifdef HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	in = malloc(MB_CUR_MAX);
	إذا (!in)
		أرجع -1;

	n = wctomb(in, wc);
	إذا (n == (size_t) -1)
		اذهب out;
	*ncells = wcwidth(wc);
#وإلا
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
	أرجع n;
}

ساكن صحيح mbs_edit_remove(بنية mbs_editor *edit)
{
	size_t n, ncells;

	إذا (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		أرجع 1;

	n = remove_next(edit->buf + edit->cursor, &ncells);
	إذا (n == (size_t)-1)
		أرجع 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	أرجع 0;
}

صحيح mbs_edit_delete(بنية mbs_editor *edit)
{
	إذا (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_سول) == 1)
		أرجع 1;

	أرجع mbs_edit_remove(edit);
}

صحيح mbs_edit_backspace(بنية mbs_editor *edit)
{
	إذا (mbs_edit_goto(edit, MBS_EDIT_سول) == 0)
		أرجع mbs_edit_remove(edit);
	أرجع 1;
}

صحيح mbs_edit_قىستۇرۇش(بنية mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	إذا (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		أرجع 1;

	n = mbs_قىستۇرۇش(edit->buf + edit->cursor, c, &ncells);
	إذا (n == (size_t)-1)
		أرجع 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	أرجع 0;
}
