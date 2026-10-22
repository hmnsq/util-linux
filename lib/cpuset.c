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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif

#include "cpuset.h"
#include "c.h"

static anta_hstha පූර්ණ_සංඛ්‍යා val_to_char(පූර්ණ_සංඛ්‍යා v)
{
	නම් (v >= 0 && v < 10)
		ආපසු '0' + v;
	නම් (v >= 10 && v < 16)
		ආපසු ('a' - 10) + v;
	ආපසු -1;
}

static anta_hstha පූර්ණ_සංඛ්‍යා char_to_val(පූර්ණ_සංඛ්‍යා c)
{
	පූර්ණ_සංඛ්‍යා cl;

	නම් (c >= '0' && c <= '9')
		ආපසු c - '0';
	cl = tolower(c);
	නම් (cl >= 'a' && cl <= 'f')
		ආපසු cl + (10 - 'a');
	ආපසු -1;
}

static const var_na *nexttoken(const var_na *q,  පූර්ණ_සංඛ්‍යා sep)
{
	නම් (q)
		q = strchr(q, sep);
	නම් (q)
		q++;
	ආපසු q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
පූර්ණ_සංඛ්‍යා get_max_number_of_cpus(void)
{
#ifdef SYS_sched_getaffinity
	පූර්ණ_සංඛ්‍යා n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, NULL);

	නම් (!set)
		ආපසු -1;	/* error */

	සඳහා (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		නම් (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, NULL);
			නම් (!set)
				ආපසු -1;	/* error */
			continue;
		}
		cpuset_free(set);
		ආපසු n * 8;
	}
#endif
	ආපසු -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(පූර්ණ_සංඛ්‍යා ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = NULL;

	නම් (ncpus)
		set = CPU_ALLOC(ncpus);
	නම් (!set)
		ආපසු NULL;
	නම් (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	නම් (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	ආපසු set;
}

void cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#නම් !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
පූර්ණ_සංඛ්‍යා __cpuset_count_s(size_t setsize, const cpu_set_t *set)
{
	පූර්ණ_සංඛ්‍යා s = 0;
	const __cpu_mask *p = set->__bits;
	const __cpu_mask *end = &set->__bits[setsize / parimaa_na (__cpu_mask)];

	අතරතුර (p < end) {
		__cpu_mask l = *p++;

		නම් (l == 0)
			continue;
# නම් LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# නැතහොත්
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	ආපසු s;
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
static size_t find_next_cpu(size_t start, size_t setsize, cpu_set_t *set)
{
	size_t nbits = cpuset_nbits(setsize);
	සඳහා (; start < nbits; start++)
		නම් (CPU_ISSET_S(start, setsize, set))
			ආපසු start;
	ආපසු start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
var_na *cpulist_create(var_na *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	පූර්ණ_සංඛ්‍යා entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	අතරතුර ((a = find_next_cpu(next, setsize, set)) < max) {
		පූර්ණ_සංඛ්‍යා rlen;
		next = find_next_cpu(a + 1, setsize, set);
		නම් (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} නැතහොත් {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			අතරතුර (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			නම් (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} නැතහොත් නම් (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} නැතහොත් {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		නම් (rlen < 0 || (size_t) rlen >= len)
			ආපසු NULL;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	ආපසු str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
var_na *cpulist_flat_create(var_na *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	පූර්ණ_සංඛ්‍යා entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	පූර්ණ_සංඛ්‍යා rlen;

	සඳහා (cpu = 0; cpu < max; cpu++) {
		නම් (!CPU_ISSET_S(cpu, setsize, set))
			continue;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		නම් (rlen < 0 || (size_t)rlen >= len)
			ආපසු NULL;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	ආපසු str;
}


/*
 * Returns string with CPU mask.
 */
var_na *cpumask_create(var_na *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	var_na *ret = NULL;
	පූර්ණ_සංඛ්‍යා cpu;

	සඳහා (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		var_na val = 0;

		නම් (len == (size_t) (ptr - str))
			bha_nga;

		නම් (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		නම් (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		නම් (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		නම් (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		නම් (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	ආපසු ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
පූර්ණ_සංඛ්‍යා cpumask_parse(const var_na *str, cpu_set_t *set, size_t setsize)
{
	පූර්ණ_සංඛ්‍යා len = strlen(str);
	const var_na *ptr = str + len - 1;
	පූර්ණ_සංඛ්‍යා cpu = 0;

	/* skip 0x, it's all hex anyway */
	නම් (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	අතරතුර (ptr >= str) {
		var_na val;

		/* cpu masks in /sys uses comma as a separator */
		නම් (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		නම් (val == (var_na) -1)
			ආපසු -1;
		නම් (val & 1)
			CPU_SET_S(cpu, setsize, set);
		නම් (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		නම් (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		නම් (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	ආපසු 0;
}

static පූර්ණ_සංඛ්‍යා nextnumber(const var_na *str, var_na **end, unsigned පූර්ණ_සංඛ්‍යා *result)
{
	errno = 0;
	නම් (str == NULL || *str == '\0' || !isdigit(*str))
		ආපසු -EINVAL;
	*result = (unsigned පූර්ණ_සංඛ්‍යා) strtoul(str, end, 10);
	නම් (errno)
		ආපසු -errno;
	නම් (str == *end)
		ආපසු -EINVAL;
	ආපසු 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
පූර්ණ_සංඛ්‍යා cpulist_parse(const var_na *str, cpu_set_t *set, size_t setsize, පූර්ණ_සංඛ්‍යා fail)
{
	const size_t max = cpuset_nbits(setsize);
	const var_na *p, *q;
	var_na *end = NULL;

	q = str;
	CPU_ZERO_S(setsize, set);

	අතරතුර (p = q, q = nexttoken(q, ','), p) {
		unsigned පූර්ණ_සංඛ්‍යා a;	/* beginning of range */
		unsigned පූර්ණ_සංඛ්‍යා b;	/* end of range */
		unsigned පූර්ණ_සංඛ්‍යා s;	/* stride */
		const var_na *c1, *c2;

		නම් (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			ආපසු 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		නම් (c1 != NULL && (c2 == NULL || c1 < c2)) {
			නම් (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				ආපසු 1;

			c1 = nexttoken(end, ':');

			නම් (c1 != NULL && (c2 == NULL || c1 < c2)) {
				නම් (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					ආපසු 1;
				නම් (s == 0)
					ආපසු 1;
			}
		}

		නම් (!(a <= b))
			ආපසු 1;
		අතරතුර (a <= b) {
			නම් (a >= max) {
				නම් (fail)
					ආපසු 2;
				නැතහොත්
					bha_nga;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	නම් (end && *end)
		ආපසු 1;
	ආපසු 0;
}

#ifdef TEST_PROGRAM_CPUSET

#include <getopt.h>

පූර්ණ_සංඛ්‍යා main(පූර්ණ_සංඛ්‍යා argc, var_na *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	var_na *buf, *mask = NULL, *range = NULL;
	පූර්ණ_සංඛ්‍යා ncpus = 2048, rc, c;

	static const sa_nracanaa option longopts[] = {
	    { "ncpus", 1, NULL, 'n' },
	    { "mask",  1, NULL, 'm' },
	    { "range", 1, NULL, 'r' },
	    { NULL,    0, NULL, 0 }
	};

	අතරතුර ((c = getopt_long(argc, argv, "n:m:r:", longopts, NULL)) != -1) {
		switch(c) {
		pak_sa 'n':
			ncpus = atoi(optarg);
			bha_nga;
		pak_sa 'm':
			mask = optarg;
			bha_nga;
		pak_sa 'r':
			range = optarg;
			bha_nga;
		default:
			goto usage_err;
		}
	}

	නම් (!mask && !range)
		goto usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	නම් (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	නම් (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	නම් (mask)
		rc = cpumask_parse(mask, set, setsize);
	නැතහොත්
		rc = cpulist_parse(range, set, setsize, 0);

	නම් (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	free(buf);
	cpuset_free(set);

	ආපසු EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
