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
                أرجع;

        لكل (k = l; *k; k++)
                حرر(*k);

        *l = عدم;
}

حرف **ul_strv_free(حرف **l)
{
        ul_strv_clear(l);
        حرر(l);
        أرجع عدم;
}

حرف **ul_strv_copy(حرف * ثابت *l)
{
        حرف **r, **k;

        k = r = حافظہ_دیو(حجم(حرف *) * (ul_strv_لمائی(l) + 1));
        إذا (!r)
                أرجع عدم;

        إذا (l)
                لكل (; *l; k++, l++) {
                        *k = strdup(*l);
                        إذا (!*k) {
                                ul_strv_free(r);
                                أرجع عدم;
                        }
                }

        *k = عدم;
        أرجع r;
}

بنانشانی ul_strv_لمائی(حرف * ثابت *l) {
        بنانشانی n = 0;

        إذا (!l)
                أرجع 0;

        لكل (; *l; l++)
                n++;

        أرجع n;
}

ساكن حرف **ul_strv_new_ap(ثابت حرف *x, va_list ap)
{
        ثابت حرف *s;
        حرف **a;
        بنانشانی n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        إذا (x) {
                n = x == (ثابت حرف*) -1 ? 0 : 1;

                va_copy(aq, ap);
                طالما ((s = va_arg(aq, ثابت حرف*))) {
                        إذا (s == (ثابت حرف*) -1)
                                استمر;

                        n++;
                }

                va_end(aq);
        }

        a = حافظہ_دیو(حجم(حرف *) * (n + 1));
        إذا (!a)
                أرجع عدم;

        إذا (x) {
                إذا (x != (ثابت حرف*) -1) {
                        a[i] = strdup(x);
                        إذا (!a[i])
                                اذهب fail;
                        i++;
                }

                طالما ((s = va_arg(ap, ثابت حرف*))) {

                        إذا (s == (ثابت حرف*) -1)
                                استمر;

                        a[i] = strdup(s);
                        إذا (!a[i])
                                اذهب fail;

                        i++;
                }
        }

        a[i] = عدم;

        أرجع a;

fail:
        ul_strv_free(a);
        أرجع عدم;
}

حرف **ul_strv_new(ثابت حرف *x, ...)
{
        حرف **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        أرجع r;
}

صحيح ul_strv_extend_strv(حرف ***a, حرف **b)
{
        صحيح r;
        حرف **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                إذا (r < 0)
                        أرجع r;
        }

        أرجع 0;
}

صحيح ul_strv_extend_strv_concat(حرف ***a, حرف **b, ثابت حرف *suffix)
{
        صحيح r;
        حرف **s;

        UL_STRV_FOREACH(s, b) {
                حرف *v;

                v = ul_strconcat(*s, suffix);
                إذا (!v)
                        أرجع -ENOMEM;

                r = ul_strv_ڈالو(a, v);
                إذا (r < 0) {
                        حرر(v);
                        أرجع r;
                }
        }

        أرجع 0;
}


#تعریف_کرو FOREACH_لفظ(لفظ, لمائی, s, separator, quoted, state)        \
        لكل ((state) = (s), (لفظ) = ul_ونڈو(&(state), &(لمائی), (separator), (quoted)); (لفظ); (لفظ) = ul_ونڈو(&(state), &(لمائی), (separator), (quoted)))

#تعریف_کرو FOREACH_لفظ_SEPARATOR(لفظ, لمائی, s, separator, state)       \
        FOREACH_لفظ(لفظ, لمائی, s, separator, false, state)


حرف **ul_strv_ونڈو(ثابت حرف *s, ثابت حرف *separator)
{
        ثابت حرف *لفظ, *state;
        size_t l;
        بنانشانی n, i;
        حرف **r;

        أكد(s);

        n = 0;
        FOREACH_لفظ_SEPARATOR(لفظ, l, s, separator, state)
                n++;

        r = حافظہ_دیو(حجم(حرف *) * (n + 1));
        إذا (!r)
                أرجع عدم;

        i = 0;
        FOREACH_لفظ_SEPARATOR(لفظ, l, s, separator, state) {
                r[i] = strndup(لفظ, l);
                إذا (!r[i]) {
                        ul_strv_free(r);
                        أرجع عدم;
                }

                i++;
        }

        r[i] = عدم;
        أرجع r;
}

حرف *ul_strv_join(حرف **l, ثابت حرف *separator)
{
        حرف *r, *e;
        حرف **s;
        size_t n, k;

        إذا (!separator)
                separator = " ";

        k = لڑی_لمبائی(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                إذا (n != 0)
                        n += k;
                n += لڑی_لمبائی(*s);
        }

        r = حافظہ_دیو(n + 1);
        إذا (!r)
                أرجع عدم;

        e = r;
        UL_STRV_FOREACH(s, l) {
                إذا (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        أرجع r;
}

صحيح ul_strv_ڈالو(حرف ***l, حرف *قدر)
{
        حرف **c;
        بنانشانی n, m;

        إذا (!قدر)
                أرجع 0;

        n = ul_strv_لمائی(*l);

        /* Increase and check for overflow */
        m = n + 2;
        إذا (m < n)
                أرجع -ENOMEM;

        c = reallocarray(*l, m, حجم(حرف *));
        إذا (!c)
                أرجع -ENOMEM;

        c[n] = قدر;
        c[n+1] = عدم;

        *l = c;
        أرجع 0;
}

صحيح ul_strv_ڈالو_prepend(حرف ***l, حرف *قدر)
{
        حرف **c;
        بنانشانی n, m, i;

        إذا (!قدر)
                أرجع 0;

        n = ul_strv_لمائی(*l);

        /* increase and check for overflow */
        m = n + 2;
        إذا (m < n)
                أرجع -ENOMEM;

        c = حافظہ_دیو(حجم(حرف *) * m);
        إذا (!c)
                أرجع -ENOMEM;

        لكل (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = قدر;
        c[n+1] = عدم;

        حرر(*l);
        *l = c;

        أرجع 0;
}

صحيح ul_strv_consume(حرف ***l, حرف *قدر)
{
        صحيح r;

        r = ul_strv_ڈالو(l, قدر);
        إذا (r < 0)
                حرر(قدر);

        أرجع r;
}

صحيح ul_strv_consume_prepend(حرف ***l, حرف *قدر)
{
        صحيح r;

        r = ul_strv_ڈالو_prepend(l, قدر);
        إذا (r < 0)
                حرر(قدر);

        أرجع r;
}

صحيح ul_strv_extend(حرف ***l, ثابت حرف *قدر)
{
        حرف *v;

        إذا (!قدر)
                أرجع 0;

        v = strdup(قدر);
        إذا (!v)
                أرجع -ENOMEM;

        أرجع ul_strv_consume(l, v);
}

حرف **ul_strv_remove(حرف **l, ثابت حرف *s)
{
        حرف **f, **t;

        إذا (!l)
                أرجع عدم;

        أكد(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        لكل (f = t = l; *f; f++)
                إذا (strcmp(*f, s) == 0)
                        حرر(*f);
                وإلا
                        *(t++) = *f;

        *t = عدم;
        أرجع l;
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
                أرجع -ENOMEM;

        أرجع ul_strv_consume(l, x);
}

صحيح ul_strv_extendv(حرف ***l, ثابت حرف *format, va_list ap)
{
        حرف *x;
        صحيح r;

        r = vasprintf(&x, format, ap);
        إذا (r < 0)
                أرجع -ENOMEM;

        أرجع ul_strv_consume(l, x);
}

حرف **ul_strv_الٹ(حرف **l)
{
        بنانشانی n, i;

        n = ul_strv_لمائی(l);
        إذا (n <= 1)
                أرجع l;

        لكل (i = 0; i < n / 2; i++) {
                حرف *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        أرجع l;
}
