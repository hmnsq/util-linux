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

#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <sched.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <ctype.h>
#నిర్వచితమైతే HAVE_SYS_SYSCALL_H
#అన్తర్భావ <sys/syscall.h>
#అయితే_ముగింపు

#అన్తర్భావ "cpuset.h"
#అన్తర్భావ "c.h"

స్థిర అంతఃవరుస పూర్ణాంకం val_గమ్యం_char(పూర్ణాంకం v)
{
	ఒకవేళ (v >= 0 && v < 10)
		తిరిగి '0' + v;
	ఒకవేళ (v >= 10 && v < 16)
		తిరిగి ('a' - 10) + v;
	తిరిగి -1;
}

స్థిర అంతఃవరుస పూర్ణాంకం char_గమ్యం_val(పూర్ణాంకం c)
{
	పూర్ణాంకం cl;

	ఒకవేళ (c >= '0' && c <= '9')
		తిరిగి c - '0';
	cl = tolower(c);
	ఒకవేళ (cl >= 'a' && cl <= 'f')
		తిరిగి cl + (10 - 'a');
	తిరిగి -1;
}

స్థిర స్థిరరాశి అక్షరం *nexttoken(స్థిరరాశి అక్షరం *q,  పూర్ణాంకం sep)
{
	ఒకవేళ (q)
		q = strchr(q, sep);
	ఒకవేళ (q)
		q++;
	తిరిగి q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
పూర్ణాంకం get_max_సంఖ్య_యొక్క_cpus(శూన్య)
{
#నిర్వచితమైతే SYS_sched_getaffinity
	పూర్ణాంకం n, cpus = 2048;
	size_t setsize;
	cpu_set_t *సమితి = cpuset_alloc(cpus, &setsize, ఖాళీ);

	ఒకవేళ (!సమితి)
		తిరిగి -1;	/* error */

	కొరకు (;;) {
		CPU_ZERO_S(setsize, సమితి);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, సమితి);

		ఒకవేళ (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(సమితి);
			cpus *= 2;
			సమితి = cpuset_alloc(cpus, &setsize, ఖాళీ);
			ఒకవేళ (!సమితి)
				తిరిగి -1;	/* error */
			కొనసాగు;
		}
		cpuset_free(సమితి);
		తిరిగి n * 8;
	}
#అయితే_ముగింపు
	తిరిగి -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(పూర్ణాంకం ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *సమితి = ఖాళీ;

	ఒకవేళ (ncpus)
		సమితి = CPU_ALLOC(ncpus);
	ఒకవేళ (!సమితి)
		తిరిగి ఖాళీ;
	ఒకవేళ (setsize)
		*setsize = CPU_ALLOC_కొలత(ncpus);
	ఒకవేళ (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_కొలత(ncpus));
	తిరిగి సమితి;
}

శూన్య cpuset_free(cpu_set_t *సమితి)
{
	CPU_FREE(సమితి);
}

#ఒకవేళ !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
పూర్ణాంకం __cpuset_count_s(size_t setsize, స్థిరరాశి cpu_set_t *సమితి)
{
	పూర్ణాంకం s = 0;
	స్థిరరాశి __cpu_mask *p = సమితి->__bits;
	స్థిరరాశి __cpu_mask *చివర = &సమితి->__bits[setsize / పరిమాణం (__cpu_mask)];

	అయితే (p < చివర) {
		__cpu_mask l = *p++;

		ఒకవేళ (l == 0)
			కొనసాగు;
# ఒకవేళ LONG_బిట్టు > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# లేకపోతే
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	తిరిగి s;
}
#అయితే_ముగింపు

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
స్థిర size_t కనుగొను_తదుపరి_cpu(size_t ప్రారంభం, size_t setsize, cpu_set_t *సమితి)
{
	size_t nbits = cpuset_nbits(setsize);
	కొరకు (; ప్రారంభం < nbits; ప్రారంభం++)
		ఒకవేళ (CPU_ISSET_S(ప్రారంభం, setsize, సమితి))
			తిరిగి ప్రారంభం;
	తిరిగి ప్రారంభం;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
అక్షరం *cpulist_create(అక్షరం *str, size_t len,
			cpu_set_t *సమితి, size_t setsize)
{
	అక్షరం *ptr = str;
	పూర్ణాంకం నమోదు_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t తదుపరి = 0;  /* where to start looking for next cpu */

	అయితే ((a = కనుగొను_తదుపరి_cpu(తదుపరి, setsize, సమితి)) < max) {
		పూర్ణాంకం rlen;
		తదుపరి = కనుగొను_తదుపరి_cpu(a + 1, setsize, సమితి);
		ఒకవేళ (తదుపరి == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} లేకపోతే {
			/* Extend range as long as we have the same stride. */
			size_t b = తదుపరి;
			size_t s = b - a;
			అయితే (((తదుపరి = కనుగొను_తదుపరి_cpu(b + 1, setsize, సమితి)) <
				max) && తదుపరి - b == s) {
				b = తదుపరి;
			}
			ఒకవేళ (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				తదుపరి = b;
			} లేకపోతే ఒకవేళ (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} లేకపోతే {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		ఒకవేళ (rlen < 0 || (size_t) rlen >= len)
			తిరిగి ఖాళీ;
		ptr += rlen;
		len -= rlen;
		నమోదు_made = 1;
	}
	ptr -= నమోదు_made;
	*ptr = '\0';

	తిరిగి str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
అక్షరం *cpulist_flat_create(అక్షరం *str, size_t len,
			    cpu_set_t *సమితి, size_t setsize)
{
	అక్షరం *ptr = str;
	పూర్ణాంకం నమోదు_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	పూర్ణాంకం rlen;

	కొరకు (cpu = 0; cpu < max; cpu++) {
		ఒకవేళ (!CPU_ISSET_S(cpu, setsize, సమితి))
			కొనసాగు;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		ఒకవేళ (rlen < 0 || (size_t)rlen >= len)
			తిరిగి ఖాళీ;

		ptr += rlen;
		len -= rlen;
		నమోదు_made = 1;
	}
	ptr -= నమోదు_made;
	*ptr = '\0';

	తిరిగి str;
}


/*
 * Returns string with CPU mask.
 */
అక్షరం *cpumask_create(అక్షరం *str, size_t len,
			cpu_set_t *సమితి, size_t setsize)
{
	అక్షరం *ptr = str;
	అక్షరం *ret = ఖాళీ;
	పూర్ణాంకం cpu;

	కొరకు (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		అక్షరం val = 0;

		ఒకవేళ (len == (size_t) (ptr - str))
			ఆపు;

		ఒకవేళ (CPU_ISSET_S(cpu, setsize, సమితి))
			val |= 1;
		ఒకవేళ (CPU_ISSET_S(cpu + 1, setsize, సమితి))
			val |= 2;
		ఒకవేళ (CPU_ISSET_S(cpu + 2, setsize, సమితి))
			val |= 4;
		ఒకవేళ (CPU_ISSET_S(cpu + 3, setsize, సమితి))
			val |= 8;

		ఒకవేళ (!ret && val)
			ret = ptr;
		*ptr++ = val_గమ్యం_char(val);
	}
	*ptr = '\0';
	తిరిగి ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
పూర్ణాంకం cpumask_parse(స్థిరరాశి అక్షరం *str, cpu_set_t *సమితి, size_t setsize)
{
	పూర్ణాంకం len = అక్షరశ్రేణి_పొడవు(str);
	స్థిరరాశి అక్షరం *ptr = str + len - 1;
	పూర్ణాంకం cpu = 0;

	/* skip 0x, it's all hex anyway */
	ఒకవేళ (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, సమితి);

	అయితే (ptr >= str) {
		అక్షరం val;

		/* cpu masks in /sys uses comma as a separator */
		ఒకవేళ (*ptr == ',')
			ptr--;

		val = char_గమ్యం_val(*ptr);
		ఒకవేళ (val == (అక్షరం) -1)
			తిరిగి -1;
		ఒకవేళ (val & 1)
			CPU_సమితి_S(cpu, setsize, సమితి);
		ఒకవేళ (val & 2)
			CPU_సమితి_S(cpu + 1, setsize, సమితి);
		ఒకవేళ (val & 4)
			CPU_సమితి_S(cpu + 2, setsize, సమితి);
		ఒకవేళ (val & 8)
			CPU_సమితి_S(cpu + 3, setsize, సమితి);
		ptr--;
		cpu += 4;
	}

	తిరిగి 0;
}

స్థిర పూర్ణాంకం nextnumber(స్థిరరాశి అక్షరం *str, అక్షరం **చివర, గుర్తులేని పూర్ణాంకం *ఫలితం)
{
	errno = 0;
	ఒకవేళ (str == ఖాళీ || *str == '\0' || !అంకెనా(*str))
		తిరిగి -EINVAL;
	*ఫలితం = (గుర్తులేని పూర్ణాంకం) strtoul(str, చివర, 10);
	ఒకవేళ (errno)
		తిరిగి -errno;
	ఒకవేళ (str == *చివర)
		తిరిగి -EINVAL;
	తిరిగి 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
పూర్ణాంకం cpulist_parse(స్థిరరాశి అక్షరం *str, cpu_set_t *సమితి, size_t setsize, పూర్ణాంకం fail)
{
	స్థిరరాశి size_t max = cpuset_nbits(setsize);
	స్థిరరాశి అక్షరం *p, *q;
	అక్షరం *చివర = ఖాళీ;

	q = str;
	CPU_ZERO_S(setsize, సమితి);

	అయితే (p = q, q = nexttoken(q, ','), p) {
		గుర్తులేని పూర్ణాంకం a;	/* beginning of range */
		గుర్తులేని పూర్ణాంకం b;	/* end of range */
		గుర్తులేని పూర్ణాంకం s;	/* stride */
		స్థిరరాశి అక్షరం *c1, *c2;

		ఒకవేళ (nextnumber(p, &చివర, &a) != 0 ||
			(*చివర && *చివర != ',' && *చివర != '-'))
			తిరిగి 1;
		b = a;
		s = 1;
		p = చివర;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		ఒకవేళ (c1 != ఖాళీ && (c2 == ఖాళీ || c1 < c2)) {
			ఒకవేళ (nextnumber(c1, &చివర, &b) != 0 ||
				(*చివర && *చివర != ',' && *చివర != ':'))
				తిరిగి 1;

			c1 = nexttoken(చివర, ':');

			ఒకవేళ (c1 != ఖాళీ && (c2 == ఖాళీ || c1 < c2)) {
				ఒకవేళ (nextnumber(c1, &చివర, &s) != 0 ||
					(*చివర && *చివర != ','))
					తిరిగి 1;
				ఒకవేళ (s == 0)
					తిరిగి 1;
			}
		}

		ఒకవేళ (!(a <= b))
			తిరిగి 1;
		అయితే (a <= b) {
			ఒకవేళ (a >= max) {
				ఒకవేళ (fail)
					తిరిగి 2;
				లేకపోతే
					ఆపు;
			}
			CPU_సమితి_S(a, setsize, సమితి);
			a += s;
		}
	}

	ఒకవేళ (చివర && *చివర)
		తిరిగి 1;
	తిరిగి 0;
}

#నిర్వచితమైతే TEST_కార్యక్రమం_CPUSET

#అన్తర్భావ <getopt.h>

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
	cpu_set_t *సమితి;
	size_t setsize, buflen, nbits;
	అక్షరం *buf, *ముసుగు = ఖాళీ, *range = ఖాళీ;
	పూర్ణాంకం ncpus = 2048, rc, c;

	స్థిర స్థిరరాశి నిర్మాణం option longopts[] = {
	    { "ncpus", 1, ఖాళీ, 'n' },
	    { "mask",  1, ఖాళీ, 'm' },
	    { "range", 1, ఖాళీ, 'r' },
	    { ఖాళీ,    0, ఖాళీ, 0 }
	};

	అయితే ((c = getopt_long(argc, argv, "n:m:r:", longopts, ఖాళీ)) != -1) {
		ఎంపిక(c) {
		స్థితి 'n':
			ncpus = atoi(optarg);
			ఆపు;
		స్థితి 'm':
			ముసుగు = optarg;
			ఆపు;
		స్థితి 'r':
			range = optarg;
			ఆపు;
		అప్రమేయ:
			వెళ్ళు usage_err;
		}
	}

	ఒకవేళ (!ముసుగు && !range)
		వెళ్ళు usage_err;

	సమితి = cpuset_alloc(ncpus, &setsize, &nbits);
	ఒకవేళ (!సమితి)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = జ్ఞాపకం_ఇవ్వు(buflen);
	ఒకవేళ (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	ఒకవేళ (ముసుగు)
		rc = cpumask_parse(ముసుగు, సమితి, setsize);
	లేకపోతే
		rc = cpulist_parse(range, సమితి, setsize, 0);

	ఒకవేళ (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", ముసుగు ? : range);

	రాయి("%-15s = %15s ", ముసుగు ? : range,
				cpumask_create(buf, buflen, సమితి, setsize));
	రాయి("[%s]\n", cpulist_create(buf, buflen, సమితి, setsize));

	విడుదల(buf);
	cpuset_free(సమితి);

	తిరిగి EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		కార్యక్రమం_invocation_short_name);
	నిష్క్రమించు(EXIT_FAILURE);
}
#అయితే_ముగింపు
