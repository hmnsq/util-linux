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
	जइ (v >= 0 && v < 10)
		प्रत्यागम '0' + v;
	जइ (v >= 10 && v < 16)
		प्रत्यागम ('a' - 10) + v;
	प्रत्यागम -1;
}

स्थिर अन्तःस्थ पूर्णाङ्क char_to_val(पूर्णाङ्क c)
{
	पूर्णाङ्क cl;

	जइ (c >= '0' && c <= '9')
		प्रत्यागम c - '0';
	cl = tolower(c);
	जइ (cl >= 'a' && cl <= 'f')
		प्रत्यागम cl + (10 - 'a');
	प्रत्यागम -1;
}

स्थिर ध्रुव वर्ण *nexttoken(ध्रुव वर्ण *q,  पूर्णाङ्क sep)
{
	जइ (q)
		q = strchr(q, sep);
	जइ (q)
		q++;
	प्रत्यागम q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
पूर्णाङ्क get_max_सङ्ख्या_of_cpus(शून्य)
{
#ifdef SYS_sched_getaffinity
	पूर्णाङ्क n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, रिक्त);

	जइ (!set)
		प्रत्यागम -1;	/* error */

	प्रति (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		जइ (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, रिक्त);
			जइ (!set)
				प्रत्यागम -1;	/* error */
			अनुवर्त;
		}
		cpuset_free(set);
		प्रत्यागम n * 8;
	}
#endif
	प्रत्यागम -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(पूर्णाङ्क ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = रिक्त;

	जइ (ncpus)
		set = CPU_ALLOC(ncpus);
	जइ (!set)
		प्रत्यागम रिक्त;
	जइ (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	जइ (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	प्रत्यागम set;
}

शून्य cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#जइ !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
पूर्णाङ्क __cpuset_count_s(size_t setsize, ध्रुव cpu_set_t *set)
{
	पूर्णाङ्क s = 0;
	ध्रुव __cpu_mask *p = set->__bits;
	ध्रुव __cpu_mask *end = &set->__bits[setsize / परिमाण (__cpu_mask)];

	जाव (p < end) {
		__cpu_mask l = *p++;

		जइ (l == 0)
			अनुवर्त;
# जइ LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# अण्णहा
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	प्रत्यागम s;
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
स्थिर size_t find_next_cpu(size_t start, size_t setsize, cpu_set_t *set)
{
	size_t nbits = cpuset_nbits(setsize);
	प्रति (; start < nbits; start++)
		जइ (CPU_ISSET_S(start, setsize, set))
			प्रत्यागम start;
	प्रत्यागम start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
वर्ण *cpulist_create(वर्ण *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	वर्ण *ptr = str;
	पूर्णाङ्क entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	जाव ((a = find_next_cpu(next, setsize, set)) < max) {
		पूर्णाङ्क rlen;
		next = find_next_cpu(a + 1, setsize, set);
		जइ (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} अण्णहा {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			जाव (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			जइ (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} अण्णहा जइ (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} अण्णहा {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		जइ (rlen < 0 || (size_t) rlen >= len)
			प्रत्यागम रिक्त;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	प्रत्यागम str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
वर्ण *cpulist_flat_create(वर्ण *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	वर्ण *ptr = str;
	पूर्णाङ्क entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	पूर्णाङ्क rlen;

	प्रति (cpu = 0; cpu < max; cpu++) {
		जइ (!CPU_ISSET_S(cpu, setsize, set))
			अनुवर्त;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		जइ (rlen < 0 || (size_t)rlen >= len)
			प्रत्यागम रिक्त;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	प्रत्यागम str;
}


/*
 * Returns string with CPU mask.
 */
वर्ण *cpumask_create(वर्ण *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	वर्ण *ptr = str;
	वर्ण *ret = रिक्त;
	पूर्णाङ्क cpu;

	प्रति (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		वर्ण val = 0;

		जइ (len == (size_t) (ptr - str))
			भङ्ग;

		जइ (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		जइ (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		जइ (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		जइ (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		जइ (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	प्रत्यागम ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
पूर्णाङ्क cpumask_parse(ध्रुव वर्ण *str, cpu_set_t *set, size_t setsize)
{
	पूर्णाङ्क len = strlen(str);
	ध्रुव वर्ण *ptr = str + len - 1;
	पूर्णाङ्क cpu = 0;

	/* skip 0x, it's all hex anyway */
	जइ (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	जाव (ptr >= str) {
		वर्ण val;

		/* cpu masks in /sys uses comma as a separator */
		जइ (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		जइ (val == (वर्ण) -1)
			प्रत्यागम -1;
		जइ (val & 1)
			CPU_SET_S(cpu, setsize, set);
		जइ (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		जइ (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		जइ (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क nextnumber(ध्रुव वर्ण *str, वर्ण **end, unsigned पूर्णाङ्क *result)
{
	errno = 0;
	जइ (str == रिक्त || *str == '\0' || !isdigit(*str))
		प्रत्यागम -EINVAL;
	*result = (unsigned पूर्णाङ्क) strtoul(str, end, 10);
	जइ (errno)
		प्रत्यागम -errno;
	जइ (str == *end)
		प्रत्यागम -EINVAL;
	प्रत्यागम 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
पूर्णाङ्क cpulist_parse(ध्रुव वर्ण *str, cpu_set_t *set, size_t setsize, पूर्णाङ्क fail)
{
	ध्रुव size_t max = cpuset_nbits(setsize);
	ध्रुव वर्ण *p, *q;
	वर्ण *end = रिक्त;

	q = str;
	CPU_ZERO_S(setsize, set);

	जाव (p = q, q = nexttoken(q, ','), p) {
		unsigned पूर्णाङ्क a;	/* beginning of range */
		unsigned पूर्णाङ्क b;	/* end of range */
		unsigned पूर्णाङ्क s;	/* stride */
		ध्रुव वर्ण *c1, *c2;

		जइ (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			प्रत्यागम 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		जइ (c1 != रिक्त && (c2 == रिक्त || c1 < c2)) {
			जइ (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				प्रत्यागम 1;

			c1 = nexttoken(end, ':');

			जइ (c1 != रिक्त && (c2 == रिक्त || c1 < c2)) {
				जइ (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					प्रत्यागम 1;
				जइ (s == 0)
					प्रत्यागम 1;
			}
		}

		जइ (!(a <= b))
			प्रत्यागम 1;
		जाव (a <= b) {
			जइ (a >= max) {
				जइ (fail)
					प्रत्यागम 2;
				अण्णहा
					भङ्ग;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	जइ (end && *end)
		प्रत्यागम 1;
	प्रत्यागम 0;
}

#ifdef TEST_PROGRAM_CPUSET

#अन्तर्भाव <getopt.h>

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	वर्ण *buf, *mask = रिक्त, *range = रिक्त;
	पूर्णाङ्क ncpus = 2048, rc, c;

	स्थिर ध्रुव संरचना option longopts[] = {
	    { "ncpus", 1, रिक्त, 'n' },
	    { "mask",  1, रिक्त, 'm' },
	    { "range", 1, रिक्त, 'r' },
	    { रिक्त,    0, रिक्त, 0 }
	};

	जाव ((c = getopt_long(argc, argv, "n:m:r:", longopts, रिक्त)) != -1) {
		विकल्प(c) {
		पक्ष 'n':
			ncpus = atoi(optarg);
			भङ्ग;
		पक्ष 'm':
			mask = optarg;
			भङ्ग;
		पक्ष 'r':
			range = optarg;
			भङ्ग;
		स्वभाव:
			गच्छ usage_err;
		}
	}

	जइ (!mask && !range)
		गच्छ usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	जइ (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	जइ (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	जइ (mask)
		rc = cpumask_parse(mask, set, setsize);
	अण्णहा
		rc = cpulist_parse(range, set, setsize, 0);

	जइ (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	मुक्त(buf);
	cpuset_free(set);

	प्रत्यागम EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
