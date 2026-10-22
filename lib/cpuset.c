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

स्थिर ओळीत पूर्णांक val_to_char(पूर्णांक v)
{
	जर (v >= 0 && v < 10)
		परत '0' + v;
	जर (v >= 10 && v < 16)
		परत ('a' - 10) + v;
	परत -1;
}

स्थिर ओळीत पूर्णांक char_to_val(पूर्णांक c)
{
	पूर्णांक cl;

	जर (c >= '0' && c <= '9')
		परत c - '0';
	cl = छोटे(c);
	जर (cl >= 'a' && cl <= 'f')
		परत cl + (10 - 'a');
	परत -1;
}

स्थिर स्थिरांक अक्षर *nexttoken(स्थिरांक अक्षर *q,  पूर्णांक sep)
{
	जर (q)
		q = म_अक्षर(q, sep);
	जर (q)
		q++;
	परत q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
पूर्णांक get_max_संख्या_of_cpus(शून्य)
{
#ifdef SYS_sched_getaffinity
	पूर्णांक n, cpus = 2048;
	माप_प्रकार setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, रिक्त);

	जर (!set)
		परत -1;	/* error */

	साठी (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		जर (n < 0 && त्रुटि_सं == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, रिक्त);
			जर (!set)
				परत -1;	/* error */
			सुरूठेवा;
		}
		cpuset_free(set);
		परत n * 8;
	}
#endif
	परत -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(पूर्णांक ncpus, माप_प्रकार *setsize, माप_प्रकार *nbits)
{
	cpu_set_t *set = रिक्त;

	जर (ncpus)
		set = CPU_ALLOC(ncpus);
	जर (!set)
		परत रिक्त;
	जर (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	जर (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	परत set;
}

शून्य cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#जर !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
पूर्णांक __cpuset_count_s(माप_प्रकार setsize, स्थिरांक cpu_set_t *set)
{
	पूर्णांक s = 0;
	स्थिरांक __cpu_mask *p = set->__bits;
	स्थिरांक __cpu_mask *end = &set->__bits[setsize / आकार (__cpu_mask)];

	पर्यंत (p < end) {
		__cpu_mask l = *p++;

		जर (l == 0)
			सुरूठेवा;
# जर LONG_BIT > 32
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
	परत s;
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
	साठी (; start < nbits; start++)
		जर (CPU_ISSET_S(start, setsize, set))
			परत start;
	परत start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
अक्षर *cpulist_create(अक्षर *str, माप_प्रकार len,
			cpu_set_t *set, माप_प्रकार setsize)
{
	अक्षर *ptr = str;
	पूर्णांक entry_made = 0;
	माप_प्रकार max = cpuset_nbits(setsize);
	माप_प्रकार a = 0;  /* min for cpu range */
	माप_प्रकार next = 0;  /* where to start looking for next cpu */

	पर्यंत ((a = find_next_cpu(next, setsize, set)) < max) {
		पूर्णांक rlen;
		next = find_next_cpu(a + 1, setsize, set);
		जर (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} अन्यथा {
			/* Extend range as long as we have the same stride. */
			माप_प्रकार b = next;
			माप_प्रकार s = b - a;
			पर्यंत (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			जर (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} अन्यथा जर (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} अन्यथा {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		जर (rlen < 0 || (माप_प्रकार) rlen >= len)
			परत रिक्त;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	परत str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
अक्षर *cpulist_flat_create(अक्षर *str, माप_प्रकार len,
			    cpu_set_t *set, माप_प्रकार setsize)
{
	अक्षर *ptr = str;
	पूर्णांक entry_made = 0;
	माप_प्रकार max = cpuset_nbits(setsize);
	माप_प्रकार cpu;
	पूर्णांक rlen;

	साठी (cpu = 0; cpu < max; cpu++) {
		जर (!CPU_ISSET_S(cpu, setsize, set))
			सुरूठेवा;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		जर (rlen < 0 || (माप_प्रकार)rlen >= len)
			परत रिक्त;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	परत str;
}


/*
 * Returns string with CPU mask.
 */
अक्षर *cpumask_create(अक्षर *str, माप_प्रकार len,
			cpu_set_t *set, माप_प्रकार setsize)
{
	अक्षर *ptr = str;
	अक्षर *ret = रिक्त;
	पूर्णांक cpu;

	साठी (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		अक्षर val = 0;

		जर (len == (माप_प्रकार) (ptr - str))
			थांबा;

		जर (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		जर (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		जर (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		जर (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		जर (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	परत ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
पूर्णांक cpumask_parse(स्थिरांक अक्षर *str, cpu_set_t *set, माप_प्रकार setsize)
{
	पूर्णांक len = म_माप(str);
	स्थिरांक अक्षर *ptr = str + len - 1;
	पूर्णांक cpu = 0;

	/* skip 0x, it's all hex anyway */
	जर (len > 1 && !स_भेद(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	पर्यंत (ptr >= str) {
		अक्षर val;

		/* cpu masks in /sys uses comma as a separator */
		जर (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		जर (val == (अक्षर) -1)
			परत -1;
		जर (val & 1)
			CPU_SET_S(cpu, setsize, set);
		जर (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		जर (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		जर (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	परत 0;
}

स्थिर पूर्णांक nextnumber(स्थिरांक अक्षर *str, अक्षर **end, अचिन्हित पूर्णांक *result)
{
	त्रुटि_सं = 0;
	जर (str == रिक्त || *str == '\0' || !है_अंक(*str))
		परत -EINVAL;
	*result = (अचिन्हित पूर्णांक) म_से_अदीर्घ(str, end, 10);
	जर (त्रुटि_सं)
		परत -त्रुटि_सं;
	जर (str == *end)
		परत -EINVAL;
	परत 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
पूर्णांक cpulist_parse(स्थिरांक अक्षर *str, cpu_set_t *set, माप_प्रकार setsize, पूर्णांक fail)
{
	स्थिरांक माप_प्रकार max = cpuset_nbits(setsize);
	स्थिरांक अक्षर *p, *q;
	अक्षर *end = रिक्त;

	q = str;
	CPU_ZERO_S(setsize, set);

	पर्यंत (p = q, q = nexttoken(q, ','), p) {
		अचिन्हित पूर्णांक a;	/* beginning of range */
		अचिन्हित पूर्णांक b;	/* end of range */
		अचिन्हित पूर्णांक s;	/* stride */
		स्थिरांक अक्षर *c1, *c2;

		जर (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			परत 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		जर (c1 != रिक्त && (c2 == रिक्त || c1 < c2)) {
			जर (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				परत 1;

			c1 = nexttoken(end, ':');

			जर (c1 != रिक्त && (c2 == रिक्त || c1 < c2)) {
				जर (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					परत 1;
				जर (s == 0)
					परत 1;
			}
		}

		जर (!(a <= b))
			परत 1;
		पर्यंत (a <= b) {
			जर (a >= max) {
				जर (fail)
					परत 2;
				अन्यथा
					थांबा;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	जर (end && *end)
		परत 1;
	परत 0;
}

#ifdef TEST_PROGRAM_CPUSET

#अन्तर्भाव <getopt.h>

पूर्णांक main(पूर्णांक argc, अक्षर *argv[])
{
	cpu_set_t *set;
	माप_प्रकार setsize, buflen, nbits;
	अक्षर *buf, *mask = रिक्त, *range = रिक्त;
	पूर्णांक ncpus = 2048, rc, c;

	स्थिर स्थिरांक संरचना option longopts[] = {
	    { "ncpus", 1, रिक्त, 'n' },
	    { "mask",  1, रिक्त, 'm' },
	    { "range", 1, रिक्त, 'r' },
	    { रिक्त,    0, रिक्त, 0 }
	};

	पर्यंत ((c = getopt_long(argc, argv, "n:m:r:", longopts, रिक्त)) != -1) {
		निवड(c) {
		स्थिती 'n':
			ncpus = म_से_प(optarg);
			थांबा;
		स्थिती 'm':
			mask = optarg;
			थांबा;
		स्थिती 'r':
			range = optarg;
			थांबा;
		पूर्वनिर्धारित:
			जा usage_err;
		}
	}

	जर (!mask && !range)
		जा usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	जर (!set)
		err(निकास_त्रुटि, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = दो_स्मृति(buflen);
	जर (!buf)
		err(निकास_त्रुटि, "failed to allocate cpu set buffer");

	जर (mask)
		rc = cpumask_parse(mask, set, setsize);
	अन्यथा
		rc = cpulist_parse(range, set, setsize, 0);

	जर (rc)
		errx(निकास_त्रुटि, "failed to parse string: %s", mask ? : range);

	म_लिखो("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	म_लिखो("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	मुक्त(buf);
	cpuset_free(set);

	परत निकास_सफल;

usage_err:
	ख_लिखो(मानक_त्रुटि,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	निकास(निकास_त्रुटि);
}
#endif
