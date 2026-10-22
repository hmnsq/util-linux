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

        만약 (!l)
                반환;

        위해 (k = l; *k; k++)
                free(*k);

        *l = NULL;
}

char **ul_strv_free(char **l)
{
        ul_strv_clear(l);
        free(l);
        반환 NULL;
}

char **ul_strv_copy(char * const *l)
{
        char **r, **k;

        k = r = malloc(sizeof(char *) * (ul_strv_length(l) + 1));
        만약 (!r)
                반환 NULL;

        만약 (l)
                위해 (; *l; k++, l++) {
                        *k = strdup(*l);
                        만약 (!*k) {
                                ul_strv_free(r);
                                반환 NULL;
                        }
                }

        *k = NULL;
        반환 r;
}

unsigned ul_strv_length(char * const *l) {
        unsigned n = 0;

        만약 (!l)
                반환 0;

        위해 (; *l; l++)
                n++;

        반환 n;
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

        만약 (x) {
                n = x == (const char*) -1 ? 0 : 1;

                va_copy(aq, ap);
                동안 ((s = va_arg(aq, const char*))) {
                        만약 (s == (const char*) -1)
                                continue;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(sizeof(char *) * (n + 1));
        만약 (!a)
                반환 NULL;

        만약 (x) {
                만약 (x != (const char*) -1) {
                        a[i] = strdup(x);
                        만약 (!a[i])
                                goto fail;
                        i++;
                }

                동안 ((s = va_arg(ap, const char*))) {

                        만약 (s == (const char*) -1)
                                continue;

                        a[i] = strdup(s);
                        만약 (!a[i])
                                goto fail;

                        i++;
                }
        }

        a[i] = NULL;

        반환 a;

fail:
        ul_strv_free(a);
        반환 NULL;
}

char **ul_strv_new(const char *x, ...)
{
        char **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        반환 r;
}

정수 ul_strv_extend_strv(char ***a, char **b)
{
        정수 r;
        char **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                만약 (r < 0)
                        반환 r;
        }

        반환 0;
}

정수 ul_strv_extend_strv_concat(char ***a, char **b, const char *suffix)
{
        정수 r;
        char **s;

        UL_STRV_FOREACH(s, b) {
                char *v;

                v = ul_strconcat(*s, suffix);
                만약 (!v)
                        반환 -ENOMEM;

                r = ul_strv_push(a, v);
                만약 (r < 0) {
                        free(v);
                        반환 r;
                }
        }

        반환 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        위해 ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

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
        만약 (!r)
                반환 NULL;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                만약 (!r[i]) {
                        ul_strv_free(r);
                        반환 NULL;
                }

                i++;
        }

        r[i] = NULL;
        반환 r;
}

char *ul_strv_join(char **l, const char *separator)
{
        char *r, *e;
        char **s;
        size_t n, k;

        만약 (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                만약 (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        만약 (!r)
                반환 NULL;

        e = r;
        UL_STRV_FOREACH(s, l) {
                만약 (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        반환 r;
}

정수 ul_strv_push(char ***l, char *value)
{
        char **c;
        unsigned n, m;

        만약 (!value)
                반환 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        만약 (m < n)
                반환 -ENOMEM;

        c = reallocarray(*l, m, sizeof(char *));
        만약 (!c)
                반환 -ENOMEM;

        c[n] = value;
        c[n+1] = NULL;

        *l = c;
        반환 0;
}

정수 ul_strv_push_prepend(char ***l, char *value)
{
        char **c;
        unsigned n, m, i;

        만약 (!value)
                반환 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        만약 (m < n)
                반환 -ENOMEM;

        c = malloc(sizeof(char *) * m);
        만약 (!c)
                반환 -ENOMEM;

        위해 (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = value;
        c[n+1] = NULL;

        free(*l);
        *l = c;

        반환 0;
}

정수 ul_strv_consume(char ***l, char *value)
{
        정수 r;

        r = ul_strv_push(l, value);
        만약 (r < 0)
                free(value);

        반환 r;
}

정수 ul_strv_consume_prepend(char ***l, char *value)
{
        정수 r;

        r = ul_strv_push_prepend(l, value);
        만약 (r < 0)
                free(value);

        반환 r;
}

정수 ul_strv_extend(char ***l, const char *value)
{
        char *v;

        만약 (!value)
                반환 0;

        v = strdup(value);
        만약 (!v)
                반환 -ENOMEM;

        반환 ul_strv_consume(l, v);
}

char **ul_strv_remove(char **l, const char *s)
{
        char **f, **t;

        만약 (!l)
                반환 NULL;

        assert(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        위해 (f = t = l; *f; f++)
                만약 (strcmp(*f, s) == 0)
                        free(*f);
                아니면
                        *(t++) = *f;

        *t = NULL;
        반환 l;
}

정수 ul_strv_extendf(char ***l, const char *format, ...)
{
        va_list ap;
        char *x;
        정수 r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        만약 (r < 0)
                반환 -ENOMEM;

        반환 ul_strv_consume(l, x);
}

정수 ul_strv_extendv(char ***l, const char *format, va_list ap)
{
        char *x;
        정수 r;

        r = vasprintf(&x, format, ap);
        만약 (r < 0)
                반환 -ENOMEM;

        반환 ul_strv_consume(l, x);
}

char **ul_strv_reverse(char **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        만약 (n <= 1)
                반환 l;

        위해 (i = 0; i < n / 2; i++) {
                char *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        반환 l;
}
