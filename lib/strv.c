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

        إذا (!l)
                إرجاع;

        لكل (k = l; *k; k++)
                حرر(*k);

        *l = عدم;
}

حرف **ul_strv_free(حرف **l)
{
        ul_strv_clear(l);
        حرر(l);
        إرجاع عدم;
}

حرف **ul_strv_copy(حرف * ثابت *l)
{
        حرف **r, **k;

        k = r = malloc(حجم(حرف *) * (ul_strv_length(l) + 1));
        إذا (!r)
                إرجاع عدم;

        إذا (l)
                لكل (; *l; k++, l++) {
                        *k = strdup(*l);
                        إذا (!*k) {
                                ul_strv_free(r);
                                إرجاع عدم;
                        }
                }

        *k = عدم;
        إرجاع r;
}

unsigned ul_strv_length(حرف * ثابت *l) {
        unsigned n = 0;

        إذا (!l)
                إرجاع 0;

        لكل (; *l; l++)
                n++;

        إرجاع n;
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

        إذا (x) {
                n = x == (ثابت حرف*) -1 ? 0 : 1;

                va_copy(aq, ap);
                بينما ((s = va_arg(aq, ثابت حرف*))) {
                        إذا (s == (ثابت حرف*) -1)
                                استمر;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(حجم(حرف *) * (n + 1));
        إذا (!a)
                إرجاع عدم;

        إذا (x) {
                إذا (x != (ثابت حرف*) -1) {
                        a[i] = strdup(x);
                        إذا (!a[i])
                                اذهب fail;
                        i++;
                }

                بينما ((s = va_arg(ap, ثابت حرف*))) {

                        إذا (s == (ثابت حرف*) -1)
                                استمر;

                        a[i] = strdup(s);
                        إذا (!a[i])
                                اذهب fail;

                        i++;
                }
        }

        a[i] = عدم;

        إرجاع a;

fail:
        ul_strv_free(a);
        إرجاع عدم;
}

حرف **ul_strv_new(ثابت حرف *x, ...)
{
        حرف **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        إرجاع r;
}

صحيح ul_strv_extend_strv(حرف ***a, حرف **b)
{
        صحيح r;
        حرف **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                إذا (r < 0)
                        إرجاع r;
        }

        إرجاع 0;
}

صحيح ul_strv_extend_strv_concat(حرف ***a, حرف **b, ثابت حرف *suffix)
{
        صحيح r;
        حرف **s;

        UL_STRV_FOREACH(s, b) {
                حرف *v;

                v = ul_strconcat(*s, suffix);
                إذا (!v)
                        إرجاع -ENOMEM;

                r = ul_strv_ادفع(a, v);
                إذا (r < 0) {
                        حرر(v);
                        إرجاع r;
                }
        }

        إرجاع 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        لكل ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

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
        إذا (!r)
                إرجاع عدم;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                إذا (!r[i]) {
                        ul_strv_free(r);
                        إرجاع عدم;
                }

                i++;
        }

        r[i] = عدم;
        إرجاع r;
}

حرف *ul_strv_join(حرف **l, ثابت حرف *separator)
{
        حرف *r, *e;
        حرف **s;
        size_t n, k;

        إذا (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                إذا (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        إذا (!r)
                إرجاع عدم;

        e = r;
        UL_STRV_FOREACH(s, l) {
                إذا (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        إرجاع r;
}

صحيح ul_strv_ادفع(حرف ***l, حرف *قيمة)
{
        حرف **c;
        unsigned n, m;

        إذا (!قيمة)
                إرجاع 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        إذا (m < n)
                إرجاع -ENOMEM;

        c = reallocarray(*l, m, حجم(حرف *));
        إذا (!c)
                إرجاع -ENOMEM;

        c[n] = قيمة;
        c[n+1] = عدم;

        *l = c;
        إرجاع 0;
}

صحيح ul_strv_ادفع_prepend(حرف ***l, حرف *قيمة)
{
        حرف **c;
        unsigned n, m, i;

        إذا (!قيمة)
                إرجاع 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        إذا (m < n)
                إرجاع -ENOMEM;

        c = malloc(حجم(حرف *) * m);
        إذا (!c)
                إرجاع -ENOMEM;

        لكل (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = قيمة;
        c[n+1] = عدم;

        حرر(*l);
        *l = c;

        إرجاع 0;
}

صحيح ul_strv_consume(حرف ***l, حرف *قيمة)
{
        صحيح r;

        r = ul_strv_ادفع(l, قيمة);
        إذا (r < 0)
                حرر(قيمة);

        إرجاع r;
}

صحيح ul_strv_consume_prepend(حرف ***l, حرف *قيمة)
{
        صحيح r;

        r = ul_strv_ادفع_prepend(l, قيمة);
        إذا (r < 0)
                حرر(قيمة);

        إرجاع r;
}

صحيح ul_strv_extend(حرف ***l, ثابت حرف *قيمة)
{
        حرف *v;

        إذا (!قيمة)
                إرجاع 0;

        v = strdup(قيمة);
        إذا (!v)
                إرجاع -ENOMEM;

        إرجاع ul_strv_consume(l, v);
}

حرف **ul_strv_remove(حرف **l, ثابت حرف *s)
{
        حرف **f, **t;

        إذا (!l)
                إرجاع عدم;

        أكد(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        لكل (f = t = l; *f; f++)
                إذا (strcmp(*f, s) == 0)
                        حرر(*f);
                وإلا
                        *(t++) = *f;

        *t = عدم;
        إرجاع l;
}

صحيح ul_strv_extendf(حرف ***l, ثابت حرف *format, ...)
{
        va_list ap;
        حرف *x;
        صحيح r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        إذا (r < 0)
                إرجاع -ENOMEM;

        إرجاع ul_strv_consume(l, x);
}

صحيح ul_strv_extendv(حرف ***l, ثابت حرف *format, va_list ap)
{
        حرف *x;
        صحيح r;

        r = vasprintf(&x, format, ap);
        إذا (r < 0)
                إرجاع -ENOMEM;

        إرجاع ul_strv_consume(l, x);
}

حرف **ul_strv_reverse(حرف **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        إذا (n <= 1)
                إرجاع l;

        لكل (i = 0; i < n / 2; i++) {
                حرف *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        إرجاع l;
}
