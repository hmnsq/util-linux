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

#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdarg.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>

#ଅନ୍ତର୍ଭାଵ "strutils.h"
#ଅନ୍ତର୍ଭାଵ "strv.h"

ସ୍ଥିର ଶୂନ୍ୟ ul_strv_clear(ଅକ୍ଷର **l)
{
        ଅକ୍ଷର **k;

        ଯଦି (!l)
                ଫେରନ୍ତୁ;

        ପାଇଁ (k = l; *k; k++)
                ମୁକ୍ତ(*k);

        *l = ଖାଲି;
}

ଅକ୍ଷର **ul_strv_free(ଅକ୍ଷର **l)
{
        ul_strv_clear(l);
        ମୁକ୍ତ(l);
        ଫେରନ୍ତୁ ଖାଲି;
}

ଅକ୍ଷର **ul_strv_copy(ଅକ୍ଷର * ଅଚଳ *l)
{
        ଅକ୍ଷର **r, **k;

        k = r = malloc(ଆକାର(ଅକ୍ଷର *) * (ul_strv_length(l) + 1));
        ଯଦି (!r)
                ଫେରନ୍ତୁ ଖାଲି;

        ଯଦି (l)
                ପାଇଁ (; *l; k++, l++) {
                        *k = strdup(*l);
                        ଯଦି (!*k) {
                                ul_strv_free(r);
                                ଫେରନ୍ତୁ ଖାଲି;
                        }
                }

        *k = ଖାଲି;
        ଫେରନ୍ତୁ r;
}

unsigned ul_strv_length(ଅକ୍ଷର * ଅଚଳ *l) {
        unsigned n = 0;

        ଯଦି (!l)
                ଫେରନ୍ତୁ 0;

        ପାଇଁ (; *l; l++)
                n++;

        ଫେରନ୍ତୁ n;
}

ସ୍ଥିର ଅକ୍ଷର **ul_strv_new_ap(ଅଚଳ ଅକ୍ଷର *x, va_list ap)
{
        ଅଚଳ ଅକ୍ଷର *s;
        ଅକ୍ଷର **a;
        unsigned n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        ଯଦି (x) {
                n = x == (ଅଚଳ ଅକ୍ଷର*) -1 ? 0 : 1;

                va_copy(aq, ap);
                ଯେତେବେଳେ ((s = va_arg(aq, ଅଚଳ ଅକ୍ଷର*))) {
                        ଯଦି (s == (ଅଚଳ ଅକ୍ଷର*) -1)
                                ଜାରି;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(ଆକାର(ଅକ୍ଷର *) * (n + 1));
        ଯଦି (!a)
                ଫେରନ୍ତୁ ଖାଲି;

        ଯଦି (x) {
                ଯଦି (x != (ଅଚଳ ଅକ୍ଷର*) -1) {
                        a[i] = strdup(x);
                        ଯଦି (!a[i])
                                ଯାଅ fail;
                        i++;
                }

                ଯେତେବେଳେ ((s = va_arg(ap, ଅଚଳ ଅକ୍ଷର*))) {

                        ଯଦି (s == (ଅଚଳ ଅକ୍ଷର*) -1)
                                ଜାରି;

                        a[i] = strdup(s);
                        ଯଦି (!a[i])
                                ଯାଅ fail;

                        i++;
                }
        }

        a[i] = ଖାଲି;

        ଫେରନ୍ତୁ a;

fail:
        ul_strv_free(a);
        ଫେରନ୍ତୁ ଖାଲି;
}

ଅକ୍ଷର **ul_strv_new(ଅଚଳ ଅକ୍ଷର *x, ...)
{
        ଅକ୍ଷର **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        ଫେରନ୍ତୁ r;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_strv_extend_strv(ଅକ୍ଷର ***a, ଅକ୍ଷର **b)
{
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା r;
        ଅକ୍ଷର **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                ଯଦି (r < 0)
                        ଫେରନ୍ତୁ r;
        }

        ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_strv_extend_strv_concat(ଅକ୍ଷର ***a, ଅକ୍ଷର **b, ଅଚଳ ଅକ୍ଷର *suffix)
{
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା r;
        ଅକ୍ଷର **s;

        UL_STRV_FOREACH(s, b) {
                ଅକ୍ଷର *v;

                v = ul_strconcat(*s, suffix);
                ଯଦି (!v)
                        ଫେରନ୍ତୁ -ENOMEM;

                r = ul_strv_ଠେଲ(a, v);
                ଯଦି (r < 0) {
                        ମୁକ୍ତ(v);
                        ଫେରନ୍ତୁ r;
                }
        }

        ଫେରନ୍ତୁ 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        ପାଇଁ ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

#define FOREACH_WORD_SEPARATOR(word, length, s, separator, state)       \
        _FOREACH_WORD(word, length, s, separator, false, state)


ଅକ୍ଷର **ul_strv_split(ଅଚଳ ଅକ୍ଷର *s, ଅଚଳ ଅକ୍ଷର *separator)
{
        ଅଚଳ ଅକ୍ଷର *word, *state;
        size_t l;
        unsigned n, i;
        ଅକ୍ଷର **r;

        ପ୍ରତିଜ୍ଞା(s);

        n = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state)
                n++;

        r = malloc(ଆକାର(ଅକ୍ଷର *) * (n + 1));
        ଯଦି (!r)
                ଫେରନ୍ତୁ ଖାଲି;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                ଯଦି (!r[i]) {
                        ul_strv_free(r);
                        ଫେରନ୍ତୁ ଖାଲି;
                }

                i++;
        }

        r[i] = ଖାଲି;
        ଫେରନ୍ତୁ r;
}

ଅକ୍ଷର *ul_strv_join(ଅକ୍ଷର **l, ଅଚଳ ଅକ୍ଷର *separator)
{
        ଅକ୍ଷର *r, *e;
        ଅକ୍ଷର **s;
        size_t n, k;

        ଯଦି (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                ଯଦି (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        ଯଦି (!r)
                ଫେରନ୍ତୁ ଖାଲି;

        e = r;
        UL_STRV_FOREACH(s, l) {
                ଯଦି (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        ଫେରନ୍ତୁ r;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_strv_ଠେଲ(ଅକ୍ଷର ***l, ଅକ୍ଷର *ମୂଲ୍ୟ)
{
        ଅକ୍ଷର **c;
        unsigned n, m;

        ଯଦି (!ମୂଲ୍ୟ)
                ଫେରନ୍ତୁ 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        ଯଦି (m < n)
                ଫେରନ୍ତୁ -ENOMEM;

        c = reallocarray(*l, m, ଆକାର(ଅକ୍ଷର *));
        ଯଦି (!c)
                ଫେରନ୍ତୁ -ENOMEM;

        c[n] = ମୂଲ୍ୟ;
        c[n+1] = ଖାଲି;

        *l = c;
        ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_strv_ଠେଲ_prepend(ଅକ୍ଷର ***l, ଅକ୍ଷର *ମୂଲ୍ୟ)
{
        ଅକ୍ଷର **c;
        unsigned n, m, i;

        ଯଦି (!ମୂଲ୍ୟ)
                ଫେରନ୍ତୁ 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        ଯଦି (m < n)
                ଫେରନ୍ତୁ -ENOMEM;

        c = malloc(ଆକାର(ଅକ୍ଷର *) * m);
        ଯଦି (!c)
                ଫେରନ୍ତୁ -ENOMEM;

        ପାଇଁ (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = ମୂଲ୍ୟ;
        c[n+1] = ଖାଲି;

        ମୁକ୍ତ(*l);
        *l = c;

        ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_strv_consume(ଅକ୍ଷର ***l, ଅକ୍ଷର *ମୂଲ୍ୟ)
{
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା r;

        r = ul_strv_ଠେଲ(l, ମୂଲ୍ୟ);
        ଯଦି (r < 0)
                ମୁକ୍ତ(ମୂଲ୍ୟ);

        ଫେରନ୍ତୁ r;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_strv_consume_prepend(ଅକ୍ଷର ***l, ଅକ୍ଷର *ମୂଲ୍ୟ)
{
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା r;

        r = ul_strv_ଠେଲ_prepend(l, ମୂଲ୍ୟ);
        ଯଦି (r < 0)
                ମୁକ୍ତ(ମୂଲ୍ୟ);

        ଫେରନ୍ତୁ r;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_strv_extend(ଅକ୍ଷର ***l, ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ)
{
        ଅକ୍ଷର *v;

        ଯଦି (!ମୂଲ୍ୟ)
                ଫେରନ୍ତୁ 0;

        v = strdup(ମୂଲ୍ୟ);
        ଯଦି (!v)
                ଫେରନ୍ତୁ -ENOMEM;

        ଫେରନ୍ତୁ ul_strv_consume(l, v);
}

ଅକ୍ଷର **ul_strv_remove(ଅକ୍ଷର **l, ଅଚଳ ଅକ୍ଷର *s)
{
        ଅକ୍ଷର **f, **t;

        ଯଦି (!l)
                ଫେରନ୍ତୁ ଖାଲି;

        ପ୍ରତିଜ୍ଞା(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        ପାଇଁ (f = t = l; *f; f++)
                ଯଦି (strcmp(*f, s) == 0)
                        ମୁକ୍ତ(*f);
                ଅନ୍ୟଥା
                        *(t++) = *f;

        *t = ଖାଲି;
        ଫେରନ୍ତୁ l;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_strv_extendf(ଅକ୍ଷର ***l, ଅଚଳ ଅକ୍ଷର *format, ...)
{
        va_list ap;
        ଅକ୍ଷର *x;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        ଯଦି (r < 0)
                ଫେରନ୍ତୁ -ENOMEM;

        ଫେରନ୍ତୁ ul_strv_consume(l, x);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_strv_extendv(ଅକ୍ଷର ***l, ଅଚଳ ଅକ୍ଷର *format, va_list ap)
{
        ଅକ୍ଷର *x;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା r;

        r = vasprintf(&x, format, ap);
        ଯଦି (r < 0)
                ଫେରନ୍ତୁ -ENOMEM;

        ଫେରନ୍ତୁ ul_strv_consume(l, x);
}

ଅକ୍ଷର **ul_strv_reverse(ଅକ୍ଷର **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        ଯଦି (n <= 1)
                ଫେରନ୍ତୁ l;

        ପାଇଁ (i = 0; i < n / 2; i++) {
                ଅକ୍ଷର *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        ଫେରନ୍ତୁ l;
}
