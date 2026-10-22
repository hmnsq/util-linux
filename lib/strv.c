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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "strutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "strv.h"

ਸਥਿਰ ਖਾਲੀ ul_strv_clear(ਅੱਖਰ **l)
{
        ਅੱਖਰ **k;

        ਜੇ (!l)
                ਵਾਪਸ;

        ਲਈ (k = l; *k; k++)
                ਮੁਕਤ(*k);

        *l = ਨਿਰਾ;
}

ਅੱਖਰ **ul_strv_free(ਅੱਖਰ **l)
{
        ul_strv_clear(l);
        ਮੁਕਤ(l);
        ਵਾਪਸ ਨਿਰਾ;
}

ਅੱਖਰ **ul_strv_copy(ਅੱਖਰ * ਅਚਲ *l)
{
        ਅੱਖਰ **r, **k;

        k = r = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਅੱਖਰ *) * (ul_strv_ਲੰਮਾਈ(l) + 1));
        ਜੇ (!r)
                ਵਾਪਸ ਨਿਰਾ;

        ਜੇ (l)
                ਲਈ (; *l; k++, l++) {
                        *k = strdup(*l);
                        ਜੇ (!*k) {
                                ul_strv_free(r);
                                ਵਾਪਸ ਨਿਰਾ;
                        }
                }

        *k = ਨਿਰਾ;
        ਵਾਪਸ r;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ul_strv_ਲੰਮਾਈ(ਅੱਖਰ * ਅਚਲ *l) {
        ਬਿਨਾਨਿਸ਼ਾਨੀ n = 0;

        ਜੇ (!l)
                ਵਾਪਸ 0;

        ਲਈ (; *l; l++)
                n++;

        ਵਾਪਸ n;
}

ਸਥਿਰ ਅੱਖਰ **ul_strv_new_ap(ਅਚਲ ਅੱਖਰ *x, va_list ap)
{
        ਅਚਲ ਅੱਖਰ *s;
        ਅੱਖਰ **a;
        ਬਿਨਾਨਿਸ਼ਾਨੀ n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        ਜੇ (x) {
                n = x == (ਅਚਲ ਅੱਖਰ*) -1 ? 0 : 1;

                va_copy(aq, ap);
                ਜਦੋਂ ((s = va_arg(aq, ਅਚਲ ਅੱਖਰ*))) {
                        ਜੇ (s == (ਅਚਲ ਅੱਖਰ*) -1)
                                ਜਾਰੀ;

                        n++;
                }

                va_end(aq);
        }

        a = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਅੱਖਰ *) * (n + 1));
        ਜੇ (!a)
                ਵਾਪਸ ਨਿਰਾ;

        ਜੇ (x) {
                ਜੇ (x != (ਅਚਲ ਅੱਖਰ*) -1) {
                        a[i] = strdup(x);
                        ਜੇ (!a[i])
                                ਜਾਓ fail;
                        i++;
                }

                ਜਦੋਂ ((s = va_arg(ap, ਅਚਲ ਅੱਖਰ*))) {

                        ਜੇ (s == (ਅਚਲ ਅੱਖਰ*) -1)
                                ਜਾਰੀ;

                        a[i] = strdup(s);
                        ਜੇ (!a[i])
                                ਜਾਓ fail;

                        i++;
                }
        }

        a[i] = ਨਿਰਾ;

        ਵਾਪਸ a;

fail:
        ul_strv_free(a);
        ਵਾਪਸ ਨਿਰਾ;
}

ਅੱਖਰ **ul_strv_new(ਅਚਲ ਅੱਖਰ *x, ...)
{
        ਅੱਖਰ **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        ਵਾਪਸ r;
}

ਪੂਰਨ ul_strv_extend_strv(ਅੱਖਰ ***a, ਅੱਖਰ **b)
{
        ਪੂਰਨ r;
        ਅੱਖਰ **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                ਜੇ (r < 0)
                        ਵਾਪਸ r;
        }

        ਵਾਪਸ 0;
}

ਪੂਰਨ ul_strv_extend_strv_concat(ਅੱਖਰ ***a, ਅੱਖਰ **b, ਅਚਲ ਅੱਖਰ *suffix)
{
        ਪੂਰਨ r;
        ਅੱਖਰ **s;

        UL_STRV_FOREACH(s, b) {
                ਅੱਖਰ *v;

                v = ul_strconcat(*s, suffix);
                ਜੇ (!v)
                        ਵਾਪਸ -ENOMEM;

                r = ul_strv_ਧੱਕੋ(a, v);
                ਜੇ (r < 0) {
                        ਮੁਕਤ(v);
                        ਵਾਪਸ r;
                }
        }

        ਵਾਪਸ 0;
}


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FOREACH_ਸ਼ਬਦ(ਸ਼ਬਦ, ਲੰਮਾਈ, s, separator, quoted, state)        \
        ਲਈ ((state) = (s), (ਸ਼ਬਦ) = ul_ਵੰਡੋ(&(state), &(ਲੰਮਾਈ), (separator), (quoted)); (ਸ਼ਬਦ); (ਸ਼ਬਦ) = ul_ਵੰਡੋ(&(state), &(ਲੰਮਾਈ), (separator), (quoted)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FOREACH_ਸ਼ਬਦ_SEPARATOR(ਸ਼ਬਦ, ਲੰਮਾਈ, s, separator, state)       \
        FOREACH_ਸ਼ਬਦ(ਸ਼ਬਦ, ਲੰਮਾਈ, s, separator, false, state)


ਅੱਖਰ **ul_strv_ਵੰਡੋ(ਅਚਲ ਅੱਖਰ *s, ਅਚਲ ਅੱਖਰ *separator)
{
        ਅਚਲ ਅੱਖਰ *ਸ਼ਬਦ, *state;
        size_t l;
        ਬਿਨਾਨਿਸ਼ਾਨੀ n, i;
        ਅੱਖਰ **r;

        ਪ੍ਰਤਿਜ੍ਞਾ(s);

        n = 0;
        FOREACH_ਸ਼ਬਦ_SEPARATOR(ਸ਼ਬਦ, l, s, separator, state)
                n++;

        r = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਅੱਖਰ *) * (n + 1));
        ਜੇ (!r)
                ਵਾਪਸ ਨਿਰਾ;

        i = 0;
        FOREACH_ਸ਼ਬਦ_SEPARATOR(ਸ਼ਬਦ, l, s, separator, state) {
                r[i] = strndup(ਸ਼ਬਦ, l);
                ਜੇ (!r[i]) {
                        ul_strv_free(r);
                        ਵਾਪਸ ਨਿਰਾ;
                }

                i++;
        }

        r[i] = ਨਿਰਾ;
        ਵਾਪਸ r;
}

ਅੱਖਰ *ul_strv_join(ਅੱਖਰ **l, ਅਚਲ ਅੱਖਰ *separator)
{
        ਅੱਖਰ *r, *e;
        ਅੱਖਰ **s;
        size_t n, k;

        ਜੇ (!separator)
                separator = " ";

        k = ਲੜੀ_ਲੰਬਾਈ(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                ਜੇ (n != 0)
                        n += k;
                n += ਲੜੀ_ਲੰਬਾਈ(*s);
        }

        r = ਸਮ੍ਰਿਤੀ_ਦਿਓ(n + 1);
        ਜੇ (!r)
                ਵਾਪਸ ਨਿਰਾ;

        e = r;
        UL_STRV_FOREACH(s, l) {
                ਜੇ (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        ਵਾਪਸ r;
}

ਪੂਰਨ ul_strv_ਧੱਕੋ(ਅੱਖਰ ***l, ਅੱਖਰ *ਮੁੱਲ)
{
        ਅੱਖਰ **c;
        ਬਿਨਾਨਿਸ਼ਾਨੀ n, m;

        ਜੇ (!ਮੁੱਲ)
                ਵਾਪਸ 0;

        n = ul_strv_ਲੰਮਾਈ(*l);

        /* Increase and check for overflow */
        m = n + 2;
        ਜੇ (m < n)
                ਵਾਪਸ -ENOMEM;

        c = reallocarray(*l, m, ਆਕਾਰ(ਅੱਖਰ *));
        ਜੇ (!c)
                ਵਾਪਸ -ENOMEM;

        c[n] = ਮੁੱਲ;
        c[n+1] = ਨਿਰਾ;

        *l = c;
        ਵਾਪਸ 0;
}

ਪੂਰਨ ul_strv_ਧੱਕੋ_prepend(ਅੱਖਰ ***l, ਅੱਖਰ *ਮੁੱਲ)
{
        ਅੱਖਰ **c;
        ਬਿਨਾਨਿਸ਼ਾਨੀ n, m, i;

        ਜੇ (!ਮੁੱਲ)
                ਵਾਪਸ 0;

        n = ul_strv_ਲੰਮਾਈ(*l);

        /* increase and check for overflow */
        m = n + 2;
        ਜੇ (m < n)
                ਵਾਪਸ -ENOMEM;

        c = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਅੱਖਰ *) * m);
        ਜੇ (!c)
                ਵਾਪਸ -ENOMEM;

        ਲਈ (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = ਮੁੱਲ;
        c[n+1] = ਨਿਰਾ;

        ਮੁਕਤ(*l);
        *l = c;

        ਵਾਪਸ 0;
}

ਪੂਰਨ ul_strv_consume(ਅੱਖਰ ***l, ਅੱਖਰ *ਮੁੱਲ)
{
        ਪੂਰਨ r;

        r = ul_strv_ਧੱਕੋ(l, ਮੁੱਲ);
        ਜੇ (r < 0)
                ਮੁਕਤ(ਮੁੱਲ);

        ਵਾਪਸ r;
}

ਪੂਰਨ ul_strv_consume_prepend(ਅੱਖਰ ***l, ਅੱਖਰ *ਮੁੱਲ)
{
        ਪੂਰਨ r;

        r = ul_strv_ਧੱਕੋ_prepend(l, ਮੁੱਲ);
        ਜੇ (r < 0)
                ਮੁਕਤ(ਮੁੱਲ);

        ਵਾਪਸ r;
}

ਪੂਰਨ ul_strv_extend(ਅੱਖਰ ***l, ਅਚਲ ਅੱਖਰ *ਮੁੱਲ)
{
        ਅੱਖਰ *v;

        ਜੇ (!ਮੁੱਲ)
                ਵਾਪਸ 0;

        v = strdup(ਮੁੱਲ);
        ਜੇ (!v)
                ਵਾਪਸ -ENOMEM;

        ਵਾਪਸ ul_strv_consume(l, v);
}

ਅੱਖਰ **ul_strv_remove(ਅੱਖਰ **l, ਅਚਲ ਅੱਖਰ *s)
{
        ਅੱਖਰ **f, **t;

        ਜੇ (!l)
                ਵਾਪਸ ਨਿਰਾ;

        ਪ੍ਰਤਿਜ੍ਞਾ(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        ਲਈ (f = t = l; *f; f++)
                ਜੇ (strcmp(*f, s) == 0)
                        ਮੁਕਤ(*f);
                ਨਹੀਂਤਾਂ
                        *(t++) = *f;

        *t = ਨਿਰਾ;
        ਵਾਪਸ l;
}

ਪੂਰਨ ul_strv_extendf(ਅੱਖਰ ***l, ਅਚਲ ਅੱਖਰ *format, ...)
{
        va_list ap;
        ਅੱਖਰ *x;
        ਪੂਰਨ r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        ਜੇ (r < 0)
                ਵਾਪਸ -ENOMEM;

        ਵਾਪਸ ul_strv_consume(l, x);
}

ਪੂਰਨ ul_strv_extendv(ਅੱਖਰ ***l, ਅਚਲ ਅੱਖਰ *format, va_list ap)
{
        ਅੱਖਰ *x;
        ਪੂਰਨ r;

        r = vasprintf(&x, format, ap);
        ਜੇ (r < 0)
                ਵਾਪਸ -ENOMEM;

        ਵਾਪਸ ul_strv_consume(l, x);
}

ਅੱਖਰ **ul_strv_ਉਲਟ(ਅੱਖਰ **l)
{
        ਬਿਨਾਨਿਸ਼ਾਨੀ n, i;

        n = ul_strv_ਲੰਮਾਈ(l);
        ਜੇ (n <= 1)
                ਵਾਪਸ l;

        ਲਈ (i = 0; i < n / 2; i++) {
                ਅੱਖਰ *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        ਵਾਪਸ l;
}
