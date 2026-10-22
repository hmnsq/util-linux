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
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <stdio.h>

#అన్తర్భావ "mbsalign.h"
#అన్తర్భావ "mbsedit.h"

నిర్మాణం mbs_editor *mbs_new_edit(అక్షరం *buf, size_t bufsz, size_t ncells)
{
	నిర్మాణం mbs_editor *edit = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*edit));

	ఒకవేళ (edit) {
		edit->buf = buf;
		edit->max_బైట్లు = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_సురక్షిత_వెడల్పు(buf);
		edit->cur_బైట్లు = అక్షరశ్రేణి_పొడవు(buf);
	}
	తిరిగి edit;
}

అక్షరం *mbs_free_edit(నిర్మాణం mbs_editor *edit)
{
	అక్షరం *ret = edit ? edit->buf : ఖాళీ;

	విడుదల(edit);
	తిరిగి ret;
}

స్థిర size_t mbs_తదుపరి(స్థిరరాశి అక్షరం *str, size_t *ncells)
{
#నిర్వచితమైతే HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	ఒకవేళ (!str || !*str)
		తిరిగి 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, ఖాళీ);
	*ncells = wcwidth(wc);
	తిరిగి n;
#లేకపోతే
	ఒకవేళ (!str || !*str)
		తిరిగి 0;
	*ncells = 1;
	తిరిగి 1;
#అయితే_ముగింపు
}

స్థిర size_t mbs_prev(స్థిరరాశి అక్షరం *ప్రారంభం, స్థిరరాశి అక్షరం *చివర, size_t *ncells)
{
#నిర్వచితమైతే HAVE_WIDECHAR
	wchar_t wc = 0;
	స్థిరరాశి అక్షరం *p, *prev;
	size_t n = 0;

	ఒకవేళ (!ప్రారంభం || !చివర || ప్రారంభం == చివర || !*ప్రారంభం)
		తిరిగి 0;

	prev = p = ప్రారంభం;
	అయితే (p < చివర) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, ఖాళీ);
		prev = p;

		ఒకవేళ (n == (size_t) -1 || n == (size_t) -2)
			p++;
		లేకపోతే
			p += n;
	}

	ఒకవేళ (prev == చివర)
		తిరిగి 0;
	*ncells = wcwidth(wc);
	తిరిగి n;
#లేకపోతే
	ఒకవేళ (!ప్రారంభం || !చివర || ప్రారంభం == చివర || !*ప్రారంభం)
		తిరిగి 0;
	*ncells = 1;
	తిరిగి 1;
#అయితే_ముగింపు
}

పూర్ణాంకం mbs_edit_goto(నిర్మాణం mbs_editor *edit, పూర్ణాంకం where)
{
	ఎంపిక (where) {
	స్థితి MBS_EDIT_ఎడమ:
		ఒకవేళ (edit->cursor == 0)
			తిరిగి 1;
		లేకపోతే {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			ఒకవేళ (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		ఆపు;
	స్థితి MBS_EDIT_కుడి:
		ఒకవేళ (edit->cursor_cells >= edit->cur_cells)
			తిరిగి 1;
		లేకపోతే {
			size_t n, cells;
			n = mbs_తదుపరి(edit->buf + edit->cursor, &cells);
			ఒకవేళ (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		ఆపు;
	స్థితి MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		ఆపు;
	స్థితి MBS_EDIT_చివర:
		edit->cursor = edit->cur_బైట్లు;
		edit->cursor_cells = edit->cur_cells;
		ఆపు;
	అప్రమేయ:
		తిరిగి -EINVAL;
	}

	తిరిగి 0;
}

/* Remove next MB from @str, returns number of removed bytes */
స్థిర size_t remove_తదుపరి(అక్షరం *str, size_t *ncells)
{
	/* all in bytes! */
	size_t బైట్లు, move_బైట్లు, n;

	n          = mbs_తదుపరి(str, ncells);
	బైట్లు      = అక్షరశ్రేణి_పొడవు(str);
	move_బైట్లు = బైట్లు - n;

	memmove(str, str + n, move_బైట్లు);
	str[బైట్లు - n] = '\0';
	తిరిగి n;
}

స్థిర size_t mbs_చొప్పించు(అక్షరం *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, బైట్లు;
	అక్షరం *లోపల;

#నిర్వచితమైతే HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	లోపల = జ్ఞాపకం_ఇవ్వు(MB_CUR_MAX);
	ఒకవేళ (!లోపల)
		తిరిగి -1;

	n = wctomb(లోపల, wc);
	ఒకవేళ (n == (size_t) -1)
		వెళ్ళు బహిర్గతం;
	*ncells = wcwidth(wc);
#లేకపోతే
	*ncells = 1;
	లోపల = (అక్షరం *) &c;
#అయితే_ముగింపు
	బైట్లు       = అక్షరశ్రేణి_పొడవు(str);

	memmove(str + n, str, బైట్లు);
	జ్ఞాపకం_నకలు(str, లోపల, n);
	str[బైట్లు + n] = '\0';
#నిర్వచితమైతే HAVE_WIDECHAR
బహిర్గతం:
	విడుదల(లోపల);
#అయితే_ముగింపు
	తిరిగి n;
}

స్థిర పూర్ణాంకం mbs_edit_remove(నిర్మాణం mbs_editor *edit)
{
	size_t n, ncells;

	ఒకవేళ (edit->cur_cells == 0 || edit->cursor >= edit->cur_బైట్లు)
		తిరిగి 1;

	n = remove_తదుపరి(edit->buf + edit->cursor, &ncells);
	ఒకవేళ (n == (size_t)-1)
		తిరిగి 1;

	edit->cur_బైట్లు -= n;
	edit->cur_cells = mbs_సురక్షిత_వెడల్పు(edit->buf);
	తిరిగి 0;
}

పూర్ణాంకం mbs_edit_delete(నిర్మాణం mbs_editor *edit)
{
	ఒకవేళ (edit->cursor >= edit->cur_బైట్లు
	    && mbs_edit_goto(edit, MBS_EDIT_ఎడమ) == 1)
		తిరిగి 1;

	తిరిగి mbs_edit_remove(edit);
}

పూర్ణాంకం mbs_edit_backspace(నిర్మాణం mbs_editor *edit)
{
	ఒకవేళ (mbs_edit_goto(edit, MBS_EDIT_ఎడమ) == 0)
		తిరిగి mbs_edit_remove(edit);
	తిరిగి 1;
}

పూర్ణాంకం mbs_edit_చొప్పించు(నిర్మాణం mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	ఒకవేళ (edit->cur_బైట్లు + MB_CUR_MAX > edit->max_బైట్లు)
		తిరిగి 1;

	n = mbs_చొప్పించు(edit->buf + edit->cursor, c, &ncells);
	ఒకవేళ (n == (size_t)-1)
		తిరిగి 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_బైట్లు += n;
	edit->cur_cells = mbs_సురక్షిత_వెడల్పు(edit->buf);
	తిరిగి 0;
}
