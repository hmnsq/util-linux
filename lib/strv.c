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

#شامل <stdlib.h>
#شامل <stdarg.h>
#شامل <string.h>
#شامل <errno.h>
#شامل <stdbool.h>
#شامل <assert.h>

#شامل "strutils.h"
#شامل "strv.h"

ساکن خالی ul_strv_clear(حرف **l)
{
        حرف **k;

        اگر (!l)
                واپس;

        برائے (k = l; *k; k++)
                آزاد(*k);

        *l = تہی;
}

حرف **ul_strv_free(حرف **l)
{
        ul_strv_clear(l);
        آزاد(l);
        واپس تہی;
}

حرف **ul_strv_copy(حرف * ثابت *l)
{
        حرف **r, **k;

        k = r = حافظہ_دو(حجم(حرف *) * (ul_strv_لمبائی(l) + 1));
        اگر (!r)
                واپس تہی;

        اگر (l)
                برائے (; *l; k++, l++) {
                        *k = strdup(*l);
                        اگر (!*k) {
                                ul_strv_free(r);
                                واپس تہی;
                        }
                }

        *k = تہی;
        واپس r;
}

غیرعلامتی ul_strv_لمبائی(حرف * ثابت *l) {
        غیرعلامتی n = 0;

        اگر (!l)
                واپس 0;

        برائے (; *l; l++)
                n++;

        واپس n;
}

ساکن حرف **ul_strv_new_ap(ثابت حرف *x, va_list ap)
{
        ثابت حرف *s;
        حرف **a;
        غیرعلامتی n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        اگر (x) {
                n = x == (ثابت حرف*) -1 ? 0 : 1;

                va_copy(aq, ap);
                جب ((s = va_arg(aq, ثابت حرف*))) {
                        اگر (s == (ثابت حرف*) -1)
                                جاری;

                        n++;
                }

                va_end(aq);
        }

        a = حافظہ_دو(حجم(حرف *) * (n + 1));
        اگر (!a)
                واپس تہی;

        اگر (x) {
                اگر (x != (ثابت حرف*) -1) {
                        a[i] = strdup(x);
                        اگر (!a[i])
                                جاؤ fail;
                        i++;
                }

                جب ((s = va_arg(ap, ثابت حرف*))) {

                        اگر (s == (ثابت حرف*) -1)
                                جاری;

                        a[i] = strdup(s);
                        اگر (!a[i])
                                جاؤ fail;

                        i++;
                }
        }

        a[i] = تہی;

        واپس a;

fail:
        ul_strv_free(a);
        واپس تہی;
}

حرف **ul_strv_new(ثابت حرف *x, ...)
{
        حرف **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        واپس r;
}

صحیح ul_strv_extend_strv(حرف ***a, حرف **b)
{
        صحیح r;
        حرف **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                اگر (r < 0)
                        واپس r;
        }

        واپس 0;
}

صحیح ul_strv_extend_strv_concat(حرف ***a, حرف **b, ثابت حرف *suffix)
{
        صحیح r;
        حرف **s;

        UL_STRV_FOREACH(s, b) {
                حرف *v;

                v = ul_strconcat(*s, suffix);
                اگر (!v)
                        واپس -ENOMEM;

                r = ul_strv_ڈالو(a, v);
                اگر (r < 0) {
                        آزاد(v);
                        واپس r;
                }
        }

        واپس 0;
}


#تعریف_کرو FOREACH_لفظ(لفظ, لمبائی, s, separator, quoted, state)        \
        برائے ((state) = (s), (لفظ) = ul_بانٹو(&(state), &(لمبائی), (separator), (quoted)); (لفظ); (لفظ) = ul_بانٹو(&(state), &(لمبائی), (separator), (quoted)))

#تعریف_کرو FOREACH_لفظ_SEPARATOR(لفظ, لمبائی, s, separator, state)       \
        FOREACH_لفظ(لفظ, لمبائی, s, separator, false, state)


حرف **ul_strv_بانٹو(ثابت حرف *s, ثابت حرف *separator)
{
        ثابت حرف *لفظ, *state;
        size_t l;
        غیرعلامتی n, i;
        حرف **r;

        أكد(s);

        n = 0;
        FOREACH_لفظ_SEPARATOR(لفظ, l, s, separator, state)
                n++;

        r = حافظہ_دو(حجم(حرف *) * (n + 1));
        اگر (!r)
                واپس تہی;

        i = 0;
        FOREACH_لفظ_SEPARATOR(لفظ, l, s, separator, state) {
                r[i] = strndup(لفظ, l);
                اگر (!r[i]) {
                        ul_strv_free(r);
                        واپس تہی;
                }

                i++;
        }

        r[i] = تہی;
        واپس r;
}

حرف *ul_strv_join(حرف **l, ثابت حرف *separator)
{
        حرف *r, *e;
        حرف **s;
        size_t n, k;

        اگر (!separator)
                separator = " ";

        k = لڑی_لمبائی(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                اگر (n != 0)
                        n += k;
                n += لڑی_لمبائی(*s);
        }

        r = حافظہ_دو(n + 1);
        اگر (!r)
                واپس تہی;

        e = r;
        UL_STRV_FOREACH(s, l) {
                اگر (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        واپس r;
}

صحیح ul_strv_ڈالو(حرف ***l, حرف *قدر)
{
        حرف **c;
        غیرعلامتی n, m;

        اگر (!قدر)
                واپس 0;

        n = ul_strv_لمبائی(*l);

        /* Increase and check for overflow */
        m = n + 2;
        اگر (m < n)
                واپس -ENOMEM;

        c = reallocarray(*l, m, حجم(حرف *));
        اگر (!c)
                واپس -ENOMEM;

        c[n] = قدر;
        c[n+1] = تہی;

        *l = c;
        واپس 0;
}

صحیح ul_strv_ڈالو_prepend(حرف ***l, حرف *قدر)
{
        حرف **c;
        غیرعلامتی n, m, i;

        اگر (!قدر)
                واپس 0;

        n = ul_strv_لمبائی(*l);

        /* increase and check for overflow */
        m = n + 2;
        اگر (m < n)
                واپس -ENOMEM;

        c = حافظہ_دو(حجم(حرف *) * m);
        اگر (!c)
                واپس -ENOMEM;

        برائے (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = قدر;
        c[n+1] = تہی;

        آزاد(*l);
        *l = c;

        واپس 0;
}

صحیح ul_strv_consume(حرف ***l, حرف *قدر)
{
        صحیح r;

        r = ul_strv_ڈالو(l, قدر);
        اگر (r < 0)
                آزاد(قدر);

        واپس r;
}

صحیح ul_strv_consume_prepend(حرف ***l, حرف *قدر)
{
        صحیح r;

        r = ul_strv_ڈالو_prepend(l, قدر);
        اگر (r < 0)
                آزاد(قدر);

        واپس r;
}

صحیح ul_strv_extend(حرف ***l, ثابت حرف *قدر)
{
        حرف *v;

        اگر (!قدر)
                واپس 0;

        v = strdup(قدر);
        اگر (!v)
                واپس -ENOMEM;

        واپس ul_strv_consume(l, v);
}

حرف **ul_strv_remove(حرف **l, ثابت حرف *s)
{
        حرف **f, **t;

        اگر (!l)
                واپس تہی;

        أكد(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        برائے (f = t = l; *f; f++)
                اگر (strcmp(*f, s) == 0)
                        آزاد(*f);
                ورنہ
                        *(t++) = *f;

        *t = تہی;
        واپس l;
}

صحیح ul_strv_extendf(حرف ***l, ثابت حرف *format, ...)
{
        va_list ap;
        حرف *x;
        صحیح r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        اگر (r < 0)
                واپس -ENOMEM;

        واپس ul_strv_consume(l, x);
}

صحیح ul_strv_extendv(حرف ***l, ثابت حرف *format, va_list ap)
{
        حرف *x;
        صحیح r;

        r = vasprintf(&x, format, ap);
        اگر (r < 0)
                واپس -ENOMEM;

        واپس ul_strv_consume(l, x);
}

حرف **ul_strv_الٹ(حرف **l)
{
        غیرعلامتی n, i;

        n = ul_strv_لمبائی(l);
        اگر (n <= 1)
                واپس l;

        برائے (i = 0; i < n / 2; i++) {
                حرف *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        واپس l;
}
