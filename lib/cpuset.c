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

#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <sched.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <ctype.h>
#ifdef HAVE_SYS_SYSCALL_H
#অন্তর্ভাব <sys/syscall.h>
#endif

#অন্তর্ভাব "cpuset.h"
#অন্তর্ভাব "c.h"

স্থির অন্তঃরেখ পূর্ণসংখ্যা val_to_char(পূর্ণসংখ্যা v)
{
	যদি (v >= 0 && v < 10)
		ফেরত '0' + v;
	যদি (v >= 10 && v < 16)
		ফেরত ('a' - 10) + v;
	ফেরত -1;
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা char_to_val(পূর্ণসংখ্যা c)
{
	পূর্ণসংখ্যা cl;

	যদি (c >= '0' && c <= '9')
		ফেরত c - '0';
	cl = tolower(c);
	যদি (cl >= 'a' && cl <= 'f')
		ফেরত cl + (10 - 'a');
	ফেরত -1;
}

স্থির ধ্রুবক অক্ষর *nexttoken(ধ্রুবক অক্ষর *q,  পূর্ণসংখ্যা sep)
{
	যদি (q)
		q = strchr(q, sep);
	যদি (q)
		q++;
	ফেরত q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
পূর্ণসংখ্যা get_max_সংখ্যা_of_cpus(শূন্য)
{
#ifdef SYS_sched_getaffinity
	পূর্ণসংখ্যা n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, ফাঁকা);

	যদি (!set)
		ফেরত -1;	/* error */

	জন্য (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		যদি (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, ফাঁকা);
			যদি (!set)
				ফেরত -1;	/* error */
			চালু;
		}
		cpuset_free(set);
		ফেরত n * 8;
	}
#endif
	ফেরত -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(পূর্ণসংখ্যা ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = ফাঁকা;

	যদি (ncpus)
		set = CPU_ALLOC(ncpus);
	যদি (!set)
		ফেরত ফাঁকা;
	যদি (setsize)
		*setsize = CPU_ALLOC_আকার(ncpus);
	যদি (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_আকার(ncpus));
	ফেরত set;
}

শূন্য cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#যদি !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
পূর্ণসংখ্যা __cpuset_count_s(size_t setsize, ধ্রুবক cpu_set_t *set)
{
	পূর্ণসংখ্যা s = 0;
	ধ্রুবক __cpu_mask *p = set->__bits;
	ধ্রুবক __cpu_mask *end = &set->__bits[setsize / আকার (__cpu_mask)];

	যখন (p < end) {
		__cpu_mask l = *p++;

		যদি (l == 0)
			চালু;
# যদি LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# অন্যথা
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	ফেরত s;
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
স্থির size_t find_পরবর্তী_cpu(size_t start, size_t setsize, cpu_set_t *set)
{
	size_t nbits = cpuset_nbits(setsize);
	জন্য (; start < nbits; start++)
		যদি (CPU_ISSET_S(start, setsize, set))
			ফেরত start;
	ফেরত start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
অক্ষর *cpulist_create(অক্ষর *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	অক্ষর *ptr = str;
	পূর্ণসংখ্যা entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t পরবর্তী = 0;  /* where to start looking for next cpu */

	যখন ((a = find_পরবর্তী_cpu(পরবর্তী, setsize, set)) < max) {
		পূর্ণসংখ্যা rlen;
		পরবর্তী = find_পরবর্তী_cpu(a + 1, setsize, set);
		যদি (পরবর্তী == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} অন্যথা {
			/* Extend range as long as we have the same stride. */
			size_t b = পরবর্তী;
			size_t s = b - a;
			যখন (((পরবর্তী = find_পরবর্তী_cpu(b + 1, setsize, set)) <
				max) && পরবর্তী - b == s) {
				b = পরবর্তী;
			}
			যদি (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				পরবর্তী = b;
			} অন্যথা যদি (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} অন্যথা {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		যদি (rlen < 0 || (size_t) rlen >= len)
			ফেরত ফাঁকা;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	ফেরত str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
অক্ষর *cpulist_flat_create(অক্ষর *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	অক্ষর *ptr = str;
	পূর্ণসংখ্যা entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	পূর্ণসংখ্যা rlen;

	জন্য (cpu = 0; cpu < max; cpu++) {
		যদি (!CPU_ISSET_S(cpu, setsize, set))
			চালু;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		যদি (rlen < 0 || (size_t)rlen >= len)
			ফেরত ফাঁকা;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	ফেরত str;
}


/*
 * Returns string with CPU mask.
 */
অক্ষর *cpumask_create(অক্ষর *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	অক্ষর *ptr = str;
	অক্ষর *ret = ফাঁকা;
	পূর্ণসংখ্যা cpu;

	জন্য (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		অক্ষর val = 0;

		যদি (len == (size_t) (ptr - str))
			বিরতি;

		যদি (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		যদি (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		যদি (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		যদি (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		যদি (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	ফেরত ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
পূর্ণসংখ্যা cpumask_parse(ধ্রুবক অক্ষর *str, cpu_set_t *set, size_t setsize)
{
	পূর্ণসংখ্যা len = strlen(str);
	ধ্রুবক অক্ষর *ptr = str + len - 1;
	পূর্ণসংখ্যা cpu = 0;

	/* skip 0x, it's all hex anyway */
	যদি (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	যখন (ptr >= str) {
		অক্ষর val;

		/* cpu masks in /sys uses comma as a separator */
		যদি (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		যদি (val == (অক্ষর) -1)
			ফেরত -1;
		যদি (val & 1)
			CPU_SET_S(cpu, setsize, set);
		যদি (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		যদি (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		যদি (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা nextnumber(ধ্রুবক অক্ষর *str, অক্ষর **end, unsigned পূর্ণসংখ্যা *ফলাফল)
{
	errno = 0;
	যদি (str == ফাঁকা || *str == '\0' || !isdigit(*str))
		ফেরত -EINVAL;
	*ফলাফল = (unsigned পূর্ণসংখ্যা) strtoul(str, end, 10);
	যদি (errno)
		ফেরত -errno;
	যদি (str == *end)
		ফেরত -EINVAL;
	ফেরত 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
পূর্ণসংখ্যা cpulist_parse(ধ্রুবক অক্ষর *str, cpu_set_t *set, size_t setsize, পূর্ণসংখ্যা fail)
{
	ধ্রুবক size_t max = cpuset_nbits(setsize);
	ধ্রুবক অক্ষর *p, *q;
	অক্ষর *end = ফাঁকা;

	q = str;
	CPU_ZERO_S(setsize, set);

	যখন (p = q, q = nexttoken(q, ','), p) {
		unsigned পূর্ণসংখ্যা a;	/* beginning of range */
		unsigned পূর্ণসংখ্যা b;	/* end of range */
		unsigned পূর্ণসংখ্যা s;	/* stride */
		ধ্রুবক অক্ষর *c1, *c2;

		যদি (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			ফেরত 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		যদি (c1 != ফাঁকা && (c2 == ফাঁকা || c1 < c2)) {
			যদি (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				ফেরত 1;

			c1 = nexttoken(end, ':');

			যদি (c1 != ফাঁকা && (c2 == ফাঁকা || c1 < c2)) {
				যদি (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					ফেরত 1;
				যদি (s == 0)
					ফেরত 1;
			}
		}

		যদি (!(a <= b))
			ফেরত 1;
		যখন (a <= b) {
			যদি (a >= max) {
				যদি (fail)
					ফেরত 2;
				অন্যথা
					বিরতি;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	যদি (end && *end)
		ফেরত 1;
	ফেরত 0;
}

#ifdef TEST_PROGRAM_CPUSET

#অন্তর্ভাব <getopt.h>

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	অক্ষর *buf, *mask = ফাঁকা, *range = ফাঁকা;
	পূর্ণসংখ্যা ncpus = 2048, rc, c;

	স্থির ধ্রুবক কাঠামো option longopts[] = {
	    { "ncpus", 1, ফাঁকা, 'n' },
	    { "mask",  1, ফাঁকা, 'm' },
	    { "range", 1, ফাঁকা, 'r' },
	    { ফাঁকা,    0, ফাঁকা, 0 }
	};

	যখন ((c = getopt_long(argc, argv, "n:m:r:", longopts, ফাঁকা)) != -1) {
		নির্বাচন(c) {
		ক্ষেত্র 'n':
			ncpus = atoi(optarg);
			বিরতি;
		ক্ষেত্র 'm':
			mask = optarg;
			বিরতি;
		ক্ষেত্র 'r':
			range = optarg;
			বিরতি;
		পূর্বনির্ধারিত:
			যাও usage_err;
		}
	}

	যদি (!mask && !range)
		যাও usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	যদি (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	যদি (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	যদি (mask)
		rc = cpumask_parse(mask, set, setsize);
	অন্যথা
		rc = cpulist_parse(range, set, setsize, 0);

	যদি (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	মুক্ত(buf);
	cpuset_free(set);

	ফেরত EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
