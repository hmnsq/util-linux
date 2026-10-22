/*
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Copyright (C) 2010 Lennart Poettering
 * Copyright (C) 2015-2022 Karel Zak <kzak@redhat.com>
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 *
 * Copyright (C) 2015 Karel Zak <kzak@redhat.com>
 *    Modified the original version from systemd project for util-linux.
 */

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <assert.h>

#ಅನ್ತರ್ಭಾವ "strutils.h"
#ಅನ್ತರ್ಭಾವ "strv.h"

ಸ್ಥಿರ ಶೂನ್ಯ ul_strv_clear(ವರ್ಣ **l)
{
        ವರ್ಣ **k;

        ಯದಿ (!l)
                ಪ್ರತ್ಯಾಗಮ;

        ಪ್ರತಿ (k = l; *k; k++)
                ಮುಕ್ತ(*k);

        *l = ರಿಕ್ತ;
}

ವರ್ಣ **ul_strv_free(ವರ್ಣ **l)
{
        ul_strv_clear(l);
        ಮುಕ್ತ(l);
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ವರ್ಣ **ul_strv_copy(ವರ್ಣ * ಧ್ರುವ *l)
{
        ವರ್ಣ **r, **k;

        k = r = malloc(ಪರಿಮಾಣ(ವರ್ಣ *) * (ul_strv_length(l) + 1));
        ಯದಿ (!r)
                ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

        ಯದಿ (l)
                ಪ್ರತಿ (; *l; k++, l++) {
                        *k = strdup(*l);
                        ಯದಿ (!*k) {
                                ul_strv_free(r);
                                ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
                        }
                }

        *k = ರಿಕ್ತ;
        ಪ್ರತ್ಯಾಗಮ r;
}

unsigned ul_strv_length(ವರ್ಣ * ಧ್ರುವ *l) {
        unsigned n = 0;

        ಯದಿ (!l)
                ಪ್ರತ್ಯಾಗಮ 0;

        ಪ್ರತಿ (; *l; l++)
                n++;

        ಪ್ರತ್ಯಾಗಮ n;
}

ಸ್ಥಿರ ವರ್ಣ **ul_strv_new_ap(ಧ್ರುವ ವರ್ಣ *x, va_list ap)
{
        ಧ್ರುವ ವರ್ಣ *s;
        ವರ್ಣ **a;
        unsigned n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        ಯದಿ (x) {
                n = x == (ಧ್ರುವ ವರ್ಣ*) -1 ? 0 : 1;

                va_copy(aq, ap);
                ಯಾವತ್ ((s = va_arg(aq, ಧ್ರುವ ವರ್ಣ*))) {
                        ಯದಿ (s == (ಧ್ರುವ ವರ್ಣ*) -1)
                                ಅನುವರ್ತ;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(ಪರಿಮಾಣ(ವರ್ಣ *) * (n + 1));
        ಯದಿ (!a)
                ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

        ಯದಿ (x) {
                ಯದಿ (x != (ಧ್ರುವ ವರ್ಣ*) -1) {
                        a[i] = strdup(x);
                        ಯದಿ (!a[i])
                                ಗಚ್ಛ fail;
                        i++;
                }

                ಯಾವತ್ ((s = va_arg(ap, ಧ್ರುವ ವರ್ಣ*))) {

                        ಯದಿ (s == (ಧ್ರುವ ವರ್ಣ*) -1)
                                ಅನುವರ್ತ;

                        a[i] = strdup(s);
                        ಯದಿ (!a[i])
                                ಗಚ್ಛ fail;

                        i++;
                }
        }

        a[i] = ರಿಕ್ತ;

        ಪ್ರತ್ಯಾಗಮ a;

fail:
        ul_strv_free(a);
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ವರ್ಣ **ul_strv_new(ಧ್ರುವ ವರ್ಣ *x, ...)
{
        ವರ್ಣ **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        ಪ್ರತ್ಯಾಗಮ r;
}

ಪೂರ್ಣಾಙ್ಕ ul_strv_extend_strv(ವರ್ಣ ***a, ವರ್ಣ **b)
{
        ಪೂರ್ಣಾಙ್ಕ r;
        ವರ್ಣ **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                ಯದಿ (r < 0)
                        ಪ್ರತ್ಯಾಗಮ r;
        }

        ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ ul_strv_extend_strv_concat(ವರ್ಣ ***a, ವರ್ಣ **b, ಧ್ರುವ ವರ್ಣ *suffix)
{
        ಪೂರ್ಣಾಙ್ಕ r;
        ವರ್ಣ **s;

        UL_STRV_FOREACH(s, b) {
                ವರ್ಣ *v;

                v = ul_strconcat(*s, suffix);
                ಯದಿ (!v)
                        ಪ್ರತ್ಯಾಗಮ -ENOMEM;

                r = ul_strv_ತಳ್ಳು(a, v);
                ಯದಿ (r < 0) {
                        ಮುಕ್ತ(v);
                        ಪ್ರತ್ಯಾಗಮ r;
                }
        }

        ಪ್ರತ್ಯಾಗಮ 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        ಪ್ರತಿ ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

#define FOREACH_WORD_SEPARATOR(word, length, s, separator, state)       \
        _FOREACH_WORD(word, length, s, separator, false, state)


ವರ್ಣ **ul_strv_split(ಧ್ರುವ ವರ್ಣ *s, ಧ್ರುವ ವರ್ಣ *separator)
{
        ಧ್ರುವ ವರ್ಣ *word, *state;
        size_t l;
        unsigned n, i;
        ವರ್ಣ **r;

        ಪ್ರತಿಜ್ಞಾ(s);

        n = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state)
                n++;

        r = malloc(ಪರಿಮಾಣ(ವರ್ಣ *) * (n + 1));
        ಯದಿ (!r)
                ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                ಯದಿ (!r[i]) {
                        ul_strv_free(r);
                        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
                }

                i++;
        }

        r[i] = ರಿಕ್ತ;
        ಪ್ರತ್ಯಾಗಮ r;
}

ವರ್ಣ *ul_strv_join(ವರ್ಣ **l, ಧ್ರುವ ವರ್ಣ *separator)
{
        ವರ್ಣ *r, *e;
        ವರ್ಣ **s;
        size_t n, k;

        ಯದಿ (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                ಯದಿ (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        ಯದಿ (!r)
                ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

        e = r;
        UL_STRV_FOREACH(s, l) {
                ಯದಿ (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        ಪ್ರತ್ಯಾಗಮ r;
}

ಪೂರ್ಣಾಙ್ಕ ul_strv_ತಳ್ಳು(ವರ್ಣ ***l, ವರ್ಣ *ಮೌಲ್ಯ)
{
        ವರ್ಣ **c;
        unsigned n, m;

        ಯದಿ (!ಮೌಲ್ಯ)
                ಪ್ರತ್ಯಾಗಮ 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        ಯದಿ (m < n)
                ಪ್ರತ್ಯಾಗಮ -ENOMEM;

        c = reallocarray(*l, m, ಪರಿಮಾಣ(ವರ್ಣ *));
        ಯದಿ (!c)
                ಪ್ರತ್ಯಾಗಮ -ENOMEM;

        c[n] = ಮೌಲ್ಯ;
        c[n+1] = ರಿಕ್ತ;

        *l = c;
        ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ ul_strv_ತಳ್ಳು_prepend(ವರ್ಣ ***l, ವರ್ಣ *ಮೌಲ್ಯ)
{
        ವರ್ಣ **c;
        unsigned n, m, i;

        ಯದಿ (!ಮೌಲ್ಯ)
                ಪ್ರತ್ಯಾಗಮ 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        ಯದಿ (m < n)
                ಪ್ರತ್ಯಾಗಮ -ENOMEM;

        c = malloc(ಪರಿಮಾಣ(ವರ್ಣ *) * m);
        ಯದಿ (!c)
                ಪ್ರತ್ಯಾಗಮ -ENOMEM;

        ಪ್ರತಿ (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = ಮೌಲ್ಯ;
        c[n+1] = ರಿಕ್ತ;

        ಮುಕ್ತ(*l);
        *l = c;

        ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ ul_strv_consume(ವರ್ಣ ***l, ವರ್ಣ *ಮೌಲ್ಯ)
{
        ಪೂರ್ಣಾಙ್ಕ r;

        r = ul_strv_ತಳ್ಳು(l, ಮೌಲ್ಯ);
        ಯದಿ (r < 0)
                ಮುಕ್ತ(ಮೌಲ್ಯ);

        ಪ್ರತ್ಯಾಗಮ r;
}

ಪೂರ್ಣಾಙ್ಕ ul_strv_consume_prepend(ವರ್ಣ ***l, ವರ್ಣ *ಮೌಲ್ಯ)
{
        ಪೂರ್ಣಾಙ್ಕ r;

        r = ul_strv_ತಳ್ಳು_prepend(l, ಮೌಲ್ಯ);
        ಯದಿ (r < 0)
                ಮುಕ್ತ(ಮೌಲ್ಯ);

        ಪ್ರತ್ಯಾಗಮ r;
}

ಪೂರ್ಣಾಙ್ಕ ul_strv_extend(ವರ್ಣ ***l, ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ)
{
        ವರ್ಣ *v;

        ಯದಿ (!ಮೌಲ್ಯ)
                ಪ್ರತ್ಯಾಗಮ 0;

        v = strdup(ಮೌಲ್ಯ);
        ಯದಿ (!v)
                ಪ್ರತ್ಯಾಗಮ -ENOMEM;

        ಪ್ರತ್ಯಾಗಮ ul_strv_consume(l, v);
}

ವರ್ಣ **ul_strv_remove(ವರ್ಣ **l, ಧ್ರುವ ವರ್ಣ *s)
{
        ವರ್ಣ **f, **t;

        ಯದಿ (!l)
                ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

        ಪ್ರತಿಜ್ಞಾ(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        ಪ್ರತಿ (f = t = l; *f; f++)
                ಯದಿ (strcmp(*f, s) == 0)
                        ಮುಕ್ತ(*f);
                ಅನ್ಯಥಾ
                        *(t++) = *f;

        *t = ರಿಕ್ತ;
        ಪ್ರತ್ಯಾಗಮ l;
}

ಪೂರ್ಣಾಙ್ಕ ul_strv_extendf(ವರ್ಣ ***l, ಧ್ರುವ ವರ್ಣ *format, ...)
{
        va_list ap;
        ವರ್ಣ *x;
        ಪೂರ್ಣಾಙ್ಕ r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        ಯದಿ (r < 0)
                ಪ್ರತ್ಯಾಗಮ -ENOMEM;

        ಪ್ರತ್ಯಾಗಮ ul_strv_consume(l, x);
}

ಪೂರ್ಣಾಙ್ಕ ul_strv_extendv(ವರ್ಣ ***l, ಧ್ರುವ ವರ್ಣ *format, va_list ap)
{
        ವರ್ಣ *x;
        ಪೂರ್ಣಾಙ್ಕ r;

        r = vasprintf(&x, format, ap);
        ಯದಿ (r < 0)
                ಪ್ರತ್ಯಾಗಮ -ENOMEM;

        ಪ್ರತ್ಯಾಗಮ ul_strv_consume(l, x);
}

ವರ್ಣ **ul_strv_reverse(ವರ್ಣ **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        ಯದಿ (n <= 1)
                ಪ್ರತ್ಯಾಗಮ l;

        ಪ್ರತಿ (i = 0; i < n / 2; i++) {
                ವರ್ಣ *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        ಪ್ರತ್ಯಾಗಮ l;
}
