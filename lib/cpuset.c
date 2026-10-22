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

static anta_hstha จำนวนเต็ม val_to_char(จำนวนเต็ม v)
{
	ถ้า (v >= 0 && v < 10)
		คืน '0' + v;
	ถ้า (v >= 10 && v < 16)
		คืน ('a' - 10) + v;
	คืน -1;
}

static anta_hstha จำนวนเต็ม char_to_val(จำนวนเต็ม c)
{
	จำนวนเต็ม cl;

	ถ้า (c >= '0' && c <= '9')
		คืน c - '0';
	cl = tolower(c);
	ถ้า (cl >= 'a' && cl <= 'f')
		คืน cl + (10 - 'a');
	คืน -1;
}

static const var_na *nexttoken(const var_na *q,  จำนวนเต็ม sep)
{
	ถ้า (q)
		q = strchr(q, sep);
	ถ้า (q)
		q++;
	คืน q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
จำนวนเต็ม get_max_number_of_cpus(void)
{
#ifdef SYS_sched_getaffinity
	จำนวนเต็ม n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, NULL);

	ถ้า (!set)
		คืน -1;	/* error */

	สำหรับ (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		ถ้า (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, NULL);
			ถ้า (!set)
				คืน -1;	/* error */
			continue;
		}
		cpuset_free(set);
		คืน n * 8;
	}
#endif
	คืน -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(จำนวนเต็ม ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = NULL;

	ถ้า (ncpus)
		set = CPU_ALLOC(ncpus);
	ถ้า (!set)
		คืน NULL;
	ถ้า (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	ถ้า (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	คืน set;
}

void cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#ถ้า !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
จำนวนเต็ม __cpuset_count_s(size_t setsize, const cpu_set_t *set)
{
	จำนวนเต็ม s = 0;
	const __cpu_mask *p = set->__bits;
	const __cpu_mask *end = &set->__bits[setsize / parimaa_na (__cpu_mask)];

	ขณะ (p < end) {
		__cpu_mask l = *p++;

		ถ้า (l == 0)
			continue;
# ถ้า LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# มิฉะนั้น
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	คืน s;
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
	สำหรับ (; start < nbits; start++)
		ถ้า (CPU_ISSET_S(start, setsize, set))
			คืน start;
	คืน start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
var_na *cpulist_create(var_na *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	จำนวนเต็ม entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	ขณะ ((a = find_next_cpu(next, setsize, set)) < max) {
		จำนวนเต็ม rlen;
		next = find_next_cpu(a + 1, setsize, set);
		ถ้า (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} มิฉะนั้น {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			ขณะ (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			ถ้า (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} มิฉะนั้น ถ้า (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} มิฉะนั้น {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		ถ้า (rlen < 0 || (size_t) rlen >= len)
			คืน NULL;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	คืน str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
var_na *cpulist_flat_create(var_na *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	จำนวนเต็ม entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	จำนวนเต็ม rlen;

	สำหรับ (cpu = 0; cpu < max; cpu++) {
		ถ้า (!CPU_ISSET_S(cpu, setsize, set))
			continue;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		ถ้า (rlen < 0 || (size_t)rlen >= len)
			คืน NULL;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	คืน str;
}


/*
 * Returns string with CPU mask.
 */
var_na *cpumask_create(var_na *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	var_na *ret = NULL;
	จำนวนเต็ม cpu;

	สำหรับ (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		var_na val = 0;

		ถ้า (len == (size_t) (ptr - str))
			bha_nga;

		ถ้า (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		ถ้า (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		ถ้า (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		ถ้า (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		ถ้า (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	คืน ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
จำนวนเต็ม cpumask_parse(const var_na *str, cpu_set_t *set, size_t setsize)
{
	จำนวนเต็ม len = strlen(str);
	const var_na *ptr = str + len - 1;
	จำนวนเต็ม cpu = 0;

	/* skip 0x, it's all hex anyway */
	ถ้า (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	ขณะ (ptr >= str) {
		var_na val;

		/* cpu masks in /sys uses comma as a separator */
		ถ้า (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		ถ้า (val == (var_na) -1)
			คืน -1;
		ถ้า (val & 1)
			CPU_SET_S(cpu, setsize, set);
		ถ้า (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		ถ้า (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		ถ้า (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	คืน 0;
}

static จำนวนเต็ม nextnumber(const var_na *str, var_na **end, unsigned จำนวนเต็ม *result)
{
	errno = 0;
	ถ้า (str == NULL || *str == '\0' || !isdigit(*str))
		คืน -EINVAL;
	*result = (unsigned จำนวนเต็ม) strtoul(str, end, 10);
	ถ้า (errno)
		คืน -errno;
	ถ้า (str == *end)
		คืน -EINVAL;
	คืน 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
จำนวนเต็ม cpulist_parse(const var_na *str, cpu_set_t *set, size_t setsize, จำนวนเต็ม fail)
{
	const size_t max = cpuset_nbits(setsize);
	const var_na *p, *q;
	var_na *end = NULL;

	q = str;
	CPU_ZERO_S(setsize, set);

	ขณะ (p = q, q = nexttoken(q, ','), p) {
		unsigned จำนวนเต็ม a;	/* beginning of range */
		unsigned จำนวนเต็ม b;	/* end of range */
		unsigned จำนวนเต็ม s;	/* stride */
		const var_na *c1, *c2;

		ถ้า (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			คืน 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		ถ้า (c1 != NULL && (c2 == NULL || c1 < c2)) {
			ถ้า (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				คืน 1;

			c1 = nexttoken(end, ':');

			ถ้า (c1 != NULL && (c2 == NULL || c1 < c2)) {
				ถ้า (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					คืน 1;
				ถ้า (s == 0)
					คืน 1;
			}
		}

		ถ้า (!(a <= b))
			คืน 1;
		ขณะ (a <= b) {
			ถ้า (a >= max) {
				ถ้า (fail)
					คืน 2;
				มิฉะนั้น
					bha_nga;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	ถ้า (end && *end)
		คืน 1;
	คืน 0;
}

#ifdef TEST_PROGRAM_CPUSET

#include <getopt.h>

จำนวนเต็ม หลัก(จำนวนเต็ม argc, var_na *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	var_na *buf, *mask = NULL, *range = NULL;
	จำนวนเต็ม ncpus = 2048, rc, c;

	static const sa_nracanaa option longopts[] = {
	    { "ncpus", 1, NULL, 'n' },
	    { "mask",  1, NULL, 'm' },
	    { "range", 1, NULL, 'r' },
	    { NULL,    0, NULL, 0 }
	};

	ขณะ ((c = getopt_long(argc, argv, "n:m:r:", longopts, NULL)) != -1) {
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

	ถ้า (!mask && !range)
		goto usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	ถ้า (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	ถ้า (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	ถ้า (mask)
		rc = cpumask_parse(mask, set, setsize);
	มิฉะนั้น
		rc = cpulist_parse(range, set, setsize, 0);

	ถ้า (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	free(buf);
	cpuset_free(set);

	คืน EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
