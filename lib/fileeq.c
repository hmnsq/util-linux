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
#جيڪڏهن_وصف_ٿيل HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#جيڪڏهن_پورو

#ضمن "c.h"
#ضمن "all-io.h"
#ضمن "fileeq.h"
#ضمن "debug.h"

ساكن UL_DEBUG_DEFINE_نقاب(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_خالي_جايون_MASKNAMES;

#وصف_ڪريو ULFILEEQ_DEBUG_INIT	(1 << 1)
#وصف_ڪريو ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#وصف_ڪريو ULFILEEQ_DEBUG_معلومات	(1 << 3)
#وصف_ڪريو ULFILEEQ_DEBUG_EQ	(1 << 4)

#وصف_ڪريو DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#وصف_ڪريو DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#وصف_ڪريو ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

ساكن فراغ ul_fileeq_init_debug(فراغ)
{
	إذا (ulfileeq_debug_نقاب)
		أرجع;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

تعداد {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_چڪري_جاچ_32
};

بنية ul_fileeq_طريقو {
	ثابت حرف *name;	/* name used by applications */
	ثابت حرف *kname;	/* name used by kernel crypto */
	صحيح id;
	ننڍو digsiz;
};

ساكن ثابت بنية ul_fileeq_طريقو ul_eq_methods[] = {
	[UL_FILEEQ_MEMCMP] = {
		.id = UL_FILEEQ_MEMCMP, .name = "memcmp"
	},
#جيڪڏهن_وصف_ٿيل USE_FILEEQ_CRYPTOAPI
	[UL_FILEEQ_SHA1] = {
		.id = UL_FILEEQ_SHA1, .name = "sha1",
		.digsiz = 20, .kname = "sha1"
	},
	[UL_FILEEQ_SHA256] = {
		.id = UL_FILEEQ_SHA256, .name = "sha256",
		.digsiz = 32, .kname = "sha256"
	},

	[UL_FILEEQ_چڪري_جاچ_32] = {
		.id = UL_FILEEQ_چڪري_جاچ_32, .name = "crc32",
		.digsiz = 4, .kname = "crc32c"
	}
#جيڪڏهن_پورو
};

#جيڪڏهن_وصف_ٿيل USE_FILEEQ_CRYPTOAPI
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

	أكد(eq->طريقو);
	أكد(eq->طريقو->kname);
	أكد(eq->fd_api == -1);
	أكد(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->طريقو->kname));

	أكد(حجم(sa.salg_name) > لڙي_ڊيگهه(eq->طريقو->kname) + 1);
	يادگيري_نقل(&sa.salg_name, eq->طريقو->kname, لڙي_ڊيگهه(eq->طريقو->kname) + 1);

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
#جيڪڏهن_پورو

صحيح ul_fileeq_init(بنية ul_fileeq *eq, ثابت حرف *طريقو)
{
	size_t i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", طريقو));

	memset(eq, 0, حجم(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	لكل (i = 0; i < ARRAY_سائيز(ul_eq_methods); i++) {
		ثابت بنية ul_fileeq_طريقو *m = &ul_eq_methods[i];

		إذا (strcmp(m->name, طريقو) == 0) {
			eq->طريقو = m;
			اكسر;
		}
	}

	إذا (!eq->طريقو)
		أرجع -1;
#جيڪڏهن_وصف_ٿيل USE_FILEEQ_CRYPTOAPI
	إذا (eq->طريقو->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		أرجع -1;
#جيڪڏهن_پورو
	أرجع 0;
}

ساكن فراغ reset_fileeq_bufs(بنية ul_fileeq *eq)
{
	حرر(eq->buf_a);
	حرر(eq->buf_b);
	eq->buf_آخري = eq->buf_a = eq->buf_b = عدم;
}

فراغ ul_fileeq_deinit(بنية ul_fileeq *eq)
{
	إذا (!eq)
		أرجع;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#جيڪڏهن_وصف_ٿيل USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#جيڪڏهن_پورو
	reset_fileeq_bufs(eq);
}

فراغ ul_fileeq_معلومات_close_file(بنية ul_fileeq_معلومات *معلومات)
{
	أكد(معلومات);

	إذا (معلومات->fd >= 0) {
		DBG_OBJ(معلومات_2, معلومات, ul_debug("close"));
		close(معلومات->fd);
	}
	معلومات->fd = -1;
}

فراغ ul_fileeq_معلومات_init(بنية ul_fileeq_معلومات *معلومات)
{
	DBG_OBJ(معلومات_2, معلومات, ul_debug("init"));
	memset(معلومات, 0, حجم(*معلومات));
	معلومات->fd = -1;
}

فراغ ul_fileeq_معلومات_deinit(بنية ul_fileeq_معلومات *معلومات)
{
	أكد(معلومات);

	DBG_OBJ(معلومات_2, معلومات, ul_debug("deinit"));
	حرر(معلومات->بلاڪون);
	معلومات->بلاڪون = عدم;
	معلومات->nblocks = 0;
	معلومات->maxblocks = 0;
	معلومات->is_eof = 0;
	معلومات->name = عدم;

	ul_fileeq_معلومات_close_file(معلومات);
}

صحيح ul_fileeq_معلومات_associated(بنية ul_fileeq_معلومات *معلومات)
{
	أرجع معلومات->name != عدم;
}

فراغ ul_fileeq_معلومات_سيٽ_file(بنية ul_fileeq_معلومات *معلومات, ثابت حرف *name)
{
	أكد(معلومات);
	أكد(name);

	DBG_OBJ(معلومات_2, معلومات, ul_debug("set file: %s", name));
	ul_fileeq_معلومات_init(معلومات);
	معلومات->name = name;
}

bool ul_fileeq_سيٽ_سائيز(بنية ul_fileeq *eq, int64_t st_size,
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

	بدل (eq->طريقو->id) {
	حالة UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		اكسر;
	افتراضي:
		digsiz = eq->طريقو->digsiz;
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

ساكن بنانشان حرف *get_بفر(بنية ul_fileeq *eq)
{
	إذا (!eq->buf_a)
		eq->buf_a = يادگيري_ڏيو(eq->readsiz);
	إذا (!eq->buf_b)
		eq->buf_b = يادگيري_ڏيو(eq->readsiz);

	إذا (!eq->buf_a || !eq->buf_b)
		أرجع عدم;

	إذا (eq->buf_آخري == eq->buf_b)
		eq->buf_آخري = eq->buf_a;
	وإلا
		eq->buf_آخري = eq->buf_b;

	أرجع eq->buf_آخري;
}

#وصف_ڪريو get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#وصف_ڪريو get_cached_وٿي(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				حجم((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


ساكن صحيح get_fd(بنية ul_fileeq *eq, بنية ul_fileeq_معلومات *معلومات, off_t *off)
{
	off_t o = get_cached_وٿي(eq, معلومات);

	أكد(eq);
	أكد(معلومات);


	إذا (معلومات->fd < 0) {
		DBG_OBJ(معلومات_2, معلومات, ul_debug("open: %s", معلومات->name));
		معلومات->fd = open(معلومات->name, O_RDONLY | O_CLOEXEC);
		إذا (معلومات->fd < 0)
			أرجع معلومات->fd;

#إذا defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_نتيجو( posix_fadvise(معلومات->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#جيڪڏهن_پورو
		إذا (o) {
			DBG_OBJ(معلومات_2, معلومات, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(معلومات->fd, o, SEEK_SET);
		}
	}

	إذا (off)
		*off = o;

	أرجع معلومات->fd;
}

ساكن فراغ memcmp_reset(بنية ul_fileeq *eq, بنية ul_fileeq_معلومات *معلومات)
{
	/* only intro[] is cached */
	إذا (معلومات->nblocks)
		معلومات->nblocks = 1;
	/* reset file position */
	إذا (معلومات->fd >= 0)
		lseek(معلومات->fd, get_cached_وٿي(eq, معلومات), SEEK_SET);
	معلومات->is_eof = 0;
}

ساكن ssize_t read_بلاڪ(بنية ul_fileeq *eq, بنية ul_fileeq_معلومات *معلومات,
				uint64_t n, بنانشان حرف **بلاڪ)
{
	صحيح fd;
	off_t off = 0;
	ssize_t rsz;

	إذا (معلومات->is_eof)
		أرجع 0;

	إذا (n >= eq->blocksmax)
		أرجع -EINVAL;

	fd = get_fd(eq, معلومات, &off);
	إذا (fd < 0)
		أرجع fd;

	DBG_OBJ(معلومات_2, معلومات, ul_debug(" read block off=%ju", (uintmax_t) off));

	*بلاڪ = get_بفر(eq);
	إذا (!*بلاڪ)
		أرجع -ENOMEM;

	rsz = ul_read_all(معلومات->fd, (حرف *) *بلاڪ, eq->readsiz);
	إذا (rsz < 0) {
		DBG_OBJ(معلومات_2, معلومات, ul_debug("  read failed"));
		أرجع rsz;
	}
	off += rsz;
	معلومات->nblocks++;

	إذا (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		معلومات->is_eof = 1;
		ul_fileeq_معلومات_close_file(معلومات);
	}

	DBG_OBJ(معلومات_2, معلومات, ul_debug("  read sz=%zd", rsz));
	أرجع rsz;
}

#جيڪڏهن_وصف_ٿيل USE_FILEEQ_CRYPTOAPI
ساكن ssize_t get_digest(بنية ul_fileeq *eq, بنية ul_fileeq_معلومات *معلومات,
				uint64_t n, بنانشان حرف **بلاڪ)
{
	off_t off = 0;
	ssize_t rsz;
	size_t sz;
	صحيح fd;

	/* return already cached if available */
	إذا (n < get_cached_nblocks(معلومات)) {
		DBG_OBJ(معلومات_2, معلومات, ul_debug(" digest cached"));
		أكد(معلومات->بلاڪون);
		*بلاڪ = معلومات->بلاڪون + (n * eq->طريقو->digsiz);
		أرجع eq->طريقو->digsiz;
	}

	إذا (معلومات->is_eof) {
		DBG_OBJ(معلومات_2, معلومات, ul_debug(" file EOF"));
		أرجع 0;
	}

	إذا (n >= eq->blocksmax)
		أرجع -EINVAL;

	/* read new block */
	fd = get_fd(eq, معلومات, &off);
	إذا (fd < 0)
		أرجع fd;

	DBG_OBJ(معلومات_2, معلومات, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->طريقو->digsiz;

	إذا (!معلومات->بلاڪون) {
		DBG_OBJ(معلومات_2, معلومات, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		معلومات->بلاڪون = يادگيري_ڏيو(eq->blocksmax * sz);
		إذا (!معلومات->بلاڪون)
			أرجع -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, معلومات->fd, عدم, eq->readsiz);
	DBG_OBJ(معلومات_2, معلومات, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	إذا (rsz < 0)
		أرجع rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*بلاڪ = معلومات->بلاڪون + (n * eq->طريقو->digsiz);
	rsz = ul_read_all(eq->fd_cip, (حرف *) *بلاڪ, sz);

	إذا (rsz > 0)
		معلومات->nblocks++;
	إذا (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		معلومات->is_eof = 1;
		ul_fileeq_معلومات_close_file(معلومات);
	}
	DBG_OBJ(معلومات_2, معلومات, ul_debug("  get %zdB digest", rsz));
	أرجع rsz;
}
#جيڪڏهن_پورو

ساكن ssize_t get_intro(بنية ul_fileeq *eq, بنية ul_fileeq_معلومات *معلومات,
				بنانشان حرف **بلاڪ)
{
	إذا (معلومات->nblocks == 0) {
		صحيح fd = get_fd(eq, معلومات, عدم);
		ssize_t rsz;

		إذا (fd < 0)
			أرجع -1;
		rsz = ul_read_all(fd, (حرف *) معلومات->intro, حجم(معلومات->intro));
		DBG_OBJ(معلومات_2, معلومات, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, حجم(معلومات->intro)));
		إذا (rsz < 0)
			أرجع -1;
		معلومات->nblocks = 1;
	}

	DBG_OBJ(معلومات_2, معلومات, ul_debug(" return intro"));
	*بلاڪ = معلومات->intro;
	أرجع حجم(معلومات->intro);
}

ساكن ssize_t get_cmp_معلومات(بنية ul_fileeq *eq, بنية ul_fileeq_معلومات *معلومات,
				uint64_t blockno, بنانشان حرف **بلاڪ)
{
	إذا (blockno == 0)
		أرجع get_intro(eq, معلومات, بلاڪ);

	blockno--;

	بدل (eq->طريقو->id) {
	حالة UL_FILEEQ_MEMCMP:
		أرجع read_بلاڪ(eq, معلومات, blockno, بلاڪ);
	افتراضي:
		اكسر;
	}
#جيڪڏهن_وصف_ٿيل USE_FILEEQ_CRYPTOAPI
	أرجع get_digest(eq, معلومات, blockno, بلاڪ);
#وإلا
	أرجع -1;
#جيڪڏهن_پورو
}

#وصف_ڪريو CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

صحيح ul_fileeq(بنية ul_fileeq *eq,
	      بنية ul_fileeq_معلومات *a, بنية ul_fileeq_معلومات *b)
{
	صحيح cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	إذا (eq->طريقو->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	افعل {
		بنانشان حرف *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_معلومات(eq, a, n, &da);
		إذا (ca < 0)
			اذهب پورو;
		cb = get_cmp_معلومات(eq, b, n, &db);
		إذا (cb < 0)
			اذهب پورو;
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
			اذهب پورو; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		أرجع 1;
	}
پورو:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	أرجع 0;
}

#جيڪڏهن_وصف_ٿيل TEST_پروگرام_FILEEQ
# include <getopt.h>

صحيح مکيه(صحيح argc, حرف *argv[])
{
	بنية ul_fileeq eq;
	بنية ul_fileeq_معلومات a, b, c;
	ثابت حرف *طريقو = "sha1";
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
			طريقو = optarg;
			اكسر;
		حالة 'h':
			لکو("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				پروگرام_invocation_short_name);
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


	rc = ul_fileeq_init(&eq, طريقو);
	إذا (rc != 0 && strcmp(طريقو, "memcmp") != 0) {
		طريقو = "memcmp";
		rc = ul_fileeq_init(&eq, طريقو);
	}
	إذا (rc < 0)
		err(EXIT_FAILURE, "failed to initialize files comparior");

	ul_fileeq_معلومات_سيٽ_file(&a, file_a);
	ul_fileeq_معلومات_سيٽ_file(&b, file_b);

	/* 3rd is optional */
	إذا (file_c)
		ul_fileeq_معلومات_سيٽ_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	إذا (!ul_fileeq_سيٽ_سائيز(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	لکو("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	إذا (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		لکو("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		لکو("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_معلومات_deinit(&a);
	ul_fileeq_معلومات_deinit(&b);

	إذا (file_c)
		ul_fileeq_معلومات_deinit(&c);

	ul_fileeq_deinit(&eq);
	أرجع EXIT_FAILURE;
}
#جيڪڏهن_پورو /* TEST_PROGRAM_FILEEQ */
