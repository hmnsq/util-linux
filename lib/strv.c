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

#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdarg.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <assert.h>

#അന്തര്ഭാവ "strutils.h"
#അന്തര്ഭാവ "strv.h"

സ്ഥിതം ശൂന്യം ul_strv_clear(അക്ഷരം **l)
{
        അക്ഷരം **k;

        എങ്കിൽ (!l)
                തിരികെ;

        വേണ്ടി (k = l; *k; k++)
                സ്വതന്ത്രം(*k);

        *l = ശൂന്യ;
}

അക്ഷരം **ul_strv_free(അക്ഷരം **l)
{
        ul_strv_clear(l);
        സ്വതന്ത്രം(l);
        തിരികെ ശൂന്യ;
}

അക്ഷരം **ul_strv_copy(അക്ഷരം * സ്ഥിരം *l)
{
        അക്ഷരം **r, **k;

        k = r = malloc(വലുപ്പം(അക്ഷരം *) * (ul_strv_length(l) + 1));
        എങ്കിൽ (!r)
                തിരികെ ശൂന്യ;

        എങ്കിൽ (l)
                വേണ്ടി (; *l; k++, l++) {
                        *k = strdup(*l);
                        എങ്കിൽ (!*k) {
                                ul_strv_free(r);
                                തിരികെ ശൂന്യ;
                        }
                }

        *k = ശൂന്യ;
        തിരികെ r;
}

unsigned ul_strv_length(അക്ഷരം * സ്ഥിരം *l) {
        unsigned n = 0;

        എങ്കിൽ (!l)
                തിരികെ 0;

        വേണ്ടി (; *l; l++)
                n++;

        തിരികെ n;
}

സ്ഥിതം അക്ഷരം **ul_strv_new_ap(സ്ഥിരം അക്ഷരം *x, va_list ap)
{
        സ്ഥിരം അക്ഷരം *s;
        അക്ഷരം **a;
        unsigned n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        എങ്കിൽ (x) {
                n = x == (സ്ഥിരം അക്ഷരം*) -1 ? 0 : 1;

                va_copy(aq, ap);
                ആകുമ്പോൾ ((s = va_arg(aq, സ്ഥിരം അക്ഷരം*))) {
                        എങ്കിൽ (s == (സ്ഥിരം അക്ഷരം*) -1)
                                തുടരുക;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(വലുപ്പം(അക്ഷരം *) * (n + 1));
        എങ്കിൽ (!a)
                തിരികെ ശൂന്യ;

        എങ്കിൽ (x) {
                എങ്കിൽ (x != (സ്ഥിരം അക്ഷരം*) -1) {
                        a[i] = strdup(x);
                        എങ്കിൽ (!a[i])
                                പോകുക fail;
                        i++;
                }

                ആകുമ്പോൾ ((s = va_arg(ap, സ്ഥിരം അക്ഷരം*))) {

                        എങ്കിൽ (s == (സ്ഥിരം അക്ഷരം*) -1)
                                തുടരുക;

                        a[i] = strdup(s);
                        എങ്കിൽ (!a[i])
                                പോകുക fail;

                        i++;
                }
        }

        a[i] = ശൂന്യ;

        തിരികെ a;

fail:
        ul_strv_free(a);
        തിരികെ ശൂന്യ;
}

അക്ഷരം **ul_strv_new(സ്ഥിരം അക്ഷരം *x, ...)
{
        അക്ഷരം **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        തിരികെ r;
}

പൂർണ്ണസംഖ്യ ul_strv_extend_strv(അക്ഷരം ***a, അക്ഷരം **b)
{
        പൂർണ്ണസംഖ്യ r;
        അക്ഷരം **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                എങ്കിൽ (r < 0)
                        തിരികെ r;
        }

        തിരികെ 0;
}

പൂർണ്ണസംഖ്യ ul_strv_extend_strv_concat(അക്ഷരം ***a, അക്ഷരം **b, സ്ഥിരം അക്ഷരം *suffix)
{
        പൂർണ്ണസംഖ്യ r;
        അക്ഷരം **s;

        UL_STRV_FOREACH(s, b) {
                അക്ഷരം *v;

                v = ul_strconcat(*s, suffix);
                എങ്കിൽ (!v)
                        തിരികെ -ENOMEM;

                r = ul_strv_തള്ളുക(a, v);
                എങ്കിൽ (r < 0) {
                        സ്വതന്ത്രം(v);
                        തിരികെ r;
                }
        }

        തിരികെ 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        വേണ്ടി ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

#define FOREACH_WORD_SEPARATOR(word, length, s, separator, state)       \
        _FOREACH_WORD(word, length, s, separator, false, state)


അക്ഷരം **ul_strv_split(സ്ഥിരം അക്ഷരം *s, സ്ഥിരം അക്ഷരം *separator)
{
        സ്ഥിരം അക്ഷരം *word, *state;
        size_t l;
        unsigned n, i;
        അക്ഷരം **r;

        പ്രതിജ്ഞാ(s);

        n = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state)
                n++;

        r = malloc(വലുപ്പം(അക്ഷരം *) * (n + 1));
        എങ്കിൽ (!r)
                തിരികെ ശൂന്യ;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                എങ്കിൽ (!r[i]) {
                        ul_strv_free(r);
                        തിരികെ ശൂന്യ;
                }

                i++;
        }

        r[i] = ശൂന്യ;
        തിരികെ r;
}

അക്ഷരം *ul_strv_join(അക്ഷരം **l, സ്ഥിരം അക്ഷരം *separator)
{
        അക്ഷരം *r, *e;
        അക്ഷരം **s;
        size_t n, k;

        എങ്കിൽ (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                എങ്കിൽ (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        എങ്കിൽ (!r)
                തിരികെ ശൂന്യ;

        e = r;
        UL_STRV_FOREACH(s, l) {
                എങ്കിൽ (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        തിരികെ r;
}

പൂർണ്ണസംഖ്യ ul_strv_തള്ളുക(അക്ഷരം ***l, അക്ഷരം *മൂല്യം)
{
        അക്ഷരം **c;
        unsigned n, m;

        എങ്കിൽ (!മൂല്യം)
                തിരികെ 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        എങ്കിൽ (m < n)
                തിരികെ -ENOMEM;

        c = reallocarray(*l, m, വലുപ്പം(അക്ഷരം *));
        എങ്കിൽ (!c)
                തിരികെ -ENOMEM;

        c[n] = മൂല്യം;
        c[n+1] = ശൂന്യ;

        *l = c;
        തിരികെ 0;
}

പൂർണ്ണസംഖ്യ ul_strv_തള്ളുക_prepend(അക്ഷരം ***l, അക്ഷരം *മൂല്യം)
{
        അക്ഷരം **c;
        unsigned n, m, i;

        എങ്കിൽ (!മൂല്യം)
                തിരികെ 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        എങ്കിൽ (m < n)
                തിരികെ -ENOMEM;

        c = malloc(വലുപ്പം(അക്ഷരം *) * m);
        എങ്കിൽ (!c)
                തിരികെ -ENOMEM;

        വേണ്ടി (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = മൂല്യം;
        c[n+1] = ശൂന്യ;

        സ്വതന്ത്രം(*l);
        *l = c;

        തിരികെ 0;
}

പൂർണ്ണസംഖ്യ ul_strv_consume(അക്ഷരം ***l, അക്ഷരം *മൂല്യം)
{
        പൂർണ്ണസംഖ്യ r;

        r = ul_strv_തള്ളുക(l, മൂല്യം);
        എങ്കിൽ (r < 0)
                സ്വതന്ത്രം(മൂല്യം);

        തിരികെ r;
}

പൂർണ്ണസംഖ്യ ul_strv_consume_prepend(അക്ഷരം ***l, അക്ഷരം *മൂല്യം)
{
        പൂർണ്ണസംഖ്യ r;

        r = ul_strv_തള്ളുക_prepend(l, മൂല്യം);
        എങ്കിൽ (r < 0)
                സ്വതന്ത്രം(മൂല്യം);

        തിരികെ r;
}

പൂർണ്ണസംഖ്യ ul_strv_extend(അക്ഷരം ***l, സ്ഥിരം അക്ഷരം *മൂല്യം)
{
        അക്ഷരം *v;

        എങ്കിൽ (!മൂല്യം)
                തിരികെ 0;

        v = strdup(മൂല്യം);
        എങ്കിൽ (!v)
                തിരികെ -ENOMEM;

        തിരികെ ul_strv_consume(l, v);
}

അക്ഷരം **ul_strv_remove(അക്ഷരം **l, സ്ഥിരം അക്ഷരം *s)
{
        അക്ഷരം **f, **t;

        എങ്കിൽ (!l)
                തിരികെ ശൂന്യ;

        പ്രതിജ്ഞാ(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        വേണ്ടി (f = t = l; *f; f++)
                എങ്കിൽ (strcmp(*f, s) == 0)
                        സ്വതന്ത്രം(*f);
                അല്ലെങ്കിൽ
                        *(t++) = *f;

        *t = ശൂന്യ;
        തിരികെ l;
}

പൂർണ്ണസംഖ്യ ul_strv_extendf(അക്ഷരം ***l, സ്ഥിരം അക്ഷരം *format, ...)
{
        va_list ap;
        അക്ഷരം *x;
        പൂർണ്ണസംഖ്യ r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        എങ്കിൽ (r < 0)
                തിരികെ -ENOMEM;

        തിരികെ ul_strv_consume(l, x);
}

പൂർണ്ണസംഖ്യ ul_strv_extendv(അക്ഷരം ***l, സ്ഥിരം അക്ഷരം *format, va_list ap)
{
        അക്ഷരം *x;
        പൂർണ്ണസംഖ്യ r;

        r = vasprintf(&x, format, ap);
        എങ്കിൽ (r < 0)
                തിരികെ -ENOMEM;

        തിരികെ ul_strv_consume(l, x);
}

അക്ഷരം **ul_strv_reverse(അക്ഷരം **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        എങ്കിൽ (n <= 1)
                തിരികെ l;

        വേണ്ടി (i = 0; i < n / 2; i++) {
                അക്ഷരം *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        തിരികെ l;
}
