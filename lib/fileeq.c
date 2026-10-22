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

#অন্তর্ভাব <inttypes.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <unistd.h>

/* Linux crypto */
#ifdef HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#endif

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "all-io.h"
#অন্তর্ভাব "fileeq.h"
#অন্তর্ভাব "debug.h"

স্থির UL_DEBUG_DEFINE_MASK(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULFILEEQ_DEBUG_INIT	(1 << 1)
#define ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#define ULFILEEQ_DEBUG_তথ্য	(1 << 3)
#define ULFILEEQ_DEBUG_EQ	(1 << 4)

#define DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

স্থির শূন্য ul_fileeq_init_debug(শূন্য)
{
	যদি (ulfileeq_debug_mask)
		ফেরত;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

গণনা {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_CRC32
};

কাঠামো ul_fileeq_method {
	ধ্রুবক অক্ষর *name;	/* name used by applications */
	ধ্রুবক অক্ষর *kname;	/* name used by kernel crypto */
	পূর্ণসংখ্যা id;
	short digsiz;
};

স্থির ধ্রুবক কাঠামো ul_fileeq_method ul_eq_methods[] = {
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
স্থির শূন্য deinit_crypto_api(কাঠামো ul_fileeq *eq)
{
	যদি (!eq)
		ফেরত;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	যদি (eq->fd_cip >= 0)
		close(eq->fd_cip);
	যদি (eq->fd_api >= 0)
		close(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

স্থির পূর্ণসংখ্যা init_crypto_api(কাঠামো ul_fileeq *eq)
{
	কাঠামো sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	প্রতিজ্ঞা(eq->method);
	প্রতিজ্ঞা(eq->method->kname);
	প্রতিজ্ঞা(eq->fd_api == -1);
	প্রতিজ্ঞা(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->method->kname));

	প্রতিজ্ঞা(আকার(sa.salg_name) > strlen(eq->method->kname) + 1);
	memcpy(&sa.salg_name, eq->method->kname, strlen(eq->method->kname) + 1);

	যদি ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		যাও fail;
	যদি (bind(eq->fd_api, (কাঠামো sockaddr *) &sa, আকার(sa)) != 0)
		যাও fail;
	যদি ((eq->fd_cip = accept(eq->fd_api, ফাঁকা, 0)) < 0)
		যাও fail;
	ফেরত 0;
fail:
	deinit_crypto_api(eq);
	ফেরত -1;
}
#endif

পূর্ণসংখ্যা ul_fileeq_init(কাঠামো ul_fileeq *eq, ধ্রুবক অক্ষর *method)
{
	size_t i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", method));

	memset(eq, 0, আকার(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	জন্য (i = 0; i < ARRAY_আকার(ul_eq_methods); i++) {
		ধ্রুবক কাঠামো ul_fileeq_method *m = &ul_eq_methods[i];

		যদি (strcmp(m->name, method) == 0) {
			eq->method = m;
			বিরতি;
		}
	}

	যদি (!eq->method)
		ফেরত -1;
#ifdef USE_FILEEQ_CRYPTOAPI
	যদি (eq->method->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		ফেরত -1;
#endif
	ফেরত 0;
}

স্থির শূন্য reset_fileeq_bufs(কাঠামো ul_fileeq *eq)
{
	মুক্ত(eq->buf_a);
	মুক্ত(eq->buf_b);
	eq->buf_last = eq->buf_a = eq->buf_b = ফাঁকা;
}

শূন্য ul_fileeq_deinit(কাঠামো ul_fileeq *eq)
{
	যদি (!eq)
		ফেরত;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#ifdef USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#endif
	reset_fileeq_bufs(eq);
}

শূন্য ul_fileeq_তথ্য_close_file(কাঠামো ul_fileeq_তথ্য *তথ্য)
{
	প্রতিজ্ঞা(তথ্য);

	যদি (তথ্য->fd >= 0) {
		DBG_OBJ(তথ্য_2, তথ্য, ul_debug("close"));
		close(তথ্য->fd);
	}
	তথ্য->fd = -1;
}

শূন্য ul_fileeq_তথ্য_init(কাঠামো ul_fileeq_তথ্য *তথ্য)
{
	DBG_OBJ(তথ্য_2, তথ্য, ul_debug("init"));
	memset(তথ্য, 0, আকার(*তথ্য));
	তথ্য->fd = -1;
}

শূন্য ul_fileeq_তথ্য_deinit(কাঠামো ul_fileeq_তথ্য *তথ্য)
{
	প্রতিজ্ঞা(তথ্য);

	DBG_OBJ(তথ্য_2, তথ্য, ul_debug("deinit"));
	মুক্ত(তথ্য->blocks);
	তথ্য->blocks = ফাঁকা;
	তথ্য->nblocks = 0;
	তথ্য->maxblocks = 0;
	তথ্য->is_eof = 0;
	তথ্য->name = ফাঁকা;

	ul_fileeq_তথ্য_close_file(তথ্য);
}

পূর্ণসংখ্যা ul_fileeq_তথ্য_associated(কাঠামো ul_fileeq_তথ্য *তথ্য)
{
	ফেরত তথ্য->name != ফাঁকা;
}

শূন্য ul_fileeq_তথ্য_set_file(কাঠামো ul_fileeq_তথ্য *তথ্য, ধ্রুবক অক্ষর *name)
{
	প্রতিজ্ঞা(তথ্য);
	প্রতিজ্ঞা(name);

	DBG_OBJ(তথ্য_2, তথ্য, ul_debug("set file: %s", name));
	ul_fileeq_তথ্য_init(তথ্য);
	তথ্য->name = name;
}

bool ul_fileeq_set_আকার(কাঠামো ul_fileeq *eq, int64_t st_size,
			size_t readsiz, size_t memsiz)
{
	uint64_t filesiz, nreads, maxdigs;
	size_t digsiz;

	প্রতিজ্ঞা(eq);
	প্রতিজ্ঞা(st_size >= 0);
	প্রতিজ্ঞা(readsiz);

	filesiz = (uint64_t) st_size;

	eq->filesiz = filesiz;

	যদি (filesiz != 0 && readsiz > filesiz)
		readsiz = filesiz;

	নির্বাচন (eq->method->id) {
	ক্ষেত্র UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		বিরতি;
	পূর্বনির্ধারিত:
		digsiz = eq->method->digsiz;
		যদি (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		যদি (maxdigs == 0)
			maxdigs = 1;
		অন্যথা যদি (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		যদি (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			যদি (ceiling / maxdigs > SIZE_MAX)
				ফেরত false;
			readsiz = ceiling / maxdigs;
		}
		বিরতি;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	ফেরত true;
}

স্থির unsigned অক্ষর *get_বাফার(কাঠামো ul_fileeq *eq)
{
	যদি (!eq->buf_a)
		eq->buf_a = malloc(eq->readsiz);
	যদি (!eq->buf_b)
		eq->buf_b = malloc(eq->readsiz);

	যদি (!eq->buf_a || !eq->buf_b)
		ফেরত ফাঁকা;

	যদি (eq->buf_last == eq->buf_b)
		eq->buf_last = eq->buf_a;
	অন্যথা
		eq->buf_last = eq->buf_b;

	ফেরত eq->buf_last;
}

#define get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#define get_cached_offset(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				আকার((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


স্থির পূর্ণসংখ্যা get_fd(কাঠামো ul_fileeq *eq, কাঠামো ul_fileeq_তথ্য *তথ্য, off_t *off)
{
	off_t o = get_cached_offset(eq, তথ্য);

	প্রতিজ্ঞা(eq);
	প্রতিজ্ঞা(তথ্য);


	যদি (তথ্য->fd < 0) {
		DBG_OBJ(তথ্য_2, তথ্য, ul_debug("open: %s", তথ্য->name));
		তথ্য->fd = open(তথ্য->name, O_RDONLY | O_CLOEXEC);
		যদি (তথ্য->fd < 0)
			ফেরত তথ্য->fd;

#যদি defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_ফলাফল( posix_fadvise(তথ্য->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#endif
		যদি (o) {
			DBG_OBJ(তথ্য_2, তথ্য, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(তথ্য->fd, o, SEEK_SET);
		}
	}

	যদি (off)
		*off = o;

	ফেরত তথ্য->fd;
}

স্থির শূন্য memcmp_reset(কাঠামো ul_fileeq *eq, কাঠামো ul_fileeq_তথ্য *তথ্য)
{
	/* only intro[] is cached */
	যদি (তথ্য->nblocks)
		তথ্য->nblocks = 1;
	/* reset file position */
	যদি (তথ্য->fd >= 0)
		lseek(তথ্য->fd, get_cached_offset(eq, তথ্য), SEEK_SET);
	তথ্য->is_eof = 0;
}

স্থির ssize_t read_block(কাঠামো ul_fileeq *eq, কাঠামো ul_fileeq_তথ্য *তথ্য,
				uint64_t n, unsigned অক্ষর **block)
{
	পূর্ণসংখ্যা fd;
	off_t off = 0;
	ssize_t rsz;

	যদি (তথ্য->is_eof)
		ফেরত 0;

	যদি (n >= eq->blocksmax)
		ফেরত -EINVAL;

	fd = get_fd(eq, তথ্য, &off);
	যদি (fd < 0)
		ফেরত fd;

	DBG_OBJ(তথ্য_2, তথ্য, ul_debug(" read block off=%ju", (uintmax_t) off));

	*block = get_বাফার(eq);
	যদি (!*block)
		ফেরত -ENOMEM;

	rsz = ul_read_all(তথ্য->fd, (অক্ষর *) *block, eq->readsiz);
	যদি (rsz < 0) {
		DBG_OBJ(তথ্য_2, তথ্য, ul_debug("  read failed"));
		ফেরত rsz;
	}
	off += rsz;
	তথ্য->nblocks++;

	যদি (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		তথ্য->is_eof = 1;
		ul_fileeq_তথ্য_close_file(তথ্য);
	}

	DBG_OBJ(তথ্য_2, তথ্য, ul_debug("  read sz=%zd", rsz));
	ফেরত rsz;
}

#ifdef USE_FILEEQ_CRYPTOAPI
স্থির ssize_t get_digest(কাঠামো ul_fileeq *eq, কাঠামো ul_fileeq_তথ্য *তথ্য,
				uint64_t n, unsigned অক্ষর **block)
{
	off_t off = 0;
	ssize_t rsz;
	size_t sz;
	পূর্ণসংখ্যা fd;

	/* return already cached if available */
	যদি (n < get_cached_nblocks(তথ্য)) {
		DBG_OBJ(তথ্য_2, তথ্য, ul_debug(" digest cached"));
		প্রতিজ্ঞা(তথ্য->blocks);
		*block = তথ্য->blocks + (n * eq->method->digsiz);
		ফেরত eq->method->digsiz;
	}

	যদি (তথ্য->is_eof) {
		DBG_OBJ(তথ্য_2, তথ্য, ul_debug(" file EOF"));
		ফেরত 0;
	}

	যদি (n >= eq->blocksmax)
		ফেরত -EINVAL;

	/* read new block */
	fd = get_fd(eq, তথ্য, &off);
	যদি (fd < 0)
		ফেরত fd;

	DBG_OBJ(তথ্য_2, তথ্য, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->method->digsiz;

	যদি (!তথ্য->blocks) {
		DBG_OBJ(তথ্য_2, তথ্য, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		তথ্য->blocks = malloc(eq->blocksmax * sz);
		যদি (!তথ্য->blocks)
			ফেরত -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, তথ্য->fd, ফাঁকা, eq->readsiz);
	DBG_OBJ(তথ্য_2, তথ্য, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	যদি (rsz < 0)
		ফেরত rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*block = তথ্য->blocks + (n * eq->method->digsiz);
	rsz = ul_read_all(eq->fd_cip, (অক্ষর *) *block, sz);

	যদি (rsz > 0)
		তথ্য->nblocks++;
	যদি (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		তথ্য->is_eof = 1;
		ul_fileeq_তথ্য_close_file(তথ্য);
	}
	DBG_OBJ(তথ্য_2, তথ্য, ul_debug("  get %zdB digest", rsz));
	ফেরত rsz;
}
#endif

স্থির ssize_t get_intro(কাঠামো ul_fileeq *eq, কাঠামো ul_fileeq_তথ্য *তথ্য,
				unsigned অক্ষর **block)
{
	যদি (তথ্য->nblocks == 0) {
		পূর্ণসংখ্যা fd = get_fd(eq, তথ্য, ফাঁকা);
		ssize_t rsz;

		যদি (fd < 0)
			ফেরত -1;
		rsz = ul_read_all(fd, (অক্ষর *) তথ্য->intro, আকার(তথ্য->intro));
		DBG_OBJ(তথ্য_2, তথ্য, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, আকার(তথ্য->intro)));
		যদি (rsz < 0)
			ফেরত -1;
		তথ্য->nblocks = 1;
	}

	DBG_OBJ(তথ্য_2, তথ্য, ul_debug(" return intro"));
	*block = তথ্য->intro;
	ফেরত আকার(তথ্য->intro);
}

স্থির ssize_t get_cmp_তথ্য(কাঠামো ul_fileeq *eq, কাঠামো ul_fileeq_তথ্য *তথ্য,
				uint64_t blockno, unsigned অক্ষর **block)
{
	যদি (blockno == 0)
		ফেরত get_intro(eq, তথ্য, block);

	blockno--;

	নির্বাচন (eq->method->id) {
	ক্ষেত্র UL_FILEEQ_MEMCMP:
		ফেরত read_block(eq, তথ্য, blockno, block);
	পূর্বনির্ধারিত:
		বিরতি;
	}
#ifdef USE_FILEEQ_CRYPTOAPI
	ফেরত get_digest(eq, তথ্য, blockno, block);
#অন্যথা
	ফেরত -1;
#endif
}

#define CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

পূর্ণসংখ্যা ul_fileeq(কাঠামো ul_fileeq *eq,
	      কাঠামো ul_fileeq_তথ্য *a, কাঠামো ul_fileeq_তথ্য *b)
{
	পূর্ণসংখ্যা cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	যদি (eq->method->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	করো {
		unsigned অক্ষর *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_তথ্য(eq, a, n, &da);
		যদি (ca < 0)
			যাও done;
		cb = get_cmp_তথ্য(eq, b, n, &db);
		যদি (cb < 0)
			যাও done;
		যদি (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			বিরতি;

		}
		cmp = memcmp(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} যখন (cmp == 0);

	যদি (cmp == 0) {
		যদি (!a->is_eof || !b->is_eof)
			যাও done; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		ফেরত 1;
	}
done:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	ফেরত 0;
}

#ifdef TEST_PROGRAM_FILEEQ
# include <getopt.h>

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	কাঠামো ul_fileeq eq;
	কাঠামো ul_fileeq_তথ্য a, b, c;
	ধ্রুবক অক্ষর *method = "sha1";
	স্থির ধ্রুবক কাঠামো option longopts[] = {
		{ "method", required_argument, ফাঁকা, 'm' },
		{ "help", no_argument, ফাঁকা, 'h' },
		{ ফাঁকা, 0, ফাঁকা, 0 }
	};
	পূর্ণসংখ্যা ch, rc;
	ধ্রুবক অক্ষর *file_a = ফাঁকা, *file_b = ফাঁকা, *file_c = ফাঁকা;
	কাঠামো stat st_a, st_b, st_c;

	যখন ((ch = getopt_long(argc, argv, "m:", longopts, ফাঁকা)) != -1) {
		নির্বাচন (ch) {
		ক্ষেত্র 'm':
			method = optarg;
			বিরতি;
		ক্ষেত্র 'h':
			printf("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				program_invocation_short_name);
			ফেরত EXIT_FAILURE;
		}
	}

	যদি (optind < argc)
		file_a = argv[optind++];
	যদি (optind < argc)
		file_b = argv[optind++];
	যদি (optind < argc)
		file_c = argv[optind++];

	যদি (!file_a || !file_b)
		errx(EXIT_FAILURE, "no files specified, see --help");

	যদি (stat(file_a, &st_a) != 0 || !S_ISREG(st_a.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_a);
	যদি (stat(file_b, &st_b) != 0 || !S_ISREG(st_b.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_b);
	যদি (file_c && (stat(file_c, &st_c) != 0 || !S_ISREG(st_c.st_mode)))
		errx(EXIT_FAILURE, "%s: wrong file", file_c);


	যদি (st_a.st_size != st_b.st_size ||
	    (file_c && st_a.st_size != st_c.st_size))
		errx(EXIT_FAILURE, "size of the files does not match");


	rc = ul_fileeq_init(&eq, method);
	যদি (rc != 0 && strcmp(method, "memcmp") != 0) {
		method = "memcmp";
		rc = ul_fileeq_init(&eq, method);
	}
	যদি (rc < 0)
		err(EXIT_FAILURE, "failed to initialize files comparior");

	ul_fileeq_তথ্য_set_file(&a, file_a);
	ul_fileeq_তথ্য_set_file(&b, file_b);

	/* 3rd is optional */
	যদি (file_c)
		ul_fileeq_তথ্য_set_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	যদি (!ul_fileeq_set_আকার(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	printf("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	যদি (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		printf("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		printf("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_তথ্য_deinit(&a);
	ul_fileeq_তথ্য_deinit(&b);

	যদি (file_c)
		ul_fileeq_তথ্য_deinit(&c);

	ul_fileeq_deinit(&eq);
	ফেরত EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_FILEEQ */
