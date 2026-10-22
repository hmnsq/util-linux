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
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <stdio.h>

#অন্তর্ভাৱ "mbsalign.h"
#অন্তর্ভাৱ "mbsedit.h"

সংরচনা mbs_editor *mbs_new_edit(ৱর্ণ *buf, size_t bufsz, size_t ncells)
{
	সংরচনা mbs_editor *edit = calloc(1, পরিমাণ(*edit));

	যদি (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = strlen(buf);
	}
	ঘূৰাই edit;
}

ৱর্ণ *mbs_free_edit(সংরচনা mbs_editor *edit)
{
	ৱর্ণ *ret = edit ? edit->buf : রিক্ত;

	মুক্ত(edit);
	ঘূৰাই ret;
}

স্থির size_t mbs_next(ধ্রুৱ ৱর্ণ *str, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	যদি (!str || !*str)
		ঘূৰাই 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, রিক্ত);
	*ncells = wcwidth(wc);
	ঘূৰাই n;
#অন্যথা
	যদি (!str || !*str)
		ঘূৰাই 0;
	*ncells = 1;
	ঘূৰাই 1;
#endif
}

স্থির size_t mbs_prev(ধ্রুৱ ৱর্ণ *start, ধ্রুৱ ৱর্ণ *end, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc = 0;
	ধ্রুৱ ৱর্ণ *p, *prev;
	size_t n = 0;

	যদি (!start || !end || start == end || !*start)
		ঘূৰাই 0;

	prev = p = start;
	যেতিয়ালৈ (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, রিক্ত);
		prev = p;

		যদি (n == (size_t) -1 || n == (size_t) -2)
			p++;
		অন্যথা
			p += n;
	}

	যদি (prev == end)
		ঘূৰাই 0;
	*ncells = wcwidth(wc);
	ঘূৰাই n;
#অন্যথা
	যদি (!start || !end || start == end || !*start)
		ঘূৰাই 0;
	*ncells = 1;
	ঘূৰাই 1;
#endif
}

পূর্ণাঙ্ক mbs_edit_goto(সংরচনা mbs_editor *edit, পূর্ণাঙ্ক where)
{
	ৱিকল্প (where) {
	পক্ষ MBS_EDIT_বাম:
		যদি (edit->cursor == 0)
			ঘূৰাই 1;
		অন্যথা {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			যদি (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		ভঙ্গ;
	পক্ষ MBS_EDIT_ডান:
		যদি (edit->cursor_cells >= edit->cur_cells)
			ঘূৰাই 1;
		অন্যথা {
			size_t n, cells;
			n = mbs_next(edit->buf + edit->cursor, &cells);
			যদি (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		ভঙ্গ;
	পক্ষ MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		ভঙ্গ;
	পক্ষ MBS_EDIT_END:
		edit->cursor = edit->cur_bytes;
		edit->cursor_cells = edit->cur_cells;
		ভঙ্গ;
	স্ৱভাৱ:
		ঘূৰাই -EINVAL;
	}

	ঘূৰাই 0;
}

/* Remove next MB from @str, returns number of removed bytes */
স্থির size_t remove_next(ৱর্ণ *str, size_t *ncells)
{
	/* all in bytes! */
	size_t bytes, move_bytes, n;

	n          = mbs_next(str, ncells);
	bytes      = strlen(str);
	move_bytes = bytes - n;

	memmove(str, str + n, move_bytes);
	str[bytes - n] = '\0';
	ঘূৰাই n;
}

স্থির size_t mbs_ঢোকাও(ৱর্ণ *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, bytes;
	ৱর্ণ *in;

#ifdef HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	in = malloc(MB_CUR_MAX);
	যদি (!in)
		ঘূৰাই -1;

	n = wctomb(in, wc);
	যদি (n == (size_t) -1)
		গচ্ছ out;
	*ncells = wcwidth(wc);
#অন্যথা
	*ncells = 1;
	in = (ৱর্ণ *) &c;
#endif
	bytes       = strlen(str);

	memmove(str + n, str, bytes);
	memcpy(str, in, n);
	str[bytes + n] = '\0';
#ifdef HAVE_WIDECHAR
out:
	মুক্ত(in);
#endif
	ঘূৰাই n;
}

স্থির পূর্ণাঙ্ক mbs_edit_remove(সংরচনা mbs_editor *edit)
{
	size_t n, ncells;

	যদি (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		ঘূৰাই 1;

	n = remove_next(edit->buf + edit->cursor, &ncells);
	যদি (n == (size_t)-1)
		ঘূৰাই 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	ঘূৰাই 0;
}

পূর্ণাঙ্ক mbs_edit_delete(সংরচনা mbs_editor *edit)
{
	যদি (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_বাম) == 1)
		ঘূৰাই 1;

	ঘূৰাই mbs_edit_remove(edit);
}

পূর্ণাঙ্ক mbs_edit_backspace(সংরচনা mbs_editor *edit)
{
	যদি (mbs_edit_goto(edit, MBS_EDIT_বাম) == 0)
		ঘূৰাই mbs_edit_remove(edit);
	ঘূৰাই 1;
}

পূর্ণাঙ্ক mbs_edit_ঢোকাও(সংরচনা mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	যদি (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		ঘূৰাই 1;

	n = mbs_ঢোকাও(edit->buf + edit->cursor, c, &ncells);
	যদি (n == (size_t)-1)
		ঘূৰাই 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	ঘূৰাই 0;
}
