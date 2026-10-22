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
#اگر_تعریف_شدہ HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#اگر_ختم

#شامل "c.h"
#شامل "all-io.h"
#شامل "fileeq.h"
#شامل "debug.h"

ساکن UL_DEBUG_DEFINE_نقاب(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_خالی_خانے_MASKNAMES;

#تعریف_کرو ULFILEEQ_DEBUG_INIT	(1 << 1)
#تعریف_کرو ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#تعریف_کرو ULFILEEQ_DEBUG_ڈیٹا	(1 << 3)
#تعریف_کرو ULFILEEQ_DEBUG_EQ	(1 << 4)

#تعریف_کرو DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#تعریف_کرو DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#تعریف_کرو ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

ساکن خالی ul_fileeq_init_debug(خالی)
{
	اگر (ulfileeq_debug_نقاب)
		واپس;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

شمار {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_چکری_جانچ_32
};

ساخت ul_fileeq_طریقہ {
	ثابت حرف *name;	/* name used by applications */
	ثابت حرف *kname;	/* name used by kernel crypto */
	صحیح id;
	مختصر digsiz;
};

ساکن ثابت ساخت ul_fileeq_طریقہ ul_eq_methods[] = {
	[UL_FILEEQ_MEMCMP] = {
		.id = UL_FILEEQ_MEMCMP, .name = "memcmp"
	},
#اگر_تعریف_شدہ USE_FILEEQ_CRYPTOAPI
	[UL_FILEEQ_SHA1] = {
		.id = UL_FILEEQ_SHA1, .name = "sha1",
		.digsiz = 20, .kname = "sha1"
	},
	[UL_FILEEQ_SHA256] = {
		.id = UL_FILEEQ_SHA256, .name = "sha256",
		.digsiz = 32, .kname = "sha256"
	},

	[UL_FILEEQ_چکری_جانچ_32] = {
		.id = UL_FILEEQ_چکری_جانچ_32, .name = "crc32",
		.digsiz = 4, .kname = "crc32c"
	}
#اگر_ختم
};

#اگر_تعریف_شدہ USE_FILEEQ_CRYPTOAPI
ساکن خالی deinit_crypto_api(ساخت ul_fileeq *eq)
{
	اگر (!eq)
		واپس;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	اگر (eq->fd_cip >= 0)
		close(eq->fd_cip);
	اگر (eq->fd_api >= 0)
		close(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

ساکن صحیح init_crypto_api(ساخت ul_fileeq *eq)
{
	ساخت sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	أكد(eq->طریقہ);
	أكد(eq->طریقہ->kname);
	أكد(eq->fd_api == -1);
	أكد(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->طریقہ->kname));

	أكد(حجم(sa.salg_name) > لڑی_لمبائی(eq->طریقہ->kname) + 1);
	حافظہ_نقل(&sa.salg_name, eq->طریقہ->kname, لڑی_لمبائی(eq->طریقہ->kname) + 1);

	اگر ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		جاؤ fail;
	اگر (bind(eq->fd_api, (ساخت sockaddr *) &sa, حجم(sa)) != 0)
		جاؤ fail;
	اگر ((eq->fd_cip = accept(eq->fd_api, تہی, 0)) < 0)
		جاؤ fail;
	واپس 0;
fail:
	deinit_crypto_api(eq);
	واپس -1;
}
#اگر_ختم

صحیح ul_fileeq_init(ساخت ul_fileeq *eq, ثابت حرف *طریقہ)
{
	size_t i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", طریقہ));

	memset(eq, 0, حجم(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	برائے (i = 0; i < ARRAY_سائز(ul_eq_methods); i++) {
		ثابت ساخت ul_fileeq_طریقہ *m = &ul_eq_methods[i];

		اگر (strcmp(m->name, طریقہ) == 0) {
			eq->طریقہ = m;
			توڑو;
		}
	}

	اگر (!eq->طریقہ)
		واپس -1;
#اگر_تعریف_شدہ USE_FILEEQ_CRYPTOAPI
	اگر (eq->طریقہ->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		واپس -1;
#اگر_ختم
	واپس 0;
}

ساکن خالی reset_fileeq_bufs(ساخت ul_fileeq *eq)
{
	آزاد(eq->buf_a);
	آزاد(eq->buf_b);
	eq->buf_آخری = eq->buf_a = eq->buf_b = تہی;
}

خالی ul_fileeq_deinit(ساخت ul_fileeq *eq)
{
	اگر (!eq)
		واپس;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#اگر_تعریف_شدہ USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#اگر_ختم
	reset_fileeq_bufs(eq);
}

خالی ul_fileeq_ڈیٹا_close_file(ساخت ul_fileeq_ڈیٹا *ڈیٹا)
{
	أكد(ڈیٹا);

	اگر (ڈیٹا->fd >= 0) {
		DBG_OBJ(ڈیٹا_2, ڈیٹا, ul_debug("close"));
		close(ڈیٹا->fd);
	}
	ڈیٹا->fd = -1;
}

خالی ul_fileeq_ڈیٹا_init(ساخت ul_fileeq_ڈیٹا *ڈیٹا)
{
	DBG_OBJ(ڈیٹا_2, ڈیٹا, ul_debug("init"));
	memset(ڈیٹا, 0, حجم(*ڈیٹا));
	ڈیٹا->fd = -1;
}

خالی ul_fileeq_ڈیٹا_deinit(ساخت ul_fileeq_ڈیٹا *ڈیٹا)
{
	أكد(ڈیٹا);

	DBG_OBJ(ڈیٹا_2, ڈیٹا, ul_debug("deinit"));
	آزاد(ڈیٹا->بلاکیں);
	ڈیٹا->بلاکیں = تہی;
	ڈیٹا->nblocks = 0;
	ڈیٹا->maxblocks = 0;
	ڈیٹا->is_eof = 0;
	ڈیٹا->name = تہی;

	ul_fileeq_ڈیٹا_close_file(ڈیٹا);
}

صحیح ul_fileeq_ڈیٹا_associated(ساخت ul_fileeq_ڈیٹا *ڈیٹا)
{
	واپس ڈیٹا->name != تہی;
}

خالی ul_fileeq_ڈیٹا_سیٹ_file(ساخت ul_fileeq_ڈیٹا *ڈیٹا, ثابت حرف *name)
{
	أكد(ڈیٹا);
	أكد(name);

	DBG_OBJ(ڈیٹا_2, ڈیٹا, ul_debug("set file: %s", name));
	ul_fileeq_ڈیٹا_init(ڈیٹا);
	ڈیٹا->name = name;
}

bool ul_fileeq_سیٹ_سائز(ساخت ul_fileeq *eq, int64_t st_size,
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

	انتخاب (eq->طریقہ->id) {
	حالت UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		توڑو;
	طےشدہ:
		digsiz = eq->طریقہ->digsiz;
		اگر (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		اگر (maxdigs == 0)
			maxdigs = 1;
		ورنہ اگر (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		اگر (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			اگر (ceiling / maxdigs > SIZE_MAX)
				واپس false;
			readsiz = ceiling / maxdigs;
		}
		توڑو;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	واپس true;
}

ساکن غیرعلامتی حرف *get_بفر(ساخت ul_fileeq *eq)
{
	اگر (!eq->buf_a)
		eq->buf_a = حافظہ_دو(eq->readsiz);
	اگر (!eq->buf_b)
		eq->buf_b = حافظہ_دو(eq->readsiz);

	اگر (!eq->buf_a || !eq->buf_b)
		واپس تہی;

	اگر (eq->buf_آخری == eq->buf_b)
		eq->buf_آخری = eq->buf_a;
	ورنہ
		eq->buf_آخری = eq->buf_b;

	واپس eq->buf_آخری;
}

#تعریف_کرو get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#تعریف_کرو get_cached_انحراف(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				حجم((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


ساکن صحیح get_fd(ساخت ul_fileeq *eq, ساخت ul_fileeq_ڈیٹا *ڈیٹا, off_t *off)
{
	off_t o = get_cached_انحراف(eq, ڈیٹا);

	أكد(eq);
	أكد(ڈیٹا);


	اگر (ڈیٹا->fd < 0) {
		DBG_OBJ(ڈیٹا_2, ڈیٹا, ul_debug("open: %s", ڈیٹا->name));
		ڈیٹا->fd = open(ڈیٹا->name, O_RDONLY | O_CLOEXEC);
		اگر (ڈیٹا->fd < 0)
			واپس ڈیٹا->fd;

#اگر defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_نتیجہ( posix_fadvise(ڈیٹا->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#اگر_ختم
		اگر (o) {
			DBG_OBJ(ڈیٹا_2, ڈیٹا, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(ڈیٹا->fd, o, SEEK_SET);
		}
	}

	اگر (off)
		*off = o;

	واپس ڈیٹا->fd;
}

ساکن خالی memcmp_reset(ساخت ul_fileeq *eq, ساخت ul_fileeq_ڈیٹا *ڈیٹا)
{
	/* only intro[] is cached */
	اگر (ڈیٹا->nblocks)
		ڈیٹا->nblocks = 1;
	/* reset file position */
	اگر (ڈیٹا->fd >= 0)
		lseek(ڈیٹا->fd, get_cached_انحراف(eq, ڈیٹا), SEEK_SET);
	ڈیٹا->is_eof = 0;
}

ساکن ssize_t read_بلاک(ساخت ul_fileeq *eq, ساخت ul_fileeq_ڈیٹا *ڈیٹا,
				uint64_t n, غیرعلامتی حرف **بلاک)
{
	صحیح fd;
	off_t off = 0;
	ssize_t rsz;

	اگر (ڈیٹا->is_eof)
		واپس 0;

	اگر (n >= eq->blocksmax)
		واپس -EINVAL;

	fd = get_fd(eq, ڈیٹا, &off);
	اگر (fd < 0)
		واپس fd;

	DBG_OBJ(ڈیٹا_2, ڈیٹا, ul_debug(" read block off=%ju", (uintmax_t) off));

	*بلاک = get_بفر(eq);
	اگر (!*بلاک)
		واپس -ENOMEM;

	rsz = ul_read_all(ڈیٹا->fd, (حرف *) *بلاک, eq->readsiz);
	اگر (rsz < 0) {
		DBG_OBJ(ڈیٹا_2, ڈیٹا, ul_debug("  read failed"));
		واپس rsz;
	}
	off += rsz;
	ڈیٹا->nblocks++;

	اگر (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		ڈیٹا->is_eof = 1;
		ul_fileeq_ڈیٹا_close_file(ڈیٹا);
	}

	DBG_OBJ(ڈیٹا_2, ڈیٹا, ul_debug("  read sz=%zd", rsz));
	واپس rsz;
}

#اگر_تعریف_شدہ USE_FILEEQ_CRYPTOAPI
ساکن ssize_t get_digest(ساخت ul_fileeq *eq, ساخت ul_fileeq_ڈیٹا *ڈیٹا,
				uint64_t n, غیرعلامتی حرف **بلاک)
{
	off_t off = 0;
	ssize_t rsz;
	size_t sz;
	صحیح fd;

	/* return already cached if available */
	اگر (n < get_cached_nblocks(ڈیٹا)) {
		DBG_OBJ(ڈیٹا_2, ڈیٹا, ul_debug(" digest cached"));
		أكد(ڈیٹا->بلاکیں);
		*بلاک = ڈیٹا->بلاکیں + (n * eq->طریقہ->digsiz);
		واپس eq->طریقہ->digsiz;
	}

	اگر (ڈیٹا->is_eof) {
		DBG_OBJ(ڈیٹا_2, ڈیٹا, ul_debug(" file EOF"));
		واپس 0;
	}

	اگر (n >= eq->blocksmax)
		واپس -EINVAL;

	/* read new block */
	fd = get_fd(eq, ڈیٹا, &off);
	اگر (fd < 0)
		واپس fd;

	DBG_OBJ(ڈیٹا_2, ڈیٹا, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->طریقہ->digsiz;

	اگر (!ڈیٹا->بلاکیں) {
		DBG_OBJ(ڈیٹا_2, ڈیٹا, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		ڈیٹا->بلاکیں = حافظہ_دو(eq->blocksmax * sz);
		اگر (!ڈیٹا->بلاکیں)
			واپس -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, ڈیٹا->fd, تہی, eq->readsiz);
	DBG_OBJ(ڈیٹا_2, ڈیٹا, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	اگر (rsz < 0)
		واپس rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*بلاک = ڈیٹا->بلاکیں + (n * eq->طریقہ->digsiz);
	rsz = ul_read_all(eq->fd_cip, (حرف *) *بلاک, sz);

	اگر (rsz > 0)
		ڈیٹا->nblocks++;
	اگر (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		ڈیٹا->is_eof = 1;
		ul_fileeq_ڈیٹا_close_file(ڈیٹا);
	}
	DBG_OBJ(ڈیٹا_2, ڈیٹا, ul_debug("  get %zdB digest", rsz));
	واپس rsz;
}
#اگر_ختم

ساکن ssize_t get_intro(ساخت ul_fileeq *eq, ساخت ul_fileeq_ڈیٹا *ڈیٹا,
				غیرعلامتی حرف **بلاک)
{
	اگر (ڈیٹا->nblocks == 0) {
		صحیح fd = get_fd(eq, ڈیٹا, تہی);
		ssize_t rsz;

		اگر (fd < 0)
			واپس -1;
		rsz = ul_read_all(fd, (حرف *) ڈیٹا->intro, حجم(ڈیٹا->intro));
		DBG_OBJ(ڈیٹا_2, ڈیٹا, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, حجم(ڈیٹا->intro)));
		اگر (rsz < 0)
			واپس -1;
		ڈیٹا->nblocks = 1;
	}

	DBG_OBJ(ڈیٹا_2, ڈیٹا, ul_debug(" return intro"));
	*بلاک = ڈیٹا->intro;
	واپس حجم(ڈیٹا->intro);
}

ساکن ssize_t get_cmp_ڈیٹا(ساخت ul_fileeq *eq, ساخت ul_fileeq_ڈیٹا *ڈیٹا,
				uint64_t blockno, غیرعلامتی حرف **بلاک)
{
	اگر (blockno == 0)
		واپس get_intro(eq, ڈیٹا, بلاک);

	blockno--;

	انتخاب (eq->طریقہ->id) {
	حالت UL_FILEEQ_MEMCMP:
		واپس read_بلاک(eq, ڈیٹا, blockno, بلاک);
	طےشدہ:
		توڑو;
	}
#اگر_تعریف_شدہ USE_FILEEQ_CRYPTOAPI
	واپس get_digest(eq, ڈیٹا, blockno, بلاک);
#ورنہ
	واپس -1;
#اگر_ختم
}

#تعریف_کرو CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

صحیح ul_fileeq(ساخت ul_fileeq *eq,
	      ساخت ul_fileeq_ڈیٹا *a, ساخت ul_fileeq_ڈیٹا *b)
{
	صحیح cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	اگر (eq->طریقہ->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	کرو {
		غیرعلامتی حرف *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_ڈیٹا(eq, a, n, &da);
		اگر (ca < 0)
			جاؤ مکمل;
		cb = get_cmp_ڈیٹا(eq, b, n, &db);
		اگر (cb < 0)
			جاؤ مکمل;
		اگر (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			توڑو;

		}
		cmp = memcmp(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} جب (cmp == 0);

	اگر (cmp == 0) {
		اگر (!a->is_eof || !b->is_eof)
			جاؤ مکمل; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		واپس 1;
	}
مکمل:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	واپس 0;
}

#اگر_تعریف_شدہ TEST_پروگرام_FILEEQ
# include <getopt.h>

صحیح مین(صحیح argc, حرف *argv[])
{
	ساخت ul_fileeq eq;
	ساخت ul_fileeq_ڈیٹا a, b, c;
	ثابت حرف *طریقہ = "sha1";
	ساکن ثابت ساخت option longopts[] = {
		{ "method", required_argument, تہی, 'm' },
		{ "help", no_argument, تہی, 'h' },
		{ تہی, 0, تہی, 0 }
	};
	صحیح ch, rc;
	ثابت حرف *file_a = تہی, *file_b = تہی, *file_c = تہی;
	ساخت stat st_a, st_b, st_c;

	جب ((ch = getopt_long(argc, argv, "m:", longopts, تہی)) != -1) {
		انتخاب (ch) {
		حالت 'm':
			طریقہ = optarg;
			توڑو;
		حالت 'h':
			لکھو("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				پروگرام_invocation_short_name);
			واپس EXIT_FAILURE;
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


	rc = ul_fileeq_init(&eq, طریقہ);
	اگر (rc != 0 && strcmp(طریقہ, "memcmp") != 0) {
		طریقہ = "memcmp";
		rc = ul_fileeq_init(&eq, طریقہ);
	}
	اگر (rc < 0)
		err(EXIT_FAILURE, "failed to initialize files comparior");

	ul_fileeq_ڈیٹا_سیٹ_file(&a, file_a);
	ul_fileeq_ڈیٹا_سیٹ_file(&b, file_b);

	/* 3rd is optional */
	اگر (file_c)
		ul_fileeq_ڈیٹا_سیٹ_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	اگر (!ul_fileeq_سیٹ_سائز(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	لکھو("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	اگر (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		لکھو("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		لکھو("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_ڈیٹا_deinit(&a);
	ul_fileeq_ڈیٹا_deinit(&b);

	اگر (file_c)
		ul_fileeq_ڈیٹا_deinit(&c);

	ul_fileeq_deinit(&eq);
	واپس EXIT_FAILURE;
}
#اگر_ختم /* TEST_PROGRAM_FILEEQ */
