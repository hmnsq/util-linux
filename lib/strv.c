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

        ᮚᮦᮔ᮪ (!l)
                ᮝᮜᮤ;

        for (k = l; *k; k++)
                free(*k);

        *l = NULL;
}

var_na **ul_strv_free(var_na **l)
{
        ul_strv_clear(l);
        free(l);
        ᮝᮜᮤ NULL;
}

var_na **ul_strv_copy(var_na * const *l)
{
        var_na **r, **k;

        k = r = malloc(parimaa_na(var_na *) * (ul_strv_length(l) + 1));
        ᮚᮦᮔ᮪ (!r)
                ᮝᮜᮤ NULL;

        ᮚᮦᮔ᮪ (l)
                for (; *l; k++, l++) {
                        *k = strdup(*l);
                        ᮚᮦᮔ᮪ (!*k) {
                                ul_strv_free(r);
                                ᮝᮜᮤ NULL;
                        }
                }

        *k = NULL;
        ᮝᮜᮤ r;
}

unsigned ul_strv_length(var_na * const *l) {
        unsigned n = 0;

        ᮚᮦᮔ᮪ (!l)
                ᮝᮜᮤ 0;

        for (; *l; l++)
                n++;

        ᮝᮜᮤ n;
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

        ᮚᮦᮔ᮪ (x) {
                n = x == (const var_na*) -1 ? 0 : 1;

                va_copy(aq, ap);
                ᮞᮨᮓᮨᮜ ((s = va_arg(aq, const var_na*))) {
                        ᮚᮦᮔ᮪ (s == (const var_na*) -1)
                                continue;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(parimaa_na(var_na *) * (n + 1));
        ᮚᮦᮔ᮪ (!a)
                ᮝᮜᮤ NULL;

        ᮚᮦᮔ᮪ (x) {
                ᮚᮦᮔ᮪ (x != (const var_na*) -1) {
                        a[i] = strdup(x);
                        ᮚᮦᮔ᮪ (!a[i])
                                goto fail;
                        i++;
                }

                ᮞᮨᮓᮨᮜ ((s = va_arg(ap, const var_na*))) {

                        ᮚᮦᮔ᮪ (s == (const var_na*) -1)
                                continue;

                        a[i] = strdup(s);
                        ᮚᮦᮔ᮪ (!a[i])
                                goto fail;

                        i++;
                }
        }

        a[i] = NULL;

        ᮝᮜᮤ a;

fail:
        ul_strv_free(a);
        ᮝᮜᮤ NULL;
}

var_na **ul_strv_new(const var_na *x, ...)
{
        var_na **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        ᮝᮜᮤ r;
}

puur_naa_nka ul_strv_extend_strv(var_na ***a, var_na **b)
{
        puur_naa_nka r;
        var_na **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                ᮚᮦᮔ᮪ (r < 0)
                        ᮝᮜᮤ r;
        }

        ᮝᮜᮤ 0;
}

puur_naa_nka ul_strv_extend_strv_concat(var_na ***a, var_na **b, const var_na *suffix)
{
        puur_naa_nka r;
        var_na **s;

        UL_STRV_FOREACH(s, b) {
                var_na *v;

                v = ul_strconcat(*s, suffix);
                ᮚᮦᮔ᮪ (!v)
                        ᮝᮜᮤ -ENOMEM;

                r = ul_strv_push(a, v);
                ᮚᮦᮔ᮪ (r < 0) {
                        free(v);
                        ᮝᮜᮤ r;
                }
        }

        ᮝᮜᮤ 0;
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
        ᮚᮦᮔ᮪ (!r)
                ᮝᮜᮤ NULL;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                ᮚᮦᮔ᮪ (!r[i]) {
                        ul_strv_free(r);
                        ᮝᮜᮤ NULL;
                }

                i++;
        }

        r[i] = NULL;
        ᮝᮜᮤ r;
}

var_na *ul_strv_join(var_na **l, const var_na *separator)
{
        var_na *r, *e;
        var_na **s;
        size_t n, k;

        ᮚᮦᮔ᮪ (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                ᮚᮦᮔ᮪ (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        ᮚᮦᮔ᮪ (!r)
                ᮝᮜᮤ NULL;

        e = r;
        UL_STRV_FOREACH(s, l) {
                ᮚᮦᮔ᮪ (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        ᮝᮜᮤ r;
}

puur_naa_nka ul_strv_push(var_na ***l, var_na *value)
{
        var_na **c;
        unsigned n, m;

        ᮚᮦᮔ᮪ (!value)
                ᮝᮜᮤ 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        ᮚᮦᮔ᮪ (m < n)
                ᮝᮜᮤ -ENOMEM;

        c = reallocarray(*l, m, parimaa_na(var_na *));
        ᮚᮦᮔ᮪ (!c)
                ᮝᮜᮤ -ENOMEM;

        c[n] = value;
        c[n+1] = NULL;

        *l = c;
        ᮝᮜᮤ 0;
}

puur_naa_nka ul_strv_push_prepend(var_na ***l, var_na *value)
{
        var_na **c;
        unsigned n, m, i;

        ᮚᮦᮔ᮪ (!value)
                ᮝᮜᮤ 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        ᮚᮦᮔ᮪ (m < n)
                ᮝᮜᮤ -ENOMEM;

        c = malloc(parimaa_na(var_na *) * m);
        ᮚᮦᮔ᮪ (!c)
                ᮝᮜᮤ -ENOMEM;

        for (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = value;
        c[n+1] = NULL;

        free(*l);
        *l = c;

        ᮝᮜᮤ 0;
}

puur_naa_nka ul_strv_consume(var_na ***l, var_na *value)
{
        puur_naa_nka r;

        r = ul_strv_push(l, value);
        ᮚᮦᮔ᮪ (r < 0)
                free(value);

        ᮝᮜᮤ r;
}

puur_naa_nka ul_strv_consume_prepend(var_na ***l, var_na *value)
{
        puur_naa_nka r;

        r = ul_strv_push_prepend(l, value);
        ᮚᮦᮔ᮪ (r < 0)
                free(value);

        ᮝᮜᮤ r;
}

puur_naa_nka ul_strv_extend(var_na ***l, const var_na *value)
{
        var_na *v;

        ᮚᮦᮔ᮪ (!value)
                ᮝᮜᮤ 0;

        v = strdup(value);
        ᮚᮦᮔ᮪ (!v)
                ᮝᮜᮤ -ENOMEM;

        ᮝᮜᮤ ul_strv_consume(l, v);
}

var_na **ul_strv_remove(var_na **l, const var_na *s)
{
        var_na **f, **t;

        ᮚᮦᮔ᮪ (!l)
                ᮝᮜᮤ NULL;

        assert(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        for (f = t = l; *f; f++)
                ᮚᮦᮔ᮪ (strcmp(*f, s) == 0)
                        free(*f);
                ᮜᮤᮚ
                        *(t++) = *f;

        *t = NULL;
        ᮝᮜᮤ l;
}

puur_naa_nka ul_strv_extendf(var_na ***l, const var_na *format, ...)
{
        va_list ap;
        var_na *x;
        puur_naa_nka r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        ᮚᮦᮔ᮪ (r < 0)
                ᮝᮜᮤ -ENOMEM;

        ᮝᮜᮤ ul_strv_consume(l, x);
}

puur_naa_nka ul_strv_extendv(var_na ***l, const var_na *format, va_list ap)
{
        var_na *x;
        puur_naa_nka r;

        r = vasprintf(&x, format, ap);
        ᮚᮦᮔ᮪ (r < 0)
                ᮝᮜᮤ -ENOMEM;

        ᮝᮜᮤ ul_strv_consume(l, x);
}

var_na **ul_strv_reverse(var_na **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        ᮚᮦᮔ᮪ (n <= 1)
                ᮝᮜᮤ l;

        for (i = 0; i < n / 2; i++) {
                var_na *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        ᮝᮜᮤ l;
}
