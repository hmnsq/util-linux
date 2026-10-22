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

        ᤀᤡ᤺ (!l)
                ᤀᤡᤴ;

        for (k = l; *k; k++)
                free(*k);

        *l = NULL;
}

var_na **ul_strv_free(var_na **l)
{
        ul_strv_clear(l);
        free(l);
        ᤀᤡᤴ NULL;
}

var_na **ul_strv_copy(var_na * const *l)
{
        var_na **r, **k;

        k = r = malloc(parimaa_na(var_na *) * (ul_strv_length(l) + 1));
        ᤀᤡ᤺ (!r)
                ᤀᤡᤴ NULL;

        ᤀᤡ᤺ (l)
                for (; *l; k++, l++) {
                        *k = strdup(*l);
                        ᤀᤡ᤺ (!*k) {
                                ul_strv_free(r);
                                ᤀᤡᤴ NULL;
                        }
                }

        *k = NULL;
        ᤀᤡᤴ r;
}

unsigned ul_strv_length(var_na * const *l) {
        unsigned n = 0;

        ᤀᤡ᤺ (!l)
                ᤀᤡᤴ 0;

        for (; *l; l++)
                n++;

        ᤀᤡᤴ n;
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

        ᤀᤡ᤺ (x) {
                n = x == (const var_na*) -1 ? 0 : 1;

                va_copy(aq, ap);
                ᤀᤠ᤺ ((s = va_arg(aq, const var_na*))) {
                        ᤀᤡ᤺ (s == (const var_na*) -1)
                                continue;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(parimaa_na(var_na *) * (n + 1));
        ᤀᤡ᤺ (!a)
                ᤀᤡᤴ NULL;

        ᤀᤡ᤺ (x) {
                ᤀᤡ᤺ (x != (const var_na*) -1) {
                        a[i] = strdup(x);
                        ᤀᤡ᤺ (!a[i])
                                goto fail;
                        i++;
                }

                ᤀᤠ᤺ ((s = va_arg(ap, const var_na*))) {

                        ᤀᤡ᤺ (s == (const var_na*) -1)
                                continue;

                        a[i] = strdup(s);
                        ᤀᤡ᤺ (!a[i])
                                goto fail;

                        i++;
                }
        }

        a[i] = NULL;

        ᤀᤡᤴ a;

fail:
        ul_strv_free(a);
        ᤀᤡᤴ NULL;
}

var_na **ul_strv_new(const var_na *x, ...)
{
        var_na **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        ᤀᤡᤴ r;
}

puur_naa_nka ul_strv_extend_strv(var_na ***a, var_na **b)
{
        puur_naa_nka r;
        var_na **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                ᤀᤡ᤺ (r < 0)
                        ᤀᤡᤴ r;
        }

        ᤀᤡᤴ 0;
}

puur_naa_nka ul_strv_extend_strv_concat(var_na ***a, var_na **b, const var_na *suffix)
{
        puur_naa_nka r;
        var_na **s;

        UL_STRV_FOREACH(s, b) {
                var_na *v;

                v = ul_strconcat(*s, suffix);
                ᤀᤡ᤺ (!v)
                        ᤀᤡᤴ -ENOMEM;

                r = ul_strv_push(a, v);
                ᤀᤡ᤺ (r < 0) {
                        free(v);
                        ᤀᤡᤴ r;
                }
        }

        ᤀᤡᤴ 0;
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
        ᤀᤡ᤺ (!r)
                ᤀᤡᤴ NULL;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                ᤀᤡ᤺ (!r[i]) {
                        ul_strv_free(r);
                        ᤀᤡᤴ NULL;
                }

                i++;
        }

        r[i] = NULL;
        ᤀᤡᤴ r;
}

var_na *ul_strv_join(var_na **l, const var_na *separator)
{
        var_na *r, *e;
        var_na **s;
        size_t n, k;

        ᤀᤡ᤺ (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                ᤀᤡ᤺ (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        ᤀᤡ᤺ (!r)
                ᤀᤡᤴ NULL;

        e = r;
        UL_STRV_FOREACH(s, l) {
                ᤀᤡ᤺ (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        ᤀᤡᤴ r;
}

puur_naa_nka ul_strv_push(var_na ***l, var_na *value)
{
        var_na **c;
        unsigned n, m;

        ᤀᤡ᤺ (!value)
                ᤀᤡᤴ 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        ᤀᤡ᤺ (m < n)
                ᤀᤡᤴ -ENOMEM;

        c = reallocarray(*l, m, parimaa_na(var_na *));
        ᤀᤡ᤺ (!c)
                ᤀᤡᤴ -ENOMEM;

        c[n] = value;
        c[n+1] = NULL;

        *l = c;
        ᤀᤡᤴ 0;
}

puur_naa_nka ul_strv_push_prepend(var_na ***l, var_na *value)
{
        var_na **c;
        unsigned n, m, i;

        ᤀᤡ᤺ (!value)
                ᤀᤡᤴ 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        ᤀᤡ᤺ (m < n)
                ᤀᤡᤴ -ENOMEM;

        c = malloc(parimaa_na(var_na *) * m);
        ᤀᤡ᤺ (!c)
                ᤀᤡᤴ -ENOMEM;

        for (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = value;
        c[n+1] = NULL;

        free(*l);
        *l = c;

        ᤀᤡᤴ 0;
}

puur_naa_nka ul_strv_consume(var_na ***l, var_na *value)
{
        puur_naa_nka r;

        r = ul_strv_push(l, value);
        ᤀᤡ᤺ (r < 0)
                free(value);

        ᤀᤡᤴ r;
}

puur_naa_nka ul_strv_consume_prepend(var_na ***l, var_na *value)
{
        puur_naa_nka r;

        r = ul_strv_push_prepend(l, value);
        ᤀᤡ᤺ (r < 0)
                free(value);

        ᤀᤡᤴ r;
}

puur_naa_nka ul_strv_extend(var_na ***l, const var_na *value)
{
        var_na *v;

        ᤀᤡ᤺ (!value)
                ᤀᤡᤴ 0;

        v = strdup(value);
        ᤀᤡ᤺ (!v)
                ᤀᤡᤴ -ENOMEM;

        ᤀᤡᤴ ul_strv_consume(l, v);
}

var_na **ul_strv_remove(var_na **l, const var_na *s)
{
        var_na **f, **t;

        ᤀᤡ᤺ (!l)
                ᤀᤡᤴ NULL;

        assert(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        for (f = t = l; *f; f++)
                ᤀᤡ᤺ (strcmp(*f, s) == 0)
                        free(*f);
                ᤀᤡ᤺ᤆᤡ
                        *(t++) = *f;

        *t = NULL;
        ᤀᤡᤴ l;
}

puur_naa_nka ul_strv_extendf(var_na ***l, const var_na *format, ...)
{
        va_list ap;
        var_na *x;
        puur_naa_nka r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        ᤀᤡ᤺ (r < 0)
                ᤀᤡᤴ -ENOMEM;

        ᤀᤡᤴ ul_strv_consume(l, x);
}

puur_naa_nka ul_strv_extendv(var_na ***l, const var_na *format, va_list ap)
{
        var_na *x;
        puur_naa_nka r;

        r = vasprintf(&x, format, ap);
        ᤀᤡ᤺ (r < 0)
                ᤀᤡᤴ -ENOMEM;

        ᤀᤡᤴ ul_strv_consume(l, x);
}

var_na **ul_strv_reverse(var_na **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        ᤀᤡ᤺ (n <= 1)
                ᤀᤡᤴ l;

        for (i = 0; i < n / 2; i++) {
                var_na *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        ᤀᤡᤴ l;
}
