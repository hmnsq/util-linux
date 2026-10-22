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

#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <sched.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <ctype.h>
#यदि_परिभाषितम् HAVE_SYS_SYSCALL_H
#अन्तर्भाव <sys/syscall.h>
#यदि_अन्तः

#अन्तर्भाव "cpuset.h"
#अन्तर्भाव "c.h"

स्थिर अन्तःस्थ पूर्णाङ्क val_गन्तव्यम्_char(पूर्णाङ्क v)
{
	यदि (v >= 0 && v < 10)
		प्रत्यागम '0' + v;
	यदि (v >= 10 && v < 16)
		प्रत्यागम ('a' - 10) + v;
	प्रत्यागम -1;
}

स्थिर अन्तःस्थ पूर्णाङ्क char_गन्तव्यम्_val(पूर्णाङ्क c)
{
	पूर्णाङ्क cl;

	यदि (c >= '0' && c <= '9')
		प्रत्यागम c - '0';
	cl = tolower(c);
	यदि (cl >= 'a' && cl <= 'f')
		प्रत्यागम cl + (10 - 'a');
	प्रत्यागम -1;
}

स्थिर ध्रुव वर्ण *nexttoken(ध्रुव वर्ण *q,  पूर्णाङ्क sep)
{
	यदि (q)
		q = strchr(q, sep);
	यदि (q)
		q++;
	प्रत्यागम q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
पूर्णाङ्क get_max_सङ्ख्या_सम्बन्धि_cpus(शून्य)
{
#यदि_परिभाषितम् SYS_sched_getaffinity
	पूर्णाङ्क n, cpus = 2048;
	size_t setsize;
	cpu_set_t *समुच्चयः = cpuset_alloc(cpus, &setsize, रिक्त);

	यदि (!समुच्चयः)
		प्रत्यागम -1;	/* error */

	प्रति (;;) {
		CPU_ZERO_S(setsize, समुच्चयः);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, समुच्चयः);

		यदि (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(समुच्चयः);
			cpus *= 2;
			समुच्चयः = cpuset_alloc(cpus, &setsize, रिक्त);
			यदि (!समुच्चयः)
				प्रत्यागम -1;	/* error */
			अनुवर्त;
		}
		cpuset_free(समुच्चयः);
		प्रत्यागम n * 8;
	}
#यदि_अन्तः
	प्रत्यागम -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(पूर्णाङ्क ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *समुच्चयः = रिक्त;

	यदि (ncpus)
		समुच्चयः = CPU_ALLOC(ncpus);
	यदि (!समुच्चयः)
		प्रत्यागम रिक्त;
	यदि (setsize)
		*setsize = CPU_ALLOC_आकार(ncpus);
	यदि (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_आकार(ncpus));
	प्रत्यागम समुच्चयः;
}

शून्य cpuset_free(cpu_set_t *समुच्चयः)
{
	CPU_FREE(समुच्चयः);
}

#यदि !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
पूर्णाङ्क __cpuset_count_s(size_t setsize, ध्रुव cpu_set_t *समुच्चयः)
{
	पूर्णाङ्क s = 0;
	ध्रुव __cpu_mask *p = समुच्चयः->__bits;
	ध्रुव __cpu_mask *अन्तः = &समुच्चयः->__bits[setsize / परिमाण (__cpu_mask)];

	यावत् (p < अन्तः) {
		__cpu_mask l = *p++;

		यदि (l == 0)
			अनुवर्त;
# यदि LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# अन्यथा
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	प्रत्यागम s;
}
#यदि_अन्तः

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
स्थिर size_t find_अग्र_cpu(size_t start, size_t setsize, cpu_set_t *समुच्चयः)
{
	size_t nbits = cpuset_nbits(setsize);
	प्रति (; start < nbits; start++)
		यदि (CPU_ISSET_S(start, setsize, समुच्चयः))
			प्रत्यागम start;
	प्रत्यागम start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
वर्ण *cpulist_create(वर्ण *str, size_t len,
			cpu_set_t *समुच्चयः, size_t setsize)
{
	वर्ण *ptr = str;
	पूर्णाङ्क प्रविष्टिः_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t अग्र = 0;  /* where to start looking for next cpu */

	यावत् ((a = find_अग्र_cpu(अग्र, setsize, समुच्चयः)) < max) {
		पूर्णाङ्क rlen;
		अग्र = find_अग्र_cpu(a + 1, setsize, समुच्चयः);
		यदि (अग्र == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} अन्यथा {
			/* Extend range as long as we have the same stride. */
			size_t b = अग्र;
			size_t s = b - a;
			यावत् (((अग्र = find_अग्र_cpu(b + 1, setsize, समुच्चयः)) <
				max) && अग्र - b == s) {
				b = अग्र;
			}
			यदि (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				अग्र = b;
			} अन्यथा यदि (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} अन्यथा {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		यदि (rlen < 0 || (size_t) rlen >= len)
			प्रत्यागम रिक्त;
		ptr += rlen;
		len -= rlen;
		प्रविष्टिः_made = 1;
	}
	ptr -= प्रविष्टिः_made;
	*ptr = '\0';

	प्रत्यागम str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
वर्ण *cpulist_flat_create(वर्ण *str, size_t len,
			    cpu_set_t *समुच्चयः, size_t setsize)
{
	वर्ण *ptr = str;
	पूर्णाङ्क प्रविष्टिः_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	पूर्णाङ्क rlen;

	प्रति (cpu = 0; cpu < max; cpu++) {
		यदि (!CPU_ISSET_S(cpu, setsize, समुच्चयः))
			अनुवर्त;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		यदि (rlen < 0 || (size_t)rlen >= len)
			प्रत्यागम रिक्त;

		ptr += rlen;
		len -= rlen;
		प्रविष्टिः_made = 1;
	}
	ptr -= प्रविष्टिः_made;
	*ptr = '\0';

	प्रत्यागम str;
}


/*
 * Returns string with CPU mask.
 */
वर्ण *cpumask_create(वर्ण *str, size_t len,
			cpu_set_t *समुच्चयः, size_t setsize)
{
	वर्ण *ptr = str;
	वर्ण *ret = रिक्त;
	पूर्णाङ्क cpu;

	प्रति (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		वर्ण val = 0;

		यदि (len == (size_t) (ptr - str))
			भङ्ग;

		यदि (CPU_ISSET_S(cpu, setsize, समुच्चयः))
			val |= 1;
		यदि (CPU_ISSET_S(cpu + 1, setsize, समुच्चयः))
			val |= 2;
		यदि (CPU_ISSET_S(cpu + 2, setsize, समुच्चयः))
			val |= 4;
		यदि (CPU_ISSET_S(cpu + 3, setsize, समुच्चयः))
			val |= 8;

		यदि (!ret && val)
			ret = ptr;
		*ptr++ = val_गन्तव्यम्_char(val);
	}
	*ptr = '\0';
	प्रत्यागम ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
पूर्णाङ्क cpumask_parse(ध्रुव वर्ण *str, cpu_set_t *समुच्चयः, size_t setsize)
{
	पूर्णाङ्क len = वाक्यदैर्घ्यम्(str);
	ध्रुव वर्ण *ptr = str + len - 1;
	पूर्णाङ्क cpu = 0;

	/* skip 0x, it's all hex anyway */
	यदि (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, समुच्चयः);

	यावत् (ptr >= str) {
		वर्ण val;

		/* cpu masks in /sys uses comma as a separator */
		यदि (*ptr == ',')
			ptr--;

		val = char_गन्तव्यम्_val(*ptr);
		यदि (val == (वर्ण) -1)
			प्रत्यागम -1;
		यदि (val & 1)
			CPU_समुच्चयः_S(cpu, setsize, समुच्चयः);
		यदि (val & 2)
			CPU_समुच्चयः_S(cpu + 1, setsize, समुच्चयः);
		यदि (val & 4)
			CPU_समुच्चयः_S(cpu + 2, setsize, समुच्चयः);
		यदि (val & 8)
			CPU_समुच्चयः_S(cpu + 3, setsize, समुच्चयः);
		ptr--;
		cpu += 4;
	}

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क nextnumber(ध्रुव वर्ण *str, वर्ण **अन्तः, अचिह्नित पूर्णाङ्क *परिणाम)
{
	errno = 0;
	यदि (str == रिक्त || *str == '\0' || !अङ्कः_वा(*str))
		प्रत्यागम -EINVAL;
	*परिणाम = (अचिह्नित पूर्णाङ्क) strtoul(str, अन्तः, 10);
	यदि (errno)
		प्रत्यागम -errno;
	यदि (str == *अन्तः)
		प्रत्यागम -EINVAL;
	प्रत्यागम 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
पूर्णाङ्क cpulist_parse(ध्रुव वर्ण *str, cpu_set_t *समुच्चयः, size_t setsize, पूर्णाङ्क fail)
{
	ध्रुव size_t max = cpuset_nbits(setsize);
	ध्रुव वर्ण *p, *q;
	वर्ण *अन्तः = रिक्त;

	q = str;
	CPU_ZERO_S(setsize, समुच्चयः);

	यावत् (p = q, q = nexttoken(q, ','), p) {
		अचिह्नित पूर्णाङ्क a;	/* beginning of range */
		अचिह्नित पूर्णाङ्क b;	/* end of range */
		अचिह्नित पूर्णाङ्क s;	/* stride */
		ध्रुव वर्ण *c1, *c2;

		यदि (nextnumber(p, &अन्तः, &a) != 0 ||
			(*अन्तः && *अन्तः != ',' && *अन्तः != '-'))
			प्रत्यागम 1;
		b = a;
		s = 1;
		p = अन्तः;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		यदि (c1 != रिक्त && (c2 == रिक्त || c1 < c2)) {
			यदि (nextnumber(c1, &अन्तः, &b) != 0 ||
				(*अन्तः && *अन्तः != ',' && *अन्तः != ':'))
				प्रत्यागम 1;

			c1 = nexttoken(अन्तः, ':');

			यदि (c1 != रिक्त && (c2 == रिक्त || c1 < c2)) {
				यदि (nextnumber(c1, &अन्तः, &s) != 0 ||
					(*अन्तः && *अन्तः != ','))
					प्रत्यागम 1;
				यदि (s == 0)
					प्रत्यागम 1;
			}
		}

		यदि (!(a <= b))
			प्रत्यागम 1;
		यावत् (a <= b) {
			यदि (a >= max) {
				यदि (fail)
					प्रत्यागम 2;
				अन्यथा
					भङ्ग;
			}
			CPU_समुच्चयः_S(a, setsize, समुच्चयः);
			a += s;
		}
	}

	यदि (अन्तः && *अन्तः)
		प्रत्यागम 1;
	प्रत्यागम 0;
}

#यदि_परिभाषितम् TEST_क्रमादेशः_CPUSET

#अन्तर्भाव <getopt.h>

पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण *argv[])
{
	cpu_set_t *समुच्चयः;
	size_t setsize, buflen, nbits;
	वर्ण *buf, *आवरणम् = रिक्त, *range = रिक्त;
	पूर्णाङ्क ncpus = 2048, rc, c;

	स्थिर ध्रुव संरचना option longopts[] = {
	    { "ncpus", 1, रिक्त, 'n' },
	    { "mask",  1, रिक्त, 'm' },
	    { "range", 1, रिक्त, 'r' },
	    { रिक्त,    0, रिक्त, 0 }
	};

	यावत् ((c = getopt_long(argc, argv, "n:m:r:", longopts, रिक्त)) != -1) {
		विकल्प(c) {
		पक्ष 'n':
			ncpus = atoi(optarg);
			भङ्ग;
		पक्ष 'm':
			आवरणम् = optarg;
			भङ्ग;
		पक्ष 'r':
			range = optarg;
			भङ्ग;
		स्वभाव:
			गच्छ usage_err;
		}
	}

	यदि (!आवरणम् && !range)
		गच्छ usage_err;

	समुच्चयः = cpuset_alloc(ncpus, &setsize, &nbits);
	यदि (!समुच्चयः)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = स्मृतिं_देहि(buflen);
	यदि (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	यदि (आवरणम्)
		rc = cpumask_parse(आवरणम्, समुच्चयः, setsize);
	अन्यथा
		rc = cpulist_parse(range, समुच्चयः, setsize, 0);

	यदि (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", आवरणम् ? : range);

	प्रारूप_लिख("%-15s = %15s ", आवरणम् ? : range,
				cpumask_create(buf, buflen, समुच्चयः, setsize));
	प्रारूप_लिख("[%s]\n", cpulist_create(buf, buflen, समुच्चयः, setsize));

	मुक्त(buf);
	cpuset_free(समुच्चयः);

	प्रत्यागम EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		क्रमादेशः_invocation_short_name);
	निर्गच्छ(EXIT_FAILURE);
}
#यदि_अन्तः
