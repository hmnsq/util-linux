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

স্থির অন্তঃস্থ পূর্ণাঙ্ক val_to_char(পূর্ণাঙ্ক v)
{
	যদি (v >= 0 && v < 10)
		প্রত্যাগম '0' + v;
	যদি (v >= 10 && v < 16)
		প্রত্যাগম ('a' - 10) + v;
	প্রত্যাগম -1;
}

স্থির অন্তঃস্থ পূর্ণাঙ্ক char_to_val(পূর্ণাঙ্ক c)
{
	পূর্ণাঙ্ক cl;

	যদি (c >= '0' && c <= '9')
		প্রত্যাগম c - '0';
	cl = tolower(c);
	যদি (cl >= 'a' && cl <= 'f')
		প্রত্যাগম cl + (10 - 'a');
	প্রত্যাগম -1;
}

স্থির ধ্রুব বর্ণ *nexttoken(ধ্রুব বর্ণ *q,  পূর্ণাঙ্ক sep)
{
	যদি (q)
		q = strchr(q, sep);
	যদি (q)
		q++;
	প্রত্যাগম q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
পূর্ণাঙ্ক get_max_সংখ্যা_of_cpus(শূন্য)
{
#ifdef SYS_sched_getaffinity
	পূর্ণাঙ্ক n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, রিক্ত);

	যদি (!set)
		প্রত্যাগম -1;	/* error */

	প্রতি (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		যদি (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, রিক্ত);
			যদি (!set)
				প্রত্যাগম -1;	/* error */
			অনুবর্ত;
		}
		cpuset_free(set);
		প্রত্যাগম n * 8;
	}
#endif
	প্রত্যাগম -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(পূর্ণাঙ্ক ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = রিক্ত;

	যদি (ncpus)
		set = CPU_ALLOC(ncpus);
	যদি (!set)
		প্রত্যাগম রিক্ত;
	যদি (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	যদি (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	প্রত্যাগম set;
}

শূন্য cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#যদি !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
পূর্ণাঙ্ক __cpuset_count_s(size_t setsize, ধ্রুব cpu_set_t *set)
{
	পূর্ণাঙ্ক s = 0;
	ধ্রুব __cpu_mask *p = set->__bits;
	ধ্রুব __cpu_mask *end = &set->__bits[setsize / পরিমাণ (__cpu_mask)];

	যাবত্ (p < end) {
		__cpu_mask l = *p++;

		যদি (l == 0)
			অনুবর্ত;
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
	প্রত্যাগম s;
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
স্থির size_t find_next_cpu(size_t start, size_t setsize, cpu_set_t *set)
{
	size_t nbits = cpuset_nbits(setsize);
	প্রতি (; start < nbits; start++)
		যদি (CPU_ISSET_S(start, setsize, set))
			প্রত্যাগম start;
	প্রত্যাগম start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
বর্ণ *cpulist_create(বর্ণ *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	বর্ণ *ptr = str;
	পূর্ণাঙ্ক entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	যাবত্ ((a = find_next_cpu(next, setsize, set)) < max) {
		পূর্ণাঙ্ক rlen;
		next = find_next_cpu(a + 1, setsize, set);
		যদি (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} অন্যথা {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			যাবত্ (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			যদি (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} অন্যথা যদি (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} অন্যথা {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		যদি (rlen < 0 || (size_t) rlen >= len)
			প্রত্যাগম রিক্ত;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	প্রত্যাগম str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
বর্ণ *cpulist_flat_create(বর্ণ *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	বর্ণ *ptr = str;
	পূর্ণাঙ্ক entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	পূর্ণাঙ্ক rlen;

	প্রতি (cpu = 0; cpu < max; cpu++) {
		যদি (!CPU_ISSET_S(cpu, setsize, set))
			অনুবর্ত;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		যদি (rlen < 0 || (size_t)rlen >= len)
			প্রত্যাগম রিক্ত;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	প্রত্যাগম str;
}


/*
 * Returns string with CPU mask.
 */
বর্ণ *cpumask_create(বর্ণ *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	বর্ণ *ptr = str;
	বর্ণ *ret = রিক্ত;
	পূর্ণাঙ্ক cpu;

	প্রতি (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		বর্ণ val = 0;

		যদি (len == (size_t) (ptr - str))
			ভঙ্গ;

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
	প্রত্যাগম ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
পূর্ণাঙ্ক cpumask_parse(ধ্রুব বর্ণ *str, cpu_set_t *set, size_t setsize)
{
	পূর্ণাঙ্ক len = strlen(str);
	ধ্রুব বর্ণ *ptr = str + len - 1;
	পূর্ণাঙ্ক cpu = 0;

	/* skip 0x, it's all hex anyway */
	যদি (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	যাবত্ (ptr >= str) {
		বর্ণ val;

		/* cpu masks in /sys uses comma as a separator */
		যদি (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		যদি (val == (বর্ণ) -1)
			প্রত্যাগম -1;
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

	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক nextnumber(ধ্রুব বর্ণ *str, বর্ণ **end, unsigned পূর্ণাঙ্ক *result)
{
	errno = 0;
	যদি (str == রিক্ত || *str == '\0' || !isdigit(*str))
		প্রত্যাগম -EINVAL;
	*result = (unsigned পূর্ণাঙ্ক) strtoul(str, end, 10);
	যদি (errno)
		প্রত্যাগম -errno;
	যদি (str == *end)
		প্রত্যাগম -EINVAL;
	প্রত্যাগম 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
পূর্ণাঙ্ক cpulist_parse(ধ্রুব বর্ণ *str, cpu_set_t *set, size_t setsize, পূর্ণাঙ্ক fail)
{
	ধ্রুব size_t max = cpuset_nbits(setsize);
	ধ্রুব বর্ণ *p, *q;
	বর্ণ *end = রিক্ত;

	q = str;
	CPU_ZERO_S(setsize, set);

	যাবত্ (p = q, q = nexttoken(q, ','), p) {
		unsigned পূর্ণাঙ্ক a;	/* beginning of range */
		unsigned পূর্ণাঙ্ক b;	/* end of range */
		unsigned পূর্ণাঙ্ক s;	/* stride */
		ধ্রুব বর্ণ *c1, *c2;

		যদি (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			প্রত্যাগম 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		যদি (c1 != রিক্ত && (c2 == রিক্ত || c1 < c2)) {
			যদি (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				প্রত্যাগম 1;

			c1 = nexttoken(end, ':');

			যদি (c1 != রিক্ত && (c2 == রিক্ত || c1 < c2)) {
				যদি (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					প্রত্যাগম 1;
				যদি (s == 0)
					প্রত্যাগম 1;
			}
		}

		যদি (!(a <= b))
			প্রত্যাগম 1;
		যাবত্ (a <= b) {
			যদি (a >= max) {
				যদি (fail)
					প্রত্যাগম 2;
				অন্যথা
					ভঙ্গ;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	যদি (end && *end)
		প্রত্যাগম 1;
	প্রত্যাগম 0;
}

#ifdef TEST_PROGRAM_CPUSET

#অন্তর্ভাব <getopt.h>

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	বর্ণ *buf, *mask = রিক্ত, *range = রিক্ত;
	পূর্ণাঙ্ক ncpus = 2048, rc, c;

	স্থির ধ্রুব সংরচনা option longopts[] = {
	    { "ncpus", 1, রিক্ত, 'n' },
	    { "mask",  1, রিক্ত, 'm' },
	    { "range", 1, রিক্ত, 'r' },
	    { রিক্ত,    0, রিক্ত, 0 }
	};

	যাবত্ ((c = getopt_long(argc, argv, "n:m:r:", longopts, রিক্ত)) != -1) {
		বিকল্প(c) {
		পক্ষ 'n':
			ncpus = atoi(optarg);
			ভঙ্গ;
		পক্ষ 'm':
			mask = optarg;
			ভঙ্গ;
		পক্ষ 'r':
			range = optarg;
			ভঙ্গ;
		স্বভাব:
			গচ্ছ usage_err;
		}
	}

	যদি (!mask && !range)
		গচ্ছ usage_err;

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

	প্রত্যাগম EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
