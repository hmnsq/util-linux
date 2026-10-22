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
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>

#ಅನ್ತರ್ಭಾವ "mbsalign.h"
#ಅನ್ತರ್ಭಾವ "mbsedit.h"

ರಚನೆ mbs_editor *mbs_new_edit(ಅಕ್ಷರ *buf, size_t bufsz, size_t ncells)
{
	ರಚನೆ mbs_editor *edit = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*edit));

	ಆದರೆ (edit) {
		edit->buf = buf;
		edit->max_ಬೈಟುಗಳು = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_ಸುರಕ್ಷಿತ_ಅಗಲ(buf);
		edit->cur_ಬೈಟುಗಳು = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(buf);
	}
	ಹಿಂತಿರುಗಿ edit;
}

ಅಕ್ಷರ *mbs_free_edit(ರಚನೆ mbs_editor *edit)
{
	ಅಕ್ಷರ *ret = edit ? edit->buf : ಖಾಲಿ;

	ಬಿಡುಗಡೆ(edit);
	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ size_t mbs_ಮುಂದಿನ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, size_t *ncells)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	ಆದರೆ (!str || !*str)
		ಹಿಂತಿರುಗಿ 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, ಖಾಲಿ);
	*ncells = wcwidth(wc);
	ಹಿಂತಿರುಗಿ n;
#ಇಲ್ಲದಿದ್ದರೆ
	ಆದರೆ (!str || !*str)
		ಹಿಂತಿರುಗಿ 0;
	*ncells = 1;
	ಹಿಂತಿರುಗಿ 1;
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ size_t mbs_prev(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆರಂಭ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆ, size_t *ncells)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_WIDECHAR
	wchar_t wc = 0;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p, *prev;
	size_t n = 0;

	ಆದರೆ (!ಆರಂಭ || !ಕೊನೆ || ಆರಂಭ == ಕೊನೆ || !*ಆರಂಭ)
		ಹಿಂತಿರುಗಿ 0;

	prev = p = ಆರಂಭ;
	ಆಗ (p < ಕೊನೆ) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, ಖಾಲಿ);
		prev = p;

		ಆದರೆ (n == (size_t) -1 || n == (size_t) -2)
			p++;
		ಇಲ್ಲದಿದ್ದರೆ
			p += n;
	}

	ಆದರೆ (prev == ಕೊನೆ)
		ಹಿಂತಿರುಗಿ 0;
	*ncells = wcwidth(wc);
	ಹಿಂತಿರುಗಿ n;
#ಇಲ್ಲದಿದ್ದರೆ
	ಆದರೆ (!ಆರಂಭ || !ಕೊನೆ || ಆರಂಭ == ಕೊನೆ || !*ಆರಂಭ)
		ಹಿಂತಿರುಗಿ 0;
	*ncells = 1;
	ಹಿಂತಿರುಗಿ 1;
#ಆದರೆ_ಅಂತ್ಯ
}

ಪೂರ್ಣಾಂಕ mbs_edit_goto(ರಚನೆ mbs_editor *edit, ಪೂರ್ಣಾಂಕ where)
{
	ಆಯ್ಕೆ (where) {
	ಸ್ಥಿತಿ MBS_EDIT_ಎಡ:
		ಆದರೆ (edit->cursor == 0)
			ಹಿಂತಿರುಗಿ 1;
		ಇಲ್ಲದಿದ್ದರೆ {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			ಆದರೆ (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ MBS_EDIT_ಬಲ:
		ಆದರೆ (edit->cursor_cells >= edit->cur_cells)
			ಹಿಂತಿರುಗಿ 1;
		ಇಲ್ಲದಿದ್ದರೆ {
			size_t n, cells;
			n = mbs_ಮುಂದಿನ(edit->buf + edit->cursor, &cells);
			ಆದರೆ (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ MBS_EDIT_ಕೊನೆ:
		edit->cursor = edit->cur_ಬೈಟುಗಳು;
		edit->cursor_cells = edit->cur_cells;
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	ಹಿಂತಿರುಗಿ 0;
}

/* Remove next MB from @str, returns number of removed bytes */
ಸ್ಥಿರ size_t remove_ಮುಂದಿನ(ಅಕ್ಷರ *str, size_t *ncells)
{
	/* all in bytes! */
	size_t ಬೈಟುಗಳು, move_ಬೈಟುಗಳು, n;

	n          = mbs_ಮುಂದಿನ(str, ncells);
	ಬೈಟುಗಳು      = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str);
	move_ಬೈಟುಗಳು = ಬೈಟುಗಳು - n;

	memmove(str, str + n, move_ಬೈಟುಗಳು);
	str[ಬೈಟುಗಳು - n] = '\0';
	ಹಿಂತಿರುಗಿ n;
}

ಸ್ಥಿರ size_t mbs_ಸೇರಿಸು(ಅಕ್ಷರ *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, ಬೈಟುಗಳು;
	ಅಕ್ಷರ *ಒಳ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	ಒಳ = ಸ್ಮರಣೆ_ಕೊಡು(MB_CUR_MAX);
	ಆದರೆ (!ಒಳ)
		ಹಿಂತಿರುಗಿ -1;

	n = wctomb(ಒಳ, wc);
	ಆದರೆ (n == (size_t) -1)
		ಹೋಗು ಹೊರಹೋಗು;
	*ncells = wcwidth(wc);
#ಇಲ್ಲದಿದ್ದರೆ
	*ncells = 1;
	ಒಳ = (ಅಕ್ಷರ *) &c;
#ಆದರೆ_ಅಂತ್ಯ
	ಬೈಟುಗಳು       = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str);

	memmove(str + n, str, ಬೈಟುಗಳು);
	ಸ್ಮರಣೆ_ನಕಲು(str, ಒಳ, n);
	str[ಬೈಟುಗಳು + n] = '\0';
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_WIDECHAR
ಹೊರಹೋಗು:
	ಬಿಡುಗಡೆ(ಒಳ);
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ n;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ mbs_edit_remove(ರಚನೆ mbs_editor *edit)
{
	size_t n, ncells;

	ಆದರೆ (edit->cur_cells == 0 || edit->cursor >= edit->cur_ಬೈಟುಗಳು)
		ಹಿಂತಿರುಗಿ 1;

	n = remove_ಮುಂದಿನ(edit->buf + edit->cursor, &ncells);
	ಆದರೆ (n == (size_t)-1)
		ಹಿಂತಿರುಗಿ 1;

	edit->cur_ಬೈಟುಗಳು -= n;
	edit->cur_cells = mbs_ಸುರಕ್ಷಿತ_ಅಗಲ(edit->buf);
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ mbs_edit_delete(ರಚನೆ mbs_editor *edit)
{
	ಆದರೆ (edit->cursor >= edit->cur_ಬೈಟುಗಳು
	    && mbs_edit_goto(edit, MBS_EDIT_ಎಡ) == 1)
		ಹಿಂತಿರುಗಿ 1;

	ಹಿಂತಿರುಗಿ mbs_edit_remove(edit);
}

ಪೂರ್ಣಾಂಕ mbs_edit_backspace(ರಚನೆ mbs_editor *edit)
{
	ಆದರೆ (mbs_edit_goto(edit, MBS_EDIT_ಎಡ) == 0)
		ಹಿಂತಿರುಗಿ mbs_edit_remove(edit);
	ಹಿಂತಿರುಗಿ 1;
}

ಪೂರ್ಣಾಂಕ mbs_edit_ಸೇರಿಸು(ರಚನೆ mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	ಆದರೆ (edit->cur_ಬೈಟುಗಳು + MB_CUR_MAX > edit->max_ಬೈಟುಗಳು)
		ಹಿಂತಿರುಗಿ 1;

	n = mbs_ಸೇರಿಸು(edit->buf + edit->cursor, c, &ncells);
	ಆದರೆ (n == (size_t)-1)
		ಹಿಂತಿರುಗಿ 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_ಬೈಟುಗಳು += n;
	edit->cur_cells = mbs_ಸುರಕ್ಷಿತ_ಅಗಲ(edit->buf);
	ಹಿಂತಿರುಗಿ 0;
}
