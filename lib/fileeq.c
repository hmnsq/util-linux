/*
 * compare file contents
 *
 * The goal is to minimize amount of data we need to read from the files and be
 * ready to compare large set of files, it means reuse the previous data if
 * possible. It never reads entire file if not necessary.
 *
 * The other goal is to minimize number of open files (imagine "hardlink /"),
 * the code can open only two files and reopen the file next time if
 * necessary.
 *
 * This code supports multiple comparison methods. The very basic step which is
 * generic for all methods is to read and compare an "intro" (a few bytes from
 * the beginning of the file). This intro buffer is always cached in 'struct
 * ul_fileeq_data', this intro buffer is addressed as block=0. This primitive
 * thing can reduce a lot ...
 *
 * The next steps depend on selected method:
 *
 *  * memcmp method: always read data to userspace, nothing is cached, directly
 *  compare file contents; fast for small sets of small files.
 *
 *  * Linux crypto API: zero-copy method based on sendfile(), data blocks are
 *  sent to the kernel hash functions (sha1, ...), and only hash digest is read
 *  and cached in userspace. Fast for large set of (large) files.
 *
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [October 2021]
 */

#अन्तर्भाव <inttypes.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <unistd.h>

/* Linux crypto */
#ifdef HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#endif

#अन्तर्भाव "c.h"
#अन्तर्भाव "all-io.h"
#अन्तर्भाव "fileeq.h"
#अन्तर्भाव "debug.h"

स्थिर UL_DEBUG_DEFINE_MASK(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULFILEEQ_DEBUG_INIT	(1 << 1)
#define ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#define ULFILEEQ_DEBUG_DATA	(1 << 3)
#define ULFILEEQ_DEBUG_EQ	(1 << 4)

#define DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

स्थिर शून्य ul_fileeq_init_debug(शून्य)
{
	जइ (ulfileeq_debug_mask)
		प्रत्यागम;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

गणना {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_CRC32
};

संरचना ul_fileeq_method {
	ध्रुव वर्ण *name;	/* name used by applications */
	ध्रुव वर्ण *kname;	/* name used by kernel crypto */
	पूर्णाङ्क id;
	short digsiz;
};

स्थिर ध्रुव संरचना ul_fileeq_method ul_eq_methods[] = {
	[UL_FILEEQ_MEMCMP] = {
		.id = UL_FILEEQ_MEMCMP, .name = "memcmp"
	},
#ifdef USE_FILEEQ_CRYPTOAPI
	[UL_FILEEQ_SHA1] = {
		.id = UL_FILEEQ_SHA1, .name = "sha1",
		.digsiz = 20, .kname = "sha1"
	},
	[UL_FILEEQ_SHA256] = {
		.id = UL_FILEEQ_SHA256, .name = "sha256",
		.digsiz = 32, .kname = "sha256"
	},

	[UL_FILEEQ_CRC32] = {
		.id = UL_FILEEQ_CRC32, .name = "crc32",
		.digsiz = 4, .kname = "crc32c"
	}
#endif
};

#ifdef USE_FILEEQ_CRYPTOAPI
स्थिर शून्य deinit_crypto_api(संरचना ul_fileeq *eq)
{
	जइ (!eq)
		प्रत्यागम;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	जइ (eq->fd_cip >= 0)
		close(eq->fd_cip);
	जइ (eq->fd_api >= 0)
		close(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

स्थिर पूर्णाङ्क init_crypto_api(संरचना ul_fileeq *eq)
{
	संरचना sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	प्रतिज्ञा(eq->method);
	प्रतिज्ञा(eq->method->kname);
	प्रतिज्ञा(eq->fd_api == -1);
	प्रतिज्ञा(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->method->kname));

	प्रतिज्ञा(परिमाण(sa.salg_name) > strlen(eq->method->kname) + 1);
	memcpy(&sa.salg_name, eq->method->kname, strlen(eq->method->kname) + 1);

	जइ ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		गच्छ fail;
	जइ (bind(eq->fd_api, (संरचना sockaddr *) &sa, परिमाण(sa)) != 0)
		गच्छ fail;
	जइ ((eq->fd_cip = accept(eq->fd_api, रिक्त, 0)) < 0)
		गच्छ fail;
	प्रत्यागम 0;
fail:
	deinit_crypto_api(eq);
	प्रत्यागम -1;
}
#endif

पूर्णाङ्क ul_fileeq_init(संरचना ul_fileeq *eq, ध्रुव वर्ण *method)
{
	size_t i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", method));

	memset(eq, 0, परिमाण(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	प्रति (i = 0; i < ARRAY_SIZE(ul_eq_methods); i++) {
		ध्रुव संरचना ul_fileeq_method *m = &ul_eq_methods[i];

		जइ (strcmp(m->name, method) == 0) {
			eq->method = m;
			भङ्ग;
		}
	}

	जइ (!eq->method)
		प्रत्यागम -1;
#ifdef USE_FILEEQ_CRYPTOAPI
	जइ (eq->method->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		प्रत्यागम -1;
#endif
	प्रत्यागम 0;
}

स्थिर शून्य reset_fileeq_bufs(संरचना ul_fileeq *eq)
{
	मुक्त(eq->buf_a);
	मुक्त(eq->buf_b);
	eq->buf_last = eq->buf_a = eq->buf_b = रिक्त;
}

शून्य ul_fileeq_deinit(संरचना ul_fileeq *eq)
{
	जइ (!eq)
		प्रत्यागम;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#ifdef USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#endif
	reset_fileeq_bufs(eq);
}

शून्य ul_fileeq_data_close_file(संरचना ul_fileeq_data *data)
{
	प्रतिज्ञा(data);

	जइ (data->fd >= 0) {
		DBG_OBJ(DATA, data, ul_debug("close"));
		close(data->fd);
	}
	data->fd = -1;
}

शून्य ul_fileeq_data_init(संरचना ul_fileeq_data *data)
{
	DBG_OBJ(DATA, data, ul_debug("init"));
	memset(data, 0, परिमाण(*data));
	data->fd = -1;
}

शून्य ul_fileeq_data_deinit(संरचना ul_fileeq_data *data)
{
	प्रतिज्ञा(data);

	DBG_OBJ(DATA, data, ul_debug("deinit"));
	मुक्त(data->blocks);
	data->blocks = रिक्त;
	data->nblocks = 0;
	data->maxblocks = 0;
	data->is_eof = 0;
	data->name = रिक्त;

	ul_fileeq_data_close_file(data);
}

पूर्णाङ्क ul_fileeq_data_associated(संरचना ul_fileeq_data *data)
{
	प्रत्यागम data->name != रिक्त;
}

शून्य ul_fileeq_data_set_file(संरचना ul_fileeq_data *data, ध्रुव वर्ण *name)
{
	प्रतिज्ञा(data);
	प्रतिज्ञा(name);

	DBG_OBJ(DATA, data, ul_debug("set file: %s", name));
	ul_fileeq_data_init(data);
	data->name = name;
}

bool ul_fileeq_set_size(संरचना ul_fileeq *eq, int64_t st_size,
			size_t readsiz, size_t memsiz)
{
	uint64_t filesiz, nreads, maxdigs;
	size_t digsiz;

	प्रतिज्ञा(eq);
	प्रतिज्ञा(st_size >= 0);
	प्रतिज्ञा(readsiz);

	filesiz = (uint64_t) st_size;

	eq->filesiz = filesiz;

	जइ (filesiz != 0 && readsiz > filesiz)
		readsiz = filesiz;

	विकल्प (eq->method->id) {
	पक्ष UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		भङ्ग;
	स्वभाव:
		digsiz = eq->method->digsiz;
		जइ (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		जइ (maxdigs == 0)
			maxdigs = 1;
		अण्णहा जइ (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		जइ (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			जइ (ceiling / maxdigs > SIZE_MAX)
				प्रत्यागम false;
			readsiz = ceiling / maxdigs;
		}
		भङ्ग;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	प्रत्यागम true;
}

स्थिर unsigned वर्ण *get_buffer(संरचना ul_fileeq *eq)
{
	जइ (!eq->buf_a)
		eq->buf_a = malloc(eq->readsiz);
	जइ (!eq->buf_b)
		eq->buf_b = malloc(eq->readsiz);

	जइ (!eq->buf_a || !eq->buf_b)
		प्रत्यागम रिक्त;

	जइ (eq->buf_last == eq->buf_b)
		eq->buf_last = eq->buf_a;
	अण्णहा
		eq->buf_last = eq->buf_b;

	प्रत्यागम eq->buf_last;
}

#define get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#define get_cached_offset(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				परिमाण((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


स्थिर पूर्णाङ्क get_fd(संरचना ul_fileeq *eq, संरचना ul_fileeq_data *data, off_t *off)
{
	off_t o = get_cached_offset(eq, data);

	प्रतिज्ञा(eq);
	प्रतिज्ञा(data);


	जइ (data->fd < 0) {
		DBG_OBJ(DATA, data, ul_debug("open: %s", data->name));
		data->fd = open(data->name, O_RDONLY | O_CLOEXEC);
		जइ (data->fd < 0)
			प्रत्यागम data->fd;

#जइ defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_result( posix_fadvise(data->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#endif
		जइ (o) {
			DBG_OBJ(DATA, data, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(data->fd, o, SEEK_SET);
		}
	}

	जइ (off)
		*off = o;

	प्रत्यागम data->fd;
}

स्थिर शून्य memcmp_reset(संरचना ul_fileeq *eq, संरचना ul_fileeq_data *data)
{
	/* only intro[] is cached */
	जइ (data->nblocks)
		data->nblocks = 1;
	/* reset file position */
	जइ (data->fd >= 0)
		lseek(data->fd, get_cached_offset(eq, data), SEEK_SET);
	data->is_eof = 0;
}

स्थिर ssize_t read_block(संरचना ul_fileeq *eq, संरचना ul_fileeq_data *data,
				uint64_t n, unsigned वर्ण **block)
{
	पूर्णाङ्क fd;
	off_t off = 0;
	ssize_t rsz;

	जइ (data->is_eof)
		प्रत्यागम 0;

	जइ (n >= eq->blocksmax)
		प्रत्यागम -EINVAL;

	fd = get_fd(eq, data, &off);
	जइ (fd < 0)
		प्रत्यागम fd;

	DBG_OBJ(DATA, data, ul_debug(" read block off=%ju", (uintmax_t) off));

	*block = get_buffer(eq);
	जइ (!*block)
		प्रत्यागम -ENOMEM;

	rsz = ul_read_all(data->fd, (वर्ण *) *block, eq->readsiz);
	जइ (rsz < 0) {
		DBG_OBJ(DATA, data, ul_debug("  read failed"));
		प्रत्यागम rsz;
	}
	off += rsz;
	data->nblocks++;

	जइ (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}

	DBG_OBJ(DATA, data, ul_debug("  read sz=%zd", rsz));
	प्रत्यागम rsz;
}

#ifdef USE_FILEEQ_CRYPTOAPI
स्थिर ssize_t get_digest(संरचना ul_fileeq *eq, संरचना ul_fileeq_data *data,
				uint64_t n, unsigned वर्ण **block)
{
	off_t off = 0;
	ssize_t rsz;
	size_t sz;
	पूर्णाङ्क fd;

	/* return already cached if available */
	जइ (n < get_cached_nblocks(data)) {
		DBG_OBJ(DATA, data, ul_debug(" digest cached"));
		प्रतिज्ञा(data->blocks);
		*block = data->blocks + (n * eq->method->digsiz);
		प्रत्यागम eq->method->digsiz;
	}

	जइ (data->is_eof) {
		DBG_OBJ(DATA, data, ul_debug(" file EOF"));
		प्रत्यागम 0;
	}

	जइ (n >= eq->blocksmax)
		प्रत्यागम -EINVAL;

	/* read new block */
	fd = get_fd(eq, data, &off);
	जइ (fd < 0)
		प्रत्यागम fd;

	DBG_OBJ(DATA, data, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->method->digsiz;

	जइ (!data->blocks) {
		DBG_OBJ(DATA, data, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		data->blocks = malloc(eq->blocksmax * sz);
		जइ (!data->blocks)
			प्रत्यागम -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, data->fd, रिक्त, eq->readsiz);
	DBG_OBJ(DATA, data, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	जइ (rsz < 0)
		प्रत्यागम rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*block = data->blocks + (n * eq->method->digsiz);
	rsz = ul_read_all(eq->fd_cip, (वर्ण *) *block, sz);

	जइ (rsz > 0)
		data->nblocks++;
	जइ (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}
	DBG_OBJ(DATA, data, ul_debug("  get %zdB digest", rsz));
	प्रत्यागम rsz;
}
#endif

स्थिर ssize_t get_intro(संरचना ul_fileeq *eq, संरचना ul_fileeq_data *data,
				unsigned वर्ण **block)
{
	जइ (data->nblocks == 0) {
		पूर्णाङ्क fd = get_fd(eq, data, रिक्त);
		ssize_t rsz;

		जइ (fd < 0)
			प्रत्यागम -1;
		rsz = ul_read_all(fd, (वर्ण *) data->intro, परिमाण(data->intro));
		DBG_OBJ(DATA, data, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, परिमाण(data->intro)));
		जइ (rsz < 0)
			प्रत्यागम -1;
		data->nblocks = 1;
	}

	DBG_OBJ(DATA, data, ul_debug(" return intro"));
	*block = data->intro;
	प्रत्यागम परिमाण(data->intro);
}

स्थिर ssize_t get_cmp_data(संरचना ul_fileeq *eq, संरचना ul_fileeq_data *data,
				uint64_t blockno, unsigned वर्ण **block)
{
	जइ (blockno == 0)
		प्रत्यागम get_intro(eq, data, block);

	blockno--;

	विकल्प (eq->method->id) {
	पक्ष UL_FILEEQ_MEMCMP:
		प्रत्यागम read_block(eq, data, blockno, block);
	स्वभाव:
		भङ्ग;
	}
#ifdef USE_FILEEQ_CRYPTOAPI
	प्रत्यागम get_digest(eq, data, blockno, block);
#अण्णहा
	प्रत्यागम -1;
#endif
}

#define CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

पूर्णाङ्क ul_fileeq(संरचना ul_fileeq *eq,
	      संरचना ul_fileeq_data *a, संरचना ul_fileeq_data *b)
{
	पूर्णाङ्क cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	जइ (eq->method->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	करेहि {
		unsigned वर्ण *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_data(eq, a, n, &da);
		जइ (ca < 0)
			गच्छ done;
		cb = get_cmp_data(eq, b, n, &db);
		जइ (cb < 0)
			गच्छ done;
		जइ (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			भङ्ग;

		}
		cmp = memcmp(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} जाव (cmp == 0);

	जइ (cmp == 0) {
		जइ (!a->is_eof || !b->is_eof)
			गच्छ done; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		प्रत्यागम 1;
	}
done:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	प्रत्यागम 0;
}

#ifdef TEST_PROGRAM_FILEEQ
# include <getopt.h>

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	संरचना ul_fileeq eq;
	संरचना ul_fileeq_data a, b, c;
	ध्रुव वर्ण *method = "sha1";
	स्थिर ध्रुव संरचना option longopts[] = {
		{ "method", required_argument, रिक्त, 'm' },
		{ "help", no_argument, रिक्त, 'h' },
		{ रिक्त, 0, रिक्त, 0 }
	};
	पूर्णाङ्क ch, rc;
	ध्रुव वर्ण *file_a = रिक्त, *file_b = रिक्त, *file_c = रिक्त;
	संरचना stat st_a, st_b, st_c;

	जाव ((ch = getopt_long(argc, argv, "m:", longopts, रिक्त)) != -1) {
		विकल्प (ch) {
		पक्ष 'm':
			method = optarg;
			भङ्ग;
		पक्ष 'h':
			printf("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				program_invocation_short_name);
			प्रत्यागम EXIT_FAILURE;
		}
	}

	जइ (optind < argc)
		file_a = argv[optind++];
	जइ (optind < argc)
		file_b = argv[optind++];
	जइ (optind < argc)
		file_c = argv[optind++];

	जइ (!file_a || !file_b)
		errx(EXIT_FAILURE, "no files specified, see --help");

	जइ (stat(file_a, &st_a) != 0 || !S_ISREG(st_a.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_a);
	जइ (stat(file_b, &st_b) != 0 || !S_ISREG(st_b.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_b);
	जइ (file_c && (stat(file_c, &st_c) != 0 || !S_ISREG(st_c.st_mode)))
		errx(EXIT_FAILURE, "%s: wrong file", file_c);


	जइ (st_a.st_size != st_b.st_size ||
	    (file_c && st_a.st_size != st_c.st_size))
		errx(EXIT_FAILURE, "size of the files does not match");


	rc = ul_fileeq_init(&eq, method);
	जइ (rc != 0 && strcmp(method, "memcmp") != 0) {
		method = "memcmp";
		rc = ul_fileeq_init(&eq, method);
	}
	जइ (rc < 0)
		err(EXIT_FAILURE, "failed to initialize files comparior");

	ul_fileeq_data_set_file(&a, file_a);
	ul_fileeq_data_set_file(&b, file_b);

	/* 3rd is optional */
	जइ (file_c)
		ul_fileeq_data_set_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	जइ (!ul_fileeq_set_size(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	printf("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	जइ (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		printf("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		printf("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_data_deinit(&a);
	ul_fileeq_data_deinit(&b);

	जइ (file_c)
		ul_fileeq_data_deinit(&c);

	ul_fileeq_deinit(&eq);
	प्रत्यागम EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_FILEEQ */
