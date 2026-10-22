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
#اگر_تعریف_شده HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#پایان_اگر

#ضمن "c.h"
#ضمن "all-io.h"
#ضمن "fileeq.h"
#ضمن "debug.h"

ساكن UL_DEBUG_DEFINE_ماسک(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_خالی_MASKNAMES;

#تعریف_کن ULFILEEQ_DEBUG_INIT	(1 << 1)
#تعریف_کن ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#تعریف_کن ULFILEEQ_DEBUG_داده	(1 << 3)
#تعریف_کن ULFILEEQ_DEBUG_EQ	(1 << 4)

#تعریف_کن DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#تعریف_کن DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#تعریف_کن ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

ساكن فراغ ul_fileeq_init_debug(فراغ)
{
	إذا (ulfileeq_debug_ماسک)
		أرجع;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

تعداد {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_بررسی_چرخهای_32
};

بنية ul_fileeq_روش {
	ثابت حرف *name;	/* name used by applications */
	ثابت حرف *kname;	/* name used by kernel crypto */
	صحيح id;
	کوتاه digsiz;
};

ساكن ثابت بنية ul_fileeq_روش ul_eq_methods[] = {
	[UL_FILEEQ_MEMCMP] = {
		.id = UL_FILEEQ_MEMCMP, .name = "memcmp"
	},
#اگر_تعریف_شده USE_FILEEQ_CRYPTOAPI
	[UL_FILEEQ_SHA1] = {
		.id = UL_FILEEQ_SHA1, .name = "sha1",
		.digsiz = 20, .kname = "sha1"
	},
	[UL_FILEEQ_SHA256] = {
		.id = UL_FILEEQ_SHA256, .name = "sha256",
		.digsiz = 32, .kname = "sha256"
	},

	[UL_FILEEQ_بررسی_چرخهای_32] = {
		.id = UL_FILEEQ_بررسی_چرخهای_32, .name = "crc32",
		.digsiz = 4, .kname = "crc32c"
	}
#پایان_اگر
};

#اگر_تعریف_شده USE_FILEEQ_CRYPTOAPI
ساكن فراغ deinit_crypto_api(بنية ul_fileeq *eq)
{
	إذا (!eq)
		أرجع;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	إذا (eq->fd_cip >= 0)
		close(eq->fd_cip);
	إذا (eq->fd_api >= 0)
		close(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

ساكن صحيح init_crypto_api(بنية ul_fileeq *eq)
{
	بنية sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	أكد(eq->روش);
	أكد(eq->روش->kname);
	أكد(eq->fd_api == -1);
	أكد(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->روش->kname));

	أكد(حجم(sa.salg_name) > طول_رشته(eq->روش->kname) + 1);
	کپی_حافظه(&sa.salg_name, eq->روش->kname, طول_رشته(eq->روش->kname) + 1);

	إذا ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		اذهب fail;
	إذا (bind(eq->fd_api, (بنية sockaddr *) &sa, حجم(sa)) != 0)
		اذهب fail;
	إذا ((eq->fd_cip = accept(eq->fd_api, عدم, 0)) < 0)
		اذهب fail;
	أرجع 0;
fail:
	deinit_crypto_api(eq);
	أرجع -1;
}
#پایان_اگر

صحيح ul_fileeq_init(بنية ul_fileeq *eq, ثابت حرف *روش)
{
	size_t i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", روش));

	memset(eq, 0, حجم(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	لكل (i = 0; i < ARRAY_اندازه(ul_eq_methods); i++) {
		ثابت بنية ul_fileeq_روش *m = &ul_eq_methods[i];

		إذا (strcmp(m->name, روش) == 0) {
			eq->روش = m;
			اكسر;
		}
	}

	إذا (!eq->روش)
		أرجع -1;
#اگر_تعریف_شده USE_FILEEQ_CRYPTOAPI
	إذا (eq->روش->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		أرجع -1;
#پایان_اگر
	أرجع 0;
}

ساكن فراغ reset_fileeq_bufs(بنية ul_fileeq *eq)
{
	حرر(eq->buf_a);
	حرر(eq->buf_b);
	eq->buf_آخرین = eq->buf_a = eq->buf_b = عدم;
}

فراغ ul_fileeq_deinit(بنية ul_fileeq *eq)
{
	إذا (!eq)
		أرجع;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#اگر_تعریف_شده USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#پایان_اگر
	reset_fileeq_bufs(eq);
}

فراغ ul_fileeq_داده_close_file(بنية ul_fileeq_داده *داده)
{
	أكد(داده);

	إذا (داده->fd >= 0) {
		DBG_OBJ(داده_2, داده, ul_debug("close"));
		close(داده->fd);
	}
	داده->fd = -1;
}

فراغ ul_fileeq_داده_init(بنية ul_fileeq_داده *داده)
{
	DBG_OBJ(داده_2, داده, ul_debug("init"));
	memset(داده, 0, حجم(*داده));
	داده->fd = -1;
}

فراغ ul_fileeq_داده_deinit(بنية ul_fileeq_داده *داده)
{
	أكد(داده);

	DBG_OBJ(داده_2, داده, ul_debug("deinit"));
	حرر(داده->بلوکها);
	داده->بلوکها = عدم;
	داده->nblocks = 0;
	داده->maxblocks = 0;
	داده->is_eof = 0;
	داده->name = عدم;

	ul_fileeq_داده_close_file(داده);
}

صحيح ul_fileeq_داده_associated(بنية ul_fileeq_داده *داده)
{
	أرجع داده->name != عدم;
}

فراغ ul_fileeq_داده_مجموعه_file(بنية ul_fileeq_داده *داده, ثابت حرف *name)
{
	أكد(داده);
	أكد(name);

	DBG_OBJ(داده_2, داده, ul_debug("set file: %s", name));
	ul_fileeq_داده_init(داده);
	داده->name = name;
}

bool ul_fileeq_مجموعه_اندازه(بنية ul_fileeq *eq, int64_t st_size,
			size_t readsiz, size_t memsiz)
{
	uint64_t filesiz, nreads, maxdigs;
	size_t digsiz;

	أكد(eq);
	أكد(st_size >= 0);
	أكد(readsiz);

	filesiz = (uint64_t) st_size;

	eq->filesiz = filesiz;

	إذا (filesiz != 0 && readsiz > filesiz)
		readsiz = filesiz;

	بدل (eq->روش->id) {
	حالة UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		اكسر;
	افتراضي:
		digsiz = eq->روش->digsiz;
		إذا (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		إذا (maxdigs == 0)
			maxdigs = 1;
		وإلا إذا (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		إذا (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			إذا (ceiling / maxdigs > SIZE_MAX)
				أرجع false;
			readsiz = ceiling / maxdigs;
		}
		اكسر;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	أرجع true;
}

ساكن بیعلامت حرف *get_میانگیر(بنية ul_fileeq *eq)
{
	إذا (!eq->buf_a)
		eq->buf_a = حافظه_بده(eq->readsiz);
	إذا (!eq->buf_b)
		eq->buf_b = حافظه_بده(eq->readsiz);

	إذا (!eq->buf_a || !eq->buf_b)
		أرجع عدم;

	إذا (eq->buf_آخرین == eq->buf_b)
		eq->buf_آخرین = eq->buf_a;
	وإلا
		eq->buf_آخرین = eq->buf_b;

	أرجع eq->buf_آخرین;
}

#تعریف_کن get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#تعریف_کن get_cached_جابه_جایی(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				حجم((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


ساكن صحيح get_fd(بنية ul_fileeq *eq, بنية ul_fileeq_داده *داده, off_t *off)
{
	off_t o = get_cached_جابه_جایی(eq, داده);

	أكد(eq);
	أكد(داده);


	إذا (داده->fd < 0) {
		DBG_OBJ(داده_2, داده, ul_debug("open: %s", داده->name));
		داده->fd = open(داده->name, O_RDONLY | O_CLOEXEC);
		إذا (داده->fd < 0)
			أرجع داده->fd;

#إذا defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_نتیجه( posix_fadvise(داده->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#پایان_اگر
		إذا (o) {
			DBG_OBJ(داده_2, داده, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(داده->fd, o, SEEK_SET);
		}
	}

	إذا (off)
		*off = o;

	أرجع داده->fd;
}

ساكن فراغ memcmp_reset(بنية ul_fileeq *eq, بنية ul_fileeq_داده *داده)
{
	/* only intro[] is cached */
	إذا (داده->nblocks)
		داده->nblocks = 1;
	/* reset file position */
	إذا (داده->fd >= 0)
		lseek(داده->fd, get_cached_جابه_جایی(eq, داده), SEEK_SET);
	داده->is_eof = 0;
}

ساكن ssize_t read_بلوک(بنية ul_fileeq *eq, بنية ul_fileeq_داده *داده,
				uint64_t n, بیعلامت حرف **بلوک)
{
	صحيح fd;
	off_t off = 0;
	ssize_t rsz;

	إذا (داده->is_eof)
		أرجع 0;

	إذا (n >= eq->blocksmax)
		أرجع -EINVAL;

	fd = get_fd(eq, داده, &off);
	إذا (fd < 0)
		أرجع fd;

	DBG_OBJ(داده_2, داده, ul_debug(" read block off=%ju", (uintmax_t) off));

	*بلوک = get_میانگیر(eq);
	إذا (!*بلوک)
		أرجع -ENOMEM;

	rsz = ul_read_all(داده->fd, (حرف *) *بلوک, eq->readsiz);
	إذا (rsz < 0) {
		DBG_OBJ(داده_2, داده, ul_debug("  read failed"));
		أرجع rsz;
	}
	off += rsz;
	داده->nblocks++;

	إذا (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		داده->is_eof = 1;
		ul_fileeq_داده_close_file(داده);
	}

	DBG_OBJ(داده_2, داده, ul_debug("  read sz=%zd", rsz));
	أرجع rsz;
}

#اگر_تعریف_شده USE_FILEEQ_CRYPTOAPI
ساكن ssize_t get_digest(بنية ul_fileeq *eq, بنية ul_fileeq_داده *داده,
				uint64_t n, بیعلامت حرف **بلوک)
{
	off_t off = 0;
	ssize_t rsz;
	size_t sz;
	صحيح fd;

	/* return already cached if available */
	إذا (n < get_cached_nblocks(داده)) {
		DBG_OBJ(داده_2, داده, ul_debug(" digest cached"));
		أكد(داده->بلوکها);
		*بلوک = داده->بلوکها + (n * eq->روش->digsiz);
		أرجع eq->روش->digsiz;
	}

	إذا (داده->is_eof) {
		DBG_OBJ(داده_2, داده, ul_debug(" file EOF"));
		أرجع 0;
	}

	إذا (n >= eq->blocksmax)
		أرجع -EINVAL;

	/* read new block */
	fd = get_fd(eq, داده, &off);
	إذا (fd < 0)
		أرجع fd;

	DBG_OBJ(داده_2, داده, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->روش->digsiz;

	إذا (!داده->بلوکها) {
		DBG_OBJ(داده_2, داده, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		داده->بلوکها = حافظه_بده(eq->blocksmax * sz);
		إذا (!داده->بلوکها)
			أرجع -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, داده->fd, عدم, eq->readsiz);
	DBG_OBJ(داده_2, داده, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	إذا (rsz < 0)
		أرجع rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*بلوک = داده->بلوکها + (n * eq->روش->digsiz);
	rsz = ul_read_all(eq->fd_cip, (حرف *) *بلوک, sz);

	إذا (rsz > 0)
		داده->nblocks++;
	إذا (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		داده->is_eof = 1;
		ul_fileeq_داده_close_file(داده);
	}
	DBG_OBJ(داده_2, داده, ul_debug("  get %zdB digest", rsz));
	أرجع rsz;
}
#پایان_اگر

ساكن ssize_t get_intro(بنية ul_fileeq *eq, بنية ul_fileeq_داده *داده,
				بیعلامت حرف **بلوک)
{
	إذا (داده->nblocks == 0) {
		صحيح fd = get_fd(eq, داده, عدم);
		ssize_t rsz;

		إذا (fd < 0)
			أرجع -1;
		rsz = ul_read_all(fd, (حرف *) داده->intro, حجم(داده->intro));
		DBG_OBJ(داده_2, داده, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, حجم(داده->intro)));
		إذا (rsz < 0)
			أرجع -1;
		داده->nblocks = 1;
	}

	DBG_OBJ(داده_2, داده, ul_debug(" return intro"));
	*بلوک = داده->intro;
	أرجع حجم(داده->intro);
}

ساكن ssize_t get_cmp_داده(بنية ul_fileeq *eq, بنية ul_fileeq_داده *داده,
				uint64_t blockno, بیعلامت حرف **بلوک)
{
	إذا (blockno == 0)
		أرجع get_intro(eq, داده, بلوک);

	blockno--;

	بدل (eq->روش->id) {
	حالة UL_FILEEQ_MEMCMP:
		أرجع read_بلوک(eq, داده, blockno, بلوک);
	افتراضي:
		اكسر;
	}
#اگر_تعریف_شده USE_FILEEQ_CRYPTOAPI
	أرجع get_digest(eq, داده, blockno, بلوک);
#وإلا
	أرجع -1;
#پایان_اگر
}

#تعریف_کن CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

صحيح ul_fileeq(بنية ul_fileeq *eq,
	      بنية ul_fileeq_داده *a, بنية ul_fileeq_داده *b)
{
	صحيح cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	إذا (eq->روش->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	افعل {
		بیعلامت حرف *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_داده(eq, a, n, &da);
		إذا (ca < 0)
			اذهب انجامشده;
		cb = get_cmp_داده(eq, b, n, &db);
		إذا (cb < 0)
			اذهب انجامشده;
		إذا (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			اكسر;

		}
		cmp = memcmp(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} طالما (cmp == 0);

	إذا (cmp == 0) {
		إذا (!a->is_eof || !b->is_eof)
			اذهب انجامشده; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		أرجع 1;
	}
انجامشده:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	أرجع 0;
}

#اگر_تعریف_شده TEST_برنامه_FILEEQ
# include <getopt.h>

صحيح اصلی(صحيح argc, حرف *argv[])
{
	بنية ul_fileeq eq;
	بنية ul_fileeq_داده a, b, c;
	ثابت حرف *روش = "sha1";
	ساكن ثابت بنية option longopts[] = {
		{ "method", required_argument, عدم, 'm' },
		{ "help", no_argument, عدم, 'h' },
		{ عدم, 0, عدم, 0 }
	};
	صحيح ch, rc;
	ثابت حرف *file_a = عدم, *file_b = عدم, *file_c = عدم;
	بنية stat st_a, st_b, st_c;

	طالما ((ch = getopt_long(argc, argv, "m:", longopts, عدم)) != -1) {
		بدل (ch) {
		حالة 'm':
			روش = optarg;
			اكسر;
		حالة 'h':
			چاپ_کن("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				برنامه_invocation_short_name);
			أرجع EXIT_FAILURE;
		}
	}

	إذا (optind < argc)
		file_a = argv[optind++];
	إذا (optind < argc)
		file_b = argv[optind++];
	إذا (optind < argc)
		file_c = argv[optind++];

	إذا (!file_a || !file_b)
		errx(EXIT_FAILURE, "no files specified, see --help");

	إذا (stat(file_a, &st_a) != 0 || !S_ISREG(st_a.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_a);
	إذا (stat(file_b, &st_b) != 0 || !S_ISREG(st_b.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_b);
	إذا (file_c && (stat(file_c, &st_c) != 0 || !S_ISREG(st_c.st_mode)))
		errx(EXIT_FAILURE, "%s: wrong file", file_c);


	إذا (st_a.st_size != st_b.st_size ||
	    (file_c && st_a.st_size != st_c.st_size))
		errx(EXIT_FAILURE, "size of the files does not match");


	rc = ul_fileeq_init(&eq, روش);
	إذا (rc != 0 && strcmp(روش, "memcmp") != 0) {
		روش = "memcmp";
		rc = ul_fileeq_init(&eq, روش);
	}
	إذا (rc < 0)
		err(EXIT_FAILURE, "failed to initialize files comparior");

	ul_fileeq_داده_مجموعه_file(&a, file_a);
	ul_fileeq_داده_مجموعه_file(&b, file_b);

	/* 3rd is optional */
	إذا (file_c)
		ul_fileeq_داده_مجموعه_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	إذا (!ul_fileeq_مجموعه_اندازه(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	چاپ_کن("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	إذا (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		چاپ_کن("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		چاپ_کن("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_داده_deinit(&a);
	ul_fileeq_داده_deinit(&b);

	إذا (file_c)
		ul_fileeq_داده_deinit(&c);

	ul_fileeq_deinit(&eq);
	أرجع EXIT_FAILURE;
}
#پایان_اگر /* TEST_PROGRAM_FILEEQ */
