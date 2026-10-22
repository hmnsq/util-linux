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

#ଅନ୍ତର୍ଭାଵ <inttypes.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

/* Linux crypto */
#ifdef HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#endif

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "all-io.h"
#ଅନ୍ତର୍ଭାଵ "fileeq.h"
#ଅନ୍ତର୍ଭାଵ "debug.h"

ସ୍ଥିର UL_DEBUG_DEFINE_MASK(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULFILEEQ_DEBUG_INIT	(1 << 1)
#define ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#define ULFILEEQ_DEBUG_DATA	(1 << 3)
#define ULFILEEQ_DEBUG_EQ	(1 << 4)

#define DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

ସ୍ଥିର ଶୂନ୍ୟ ul_fileeq_init_debug(ଶୂନ୍ୟ)
{
	ଯଦି (ulfileeq_debug_mask)
		ଫେରନ୍ତୁ;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

ଗଣନା {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_CRC32
};

ସଂରଚନା ul_fileeq_method {
	ଅଚଳ ଅକ୍ଷର *name;	/* name used by applications */
	ଅଚଳ ଅକ୍ଷର *kname;	/* name used by kernel crypto */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା id;
	short digsiz;
};

ସ୍ଥିର ଅଚଳ ସଂରଚନା ul_fileeq_method ul_eq_methods[] = {
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
ସ୍ଥିର ଶୂନ୍ୟ deinit_crypto_api(ସଂରଚନା ul_fileeq *eq)
{
	ଯଦି (!eq)
		ଫେରନ୍ତୁ;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	ଯଦି (eq->fd_cip >= 0)
		close(eq->fd_cip);
	ଯଦି (eq->fd_api >= 0)
		close(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା init_crypto_api(ସଂରଚନା ul_fileeq *eq)
{
	ସଂରଚନା sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	ପ୍ରତିଜ୍ଞା(eq->method);
	ପ୍ରତିଜ୍ଞା(eq->method->kname);
	ପ୍ରତିଜ୍ଞା(eq->fd_api == -1);
	ପ୍ରତିଜ୍ଞା(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->method->kname));

	ପ୍ରତିଜ୍ଞା(ଆକାର(sa.salg_name) > strlen(eq->method->kname) + 1);
	memcpy(&sa.salg_name, eq->method->kname, strlen(eq->method->kname) + 1);

	ଯଦି ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		ଯାଅ fail;
	ଯଦି (bind(eq->fd_api, (ସଂରଚନା sockaddr *) &sa, ଆକାର(sa)) != 0)
		ଯାଅ fail;
	ଯଦି ((eq->fd_cip = accept(eq->fd_api, ଖାଲି, 0)) < 0)
		ଯାଅ fail;
	ଫେରନ୍ତୁ 0;
fail:
	deinit_crypto_api(eq);
	ଫେରନ୍ତୁ -1;
}
#endif

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_fileeq_init(ସଂରଚନା ul_fileeq *eq, ଅଚଳ ଅକ୍ଷର *method)
{
	size_t i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", method));

	memset(eq, 0, ଆକାର(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	ପାଇଁ (i = 0; i < ARRAY_SIZE(ul_eq_methods); i++) {
		ଅଚଳ ସଂରଚନା ul_fileeq_method *m = &ul_eq_methods[i];

		ଯଦି (strcmp(m->name, method) == 0) {
			eq->method = m;
			ଭାଙ୍ଗ;
		}
	}

	ଯଦି (!eq->method)
		ଫେରନ୍ତୁ -1;
#ifdef USE_FILEEQ_CRYPTOAPI
	ଯଦି (eq->method->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		ଫେରନ୍ତୁ -1;
#endif
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ reset_fileeq_bufs(ସଂରଚନା ul_fileeq *eq)
{
	ମୁକ୍ତ(eq->buf_a);
	ମୁକ୍ତ(eq->buf_b);
	eq->buf_last = eq->buf_a = eq->buf_b = ଖାଲି;
}

ଶୂନ୍ୟ ul_fileeq_deinit(ସଂରଚନା ul_fileeq *eq)
{
	ଯଦି (!eq)
		ଫେରନ୍ତୁ;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#ifdef USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#endif
	reset_fileeq_bufs(eq);
}

ଶୂନ୍ୟ ul_fileeq_data_close_file(ସଂରଚନା ul_fileeq_data *data)
{
	ପ୍ରତିଜ୍ଞା(data);

	ଯଦି (data->fd >= 0) {
		DBG_OBJ(DATA, data, ul_debug("close"));
		close(data->fd);
	}
	data->fd = -1;
}

ଶୂନ୍ୟ ul_fileeq_data_init(ସଂରଚନା ul_fileeq_data *data)
{
	DBG_OBJ(DATA, data, ul_debug("init"));
	memset(data, 0, ଆକାର(*data));
	data->fd = -1;
}

ଶୂନ୍ୟ ul_fileeq_data_deinit(ସଂରଚନା ul_fileeq_data *data)
{
	ପ୍ରତିଜ୍ଞା(data);

	DBG_OBJ(DATA, data, ul_debug("deinit"));
	ମୁକ୍ତ(data->blocks);
	data->blocks = ଖାଲି;
	data->nblocks = 0;
	data->maxblocks = 0;
	data->is_eof = 0;
	data->name = ଖାଲି;

	ul_fileeq_data_close_file(data);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_fileeq_data_associated(ସଂରଚନା ul_fileeq_data *data)
{
	ଫେରନ୍ତୁ data->name != ଖାଲି;
}

ଶୂନ୍ୟ ul_fileeq_data_set_file(ସଂରଚନା ul_fileeq_data *data, ଅଚଳ ଅକ୍ଷର *name)
{
	ପ୍ରତିଜ୍ଞା(data);
	ପ୍ରତିଜ୍ଞା(name);

	DBG_OBJ(DATA, data, ul_debug("set file: %s", name));
	ul_fileeq_data_init(data);
	data->name = name;
}

bool ul_fileeq_set_size(ସଂରଚନା ul_fileeq *eq, int64_t st_size,
			size_t readsiz, size_t memsiz)
{
	uint64_t filesiz, nreads, maxdigs;
	size_t digsiz;

	ପ୍ରତିଜ୍ଞା(eq);
	ପ୍ରତିଜ୍ଞା(st_size >= 0);
	ପ୍ରତିଜ୍ଞା(readsiz);

	filesiz = (uint64_t) st_size;

	eq->filesiz = filesiz;

	ଯଦି (filesiz != 0 && readsiz > filesiz)
		readsiz = filesiz;

	ବାଛ (eq->method->id) {
	ସ୍ଥିତି UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		digsiz = eq->method->digsiz;
		ଯଦି (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		ଯଦି (maxdigs == 0)
			maxdigs = 1;
		ଅନ୍ୟଥା ଯଦି (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		ଯଦି (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			ଯଦି (ceiling / maxdigs > SIZE_MAX)
				ଫେରନ୍ତୁ false;
			readsiz = ceiling / maxdigs;
		}
		ଭାଙ୍ଗ;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର unsigned ଅକ୍ଷର *get_buffer(ସଂରଚନା ul_fileeq *eq)
{
	ଯଦି (!eq->buf_a)
		eq->buf_a = malloc(eq->readsiz);
	ଯଦି (!eq->buf_b)
		eq->buf_b = malloc(eq->readsiz);

	ଯଦି (!eq->buf_a || !eq->buf_b)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (eq->buf_last == eq->buf_b)
		eq->buf_last = eq->buf_a;
	ଅନ୍ୟଥା
		eq->buf_last = eq->buf_b;

	ଫେରନ୍ତୁ eq->buf_last;
}

#define get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#define get_cached_offset(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				ଆକାର((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_fd(ସଂରଚନା ul_fileeq *eq, ସଂରଚନା ul_fileeq_data *data, off_t *off)
{
	off_t o = get_cached_offset(eq, data);

	ପ୍ରତିଜ୍ଞା(eq);
	ପ୍ରତିଜ୍ଞା(data);


	ଯଦି (data->fd < 0) {
		DBG_OBJ(DATA, data, ul_debug("open: %s", data->name));
		data->fd = open(data->name, O_RDONLY | O_CLOEXEC);
		ଯଦି (data->fd < 0)
			ଫେରନ୍ତୁ data->fd;

#ଯଦି defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_result( posix_fadvise(data->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#endif
		ଯଦି (o) {
			DBG_OBJ(DATA, data, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(data->fd, o, SEEK_SET);
		}
	}

	ଯଦି (off)
		*off = o;

	ଫେରନ୍ତୁ data->fd;
}

ସ୍ଥିର ଶୂନ୍ୟ memcmp_reset(ସଂରଚନା ul_fileeq *eq, ସଂରଚନା ul_fileeq_data *data)
{
	/* only intro[] is cached */
	ଯଦି (data->nblocks)
		data->nblocks = 1;
	/* reset file position */
	ଯଦି (data->fd >= 0)
		lseek(data->fd, get_cached_offset(eq, data), SEEK_SET);
	data->is_eof = 0;
}

ସ୍ଥିର ssize_t read_block(ସଂରଚନା ul_fileeq *eq, ସଂରଚନା ul_fileeq_data *data,
				uint64_t n, unsigned ଅକ୍ଷର **block)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;
	off_t off = 0;
	ssize_t rsz;

	ଯଦି (data->is_eof)
		ଫେରନ୍ତୁ 0;

	ଯଦି (n >= eq->blocksmax)
		ଫେରନ୍ତୁ -EINVAL;

	fd = get_fd(eq, data, &off);
	ଯଦି (fd < 0)
		ଫେରନ୍ତୁ fd;

	DBG_OBJ(DATA, data, ul_debug(" read block off=%ju", (uintmax_t) off));

	*block = get_buffer(eq);
	ଯଦି (!*block)
		ଫେରନ୍ତୁ -ENOMEM;

	rsz = ul_read_all(data->fd, (ଅକ୍ଷର *) *block, eq->readsiz);
	ଯଦି (rsz < 0) {
		DBG_OBJ(DATA, data, ul_debug("  read failed"));
		ଫେରନ୍ତୁ rsz;
	}
	off += rsz;
	data->nblocks++;

	ଯଦି (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}

	DBG_OBJ(DATA, data, ul_debug("  read sz=%zd", rsz));
	ଫେରନ୍ତୁ rsz;
}

#ifdef USE_FILEEQ_CRYPTOAPI
ସ୍ଥିର ssize_t get_digest(ସଂରଚନା ul_fileeq *eq, ସଂରଚନା ul_fileeq_data *data,
				uint64_t n, unsigned ଅକ୍ଷର **block)
{
	off_t off = 0;
	ssize_t rsz;
	size_t sz;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;

	/* return already cached if available */
	ଯଦି (n < get_cached_nblocks(data)) {
		DBG_OBJ(DATA, data, ul_debug(" digest cached"));
		ପ୍ରତିଜ୍ଞା(data->blocks);
		*block = data->blocks + (n * eq->method->digsiz);
		ଫେରନ୍ତୁ eq->method->digsiz;
	}

	ଯଦି (data->is_eof) {
		DBG_OBJ(DATA, data, ul_debug(" file EOF"));
		ଫେରନ୍ତୁ 0;
	}

	ଯଦି (n >= eq->blocksmax)
		ଫେରନ୍ତୁ -EINVAL;

	/* read new block */
	fd = get_fd(eq, data, &off);
	ଯଦି (fd < 0)
		ଫେରନ୍ତୁ fd;

	DBG_OBJ(DATA, data, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->method->digsiz;

	ଯଦି (!data->blocks) {
		DBG_OBJ(DATA, data, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		data->blocks = malloc(eq->blocksmax * sz);
		ଯଦି (!data->blocks)
			ଫେରନ୍ତୁ -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, data->fd, ଖାଲି, eq->readsiz);
	DBG_OBJ(DATA, data, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	ଯଦି (rsz < 0)
		ଫେରନ୍ତୁ rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*block = data->blocks + (n * eq->method->digsiz);
	rsz = ul_read_all(eq->fd_cip, (ଅକ୍ଷର *) *block, sz);

	ଯଦି (rsz > 0)
		data->nblocks++;
	ଯଦି (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}
	DBG_OBJ(DATA, data, ul_debug("  get %zdB digest", rsz));
	ଫେରନ୍ତୁ rsz;
}
#endif

ସ୍ଥିର ssize_t get_intro(ସଂରଚନା ul_fileeq *eq, ସଂରଚନା ul_fileeq_data *data,
				unsigned ଅକ୍ଷର **block)
{
	ଯଦି (data->nblocks == 0) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = get_fd(eq, data, ଖାଲି);
		ssize_t rsz;

		ଯଦି (fd < 0)
			ଫେରନ୍ତୁ -1;
		rsz = ul_read_all(fd, (ଅକ୍ଷର *) data->intro, ଆକାର(data->intro));
		DBG_OBJ(DATA, data, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, ଆକାର(data->intro)));
		ଯଦି (rsz < 0)
			ଫେରନ୍ତୁ -1;
		data->nblocks = 1;
	}

	DBG_OBJ(DATA, data, ul_debug(" return intro"));
	*block = data->intro;
	ଫେରନ୍ତୁ ଆକାର(data->intro);
}

ସ୍ଥିର ssize_t get_cmp_data(ସଂରଚନା ul_fileeq *eq, ସଂରଚନା ul_fileeq_data *data,
				uint64_t blockno, unsigned ଅକ୍ଷର **block)
{
	ଯଦି (blockno == 0)
		ଫେରନ୍ତୁ get_intro(eq, data, block);

	blockno--;

	ବାଛ (eq->method->id) {
	ସ୍ଥିତି UL_FILEEQ_MEMCMP:
		ଫେରନ୍ତୁ read_block(eq, data, blockno, block);
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}
#ifdef USE_FILEEQ_CRYPTOAPI
	ଫେରନ୍ତୁ get_digest(eq, data, blockno, block);
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ -1;
#endif
}

#define CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_fileeq(ସଂରଚନା ul_fileeq *eq,
	      ସଂରଚନା ul_fileeq_data *a, ସଂରଚନା ul_fileeq_data *b)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	ଯଦି (eq->method->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	କର {
		unsigned ଅକ୍ଷର *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_data(eq, a, n, &da);
		ଯଦି (ca < 0)
			ଯାଅ done;
		cb = get_cmp_data(eq, b, n, &db);
		ଯଦି (cb < 0)
			ଯାଅ done;
		ଯଦି (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			ଭାଙ୍ଗ;

		}
		cmp = memcmp(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} ଯେତେବେଳେ (cmp == 0);

	ଯଦି (cmp == 0) {
		ଯଦି (!a->is_eof || !b->is_eof)
			ଯାଅ done; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		ଫେରନ୍ତୁ 1;
	}
done:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	ଫେରନ୍ତୁ 0;
}

#ifdef TEST_PROGRAM_FILEEQ
# include <getopt.h>

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ସଂରଚନା ul_fileeq eq;
	ସଂରଚନା ul_fileeq_data a, b, c;
	ଅଚଳ ଅକ୍ଷର *method = "sha1";
	ସ୍ଥିର ଅଚଳ ସଂରଚନା option longopts[] = {
		{ "method", required_argument, ଖାଲି, 'm' },
		{ "help", no_argument, ଖାଲି, 'h' },
		{ ଖାଲି, 0, ଖାଲି, 0 }
	};
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch, rc;
	ଅଚଳ ଅକ୍ଷର *file_a = ଖାଲି, *file_b = ଖାଲି, *file_c = ଖାଲି;
	ସଂରଚନା stat st_a, st_b, st_c;

	ଯେତେବେଳେ ((ch = getopt_long(argc, argv, "m:", longopts, ଖାଲି)) != -1) {
		ବାଛ (ch) {
		ସ୍ଥିତି 'm':
			method = optarg;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'h':
			printf("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				program_invocation_short_name);
			ଫେରନ୍ତୁ EXIT_FAILURE;
		}
	}

	ଯଦି (optind < argc)
		file_a = argv[optind++];
	ଯଦି (optind < argc)
		file_b = argv[optind++];
	ଯଦି (optind < argc)
		file_c = argv[optind++];

	ଯଦି (!file_a || !file_b)
		errx(EXIT_FAILURE, "no files specified, see --help");

	ଯଦି (stat(file_a, &st_a) != 0 || !S_ISREG(st_a.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_a);
	ଯଦି (stat(file_b, &st_b) != 0 || !S_ISREG(st_b.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_b);
	ଯଦି (file_c && (stat(file_c, &st_c) != 0 || !S_ISREG(st_c.st_mode)))
		errx(EXIT_FAILURE, "%s: wrong file", file_c);


	ଯଦି (st_a.st_size != st_b.st_size ||
	    (file_c && st_a.st_size != st_c.st_size))
		errx(EXIT_FAILURE, "size of the files does not match");


	rc = ul_fileeq_init(&eq, method);
	ଯଦି (rc != 0 && strcmp(method, "memcmp") != 0) {
		method = "memcmp";
		rc = ul_fileeq_init(&eq, method);
	}
	ଯଦି (rc < 0)
		err(EXIT_FAILURE, "failed to initialize files comparior");

	ul_fileeq_data_set_file(&a, file_a);
	ul_fileeq_data_set_file(&b, file_b);

	/* 3rd is optional */
	ଯଦି (file_c)
		ul_fileeq_data_set_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	ଯଦି (!ul_fileeq_set_size(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	printf("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	ଯଦି (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		printf("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		printf("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_data_deinit(&a);
	ul_fileeq_data_deinit(&b);

	ଯଦି (file_c)
		ul_fileeq_data_deinit(&c);

	ul_fileeq_deinit(&eq);
	ଫେରନ୍ତୁ EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_FILEEQ */
