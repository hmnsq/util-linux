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

#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <unistd.h>
#समावेश <sched.h>
#समावेश <errno.h>
#समावेश <string.h>
#समावेश <ctype.h>
#अगर_घोषित HAVE_SYS_SYSCALL_H
#समावेश <sys/syscall.h>
#पूर्ण_अगर

#समावेश "cpuset.h"
#समावेश "c.h"

अटल अंतरभूत पूर्णांक val_to_char(पूर्णांक v)
{
	अगर (v >= 0 && v < 10)
		वापस '0' + v;
	अगर (v >= 10 && v < 16)
		वापस ('a' - 10) + v;
	वापस -1;
}

अटल अंतरभूत पूर्णांक char_to_val(पूर्णांक c)
{
	पूर्णांक cl;

	अगर (c >= '0' && c <= '9')
		वापस c - '0';
	cl = छोटे(c);
	अगर (cl >= 'a' && cl <= 'f')
		वापस cl + (10 - 'a');
	वापस -1;
}

अटल स्थिर अक्षर *nexttoken(स्थिर अक्षर *q,  पूर्णांक sep)
{
	अगर (q)
		q = म_अक्षर(q, sep);
	अगर (q)
		q++;
	वापस q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
पूर्णांक get_max_संख्या_of_cpus(व्योम)
{
#अगर_घोषित SYS_sched_getaffinity
	पूर्णांक n, cpus = 2048;
	माप_प्रकार setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, शून्य);

	अगर (!set)
		वापस -1;	/* error */

	क्रम (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		अगर (n < 0 && त्रुटि_सं == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, शून्य);
			अगर (!set)
				वापस -1;	/* error */
			जारी;
		}
		cpuset_free(set);
		वापस n * 8;
	}
#पूर्ण_अगर
	वापस -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(पूर्णांक ncpus, माप_प्रकार *setsize, माप_प्रकार *nbits)
{
	cpu_set_t *set = शून्य;

	अगर (ncpus)
		set = CPU_ALLOC(ncpus);
	अगर (!set)
		वापस शून्य;
	अगर (setsize)
		*setsize = CPU_ALLOC_आकार(ncpus);
	अगर (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_आकार(ncpus));
	वापस set;
}

व्योम cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#अगर !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
पूर्णांक __cpuset_count_s(माप_प्रकार setsize, स्थिर cpu_set_t *set)
{
	पूर्णांक s = 0;
	स्थिर __cpu_mask *p = set->__bits;
	स्थिर __cpu_mask *end = &set->__bits[setsize / माप (__cpu_mask)];

	जबतक (p < end) {
		__cpu_mask l = *p++;

		अगर (l == 0)
			जारी;
# अगर LONG_BIT > 32
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
	वापस s;
}
#पूर्ण_अगर

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
अटल माप_प्रकार find_अग्र_cpu(माप_प्रकार start, माप_प्रकार setsize, cpu_set_t *set)
{
	माप_प्रकार nbits = cpuset_nbits(setsize);
	क्रम (; start < nbits; start++)
		अगर (CPU_ISSET_S(start, setsize, set))
			वापस start;
	वापस start;
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
	माप_प्रकार अग्र = 0;  /* where to start looking for next cpu */

	जबतक ((a = find_अग्र_cpu(अग्र, setsize, set)) < max) {
		पूर्णांक rlen;
		अग्र = find_अग्र_cpu(a + 1, setsize, set);
		अगर (अग्र == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} अन्यथा {
			/* Extend range as long as we have the same stride. */
			माप_प्रकार b = अग्र;
			माप_प्रकार s = b - a;
			जबतक (((अग्र = find_अग्र_cpu(b + 1, setsize, set)) <
				max) && अग्र - b == s) {
				b = अग्र;
			}
			अगर (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				अग्र = b;
			} अन्यथा अगर (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} अन्यथा {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		अगर (rlen < 0 || (माप_प्रकार) rlen >= len)
			वापस शून्य;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	वापस str;
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

	क्रम (cpu = 0; cpu < max; cpu++) {
		अगर (!CPU_ISSET_S(cpu, setsize, set))
			जारी;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		अगर (rlen < 0 || (माप_प्रकार)rlen >= len)
			वापस शून्य;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	वापस str;
}


/*
 * Returns string with CPU mask.
 */
अक्षर *cpumask_create(अक्षर *str, माप_प्रकार len,
			cpu_set_t *set, माप_प्रकार setsize)
{
	अक्षर *ptr = str;
	अक्षर *ret = शून्य;
	पूर्णांक cpu;

	क्रम (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		अक्षर val = 0;

		अगर (len == (माप_प्रकार) (ptr - str))
			अवरोध;

		अगर (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		अगर (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		अगर (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		अगर (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		अगर (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	वापस ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
पूर्णांक cpumask_parse(स्थिर अक्षर *str, cpu_set_t *set, माप_प्रकार setsize)
{
	पूर्णांक len = म_माप(str);
	स्थिर अक्षर *ptr = str + len - 1;
	पूर्णांक cpu = 0;

	/* skip 0x, it's all hex anyway */
	अगर (len > 1 && !स_भेद(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	जबतक (ptr >= str) {
		अक्षर val;

		/* cpu masks in /sys uses comma as a separator */
		अगर (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		अगर (val == (अक्षर) -1)
			वापस -1;
		अगर (val & 1)
			CPU_SET_S(cpu, setsize, set);
		अगर (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		अगर (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		अगर (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	वापस 0;
}

अटल पूर्णांक nextnumber(स्थिर अक्षर *str, अक्षर **end, अचिन्हित पूर्णांक *परिणाम)
{
	त्रुटि_सं = 0;
	अगर (str == शून्य || *str == '\0' || !है_अंक(*str))
		वापस -EINVAL;
	*परिणाम = (अचिन्हित पूर्णांक) म_से_अदीर्घ(str, end, 10);
	अगर (त्रुटि_सं)
		वापस -त्रुटि_सं;
	अगर (str == *end)
		वापस -EINVAL;
	वापस 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
पूर्णांक cpulist_parse(स्थिर अक्षर *str, cpu_set_t *set, माप_प्रकार setsize, पूर्णांक fail)
{
	स्थिर माप_प्रकार max = cpuset_nbits(setsize);
	स्थिर अक्षर *p, *q;
	अक्षर *end = शून्य;

	q = str;
	CPU_ZERO_S(setsize, set);

	जबतक (p = q, q = nexttoken(q, ','), p) {
		अचिन्हित पूर्णांक a;	/* beginning of range */
		अचिन्हित पूर्णांक b;	/* end of range */
		अचिन्हित पूर्णांक s;	/* stride */
		स्थिर अक्षर *c1, *c2;

		अगर (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			वापस 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		अगर (c1 != शून्य && (c2 == शून्य || c1 < c2)) {
			अगर (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				वापस 1;

			c1 = nexttoken(end, ':');

			अगर (c1 != शून्य && (c2 == शून्य || c1 < c2)) {
				अगर (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					वापस 1;
				अगर (s == 0)
					वापस 1;
			}
		}

		अगर (!(a <= b))
			वापस 1;
		जबतक (a <= b) {
			अगर (a >= max) {
				अगर (fail)
					वापस 2;
				अन्यथा
					अवरोध;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	अगर (end && *end)
		वापस 1;
	वापस 0;
}

#अगर_घोषित TEST_PROGRAM_CPUSET

#समावेश <getopt.h>

पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[])
{
	cpu_set_t *set;
	माप_प्रकार setsize, buflen, nbits;
	अक्षर *buf, *mask = शून्य, *range = शून्य;
	पूर्णांक ncpus = 2048, rc, c;

	अटल स्थिर काष्ठा option longopts[] = {
	    { "ncpus", 1, शून्य, 'n' },
	    { "mask",  1, शून्य, 'm' },
	    { "range", 1, शून्य, 'r' },
	    { शून्य,    0, शून्य, 0 }
	};

	जबतक ((c = getopt_long(argc, argv, "n:m:r:", longopts, शून्य)) != -1) {
		चयन(c) {
		हाल 'n':
			ncpus = म_से_प(optarg);
			अवरोध;
		हाल 'm':
			mask = optarg;
			अवरोध;
		हाल 'r':
			range = optarg;
			अवरोध;
		शेष:
			जाओ usage_err;
		}
	}

	अगर (!mask && !range)
		जाओ usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	अगर (!set)
		err(निकास_त्रुटि, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = दो_स्मृति(buflen);
	अगर (!buf)
		err(निकास_त्रुटि, "failed to allocate cpu set buffer");

	अगर (mask)
		rc = cpumask_parse(mask, set, setsize);
	अन्यथा
		rc = cpulist_parse(range, set, setsize, 0);

	अगर (rc)
		errx(निकास_त्रुटि, "failed to parse string: %s", mask ? : range);

	म_लिखो("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	म_लिखो("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	मुक्त(buf);
	cpuset_free(set);

	वापस निकास_सफल;

usage_err:
	ख_लिखो(मानक_त्रुटि,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	निकास(निकास_त्रुटि);
}
#पूर्ण_अगर
