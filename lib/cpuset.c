/*
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Terminology:
 *
 *	cpuset	- (libc) cpu_set_t data structure represents set of CPUs
 *	cpumask	- string with hex mask (e.g. "0x00000001")
 *	cpulist - string with CPU ranges (e.g. "0-3,5,7,8")
 *
 * Based on code from taskset.c and Linux kernel.
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2010 Karel Zak <kzak@redhat.com>
 */

#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <sched.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <ctype.h>
#ifdef HAVE_SYS_SYSCALL_H
#अन्तर्भाव <sys/syscall.h>
#endif

#अन्तर्भाव "cpuset.h"
#अन्तर्भाव "c.h"

स्थिर अन्तःस्थ पूर्णाङ्क val_to_char(पूर्णाङ्क v)
{
	यदि (v >= 0 && v < 10)
		फर्काऊ '0' + v;
	यदि (v >= 10 && v < 16)
		फर्काऊ ('a' - 10) + v;
	फर्काऊ -1;
}

स्थिर अन्तःस्थ पूर्णाङ्क char_to_val(पूर्णाङ्क c)
{
	पूर्णाङ्क cl;

	यदि (c >= '0' && c <= '9')
		फर्काऊ c - '0';
	cl = छोटे(c);
	यदि (cl >= 'a' && cl <= 'f')
		फर्काऊ cl + (10 - 'a');
	फर्काऊ -1;
}

स्थिर अचर वर्ण *nexttoken(अचर वर्ण *q,  पूर्णाङ्क sep)
{
	यदि (q)
		q = म_अक्षर(q, sep);
	यदि (q)
		q++;
	फर्काऊ q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
पूर्णाङ्क get_max_सङ्ख्या_of_cpus(शून्य)
{
#ifdef SYS_sched_getaffinity
	पूर्णाङ्क n, cpus = 2048;
	माप_प्रकार setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, रिक्त);

	यदि (!set)
		फर्काऊ -1;	/* error */

	लागि (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		यदि (n < 0 && त्रुटि_सं == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, रिक्त);
			यदि (!set)
				फर्काऊ -1;	/* error */
			जारी;
		}
		cpuset_free(set);
		फर्काऊ n * 8;
	}
#endif
	फर्काऊ -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(पूर्णाङ्क ncpus, माप_प्रकार *setsize, माप_प्रकार *nbits)
{
	cpu_set_t *set = रिक्त;

	यदि (ncpus)
		set = CPU_ALLOC(ncpus);
	यदि (!set)
		फर्काऊ रिक्त;
	यदि (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	यदि (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	फर्काऊ set;
}

शून्य cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#यदि !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
पूर्णाङ्क __cpuset_count_s(माप_प्रकार setsize, अचर cpu_set_t *set)
{
	पूर्णाङ्क s = 0;
	अचर __cpu_mask *p = set->__bits;
	अचर __cpu_mask *end = &set->__bits[setsize / परिमाण (__cpu_mask)];

	जबसम्म (p < end) {
		__cpu_mask l = *p++;

		यदि (l == 0)
			जारी;
# यदि LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# अन्यथा
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	फर्काऊ s;
}
#endif

/*
 * Finds the first CPU present after the specified index.
 *
 * start: starting index, inclusive.
 * setsize: size of the set in *bytes*.
 * set: CPU set to search.
 *
 * Return: the index of the first CPU present in `set`, starting at `start`.
 * If no such CPU exists, returns the size of the set in *bits*.
 */
स्थिर माप_प्रकार find_next_cpu(माप_प्रकार start, माप_प्रकार setsize, cpu_set_t *set)
{
	माप_प्रकार nbits = cpuset_nbits(setsize);
	लागि (; start < nbits; start++)
		यदि (CPU_ISSET_S(start, setsize, set))
			फर्काऊ start;
	फर्काऊ start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
वर्ण *cpulist_create(वर्ण *str, माप_प्रकार len,
			cpu_set_t *set, माप_प्रकार setsize)
{
	वर्ण *ptr = str;
	पूर्णाङ्क entry_made = 0;
	माप_प्रकार max = cpuset_nbits(setsize);
	माप_प्रकार a = 0;  /* min for cpu range */
	माप_प्रकार next = 0;  /* where to start looking for next cpu */

	जबसम्म ((a = find_next_cpu(next, setsize, set)) < max) {
		पूर्णाङ्क rlen;
		next = find_next_cpu(a + 1, setsize, set);
		यदि (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} अन्यथा {
			/* Extend range as long as we have the same stride. */
			माप_प्रकार b = next;
			माप_प्रकार s = b - a;
			जबसम्म (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			यदि (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} अन्यथा यदि (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} अन्यथा {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		यदि (rlen < 0 || (माप_प्रकार) rlen >= len)
			फर्काऊ रिक्त;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	फर्काऊ str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
वर्ण *cpulist_flat_create(वर्ण *str, माप_प्रकार len,
			    cpu_set_t *set, माप_प्रकार setsize)
{
	वर्ण *ptr = str;
	पूर्णाङ्क entry_made = 0;
	माप_प्रकार max = cpuset_nbits(setsize);
	माप_प्रकार cpu;
	पूर्णाङ्क rlen;

	लागि (cpu = 0; cpu < max; cpu++) {
		यदि (!CPU_ISSET_S(cpu, setsize, set))
			जारी;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		यदि (rlen < 0 || (माप_प्रकार)rlen >= len)
			फर्काऊ रिक्त;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	फर्काऊ str;
}


/*
 * Returns string with CPU mask.
 */
वर्ण *cpumask_create(वर्ण *str, माप_प्रकार len,
			cpu_set_t *set, माप_प्रकार setsize)
{
	वर्ण *ptr = str;
	वर्ण *ret = रिक्त;
	पूर्णाङ्क cpu;

	लागि (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		वर्ण val = 0;

		यदि (len == (माप_प्रकार) (ptr - str))
			रोक;

		यदि (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		यदि (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		यदि (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		यदि (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		यदि (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	फर्काऊ ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
पूर्णाङ्क cpumask_parse(अचर वर्ण *str, cpu_set_t *set, माप_प्रकार setsize)
{
	पूर्णाङ्क len = म_माप(str);
	अचर वर्ण *ptr = str + len - 1;
	पूर्णाङ्क cpu = 0;

	/* skip 0x, it's all hex anyway */
	यदि (len > 1 && !स_भेद(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	जबसम्म (ptr >= str) {
		वर्ण val;

		/* cpu masks in /sys uses comma as a separator */
		यदि (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		यदि (val == (वर्ण) -1)
			फर्काऊ -1;
		यदि (val & 1)
			CPU_SET_S(cpu, setsize, set);
		यदि (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		यदि (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		यदि (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	फर्काऊ 0;
}

स्थिर पूर्णाङ्क nextnumber(अचर वर्ण *str, वर्ण **end, अचिन्हित पूर्णाङ्क *result)
{
	त्रुटि_सं = 0;
	यदि (str == रिक्त || *str == '\0' || !है_अंक(*str))
		फर्काऊ -EINVAL;
	*result = (अचिन्हित पूर्णाङ्क) म_से_अदीर्घ(str, end, 10);
	यदि (त्रुटि_सं)
		फर्काऊ -त्रुटि_सं;
	यदि (str == *end)
		फर्काऊ -EINVAL;
	फर्काऊ 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
पूर्णाङ्क cpulist_parse(अचर वर्ण *str, cpu_set_t *set, माप_प्रकार setsize, पूर्णाङ्क fail)
{
	अचर माप_प्रकार max = cpuset_nbits(setsize);
	अचर वर्ण *p, *q;
	वर्ण *end = रिक्त;

	q = str;
	CPU_ZERO_S(setsize, set);

	जबसम्म (p = q, q = nexttoken(q, ','), p) {
		अचिन्हित पूर्णाङ्क a;	/* beginning of range */
		अचिन्हित पूर्णाङ्क b;	/* end of range */
		अचिन्हित पूर्णाङ्क s;	/* stride */
		अचर वर्ण *c1, *c2;

		यदि (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			फर्काऊ 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		यदि (c1 != रिक्त && (c2 == रिक्त || c1 < c2)) {
			यदि (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				फर्काऊ 1;

			c1 = nexttoken(end, ':');

			यदि (c1 != रिक्त && (c2 == रिक्त || c1 < c2)) {
				यदि (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					फर्काऊ 1;
				यदि (s == 0)
					फर्काऊ 1;
			}
		}

		यदि (!(a <= b))
			फर्काऊ 1;
		जबसम्म (a <= b) {
			यदि (a >= max) {
				यदि (fail)
					फर्काऊ 2;
				अन्यथा
					रोक;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	यदि (end && *end)
		फर्काऊ 1;
	फर्काऊ 0;
}

#ifdef TEST_PROGRAM_CPUSET

#अन्तर्भाव <getopt.h>

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	cpu_set_t *set;
	माप_प्रकार setsize, buflen, nbits;
	वर्ण *buf, *mask = रिक्त, *range = रिक्त;
	पूर्णाङ्क ncpus = 2048, rc, c;

	स्थिर अचर संरचना option longopts[] = {
	    { "ncpus", 1, रिक्त, 'n' },
	    { "mask",  1, रिक्त, 'm' },
	    { "range", 1, रिक्त, 'r' },
	    { रिक्त,    0, रिक्त, 0 }
	};

	जबसम्म ((c = getopt_long(argc, argv, "n:m:r:", longopts, रिक्त)) != -1) {
		विकल्प(c) {
		पक्ष 'n':
			ncpus = म_से_प(optarg);
			रोक;
		पक्ष 'm':
			mask = optarg;
			रोक;
		पक्ष 'r':
			range = optarg;
			रोक;
		स्वभाव:
			गच्छ usage_err;
		}
	}

	यदि (!mask && !range)
		गच्छ usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	यदि (!set)
		err(निकास_त्रुटि, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = दो_स्मृति(buflen);
	यदि (!buf)
		err(निकास_त्रुटि, "failed to allocate cpu set buffer");

	यदि (mask)
		rc = cpumask_parse(mask, set, setsize);
	अन्यथा
		rc = cpulist_parse(range, set, setsize, 0);

	यदि (rc)
		errx(निकास_त्रुटि, "failed to parse string: %s", mask ? : range);

	म_लिखो("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	म_लिखो("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	मुक्त(buf);
	cpuset_free(set);

	फर्काऊ निकास_सफल;

usage_err:
	ख_लिखो(मानक_त्रुटि,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	निकास(निकास_त्रुटि);
}
#endif
