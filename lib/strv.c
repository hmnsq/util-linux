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

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <assert.h>

#अन्तर्भाव "strutils.h"
#अन्तर्भाव "strv.h"

स्थिर शून्य ul_strv_clear(वर्ण **l)
{
        वर्ण **k;

        यदि (!l)
                फर्काऊ;

        लागि (k = l; *k; k++)
                मुक्त(*k);

        *l = रिक्त;
}

वर्ण **ul_strv_free(वर्ण **l)
{
        ul_strv_clear(l);
        मुक्त(l);
        फर्काऊ रिक्त;
}

वर्ण **ul_strv_copy(वर्ण * अचर *l)
{
        वर्ण **r, **k;

        k = r = दो_स्मृति(परिमाण(वर्ण *) * (ul_strv_length(l) + 1));
        यदि (!r)
                फर्काऊ रिक्त;

        यदि (l)
                लागि (; *l; k++, l++) {
                        *k = strdup(*l);
                        यदि (!*k) {
                                ul_strv_free(r);
                                फर्काऊ रिक्त;
                        }
                }

        *k = रिक्त;
        फर्काऊ r;
}

अचिन्हित ul_strv_length(वर्ण * अचर *l) {
        अचिन्हित n = 0;

        यदि (!l)
                फर्काऊ 0;

        लागि (; *l; l++)
                n++;

        फर्काऊ n;
}

स्थिर वर्ण **ul_strv_new_ap(अचर वर्ण *x, बहु_सूची ap)
{
        अचर वर्ण *s;
        वर्ण **a;
        अचिन्हित n = 0, i = 0;
        बहु_सूची aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        यदि (x) {
                n = x == (अचर वर्ण*) -1 ? 0 : 1;

                va_copy(aq, ap);
                जबसम्म ((s = बहु_तर्क(aq, अचर वर्ण*))) {
                        यदि (s == (अचर वर्ण*) -1)
                                जारी;

                        n++;
                }

                बहु_पूर्ण(aq);
        }

        a = दो_स्मृति(परिमाण(वर्ण *) * (n + 1));
        यदि (!a)
                फर्काऊ रिक्त;

        यदि (x) {
                यदि (x != (अचर वर्ण*) -1) {
                        a[i] = strdup(x);
                        यदि (!a[i])
                                गच्छ fail;
                        i++;
                }

                जबसम्म ((s = बहु_तर्क(ap, अचर वर्ण*))) {

                        यदि (s == (अचर वर्ण*) -1)
                                जारी;

                        a[i] = strdup(s);
                        यदि (!a[i])
                                गच्छ fail;

                        i++;
                }
        }

        a[i] = रिक्त;

        फर्काऊ a;

fail:
        ul_strv_free(a);
        फर्काऊ रिक्त;
}

वर्ण **ul_strv_new(अचर वर्ण *x, ...)
{
        वर्ण **r;
        बहु_सूची ap;

        बहु_शुरू(ap, x);
        r = ul_strv_new_ap(x, ap);
        बहु_पूर्ण(ap);

        फर्काऊ r;
}

पूर्णाङ्क ul_strv_extend_strv(वर्ण ***a, वर्ण **b)
{
        पूर्णाङ्क r;
        वर्ण **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                यदि (r < 0)
                        फर्काऊ r;
        }

        फर्काऊ 0;
}

पूर्णाङ्क ul_strv_extend_strv_concat(वर्ण ***a, वर्ण **b, अचर वर्ण *suffix)
{
        पूर्णाङ्क r;
        वर्ण **s;

        UL_STRV_FOREACH(s, b) {
                वर्ण *v;

                v = ul_strconcat(*s, suffix);
                यदि (!v)
                        फर्काऊ -ENOMEM;

                r = ul_strv_थप(a, v);
                यदि (r < 0) {
                        मुक्त(v);
                        फर्काऊ r;
                }
        }

        फर्काऊ 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        लागि ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

#define FOREACH_WORD_SEPARATOR(word, length, s, separator, state)       \
        _FOREACH_WORD(word, length, s, separator, false, state)


वर्ण **ul_strv_split(अचर वर्ण *s, अचर वर्ण *separator)
{
        अचर वर्ण *word, *state;
        माप_प्रकार l;
        अचिन्हित n, i;
        वर्ण **r;

        प्रतिज्ञा(s);

        n = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state)
                n++;

        r = दो_स्मृति(परिमाण(वर्ण *) * (n + 1));
        यदि (!r)
                फर्काऊ रिक्त;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                यदि (!r[i]) {
                        ul_strv_free(r);
                        फर्काऊ रिक्त;
                }

                i++;
        }

        r[i] = रिक्त;
        फर्काऊ r;
}

वर्ण *ul_strv_join(वर्ण **l, अचर वर्ण *separator)
{
        वर्ण *r, *e;
        वर्ण **s;
        माप_प्रकार n, k;

        यदि (!separator)
                separator = " ";

        k = म_माप(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                यदि (n != 0)
                        n += k;
                n += म_माप(*s);
        }

        r = दो_स्मृति(n + 1);
        यदि (!r)
                फर्काऊ रिक्त;

        e = r;
        UL_STRV_FOREACH(s, l) {
                यदि (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        फर्काऊ r;
}

पूर्णाङ्क ul_strv_थप(वर्ण ***l, वर्ण *मान)
{
        वर्ण **c;
        अचिन्हित n, m;

        यदि (!मान)
                फर्काऊ 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        यदि (m < n)
                फर्काऊ -ENOMEM;

        c = reallocarray(*l, m, परिमाण(वर्ण *));
        यदि (!c)
                फर्काऊ -ENOMEM;

        c[n] = मान;
        c[n+1] = रिक्त;

        *l = c;
        फर्काऊ 0;
}

पूर्णाङ्क ul_strv_थप_prepend(वर्ण ***l, वर्ण *मान)
{
        वर्ण **c;
        अचिन्हित n, m, i;

        यदि (!मान)
                फर्काऊ 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        यदि (m < n)
                फर्काऊ -ENOMEM;

        c = दो_स्मृति(परिमाण(वर्ण *) * m);
        यदि (!c)
                फर्काऊ -ENOMEM;

        लागि (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = मान;
        c[n+1] = रिक्त;

        मुक्त(*l);
        *l = c;

        फर्काऊ 0;
}

पूर्णाङ्क ul_strv_consume(वर्ण ***l, वर्ण *मान)
{
        पूर्णाङ्क r;

        r = ul_strv_थप(l, मान);
        यदि (r < 0)
                मुक्त(मान);

        फर्काऊ r;
}

पूर्णाङ्क ul_strv_consume_prepend(वर्ण ***l, वर्ण *मान)
{
        पूर्णाङ्क r;

        r = ul_strv_थप_prepend(l, मान);
        यदि (r < 0)
                मुक्त(मान);

        फर्काऊ r;
}

पूर्णाङ्क ul_strv_extend(वर्ण ***l, अचर वर्ण *मान)
{
        वर्ण *v;

        यदि (!मान)
                फर्काऊ 0;

        v = strdup(मान);
        यदि (!v)
                फर्काऊ -ENOMEM;

        फर्काऊ ul_strv_consume(l, v);
}

वर्ण **ul_strv_remove(वर्ण **l, अचर वर्ण *s)
{
        वर्ण **f, **t;

        यदि (!l)
                फर्काऊ रिक्त;

        प्रतिज्ञा(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        लागि (f = t = l; *f; f++)
                यदि (म_भेद(*f, s) == 0)
                        मुक्त(*f);
                अन्यथा
                        *(t++) = *f;

        *t = रिक्त;
        फर्काऊ l;
}

पूर्णाङ्क ul_strv_extendf(वर्ण ***l, अचर वर्ण *format, ...)
{
        बहु_सूची ap;
        वर्ण *x;
        पूर्णाङ्क r;

        बहु_शुरू(ap, format);
        r = vasprintf(&x, format, ap);
        बहु_पूर्ण(ap);

        यदि (r < 0)
                फर्काऊ -ENOMEM;

        फर्काऊ ul_strv_consume(l, x);
}

पूर्णाङ्क ul_strv_extendv(वर्ण ***l, अचर वर्ण *format, बहु_सूची ap)
{
        वर्ण *x;
        पूर्णाङ्क r;

        r = vasprintf(&x, format, ap);
        यदि (r < 0)
                फर्काऊ -ENOMEM;

        फर्काऊ ul_strv_consume(l, x);
}

वर्ण **ul_strv_reverse(वर्ण **l)
{
        अचिन्हित n, i;

        n = ul_strv_length(l);
        यदि (n <= 1)
                फर्काऊ l;

        लागि (i = 0; i < n / 2; i++) {
                वर्ण *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        फर्काऊ l;
}
