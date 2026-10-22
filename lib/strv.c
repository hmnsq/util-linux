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

#ضمن <stdlib.h>
#ضمن <stdarg.h>
#ضمن <string.h>
#ضمن <errno.h>
#ضمن <stdbool.h>
#ضمن <assert.h>

#ضمن "strutils.h"
#ضمن "strv.h"

ساكن فراغ ul_strv_clear(حرف **l)
{
        حرف **k;

        אם (!l)
                החזר;

        עבור (k = l; *k; k++)
                حرر(*k);

        *l = عدم;
}

حرف **ul_strv_free(حرف **l)
{
        ul_strv_clear(l);
        حرر(l);
        החזר عدم;
}

حرف **ul_strv_copy(حرف * ثابت *l)
{
        حرف **r, **k;

        k = r = malloc(حجم(حرف *) * (ul_strv_length(l) + 1));
        אם (!r)
                החזר عدم;

        אם (l)
                עבור (; *l; k++, l++) {
                        *k = strdup(*l);
                        אם (!*k) {
                                ul_strv_free(r);
                                החזר عدم;
                        }
                }

        *k = عدم;
        החזר r;
}

unsigned ul_strv_length(حرف * ثابت *l) {
        unsigned n = 0;

        אם (!l)
                החזר 0;

        עבור (; *l; l++)
                n++;

        החזר n;
}

ساكن حرف **ul_strv_new_ap(ثابت حرف *x, va_list ap)
{
        ثابت حرف *s;
        حرف **a;
        unsigned n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        אם (x) {
                n = x == (ثابت حرف*) -1 ? 0 : 1;

                va_copy(aq, ap);
                כלעוד ((s = va_arg(aq, ثابت حرف*))) {
                        אם (s == (ثابت حرف*) -1)
                                استمر;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(حجم(حرف *) * (n + 1));
        אם (!a)
                החזר عدم;

        אם (x) {
                אם (x != (ثابت حرف*) -1) {
                        a[i] = strdup(x);
                        אם (!a[i])
                                اذهب fail;
                        i++;
                }

                כלעוד ((s = va_arg(ap, ثابت حرف*))) {

                        אם (s == (ثابت حرف*) -1)
                                استمر;

                        a[i] = strdup(s);
                        אם (!a[i])
                                اذهب fail;

                        i++;
                }
        }

        a[i] = عدم;

        החזר a;

fail:
        ul_strv_free(a);
        החזר عدم;
}

حرف **ul_strv_new(ثابت حرف *x, ...)
{
        حرف **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        החזר r;
}

صحيح ul_strv_extend_strv(حرف ***a, حرف **b)
{
        صحيح r;
        حرف **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                אם (r < 0)
                        החזר r;
        }

        החזר 0;
}

صحيح ul_strv_extend_strv_concat(حرف ***a, حرف **b, ثابت حرف *suffix)
{
        صحيح r;
        حرف **s;

        UL_STRV_FOREACH(s, b) {
                حرف *v;

                v = ul_strconcat(*s, suffix);
                אם (!v)
                        החזר -ENOMEM;

                r = ul_strv_push(a, v);
                אם (r < 0) {
                        حرر(v);
                        החזר r;
                }
        }

        החזר 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        עבור ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

#define FOREACH_WORD_SEPARATOR(word, length, s, separator, state)       \
        _FOREACH_WORD(word, length, s, separator, false, state)


حرف **ul_strv_split(ثابت حرف *s, ثابت حرف *separator)
{
        ثابت حرف *word, *state;
        size_t l;
        unsigned n, i;
        حرف **r;

        أكد(s);

        n = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state)
                n++;

        r = malloc(حجم(حرف *) * (n + 1));
        אם (!r)
                החזר عدم;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                אם (!r[i]) {
                        ul_strv_free(r);
                        החזר عدم;
                }

                i++;
        }

        r[i] = عدم;
        החזר r;
}

حرف *ul_strv_join(حرف **l, ثابت حرف *separator)
{
        حرف *r, *e;
        حرف **s;
        size_t n, k;

        אם (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                אם (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        אם (!r)
                החזר عدم;

        e = r;
        UL_STRV_FOREACH(s, l) {
                אם (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        החזר r;
}

صحيح ul_strv_push(حرف ***l, حرف *value)
{
        حرف **c;
        unsigned n, m;

        אם (!value)
                החזר 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        אם (m < n)
                החזר -ENOMEM;

        c = reallocarray(*l, m, حجم(حرف *));
        אם (!c)
                החזר -ENOMEM;

        c[n] = value;
        c[n+1] = عدم;

        *l = c;
        החזר 0;
}

صحيح ul_strv_push_prepend(حرف ***l, حرف *value)
{
        حرف **c;
        unsigned n, m, i;

        אם (!value)
                החזר 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        אם (m < n)
                החזר -ENOMEM;

        c = malloc(حجم(حرف *) * m);
        אם (!c)
                החזר -ENOMEM;

        עבור (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = value;
        c[n+1] = عدم;

        حرر(*l);
        *l = c;

        החזר 0;
}

صحيح ul_strv_consume(حرف ***l, حرف *value)
{
        صحيح r;

        r = ul_strv_push(l, value);
        אם (r < 0)
                حرر(value);

        החזר r;
}

صحيح ul_strv_consume_prepend(حرف ***l, حرف *value)
{
        صحيح r;

        r = ul_strv_push_prepend(l, value);
        אם (r < 0)
                حرر(value);

        החזר r;
}

صحيح ul_strv_extend(حرف ***l, ثابت حرف *value)
{
        حرف *v;

        אם (!value)
                החזר 0;

        v = strdup(value);
        אם (!v)
                החזר -ENOMEM;

        החזר ul_strv_consume(l, v);
}

حرف **ul_strv_remove(حرف **l, ثابت حرف *s)
{
        حرف **f, **t;

        אם (!l)
                החזר عدم;

        أكد(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        עבור (f = t = l; *f; f++)
                אם (strcmp(*f, s) == 0)
                        حرر(*f);
                אחרת
                        *(t++) = *f;

        *t = عدم;
        החזר l;
}

صحيح ul_strv_extendf(حرف ***l, ثابت حرف *format, ...)
{
        va_list ap;
        حرف *x;
        صحيح r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        אם (r < 0)
                החזר -ENOMEM;

        החזר ul_strv_consume(l, x);
}

صحيح ul_strv_extendv(حرف ***l, ثابت حرف *format, va_list ap)
{
        حرف *x;
        صحيح r;

        r = vasprintf(&x, format, ap);
        אם (r < 0)
                החזר -ENOMEM;

        החזר ul_strv_consume(l, x);
}

حرف **ul_strv_reverse(حرف **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        אם (n <= 1)
                החזר l;

        עבור (i = 0; i < n / 2; i++) {
                حرف *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        החזר l;
}
