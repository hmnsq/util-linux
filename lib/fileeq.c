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

#include <inttypes.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* Linux crypto */
#ifdef HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#endif

#include "c.h"
#include "all-io.h"
#include "fileeq.h"
#include "debug.h"

static UL_DEBUG_DEFINE_MASK(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULFILEEQ_DEBUG_INIT	(1 << 1)
#define ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#define ULFILEEQ_DEBUG_DATA	(1 << 3)
#define ULFILEEQ_DEBUG_EQ	(1 << 4)

#define DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

static void ul_fileeq_init_debug(void)
{
	အကယ်၍ (ulfileeq_debug_mask)
		ပြန်ပေး;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

ga_nanaa {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_CRC32
};

sa_nracanaa ul_fileeq_method {
	const var_na *name;	/* name used by applications */
	const var_na *kname;	/* name used by kernel crypto */
	ကိန်းပြည့် id;
	short digsiz;
};

static const sa_nracanaa ul_fileeq_method ul_eq_methods[] = {
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
static void deinit_crypto_api(sa_nracanaa ul_fileeq *eq)
{
	အကယ်၍ (!eq)
		ပြန်ပေး;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	အကယ်၍ (eq->fd_cip >= 0)
		close(eq->fd_cip);
	အကယ်၍ (eq->fd_api >= 0)
		close(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

static ကိန်းပြည့် init_crypto_api(sa_nracanaa ul_fileeq *eq)
{
	sa_nracanaa sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	assert(eq->method);
	assert(eq->method->kname);
	assert(eq->fd_api == -1);
	assert(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->method->kname));

	assert(parimaa_na(sa.salg_name) > strlen(eq->method->kname) + 1);
	memcpy(&sa.salg_name, eq->method->kname, strlen(eq->method->kname) + 1);

	အကယ်၍ ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		goto fail;
	အကယ်၍ (bind(eq->fd_api, (sa_nracanaa sockaddr *) &sa, parimaa_na(sa)) != 0)
		goto fail;
	အကယ်၍ ((eq->fd_cip = accept(eq->fd_api, NULL, 0)) < 0)
		goto fail;
	ပြန်ပေး 0;
fail:
	deinit_crypto_api(eq);
	ပြန်ပေး -1;
}
#endif

ကိန်းပြည့် ul_fileeq_init(sa_nracanaa ul_fileeq *eq, const var_na *method)
{
	size_t i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", method));

	memset(eq, 0, parimaa_na(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	အတွက် (i = 0; i < ARRAY_SIZE(ul_eq_methods); i++) {
		const sa_nracanaa ul_fileeq_method *m = &ul_eq_methods[i];

		အကယ်၍ (strcmp(m->name, method) == 0) {
			eq->method = m;
			bha_nga;
		}
	}

	အကယ်၍ (!eq->method)
		ပြန်ပေး -1;
#ifdef USE_FILEEQ_CRYPTOAPI
	အကယ်၍ (eq->method->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		ပြန်ပေး -1;
#endif
	ပြန်ပေး 0;
}

static void reset_fileeq_bufs(sa_nracanaa ul_fileeq *eq)
{
	free(eq->buf_a);
	free(eq->buf_b);
	eq->buf_last = eq->buf_a = eq->buf_b = NULL;
}

void ul_fileeq_deinit(sa_nracanaa ul_fileeq *eq)
{
	အကယ်၍ (!eq)
		ပြန်ပေး;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#ifdef USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#endif
	reset_fileeq_bufs(eq);
}

void ul_fileeq_data_close_file(sa_nracanaa ul_fileeq_data *data)
{
	assert(data);

	အကယ်၍ (data->fd >= 0) {
		DBG_OBJ(DATA, data, ul_debug("close"));
		close(data->fd);
	}
	data->fd = -1;
}

void ul_fileeq_data_init(sa_nracanaa ul_fileeq_data *data)
{
	DBG_OBJ(DATA, data, ul_debug("init"));
	memset(data, 0, parimaa_na(*data));
	data->fd = -1;
}

void ul_fileeq_data_deinit(sa_nracanaa ul_fileeq_data *data)
{
	assert(data);

	DBG_OBJ(DATA, data, ul_debug("deinit"));
	free(data->blocks);
	data->blocks = NULL;
	data->nblocks = 0;
	data->maxblocks = 0;
	data->is_eof = 0;
	data->name = NULL;

	ul_fileeq_data_close_file(data);
}

ကိန်းပြည့် ul_fileeq_data_associated(sa_nracanaa ul_fileeq_data *data)
{
	ပြန်ပေး data->name != NULL;
}

void ul_fileeq_data_set_file(sa_nracanaa ul_fileeq_data *data, const var_na *name)
{
	assert(data);
	assert(name);

	DBG_OBJ(DATA, data, ul_debug("set file: %s", name));
	ul_fileeq_data_init(data);
	data->name = name;
}

bool ul_fileeq_set_size(sa_nracanaa ul_fileeq *eq, int64_t st_size,
			size_t readsiz, size_t memsiz)
{
	uint64_t filesiz, nreads, maxdigs;
	size_t digsiz;

	assert(eq);
	assert(st_size >= 0);
	assert(readsiz);

	filesiz = (uint64_t) st_size;

	eq->filesiz = filesiz;

	အကယ်၍ (filesiz != 0 && readsiz > filesiz)
		readsiz = filesiz;

	switch (eq->method->id) {
	pak_sa UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		bha_nga;
	default:
		digsiz = eq->method->digsiz;
		အကယ်၍ (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		အကယ်၍ (maxdigs == 0)
			maxdigs = 1;
		မဟုတ်ပါက အကယ်၍ (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		အကယ်၍ (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			အကယ်၍ (ceiling / maxdigs > SIZE_MAX)
				ပြန်ပေး false;
			readsiz = ceiling / maxdigs;
		}
		bha_nga;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	ပြန်ပေး true;
}

static unsigned var_na *get_buffer(sa_nracanaa ul_fileeq *eq)
{
	အကယ်၍ (!eq->buf_a)
		eq->buf_a = malloc(eq->readsiz);
	အကယ်၍ (!eq->buf_b)
		eq->buf_b = malloc(eq->readsiz);

	အကယ်၍ (!eq->buf_a || !eq->buf_b)
		ပြန်ပေး NULL;

	အကယ်၍ (eq->buf_last == eq->buf_b)
		eq->buf_last = eq->buf_a;
	မဟုတ်ပါက
		eq->buf_last = eq->buf_b;

	ပြန်ပေး eq->buf_last;
}

#define get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#define get_cached_offset(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				parimaa_na((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


static ကိန်းပြည့် get_fd(sa_nracanaa ul_fileeq *eq, sa_nracanaa ul_fileeq_data *data, off_t *off)
{
	off_t o = get_cached_offset(eq, data);

	assert(eq);
	assert(data);


	အကယ်၍ (data->fd < 0) {
		DBG_OBJ(DATA, data, ul_debug("open: %s", data->name));
		data->fd = open(data->name, O_RDONLY | O_CLOEXEC);
		အကယ်၍ (data->fd < 0)
			ပြန်ပေး data->fd;

#အကယ်၍ defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_result( posix_fadvise(data->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#endif
		အကယ်၍ (o) {
			DBG_OBJ(DATA, data, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(data->fd, o, SEEK_SET);
		}
	}

	အကယ်၍ (off)
		*off = o;

	ပြန်ပေး data->fd;
}

static void memcmp_reset(sa_nracanaa ul_fileeq *eq, sa_nracanaa ul_fileeq_data *data)
{
	/* only intro[] is cached */
	အကယ်၍ (data->nblocks)
		data->nblocks = 1;
	/* reset file position */
	အကယ်၍ (data->fd >= 0)
		lseek(data->fd, get_cached_offset(eq, data), SEEK_SET);
	data->is_eof = 0;
}

static ssize_t read_block(sa_nracanaa ul_fileeq *eq, sa_nracanaa ul_fileeq_data *data,
				uint64_t n, unsigned var_na **block)
{
	ကိန်းပြည့် fd;
	off_t off = 0;
	ssize_t rsz;

	အကယ်၍ (data->is_eof)
		ပြန်ပေး 0;

	အကယ်၍ (n >= eq->blocksmax)
		ပြန်ပေး -EINVAL;

	fd = get_fd(eq, data, &off);
	အကယ်၍ (fd < 0)
		ပြန်ပေး fd;

	DBG_OBJ(DATA, data, ul_debug(" read block off=%ju", (uintmax_t) off));

	*block = get_buffer(eq);
	အကယ်၍ (!*block)
		ပြန်ပေး -ENOMEM;

	rsz = ul_read_all(data->fd, (var_na *) *block, eq->readsiz);
	အကယ်၍ (rsz < 0) {
		DBG_OBJ(DATA, data, ul_debug("  read failed"));
		ပြန်ပေး rsz;
	}
	off += rsz;
	data->nblocks++;

	အကယ်၍ (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}

	DBG_OBJ(DATA, data, ul_debug("  read sz=%zd", rsz));
	ပြန်ပေး rsz;
}

#ifdef USE_FILEEQ_CRYPTOAPI
static ssize_t get_digest(sa_nracanaa ul_fileeq *eq, sa_nracanaa ul_fileeq_data *data,
				uint64_t n, unsigned var_na **block)
{
	off_t off = 0;
	ssize_t rsz;
	size_t sz;
	ကိန်းပြည့် fd;

	/* return already cached if available */
	အကယ်၍ (n < get_cached_nblocks(data)) {
		DBG_OBJ(DATA, data, ul_debug(" digest cached"));
		assert(data->blocks);
		*block = data->blocks + (n * eq->method->digsiz);
		ပြန်ပေး eq->method->digsiz;
	}

	အကယ်၍ (data->is_eof) {
		DBG_OBJ(DATA, data, ul_debug(" file EOF"));
		ပြန်ပေး 0;
	}

	အကယ်၍ (n >= eq->blocksmax)
		ပြန်ပေး -EINVAL;

	/* read new block */
	fd = get_fd(eq, data, &off);
	အကယ်၍ (fd < 0)
		ပြန်ပေး fd;

	DBG_OBJ(DATA, data, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->method->digsiz;

	အကယ်၍ (!data->blocks) {
		DBG_OBJ(DATA, data, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		data->blocks = malloc(eq->blocksmax * sz);
		အကယ်၍ (!data->blocks)
			ပြန်ပေး -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, data->fd, NULL, eq->readsiz);
	DBG_OBJ(DATA, data, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	အကယ်၍ (rsz < 0)
		ပြန်ပေး rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*block = data->blocks + (n * eq->method->digsiz);
	rsz = ul_read_all(eq->fd_cip, (var_na *) *block, sz);

	အကယ်၍ (rsz > 0)
		data->nblocks++;
	အကယ်၍ (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}
	DBG_OBJ(DATA, data, ul_debug("  get %zdB digest", rsz));
	ပြန်ပေး rsz;
}
#endif

static ssize_t get_intro(sa_nracanaa ul_fileeq *eq, sa_nracanaa ul_fileeq_data *data,
				unsigned var_na **block)
{
	အကယ်၍ (data->nblocks == 0) {
		ကိန်းပြည့် fd = get_fd(eq, data, NULL);
		ssize_t rsz;

		အကယ်၍ (fd < 0)
			ပြန်ပေး -1;
		rsz = ul_read_all(fd, (var_na *) data->intro, parimaa_na(data->intro));
		DBG_OBJ(DATA, data, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, parimaa_na(data->intro)));
		အကယ်၍ (rsz < 0)
			ပြန်ပေး -1;
		data->nblocks = 1;
	}

	DBG_OBJ(DATA, data, ul_debug(" return intro"));
	*block = data->intro;
	ပြန်ပေး parimaa_na(data->intro);
}

static ssize_t get_cmp_data(sa_nracanaa ul_fileeq *eq, sa_nracanaa ul_fileeq_data *data,
				uint64_t blockno, unsigned var_na **block)
{
	အကယ်၍ (blockno == 0)
		ပြန်ပေး get_intro(eq, data, block);

	blockno--;

	switch (eq->method->id) {
	pak_sa UL_FILEEQ_MEMCMP:
		ပြန်ပေး read_block(eq, data, blockno, block);
	default:
		bha_nga;
	}
#ifdef USE_FILEEQ_CRYPTOAPI
	ပြန်ပေး get_digest(eq, data, blockno, block);
#မဟုတ်ပါက
	ပြန်ပေး -1;
#endif
}

#define CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

ကိန်းပြည့် ul_fileeq(sa_nracanaa ul_fileeq *eq,
	      sa_nracanaa ul_fileeq_data *a, sa_nracanaa ul_fileeq_data *b)
{
	ကိန်းပြည့် cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	အကယ်၍ (eq->method->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	do {
		unsigned var_na *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_data(eq, a, n, &da);
		အကယ်၍ (ca < 0)
			goto done;
		cb = get_cmp_data(eq, b, n, &db);
		အကယ်၍ (cb < 0)
			goto done;
		အကယ်၍ (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			bha_nga;

		}
		cmp = memcmp(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} နေစဉ် (cmp == 0);

	အကယ်၍ (cmp == 0) {
		အကယ်၍ (!a->is_eof || !b->is_eof)
			goto done; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		ပြန်ပေး 1;
	}
done:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	ပြန်ပေး 0;
}

#ifdef TEST_PROGRAM_FILEEQ
# include <getopt.h>

ကိန်းပြည့် main(ကိန်းပြည့် argc, var_na *argv[])
{
	sa_nracanaa ul_fileeq eq;
	sa_nracanaa ul_fileeq_data a, b, c;
	const var_na *method = "sha1";
	static const sa_nracanaa option longopts[] = {
		{ "method", required_argument, NULL, 'm' },
		{ "help", no_argument, NULL, 'h' },
		{ NULL, 0, NULL, 0 }
	};
	ကိန်းပြည့် ch, rc;
	const var_na *file_a = NULL, *file_b = NULL, *file_c = NULL;
	sa_nracanaa stat st_a, st_b, st_c;

	နေစဉ် ((ch = getopt_long(argc, argv, "m:", longopts, NULL)) != -1) {
		switch (ch) {
		pak_sa 'm':
			method = optarg;
			bha_nga;
		pak_sa 'h':
			printf("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				program_invocation_short_name);
			ပြန်ပေး EXIT_FAILURE;
		}
	}

	အကယ်၍ (optind < argc)
		file_a = argv[optind++];
	အကယ်၍ (optind < argc)
		file_b = argv[optind++];
	အကယ်၍ (optind < argc)
		file_c = argv[optind++];

	အကယ်၍ (!file_a || !file_b)
		errx(EXIT_FAILURE, "no files specified, see --help");

	အကယ်၍ (stat(file_a, &st_a) != 0 || !S_ISREG(st_a.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_a);
	အကယ်၍ (stat(file_b, &st_b) != 0 || !S_ISREG(st_b.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_b);
	အကယ်၍ (file_c && (stat(file_c, &st_c) != 0 || !S_ISREG(st_c.st_mode)))
		errx(EXIT_FAILURE, "%s: wrong file", file_c);


	အကယ်၍ (st_a.st_size != st_b.st_size ||
	    (file_c && st_a.st_size != st_c.st_size))
		errx(EXIT_FAILURE, "size of the files does not match");


	rc = ul_fileeq_init(&eq, method);
	အကယ်၍ (rc != 0 && strcmp(method, "memcmp") != 0) {
		method = "memcmp";
		rc = ul_fileeq_init(&eq, method);
	}
	အကယ်၍ (rc < 0)
		err(EXIT_FAILURE, "failed to initialize files comparior");

	ul_fileeq_data_set_file(&a, file_a);
	ul_fileeq_data_set_file(&b, file_b);

	/* 3rd is optional */
	အကယ်၍ (file_c)
		ul_fileeq_data_set_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	အကယ်၍ (!ul_fileeq_set_size(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	printf("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	အကယ်၍ (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		printf("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		printf("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_data_deinit(&a);
	ul_fileeq_data_deinit(&b);

	အကယ်၍ (file_c)
		ul_fileeq_data_deinit(&c);

	ul_fileeq_deinit(&eq);
	ပြန်ပေး EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_FILEEQ */
