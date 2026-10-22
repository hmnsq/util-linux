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
	بنية mbs_editor *edit = حافظه_صفر_ورکړه(1, حجم(*edit));

	إذا (edit) {
		edit->buf = buf;
		edit->max_بایټونه = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_خوندي_پلنوالی(buf);
		edit->cur_بایټونه = مزي_اوږدوالی(buf);
	}
	أرجع edit;
}

حرف *mbs_free_edit(بنية mbs_editor *edit)
{
	حرف *ret = edit ? edit->buf : عدم;

	حرر(edit);
	أرجع ret;
}

ساكن size_t mbs_بل(ثابت حرف *str, size_t *ncells)
{
#که_تعریف_شوی HAVE_WIDECHAR
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
#د_که_پای
}

ساكن size_t mbs_prev(ثابت حرف *پیل, ثابت حرف *پای_ټکی, size_t *ncells)
{
#که_تعریف_شوی HAVE_WIDECHAR
	wchar_t wc = 0;
	ثابت حرف *p, *prev;
	size_t n = 0;

	إذا (!پیل || !پای_ټکی || پیل == پای_ټکی || !*پیل)
		أرجع 0;

	prev = p = پیل;
	طالما (p < پای_ټکی) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, عدم);
		prev = p;

		إذا (n == (size_t) -1 || n == (size_t) -2)
			p++;
		وإلا
			p += n;
	}

	إذا (prev == پای_ټکی)
		أرجع 0;
	*ncells = wcwidth(wc);
	أرجع n;
#وإلا
	إذا (!پیل || !پای_ټکی || پیل == پای_ټکی || !*پیل)
		أرجع 0;
	*ncells = 1;
	أرجع 1;
#د_که_پای
}

صحيح mbs_edit_goto(بنية mbs_editor *edit, صحيح where)
{
	بدل (where) {
	حالة MBS_EDIT_کیڼ:
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
	حالة MBS_EDIT_ښی:
		إذا (edit->cursor_cells >= edit->cur_cells)
			أرجع 1;
		وإلا {
			size_t n, cells;
			n = mbs_بل(edit->buf + edit->cursor, &cells);
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
	حالة MBS_EDIT_پای_ټکی:
		edit->cursor = edit->cur_بایټونه;
		edit->cursor_cells = edit->cur_cells;
		اكسر;
	افتراضي:
		أرجع -EINVAL;
	}

	أرجع 0;
}

/* Remove next MB from @str, returns number of removed bytes */
ساكن size_t remove_بل(حرف *str, size_t *ncells)
{
	/* all in bytes! */
	size_t بایټونه, move_بایټونه, n;

	n          = mbs_بل(str, ncells);
	بایټونه      = مزي_اوږدوالی(str);
	move_بایټونه = بایټونه - n;

	memmove(str, str + n, move_بایټونه);
	str[بایټونه - n] = '\0';
	أرجع n;
}

ساكن size_t mbs_ورننباسه(حرف *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, بایټونه;
	حرف *دننه;

#که_تعریف_شوی HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	دننه = حافظه_ورکړه(MB_CUR_MAX);
	إذا (!دننه)
		أرجع -1;

	n = wctomb(دننه, wc);
	إذا (n == (size_t) -1)
		اذهب بهر;
	*ncells = wcwidth(wc);
#وإلا
	*ncells = 1;
	دننه = (حرف *) &c;
#د_که_پای
	بایټونه       = مزي_اوږدوالی(str);

	memmove(str + n, str, بایټونه);
	حافظه_نقل(str, دننه, n);
	str[بایټونه + n] = '\0';
#که_تعریف_شوی HAVE_WIDECHAR
بهر:
	حرر(دننه);
#د_که_پای
	أرجع n;
}

ساكن صحيح mbs_edit_remove(بنية mbs_editor *edit)
{
	size_t n, ncells;

	إذا (edit->cur_cells == 0 || edit->cursor >= edit->cur_بایټونه)
		أرجع 1;

	n = remove_بل(edit->buf + edit->cursor, &ncells);
	إذا (n == (size_t)-1)
		أرجع 1;

	edit->cur_بایټونه -= n;
	edit->cur_cells = mbs_خوندي_پلنوالی(edit->buf);
	أرجع 0;
}

صحيح mbs_edit_delete(بنية mbs_editor *edit)
{
	إذا (edit->cursor >= edit->cur_بایټونه
	    && mbs_edit_goto(edit, MBS_EDIT_کیڼ) == 1)
		أرجع 1;

	أرجع mbs_edit_remove(edit);
}

صحيح mbs_edit_backspace(بنية mbs_editor *edit)
{
	إذا (mbs_edit_goto(edit, MBS_EDIT_کیڼ) == 0)
		أرجع mbs_edit_remove(edit);
	أرجع 1;
}

صحيح mbs_edit_ورننباسه(بنية mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	إذا (edit->cur_بایټونه + MB_CUR_MAX > edit->max_بایټونه)
		أرجع 1;

	n = mbs_ورننباسه(edit->buf + edit->cursor, c, &ncells);
	إذا (n == (size_t)-1)
		أرجع 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_بایټونه += n;
	edit->cur_cells = mbs_خوندي_پلنوالی(edit->buf);
	أرجع 0;
}
