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

#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <sched.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ifdef HAVE_SYS_SYSCALL_H
#ଅନ୍ତର୍ଭାଵ <sys/syscall.h>
#endif

#ଅନ୍ତର୍ଭାଵ "cpuset.h"
#ଅନ୍ତର୍ଭାଵ "c.h"

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା val_to_char(ପୂର୍ଣ୍ଣସଂଖ୍ୟା v)
{
	ଯଦି (v >= 0 && v < 10)
		ଫେରନ୍ତୁ '0' + v;
	ଯଦି (v >= 10 && v < 16)
		ଫେରନ୍ତୁ ('a' - 10) + v;
	ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା char_to_val(ପୂର୍ଣ୍ଣସଂଖ୍ୟା c)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cl;

	ଯଦି (c >= '0' && c <= '9')
		ଫେରନ୍ତୁ c - '0';
	cl = tolower(c);
	ଯଦି (cl >= 'a' && cl <= 'f')
		ଫେରନ୍ତୁ cl + (10 - 'a');
	ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *nexttoken(ଅଚଳ ଅକ୍ଷର *q,  ପୂର୍ଣ୍ଣସଂଖ୍ୟା sep)
{
	ଯଦି (q)
		q = strchr(q, sep);
	ଯଦି (q)
		q++;
	ଫେରନ୍ତୁ q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_max_ସଂଖ୍ୟା_of_cpus(ଶୂନ୍ୟ)
{
#ifdef SYS_sched_getaffinity
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, ଖାଲି);

	ଯଦି (!set)
		ଫେରନ୍ତୁ -1;	/* error */

	ପାଇଁ (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		ଯଦି (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, ଖାଲି);
			ଯଦି (!set)
				ଫେରନ୍ତୁ -1;	/* error */
			ଜାରି;
		}
		cpuset_free(set);
		ଫେରନ୍ତୁ n * 8;
	}
#endif
	ଫେରନ୍ତୁ -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = ଖାଲି;

	ଯଦି (ncpus)
		set = CPU_ALLOC(ncpus);
	ଯଦି (!set)
		ଫେରନ୍ତୁ ଖାଲି;
	ଯଦି (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	ଯଦି (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	ଫେରନ୍ତୁ set;
}

ଶୂନ୍ୟ cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#ଯଦି !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା __cpuset_count_s(size_t setsize, ଅଚଳ cpu_set_t *set)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା s = 0;
	ଅଚଳ __cpu_mask *p = set->__bits;
	ଅଚଳ __cpu_mask *end = &set->__bits[setsize / ଆକାର (__cpu_mask)];

	ଯେତେବେଳେ (p < end) {
		__cpu_mask l = *p++;

		ଯଦି (l == 0)
			ଜାରି;
# ଯଦି LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# ଅନ୍ୟଥା
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	ଫେରନ୍ତୁ s;
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
ସ୍ଥିର size_t find_next_cpu(size_t start, size_t setsize, cpu_set_t *set)
{
	size_t nbits = cpuset_nbits(setsize);
	ପାଇଁ (; start < nbits; start++)
		ଯଦି (CPU_ISSET_S(start, setsize, set))
			ଫେରନ୍ତୁ start;
	ଫେରନ୍ତୁ start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
ଅକ୍ଷର *cpulist_create(ଅକ୍ଷର *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	ଅକ୍ଷର *ptr = str;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	ଯେତେବେଳେ ((a = find_next_cpu(next, setsize, set)) < max) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rlen;
		next = find_next_cpu(a + 1, setsize, set);
		ଯଦି (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} ଅନ୍ୟଥା {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			ଯେତେବେଳେ (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			ଯଦି (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} ଅନ୍ୟଥା ଯଦି (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} ଅନ୍ୟଥା {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		ଯଦି (rlen < 0 || (size_t) rlen >= len)
			ଫେରନ୍ତୁ ଖାଲି;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	ଫେରନ୍ତୁ str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
ଅକ୍ଷର *cpulist_flat_create(ଅକ୍ଷର *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	ଅକ୍ଷର *ptr = str;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rlen;

	ପାଇଁ (cpu = 0; cpu < max; cpu++) {
		ଯଦି (!CPU_ISSET_S(cpu, setsize, set))
			ଜାରି;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		ଯଦି (rlen < 0 || (size_t)rlen >= len)
			ଫେରନ୍ତୁ ଖାଲି;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	ଫେରନ୍ତୁ str;
}


/*
 * Returns string with CPU mask.
 */
ଅକ୍ଷର *cpumask_create(ଅକ୍ଷର *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	ଅକ୍ଷର *ptr = str;
	ଅକ୍ଷର *ret = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu;

	ପାଇଁ (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		ଅକ୍ଷର val = 0;

		ଯଦି (len == (size_t) (ptr - str))
			ଭାଙ୍ଗ;

		ଯଦି (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		ଯଦି (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		ଯଦି (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		ଯଦି (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		ଯଦି (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	ଫେରନ୍ତୁ ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpumask_parse(ଅଚଳ ଅକ୍ଷର *str, cpu_set_t *set, size_t setsize)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = strlen(str);
	ଅଚଳ ଅକ୍ଷର *ptr = str + len - 1;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu = 0;

	/* skip 0x, it's all hex anyway */
	ଯଦି (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	ଯେତେବେଳେ (ptr >= str) {
		ଅକ୍ଷର val;

		/* cpu masks in /sys uses comma as a separator */
		ଯଦି (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		ଯଦି (val == (ଅକ୍ଷର) -1)
			ଫେରନ୍ତୁ -1;
		ଯଦି (val & 1)
			CPU_SET_S(cpu, setsize, set);
		ଯଦି (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		ଯଦି (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		ଯଦି (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା nextnumber(ଅଚଳ ଅକ୍ଷର *str, ଅକ୍ଷର **end, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *result)
{
	errno = 0;
	ଯଦି (str == ଖାଲି || *str == '\0' || !isdigit(*str))
		ଫେରନ୍ତୁ -EINVAL;
	*result = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) strtoul(str, end, 10);
	ଯଦି (errno)
		ଫେରନ୍ତୁ -errno;
	ଯଦି (str == *end)
		ଫେରନ୍ତୁ -EINVAL;
	ଫେରନ୍ତୁ 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpulist_parse(ଅଚଳ ଅକ୍ଷର *str, cpu_set_t *set, size_t setsize, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fail)
{
	ଅଚଳ size_t max = cpuset_nbits(setsize);
	ଅଚଳ ଅକ୍ଷର *p, *q;
	ଅକ୍ଷର *end = ଖାଲି;

	q = str;
	CPU_ZERO_S(setsize, set);

	ଯେତେବେଳେ (p = q, q = nexttoken(q, ','), p) {
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା a;	/* beginning of range */
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା b;	/* end of range */
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା s;	/* stride */
		ଅଚଳ ଅକ୍ଷର *c1, *c2;

		ଯଦି (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			ଫେରନ୍ତୁ 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		ଯଦି (c1 != ଖାଲି && (c2 == ଖାଲି || c1 < c2)) {
			ଯଦି (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				ଫେରନ୍ତୁ 1;

			c1 = nexttoken(end, ':');

			ଯଦି (c1 != ଖାଲି && (c2 == ଖାଲି || c1 < c2)) {
				ଯଦି (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					ଫେରନ୍ତୁ 1;
				ଯଦି (s == 0)
					ଫେରନ୍ତୁ 1;
			}
		}

		ଯଦି (!(a <= b))
			ଫେରନ୍ତୁ 1;
		ଯେତେବେଳେ (a <= b) {
			ଯଦି (a >= max) {
				ଯଦି (fail)
					ଫେରନ୍ତୁ 2;
				ଅନ୍ୟଥା
					ଭାଙ୍ଗ;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	ଯଦି (end && *end)
		ଫେରନ୍ତୁ 1;
	ଫେରନ୍ତୁ 0;
}

#ifdef TEST_PROGRAM_CPUSET

#ଅନ୍ତର୍ଭାଵ <getopt.h>

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	ଅକ୍ଷର *buf, *mask = ଖାଲି, *range = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ncpus = 2048, rc, c;

	ସ୍ଥିର ଅଚଳ ସଂରଚନା option longopts[] = {
	    { "ncpus", 1, ଖାଲି, 'n' },
	    { "mask",  1, ଖାଲି, 'm' },
	    { "range", 1, ଖାଲି, 'r' },
	    { ଖାଲି,    0, ଖାଲି, 0 }
	};

	ଯେତେବେଳେ ((c = getopt_long(argc, argv, "n:m:r:", longopts, ଖାଲି)) != -1) {
		ବାଛ(c) {
		ସ୍ଥିତି 'n':
			ncpus = atoi(optarg);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'm':
			mask = optarg;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'r':
			range = optarg;
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଯାଅ usage_err;
		}
	}

	ଯଦି (!mask && !range)
		ଯାଅ usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	ଯଦି (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	ଯଦି (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	ଯଦି (mask)
		rc = cpumask_parse(mask, set, setsize);
	ଅନ୍ୟଥା
		rc = cpulist_parse(range, set, setsize, 0);

	ଯଦି (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	ମୁକ୍ତ(buf);
	cpuset_free(set);

	ଫେରନ୍ତୁ EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
