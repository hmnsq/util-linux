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

static anta_hstha ຈຳນວນເຕັມ val_to_char(ຈຳນວນເຕັມ v)
{
	ຖ້າ (v >= 0 && v < 10)
		ສົ່ງຄືນ '0' + v;
	ຖ້າ (v >= 10 && v < 16)
		ສົ່ງຄືນ ('a' - 10) + v;
	ສົ່ງຄືນ -1;
}

static anta_hstha ຈຳນວນເຕັມ char_to_val(ຈຳນວນເຕັມ c)
{
	ຈຳນວນເຕັມ cl;

	ຖ້າ (c >= '0' && c <= '9')
		ສົ່ງຄືນ c - '0';
	cl = tolower(c);
	ຖ້າ (cl >= 'a' && cl <= 'f')
		ສົ່ງຄືນ cl + (10 - 'a');
	ສົ່ງຄືນ -1;
}

static const var_na *nexttoken(const var_na *q,  ຈຳນວນເຕັມ sep)
{
	ຖ້າ (q)
		q = strchr(q, sep);
	ຖ້າ (q)
		q++;
	ສົ່ງຄືນ q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
ຈຳນວນເຕັມ get_max_number_of_cpus(void)
{
#ifdef SYS_sched_getaffinity
	ຈຳນວນເຕັມ n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, NULL);

	ຖ້າ (!set)
		ສົ່ງຄືນ -1;	/* error */

	ສຳລັບ (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		ຖ້າ (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, NULL);
			ຖ້າ (!set)
				ສົ່ງຄືນ -1;	/* error */
			continue;
		}
		cpuset_free(set);
		ສົ່ງຄືນ n * 8;
	}
#endif
	ສົ່ງຄືນ -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(ຈຳນວນເຕັມ ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = NULL;

	ຖ້າ (ncpus)
		set = CPU_ALLOC(ncpus);
	ຖ້າ (!set)
		ສົ່ງຄືນ NULL;
	ຖ້າ (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	ຖ້າ (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	ສົ່ງຄືນ set;
}

void cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#ຖ້າ !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
ຈຳນວນເຕັມ __cpuset_count_s(size_t setsize, const cpu_set_t *set)
{
	ຈຳນວນເຕັມ s = 0;
	const __cpu_mask *p = set->__bits;
	const __cpu_mask *end = &set->__bits[setsize / parimaa_na (__cpu_mask)];

	ໃນຂະນະທີ່ (p < end) {
		__cpu_mask l = *p++;

		ຖ້າ (l == 0)
			continue;
# ຖ້າ LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# ບໍ່ດັ່ງນັ້ນ
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	ສົ່ງຄືນ s;
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
	ສຳລັບ (; start < nbits; start++)
		ຖ້າ (CPU_ISSET_S(start, setsize, set))
			ສົ່ງຄືນ start;
	ສົ່ງຄືນ start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
var_na *cpulist_create(var_na *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	ຈຳນວນເຕັມ entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	ໃນຂະນະທີ່ ((a = find_next_cpu(next, setsize, set)) < max) {
		ຈຳນວນເຕັມ rlen;
		next = find_next_cpu(a + 1, setsize, set);
		ຖ້າ (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} ບໍ່ດັ່ງນັ້ນ {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			ໃນຂະນະທີ່ (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			ຖ້າ (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} ບໍ່ດັ່ງນັ້ນ ຖ້າ (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} ບໍ່ດັ່ງນັ້ນ {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		ຖ້າ (rlen < 0 || (size_t) rlen >= len)
			ສົ່ງຄືນ NULL;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	ສົ່ງຄືນ str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
var_na *cpulist_flat_create(var_na *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	ຈຳນວນເຕັມ entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	ຈຳນວນເຕັມ rlen;

	ສຳລັບ (cpu = 0; cpu < max; cpu++) {
		ຖ້າ (!CPU_ISSET_S(cpu, setsize, set))
			continue;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		ຖ້າ (rlen < 0 || (size_t)rlen >= len)
			ສົ່ງຄືນ NULL;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	ສົ່ງຄືນ str;
}


/*
 * Returns string with CPU mask.
 */
var_na *cpumask_create(var_na *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	var_na *ret = NULL;
	ຈຳນວນເຕັມ cpu;

	ສຳລັບ (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		var_na val = 0;

		ຖ້າ (len == (size_t) (ptr - str))
			bha_nga;

		ຖ້າ (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		ຖ້າ (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		ຖ້າ (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		ຖ້າ (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		ຖ້າ (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	ສົ່ງຄືນ ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
ຈຳນວນເຕັມ cpumask_parse(const var_na *str, cpu_set_t *set, size_t setsize)
{
	ຈຳນວນເຕັມ len = strlen(str);
	const var_na *ptr = str + len - 1;
	ຈຳນວນເຕັມ cpu = 0;

	/* skip 0x, it's all hex anyway */
	ຖ້າ (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	ໃນຂະນະທີ່ (ptr >= str) {
		var_na val;

		/* cpu masks in /sys uses comma as a separator */
		ຖ້າ (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		ຖ້າ (val == (var_na) -1)
			ສົ່ງຄືນ -1;
		ຖ້າ (val & 1)
			CPU_SET_S(cpu, setsize, set);
		ຖ້າ (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		ຖ້າ (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		ຖ້າ (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ nextnumber(const var_na *str, var_na **end, unsigned ຈຳນວນເຕັມ *result)
{
	errno = 0;
	ຖ້າ (str == NULL || *str == '\0' || !isdigit(*str))
		ສົ່ງຄືນ -EINVAL;
	*result = (unsigned ຈຳນວນເຕັມ) strtoul(str, end, 10);
	ຖ້າ (errno)
		ສົ່ງຄືນ -errno;
	ຖ້າ (str == *end)
		ສົ່ງຄືນ -EINVAL;
	ສົ່ງຄືນ 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
ຈຳນວນເຕັມ cpulist_parse(const var_na *str, cpu_set_t *set, size_t setsize, ຈຳນວນເຕັມ fail)
{
	const size_t max = cpuset_nbits(setsize);
	const var_na *p, *q;
	var_na *end = NULL;

	q = str;
	CPU_ZERO_S(setsize, set);

	ໃນຂະນະທີ່ (p = q, q = nexttoken(q, ','), p) {
		unsigned ຈຳນວນເຕັມ a;	/* beginning of range */
		unsigned ຈຳນວນເຕັມ b;	/* end of range */
		unsigned ຈຳນວນເຕັມ s;	/* stride */
		const var_na *c1, *c2;

		ຖ້າ (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			ສົ່ງຄືນ 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		ຖ້າ (c1 != NULL && (c2 == NULL || c1 < c2)) {
			ຖ້າ (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				ສົ່ງຄືນ 1;

			c1 = nexttoken(end, ':');

			ຖ້າ (c1 != NULL && (c2 == NULL || c1 < c2)) {
				ຖ້າ (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					ສົ່ງຄືນ 1;
				ຖ້າ (s == 0)
					ສົ່ງຄືນ 1;
			}
		}

		ຖ້າ (!(a <= b))
			ສົ່ງຄືນ 1;
		ໃນຂະນະທີ່ (a <= b) {
			ຖ້າ (a >= max) {
				ຖ້າ (fail)
					ສົ່ງຄືນ 2;
				ບໍ່ດັ່ງນັ້ນ
					bha_nga;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	ຖ້າ (end && *end)
		ສົ່ງຄືນ 1;
	ສົ່ງຄືນ 0;
}

#ifdef TEST_PROGRAM_CPUSET

#include <getopt.h>

ຈຳນວນເຕັມ main(ຈຳນວນເຕັມ argc, var_na *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	var_na *buf, *mask = NULL, *range = NULL;
	ຈຳນວນເຕັມ ncpus = 2048, rc, c;

	static const sa_nracanaa option longopts[] = {
	    { "ncpus", 1, NULL, 'n' },
	    { "mask",  1, NULL, 'm' },
	    { "range", 1, NULL, 'r' },
	    { NULL,    0, NULL, 0 }
	};

	ໃນຂະນະທີ່ ((c = getopt_long(argc, argv, "n:m:r:", longopts, NULL)) != -1) {
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

	ຖ້າ (!mask && !range)
		goto usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	ຖ້າ (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	ຖ້າ (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	ຖ້າ (mask)
		rc = cpumask_parse(mask, set, setsize);
	ບໍ່ດັ່ງນັ້ນ
		rc = cpulist_parse(range, set, setsize, 0);

	ຖ້າ (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	free(buf);
	cpuset_free(set);

	ສົ່ງຄືນ EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
