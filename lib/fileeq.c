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

#ضمن <inttypes.h>
#ضمن <stdio.h>
#ضمن <sys/types.h>
#ضمن <sys/stat.h>
#ضمن <unistd.h>

/* Linux crypto */
#ifdef HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#endif

#ضمن "c.h"
#ضمن "all-io.h"
#ضمن "fileeq.h"
#ضمن "debug.h"

ساكن UL_DEBUG_DEFINE_MASK(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULFILEEQ_DEBUG_INIT	(1 << 1)
#define ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#define ULFILEEQ_DEBUG_DATA	(1 << 3)
#define ULFILEEQ_DEBUG_EQ	(1 << 4)

#define DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

ساكن فراغ ul_fileeq_init_debug(فراغ)
{
	ܐܢ (ulfileeq_debug_mask)
		ܦܢܐ;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

تعداد {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_CRC32
};

بنية ul_fileeq_method {
	ثابت حرف *name;	/* name used by applications */
	ثابت حرف *kname;	/* name used by kernel crypto */
	صحيح id;
	short digsiz;
};

ساكن ثابت بنية ul_fileeq_method ul_eq_methods[] = {
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
ساكن فراغ deinit_crypto_api(بنية ul_fileeq *eq)
{
	ܐܢ (!eq)
		ܦܢܐ;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	ܐܢ (eq->fd_cip >= 0)
		close(eq->fd_cip);
	ܐܢ (eq->fd_api >= 0)
		close(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

ساكن صحيح init_crypto_api(بنية ul_fileeq *eq)
{
	بنية sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	أكد(eq->method);
	أكد(eq->method->kname);
	أكد(eq->fd_api == -1);
	أكد(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->method->kname));

	أكد(حجم(sa.salg_name) > strlen(eq->method->kname) + 1);
	memcpy(&sa.salg_name, eq->method->kname, strlen(eq->method->kname) + 1);

	ܐܢ ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		اذهب fail;
	ܐܢ (bind(eq->fd_api, (بنية sockaddr *) &sa, حجم(sa)) != 0)
		اذهب fail;
	ܐܢ ((eq->fd_cip = accept(eq->fd_api, عدم, 0)) < 0)
		اذهب fail;
	ܦܢܐ 0;
fail:
	deinit_crypto_api(eq);
	ܦܢܐ -1;
}
#endif

صحيح ul_fileeq_init(بنية ul_fileeq *eq, ثابت حرف *method)
{
	size_t i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", method));

	memset(eq, 0, حجم(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	لكل (i = 0; i < ARRAY_SIZE(ul_eq_methods); i++) {
		ثابت بنية ul_fileeq_method *m = &ul_eq_methods[i];

		ܐܢ (strcmp(m->name, method) == 0) {
			eq->method = m;
			اكسر;
		}
	}

	ܐܢ (!eq->method)
		ܦܢܐ -1;
#ifdef USE_FILEEQ_CRYPTOAPI
	ܐܢ (eq->method->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		ܦܢܐ -1;
#endif
	ܦܢܐ 0;
}

ساكن فراغ reset_fileeq_bufs(بنية ul_fileeq *eq)
{
	حرر(eq->buf_a);
	حرر(eq->buf_b);
	eq->buf_last = eq->buf_a = eq->buf_b = عدم;
}

فراغ ul_fileeq_deinit(بنية ul_fileeq *eq)
{
	ܐܢ (!eq)
		ܦܢܐ;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#ifdef USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#endif
	reset_fileeq_bufs(eq);
}

فراغ ul_fileeq_data_close_file(بنية ul_fileeq_data *data)
{
	أكد(data);

	ܐܢ (data->fd >= 0) {
		DBG_OBJ(DATA, data, ul_debug("close"));
		close(data->fd);
	}
	data->fd = -1;
}

فراغ ul_fileeq_data_init(بنية ul_fileeq_data *data)
{
	DBG_OBJ(DATA, data, ul_debug("init"));
	memset(data, 0, حجم(*data));
	data->fd = -1;
}

فراغ ul_fileeq_data_deinit(بنية ul_fileeq_data *data)
{
	أكد(data);

	DBG_OBJ(DATA, data, ul_debug("deinit"));
	حرر(data->blocks);
	data->blocks = عدم;
	data->nblocks = 0;
	data->maxblocks = 0;
	data->is_eof = 0;
	data->name = عدم;

	ul_fileeq_data_close_file(data);
}

صحيح ul_fileeq_data_associated(بنية ul_fileeq_data *data)
{
	ܦܢܐ data->name != عدم;
}

فراغ ul_fileeq_data_set_file(بنية ul_fileeq_data *data, ثابت حرف *name)
{
	أكد(data);
	أكد(name);

	DBG_OBJ(DATA, data, ul_debug("set file: %s", name));
	ul_fileeq_data_init(data);
	data->name = name;
}

bool ul_fileeq_set_size(بنية ul_fileeq *eq, int64_t st_size,
			size_t readsiz, size_t memsiz)
{
	uint64_t filesiz, nreads, maxdigs;
	size_t digsiz;

	أكد(eq);
	أكد(st_size >= 0);
	أكد(readsiz);

	filesiz = (uint64_t) st_size;

	eq->filesiz = filesiz;

	ܐܢ (filesiz != 0 && readsiz > filesiz)
		readsiz = filesiz;

	بدل (eq->method->id) {
	حالة UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		اكسر;
	افتراضي:
		digsiz = eq->method->digsiz;
		ܐܢ (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		ܐܢ (maxdigs == 0)
			maxdigs = 1;
		ܐܚܪܢܐ ܐܢ (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		ܐܢ (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			ܐܢ (ceiling / maxdigs > SIZE_MAX)
				ܦܢܐ false;
			readsiz = ceiling / maxdigs;
		}
		اكسر;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	ܦܢܐ true;
}

ساكن unsigned حرف *get_buffer(بنية ul_fileeq *eq)
{
	ܐܢ (!eq->buf_a)
		eq->buf_a = malloc(eq->readsiz);
	ܐܢ (!eq->buf_b)
		eq->buf_b = malloc(eq->readsiz);

	ܐܢ (!eq->buf_a || !eq->buf_b)
		ܦܢܐ عدم;

	ܐܢ (eq->buf_last == eq->buf_b)
		eq->buf_last = eq->buf_a;
	ܐܚܪܢܐ
		eq->buf_last = eq->buf_b;

	ܦܢܐ eq->buf_last;
}

#define get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#define get_cached_offset(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				حجم((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


ساكن صحيح get_fd(بنية ul_fileeq *eq, بنية ul_fileeq_data *data, off_t *off)
{
	off_t o = get_cached_offset(eq, data);

	أكد(eq);
	أكد(data);


	ܐܢ (data->fd < 0) {
		DBG_OBJ(DATA, data, ul_debug("open: %s", data->name));
		data->fd = open(data->name, O_RDONLY | O_CLOEXEC);
		ܐܢ (data->fd < 0)
			ܦܢܐ data->fd;

#ܐܢ defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_result( posix_fadvise(data->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#endif
		ܐܢ (o) {
			DBG_OBJ(DATA, data, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(data->fd, o, SEEK_SET);
		}
	}

	ܐܢ (off)
		*off = o;

	ܦܢܐ data->fd;
}

ساكن فراغ memcmp_reset(بنية ul_fileeq *eq, بنية ul_fileeq_data *data)
{
	/* only intro[] is cached */
	ܐܢ (data->nblocks)
		data->nblocks = 1;
	/* reset file position */
	ܐܢ (data->fd >= 0)
		lseek(data->fd, get_cached_offset(eq, data), SEEK_SET);
	data->is_eof = 0;
}

ساكن ssize_t read_block(بنية ul_fileeq *eq, بنية ul_fileeq_data *data,
				uint64_t n, unsigned حرف **block)
{
	صحيح fd;
	off_t off = 0;
	ssize_t rsz;

	ܐܢ (data->is_eof)
		ܦܢܐ 0;

	ܐܢ (n >= eq->blocksmax)
		ܦܢܐ -EINVAL;

	fd = get_fd(eq, data, &off);
	ܐܢ (fd < 0)
		ܦܢܐ fd;

	DBG_OBJ(DATA, data, ul_debug(" read block off=%ju", (uintmax_t) off));

	*block = get_buffer(eq);
	ܐܢ (!*block)
		ܦܢܐ -ENOMEM;

	rsz = ul_read_all(data->fd, (حرف *) *block, eq->readsiz);
	ܐܢ (rsz < 0) {
		DBG_OBJ(DATA, data, ul_debug("  read failed"));
		ܦܢܐ rsz;
	}
	off += rsz;
	data->nblocks++;

	ܐܢ (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}

	DBG_OBJ(DATA, data, ul_debug("  read sz=%zd", rsz));
	ܦܢܐ rsz;
}

#ifdef USE_FILEEQ_CRYPTOAPI
ساكن ssize_t get_digest(بنية ul_fileeq *eq, بنية ul_fileeq_data *data,
				uint64_t n, unsigned حرف **block)
{
	off_t off = 0;
	ssize_t rsz;
	size_t sz;
	صحيح fd;

	/* return already cached if available */
	ܐܢ (n < get_cached_nblocks(data)) {
		DBG_OBJ(DATA, data, ul_debug(" digest cached"));
		أكد(data->blocks);
		*block = data->blocks + (n * eq->method->digsiz);
		ܦܢܐ eq->method->digsiz;
	}

	ܐܢ (data->is_eof) {
		DBG_OBJ(DATA, data, ul_debug(" file EOF"));
		ܦܢܐ 0;
	}

	ܐܢ (n >= eq->blocksmax)
		ܦܢܐ -EINVAL;

	/* read new block */
	fd = get_fd(eq, data, &off);
	ܐܢ (fd < 0)
		ܦܢܐ fd;

	DBG_OBJ(DATA, data, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->method->digsiz;

	ܐܢ (!data->blocks) {
		DBG_OBJ(DATA, data, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		data->blocks = malloc(eq->blocksmax * sz);
		ܐܢ (!data->blocks)
			ܦܢܐ -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, data->fd, عدم, eq->readsiz);
	DBG_OBJ(DATA, data, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	ܐܢ (rsz < 0)
		ܦܢܐ rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*block = data->blocks + (n * eq->method->digsiz);
	rsz = ul_read_all(eq->fd_cip, (حرف *) *block, sz);

	ܐܢ (rsz > 0)
		data->nblocks++;
	ܐܢ (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}
	DBG_OBJ(DATA, data, ul_debug("  get %zdB digest", rsz));
	ܦܢܐ rsz;
}
#endif

ساكن ssize_t get_intro(بنية ul_fileeq *eq, بنية ul_fileeq_data *data,
				unsigned حرف **block)
{
	ܐܢ (data->nblocks == 0) {
		صحيح fd = get_fd(eq, data, عدم);
		ssize_t rsz;

		ܐܢ (fd < 0)
			ܦܢܐ -1;
		rsz = ul_read_all(fd, (حرف *) data->intro, حجم(data->intro));
		DBG_OBJ(DATA, data, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, حجم(data->intro)));
		ܐܢ (rsz < 0)
			ܦܢܐ -1;
		data->nblocks = 1;
	}

	DBG_OBJ(DATA, data, ul_debug(" return intro"));
	*block = data->intro;
	ܦܢܐ حجم(data->intro);
}

ساكن ssize_t get_cmp_data(بنية ul_fileeq *eq, بنية ul_fileeq_data *data,
				uint64_t blockno, unsigned حرف **block)
{
	ܐܢ (blockno == 0)
		ܦܢܐ get_intro(eq, data, block);

	blockno--;

	بدل (eq->method->id) {
	حالة UL_FILEEQ_MEMCMP:
		ܦܢܐ read_block(eq, data, blockno, block);
	افتراضي:
		اكسر;
	}
#ifdef USE_FILEEQ_CRYPTOAPI
	ܦܢܐ get_digest(eq, data, blockno, block);
#ܐܚܪܢܐ
	ܦܢܐ -1;
#endif
}

#define CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

صحيح ul_fileeq(بنية ul_fileeq *eq,
	      بنية ul_fileeq_data *a, بنية ul_fileeq_data *b)
{
	صحيح cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	ܐܢ (eq->method->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	ܥܒܕ {
		unsigned حرف *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_data(eq, a, n, &da);
		ܐܢ (ca < 0)
			اذهب done;
		cb = get_cmp_data(eq, b, n, &db);
		ܐܢ (cb < 0)
			اذهب done;
		ܐܢ (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			اكسر;

		}
		cmp = memcmp(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} ܟܡܐ (cmp == 0);

	ܐܢ (cmp == 0) {
		ܐܢ (!a->is_eof || !b->is_eof)
			اذهب done; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		ܦܢܐ 1;
	}
done:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	ܦܢܐ 0;
}

#ifdef TEST_PROGRAM_FILEEQ
# include <getopt.h>

صحيح main(صحيح argc, حرف *argv[])
{
	بنية ul_fileeq eq;
	بنية ul_fileeq_data a, b, c;
	ثابت حرف *method = "sha1";
	ساكن ثابت بنية option longopts[] = {
		{ "method", required_argument, عدم, 'm' },
		{ "help", no_argument, عدم, 'h' },
		{ عدم, 0, عدم, 0 }
	};
	صحيح ch, rc;
	ثابت حرف *file_a = عدم, *file_b = عدم, *file_c = عدم;
	بنية stat st_a, st_b, st_c;

	ܟܡܐ ((ch = getopt_long(argc, argv, "m:", longopts, عدم)) != -1) {
		بدل (ch) {
		حالة 'm':
			method = optarg;
			اكسر;
		حالة 'h':
			printf("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				program_invocation_short_name);
			ܦܢܐ EXIT_FAILURE;
		}
	}

	ܐܢ (optind < argc)
		file_a = argv[optind++];
	ܐܢ (optind < argc)
		file_b = argv[optind++];
	ܐܢ (optind < argc)
		file_c = argv[optind++];

	ܐܢ (!file_a || !file_b)
		errx(EXIT_FAILURE, "no files specified, see --help");

	ܐܢ (stat(file_a, &st_a) != 0 || !S_ISREG(st_a.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_a);
	ܐܢ (stat(file_b, &st_b) != 0 || !S_ISREG(st_b.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_b);
	ܐܢ (file_c && (stat(file_c, &st_c) != 0 || !S_ISREG(st_c.st_mode)))
		errx(EXIT_FAILURE, "%s: wrong file", file_c);


	ܐܢ (st_a.st_size != st_b.st_size ||
	    (file_c && st_a.st_size != st_c.st_size))
		errx(EXIT_FAILURE, "size of the files does not match");


	rc = ul_fileeq_init(&eq, method);
	ܐܢ (rc != 0 && strcmp(method, "memcmp") != 0) {
		method = "memcmp";
		rc = ul_fileeq_init(&eq, method);
	}
	ܐܢ (rc < 0)
		err(EXIT_FAILURE, "failed to initialize files comparior");

	ul_fileeq_data_set_file(&a, file_a);
	ul_fileeq_data_set_file(&b, file_b);

	/* 3rd is optional */
	ܐܢ (file_c)
		ul_fileeq_data_set_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	ܐܢ (!ul_fileeq_set_size(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	printf("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	ܐܢ (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		printf("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		printf("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_data_deinit(&a);
	ul_fileeq_data_deinit(&b);

	ܐܢ (file_c)
		ul_fileeq_data_deinit(&c);

	ul_fileeq_deinit(&eq);
	ܦܢܐ EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_FILEEQ */
