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

ساخت mbs_editor *mbs_new_edit(حرف *buf, size_t bufsz, size_t ncells)
{
	ساخت mbs_editor *edit = صفر_حافظہ_دو(1, حجم(*edit));

	اگر (edit) {
		edit->buf = buf;
		edit->max_بائٹیں = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_محفوظ_چوڑائی(buf);
		edit->cur_بائٹیں = لڑی_لمبائی(buf);
	}
	واپس edit;
}

حرف *mbs_free_edit(ساخت mbs_editor *edit)
{
	حرف *ret = edit ? edit->buf : تہی;

	آزاد(edit);
	واپس ret;
}

ساکن size_t mbs_اگلا(ثابت حرف *str, size_t *ncells)
{
#اگر_تعریف_شدہ HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	اگر (!str || !*str)
		واپس 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, تہی);
	*ncells = wcwidth(wc);
	واپس n;
#ورنہ
	اگر (!str || !*str)
		واپس 0;
	*ncells = 1;
	واپس 1;
#اگر_ختم
}

ساکن size_t mbs_پچھلا(ثابت حرف *آغاز, ثابت حرف *انتہا, size_t *ncells)
{
#اگر_تعریف_شدہ HAVE_WIDECHAR
	wchar_t wc = 0;
	ثابت حرف *p, *پچھلا;
	size_t n = 0;

	اگر (!آغاز || !انتہا || آغاز == انتہا || !*آغاز)
		واپس 0;

	پچھلا = p = آغاز;
	جب (p < انتہا) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, تہی);
		پچھلا = p;

		اگر (n == (size_t) -1 || n == (size_t) -2)
			p++;
		ورنہ
			p += n;
	}

	اگر (پچھلا == انتہا)
		واپس 0;
	*ncells = wcwidth(wc);
	واپس n;
#ورنہ
	اگر (!آغاز || !انتہا || آغاز == انتہا || !*آغاز)
		واپس 0;
	*ncells = 1;
	واپس 1;
#اگر_ختم
}

صحیح mbs_edit_goto(ساخت mbs_editor *edit, صحیح where)
{
	انتخاب (where) {
	حالت MBS_EDIT_بایاں:
		اگر (edit->cursor == 0)
			واپس 1;
		ورنہ {
			size_t n, cells;
			n = mbs_پچھلا(edit->buf, edit->buf + edit->cursor, &cells);
			اگر (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		توڑو;
	حالت MBS_EDIT_دایاں:
		اگر (edit->cursor_cells >= edit->cur_cells)
			واپس 1;
		ورنہ {
			size_t n, cells;
			n = mbs_اگلا(edit->buf + edit->cursor, &cells);
			اگر (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		توڑو;
	حالت MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		توڑو;
	حالت MBS_EDIT_انتہا:
		edit->cursor = edit->cur_بائٹیں;
		edit->cursor_cells = edit->cur_cells;
		توڑو;
	طےشدہ:
		واپس -EINVAL;
	}

	واپس 0;
}

/* Remove next MB from @str, returns number of removed bytes */
ساکن size_t remove_اگلا(حرف *str, size_t *ncells)
{
	/* all in bytes! */
	size_t بائٹیں, move_بائٹیں, n;

	n          = mbs_اگلا(str, ncells);
	بائٹیں      = لڑی_لمبائی(str);
	move_بائٹیں = بائٹیں - n;

	memmove(str, str + n, move_بائٹیں);
	str[بائٹیں - n] = '\0';
	واپس n;
}

ساکن size_t mbs_داخل_کرو(حرف *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, بائٹیں;
	حرف *اندر;

#اگر_تعریف_شدہ HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	اندر = حافظہ_دو(MB_CUR_MAX);
	اگر (!اندر)
		واپس -1;

	n = wctomb(اندر, wc);
	اگر (n == (size_t) -1)
		جاؤ برآمد;
	*ncells = wcwidth(wc);
#ورنہ
	*ncells = 1;
	اندر = (حرف *) &c;
#اگر_ختم
	بائٹیں       = لڑی_لمبائی(str);

	memmove(str + n, str, بائٹیں);
	حافظہ_نقل(str, اندر, n);
	str[بائٹیں + n] = '\0';
#اگر_تعریف_شدہ HAVE_WIDECHAR
برآمد:
	آزاد(اندر);
#اگر_ختم
	واپس n;
}

ساکن صحیح mbs_edit_remove(ساخت mbs_editor *edit)
{
	size_t n, ncells;

	اگر (edit->cur_cells == 0 || edit->cursor >= edit->cur_بائٹیں)
		واپس 1;

	n = remove_اگلا(edit->buf + edit->cursor, &ncells);
	اگر (n == (size_t)-1)
		واپس 1;

	edit->cur_بائٹیں -= n;
	edit->cur_cells = mbs_محفوظ_چوڑائی(edit->buf);
	واپس 0;
}

صحیح mbs_edit_delete(ساخت mbs_editor *edit)
{
	اگر (edit->cursor >= edit->cur_بائٹیں
	    && mbs_edit_goto(edit, MBS_EDIT_بایاں) == 1)
		واپس 1;

	واپس mbs_edit_remove(edit);
}

صحیح mbs_edit_backspace(ساخت mbs_editor *edit)
{
	اگر (mbs_edit_goto(edit, MBS_EDIT_بایاں) == 0)
		واپس mbs_edit_remove(edit);
	واپس 1;
}

صحیح mbs_edit_داخل_کرو(ساخت mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	اگر (edit->cur_بائٹیں + MB_CUR_MAX > edit->max_بائٹیں)
		واپس 1;

	n = mbs_داخل_کرو(edit->buf + edit->cursor, c, &ncells);
	اگر (n == (size_t)-1)
		واپس 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_بائٹیں += n;
	edit->cur_cells = mbs_محفوظ_چوڑائی(edit->buf);
	واپس 0;
}
