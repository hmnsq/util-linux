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

        如果 (!l)
                返回;

        为 (k = l; *k; k++)
                free(*k);

        *l = NULL;
}

char **ul_strv_free(char **l)
{
        ul_strv_clear(l);
        free(l);
        返回 NULL;
}

char **ul_strv_copy(char * const *l)
{
        char **r, **k;

        k = r = malloc(sizeof(char *) * (ul_strv_length(l) + 1));
        如果 (!r)
                返回 NULL;

        如果 (l)
                为 (; *l; k++, l++) {
                        *k = strdup(*l);
                        如果 (!*k) {
                                ul_strv_free(r);
                                返回 NULL;
                        }
                }

        *k = NULL;
        返回 r;
}

unsigned ul_strv_length(char * const *l) {
        unsigned n = 0;

        如果 (!l)
                返回 0;

        为 (; *l; l++)
                n++;

        返回 n;
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

        如果 (x) {
                n = x == (const char*) -1 ? 0 : 1;

                va_copy(aq, ap);
                当 ((s = va_arg(aq, const char*))) {
                        如果 (s == (const char*) -1)
                                continue;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(sizeof(char *) * (n + 1));
        如果 (!a)
                返回 NULL;

        如果 (x) {
                如果 (x != (const char*) -1) {
                        a[i] = strdup(x);
                        如果 (!a[i])
                                goto fail;
                        i++;
                }

                当 ((s = va_arg(ap, const char*))) {

                        如果 (s == (const char*) -1)
                                continue;

                        a[i] = strdup(s);
                        如果 (!a[i])
                                goto fail;

                        i++;
                }
        }

        a[i] = NULL;

        返回 a;

fail:
        ul_strv_free(a);
        返回 NULL;
}

char **ul_strv_new(const char *x, ...)
{
        char **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        返回 r;
}

整数 ul_strv_extend_strv(char ***a, char **b)
{
        整数 r;
        char **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                如果 (r < 0)
                        返回 r;
        }

        返回 0;
}

整数 ul_strv_extend_strv_concat(char ***a, char **b, const char *suffix)
{
        整数 r;
        char **s;

        UL_STRV_FOREACH(s, b) {
                char *v;

                v = ul_strconcat(*s, suffix);
                如果 (!v)
                        返回 -ENOMEM;

                r = ul_strv_push(a, v);
                如果 (r < 0) {
                        free(v);
                        返回 r;
                }
        }

        返回 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        为 ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

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
        如果 (!r)
                返回 NULL;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                如果 (!r[i]) {
                        ul_strv_free(r);
                        返回 NULL;
                }

                i++;
        }

        r[i] = NULL;
        返回 r;
}

char *ul_strv_join(char **l, const char *separator)
{
        char *r, *e;
        char **s;
        size_t n, k;

        如果 (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                如果 (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        如果 (!r)
                返回 NULL;

        e = r;
        UL_STRV_FOREACH(s, l) {
                如果 (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        返回 r;
}

整数 ul_strv_push(char ***l, char *value)
{
        char **c;
        unsigned n, m;

        如果 (!value)
                返回 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        如果 (m < n)
                返回 -ENOMEM;

        c = reallocarray(*l, m, sizeof(char *));
        如果 (!c)
                返回 -ENOMEM;

        c[n] = value;
        c[n+1] = NULL;

        *l = c;
        返回 0;
}

整数 ul_strv_push_prepend(char ***l, char *value)
{
        char **c;
        unsigned n, m, i;

        如果 (!value)
                返回 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        如果 (m < n)
                返回 -ENOMEM;

        c = malloc(sizeof(char *) * m);
        如果 (!c)
                返回 -ENOMEM;

        为 (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = value;
        c[n+1] = NULL;

        free(*l);
        *l = c;

        返回 0;
}

整数 ul_strv_consume(char ***l, char *value)
{
        整数 r;

        r = ul_strv_push(l, value);
        如果 (r < 0)
                free(value);

        返回 r;
}

整数 ul_strv_consume_prepend(char ***l, char *value)
{
        整数 r;

        r = ul_strv_push_prepend(l, value);
        如果 (r < 0)
                free(value);

        返回 r;
}

整数 ul_strv_extend(char ***l, const char *value)
{
        char *v;

        如果 (!value)
                返回 0;

        v = strdup(value);
        如果 (!v)
                返回 -ENOMEM;

        返回 ul_strv_consume(l, v);
}

char **ul_strv_remove(char **l, const char *s)
{
        char **f, **t;

        如果 (!l)
                返回 NULL;

        assert(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        为 (f = t = l; *f; f++)
                如果 (strcmp(*f, s) == 0)
                        free(*f);
                否则
                        *(t++) = *f;

        *t = NULL;
        返回 l;
}

整数 ul_strv_extendf(char ***l, const char *format, ...)
{
        va_list ap;
        char *x;
        整数 r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        如果 (r < 0)
                返回 -ENOMEM;

        返回 ul_strv_consume(l, x);
}

整数 ul_strv_extendv(char ***l, const char *format, va_list ap)
{
        char *x;
        整数 r;

        r = vasprintf(&x, format, ap);
        如果 (r < 0)
                返回 -ENOMEM;

        返回 ul_strv_consume(l, x);
}

char **ul_strv_reverse(char **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        如果 (n <= 1)
                返回 l;

        为 (i = 0; i < n / 2; i++) {
                char *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        返回 l;
}
