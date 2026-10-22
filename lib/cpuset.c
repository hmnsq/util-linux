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

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <sched.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_SYS_SYSCALL_H
#ಅನ್ತರ್ಭಾವ <sys/syscall.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "cpuset.h"
#ಅನ್ತರ್ಭಾವ "c.h"

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ val_ಗಮ್ಯ_char(ಪೂರ್ಣಾಂಕ v)
{
	ಆದರೆ (v >= 0 && v < 10)
		ಹಿಂತಿರುಗಿ '0' + v;
	ಆದರೆ (v >= 10 && v < 16)
		ಹಿಂತಿರುಗಿ ('a' - 10) + v;
	ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ char_ಗಮ್ಯ_val(ಪೂರ್ಣಾಂಕ c)
{
	ಪೂರ್ಣಾಂಕ cl;

	ಆದರೆ (c >= '0' && c <= '9')
		ಹಿಂತಿರುಗಿ c - '0';
	cl = tolower(c);
	ಆದರೆ (cl >= 'a' && cl <= 'f')
		ಹಿಂತಿರುಗಿ cl + (10 - 'a');
	ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *nexttoken(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *q,  ಪೂರ್ಣಾಂಕ sep)
{
	ಆದರೆ (q)
		q = strchr(q, sep);
	ಆದರೆ (q)
		q++;
	ಹಿಂತಿರುಗಿ q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
ಪೂರ್ಣಾಂಕ get_max_ಸಂಖ್ಯೆ_ದ_cpus(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SYS_sched_getaffinity
	ಪೂರ್ಣಾಂಕ n, cpus = 2048;
	size_t setsize;
	cpu_set_t *ಗಣ = cpuset_alloc(cpus, &setsize, ಖಾಲಿ);

	ಆದರೆ (!ಗಣ)
		ಹಿಂತಿರುಗಿ -1;	/* error */

	ಗಾಗಿ (;;) {
		CPU_ZERO_S(setsize, ಗಣ);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, ಗಣ);

		ಆದರೆ (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(ಗಣ);
			cpus *= 2;
			ಗಣ = cpuset_alloc(cpus, &setsize, ಖಾಲಿ);
			ಆದರೆ (!ಗಣ)
				ಹಿಂತಿರುಗಿ -1;	/* error */
			ಮುಂದುವರಿ;
		}
		cpuset_free(ಗಣ);
		ಹಿಂತಿರುಗಿ n * 8;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(ಪೂರ್ಣಾಂಕ ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *ಗಣ = ಖಾಲಿ;

	ಆದರೆ (ncpus)
		ಗಣ = CPU_ALLOC(ncpus);
	ಆದರೆ (!ಗಣ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಆದರೆ (setsize)
		*setsize = CPU_ALLOC_ಗಾತ್ರ(ncpus);
	ಆದರೆ (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_ಗಾತ್ರ(ncpus));
	ಹಿಂತಿರುಗಿ ಗಣ;
}

ಶೂನ್ಯ cpuset_free(cpu_set_t *ಗಣ)
{
	CPU_FREE(ಗಣ);
}

#ಆದರೆ !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
ಪೂರ್ಣಾಂಕ __cpuset_count_s(size_t setsize, ಸ್ಥಿರಾಂಕ cpu_set_t *ಗಣ)
{
	ಪೂರ್ಣಾಂಕ s = 0;
	ಸ್ಥಿರಾಂಕ __cpu_mask *p = ಗಣ->__bits;
	ಸ್ಥಿರಾಂಕ __cpu_mask *ಕೊನೆ = &ಗಣ->__bits[setsize / ಗಾತ್ರ (__cpu_mask)];

	ಆಗ (p < ಕೊನೆ) {
		__cpu_mask l = *p++;

		ಆದರೆ (l == 0)
			ಮುಂದುವರಿ;
# ಆದರೆ LONG_ಬಿಟ್ > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# ಇಲ್ಲದಿದ್ದರೆ
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	ಹಿಂತಿರುಗಿ s;
}
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ size_t ಕಂಡುಹಿಡಿ_ಮುಂದಿನ_cpu(size_t ಆರಂಭ, size_t setsize, cpu_set_t *ಗಣ)
{
	size_t nbits = cpuset_nbits(setsize);
	ಗಾಗಿ (; ಆರಂಭ < nbits; ಆರಂಭ++)
		ಆದರೆ (CPU_ISSET_S(ಆರಂಭ, setsize, ಗಣ))
			ಹಿಂತಿರುಗಿ ಆರಂಭ;
	ಹಿಂತಿರುಗಿ ಆರಂಭ;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
ಅಕ್ಷರ *cpulist_create(ಅಕ್ಷರ *str, size_t len,
			cpu_set_t *ಗಣ, size_t setsize)
{
	ಅಕ್ಷರ *ptr = str;
	ಪೂರ್ಣಾಂಕ ನಮೂದು_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t ಮುಂದಿನ = 0;  /* where to start looking for next cpu */

	ಆಗ ((a = ಕಂಡುಹಿಡಿ_ಮುಂದಿನ_cpu(ಮುಂದಿನ, setsize, ಗಣ)) < max) {
		ಪೂರ್ಣಾಂಕ rlen;
		ಮುಂದಿನ = ಕಂಡುಹಿಡಿ_ಮುಂದಿನ_cpu(a + 1, setsize, ಗಣ);
		ಆದರೆ (ಮುಂದಿನ == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Extend range as long as we have the same stride. */
			size_t b = ಮುಂದಿನ;
			size_t s = b - a;
			ಆಗ (((ಮುಂದಿನ = ಕಂಡುಹಿಡಿ_ಮುಂದಿನ_cpu(b + 1, setsize, ಗಣ)) <
				max) && ಮುಂದಿನ - b == s) {
				b = ಮುಂದಿನ;
			}
			ಆದರೆ (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				ಮುಂದಿನ = b;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} ಇಲ್ಲದಿದ್ದರೆ {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		ಆದರೆ (rlen < 0 || (size_t) rlen >= len)
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		ptr += rlen;
		len -= rlen;
		ನಮೂದು_made = 1;
	}
	ptr -= ನಮೂದು_made;
	*ptr = '\0';

	ಹಿಂತಿರುಗಿ str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
ಅಕ್ಷರ *cpulist_flat_create(ಅಕ್ಷರ *str, size_t len,
			    cpu_set_t *ಗಣ, size_t setsize)
{
	ಅಕ್ಷರ *ptr = str;
	ಪೂರ್ಣಾಂಕ ನಮೂದು_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	ಪೂರ್ಣಾಂಕ rlen;

	ಗಾಗಿ (cpu = 0; cpu < max; cpu++) {
		ಆದರೆ (!CPU_ISSET_S(cpu, setsize, ಗಣ))
			ಮುಂದುವರಿ;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		ಆದರೆ (rlen < 0 || (size_t)rlen >= len)
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;

		ptr += rlen;
		len -= rlen;
		ನಮೂದು_made = 1;
	}
	ptr -= ನಮೂದು_made;
	*ptr = '\0';

	ಹಿಂತಿರುಗಿ str;
}


/*
 * Returns string with CPU mask.
 */
ಅಕ್ಷರ *cpumask_create(ಅಕ್ಷರ *str, size_t len,
			cpu_set_t *ಗಣ, size_t setsize)
{
	ಅಕ್ಷರ *ptr = str;
	ಅಕ್ಷರ *ret = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ cpu;

	ಗಾಗಿ (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		ಅಕ್ಷರ val = 0;

		ಆದರೆ (len == (size_t) (ptr - str))
			ನಿಲ್ಲಿಸು;

		ಆದರೆ (CPU_ISSET_S(cpu, setsize, ಗಣ))
			val |= 1;
		ಆದರೆ (CPU_ISSET_S(cpu + 1, setsize, ಗಣ))
			val |= 2;
		ಆದರೆ (CPU_ISSET_S(cpu + 2, setsize, ಗಣ))
			val |= 4;
		ಆದರೆ (CPU_ISSET_S(cpu + 3, setsize, ಗಣ))
			val |= 8;

		ಆದರೆ (!ret && val)
			ret = ptr;
		*ptr++ = val_ಗಮ್ಯ_char(val);
	}
	*ptr = '\0';
	ಹಿಂತಿರುಗಿ ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
ಪೂರ್ಣಾಂಕ cpumask_parse(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, cpu_set_t *ಗಣ, size_t setsize)
{
	ಪೂರ್ಣಾಂಕ len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str);
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ptr = str + len - 1;
	ಪೂರ್ಣಾಂಕ cpu = 0;

	/* skip 0x, it's all hex anyway */
	ಆದರೆ (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, ಗಣ);

	ಆಗ (ptr >= str) {
		ಅಕ್ಷರ val;

		/* cpu masks in /sys uses comma as a separator */
		ಆದರೆ (*ptr == ',')
			ptr--;

		val = char_ಗಮ್ಯ_val(*ptr);
		ಆದರೆ (val == (ಅಕ್ಷರ) -1)
			ಹಿಂತಿರುಗಿ -1;
		ಆದರೆ (val & 1)
			CPU_ಗಣ_S(cpu, setsize, ಗಣ);
		ಆದರೆ (val & 2)
			CPU_ಗಣ_S(cpu + 1, setsize, ಗಣ);
		ಆದರೆ (val & 4)
			CPU_ಗಣ_S(cpu + 2, setsize, ಗಣ);
		ಆದರೆ (val & 8)
			CPU_ಗಣ_S(cpu + 3, setsize, ಗಣ);
		ptr--;
		cpu += 4;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ nextnumber(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಅಕ್ಷರ **ಕೊನೆ, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *ಫಲಿತಾಂಶ)
{
	errno = 0;
	ಆದರೆ (str == ಖಾಲಿ || *str == '\0' || !ಅಂಕಿಯೇ(*str))
		ಹಿಂತಿರುಗಿ -EINVAL;
	*ಫಲಿತಾಂಶ = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) strtoul(str, ಕೊನೆ, 10);
	ಆದರೆ (errno)
		ಹಿಂತಿರುಗಿ -errno;
	ಆದರೆ (str == *ಕೊನೆ)
		ಹಿಂತಿರುಗಿ -EINVAL;
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
ಪೂರ್ಣಾಂಕ cpulist_parse(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, cpu_set_t *ಗಣ, size_t setsize, ಪೂರ್ಣಾಂಕ fail)
{
	ಸ್ಥಿರಾಂಕ size_t max = cpuset_nbits(setsize);
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p, *q;
	ಅಕ್ಷರ *ಕೊನೆ = ಖಾಲಿ;

	q = str;
	CPU_ZERO_S(setsize, ಗಣ);

	ಆಗ (p = q, q = nexttoken(q, ','), p) {
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ a;	/* beginning of range */
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ b;	/* end of range */
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ s;	/* stride */
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *c1, *c2;

		ಆದರೆ (nextnumber(p, &ಕೊನೆ, &a) != 0 ||
			(*ಕೊನೆ && *ಕೊನೆ != ',' && *ಕೊನೆ != '-'))
			ಹಿಂತಿರುಗಿ 1;
		b = a;
		s = 1;
		p = ಕೊನೆ;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		ಆದರೆ (c1 != ಖಾಲಿ && (c2 == ಖಾಲಿ || c1 < c2)) {
			ಆದರೆ (nextnumber(c1, &ಕೊನೆ, &b) != 0 ||
				(*ಕೊನೆ && *ಕೊನೆ != ',' && *ಕೊನೆ != ':'))
				ಹಿಂತಿರುಗಿ 1;

			c1 = nexttoken(ಕೊನೆ, ':');

			ಆದರೆ (c1 != ಖಾಲಿ && (c2 == ಖಾಲಿ || c1 < c2)) {
				ಆದರೆ (nextnumber(c1, &ಕೊನೆ, &s) != 0 ||
					(*ಕೊನೆ && *ಕೊನೆ != ','))
					ಹಿಂತಿರುಗಿ 1;
				ಆದರೆ (s == 0)
					ಹಿಂತಿರುಗಿ 1;
			}
		}

		ಆದರೆ (!(a <= b))
			ಹಿಂತಿರುಗಿ 1;
		ಆಗ (a <= b) {
			ಆದರೆ (a >= max) {
				ಆದರೆ (fail)
					ಹಿಂತಿರುಗಿ 2;
				ಇಲ್ಲದಿದ್ದರೆ
					ನಿಲ್ಲಿಸು;
			}
			CPU_ಗಣ_S(a, setsize, ಗಣ);
			a += s;
		}
	}

	ಆದರೆ (ಕೊನೆ && *ಕೊನೆ)
		ಹಿಂತಿರುಗಿ 1;
	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_CPUSET

#ಅನ್ತರ್ಭಾವ <getopt.h>

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	cpu_set_t *ಗಣ;
	size_t setsize, buflen, nbits;
	ಅಕ್ಷರ *buf, *ಮುಖವಾಡ = ಖಾಲಿ, *range = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ ncpus = 2048, rc, c;

	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
	    { "ncpus", 1, ಖಾಲಿ, 'n' },
	    { "mask",  1, ಖಾಲಿ, 'm' },
	    { "range", 1, ಖಾಲಿ, 'r' },
	    { ಖಾಲಿ,    0, ಖಾಲಿ, 0 }
	};

	ಆಗ ((c = getopt_long(argc, argv, "n:m:r:", longopts, ಖಾಲಿ)) != -1) {
		ಆಯ್ಕೆ(c) {
		ಸ್ಥಿತಿ 'n':
			ncpus = atoi(optarg);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'm':
			ಮುಖವಾಡ = optarg;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'r':
			range = optarg;
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ಹೋಗು usage_err;
		}
	}

	ಆದರೆ (!ಮುಖವಾಡ && !range)
		ಹೋಗು usage_err;

	ಗಣ = cpuset_alloc(ncpus, &setsize, &nbits);
	ಆದರೆ (!ಗಣ)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = ಸ್ಮರಣೆ_ಕೊಡು(buflen);
	ಆದರೆ (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	ಆದರೆ (ಮುಖವಾಡ)
		rc = cpumask_parse(ಮುಖವಾಡ, ಗಣ, setsize);
	ಇಲ್ಲದಿದ್ದರೆ
		rc = cpulist_parse(range, ಗಣ, setsize, 0);

	ಆದರೆ (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", ಮುಖವಾಡ ? : range);

	ಬರೆ("%-15s = %15s ", ಮುಖವಾಡ ? : range,
				cpumask_create(buf, buflen, ಗಣ, setsize));
	ಬರೆ("[%s]\n", cpulist_create(buf, buflen, ಗಣ, setsize));

	ಬಿಡುಗಡೆ(buf);
	cpuset_free(ಗಣ);

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		ಕಾರ್ಯಕ್ರಮ_invocation_short_name);
	ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
}
#ಆದರೆ_ಅಂತ್ಯ
