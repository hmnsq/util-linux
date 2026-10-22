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
	如果 (ulfileeq_debug_mask)
		返回;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

enum {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_CRC32
};

struct ul_fileeq_method {
	const char *name;	/* name used by applications */
	const char *kname;	/* name used by kernel crypto */
	整数 id;
	short digsiz;
};

static const struct ul_fileeq_method ul_eq_methods[] = {
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
static void deinit_crypto_api(struct ul_fileeq *eq)
{
	如果 (!eq)
		返回;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	如果 (eq->fd_cip >= 0)
		close(eq->fd_cip);
	如果 (eq->fd_api >= 0)
		close(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

static 整数 init_crypto_api(struct ul_fileeq *eq)
{
	struct sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	assert(eq->method);
	assert(eq->method->kname);
	assert(eq->fd_api == -1);
	assert(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->method->kname));

	assert(sizeof(sa.salg_name) > strlen(eq->method->kname) + 1);
	memcpy(&sa.salg_name, eq->method->kname, strlen(eq->method->kname) + 1);

	如果 ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		goto fail;
	如果 (bind(eq->fd_api, (struct sockaddr *) &sa, sizeof(sa)) != 0)
		goto fail;
	如果 ((eq->fd_cip = accept(eq->fd_api, NULL, 0)) < 0)
		goto fail;
	返回 0;
fail:
	deinit_crypto_api(eq);
	返回 -1;
}
#endif

整数 ul_fileeq_init(struct ul_fileeq *eq, const char *method)
{
	size_t i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", method));

	memset(eq, 0, sizeof(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	为 (i = 0; i < ARRAY_SIZE(ul_eq_methods); i++) {
		const struct ul_fileeq_method *m = &ul_eq_methods[i];

		如果 (strcmp(m->name, method) == 0) {
			eq->method = m;
			break;
		}
	}

	如果 (!eq->method)
		返回 -1;
#ifdef USE_FILEEQ_CRYPTOAPI
	如果 (eq->method->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		返回 -1;
#endif
	返回 0;
}

static void reset_fileeq_bufs(struct ul_fileeq *eq)
{
	free(eq->buf_a);
	free(eq->buf_b);
	eq->buf_last = eq->buf_a = eq->buf_b = NULL;
}

void ul_fileeq_deinit(struct ul_fileeq *eq)
{
	如果 (!eq)
		返回;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#ifdef USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#endif
	reset_fileeq_bufs(eq);
}

void ul_fileeq_data_close_file(struct ul_fileeq_data *data)
{
	assert(data);

	如果 (data->fd >= 0) {
		DBG_OBJ(DATA, data, ul_debug("close"));
		close(data->fd);
	}
	data->fd = -1;
}

void ul_fileeq_data_init(struct ul_fileeq_data *data)
{
	DBG_OBJ(DATA, data, ul_debug("init"));
	memset(data, 0, sizeof(*data));
	data->fd = -1;
}

void ul_fileeq_data_deinit(struct ul_fileeq_data *data)
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

整数 ul_fileeq_data_associated(struct ul_fileeq_data *data)
{
	返回 data->name != NULL;
}

void ul_fileeq_data_set_file(struct ul_fileeq_data *data, const char *name)
{
	assert(data);
	assert(name);

	DBG_OBJ(DATA, data, ul_debug("set file: %s", name));
	ul_fileeq_data_init(data);
	data->name = name;
}

bool ul_fileeq_set_size(struct ul_fileeq *eq, int64_t st_size,
			size_t readsiz, size_t memsiz)
{
	uint64_t filesiz, nreads, maxdigs;
	size_t digsiz;

	assert(eq);
	assert(st_size >= 0);
	assert(readsiz);

	filesiz = (uint64_t) st_size;

	eq->filesiz = filesiz;

	如果 (filesiz != 0 && readsiz > filesiz)
		readsiz = filesiz;

	switch (eq->method->id) {
	case UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		break;
	default:
		digsiz = eq->method->digsiz;
		如果 (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		如果 (maxdigs == 0)
			maxdigs = 1;
		否则 如果 (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		如果 (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			如果 (ceiling / maxdigs > SIZE_MAX)
				返回 false;
			readsiz = ceiling / maxdigs;
		}
		break;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	返回 true;
}

static unsigned char *get_buffer(struct ul_fileeq *eq)
{
	如果 (!eq->buf_a)
		eq->buf_a = malloc(eq->readsiz);
	如果 (!eq->buf_b)
		eq->buf_b = malloc(eq->readsiz);

	如果 (!eq->buf_a || !eq->buf_b)
		返回 NULL;

	如果 (eq->buf_last == eq->buf_b)
		eq->buf_last = eq->buf_a;
	否则
		eq->buf_last = eq->buf_b;

	返回 eq->buf_last;
}

#define get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#define get_cached_offset(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				sizeof((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


static 整数 get_fd(struct ul_fileeq *eq, struct ul_fileeq_data *data, off_t *off)
{
	off_t o = get_cached_offset(eq, data);

	assert(eq);
	assert(data);


	如果 (data->fd < 0) {
		DBG_OBJ(DATA, data, ul_debug("open: %s", data->name));
		data->fd = open(data->name, O_RDONLY | O_CLOEXEC);
		如果 (data->fd < 0)
			返回 data->fd;

#如果 defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_result( posix_fadvise(data->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#endif
		如果 (o) {
			DBG_OBJ(DATA, data, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(data->fd, o, SEEK_SET);
		}
	}

	如果 (off)
		*off = o;

	返回 data->fd;
}

static void memcmp_reset(struct ul_fileeq *eq, struct ul_fileeq_data *data)
{
	/* only intro[] is cached */
	如果 (data->nblocks)
		data->nblocks = 1;
	/* reset file position */
	如果 (data->fd >= 0)
		lseek(data->fd, get_cached_offset(eq, data), SEEK_SET);
	data->is_eof = 0;
}

static ssize_t read_block(struct ul_fileeq *eq, struct ul_fileeq_data *data,
				uint64_t n, unsigned char **block)
{
	整数 fd;
	off_t off = 0;
	ssize_t rsz;

	如果 (data->is_eof)
		返回 0;

	如果 (n >= eq->blocksmax)
		返回 -EINVAL;

	fd = get_fd(eq, data, &off);
	如果 (fd < 0)
		返回 fd;

	DBG_OBJ(DATA, data, ul_debug(" read block off=%ju", (uintmax_t) off));

	*block = get_buffer(eq);
	如果 (!*block)
		返回 -ENOMEM;

	rsz = ul_read_all(data->fd, (char *) *block, eq->readsiz);
	如果 (rsz < 0) {
		DBG_OBJ(DATA, data, ul_debug("  read failed"));
		返回 rsz;
	}
	off += rsz;
	data->nblocks++;

	如果 (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}

	DBG_OBJ(DATA, data, ul_debug("  read sz=%zd", rsz));
	返回 rsz;
}

#ifdef USE_FILEEQ_CRYPTOAPI
static ssize_t get_digest(struct ul_fileeq *eq, struct ul_fileeq_data *data,
				uint64_t n, unsigned char **block)
{
	off_t off = 0;
	ssize_t rsz;
	size_t sz;
	整数 fd;

	/* return already cached if available */
	如果 (n < get_cached_nblocks(data)) {
		DBG_OBJ(DATA, data, ul_debug(" digest cached"));
		assert(data->blocks);
		*block = data->blocks + (n * eq->method->digsiz);
		返回 eq->method->digsiz;
	}

	如果 (data->is_eof) {
		DBG_OBJ(DATA, data, ul_debug(" file EOF"));
		返回 0;
	}

	如果 (n >= eq->blocksmax)
		返回 -EINVAL;

	/* read new block */
	fd = get_fd(eq, data, &off);
	如果 (fd < 0)
		返回 fd;

	DBG_OBJ(DATA, data, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->method->digsiz;

	如果 (!data->blocks) {
		DBG_OBJ(DATA, data, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		data->blocks = malloc(eq->blocksmax * sz);
		如果 (!data->blocks)
			返回 -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, data->fd, NULL, eq->readsiz);
	DBG_OBJ(DATA, data, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	如果 (rsz < 0)
		返回 rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*block = data->blocks + (n * eq->method->digsiz);
	rsz = ul_read_all(eq->fd_cip, (char *) *block, sz);

	如果 (rsz > 0)
		data->nblocks++;
	如果 (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}
	DBG_OBJ(DATA, data, ul_debug("  get %zdB digest", rsz));
	返回 rsz;
}
#endif

static ssize_t get_intro(struct ul_fileeq *eq, struct ul_fileeq_data *data,
				unsigned char **block)
{
	如果 (data->nblocks == 0) {
		整数 fd = get_fd(eq, data, NULL);
		ssize_t rsz;

		如果 (fd < 0)
			返回 -1;
		rsz = ul_read_all(fd, (char *) data->intro, sizeof(data->intro));
		DBG_OBJ(DATA, data, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, sizeof(data->intro)));
		如果 (rsz < 0)
			返回 -1;
		data->nblocks = 1;
	}

	DBG_OBJ(DATA, data, ul_debug(" return intro"));
	*block = data->intro;
	返回 sizeof(data->intro);
}

static ssize_t get_cmp_data(struct ul_fileeq *eq, struct ul_fileeq_data *data,
				uint64_t blockno, unsigned char **block)
{
	如果 (blockno == 0)
		返回 get_intro(eq, data, block);

	blockno--;

	switch (eq->method->id) {
	case UL_FILEEQ_MEMCMP:
		返回 read_block(eq, data, blockno, block);
	default:
		break;
	}
#ifdef USE_FILEEQ_CRYPTOAPI
	返回 get_digest(eq, data, blockno, block);
#否则
	返回 -1;
#endif
}

#define CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

整数 ul_fileeq(struct ul_fileeq *eq,
	      struct ul_fileeq_data *a, struct ul_fileeq_data *b)
{
	整数 cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	如果 (eq->method->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	do {
		unsigned char *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_data(eq, a, n, &da);
		如果 (ca < 0)
			goto done;
		cb = get_cmp_data(eq, b, n, &db);
		如果 (cb < 0)
			goto done;
		如果 (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			break;

		}
		cmp = memcmp(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} 当 (cmp == 0);

	如果 (cmp == 0) {
		如果 (!a->is_eof || !b->is_eof)
			goto done; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		返回 1;
	}
done:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	返回 0;
}

#ifdef TEST_PROGRAM_FILEEQ
# include <getopt.h>

整数 主要(整数 argc, char *argv[])
{
	struct ul_fileeq eq;
	struct ul_fileeq_data a, b, c;
	const char *method = "sha1";
	static const struct option longopts[] = {
		{ "method", required_argument, NULL, 'm' },
		{ "help", no_argument, NULL, 'h' },
		{ NULL, 0, NULL, 0 }
	};
	整数 ch, rc;
	const char *file_a = NULL, *file_b = NULL, *file_c = NULL;
	struct stat st_a, st_b, st_c;

	当 ((ch = getopt_long(argc, argv, "m:", longopts, NULL)) != -1) {
		switch (ch) {
		case 'm':
			method = optarg;
			break;
		case 'h':
			printf("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				program_invocation_short_name);
			返回 EXIT_FAILURE;
		}
	}

	如果 (optind < argc)
		file_a = argv[optind++];
	如果 (optind < argc)
		file_b = argv[optind++];
	如果 (optind < argc)
		file_c = argv[optind++];

	如果 (!file_a || !file_b)
		errx(EXIT_FAILURE, "no files specified, see --help");

	如果 (stat(file_a, &st_a) != 0 || !S_ISREG(st_a.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_a);
	如果 (stat(file_b, &st_b) != 0 || !S_ISREG(st_b.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_b);
	如果 (file_c && (stat(file_c, &st_c) != 0 || !S_ISREG(st_c.st_mode)))
		errx(EXIT_FAILURE, "%s: wrong file", file_c);


	如果 (st_a.st_size != st_b.st_size ||
	    (file_c && st_a.st_size != st_c.st_size))
		errx(EXIT_FAILURE, "size of the files does not match");


	rc = ul_fileeq_init(&eq, method);
	如果 (rc != 0 && strcmp(method, "memcmp") != 0) {
		method = "memcmp";
		rc = ul_fileeq_init(&eq, method);
	}
	如果 (rc < 0)
		err(EXIT_FAILURE, "failed to initialize files comparior");

	ul_fileeq_data_set_file(&a, file_a);
	ul_fileeq_data_set_file(&b, file_b);

	/* 3rd is optional */
	如果 (file_c)
		ul_fileeq_data_set_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	如果 (!ul_fileeq_set_size(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	printf("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	如果 (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		printf("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		printf("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_data_deinit(&a);
	ul_fileeq_data_deinit(&b);

	如果 (file_c)
		ul_fileeq_data_deinit(&c);

	ul_fileeq_deinit(&eq);
	返回 EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_FILEEQ */
