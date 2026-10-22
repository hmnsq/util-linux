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
	بنية mbs_editor *edit = حافظه_صفر_بده(1, حجم(*edit));

	إذا (edit) {
		edit->buf = buf;
		edit->max_بایتها = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_امن_عرض(buf);
		edit->cur_بایتها = طول_رشته(buf);
	}
	أرجع edit;
}

حرف *mbs_free_edit(بنية mbs_editor *edit)
{
	حرف *ret = edit ? edit->buf : عدم;

	حرر(edit);
	أرجع ret;
}

ساكن size_t mbs_بعدی(ثابت حرف *str, size_t *ncells)
{
#اگر_تعریف_شده HAVE_WIDECHAR
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
#پایان_اگر
}

ساكن size_t mbs_prev(ثابت حرف *شروع, ثابت حرف *انتها, size_t *ncells)
{
#اگر_تعریف_شده HAVE_WIDECHAR
	wchar_t wc = 0;
	ثابت حرف *p, *prev;
	size_t n = 0;

	إذا (!شروع || !انتها || شروع == انتها || !*شروع)
		أرجع 0;

	prev = p = شروع;
	طالما (p < انتها) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, عدم);
		prev = p;

		إذا (n == (size_t) -1 || n == (size_t) -2)
			p++;
		وإلا
			p += n;
	}

	إذا (prev == انتها)
		أرجع 0;
	*ncells = wcwidth(wc);
	أرجع n;
#وإلا
	إذا (!شروع || !انتها || شروع == انتها || !*شروع)
		أرجع 0;
	*ncells = 1;
	أرجع 1;
#پایان_اگر
}

صحيح mbs_edit_goto(بنية mbs_editor *edit, صحيح where)
{
	بدل (where) {
	حالة MBS_EDIT_چپ:
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
	حالة MBS_EDIT_راست:
		إذا (edit->cursor_cells >= edit->cur_cells)
			أرجع 1;
		وإلا {
			size_t n, cells;
			n = mbs_بعدی(edit->buf + edit->cursor, &cells);
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
	حالة MBS_EDIT_انتها:
		edit->cursor = edit->cur_بایتها;
		edit->cursor_cells = edit->cur_cells;
		اكسر;
	افتراضي:
		أرجع -EINVAL;
	}

	أرجع 0;
}

/* Remove next MB from @str, returns number of removed bytes */
ساكن size_t remove_بعدی(حرف *str, size_t *ncells)
{
	/* all in bytes! */
	size_t بایتها, move_بایتها, n;

	n          = mbs_بعدی(str, ncells);
	بایتها      = طول_رشته(str);
	move_بایتها = بایتها - n;

	memmove(str, str + n, move_بایتها);
	str[بایتها - n] = '\0';
	أرجع n;
}

ساكن size_t mbs_درج(حرف *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, بایتها;
	حرف *درون;

#اگر_تعریف_شده HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	درون = حافظه_بده(MB_CUR_MAX);
	إذا (!درون)
		أرجع -1;

	n = wctomb(درون, wc);
	إذا (n == (size_t) -1)
		اذهب بیرون;
	*ncells = wcwidth(wc);
#وإلا
	*ncells = 1;
	درون = (حرف *) &c;
#پایان_اگر
	بایتها       = طول_رشته(str);

	memmove(str + n, str, بایتها);
	کپی_حافظه(str, درون, n);
	str[بایتها + n] = '\0';
#اگر_تعریف_شده HAVE_WIDECHAR
بیرون:
	حرر(درون);
#پایان_اگر
	أرجع n;
}

ساكن صحيح mbs_edit_remove(بنية mbs_editor *edit)
{
	size_t n, ncells;

	إذا (edit->cur_cells == 0 || edit->cursor >= edit->cur_بایتها)
		أرجع 1;

	n = remove_بعدی(edit->buf + edit->cursor, &ncells);
	إذا (n == (size_t)-1)
		أرجع 1;

	edit->cur_بایتها -= n;
	edit->cur_cells = mbs_امن_عرض(edit->buf);
	أرجع 0;
}

صحيح mbs_edit_delete(بنية mbs_editor *edit)
{
	إذا (edit->cursor >= edit->cur_بایتها
	    && mbs_edit_goto(edit, MBS_EDIT_چپ) == 1)
		أرجع 1;

	أرجع mbs_edit_remove(edit);
}

صحيح mbs_edit_backspace(بنية mbs_editor *edit)
{
	إذا (mbs_edit_goto(edit, MBS_EDIT_چپ) == 0)
		أرجع mbs_edit_remove(edit);
	أرجع 1;
}

صحيح mbs_edit_درج(بنية mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	إذا (edit->cur_بایتها + MB_CUR_MAX > edit->max_بایتها)
		أرجع 1;

	n = mbs_درج(edit->buf + edit->cursor, c, &ncells);
	إذا (n == (size_t)-1)
		أرجع 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_بایتها += n;
	edit->cur_cells = mbs_امن_عرض(edit->buf);
	أرجع 0;
}
