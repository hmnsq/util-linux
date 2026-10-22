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

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>

#include "strutils.h"
#include "strv.h"

static void ul_strv_clear(var_na **l)
{
        var_na **k;

        ᯀᯮᯝ (!l)
                ᯀᯮᯞ;

        for (k = l; *k; k++)
                free(*k);

        *l = NULL;
}

var_na **ul_strv_free(var_na **l)
{
        ul_strv_clear(l);
        free(l);
        ᯀᯮᯞ NULL;
}

var_na **ul_strv_copy(var_na * const *l)
{
        var_na **r, **k;

        k = r = malloc(parimaa_na(var_na *) * (ul_strv_length(l) + 1));
        ᯀᯮᯝ (!r)
                ᯀᯮᯞ NULL;

        ᯀᯮᯝ (l)
                for (; *l; k++, l++) {
                        *k = strdup(*l);
                        ᯀᯮᯝ (!*k) {
                                ul_strv_free(r);
                                ᯀᯮᯞ NULL;
                        }
                }

        *k = NULL;
        ᯀᯮᯞ r;
}

unsigned ul_strv_length(var_na * const *l) {
        unsigned n = 0;

        ᯀᯮᯝ (!l)
                ᯀᯮᯞ 0;

        for (; *l; l++)
                n++;

        ᯀᯮᯞ n;
}

static var_na **ul_strv_new_ap(const var_na *x, va_list ap)
{
        const var_na *s;
        var_na **a;
        unsigned n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        ᯀᯮᯝ (x) {
                n = x == (const var_na*) -1 ? 0 : 1;

                va_copy(aq, ap);
                ᯀᯬᯝ ((s = va_arg(aq, const var_na*))) {
                        ᯀᯮᯝ (s == (const var_na*) -1)
                                continue;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(parimaa_na(var_na *) * (n + 1));
        ᯀᯮᯝ (!a)
                ᯀᯮᯞ NULL;

        ᯀᯮᯝ (x) {
                ᯀᯮᯝ (x != (const var_na*) -1) {
                        a[i] = strdup(x);
                        ᯀᯮᯝ (!a[i])
                                goto fail;
                        i++;
                }

                ᯀᯬᯝ ((s = va_arg(ap, const var_na*))) {

                        ᯀᯮᯝ (s == (const var_na*) -1)
                                continue;

                        a[i] = strdup(s);
                        ᯀᯮᯝ (!a[i])
                                goto fail;

                        i++;
                }
        }

        a[i] = NULL;

        ᯀᯮᯞ a;

fail:
        ul_strv_free(a);
        ᯀᯮᯞ NULL;
}

var_na **ul_strv_new(const var_na *x, ...)
{
        var_na **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        ᯀᯮᯞ r;
}

puur_naa_nka ul_strv_extend_strv(var_na ***a, var_na **b)
{
        puur_naa_nka r;
        var_na **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                ᯀᯮᯝ (r < 0)
                        ᯀᯮᯞ r;
        }

        ᯀᯮᯞ 0;
}

puur_naa_nka ul_strv_extend_strv_concat(var_na ***a, var_na **b, const var_na *suffix)
{
        puur_naa_nka r;
        var_na **s;

        UL_STRV_FOREACH(s, b) {
                var_na *v;

                v = ul_strconcat(*s, suffix);
                ᯀᯮᯝ (!v)
                        ᯀᯮᯞ -ENOMEM;

                r = ul_strv_push(a, v);
                ᯀᯮᯝ (r < 0) {
                        free(v);
                        ᯀᯮᯞ r;
                }
        }

        ᯀᯮᯞ 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        for ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

#define FOREACH_WORD_SEPARATOR(word, length, s, separator, state)       \
        _FOREACH_WORD(word, length, s, separator, false, state)


var_na **ul_strv_split(const var_na *s, const var_na *separator)
{
        const var_na *word, *state;
        size_t l;
        unsigned n, i;
        var_na **r;

        assert(s);

        n = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state)
                n++;

        r = malloc(parimaa_na(var_na *) * (n + 1));
        ᯀᯮᯝ (!r)
                ᯀᯮᯞ NULL;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                ᯀᯮᯝ (!r[i]) {
                        ul_strv_free(r);
                        ᯀᯮᯞ NULL;
                }

                i++;
        }

        r[i] = NULL;
        ᯀᯮᯞ r;
}

var_na *ul_strv_join(var_na **l, const var_na *separator)
{
        var_na *r, *e;
        var_na **s;
        size_t n, k;

        ᯀᯮᯝ (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                ᯀᯮᯝ (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        ᯀᯮᯝ (!r)
                ᯀᯮᯞ NULL;

        e = r;
        UL_STRV_FOREACH(s, l) {
                ᯀᯮᯝ (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        ᯀᯮᯞ r;
}

puur_naa_nka ul_strv_push(var_na ***l, var_na *value)
{
        var_na **c;
        unsigned n, m;

        ᯀᯮᯝ (!value)
                ᯀᯮᯞ 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        ᯀᯮᯝ (m < n)
                ᯀᯮᯞ -ENOMEM;

        c = reallocarray(*l, m, parimaa_na(var_na *));
        ᯀᯮᯝ (!c)
                ᯀᯮᯞ -ENOMEM;

        c[n] = value;
        c[n+1] = NULL;

        *l = c;
        ᯀᯮᯞ 0;
}

puur_naa_nka ul_strv_push_prepend(var_na ***l, var_na *value)
{
        var_na **c;
        unsigned n, m, i;

        ᯀᯮᯝ (!value)
                ᯀᯮᯞ 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        ᯀᯮᯝ (m < n)
                ᯀᯮᯞ -ENOMEM;

        c = malloc(parimaa_na(var_na *) * m);
        ᯀᯮᯝ (!c)
                ᯀᯮᯞ -ENOMEM;

        for (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = value;
        c[n+1] = NULL;

        free(*l);
        *l = c;

        ᯀᯮᯞ 0;
}

puur_naa_nka ul_strv_consume(var_na ***l, var_na *value)
{
        puur_naa_nka r;

        r = ul_strv_push(l, value);
        ᯀᯮᯝ (r < 0)
                free(value);

        ᯀᯮᯞ r;
}

puur_naa_nka ul_strv_consume_prepend(var_na ***l, var_na *value)
{
        puur_naa_nka r;

        r = ul_strv_push_prepend(l, value);
        ᯀᯮᯝ (r < 0)
                free(value);

        ᯀᯮᯞ r;
}

puur_naa_nka ul_strv_extend(var_na ***l, const var_na *value)
{
        var_na *v;

        ᯀᯮᯝ (!value)
                ᯀᯮᯞ 0;

        v = strdup(value);
        ᯀᯮᯝ (!v)
                ᯀᯮᯞ -ENOMEM;

        ᯀᯮᯞ ul_strv_consume(l, v);
}

var_na **ul_strv_remove(var_na **l, const var_na *s)
{
        var_na **f, **t;

        ᯀᯮᯝ (!l)
                ᯀᯮᯞ NULL;

        assert(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        for (f = t = l; *f; f++)
                ᯀᯮᯝ (strcmp(*f, s) == 0)
                        free(*f);
                ᯀᯮᯝᯂᯪ
                        *(t++) = *f;

        *t = NULL;
        ᯀᯮᯞ l;
}

puur_naa_nka ul_strv_extendf(var_na ***l, const var_na *format, ...)
{
        va_list ap;
        var_na *x;
        puur_naa_nka r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        ᯀᯮᯝ (r < 0)
                ᯀᯮᯞ -ENOMEM;

        ᯀᯮᯞ ul_strv_consume(l, x);
}

puur_naa_nka ul_strv_extendv(var_na ***l, const var_na *format, va_list ap)
{
        var_na *x;
        puur_naa_nka r;

        r = vasprintf(&x, format, ap);
        ᯀᯮᯝ (r < 0)
                ᯀᯮᯞ -ENOMEM;

        ᯀᯮᯞ ul_strv_consume(l, x);
}

var_na **ul_strv_reverse(var_na **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        ᯀᯮᯝ (n <= 1)
                ᯀᯮᯞ l;

        for (i = 0; i < n / 2; i++) {
                var_na *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        ᯀᯮᯞ l;
}
