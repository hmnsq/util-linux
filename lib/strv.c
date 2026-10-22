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

#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <assert.h>

#அந்தர்பாவ "strutils.h"
#அந்தர்பாவ "strv.h"

நிலையான வெறுமை ul_strv_clear(எழுத்து **l)
{
        எழுத்து **k;

        எனில் (!l)
                திரும்பு;

        க்கு (k = l; *k; k++)
                விடுவி(*k);

        *l = வெற்று;
}

எழுத்து **ul_strv_free(எழுத்து **l)
{
        ul_strv_clear(l);
        விடுவி(l);
        திரும்பு வெற்று;
}

எழுத்து **ul_strv_copy(எழுத்து * மாறிலி *l)
{
        எழுத்து **r, **k;

        k = r = malloc(அளவு(எழுத்து *) * (ul_strv_length(l) + 1));
        எனில் (!r)
                திரும்பு வெற்று;

        எனில் (l)
                க்கு (; *l; k++, l++) {
                        *k = strdup(*l);
                        எனில் (!*k) {
                                ul_strv_free(r);
                                திரும்பு வெற்று;
                        }
                }

        *k = வெற்று;
        திரும்பு r;
}

unsigned ul_strv_length(எழுத்து * மாறிலி *l) {
        unsigned n = 0;

        எனில் (!l)
                திரும்பு 0;

        க்கு (; *l; l++)
                n++;

        திரும்பு n;
}

நிலையான எழுத்து **ul_strv_new_ap(மாறிலி எழுத்து *x, va_list ap)
{
        மாறிலி எழுத்து *s;
        எழுத்து **a;
        unsigned n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        எனில் (x) {
                n = x == (மாறிலி எழுத்து*) -1 ? 0 : 1;

                va_copy(aq, ap);
                போது ((s = va_arg(aq, மாறிலி எழுத்து*))) {
                        எனில் (s == (மாறிலி எழுத்து*) -1)
                                தொடர்;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(அளவு(எழுத்து *) * (n + 1));
        எனில் (!a)
                திரும்பு வெற்று;

        எனில் (x) {
                எனில் (x != (மாறிலி எழுத்து*) -1) {
                        a[i] = strdup(x);
                        எனில் (!a[i])
                                செல் fail;
                        i++;
                }

                போது ((s = va_arg(ap, மாறிலி எழுத்து*))) {

                        எனில் (s == (மாறிலி எழுத்து*) -1)
                                தொடர்;

                        a[i] = strdup(s);
                        எனில் (!a[i])
                                செல் fail;

                        i++;
                }
        }

        a[i] = வெற்று;

        திரும்பு a;

fail:
        ul_strv_free(a);
        திரும்பு வெற்று;
}

எழுத்து **ul_strv_new(மாறிலி எழுத்து *x, ...)
{
        எழுத்து **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        திரும்பு r;
}

முழு ul_strv_extend_strv(எழுத்து ***a, எழுத்து **b)
{
        முழு r;
        எழுத்து **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                எனில் (r < 0)
                        திரும்பு r;
        }

        திரும்பு 0;
}

முழு ul_strv_extend_strv_concat(எழுத்து ***a, எழுத்து **b, மாறிலி எழுத்து *suffix)
{
        முழு r;
        எழுத்து **s;

        UL_STRV_FOREACH(s, b) {
                எழுத்து *v;

                v = ul_strconcat(*s, suffix);
                எனில் (!v)
                        திரும்பு -ENOMEM;

                r = ul_strv_தள்ளு(a, v);
                எனில் (r < 0) {
                        விடுவி(v);
                        திரும்பு r;
                }
        }

        திரும்பு 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        க்கு ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

#define FOREACH_WORD_SEPARATOR(word, length, s, separator, state)       \
        _FOREACH_WORD(word, length, s, separator, false, state)


எழுத்து **ul_strv_split(மாறிலி எழுத்து *s, மாறிலி எழுத்து *separator)
{
        மாறிலி எழுத்து *word, *state;
        size_t l;
        unsigned n, i;
        எழுத்து **r;

        ப்ரதிஜ்ஞா(s);

        n = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state)
                n++;

        r = malloc(அளவு(எழுத்து *) * (n + 1));
        எனில் (!r)
                திரும்பு வெற்று;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                எனில் (!r[i]) {
                        ul_strv_free(r);
                        திரும்பு வெற்று;
                }

                i++;
        }

        r[i] = வெற்று;
        திரும்பு r;
}

எழுத்து *ul_strv_join(எழுத்து **l, மாறிலி எழுத்து *separator)
{
        எழுத்து *r, *e;
        எழுத்து **s;
        size_t n, k;

        எனில் (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                எனில் (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        எனில் (!r)
                திரும்பு வெற்று;

        e = r;
        UL_STRV_FOREACH(s, l) {
                எனில் (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        திரும்பு r;
}

முழு ul_strv_தள்ளு(எழுத்து ***l, எழுத்து *மதிப்பு)
{
        எழுத்து **c;
        unsigned n, m;

        எனில் (!மதிப்பு)
                திரும்பு 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        எனில் (m < n)
                திரும்பு -ENOMEM;

        c = reallocarray(*l, m, அளவு(எழுத்து *));
        எனில் (!c)
                திரும்பு -ENOMEM;

        c[n] = மதிப்பு;
        c[n+1] = வெற்று;

        *l = c;
        திரும்பு 0;
}

முழு ul_strv_தள்ளு_prepend(எழுத்து ***l, எழுத்து *மதிப்பு)
{
        எழுத்து **c;
        unsigned n, m, i;

        எனில் (!மதிப்பு)
                திரும்பு 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        எனில் (m < n)
                திரும்பு -ENOMEM;

        c = malloc(அளவு(எழுத்து *) * m);
        எனில் (!c)
                திரும்பு -ENOMEM;

        க்கு (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = மதிப்பு;
        c[n+1] = வெற்று;

        விடுவி(*l);
        *l = c;

        திரும்பு 0;
}

முழு ul_strv_consume(எழுத்து ***l, எழுத்து *மதிப்பு)
{
        முழு r;

        r = ul_strv_தள்ளு(l, மதிப்பு);
        எனில் (r < 0)
                விடுவி(மதிப்பு);

        திரும்பு r;
}

முழு ul_strv_consume_prepend(எழுத்து ***l, எழுத்து *மதிப்பு)
{
        முழு r;

        r = ul_strv_தள்ளு_prepend(l, மதிப்பு);
        எனில் (r < 0)
                விடுவி(மதிப்பு);

        திரும்பு r;
}

முழு ul_strv_extend(எழுத்து ***l, மாறிலி எழுத்து *மதிப்பு)
{
        எழுத்து *v;

        எனில் (!மதிப்பு)
                திரும்பு 0;

        v = strdup(மதிப்பு);
        எனில் (!v)
                திரும்பு -ENOMEM;

        திரும்பு ul_strv_consume(l, v);
}

எழுத்து **ul_strv_remove(எழுத்து **l, மாறிலி எழுத்து *s)
{
        எழுத்து **f, **t;

        எனில் (!l)
                திரும்பு வெற்று;

        ப்ரதிஜ்ஞா(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        க்கு (f = t = l; *f; f++)
                எனில் (strcmp(*f, s) == 0)
                        விடுவி(*f);
                இல்லையெனில்
                        *(t++) = *f;

        *t = வெற்று;
        திரும்பு l;
}

முழு ul_strv_extendf(எழுத்து ***l, மாறிலி எழுத்து *format, ...)
{
        va_list ap;
        எழுத்து *x;
        முழு r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        எனில் (r < 0)
                திரும்பு -ENOMEM;

        திரும்பு ul_strv_consume(l, x);
}

முழு ul_strv_extendv(எழுத்து ***l, மாறிலி எழுத்து *format, va_list ap)
{
        எழுத்து *x;
        முழு r;

        r = vasprintf(&x, format, ap);
        எனில் (r < 0)
                திரும்பு -ENOMEM;

        திரும்பு ul_strv_consume(l, x);
}

எழுத்து **ul_strv_reverse(எழுத்து **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        எனில் (n <= 1)
                திரும்பு l;

        க்கு (i = 0; i < n / 2; i++) {
                எழுத்து *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        திரும்பு l;
}
