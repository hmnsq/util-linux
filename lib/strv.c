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

#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <assert.h>

#અન્તર્ભાવ "strutils.h"
#અન્તર્ભાવ "strv.h"

સ્થિર શૂન્ય ul_strv_clear(અક્ષર **l)
{
        અક્ષર **k;

        જો (!l)
                પાછા;

        માટે (k = l; *k; k++)
                મુક્ત(*k);

        *l = ખાલી;
}

અક્ષર **ul_strv_free(અક્ષર **l)
{
        ul_strv_clear(l);
        મુક્ત(l);
        પાછા ખાલી;
}

અક્ષર **ul_strv_copy(અક્ષર * અચલ *l)
{
        અક્ષર **r, **k;

        k = r = malloc(કદ(અક્ષર *) * (ul_strv_length(l) + 1));
        જો (!r)
                પાછા ખાલી;

        જો (l)
                માટે (; *l; k++, l++) {
                        *k = strdup(*l);
                        જો (!*k) {
                                ul_strv_free(r);
                                પાછા ખાલી;
                        }
                }

        *k = ખાલી;
        પાછા r;
}

unsigned ul_strv_length(અક્ષર * અચલ *l) {
        unsigned n = 0;

        જો (!l)
                પાછા 0;

        માટે (; *l; l++)
                n++;

        પાછા n;
}

સ્થિર અક્ષર **ul_strv_new_ap(અચલ અક્ષર *x, va_list ap)
{
        અચલ અક્ષર *s;
        અક્ષર **a;
        unsigned n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        જો (x) {
                n = x == (અચલ અક્ષર*) -1 ? 0 : 1;

                va_copy(aq, ap);
                જ્યારે ((s = va_arg(aq, અચલ અક્ષર*))) {
                        જો (s == (અચલ અક્ષર*) -1)
                                ચાલુ;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(કદ(અક્ષર *) * (n + 1));
        જો (!a)
                પાછા ખાલી;

        જો (x) {
                જો (x != (અચલ અક્ષર*) -1) {
                        a[i] = strdup(x);
                        જો (!a[i])
                                જાઓ fail;
                        i++;
                }

                જ્યારે ((s = va_arg(ap, અચલ અક્ષર*))) {

                        જો (s == (અચલ અક્ષર*) -1)
                                ચાલુ;

                        a[i] = strdup(s);
                        જો (!a[i])
                                જાઓ fail;

                        i++;
                }
        }

        a[i] = ખાલી;

        પાછા a;

fail:
        ul_strv_free(a);
        પાછા ખાલી;
}

અક્ષર **ul_strv_new(અચલ અક્ષર *x, ...)
{
        અક્ષર **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        પાછા r;
}

પૂર્ણાંક ul_strv_extend_strv(અક્ષર ***a, અક્ષર **b)
{
        પૂર્ણાંક r;
        અક્ષર **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                જો (r < 0)
                        પાછા r;
        }

        પાછા 0;
}

પૂર્ણાંક ul_strv_extend_strv_concat(અક્ષર ***a, અક્ષર **b, અચલ અક્ષર *suffix)
{
        પૂર્ણાંક r;
        અક્ષર **s;

        UL_STRV_FOREACH(s, b) {
                અક્ષર *v;

                v = ul_strconcat(*s, suffix);
                જો (!v)
                        પાછા -ENOMEM;

                r = ul_strv_ધકેલો(a, v);
                જો (r < 0) {
                        મુક્ત(v);
                        પાછા r;
                }
        }

        પાછા 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        માટે ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

#define FOREACH_WORD_SEPARATOR(word, length, s, separator, state)       \
        _FOREACH_WORD(word, length, s, separator, false, state)


અક્ષર **ul_strv_split(અચલ અક્ષર *s, અચલ અક્ષર *separator)
{
        અચલ અક્ષર *word, *state;
        size_t l;
        unsigned n, i;
        અક્ષર **r;

        પ્રતિજ્ઞા(s);

        n = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state)
                n++;

        r = malloc(કદ(અક્ષર *) * (n + 1));
        જો (!r)
                પાછા ખાલી;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                જો (!r[i]) {
                        ul_strv_free(r);
                        પાછા ખાલી;
                }

                i++;
        }

        r[i] = ખાલી;
        પાછા r;
}

અક્ષર *ul_strv_join(અક્ષર **l, અચલ અક્ષર *separator)
{
        અક્ષર *r, *e;
        અક્ષર **s;
        size_t n, k;

        જો (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                જો (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        જો (!r)
                પાછા ખાલી;

        e = r;
        UL_STRV_FOREACH(s, l) {
                જો (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        પાછા r;
}

પૂર્ણાંક ul_strv_ધકેલો(અક્ષર ***l, અક્ષર *કિંમત)
{
        અક્ષર **c;
        unsigned n, m;

        જો (!કિંમત)
                પાછા 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        જો (m < n)
                પાછા -ENOMEM;

        c = reallocarray(*l, m, કદ(અક્ષર *));
        જો (!c)
                પાછા -ENOMEM;

        c[n] = કિંમત;
        c[n+1] = ખાલી;

        *l = c;
        પાછા 0;
}

પૂર્ણાંક ul_strv_ધકેલો_prepend(અક્ષર ***l, અક્ષર *કિંમત)
{
        અક્ષર **c;
        unsigned n, m, i;

        જો (!કિંમત)
                પાછા 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        જો (m < n)
                પાછા -ENOMEM;

        c = malloc(કદ(અક્ષર *) * m);
        જો (!c)
                પાછા -ENOMEM;

        માટે (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = કિંમત;
        c[n+1] = ખાલી;

        મુક્ત(*l);
        *l = c;

        પાછા 0;
}

પૂર્ણાંક ul_strv_consume(અક્ષર ***l, અક્ષર *કિંમત)
{
        પૂર્ણાંક r;

        r = ul_strv_ધકેલો(l, કિંમત);
        જો (r < 0)
                મુક્ત(કિંમત);

        પાછા r;
}

પૂર્ણાંક ul_strv_consume_prepend(અક્ષર ***l, અક્ષર *કિંમત)
{
        પૂર્ણાંક r;

        r = ul_strv_ધકેલો_prepend(l, કિંમત);
        જો (r < 0)
                મુક્ત(કિંમત);

        પાછા r;
}

પૂર્ણાંક ul_strv_extend(અક્ષર ***l, અચલ અક્ષર *કિંમત)
{
        અક્ષર *v;

        જો (!કિંમત)
                પાછા 0;

        v = strdup(કિંમત);
        જો (!v)
                પાછા -ENOMEM;

        પાછા ul_strv_consume(l, v);
}

અક્ષર **ul_strv_remove(અક્ષર **l, અચલ અક્ષર *s)
{
        અક્ષર **f, **t;

        જો (!l)
                પાછા ખાલી;

        પ્રતિજ્ઞા(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        માટે (f = t = l; *f; f++)
                જો (strcmp(*f, s) == 0)
                        મુક્ત(*f);
                નહીંતર
                        *(t++) = *f;

        *t = ખાલી;
        પાછા l;
}

પૂર્ણાંક ul_strv_extendf(અક્ષર ***l, અચલ અક્ષર *format, ...)
{
        va_list ap;
        અક્ષર *x;
        પૂર્ણાંક r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        જો (r < 0)
                પાછા -ENOMEM;

        પાછા ul_strv_consume(l, x);
}

પૂર્ણાંક ul_strv_extendv(અક્ષર ***l, અચલ અક્ષર *format, va_list ap)
{
        અક્ષર *x;
        પૂર્ણાંક r;

        r = vasprintf(&x, format, ap);
        જો (r < 0)
                પાછા -ENOMEM;

        પાછા ul_strv_consume(l, x);
}

અક્ષર **ul_strv_reverse(અક્ષર **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        જો (n <= 1)
                પાછા l;

        માટે (i = 0; i < n / 2; i++) {
                અક્ષર *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        પાછા l;
}
