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

#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <sched.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <ctype.h>
#ifdef HAVE_SYS_SYSCALL_H
#അന്തര്ഭാവ <sys/syscall.h>
#endif

#അന്തര്ഭാവ "cpuset.h"
#അന്തര്ഭാവ "c.h"

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ val_to_char(പൂർണ്ണസംഖ്യ v)
{
	എങ്കിൽ (v >= 0 && v < 10)
		തിരികെ '0' + v;
	എങ്കിൽ (v >= 10 && v < 16)
		തിരികെ ('a' - 10) + v;
	തിരികെ -1;
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ char_to_val(പൂർണ്ണസംഖ്യ c)
{
	പൂർണ്ണസംഖ്യ cl;

	എങ്കിൽ (c >= '0' && c <= '9')
		തിരികെ c - '0';
	cl = tolower(c);
	എങ്കിൽ (cl >= 'a' && cl <= 'f')
		തിരികെ cl + (10 - 'a');
	തിരികെ -1;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *nexttoken(സ്ഥിരം അക്ഷരം *q,  പൂർണ്ണസംഖ്യ sep)
{
	എങ്കിൽ (q)
		q = strchr(q, sep);
	എങ്കിൽ (q)
		q++;
	തിരികെ q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
പൂർണ്ണസംഖ്യ get_max_സംഖ്യ_of_cpus(ശൂന്യം)
{
#ifdef SYS_sched_getaffinity
	പൂർണ്ണസംഖ്യ n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, ശൂന്യ);

	എങ്കിൽ (!set)
		തിരികെ -1;	/* error */

	വേണ്ടി (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		എങ്കിൽ (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, ശൂന്യ);
			എങ്കിൽ (!set)
				തിരികെ -1;	/* error */
			തുടരുക;
		}
		cpuset_free(set);
		തിരികെ n * 8;
	}
#endif
	തിരികെ -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(പൂർണ്ണസംഖ്യ ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = ശൂന്യ;

	എങ്കിൽ (ncpus)
		set = CPU_ALLOC(ncpus);
	എങ്കിൽ (!set)
		തിരികെ ശൂന്യ;
	എങ്കിൽ (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	എങ്കിൽ (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	തിരികെ set;
}

ശൂന്യം cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#എങ്കിൽ !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
പൂർണ്ണസംഖ്യ __cpuset_count_s(size_t setsize, സ്ഥിരം cpu_set_t *set)
{
	പൂർണ്ണസംഖ്യ s = 0;
	സ്ഥിരം __cpu_mask *p = set->__bits;
	സ്ഥിരം __cpu_mask *end = &set->__bits[setsize / വലുപ്പം (__cpu_mask)];

	ആകുമ്പോൾ (p < end) {
		__cpu_mask l = *p++;

		എങ്കിൽ (l == 0)
			തുടരുക;
# എങ്കിൽ LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# അല്ലെങ്കിൽ
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	തിരികെ s;
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
സ്ഥിതം size_t find_next_cpu(size_t start, size_t setsize, cpu_set_t *set)
{
	size_t nbits = cpuset_nbits(setsize);
	വേണ്ടി (; start < nbits; start++)
		എങ്കിൽ (CPU_ISSET_S(start, setsize, set))
			തിരികെ start;
	തിരികെ start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
അക്ഷരം *cpulist_create(അക്ഷരം *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	അക്ഷരം *ptr = str;
	പൂർണ്ണസംഖ്യ entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	ആകുമ്പോൾ ((a = find_next_cpu(next, setsize, set)) < max) {
		പൂർണ്ണസംഖ്യ rlen;
		next = find_next_cpu(a + 1, setsize, set);
		എങ്കിൽ (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} അല്ലെങ്കിൽ {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			ആകുമ്പോൾ (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			എങ്കിൽ (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} അല്ലെങ്കിൽ എങ്കിൽ (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} അല്ലെങ്കിൽ {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		എങ്കിൽ (rlen < 0 || (size_t) rlen >= len)
			തിരികെ ശൂന്യ;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	തിരികെ str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
അക്ഷരം *cpulist_flat_create(അക്ഷരം *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	അക്ഷരം *ptr = str;
	പൂർണ്ണസംഖ്യ entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	പൂർണ്ണസംഖ്യ rlen;

	വേണ്ടി (cpu = 0; cpu < max; cpu++) {
		എങ്കിൽ (!CPU_ISSET_S(cpu, setsize, set))
			തുടരുക;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		എങ്കിൽ (rlen < 0 || (size_t)rlen >= len)
			തിരികെ ശൂന്യ;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	തിരികെ str;
}


/*
 * Returns string with CPU mask.
 */
അക്ഷരം *cpumask_create(അക്ഷരം *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	അക്ഷരം *ptr = str;
	അക്ഷരം *ret = ശൂന്യ;
	പൂർണ്ണസംഖ്യ cpu;

	വേണ്ടി (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		അക്ഷരം val = 0;

		എങ്കിൽ (len == (size_t) (ptr - str))
			നിർത്തുക;

		എങ്കിൽ (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		എങ്കിൽ (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		എങ്കിൽ (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		എങ്കിൽ (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		എങ്കിൽ (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	തിരികെ ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
പൂർണ്ണസംഖ്യ cpumask_parse(സ്ഥിരം അക്ഷരം *str, cpu_set_t *set, size_t setsize)
{
	പൂർണ്ണസംഖ്യ len = strlen(str);
	സ്ഥിരം അക്ഷരം *ptr = str + len - 1;
	പൂർണ്ണസംഖ്യ cpu = 0;

	/* skip 0x, it's all hex anyway */
	എങ്കിൽ (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	ആകുമ്പോൾ (ptr >= str) {
		അക്ഷരം val;

		/* cpu masks in /sys uses comma as a separator */
		എങ്കിൽ (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		എങ്കിൽ (val == (അക്ഷരം) -1)
			തിരികെ -1;
		എങ്കിൽ (val & 1)
			CPU_SET_S(cpu, setsize, set);
		എങ്കിൽ (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		എങ്കിൽ (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		എങ്കിൽ (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ nextnumber(സ്ഥിരം അക്ഷരം *str, അക്ഷരം **end, unsigned പൂർണ്ണസംഖ്യ *result)
{
	errno = 0;
	എങ്കിൽ (str == ശൂന്യ || *str == '\0' || !isdigit(*str))
		തിരികെ -EINVAL;
	*result = (unsigned പൂർണ്ണസംഖ്യ) strtoul(str, end, 10);
	എങ്കിൽ (errno)
		തിരികെ -errno;
	എങ്കിൽ (str == *end)
		തിരികെ -EINVAL;
	തിരികെ 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
പൂർണ്ണസംഖ്യ cpulist_parse(സ്ഥിരം അക്ഷരം *str, cpu_set_t *set, size_t setsize, പൂർണ്ണസംഖ്യ fail)
{
	സ്ഥിരം size_t max = cpuset_nbits(setsize);
	സ്ഥിരം അക്ഷരം *p, *q;
	അക്ഷരം *end = ശൂന്യ;

	q = str;
	CPU_ZERO_S(setsize, set);

	ആകുമ്പോൾ (p = q, q = nexttoken(q, ','), p) {
		unsigned പൂർണ്ണസംഖ്യ a;	/* beginning of range */
		unsigned പൂർണ്ണസംഖ്യ b;	/* end of range */
		unsigned പൂർണ്ണസംഖ്യ s;	/* stride */
		സ്ഥിരം അക്ഷരം *c1, *c2;

		എങ്കിൽ (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			തിരികെ 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		എങ്കിൽ (c1 != ശൂന്യ && (c2 == ശൂന്യ || c1 < c2)) {
			എങ്കിൽ (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				തിരികെ 1;

			c1 = nexttoken(end, ':');

			എങ്കിൽ (c1 != ശൂന്യ && (c2 == ശൂന്യ || c1 < c2)) {
				എങ്കിൽ (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					തിരികെ 1;
				എങ്കിൽ (s == 0)
					തിരികെ 1;
			}
		}

		എങ്കിൽ (!(a <= b))
			തിരികെ 1;
		ആകുമ്പോൾ (a <= b) {
			എങ്കിൽ (a >= max) {
				എങ്കിൽ (fail)
					തിരികെ 2;
				അല്ലെങ്കിൽ
					നിർത്തുക;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	എങ്കിൽ (end && *end)
		തിരികെ 1;
	തിരികെ 0;
}

#ifdef TEST_PROGRAM_CPUSET

#അന്തര്ഭാവ <getopt.h>

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	അക്ഷരം *buf, *mask = ശൂന്യ, *range = ശൂന്യ;
	പൂർണ്ണസംഖ്യ ncpus = 2048, rc, c;

	സ്ഥിതം സ്ഥിരം ഘടന option longopts[] = {
	    { "ncpus", 1, ശൂന്യ, 'n' },
	    { "mask",  1, ശൂന്യ, 'm' },
	    { "range", 1, ശൂന്യ, 'r' },
	    { ശൂന്യ,    0, ശൂന്യ, 0 }
	};

	ആകുമ്പോൾ ((c = getopt_long(argc, argv, "n:m:r:", longopts, ശൂന്യ)) != -1) {
		തിരഞ്ഞെടുപ്പ്(c) {
		അവസ്ഥ 'n':
			ncpus = atoi(optarg);
			നിർത്തുക;
		അവസ്ഥ 'm':
			mask = optarg;
			നിർത്തുക;
		അവസ്ഥ 'r':
			range = optarg;
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			പോകുക usage_err;
		}
	}

	എങ്കിൽ (!mask && !range)
		പോകുക usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	എങ്കിൽ (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	എങ്കിൽ (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	എങ്കിൽ (mask)
		rc = cpumask_parse(mask, set, setsize);
	അല്ലെങ്കിൽ
		rc = cpulist_parse(range, set, setsize, 0);

	എങ്കിൽ (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	സ്വതന്ത്രം(buf);
	cpuset_free(set);

	തിരികെ EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
