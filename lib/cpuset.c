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

static anta_hstha ཧྲིལ val_to_char(ཧྲིལ v)
{
	གལ་སྲིད (v >= 0 && v < 10)
		ལོག '0' + v;
	གལ་སྲིད (v >= 10 && v < 16)
		ལོག ('a' - 10) + v;
	ལོག -1;
}

static anta_hstha ཧྲིལ char_to_val(ཧྲིལ c)
{
	ཧྲིལ cl;

	གལ་སྲིད (c >= '0' && c <= '9')
		ལོག c - '0';
	cl = tolower(c);
	གལ་སྲིད (cl >= 'a' && cl <= 'f')
		ལོག cl + (10 - 'a');
	ལོག -1;
}

static const var_na *nexttoken(const var_na *q,  ཧྲིལ sep)
{
	གལ་སྲིད (q)
		q = strchr(q, sep);
	གལ་སྲིད (q)
		q++;
	ལོག q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
ཧྲིལ get_max_number_of_cpus(void)
{
#ifdef SYS_sched_getaffinity
	ཧྲིལ n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, NULL);

	གལ་སྲིད (!set)
		ལོག -1;	/* error */

	དོན་ལུ (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		གལ་སྲིད (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, NULL);
			གལ་སྲིད (!set)
				ལོག -1;	/* error */
			continue;
		}
		cpuset_free(set);
		ལོག n * 8;
	}
#endif
	ལོག -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(ཧྲིལ ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = NULL;

	གལ་སྲིད (ncpus)
		set = CPU_ALLOC(ncpus);
	གལ་སྲིད (!set)
		ལོག NULL;
	གལ་སྲིད (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	གལ་སྲིད (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	ལོག set;
}

void cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#གལ་སྲིད !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
ཧྲིལ __cpuset_count_s(size_t setsize, const cpu_set_t *set)
{
	ཧྲིལ s = 0;
	const __cpu_mask *p = set->__bits;
	const __cpu_mask *end = &set->__bits[setsize / parimaa_na (__cpu_mask)];

	སྐབས (p < end) {
		__cpu_mask l = *p++;

		གལ་སྲིད (l == 0)
			continue;
# གལ་སྲིད LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# དེ་མིན
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	ལོག s;
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
	དོན་ལུ (; start < nbits; start++)
		གལ་སྲིད (CPU_ISSET_S(start, setsize, set))
			ལོག start;
	ལོག start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
var_na *cpulist_create(var_na *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	ཧྲིལ entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	སྐབས ((a = find_next_cpu(next, setsize, set)) < max) {
		ཧྲིལ rlen;
		next = find_next_cpu(a + 1, setsize, set);
		གལ་སྲིད (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} དེ་མིན {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			སྐབས (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			གལ་སྲིད (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} དེ་མིན གལ་སྲིད (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} དེ་མིན {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		གལ་སྲིད (rlen < 0 || (size_t) rlen >= len)
			ལོག NULL;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	ལོག str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
var_na *cpulist_flat_create(var_na *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	ཧྲིལ entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	ཧྲིལ rlen;

	དོན་ལུ (cpu = 0; cpu < max; cpu++) {
		གལ་སྲིད (!CPU_ISSET_S(cpu, setsize, set))
			continue;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		གལ་སྲིད (rlen < 0 || (size_t)rlen >= len)
			ལོག NULL;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	ལོག str;
}


/*
 * Returns string with CPU mask.
 */
var_na *cpumask_create(var_na *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	var_na *ret = NULL;
	ཧྲིལ cpu;

	དོན་ལུ (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		var_na val = 0;

		གལ་སྲིད (len == (size_t) (ptr - str))
			bha_nga;

		གལ་སྲིད (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		གལ་སྲིད (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		གལ་སྲིད (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		གལ་སྲིད (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		གལ་སྲིད (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	ལོག ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
ཧྲིལ cpumask_parse(const var_na *str, cpu_set_t *set, size_t setsize)
{
	ཧྲིལ len = strlen(str);
	const var_na *ptr = str + len - 1;
	ཧྲིལ cpu = 0;

	/* skip 0x, it's all hex anyway */
	གལ་སྲིད (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	སྐབས (ptr >= str) {
		var_na val;

		/* cpu masks in /sys uses comma as a separator */
		གལ་སྲིད (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		གལ་སྲིད (val == (var_na) -1)
			ལོག -1;
		གལ་སྲིད (val & 1)
			CPU_SET_S(cpu, setsize, set);
		གལ་སྲིད (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		གལ་སྲིད (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		གལ་སྲིད (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	ལོག 0;
}

static ཧྲིལ nextnumber(const var_na *str, var_na **end, unsigned ཧྲིལ *result)
{
	errno = 0;
	གལ་སྲིད (str == NULL || *str == '\0' || !isdigit(*str))
		ལོག -EINVAL;
	*result = (unsigned ཧྲིལ) strtoul(str, end, 10);
	གལ་སྲིད (errno)
		ལོག -errno;
	གལ་སྲིད (str == *end)
		ལོག -EINVAL;
	ལོག 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
ཧྲིལ cpulist_parse(const var_na *str, cpu_set_t *set, size_t setsize, ཧྲིལ fail)
{
	const size_t max = cpuset_nbits(setsize);
	const var_na *p, *q;
	var_na *end = NULL;

	q = str;
	CPU_ZERO_S(setsize, set);

	སྐབས (p = q, q = nexttoken(q, ','), p) {
		unsigned ཧྲིལ a;	/* beginning of range */
		unsigned ཧྲིལ b;	/* end of range */
		unsigned ཧྲིལ s;	/* stride */
		const var_na *c1, *c2;

		གལ་སྲིད (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			ལོག 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		གལ་སྲིད (c1 != NULL && (c2 == NULL || c1 < c2)) {
			གལ་སྲིད (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				ལོག 1;

			c1 = nexttoken(end, ':');

			གལ་སྲིད (c1 != NULL && (c2 == NULL || c1 < c2)) {
				གལ་སྲིད (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					ལོག 1;
				གལ་སྲིད (s == 0)
					ལོག 1;
			}
		}

		གལ་སྲིད (!(a <= b))
			ལོག 1;
		སྐབས (a <= b) {
			གལ་སྲིད (a >= max) {
				གལ་སྲིད (fail)
					ལོག 2;
				དེ་མིན
					bha_nga;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	གལ་སྲིད (end && *end)
		ལོག 1;
	ལོག 0;
}

#ifdef TEST_PROGRAM_CPUSET

#include <getopt.h>

ཧྲིལ main(ཧྲིལ argc, var_na *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	var_na *buf, *mask = NULL, *range = NULL;
	ཧྲིལ ncpus = 2048, rc, c;

	static const sa_nracanaa option longopts[] = {
	    { "ncpus", 1, NULL, 'n' },
	    { "mask",  1, NULL, 'm' },
	    { "range", 1, NULL, 'r' },
	    { NULL,    0, NULL, 0 }
	};

	སྐབས ((c = getopt_long(argc, argv, "n:m:r:", longopts, NULL)) != -1) {
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

	གལ་སྲིད (!mask && !range)
		goto usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	གལ་སྲིད (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	གལ་སྲིད (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	གལ་སྲིད (mask)
		rc = cpumask_parse(mask, set, setsize);
	དེ་མིན
		rc = cpulist_parse(range, set, setsize, 0);

	གལ་སྲིད (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	free(buf);
	cpuset_free(set);

	ལོག EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
