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

#شامل <inttypes.h>
#شامل <stdio.h>
#شامل <sys/types.h>
#شامل <sys/stat.h>
#شامل <unistd.h>

/* Linux crypto */
#ifdef HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#endif

#شامل "c.h"
#شامل "all-io.h"
#شامل "fileeq.h"
#شامل "debug.h"

ایستا UL_DEBUG_DEFINE_MASK(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULFILEEQ_DEBUG_INIT	(1 << 1)
#define ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#define ULFILEEQ_DEBUG_DATA	(1 << 3)
#define ULFILEEQ_DEBUG_EQ	(1 << 4)

#define DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

ایستا تهی ul_fileeq_init_debug(تهی)
{
	اگر (ulfileeq_debug_mask)
		بازگشت;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

شمارش {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_CRC32
};

ساختار ul_fileeq_method {
	ثابت نویسه *name;	/* name used by applications */
	ثابت نویسه *kname;	/* name used by kernel crypto */
	صحیح id;
	short digsiz;
};

ایستا ثابت ساختار ul_fileeq_method ul_eq_methods[] = {
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
ایستا تهی deinit_crypto_api(ساختار ul_fileeq *eq)
{
	اگر (!eq)
		بازگشت;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	اگر (eq->fd_cip >= 0)
		close(eq->fd_cip);
	اگر (eq->fd_api >= 0)
		close(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

ایستا صحیح init_crypto_api(ساختار ul_fileeq *eq)
{
	ساختار sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	أكد(eq->method);
	أكد(eq->method->kname);
	أكد(eq->fd_api == -1);
	أكد(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->method->kname));

	أكد(اندازه(sa.salg_name) > strlen(eq->method->kname) + 1);
	memcpy(&sa.salg_name, eq->method->kname, strlen(eq->method->kname) + 1);

	اگر ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		برو fail;
	اگر (bind(eq->fd_api, (ساختار sockaddr *) &sa, اندازه(sa)) != 0)
		برو fail;
	اگر ((eq->fd_cip = accept(eq->fd_api, پوچ, 0)) < 0)
		برو fail;
	بازگشت 0;
fail:
	deinit_crypto_api(eq);
	بازگشت -1;
}
#endif

صحیح ul_fileeq_init(ساختار ul_fileeq *eq, ثابت نویسه *method)
{
	size_t i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", method));

	memset(eq, 0, اندازه(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	برای (i = 0; i < ARRAY_SIZE(ul_eq_methods); i++) {
		ثابت ساختار ul_fileeq_method *m = &ul_eq_methods[i];

		اگر (strcmp(m->name, method) == 0) {
			eq->method = m;
			بشکن;
		}
	}

	اگر (!eq->method)
		بازگشت -1;
#ifdef USE_FILEEQ_CRYPTOAPI
	اگر (eq->method->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		بازگشت -1;
#endif
	بازگشت 0;
}

ایستا تهی reset_fileeq_bufs(ساختار ul_fileeq *eq)
{
	آزاد(eq->buf_a);
	آزاد(eq->buf_b);
	eq->buf_last = eq->buf_a = eq->buf_b = پوچ;
}

تهی ul_fileeq_deinit(ساختار ul_fileeq *eq)
{
	اگر (!eq)
		بازگشت;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#ifdef USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#endif
	reset_fileeq_bufs(eq);
}

تهی ul_fileeq_data_close_file(ساختار ul_fileeq_data *data)
{
	أكد(data);

	اگر (data->fd >= 0) {
		DBG_OBJ(DATA, data, ul_debug("close"));
		close(data->fd);
	}
	data->fd = -1;
}

تهی ul_fileeq_data_init(ساختار ul_fileeq_data *data)
{
	DBG_OBJ(DATA, data, ul_debug("init"));
	memset(data, 0, اندازه(*data));
	data->fd = -1;
}

تهی ul_fileeq_data_deinit(ساختار ul_fileeq_data *data)
{
	أكد(data);

	DBG_OBJ(DATA, data, ul_debug("deinit"));
	آزاد(data->blocks);
	data->blocks = پوچ;
	data->nblocks = 0;
	data->maxblocks = 0;
	data->is_eof = 0;
	data->name = پوچ;

	ul_fileeq_data_close_file(data);
}

صحیح ul_fileeq_data_associated(ساختار ul_fileeq_data *data)
{
	بازگشت data->name != پوچ;
}

تهی ul_fileeq_data_set_file(ساختار ul_fileeq_data *data, ثابت نویسه *name)
{
	أكد(data);
	أكد(name);

	DBG_OBJ(DATA, data, ul_debug("set file: %s", name));
	ul_fileeq_data_init(data);
	data->name = name;
}

bool ul_fileeq_set_size(ساختار ul_fileeq *eq, int64_t st_size,
			size_t readsiz, size_t memsiz)
{
	uint64_t filesiz, nreads, maxdigs;
	size_t digsiz;

	أكد(eq);
	أكد(st_size >= 0);
	أكد(readsiz);

	filesiz = (uint64_t) st_size;

	eq->filesiz = filesiz;

	اگر (filesiz != 0 && readsiz > filesiz)
		readsiz = filesiz;

	انتخاب (eq->method->id) {
	حالت UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		بشکن;
	پیشفرض:
		digsiz = eq->method->digsiz;
		اگر (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		اگر (maxdigs == 0)
			maxdigs = 1;
		وگرنه اگر (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		اگر (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			اگر (ceiling / maxdigs > SIZE_MAX)
				بازگشت false;
			readsiz = ceiling / maxdigs;
		}
		بشکن;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	بازگشت true;
}

ایستا unsigned نویسه *get_buffer(ساختار ul_fileeq *eq)
{
	اگر (!eq->buf_a)
		eq->buf_a = malloc(eq->readsiz);
	اگر (!eq->buf_b)
		eq->buf_b = malloc(eq->readsiz);

	اگر (!eq->buf_a || !eq->buf_b)
		بازگشت پوچ;

	اگر (eq->buf_last == eq->buf_b)
		eq->buf_last = eq->buf_a;
	وگرنه
		eq->buf_last = eq->buf_b;

	بازگشت eq->buf_last;
}

#define get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#define get_cached_offset(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				اندازه((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


ایستا صحیح get_fd(ساختار ul_fileeq *eq, ساختار ul_fileeq_data *data, off_t *off)
{
	off_t o = get_cached_offset(eq, data);

	أكد(eq);
	أكد(data);


	اگر (data->fd < 0) {
		DBG_OBJ(DATA, data, ul_debug("open: %s", data->name));
		data->fd = open(data->name, O_RDONLY | O_CLOEXEC);
		اگر (data->fd < 0)
			بازگشت data->fd;

#اگر defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_result( posix_fadvise(data->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#endif
		اگر (o) {
			DBG_OBJ(DATA, data, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(data->fd, o, SEEK_SET);
		}
	}

	اگر (off)
		*off = o;

	بازگشت data->fd;
}

ایستا تهی memcmp_reset(ساختار ul_fileeq *eq, ساختار ul_fileeq_data *data)
{
	/* only intro[] is cached */
	اگر (data->nblocks)
		data->nblocks = 1;
	/* reset file position */
	اگر (data->fd >= 0)
		lseek(data->fd, get_cached_offset(eq, data), SEEK_SET);
	data->is_eof = 0;
}

ایستا ssize_t read_block(ساختار ul_fileeq *eq, ساختار ul_fileeq_data *data,
				uint64_t n, unsigned نویسه **block)
{
	صحیح fd;
	off_t off = 0;
	ssize_t rsz;

	اگر (data->is_eof)
		بازگشت 0;

	اگر (n >= eq->blocksmax)
		بازگشت -EINVAL;

	fd = get_fd(eq, data, &off);
	اگر (fd < 0)
		بازگشت fd;

	DBG_OBJ(DATA, data, ul_debug(" read block off=%ju", (uintmax_t) off));

	*block = get_buffer(eq);
	اگر (!*block)
		بازگشت -ENOMEM;

	rsz = ul_read_all(data->fd, (نویسه *) *block, eq->readsiz);
	اگر (rsz < 0) {
		DBG_OBJ(DATA, data, ul_debug("  read failed"));
		بازگشت rsz;
	}
	off += rsz;
	data->nblocks++;

	اگر (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}

	DBG_OBJ(DATA, data, ul_debug("  read sz=%zd", rsz));
	بازگشت rsz;
}

#ifdef USE_FILEEQ_CRYPTOAPI
ایستا ssize_t get_digest(ساختار ul_fileeq *eq, ساختار ul_fileeq_data *data,
				uint64_t n, unsigned نویسه **block)
{
	off_t off = 0;
	ssize_t rsz;
	size_t sz;
	صحیح fd;

	/* return already cached if available */
	اگر (n < get_cached_nblocks(data)) {
		DBG_OBJ(DATA, data, ul_debug(" digest cached"));
		أكد(data->blocks);
		*block = data->blocks + (n * eq->method->digsiz);
		بازگشت eq->method->digsiz;
	}

	اگر (data->is_eof) {
		DBG_OBJ(DATA, data, ul_debug(" file EOF"));
		بازگشت 0;
	}

	اگر (n >= eq->blocksmax)
		بازگشت -EINVAL;

	/* read new block */
	fd = get_fd(eq, data, &off);
	اگر (fd < 0)
		بازگشت fd;

	DBG_OBJ(DATA, data, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->method->digsiz;

	اگر (!data->blocks) {
		DBG_OBJ(DATA, data, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		data->blocks = malloc(eq->blocksmax * sz);
		اگر (!data->blocks)
			بازگشت -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, data->fd, پوچ, eq->readsiz);
	DBG_OBJ(DATA, data, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	اگر (rsz < 0)
		بازگشت rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*block = data->blocks + (n * eq->method->digsiz);
	rsz = ul_read_all(eq->fd_cip, (نویسه *) *block, sz);

	اگر (rsz > 0)
		data->nblocks++;
	اگر (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}
	DBG_OBJ(DATA, data, ul_debug("  get %zdB digest", rsz));
	بازگشت rsz;
}
#endif

ایستا ssize_t get_intro(ساختار ul_fileeq *eq, ساختار ul_fileeq_data *data,
				unsigned نویسه **block)
{
	اگر (data->nblocks == 0) {
		صحیح fd = get_fd(eq, data, پوچ);
		ssize_t rsz;

		اگر (fd < 0)
			بازگشت -1;
		rsz = ul_read_all(fd, (نویسه *) data->intro, اندازه(data->intro));
		DBG_OBJ(DATA, data, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, اندازه(data->intro)));
		اگر (rsz < 0)
			بازگشت -1;
		data->nblocks = 1;
	}

	DBG_OBJ(DATA, data, ul_debug(" return intro"));
	*block = data->intro;
	بازگشت اندازه(data->intro);
}

ایستا ssize_t get_cmp_data(ساختار ul_fileeq *eq, ساختار ul_fileeq_data *data,
				uint64_t blockno, unsigned نویسه **block)
{
	اگر (blockno == 0)
		بازگشت get_intro(eq, data, block);

	blockno--;

	انتخاب (eq->method->id) {
	حالت UL_FILEEQ_MEMCMP:
		بازگشت read_block(eq, data, blockno, block);
	پیشفرض:
		بشکن;
	}
#ifdef USE_FILEEQ_CRYPTOAPI
	بازگشت get_digest(eq, data, blockno, block);
#وگرنه
	بازگشت -1;
#endif
}

#define CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

صحیح ul_fileeq(ساختار ul_fileeq *eq,
	      ساختار ul_fileeq_data *a, ساختار ul_fileeq_data *b)
{
	صحیح cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	اگر (eq->method->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	انجامبده {
		unsigned نویسه *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_data(eq, a, n, &da);
		اگر (ca < 0)
			برو done;
		cb = get_cmp_data(eq, b, n, &db);
		اگر (cb < 0)
			برو done;
		اگر (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			بشکن;

		}
		cmp = memcmp(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} وقتی (cmp == 0);

	اگر (cmp == 0) {
		اگر (!a->is_eof || !b->is_eof)
			برو done; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		بازگشت 1;
	}
done:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	بازگشت 0;
}

#ifdef TEST_PROGRAM_FILEEQ
# include <getopt.h>

صحیح اصلی(صحیح argc, نویسه *argv[])
{
	ساختار ul_fileeq eq;
	ساختار ul_fileeq_data a, b, c;
	ثابت نویسه *method = "sha1";
	ایستا ثابت ساختار option longopts[] = {
		{ "method", required_argument, پوچ, 'm' },
		{ "help", no_argument, پوچ, 'h' },
		{ پوچ, 0, پوچ, 0 }
	};
	صحیح ch, rc;
	ثابت نویسه *file_a = پوچ, *file_b = پوچ, *file_c = پوچ;
	ساختار stat st_a, st_b, st_c;

	وقتی ((ch = getopt_long(argc, argv, "m:", longopts, پوچ)) != -1) {
		انتخاب (ch) {
		حالت 'm':
			method = optarg;
			بشکن;
		حالت 'h':
			printf("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				program_invocation_short_name);
			بازگشت EXIT_FAILURE;
		}
	}

	اگر (optind < argc)
		file_a = argv[optind++];
	اگر (optind < argc)
		file_b = argv[optind++];
	اگر (optind < argc)
		file_c = argv[optind++];

	اگر (!file_a || !file_b)
		errx(EXIT_FAILURE, "no files specified, see --help");

	اگر (stat(file_a, &st_a) != 0 || !S_ISREG(st_a.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_a);
	اگر (stat(file_b, &st_b) != 0 || !S_ISREG(st_b.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_b);
	اگر (file_c && (stat(file_c, &st_c) != 0 || !S_ISREG(st_c.st_mode)))
		errx(EXIT_FAILURE, "%s: wrong file", file_c);


	اگر (st_a.st_size != st_b.st_size ||
	    (file_c && st_a.st_size != st_c.st_size))
		errx(EXIT_FAILURE, "size of the files does not match");


	rc = ul_fileeq_init(&eq, method);
	اگر (rc != 0 && strcmp(method, "memcmp") != 0) {
		method = "memcmp";
		rc = ul_fileeq_init(&eq, method);
	}
	اگر (rc < 0)
		err(EXIT_FAILURE, "failed to initialize files comparior");

	ul_fileeq_data_set_file(&a, file_a);
	ul_fileeq_data_set_file(&b, file_b);

	/* 3rd is optional */
	اگر (file_c)
		ul_fileeq_data_set_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	اگر (!ul_fileeq_set_size(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	printf("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	اگر (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		printf("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		printf("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_data_deinit(&a);
	ul_fileeq_data_deinit(&b);

	اگر (file_c)
		ul_fileeq_data_deinit(&c);

	ul_fileeq_deinit(&eq);
	بازگشت EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_FILEEQ */
