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

#ಅನ್ತರ್ಭಾವ <inttypes.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

/* Linux crypto */
#ifdef HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#endif

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "all-io.h"
#ಅನ್ತರ್ಭಾವ "fileeq.h"
#ಅನ್ತರ್ಭಾವ "debug.h"

ಸ್ಥಿರ UL_DEBUG_DEFINE_MASK(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULFILEEQ_DEBUG_INIT	(1 << 1)
#define ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#define ULFILEEQ_DEBUG_DATA	(1 << 3)
#define ULFILEEQ_DEBUG_EQ	(1 << 4)

#define DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

ಸ್ಥಿರ ಶೂನ್ಯ ul_fileeq_init_debug(ಶೂನ್ಯ)
{
	ಯದಿ (ulfileeq_debug_mask)
		ಪ್ರತ್ಯಾಗಮ;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

ಗಣನಾ {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_CRC32
};

ಸಂರಚನಾ ul_fileeq_method {
	ಧ್ರುವ ವರ್ಣ *name;	/* name used by applications */
	ಧ್ರುವ ವರ್ಣ *kname;	/* name used by kernel crypto */
	ಪೂರ್ಣಾಙ್ಕ id;
	short digsiz;
};

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ ul_fileeq_method ul_eq_methods[] = {
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
ಸ್ಥಿರ ಶೂನ್ಯ deinit_crypto_api(ಸಂರಚನಾ ul_fileeq *eq)
{
	ಯದಿ (!eq)
		ಪ್ರತ್ಯಾಗಮ;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	ಯದಿ (eq->fd_cip >= 0)
		close(eq->fd_cip);
	ಯದಿ (eq->fd_api >= 0)
		close(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ init_crypto_api(ಸಂರಚನಾ ul_fileeq *eq)
{
	ಸಂರಚನಾ sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	ಪ್ರತಿಜ್ಞಾ(eq->method);
	ಪ್ರತಿಜ್ಞಾ(eq->method->kname);
	ಪ್ರತಿಜ್ಞಾ(eq->fd_api == -1);
	ಪ್ರತಿಜ್ಞಾ(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->method->kname));

	ಪ್ರತಿಜ್ಞಾ(ಪರಿಮಾಣ(sa.salg_name) > strlen(eq->method->kname) + 1);
	memcpy(&sa.salg_name, eq->method->kname, strlen(eq->method->kname) + 1);

	ಯದಿ ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		ಗಚ್ಛ fail;
	ಯದಿ (bind(eq->fd_api, (ಸಂರಚನಾ sockaddr *) &sa, ಪರಿಮಾಣ(sa)) != 0)
		ಗಚ್ಛ fail;
	ಯದಿ ((eq->fd_cip = accept(eq->fd_api, ರಿಕ್ತ, 0)) < 0)
		ಗಚ್ಛ fail;
	ಪ್ರತ್ಯಾಗಮ 0;
fail:
	deinit_crypto_api(eq);
	ಪ್ರತ್ಯಾಗಮ -1;
}
#endif

ಪೂರ್ಣಾಙ್ಕ ul_fileeq_init(ಸಂರಚನಾ ul_fileeq *eq, ಧ್ರುವ ವರ್ಣ *method)
{
	size_t i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", method));

	memset(eq, 0, ಪರಿಮಾಣ(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(ul_eq_methods); i++) {
		ಧ್ರುವ ಸಂರಚನಾ ul_fileeq_method *m = &ul_eq_methods[i];

		ಯದಿ (strcmp(m->name, method) == 0) {
			eq->method = m;
			ಭಙ್ಗ;
		}
	}

	ಯದಿ (!eq->method)
		ಪ್ರತ್ಯಾಗಮ -1;
#ifdef USE_FILEEQ_CRYPTOAPI
	ಯದಿ (eq->method->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		ಪ್ರತ್ಯಾಗಮ -1;
#endif
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ reset_fileeq_bufs(ಸಂರಚನಾ ul_fileeq *eq)
{
	ಮುಕ್ತ(eq->buf_a);
	ಮುಕ್ತ(eq->buf_b);
	eq->buf_last = eq->buf_a = eq->buf_b = ರಿಕ್ತ;
}

ಶೂನ್ಯ ul_fileeq_deinit(ಸಂರಚನಾ ul_fileeq *eq)
{
	ಯದಿ (!eq)
		ಪ್ರತ್ಯಾಗಮ;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#ifdef USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#endif
	reset_fileeq_bufs(eq);
}

ಶೂನ್ಯ ul_fileeq_data_close_file(ಸಂರಚನಾ ul_fileeq_data *data)
{
	ಪ್ರತಿಜ್ಞಾ(data);

	ಯದಿ (data->fd >= 0) {
		DBG_OBJ(DATA, data, ul_debug("close"));
		close(data->fd);
	}
	data->fd = -1;
}

ಶೂನ್ಯ ul_fileeq_data_init(ಸಂರಚನಾ ul_fileeq_data *data)
{
	DBG_OBJ(DATA, data, ul_debug("init"));
	memset(data, 0, ಪರಿಮಾಣ(*data));
	data->fd = -1;
}

ಶೂನ್ಯ ul_fileeq_data_deinit(ಸಂರಚನಾ ul_fileeq_data *data)
{
	ಪ್ರತಿಜ್ಞಾ(data);

	DBG_OBJ(DATA, data, ul_debug("deinit"));
	ಮುಕ್ತ(data->blocks);
	data->blocks = ರಿಕ್ತ;
	data->nblocks = 0;
	data->maxblocks = 0;
	data->is_eof = 0;
	data->name = ರಿಕ್ತ;

	ul_fileeq_data_close_file(data);
}

ಪೂರ್ಣಾಙ್ಕ ul_fileeq_data_associated(ಸಂರಚನಾ ul_fileeq_data *data)
{
	ಪ್ರತ್ಯಾಗಮ data->name != ರಿಕ್ತ;
}

ಶೂನ್ಯ ul_fileeq_data_set_file(ಸಂರಚನಾ ul_fileeq_data *data, ಧ್ರುವ ವರ್ಣ *name)
{
	ಪ್ರತಿಜ್ಞಾ(data);
	ಪ್ರತಿಜ್ಞಾ(name);

	DBG_OBJ(DATA, data, ul_debug("set file: %s", name));
	ul_fileeq_data_init(data);
	data->name = name;
}

bool ul_fileeq_set_size(ಸಂರಚನಾ ul_fileeq *eq, int64_t st_size,
			size_t readsiz, size_t memsiz)
{
	uint64_t filesiz, nreads, maxdigs;
	size_t digsiz;

	ಪ್ರತಿಜ್ಞಾ(eq);
	ಪ್ರತಿಜ್ಞಾ(st_size >= 0);
	ಪ್ರತಿಜ್ಞಾ(readsiz);

	filesiz = (uint64_t) st_size;

	eq->filesiz = filesiz;

	ಯದಿ (filesiz != 0 && readsiz > filesiz)
		readsiz = filesiz;

	ವಿಕಲ್ಪ (eq->method->id) {
	ಪಕ್ಷ UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		digsiz = eq->method->digsiz;
		ಯದಿ (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		ಯದಿ (maxdigs == 0)
			maxdigs = 1;
		ಅನ್ಯಥಾ ಯದಿ (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		ಯದಿ (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			ಯದಿ (ceiling / maxdigs > SIZE_MAX)
				ಪ್ರತ್ಯಾಗಮ false;
			readsiz = ceiling / maxdigs;
		}
		ಭಙ್ಗ;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ unsigned ವರ್ಣ *get_buffer(ಸಂರಚನಾ ul_fileeq *eq)
{
	ಯದಿ (!eq->buf_a)
		eq->buf_a = malloc(eq->readsiz);
	ಯದಿ (!eq->buf_b)
		eq->buf_b = malloc(eq->readsiz);

	ಯದಿ (!eq->buf_a || !eq->buf_b)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (eq->buf_last == eq->buf_b)
		eq->buf_last = eq->buf_a;
	ಅನ್ಯಥಾ
		eq->buf_last = eq->buf_b;

	ಪ್ರತ್ಯಾಗಮ eq->buf_last;
}

#define get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#define get_cached_offset(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				ಪರಿಮಾಣ((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ get_fd(ಸಂರಚನಾ ul_fileeq *eq, ಸಂರಚನಾ ul_fileeq_data *data, off_t *off)
{
	off_t o = get_cached_offset(eq, data);

	ಪ್ರತಿಜ್ಞಾ(eq);
	ಪ್ರತಿಜ್ಞಾ(data);


	ಯದಿ (data->fd < 0) {
		DBG_OBJ(DATA, data, ul_debug("open: %s", data->name));
		data->fd = open(data->name, O_RDONLY | O_CLOEXEC);
		ಯದಿ (data->fd < 0)
			ಪ್ರತ್ಯಾಗಮ data->fd;

#ಯದಿ defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_result( posix_fadvise(data->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#endif
		ಯದಿ (o) {
			DBG_OBJ(DATA, data, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(data->fd, o, SEEK_SET);
		}
	}

	ಯದಿ (off)
		*off = o;

	ಪ್ರತ್ಯಾಗಮ data->fd;
}

ಸ್ಥಿರ ಶೂನ್ಯ memcmp_reset(ಸಂರಚನಾ ul_fileeq *eq, ಸಂರಚನಾ ul_fileeq_data *data)
{
	/* only intro[] is cached */
	ಯದಿ (data->nblocks)
		data->nblocks = 1;
	/* reset file position */
	ಯದಿ (data->fd >= 0)
		lseek(data->fd, get_cached_offset(eq, data), SEEK_SET);
	data->is_eof = 0;
}

ಸ್ಥಿರ ssize_t read_block(ಸಂರಚನಾ ul_fileeq *eq, ಸಂರಚನಾ ul_fileeq_data *data,
				uint64_t n, unsigned ವರ್ಣ **block)
{
	ಪೂರ್ಣಾಙ್ಕ fd;
	off_t off = 0;
	ssize_t rsz;

	ಯದಿ (data->is_eof)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (n >= eq->blocksmax)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	fd = get_fd(eq, data, &off);
	ಯದಿ (fd < 0)
		ಪ್ರತ್ಯಾಗಮ fd;

	DBG_OBJ(DATA, data, ul_debug(" read block off=%ju", (uintmax_t) off));

	*block = get_buffer(eq);
	ಯದಿ (!*block)
		ಪ್ರತ್ಯಾಗಮ -ENOMEM;

	rsz = ul_read_all(data->fd, (ವರ್ಣ *) *block, eq->readsiz);
	ಯದಿ (rsz < 0) {
		DBG_OBJ(DATA, data, ul_debug("  read failed"));
		ಪ್ರತ್ಯಾಗಮ rsz;
	}
	off += rsz;
	data->nblocks++;

	ಯದಿ (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}

	DBG_OBJ(DATA, data, ul_debug("  read sz=%zd", rsz));
	ಪ್ರತ್ಯಾಗಮ rsz;
}

#ifdef USE_FILEEQ_CRYPTOAPI
ಸ್ಥಿರ ssize_t get_digest(ಸಂರಚನಾ ul_fileeq *eq, ಸಂರಚನಾ ul_fileeq_data *data,
				uint64_t n, unsigned ವರ್ಣ **block)
{
	off_t off = 0;
	ssize_t rsz;
	size_t sz;
	ಪೂರ್ಣಾಙ್ಕ fd;

	/* return already cached if available */
	ಯದಿ (n < get_cached_nblocks(data)) {
		DBG_OBJ(DATA, data, ul_debug(" digest cached"));
		ಪ್ರತಿಜ್ಞಾ(data->blocks);
		*block = data->blocks + (n * eq->method->digsiz);
		ಪ್ರತ್ಯಾಗಮ eq->method->digsiz;
	}

	ಯದಿ (data->is_eof) {
		DBG_OBJ(DATA, data, ul_debug(" file EOF"));
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಯದಿ (n >= eq->blocksmax)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	/* read new block */
	fd = get_fd(eq, data, &off);
	ಯದಿ (fd < 0)
		ಪ್ರತ್ಯಾಗಮ fd;

	DBG_OBJ(DATA, data, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->method->digsiz;

	ಯದಿ (!data->blocks) {
		DBG_OBJ(DATA, data, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		data->blocks = malloc(eq->blocksmax * sz);
		ಯದಿ (!data->blocks)
			ಪ್ರತ್ಯಾಗಮ -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, data->fd, ರಿಕ್ತ, eq->readsiz);
	DBG_OBJ(DATA, data, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	ಯದಿ (rsz < 0)
		ಪ್ರತ್ಯಾಗಮ rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*block = data->blocks + (n * eq->method->digsiz);
	rsz = ul_read_all(eq->fd_cip, (ವರ್ಣ *) *block, sz);

	ಯದಿ (rsz > 0)
		data->nblocks++;
	ಯದಿ (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}
	DBG_OBJ(DATA, data, ul_debug("  get %zdB digest", rsz));
	ಪ್ರತ್ಯಾಗಮ rsz;
}
#endif

ಸ್ಥಿರ ssize_t get_intro(ಸಂರಚನಾ ul_fileeq *eq, ಸಂರಚನಾ ul_fileeq_data *data,
				unsigned ವರ್ಣ **block)
{
	ಯದಿ (data->nblocks == 0) {
		ಪೂರ್ಣಾಙ್ಕ fd = get_fd(eq, data, ರಿಕ್ತ);
		ssize_t rsz;

		ಯದಿ (fd < 0)
			ಪ್ರತ್ಯಾಗಮ -1;
		rsz = ul_read_all(fd, (ವರ್ಣ *) data->intro, ಪರಿಮಾಣ(data->intro));
		DBG_OBJ(DATA, data, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, ಪರಿಮಾಣ(data->intro)));
		ಯದಿ (rsz < 0)
			ಪ್ರತ್ಯಾಗಮ -1;
		data->nblocks = 1;
	}

	DBG_OBJ(DATA, data, ul_debug(" return intro"));
	*block = data->intro;
	ಪ್ರತ್ಯಾಗಮ ಪರಿಮಾಣ(data->intro);
}

ಸ್ಥಿರ ssize_t get_cmp_data(ಸಂರಚನಾ ul_fileeq *eq, ಸಂರಚನಾ ul_fileeq_data *data,
				uint64_t blockno, unsigned ವರ್ಣ **block)
{
	ಯದಿ (blockno == 0)
		ಪ್ರತ್ಯಾಗಮ get_intro(eq, data, block);

	blockno--;

	ವಿಕಲ್ಪ (eq->method->id) {
	ಪಕ್ಷ UL_FILEEQ_MEMCMP:
		ಪ್ರತ್ಯಾಗಮ read_block(eq, data, blockno, block);
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}
#ifdef USE_FILEEQ_CRYPTOAPI
	ಪ್ರತ್ಯಾಗಮ get_digest(eq, data, blockno, block);
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ -1;
#endif
}

#define CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

ಪೂರ್ಣಾಙ್ಕ ul_fileeq(ಸಂರಚನಾ ul_fileeq *eq,
	      ಸಂರಚನಾ ul_fileeq_data *a, ಸಂರಚನಾ ul_fileeq_data *b)
{
	ಪೂರ್ಣಾಙ್ಕ cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	ಯದಿ (eq->method->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	ಕುರು {
		unsigned ವರ್ಣ *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_data(eq, a, n, &da);
		ಯದಿ (ca < 0)
			ಗಚ್ಛ done;
		cb = get_cmp_data(eq, b, n, &db);
		ಯದಿ (cb < 0)
			ಗಚ್ಛ done;
		ಯದಿ (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			ಭಙ್ಗ;

		}
		cmp = memcmp(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} ಯಾವತ್ (cmp == 0);

	ಯದಿ (cmp == 0) {
		ಯದಿ (!a->is_eof || !b->is_eof)
			ಗಚ್ಛ done; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		ಪ್ರತ್ಯಾಗಮ 1;
	}
done:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	ಪ್ರತ್ಯಾಗಮ 0;
}

#ifdef TEST_PROGRAM_FILEEQ
# include <getopt.h>

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ಸಂರಚನಾ ul_fileeq eq;
	ಸಂರಚನಾ ul_fileeq_data a, b, c;
	ಧ್ರುವ ವರ್ಣ *method = "sha1";
	ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
		{ "method", required_argument, ರಿಕ್ತ, 'm' },
		{ "help", no_argument, ರಿಕ್ತ, 'h' },
		{ ರಿಕ್ತ, 0, ರಿಕ್ತ, 0 }
	};
	ಪೂರ್ಣಾಙ್ಕ ch, rc;
	ಧ್ರುವ ವರ್ಣ *file_a = ರಿಕ್ತ, *file_b = ರಿಕ್ತ, *file_c = ರಿಕ್ತ;
	ಸಂರಚನಾ stat st_a, st_b, st_c;

	ಯಾವತ್ ((ch = getopt_long(argc, argv, "m:", longopts, ರಿಕ್ತ)) != -1) {
		ವಿಕಲ್ಪ (ch) {
		ಪಕ್ಷ 'm':
			method = optarg;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'h':
			printf("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				program_invocation_short_name);
			ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
		}
	}

	ಯದಿ (optind < argc)
		file_a = argv[optind++];
	ಯದಿ (optind < argc)
		file_b = argv[optind++];
	ಯದಿ (optind < argc)
		file_c = argv[optind++];

	ಯದಿ (!file_a || !file_b)
		errx(EXIT_FAILURE, "no files specified, see --help");

	ಯದಿ (stat(file_a, &st_a) != 0 || !S_ISREG(st_a.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_a);
	ಯದಿ (stat(file_b, &st_b) != 0 || !S_ISREG(st_b.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_b);
	ಯದಿ (file_c && (stat(file_c, &st_c) != 0 || !S_ISREG(st_c.st_mode)))
		errx(EXIT_FAILURE, "%s: wrong file", file_c);


	ಯದಿ (st_a.st_size != st_b.st_size ||
	    (file_c && st_a.st_size != st_c.st_size))
		errx(EXIT_FAILURE, "size of the files does not match");


	rc = ul_fileeq_init(&eq, method);
	ಯದಿ (rc != 0 && strcmp(method, "memcmp") != 0) {
		method = "memcmp";
		rc = ul_fileeq_init(&eq, method);
	}
	ಯದಿ (rc < 0)
		err(EXIT_FAILURE, "failed to initialize files comparior");

	ul_fileeq_data_set_file(&a, file_a);
	ul_fileeq_data_set_file(&b, file_b);

	/* 3rd is optional */
	ಯದಿ (file_c)
		ul_fileeq_data_set_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	ಯದಿ (!ul_fileeq_set_size(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	printf("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	ಯದಿ (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		printf("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		printf("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_data_deinit(&a);
	ul_fileeq_data_deinit(&b);

	ಯದಿ (file_c)
		ul_fileeq_data_deinit(&c);

	ul_fileeq_deinit(&eq);
	ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_FILEEQ */
