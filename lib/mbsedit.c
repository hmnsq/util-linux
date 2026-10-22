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
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <stdio.h>

#অন্তর্ভাব "mbsalign.h"
#অন্তর্ভাব "mbsedit.h"

কাঠামো mbs_editor *mbs_new_edit(অক্ষর *buf, size_t bufsz, size_t ncells)
{
	কাঠামো mbs_editor *edit = calloc(1, আকার(*edit));

	যদি (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = strlen(buf);
	}
	ফেরত edit;
}

অক্ষর *mbs_free_edit(কাঠামো mbs_editor *edit)
{
	অক্ষর *ret = edit ? edit->buf : ফাঁকা;

	মুক্ত(edit);
	ফেরত ret;
}

স্থির size_t mbs_পরবর্তী(ধ্রুবক অক্ষর *str, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	যদি (!str || !*str)
		ফেরত 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, ফাঁকা);
	*ncells = wcwidth(wc);
	ফেরত n;
#অন্যথা
	যদি (!str || !*str)
		ফেরত 0;
	*ncells = 1;
	ফেরত 1;
#endif
}

স্থির size_t mbs_পূর্ব(ধ্রুবক অক্ষর *start, ধ্রুবক অক্ষর *end, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc = 0;
	ধ্রুবক অক্ষর *p, *পূর্ব;
	size_t n = 0;

	যদি (!start || !end || start == end || !*start)
		ফেরত 0;

	পূর্ব = p = start;
	যখন (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, ফাঁকা);
		পূর্ব = p;

		যদি (n == (size_t) -1 || n == (size_t) -2)
			p++;
		অন্যথা
			p += n;
	}

	যদি (পূর্ব == end)
		ফেরত 0;
	*ncells = wcwidth(wc);
	ফেরত n;
#অন্যথা
	যদি (!start || !end || start == end || !*start)
		ফেরত 0;
	*ncells = 1;
	ফেরত 1;
#endif
}

পূর্ণসংখ্যা mbs_edit_goto(কাঠামো mbs_editor *edit, পূর্ণসংখ্যা where)
{
	নির্বাচন (where) {
	ক্ষেত্র MBS_EDIT_বাম:
		যদি (edit->cursor == 0)
			ফেরত 1;
		অন্যথা {
			size_t n, cells;
			n = mbs_পূর্ব(edit->buf, edit->buf + edit->cursor, &cells);
			যদি (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		বিরতি;
	ক্ষেত্র MBS_EDIT_ডান:
		যদি (edit->cursor_cells >= edit->cur_cells)
			ফেরত 1;
		অন্যথা {
			size_t n, cells;
			n = mbs_পরবর্তী(edit->buf + edit->cursor, &cells);
			যদি (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		বিরতি;
	ক্ষেত্র MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		বিরতি;
	ক্ষেত্র MBS_EDIT_END:
		edit->cursor = edit->cur_bytes;
		edit->cursor_cells = edit->cur_cells;
		বিরতি;
	পূর্বনির্ধারিত:
		ফেরত -EINVAL;
	}

	ফেরত 0;
}

/* Remove next MB from @str, returns number of removed bytes */
স্থির size_t remove_পরবর্তী(অক্ষর *str, size_t *ncells)
{
	/* all in bytes! */
	size_t bytes, move_bytes, n;

	n          = mbs_পরবর্তী(str, ncells);
	bytes      = strlen(str);
	move_bytes = bytes - n;

	memmove(str, str + n, move_bytes);
	str[bytes - n] = '\0';
	ফেরত n;
}

স্থির size_t mbs_ঢোকাও(অক্ষর *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, bytes;
	অক্ষর *in;

#ifdef HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	in = malloc(MB_CUR_MAX);
	যদি (!in)
		ফেরত -1;

	n = wctomb(in, wc);
	যদি (n == (size_t) -1)
		যাও out;
	*ncells = wcwidth(wc);
#অন্যথা
	*ncells = 1;
	in = (অক্ষর *) &c;
#endif
	bytes       = strlen(str);

	memmove(str + n, str, bytes);
	memcpy(str, in, n);
	str[bytes + n] = '\0';
#ifdef HAVE_WIDECHAR
out:
	মুক্ত(in);
#endif
	ফেরত n;
}

স্থির পূর্ণসংখ্যা mbs_edit_remove(কাঠামো mbs_editor *edit)
{
	size_t n, ncells;

	যদি (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		ফেরত 1;

	n = remove_পরবর্তী(edit->buf + edit->cursor, &ncells);
	যদি (n == (size_t)-1)
		ফেরত 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	ফেরত 0;
}

পূর্ণসংখ্যা mbs_edit_delete(কাঠামো mbs_editor *edit)
{
	যদি (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_বাম) == 1)
		ফেরত 1;

	ফেরত mbs_edit_remove(edit);
}

পূর্ণসংখ্যা mbs_edit_backspace(কাঠামো mbs_editor *edit)
{
	যদি (mbs_edit_goto(edit, MBS_EDIT_বাম) == 0)
		ফেরত mbs_edit_remove(edit);
	ফেরত 1;
}

পূর্ণসংখ্যা mbs_edit_ঢোকাও(কাঠামো mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	যদি (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		ফেরত 1;

	n = mbs_ঢোকাও(edit->buf + edit->cursor, c, &ncells);
	যদি (n == (size_t)-1)
		ফেরত 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	ফেরত 0;
}
