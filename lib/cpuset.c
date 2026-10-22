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

#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <sched.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <ctype.h>
#ifdef HAVE_SYS_SYSCALL_H
#அந்தர்பாவ <sys/syscall.h>
#endif

#அந்தர்பாவ "cpuset.h"
#அந்தர்பாவ "c.h"

நிலையான உள்வரி முழு val_to_char(முழு v)
{
	எனில் (v >= 0 && v < 10)
		திரும்பு '0' + v;
	எனில் (v >= 10 && v < 16)
		திரும்பு ('a' - 10) + v;
	திரும்பு -1;
}

நிலையான உள்வரி முழு char_to_val(முழு c)
{
	முழு cl;

	எனில் (c >= '0' && c <= '9')
		திரும்பு c - '0';
	cl = tolower(c);
	எனில் (cl >= 'a' && cl <= 'f')
		திரும்பு cl + (10 - 'a');
	திரும்பு -1;
}

நிலையான மாறிலி எழுத்து *nexttoken(மாறிலி எழுத்து *q,  முழு sep)
{
	எனில் (q)
		q = strchr(q, sep);
	எனில் (q)
		q++;
	திரும்பு q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
முழு get_max_எண்_of_cpus(வெறுமை)
{
#ifdef SYS_sched_getaffinity
	முழு n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, வெற்று);

	எனில் (!set)
		திரும்பு -1;	/* error */

	க்கு (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		எனில் (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, வெற்று);
			எனில் (!set)
				திரும்பு -1;	/* error */
			தொடர்;
		}
		cpuset_free(set);
		திரும்பு n * 8;
	}
#endif
	திரும்பு -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(முழு ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = வெற்று;

	எனில் (ncpus)
		set = CPU_ALLOC(ncpus);
	எனில் (!set)
		திரும்பு வெற்று;
	எனில் (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	எனில் (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	திரும்பு set;
}

வெறுமை cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#எனில் !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
முழு __cpuset_count_s(size_t setsize, மாறிலி cpu_set_t *set)
{
	முழு s = 0;
	மாறிலி __cpu_mask *p = set->__bits;
	மாறிலி __cpu_mask *end = &set->__bits[setsize / அளவு (__cpu_mask)];

	போது (p < end) {
		__cpu_mask l = *p++;

		எனில் (l == 0)
			தொடர்;
# எனில் LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# இல்லையெனில்
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	திரும்பு s;
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
நிலையான size_t find_next_cpu(size_t start, size_t setsize, cpu_set_t *set)
{
	size_t nbits = cpuset_nbits(setsize);
	க்கு (; start < nbits; start++)
		எனில் (CPU_ISSET_S(start, setsize, set))
			திரும்பு start;
	திரும்பு start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
எழுத்து *cpulist_create(எழுத்து *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	எழுத்து *ptr = str;
	முழு entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	போது ((a = find_next_cpu(next, setsize, set)) < max) {
		முழு rlen;
		next = find_next_cpu(a + 1, setsize, set);
		எனில் (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} இல்லையெனில் {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			போது (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			எனில் (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} இல்லையெனில் எனில் (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} இல்லையெனில் {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		எனில் (rlen < 0 || (size_t) rlen >= len)
			திரும்பு வெற்று;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	திரும்பு str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
எழுத்து *cpulist_flat_create(எழுத்து *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	எழுத்து *ptr = str;
	முழு entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	முழு rlen;

	க்கு (cpu = 0; cpu < max; cpu++) {
		எனில் (!CPU_ISSET_S(cpu, setsize, set))
			தொடர்;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		எனில் (rlen < 0 || (size_t)rlen >= len)
			திரும்பு வெற்று;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	திரும்பு str;
}


/*
 * Returns string with CPU mask.
 */
எழுத்து *cpumask_create(எழுத்து *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	எழுத்து *ptr = str;
	எழுத்து *ret = வெற்று;
	முழு cpu;

	க்கு (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		எழுத்து val = 0;

		எனில் (len == (size_t) (ptr - str))
			நிறுத்து;

		எனில் (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		எனில் (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		எனில் (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		எனில் (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		எனில் (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	திரும்பு ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
முழு cpumask_parse(மாறிலி எழுத்து *str, cpu_set_t *set, size_t setsize)
{
	முழு len = strlen(str);
	மாறிலி எழுத்து *ptr = str + len - 1;
	முழு cpu = 0;

	/* skip 0x, it's all hex anyway */
	எனில் (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	போது (ptr >= str) {
		எழுத்து val;

		/* cpu masks in /sys uses comma as a separator */
		எனில் (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		எனில் (val == (எழுத்து) -1)
			திரும்பு -1;
		எனில் (val & 1)
			CPU_SET_S(cpu, setsize, set);
		எனில் (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		எனில் (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		எனில் (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	திரும்பு 0;
}

நிலையான முழு nextnumber(மாறிலி எழுத்து *str, எழுத்து **end, unsigned முழு *result)
{
	errno = 0;
	எனில் (str == வெற்று || *str == '\0' || !isdigit(*str))
		திரும்பு -EINVAL;
	*result = (unsigned முழு) strtoul(str, end, 10);
	எனில் (errno)
		திரும்பு -errno;
	எனில் (str == *end)
		திரும்பு -EINVAL;
	திரும்பு 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
முழு cpulist_parse(மாறிலி எழுத்து *str, cpu_set_t *set, size_t setsize, முழு fail)
{
	மாறிலி size_t max = cpuset_nbits(setsize);
	மாறிலி எழுத்து *p, *q;
	எழுத்து *end = வெற்று;

	q = str;
	CPU_ZERO_S(setsize, set);

	போது (p = q, q = nexttoken(q, ','), p) {
		unsigned முழு a;	/* beginning of range */
		unsigned முழு b;	/* end of range */
		unsigned முழு s;	/* stride */
		மாறிலி எழுத்து *c1, *c2;

		எனில் (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			திரும்பு 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		எனில் (c1 != வெற்று && (c2 == வெற்று || c1 < c2)) {
			எனில் (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				திரும்பு 1;

			c1 = nexttoken(end, ':');

			எனில் (c1 != வெற்று && (c2 == வெற்று || c1 < c2)) {
				எனில் (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					திரும்பு 1;
				எனில் (s == 0)
					திரும்பு 1;
			}
		}

		எனில் (!(a <= b))
			திரும்பு 1;
		போது (a <= b) {
			எனில் (a >= max) {
				எனில் (fail)
					திரும்பு 2;
				இல்லையெனில்
					நிறுத்து;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	எனில் (end && *end)
		திரும்பு 1;
	திரும்பு 0;
}

#ifdef TEST_PROGRAM_CPUSET

#அந்தர்பாவ <getopt.h>

முழு முதன்மை(முழு argc, எழுத்து *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	எழுத்து *buf, *mask = வெற்று, *range = வெற்று;
	முழு ncpus = 2048, rc, c;

	நிலையான மாறிலி கட்டமைப்பு option longopts[] = {
	    { "ncpus", 1, வெற்று, 'n' },
	    { "mask",  1, வெற்று, 'm' },
	    { "range", 1, வெற்று, 'r' },
	    { வெற்று,    0, வெற்று, 0 }
	};

	போது ((c = getopt_long(argc, argv, "n:m:r:", longopts, வெற்று)) != -1) {
		தேர்வு(c) {
		நிலை 'n':
			ncpus = atoi(optarg);
			நிறுத்து;
		நிலை 'm':
			mask = optarg;
			நிறுத்து;
		நிலை 'r':
			range = optarg;
			நிறுத்து;
		இயல்பு:
			செல் usage_err;
		}
	}

	எனில் (!mask && !range)
		செல் usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	எனில் (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	எனில் (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	எனில் (mask)
		rc = cpumask_parse(mask, set, setsize);
	இல்லையெனில்
		rc = cpulist_parse(range, set, setsize, 0);

	எனில் (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	விடுவி(buf);
	cpuset_free(set);

	திரும்பு EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
