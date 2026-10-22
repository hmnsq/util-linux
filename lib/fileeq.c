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

#অন্তর্ভাৱ <inttypes.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <sys/stat.h>
#অন্তর্ভাৱ <unistd.h>

/* Linux crypto */
#ifdef HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#endif

#অন্তর্ভাৱ "c.h"
#অন্তর্ভাৱ "all-io.h"
#অন্তর্ভাৱ "fileeq.h"
#অন্তর্ভাৱ "debug.h"

স্থির UL_DEBUG_DEFINE_MASK(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULFILEEQ_DEBUG_INIT	(1 << 1)
#define ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#define ULFILEEQ_DEBUG_DATA	(1 << 3)
#define ULFILEEQ_DEBUG_EQ	(1 << 4)

#define DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

স্থির শূন্য ul_fileeq_init_debug(শূন্য)
{
	যদি (ulfileeq_debug_mask)
		ঘূৰাই;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

গণনা {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_CRC32
};

সংরচনা ul_fileeq_method {
	ধ্রুৱ ৱর্ণ *name;	/* name used by applications */
	ধ্রুৱ ৱর্ণ *kname;	/* name used by kernel crypto */
	পূর্ণাঙ্ক id;
	short digsiz;
};

স্থির ধ্রুৱ সংরচনা ul_fileeq_method ul_eq_methods[] = {
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
স্থির শূন্য deinit_crypto_api(সংরচনা ul_fileeq *eq)
{
	যদি (!eq)
		ঘূৰাই;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	যদি (eq->fd_cip >= 0)
		close(eq->fd_cip);
	যদি (eq->fd_api >= 0)
		close(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

স্থির পূর্ণাঙ্ক init_crypto_api(সংরচনা ul_fileeq *eq)
{
	সংরচনা sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	প্রতিজ্ঞা(eq->method);
	প্রতিজ্ঞা(eq->method->kname);
	প্রতিজ্ঞা(eq->fd_api == -1);
	প্রতিজ্ঞা(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->method->kname));

	প্রতিজ্ঞা(পরিমাণ(sa.salg_name) > strlen(eq->method->kname) + 1);
	memcpy(&sa.salg_name, eq->method->kname, strlen(eq->method->kname) + 1);

	যদি ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		গচ্ছ fail;
	যদি (bind(eq->fd_api, (সংরচনা sockaddr *) &sa, পরিমাণ(sa)) != 0)
		গচ্ছ fail;
	যদি ((eq->fd_cip = accept(eq->fd_api, রিক্ত, 0)) < 0)
		গচ্ছ fail;
	ঘূৰাই 0;
fail:
	deinit_crypto_api(eq);
	ঘূৰাই -1;
}
#endif

পূর্ণাঙ্ক ul_fileeq_init(সংরচনা ul_fileeq *eq, ধ্রুৱ ৱর্ণ *method)
{
	size_t i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", method));

	memset(eq, 0, পরিমাণ(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	বাবে (i = 0; i < ARRAY_SIZE(ul_eq_methods); i++) {
		ধ্রুৱ সংরচনা ul_fileeq_method *m = &ul_eq_methods[i];

		যদি (strcmp(m->name, method) == 0) {
			eq->method = m;
			ভঙ্গ;
		}
	}

	যদি (!eq->method)
		ঘূৰাই -1;
#ifdef USE_FILEEQ_CRYPTOAPI
	যদি (eq->method->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		ঘূৰাই -1;
#endif
	ঘূৰাই 0;
}

স্থির শূন্য reset_fileeq_bufs(সংরচনা ul_fileeq *eq)
{
	মুক্ত(eq->buf_a);
	মুক্ত(eq->buf_b);
	eq->buf_last = eq->buf_a = eq->buf_b = রিক্ত;
}

শূন্য ul_fileeq_deinit(সংরচনা ul_fileeq *eq)
{
	যদি (!eq)
		ঘূৰাই;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#ifdef USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#endif
	reset_fileeq_bufs(eq);
}

শূন্য ul_fileeq_data_close_file(সংরচনা ul_fileeq_data *data)
{
	প্রতিজ্ঞা(data);

	যদি (data->fd >= 0) {
		DBG_OBJ(DATA, data, ul_debug("close"));
		close(data->fd);
	}
	data->fd = -1;
}

শূন্য ul_fileeq_data_init(সংরচনা ul_fileeq_data *data)
{
	DBG_OBJ(DATA, data, ul_debug("init"));
	memset(data, 0, পরিমাণ(*data));
	data->fd = -1;
}

শূন্য ul_fileeq_data_deinit(সংরচনা ul_fileeq_data *data)
{
	প্রতিজ্ঞা(data);

	DBG_OBJ(DATA, data, ul_debug("deinit"));
	মুক্ত(data->blocks);
	data->blocks = রিক্ত;
	data->nblocks = 0;
	data->maxblocks = 0;
	data->is_eof = 0;
	data->name = রিক্ত;

	ul_fileeq_data_close_file(data);
}

পূর্ণাঙ্ক ul_fileeq_data_associated(সংরচনা ul_fileeq_data *data)
{
	ঘূৰাই data->name != রিক্ত;
}

শূন্য ul_fileeq_data_set_file(সংরচনা ul_fileeq_data *data, ধ্রুৱ ৱর্ণ *name)
{
	প্রতিজ্ঞা(data);
	প্রতিজ্ঞা(name);

	DBG_OBJ(DATA, data, ul_debug("set file: %s", name));
	ul_fileeq_data_init(data);
	data->name = name;
}

bool ul_fileeq_set_size(সংরচনা ul_fileeq *eq, int64_t st_size,
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

	ৱিকল্প (eq->method->id) {
	পক্ষ UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		ভঙ্গ;
	স্ৱভাৱ:
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
				ঘূৰাই false;
			readsiz = ceiling / maxdigs;
		}
		ভঙ্গ;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	ঘূৰাই true;
}

স্থির unsigned ৱর্ণ *get_buffer(সংরচনা ul_fileeq *eq)
{
	যদি (!eq->buf_a)
		eq->buf_a = malloc(eq->readsiz);
	যদি (!eq->buf_b)
		eq->buf_b = malloc(eq->readsiz);

	যদি (!eq->buf_a || !eq->buf_b)
		ঘূৰাই রিক্ত;

	যদি (eq->buf_last == eq->buf_b)
		eq->buf_last = eq->buf_a;
	অন্যথা
		eq->buf_last = eq->buf_b;

	ঘূৰাই eq->buf_last;
}

#define get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#define get_cached_offset(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				পরিমাণ((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


স্থির পূর্ণাঙ্ক get_fd(সংরচনা ul_fileeq *eq, সংরচনা ul_fileeq_data *data, off_t *off)
{
	off_t o = get_cached_offset(eq, data);

	প্রতিজ্ঞা(eq);
	প্রতিজ্ঞা(data);


	যদি (data->fd < 0) {
		DBG_OBJ(DATA, data, ul_debug("open: %s", data->name));
		data->fd = open(data->name, O_RDONLY | O_CLOEXEC);
		যদি (data->fd < 0)
			ঘূৰাই data->fd;

#যদি defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_result( posix_fadvise(data->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#endif
		যদি (o) {
			DBG_OBJ(DATA, data, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(data->fd, o, SEEK_SET);
		}
	}

	যদি (off)
		*off = o;

	ঘূৰাই data->fd;
}

স্থির শূন্য memcmp_reset(সংরচনা ul_fileeq *eq, সংরচনা ul_fileeq_data *data)
{
	/* only intro[] is cached */
	যদি (data->nblocks)
		data->nblocks = 1;
	/* reset file position */
	যদি (data->fd >= 0)
		lseek(data->fd, get_cached_offset(eq, data), SEEK_SET);
	data->is_eof = 0;
}

স্থির ssize_t read_block(সংরচনা ul_fileeq *eq, সংরচনা ul_fileeq_data *data,
				uint64_t n, unsigned ৱর্ণ **block)
{
	পূর্ণাঙ্ক fd;
	off_t off = 0;
	ssize_t rsz;

	যদি (data->is_eof)
		ঘূৰাই 0;

	যদি (n >= eq->blocksmax)
		ঘূৰাই -EINVAL;

	fd = get_fd(eq, data, &off);
	যদি (fd < 0)
		ঘূৰাই fd;

	DBG_OBJ(DATA, data, ul_debug(" read block off=%ju", (uintmax_t) off));

	*block = get_buffer(eq);
	যদি (!*block)
		ঘূৰাই -ENOMEM;

	rsz = ul_read_all(data->fd, (ৱর্ণ *) *block, eq->readsiz);
	যদি (rsz < 0) {
		DBG_OBJ(DATA, data, ul_debug("  read failed"));
		ঘূৰাই rsz;
	}
	off += rsz;
	data->nblocks++;

	যদি (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}

	DBG_OBJ(DATA, data, ul_debug("  read sz=%zd", rsz));
	ঘূৰাই rsz;
}

#ifdef USE_FILEEQ_CRYPTOAPI
স্থির ssize_t get_digest(সংরচনা ul_fileeq *eq, সংরচনা ul_fileeq_data *data,
				uint64_t n, unsigned ৱর্ণ **block)
{
	off_t off = 0;
	ssize_t rsz;
	size_t sz;
	পূর্ণাঙ্ক fd;

	/* return already cached if available */
	যদি (n < get_cached_nblocks(data)) {
		DBG_OBJ(DATA, data, ul_debug(" digest cached"));
		প্রতিজ্ঞা(data->blocks);
		*block = data->blocks + (n * eq->method->digsiz);
		ঘূৰাই eq->method->digsiz;
	}

	যদি (data->is_eof) {
		DBG_OBJ(DATA, data, ul_debug(" file EOF"));
		ঘূৰাই 0;
	}

	যদি (n >= eq->blocksmax)
		ঘূৰাই -EINVAL;

	/* read new block */
	fd = get_fd(eq, data, &off);
	যদি (fd < 0)
		ঘূৰাই fd;

	DBG_OBJ(DATA, data, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->method->digsiz;

	যদি (!data->blocks) {
		DBG_OBJ(DATA, data, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		data->blocks = malloc(eq->blocksmax * sz);
		যদি (!data->blocks)
			ঘূৰাই -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, data->fd, রিক্ত, eq->readsiz);
	DBG_OBJ(DATA, data, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	যদি (rsz < 0)
		ঘূৰাই rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*block = data->blocks + (n * eq->method->digsiz);
	rsz = ul_read_all(eq->fd_cip, (ৱর্ণ *) *block, sz);

	যদি (rsz > 0)
		data->nblocks++;
	যদি (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}
	DBG_OBJ(DATA, data, ul_debug("  get %zdB digest", rsz));
	ঘূৰাই rsz;
}
#endif

স্থির ssize_t get_intro(সংরচনা ul_fileeq *eq, সংরচনা ul_fileeq_data *data,
				unsigned ৱর্ণ **block)
{
	যদি (data->nblocks == 0) {
		পূর্ণাঙ্ক fd = get_fd(eq, data, রিক্ত);
		ssize_t rsz;

		যদি (fd < 0)
			ঘূৰাই -1;
		rsz = ul_read_all(fd, (ৱর্ণ *) data->intro, পরিমাণ(data->intro));
		DBG_OBJ(DATA, data, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, পরিমাণ(data->intro)));
		যদি (rsz < 0)
			ঘূৰাই -1;
		data->nblocks = 1;
	}

	DBG_OBJ(DATA, data, ul_debug(" return intro"));
	*block = data->intro;
	ঘূৰাই পরিমাণ(data->intro);
}

স্থির ssize_t get_cmp_data(সংরচনা ul_fileeq *eq, সংরচনা ul_fileeq_data *data,
				uint64_t blockno, unsigned ৱর্ণ **block)
{
	যদি (blockno == 0)
		ঘূৰাই get_intro(eq, data, block);

	blockno--;

	ৱিকল্প (eq->method->id) {
	পক্ষ UL_FILEEQ_MEMCMP:
		ঘূৰাই read_block(eq, data, blockno, block);
	স্ৱভাৱ:
		ভঙ্গ;
	}
#ifdef USE_FILEEQ_CRYPTOAPI
	ঘূৰাই get_digest(eq, data, blockno, block);
#অন্যথা
	ঘূৰাই -1;
#endif
}

#define CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

পূর্ণাঙ্ক ul_fileeq(সংরচনা ul_fileeq *eq,
	      সংরচনা ul_fileeq_data *a, সংরচনা ul_fileeq_data *b)
{
	পূর্ণাঙ্ক cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	যদি (eq->method->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	কৰা {
		unsigned ৱর্ণ *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_data(eq, a, n, &da);
		যদি (ca < 0)
			গচ্ছ done;
		cb = get_cmp_data(eq, b, n, &db);
		যদি (cb < 0)
			গচ্ছ done;
		যদি (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			ভঙ্গ;

		}
		cmp = memcmp(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} যেতিয়ালৈ (cmp == 0);

	যদি (cmp == 0) {
		যদি (!a->is_eof || !b->is_eof)
			গচ্ছ done; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		ঘূৰাই 1;
	}
done:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	ঘূৰাই 0;
}

#ifdef TEST_PROGRAM_FILEEQ
# include <getopt.h>

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ *argv[])
{
	সংরচনা ul_fileeq eq;
	সংরচনা ul_fileeq_data a, b, c;
	ধ্রুৱ ৱর্ণ *method = "sha1";
	স্থির ধ্রুৱ সংরচনা option longopts[] = {
		{ "method", required_argument, রিক্ত, 'm' },
		{ "help", no_argument, রিক্ত, 'h' },
		{ রিক্ত, 0, রিক্ত, 0 }
	};
	পূর্ণাঙ্ক ch, rc;
	ধ্রুৱ ৱর্ণ *file_a = রিক্ত, *file_b = রিক্ত, *file_c = রিক্ত;
	সংরচনা stat st_a, st_b, st_c;

	যেতিয়ালৈ ((ch = getopt_long(argc, argv, "m:", longopts, রিক্ত)) != -1) {
		ৱিকল্প (ch) {
		পক্ষ 'm':
			method = optarg;
			ভঙ্গ;
		পক্ষ 'h':
			printf("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				program_invocation_short_name);
			ঘূৰাই EXIT_FAILURE;
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

	ul_fileeq_data_set_file(&a, file_a);
	ul_fileeq_data_set_file(&b, file_b);

	/* 3rd is optional */
	যদি (file_c)
		ul_fileeq_data_set_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	যদি (!ul_fileeq_set_size(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	printf("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	যদি (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		printf("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		printf("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_data_deinit(&a);
	ul_fileeq_data_deinit(&b);

	যদি (file_c)
		ul_fileeq_data_deinit(&c);

	ul_fileeq_deinit(&eq);
	ঘূৰাই EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_FILEEQ */
