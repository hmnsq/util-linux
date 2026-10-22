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

static void ul_strv_clear(char **l)
{
        char **k;

        nếu (!l)
                trả_về;

        cho (k = l; *k; k++)
                free(*k);

        *l = NULL;
}

char **ul_strv_free(char **l)
{
        ul_strv_clear(l);
        free(l);
        trả_về NULL;
}

char **ul_strv_copy(char * const *l)
{
        char **r, **k;

        k = r = malloc(sizeof(char *) * (ul_strv_length(l) + 1));
        nếu (!r)
                trả_về NULL;

        nếu (l)
                cho (; *l; k++, l++) {
                        *k = strdup(*l);
                        nếu (!*k) {
                                ul_strv_free(r);
                                trả_về NULL;
                        }
                }

        *k = NULL;
        trả_về r;
}

unsigned ul_strv_length(char * const *l) {
        unsigned n = 0;

        nếu (!l)
                trả_về 0;

        cho (; *l; l++)
                n++;

        trả_về n;
}

static char **ul_strv_new_ap(const char *x, va_list ap)
{
        const char *s;
        char **a;
        unsigned n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        nếu (x) {
                n = x == (const char*) -1 ? 0 : 1;

                va_copy(aq, ap);
                trong_khi ((s = va_arg(aq, const char*))) {
                        nếu (s == (const char*) -1)
                                continue;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(sizeof(char *) * (n + 1));
        nếu (!a)
                trả_về NULL;

        nếu (x) {
                nếu (x != (const char*) -1) {
                        a[i] = strdup(x);
                        nếu (!a[i])
                                goto fail;
                        i++;
                }

                trong_khi ((s = va_arg(ap, const char*))) {

                        nếu (s == (const char*) -1)
                                continue;

                        a[i] = strdup(s);
                        nếu (!a[i])
                                goto fail;

                        i++;
                }
        }

        a[i] = NULL;

        trả_về a;

fail:
        ul_strv_free(a);
        trả_về NULL;
}

char **ul_strv_new(const char *x, ...)
{
        char **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        trả_về r;
}

số_nguyên ul_strv_extend_strv(char ***a, char **b)
{
        số_nguyên r;
        char **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                nếu (r < 0)
                        trả_về r;
        }

        trả_về 0;
}

số_nguyên ul_strv_extend_strv_concat(char ***a, char **b, const char *suffix)
{
        số_nguyên r;
        char **s;

        UL_STRV_FOREACH(s, b) {
                char *v;

                v = ul_strconcat(*s, suffix);
                nếu (!v)
                        trả_về -ENOMEM;

                r = ul_strv_push(a, v);
                nếu (r < 0) {
                        free(v);
                        trả_về r;
                }
        }

        trả_về 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        cho ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

#define FOREACH_WORD_SEPARATOR(word, length, s, separator, state)       \
        _FOREACH_WORD(word, length, s, separator, false, state)


char **ul_strv_split(const char *s, const char *separator)
{
        const char *word, *state;
        size_t l;
        unsigned n, i;
        char **r;

        assert(s);

        n = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state)
                n++;

        r = malloc(sizeof(char *) * (n + 1));
        nếu (!r)
                trả_về NULL;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                nếu (!r[i]) {
                        ul_strv_free(r);
                        trả_về NULL;
                }

                i++;
        }

        r[i] = NULL;
        trả_về r;
}

char *ul_strv_join(char **l, const char *separator)
{
        char *r, *e;
        char **s;
        size_t n, k;

        nếu (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                nếu (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        nếu (!r)
                trả_về NULL;

        e = r;
        UL_STRV_FOREACH(s, l) {
                nếu (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        trả_về r;
}

số_nguyên ul_strv_push(char ***l, char *value)
{
        char **c;
        unsigned n, m;

        nếu (!value)
                trả_về 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        nếu (m < n)
                trả_về -ENOMEM;

        c = reallocarray(*l, m, sizeof(char *));
        nếu (!c)
                trả_về -ENOMEM;

        c[n] = value;
        c[n+1] = NULL;

        *l = c;
        trả_về 0;
}

số_nguyên ul_strv_push_prepend(char ***l, char *value)
{
        char **c;
        unsigned n, m, i;

        nếu (!value)
                trả_về 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        nếu (m < n)
                trả_về -ENOMEM;

        c = malloc(sizeof(char *) * m);
        nếu (!c)
                trả_về -ENOMEM;

        cho (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = value;
        c[n+1] = NULL;

        free(*l);
        *l = c;

        trả_về 0;
}

số_nguyên ul_strv_consume(char ***l, char *value)
{
        số_nguyên r;

        r = ul_strv_push(l, value);
        nếu (r < 0)
                free(value);

        trả_về r;
}

số_nguyên ul_strv_consume_prepend(char ***l, char *value)
{
        số_nguyên r;

        r = ul_strv_push_prepend(l, value);
        nếu (r < 0)
                free(value);

        trả_về r;
}

số_nguyên ul_strv_extend(char ***l, const char *value)
{
        char *v;

        nếu (!value)
                trả_về 0;

        v = strdup(value);
        nếu (!v)
                trả_về -ENOMEM;

        trả_về ul_strv_consume(l, v);
}

char **ul_strv_remove(char **l, const char *s)
{
        char **f, **t;

        nếu (!l)
                trả_về NULL;

        assert(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        cho (f = t = l; *f; f++)
                nếu (strcmp(*f, s) == 0)
                        free(*f);
                không_thì
                        *(t++) = *f;

        *t = NULL;
        trả_về l;
}

số_nguyên ul_strv_extendf(char ***l, const char *format, ...)
{
        va_list ap;
        char *x;
        số_nguyên r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        nếu (r < 0)
                trả_về -ENOMEM;

        trả_về ul_strv_consume(l, x);
}

số_nguyên ul_strv_extendv(char ***l, const char *format, va_list ap)
{
        char *x;
        số_nguyên r;

        r = vasprintf(&x, format, ap);
        nếu (r < 0)
                trả_về -ENOMEM;

        trả_về ul_strv_consume(l, x);
}

char **ul_strv_reverse(char **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        nếu (n <= 1)
                trả_về l;

        cho (i = 0; i < n / 2; i++) {
                char *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        trả_về l;
}
