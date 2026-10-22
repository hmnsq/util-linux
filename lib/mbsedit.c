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
	بنية mbs_editor *edit = صفر_حافظہ_دیو(1, حجم(*edit));

	إذا (edit) {
		edit->buf = buf;
		edit->max_بائٹاں = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_محفوظ_چوڑائی(buf);
		edit->cur_بائٹاں = لڑی_لمبائی(buf);
	}
	أرجع edit;
}

حرف *mbs_free_edit(بنية mbs_editor *edit)
{
	حرف *ret = edit ? edit->buf : عدم;

	حرر(edit);
	أرجع ret;
}

ساكن size_t mbs_اگلا(ثابت حرف *str, size_t *ncells)
{
#جے_تعریف_شدہ HAVE_WIDECHAR
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
#جے_انت
}

ساكن size_t mbs_prev(ثابت حرف *شروع, ثابت حرف *سرا, size_t *ncells)
{
#جے_تعریف_شدہ HAVE_WIDECHAR
	wchar_t wc = 0;
	ثابت حرف *p, *prev;
	size_t n = 0;

	إذا (!شروع || !سرا || شروع == سرا || !*شروع)
		أرجع 0;

	prev = p = شروع;
	طالما (p < سرا) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, عدم);
		prev = p;

		إذا (n == (size_t) -1 || n == (size_t) -2)
			p++;
		وإلا
			p += n;
	}

	إذا (prev == سرا)
		أرجع 0;
	*ncells = wcwidth(wc);
	أرجع n;
#وإلا
	إذا (!شروع || !سرا || شروع == سرا || !*شروع)
		أرجع 0;
	*ncells = 1;
	أرجع 1;
#جے_انت
}

صحيح mbs_edit_goto(بنية mbs_editor *edit, صحيح where)
{
	بدل (where) {
	حالة MBS_EDIT_بایاں:
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
	حالة MBS_EDIT_دایاں:
		إذا (edit->cursor_cells >= edit->cur_cells)
			أرجع 1;
		وإلا {
			size_t n, cells;
			n = mbs_اگلا(edit->buf + edit->cursor, &cells);
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
	حالة MBS_EDIT_سرا:
		edit->cursor = edit->cur_بائٹاں;
		edit->cursor_cells = edit->cur_cells;
		اكسر;
	افتراضي:
		أرجع -EINVAL;
	}

	أرجع 0;
}

/* Remove next MB from @str, returns number of removed bytes */
ساكن size_t remove_اگلا(حرف *str, size_t *ncells)
{
	/* all in bytes! */
	size_t بائٹاں, move_بائٹاں, n;

	n          = mbs_اگلا(str, ncells);
	بائٹاں      = لڑی_لمبائی(str);
	move_بائٹاں = بائٹاں - n;

	memmove(str, str + n, move_بائٹاں);
	str[بائٹاں - n] = '\0';
	أرجع n;
}

ساكن size_t mbs_داخل_کرو(حرف *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, بائٹاں;
	حرف *وچ;

#جے_تعریف_شدہ HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	وچ = حافظہ_دیو(MB_CUR_MAX);
	إذا (!وچ)
		أرجع -1;

	n = wctomb(وچ, wc);
	إذا (n == (size_t) -1)
		اذهب باہرلا;
	*ncells = wcwidth(wc);
#وإلا
	*ncells = 1;
	وچ = (حرف *) &c;
#جے_انت
	بائٹاں       = لڑی_لمبائی(str);

	memmove(str + n, str, بائٹاں);
	حافظہ_نقل(str, وچ, n);
	str[بائٹاں + n] = '\0';
#جے_تعریف_شدہ HAVE_WIDECHAR
باہرلا:
	حرر(وچ);
#جے_انت
	أرجع n;
}

ساكن صحيح mbs_edit_remove(بنية mbs_editor *edit)
{
	size_t n, ncells;

	إذا (edit->cur_cells == 0 || edit->cursor >= edit->cur_بائٹاں)
		أرجع 1;

	n = remove_اگلا(edit->buf + edit->cursor, &ncells);
	إذا (n == (size_t)-1)
		أرجع 1;

	edit->cur_بائٹاں -= n;
	edit->cur_cells = mbs_محفوظ_چوڑائی(edit->buf);
	أرجع 0;
}

صحيح mbs_edit_delete(بنية mbs_editor *edit)
{
	إذا (edit->cursor >= edit->cur_بائٹاں
	    && mbs_edit_goto(edit, MBS_EDIT_بایاں) == 1)
		أرجع 1;

	أرجع mbs_edit_remove(edit);
}

صحيح mbs_edit_backspace(بنية mbs_editor *edit)
{
	إذا (mbs_edit_goto(edit, MBS_EDIT_بایاں) == 0)
		أرجع mbs_edit_remove(edit);
	أرجع 1;
}

صحيح mbs_edit_داخل_کرو(بنية mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	إذا (edit->cur_بائٹاں + MB_CUR_MAX > edit->max_بائٹاں)
		أرجع 1;

	n = mbs_داخل_کرو(edit->buf + edit->cursor, c, &ncells);
	إذا (n == (size_t)-1)
		أرجع 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_بائٹاں += n;
	edit->cur_cells = mbs_محفوظ_چوڑائی(edit->buf);
	أرجع 0;
}
