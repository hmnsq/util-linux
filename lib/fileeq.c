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
	もし (ulfileeq_debug_mask)
		戻る;
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
	もし (!eq)
		戻る;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	もし (eq->fd_cip >= 0)
		close(eq->fd_cip);
	もし (eq->fd_api >= 0)
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

	もし ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		goto fail;
	もし (bind(eq->fd_api, (struct sockaddr *) &sa, sizeof(sa)) != 0)
		goto fail;
	もし ((eq->fd_cip = accept(eq->fd_api, NULL, 0)) < 0)
		goto fail;
	戻る 0;
fail:
	deinit_crypto_api(eq);
	戻る -1;
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

	ために (i = 0; i < ARRAY_SIZE(ul_eq_methods); i++) {
		const struct ul_fileeq_method *m = &ul_eq_methods[i];

		もし (strcmp(m->name, method) == 0) {
			eq->method = m;
			break;
		}
	}

	もし (!eq->method)
		戻る -1;
#ifdef USE_FILEEQ_CRYPTOAPI
	もし (eq->method->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		戻る -1;
#endif
	戻る 0;
}

static void reset_fileeq_bufs(struct ul_fileeq *eq)
{
	free(eq->buf_a);
	free(eq->buf_b);
	eq->buf_last = eq->buf_a = eq->buf_b = NULL;
}

void ul_fileeq_deinit(struct ul_fileeq *eq)
{
	もし (!eq)
		戻る;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#ifdef USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#endif
	reset_fileeq_bufs(eq);
}

void ul_fileeq_data_close_file(struct ul_fileeq_data *data)
{
	assert(data);

	もし (data->fd >= 0) {
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
	戻る data->name != NULL;
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

	もし (filesiz != 0 && readsiz > filesiz)
		readsiz = filesiz;

	switch (eq->method->id) {
	case UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		break;
	default:
		digsiz = eq->method->digsiz;
		もし (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		もし (maxdigs == 0)
			maxdigs = 1;
		否则 もし (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		もし (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			もし (ceiling / maxdigs > SIZE_MAX)
				戻る false;
			readsiz = ceiling / maxdigs;
		}
		break;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	戻る true;
}

static unsigned char *get_buffer(struct ul_fileeq *eq)
{
	もし (!eq->buf_a)
		eq->buf_a = malloc(eq->readsiz);
	もし (!eq->buf_b)
		eq->buf_b = malloc(eq->readsiz);

	もし (!eq->buf_a || !eq->buf_b)
		戻る NULL;

	もし (eq->buf_last == eq->buf_b)
		eq->buf_last = eq->buf_a;
	否则
		eq->buf_last = eq->buf_b;

	戻る eq->buf_last;
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


	もし (data->fd < 0) {
		DBG_OBJ(DATA, data, ul_debug("open: %s", data->name));
		data->fd = open(data->name, O_RDONLY | O_CLOEXEC);
		もし (data->fd < 0)
			戻る data->fd;

#もし defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_result( posix_fadvise(data->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#endif
		もし (o) {
			DBG_OBJ(DATA, data, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(data->fd, o, SEEK_SET);
		}
	}

	もし (off)
		*off = o;

	戻る data->fd;
}

static void memcmp_reset(struct ul_fileeq *eq, struct ul_fileeq_data *data)
{
	/* only intro[] is cached */
	もし (data->nblocks)
		data->nblocks = 1;
	/* reset file position */
	もし (data->fd >= 0)
		lseek(data->fd, get_cached_offset(eq, data), SEEK_SET);
	data->is_eof = 0;
}

static ssize_t read_block(struct ul_fileeq *eq, struct ul_fileeq_data *data,
				uint64_t n, unsigned char **block)
{
	整数 fd;
	off_t off = 0;
	ssize_t rsz;

	もし (data->is_eof)
		戻る 0;

	もし (n >= eq->blocksmax)
		戻る -EINVAL;

	fd = get_fd(eq, data, &off);
	もし (fd < 0)
		戻る fd;

	DBG_OBJ(DATA, data, ul_debug(" read block off=%ju", (uintmax_t) off));

	*block = get_buffer(eq);
	もし (!*block)
		戻る -ENOMEM;

	rsz = ul_read_all(data->fd, (char *) *block, eq->readsiz);
	もし (rsz < 0) {
		DBG_OBJ(DATA, data, ul_debug("  read failed"));
		戻る rsz;
	}
	off += rsz;
	data->nblocks++;

	もし (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}

	DBG_OBJ(DATA, data, ul_debug("  read sz=%zd", rsz));
	戻る rsz;
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
	もし (n < get_cached_nblocks(data)) {
		DBG_OBJ(DATA, data, ul_debug(" digest cached"));
		assert(data->blocks);
		*block = data->blocks + (n * eq->method->digsiz);
		戻る eq->method->digsiz;
	}

	もし (data->is_eof) {
		DBG_OBJ(DATA, data, ul_debug(" file EOF"));
		戻る 0;
	}

	もし (n >= eq->blocksmax)
		戻る -EINVAL;

	/* read new block */
	fd = get_fd(eq, data, &off);
	もし (fd < 0)
		戻る fd;

	DBG_OBJ(DATA, data, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->method->digsiz;

	もし (!data->blocks) {
		DBG_OBJ(DATA, data, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		data->blocks = malloc(eq->blocksmax * sz);
		もし (!data->blocks)
			戻る -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, data->fd, NULL, eq->readsiz);
	DBG_OBJ(DATA, data, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	もし (rsz < 0)
		戻る rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*block = data->blocks + (n * eq->method->digsiz);
	rsz = ul_read_all(eq->fd_cip, (char *) *block, sz);

	もし (rsz > 0)
		data->nblocks++;
	もし (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}
	DBG_OBJ(DATA, data, ul_debug("  get %zdB digest", rsz));
	戻る rsz;
}
#endif

static ssize_t get_intro(struct ul_fileeq *eq, struct ul_fileeq_data *data,
				unsigned char **block)
{
	もし (data->nblocks == 0) {
		整数 fd = get_fd(eq, data, NULL);
		ssize_t rsz;

		もし (fd < 0)
			戻る -1;
		rsz = ul_read_all(fd, (char *) data->intro, sizeof(data->intro));
		DBG_OBJ(DATA, data, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, sizeof(data->intro)));
		もし (rsz < 0)
			戻る -1;
		data->nblocks = 1;
	}

	DBG_OBJ(DATA, data, ul_debug(" return intro"));
	*block = data->intro;
	戻る sizeof(data->intro);
}

static ssize_t get_cmp_data(struct ul_fileeq *eq, struct ul_fileeq_data *data,
				uint64_t blockno, unsigned char **block)
{
	もし (blockno == 0)
		戻る get_intro(eq, data, block);

	blockno--;

	switch (eq->method->id) {
	case UL_FILEEQ_MEMCMP:
		戻る read_block(eq, data, blockno, block);
	default:
		break;
	}
#ifdef USE_FILEEQ_CRYPTOAPI
	戻る get_digest(eq, data, blockno, block);
#否则
	戻る -1;
#endif
}

#define CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

整数 ul_fileeq(struct ul_fileeq *eq,
	      struct ul_fileeq_data *a, struct ul_fileeq_data *b)
{
	整数 cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	もし (eq->method->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	do {
		unsigned char *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_data(eq, a, n, &da);
		もし (ca < 0)
			goto done;
		cb = get_cmp_data(eq, b, n, &db);
		もし (cb < 0)
			goto done;
		もし (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			break;

		}
		cmp = memcmp(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} ながら (cmp == 0);

	もし (cmp == 0) {
		もし (!a->is_eof || !b->is_eof)
			goto done; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		戻る 1;
	}
done:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	戻る 0;
}

#ifdef TEST_PROGRAM_FILEEQ
# include <getopt.h>

整数 メインページ(整数 argc, char *argv[])
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

	ながら ((ch = getopt_long(argc, argv, "m:", longopts, NULL)) != -1) {
		switch (ch) {
		case 'm':
			method = optarg;
			break;
		case 'h':
			printf("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				program_invocation_short_name);
			戻る EXIT_FAILURE;
		}
	}

	もし (optind < argc)
		file_a = argv[optind++];
	もし (optind < argc)
		file_b = argv[optind++];
	もし (optind < argc)
		file_c = argv[optind++];

	もし (!file_a || !file_b)
		errx(EXIT_FAILURE, "no files specified, see --help");

	もし (stat(file_a, &st_a) != 0 || !S_ISREG(st_a.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_a);
	もし (stat(file_b, &st_b) != 0 || !S_ISREG(st_b.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_b);
	もし (file_c && (stat(file_c, &st_c) != 0 || !S_ISREG(st_c.st_mode)))
		errx(EXIT_FAILURE, "%s: wrong file", file_c);


	もし (st_a.st_size != st_b.st_size ||
	    (file_c && st_a.st_size != st_c.st_size))
		errx(EXIT_FAILURE, "size of the files does not match");


	rc = ul_fileeq_init(&eq, method);
	もし (rc != 0 && strcmp(method, "memcmp") != 0) {
		method = "memcmp";
		rc = ul_fileeq_init(&eq, method);
	}
	もし (rc < 0)
		err(EXIT_FAILURE, "failed to initialize files comparior");

	ul_fileeq_data_set_file(&a, file_a);
	ul_fileeq_data_set_file(&b, file_b);

	/* 3rd is optional */
	もし (file_c)
		ul_fileeq_data_set_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	もし (!ul_fileeq_set_size(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	printf("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	もし (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		printf("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		printf("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_data_deinit(&a);
	ul_fileeq_data_deinit(&b);

	もし (file_c)
		ul_fileeq_data_deinit(&c);

	ul_fileeq_deinit(&eq);
	戻る EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_FILEEQ */
