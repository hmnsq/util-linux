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
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <stdio.h>

#अन्तर्भाव "mbsalign.h"
#अन्तर्भाव "mbsedit.h"

संरचना mbs_editor *mbs_new_edit(वर्ण *buf, size_t bufsz, size_t ncells)
{
	संरचना mbs_editor *edit = शून्यस्मृतिं_देहि(1, परिमाण(*edit));

	यदि (edit) {
		edit->buf = buf;
		edit->max_अष्टकानि = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_विस्तारः(buf);
		edit->cur_अष्टकानि = वाक्यदैर्घ्यम्(buf);
	}
	प्रत्यागम edit;
}

वर्ण *mbs_free_edit(संरचना mbs_editor *edit)
{
	वर्ण *ret = edit ? edit->buf : रिक्त;

	मुक्त(edit);
	प्रत्यागम ret;
}

स्थिर size_t mbs_अग्र(ध्रुव वर्ण *str, size_t *ncells)
{
#यदि_परिभाषितम् HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	यदि (!str || !*str)
		प्रत्यागम 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, रिक्त);
	*ncells = wcwidth(wc);
	प्रत्यागम n;
#अन्यथा
	यदि (!str || !*str)
		प्रत्यागम 0;
	*ncells = 1;
	प्रत्यागम 1;
#यदि_अन्तः
}

स्थिर size_t mbs_पूर्व(ध्रुव वर्ण *start, ध्रुव वर्ण *अन्तः, size_t *ncells)
{
#यदि_परिभाषितम् HAVE_WIDECHAR
	wchar_t wc = 0;
	ध्रुव वर्ण *p, *पूर्व;
	size_t n = 0;

	यदि (!start || !अन्तः || start == अन्तः || !*start)
		प्रत्यागम 0;

	पूर्व = p = start;
	यावत् (p < अन्तः) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, रिक्त);
		पूर्व = p;

		यदि (n == (size_t) -1 || n == (size_t) -2)
			p++;
		अन्यथा
			p += n;
	}

	यदि (पूर्व == अन्तः)
		प्रत्यागम 0;
	*ncells = wcwidth(wc);
	प्रत्यागम n;
#अन्यथा
	यदि (!start || !अन्तः || start == अन्तः || !*start)
		प्रत्यागम 0;
	*ncells = 1;
	प्रत्यागम 1;
#यदि_अन्तः
}

पूर्णाङ्क mbs_edit_goto(संरचना mbs_editor *edit, पूर्णाङ्क where)
{
	विकल्प (where) {
	पक्ष MBS_EDIT_वामः:
		यदि (edit->cursor == 0)
			प्रत्यागम 1;
		अन्यथा {
			size_t n, cells;
			n = mbs_पूर्व(edit->buf, edit->buf + edit->cursor, &cells);
			यदि (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		भङ्ग;
	पक्ष MBS_EDIT_दक्षिणः:
		यदि (edit->cursor_cells >= edit->cur_cells)
			प्रत्यागम 1;
		अन्यथा {
			size_t n, cells;
			n = mbs_अग्र(edit->buf + edit->cursor, &cells);
			यदि (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		भङ्ग;
	पक्ष MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		भङ्ग;
	पक्ष MBS_EDIT_अन्तः:
		edit->cursor = edit->cur_अष्टकानि;
		edit->cursor_cells = edit->cur_cells;
		भङ्ग;
	स्वभाव:
		प्रत्यागम -EINVAL;
	}

	प्रत्यागम 0;
}

/* Remove next MB from @str, returns number of removed bytes */
स्थिर size_t remove_अग्र(वर्ण *str, size_t *ncells)
{
	/* all in bytes! */
	size_t अष्टकानि, move_अष्टकानि, n;

	n          = mbs_अग्र(str, ncells);
	अष्टकानि      = वाक्यदैर्घ्यम्(str);
	move_अष्टकानि = अष्टकानि - n;

	memmove(str, str + n, move_अष्टकानि);
	str[अष्टकानि - n] = '\0';
	प्रत्यागम n;
}

स्थिर size_t mbs_निवेशय(वर्ण *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, अष्टकानि;
	वर्ण *अन्तर्;

#यदि_परिभाषितम् HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	अन्तर् = स्मृतिं_देहि(MB_CUR_MAX);
	यदि (!अन्तर्)
		प्रत्यागम -1;

	n = wctomb(अन्तर्, wc);
	यदि (n == (size_t) -1)
		गच्छ बहिर्गमनम्;
	*ncells = wcwidth(wc);
#अन्यथा
	*ncells = 1;
	अन्तर् = (वर्ण *) &c;
#यदि_अन्तः
	अष्टकानि       = वाक्यदैर्घ्यम्(str);

	memmove(str + n, str, अष्टकानि);
	स्मृतिं_प्रतिलिख(str, अन्तर्, n);
	str[अष्टकानि + n] = '\0';
#यदि_परिभाषितम् HAVE_WIDECHAR
बहिर्गमनम्:
	मुक्त(अन्तर्);
#यदि_अन्तः
	प्रत्यागम n;
}

स्थिर पूर्णाङ्क mbs_edit_remove(संरचना mbs_editor *edit)
{
	size_t n, ncells;

	यदि (edit->cur_cells == 0 || edit->cursor >= edit->cur_अष्टकानि)
		प्रत्यागम 1;

	n = remove_अग्र(edit->buf + edit->cursor, &ncells);
	यदि (n == (size_t)-1)
		प्रत्यागम 1;

	edit->cur_अष्टकानि -= n;
	edit->cur_cells = mbs_safe_विस्तारः(edit->buf);
	प्रत्यागम 0;
}

पूर्णाङ्क mbs_edit_delete(संरचना mbs_editor *edit)
{
	यदि (edit->cursor >= edit->cur_अष्टकानि
	    && mbs_edit_goto(edit, MBS_EDIT_वामः) == 1)
		प्रत्यागम 1;

	प्रत्यागम mbs_edit_remove(edit);
}

पूर्णाङ्क mbs_edit_backspace(संरचना mbs_editor *edit)
{
	यदि (mbs_edit_goto(edit, MBS_EDIT_वामः) == 0)
		प्रत्यागम mbs_edit_remove(edit);
	प्रत्यागम 1;
}

पूर्णाङ्क mbs_edit_निवेशय(संरचना mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	यदि (edit->cur_अष्टकानि + MB_CUR_MAX > edit->max_अष्टकानि)
		प्रत्यागम 1;

	n = mbs_निवेशय(edit->buf + edit->cursor, c, &ncells);
	यदि (n == (size_t)-1)
		प्रत्यागम 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_अष्टकानि += n;
	edit->cur_cells = mbs_safe_विस्तारः(edit->buf);
	प्रत्यागम 0;
}
