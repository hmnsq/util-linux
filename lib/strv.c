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

        ако (!l)
                врати;

        за (k = l; *k; k++)
                free(*k);

        *l = NULL;
}

char **ul_strv_free(char **l)
{
        ul_strv_clear(l);
        free(l);
        врати NULL;
}

char **ul_strv_copy(char * const *l)
{
        char **r, **k;

        k = r = malloc(sizeof(char *) * (ul_strv_length(l) + 1));
        ако (!r)
                врати NULL;

        ако (l)
                за (; *l; k++, l++) {
                        *k = strdup(*l);
                        ако (!*k) {
                                ul_strv_free(r);
                                врати NULL;
                        }
                }

        *k = NULL;
        врати r;
}

unsigned ul_strv_length(char * const *l) {
        unsigned n = 0;

        ако (!l)
                врати 0;

        за (; *l; l++)
                n++;

        врати n;
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

        ако (x) {
                n = x == (const char*) -1 ? 0 : 1;

                va_copy(aq, ap);
                док ((s = va_arg(aq, const char*))) {
                        ако (s == (const char*) -1)
                                continue;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(sizeof(char *) * (n + 1));
        ако (!a)
                врати NULL;

        ако (x) {
                ако (x != (const char*) -1) {
                        a[i] = strdup(x);
                        ако (!a[i])
                                goto fail;
                        i++;
                }

                док ((s = va_arg(ap, const char*))) {

                        ако (s == (const char*) -1)
                                continue;

                        a[i] = strdup(s);
                        ако (!a[i])
                                goto fail;

                        i++;
                }
        }

        a[i] = NULL;

        врати a;

fail:
        ul_strv_free(a);
        врати NULL;
}

char **ul_strv_new(const char *x, ...)
{
        char **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        врати r;
}

цео ul_strv_extend_strv(char ***a, char **b)
{
        цео r;
        char **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                ако (r < 0)
                        врати r;
        }

        врати 0;
}

цео ul_strv_extend_strv_concat(char ***a, char **b, const char *suffix)
{
        цео r;
        char **s;

        UL_STRV_FOREACH(s, b) {
                char *v;

                v = ul_strconcat(*s, suffix);
                ако (!v)
                        врати -ENOMEM;

                r = ul_strv_push(a, v);
                ако (r < 0) {
                        free(v);
                        врати r;
                }
        }

        врати 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        за ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

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
        ако (!r)
                врати NULL;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                ако (!r[i]) {
                        ul_strv_free(r);
                        врати NULL;
                }

                i++;
        }

        r[i] = NULL;
        врати r;
}

char *ul_strv_join(char **l, const char *separator)
{
        char *r, *e;
        char **s;
        size_t n, k;

        ако (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                ако (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        ако (!r)
                врати NULL;

        e = r;
        UL_STRV_FOREACH(s, l) {
                ако (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        врати r;
}

цео ul_strv_push(char ***l, char *value)
{
        char **c;
        unsigned n, m;

        ако (!value)
                врати 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        ако (m < n)
                врати -ENOMEM;

        c = reallocarray(*l, m, sizeof(char *));
        ако (!c)
                врати -ENOMEM;

        c[n] = value;
        c[n+1] = NULL;

        *l = c;
        врати 0;
}

цео ul_strv_push_prepend(char ***l, char *value)
{
        char **c;
        unsigned n, m, i;

        ако (!value)
                врати 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        ако (m < n)
                врати -ENOMEM;

        c = malloc(sizeof(char *) * m);
        ако (!c)
                врати -ENOMEM;

        за (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = value;
        c[n+1] = NULL;

        free(*l);
        *l = c;

        врати 0;
}

цео ul_strv_consume(char ***l, char *value)
{
        цео r;

        r = ul_strv_push(l, value);
        ако (r < 0)
                free(value);

        врати r;
}

цео ul_strv_consume_prepend(char ***l, char *value)
{
        цео r;

        r = ul_strv_push_prepend(l, value);
        ако (r < 0)
                free(value);

        врати r;
}

цео ul_strv_extend(char ***l, const char *value)
{
        char *v;

        ако (!value)
                врати 0;

        v = strdup(value);
        ако (!v)
                врати -ENOMEM;

        врати ul_strv_consume(l, v);
}

char **ul_strv_remove(char **l, const char *s)
{
        char **f, **t;

        ако (!l)
                врати NULL;

        assert(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        за (f = t = l; *f; f++)
                ако (strcmp(*f, s) == 0)
                        free(*f);
                инако
                        *(t++) = *f;

        *t = NULL;
        врати l;
}

цео ul_strv_extendf(char ***l, const char *format, ...)
{
        va_list ap;
        char *x;
        цео r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        ако (r < 0)
                врати -ENOMEM;

        врати ul_strv_consume(l, x);
}

цео ul_strv_extendv(char ***l, const char *format, va_list ap)
{
        char *x;
        цео r;

        r = vasprintf(&x, format, ap);
        ако (r < 0)
                врати -ENOMEM;

        врати ul_strv_consume(l, x);
}

char **ul_strv_reverse(char **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        ако (n <= 1)
                врати l;

        за (i = 0; i < n / 2; i++) {
                char *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        врати l;
}
