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

#ਅਨ੍ਤਰ੍ਭਾਵ <inttypes.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

/* Linux crypto */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "all-io.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "fileeq.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "debug.h"

ਸਥਿਰ UL_DEBUG_DEFINE_ਨਕਾਬ(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_ਖਾਲੀ_ਥਾਵਾਂ_MASKNAMES;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULFILEEQ_DEBUG_INIT	(1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULFILEEQ_DEBUG_ਅੰਕੜੇ	(1 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULFILEEQ_DEBUG_EQ	(1 << 4)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

ਸਥਿਰ ਖਾਲੀ ul_fileeq_init_debug(ਖਾਲੀ)
{
	ਜੇ (ulfileeq_debug_ਨਕਾਬ)
		ਵਾਪਸ;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

ਗਿਣਤੀ {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_ਚੱਕਰੀ_ਜਾਂਚ_32
};

ਬਣਤਰ ul_fileeq_ਵਿਧੀ {
	ਅਚਲ ਅੱਖਰ *name;	/* name used by applications */
	ਅਚਲ ਅੱਖਰ *kname;	/* name used by kernel crypto */
	ਪੂਰਨ id;
	ਛੋਟਾ digsiz;
};

ਸਥਿਰ ਅਚਲ ਬਣਤਰ ul_fileeq_ਵਿਧੀ ul_eq_methods[] = {
	[UL_FILEEQ_MEMCMP] = {
		.id = UL_FILEEQ_MEMCMP, .name = "memcmp"
	},
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ USE_FILEEQ_CRYPTOAPI
	[UL_FILEEQ_SHA1] = {
		.id = UL_FILEEQ_SHA1, .name = "sha1",
		.digsiz = 20, .kname = "sha1"
	},
	[UL_FILEEQ_SHA256] = {
		.id = UL_FILEEQ_SHA256, .name = "sha256",
		.digsiz = 32, .kname = "sha256"
	},

	[UL_FILEEQ_ਚੱਕਰੀ_ਜਾਂਚ_32] = {
		.id = UL_FILEEQ_ਚੱਕਰੀ_ਜਾਂਚ_32, .name = "crc32",
		.digsiz = 4, .kname = "crc32c"
	}
#ਜੇ_ਅੰਤ
};

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ USE_FILEEQ_CRYPTOAPI
ਸਥਿਰ ਖਾਲੀ deinit_crypto_api(ਬਣਤਰ ul_fileeq *eq)
{
	ਜੇ (!eq)
		ਵਾਪਸ;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	ਜੇ (eq->fd_cip >= 0)
		close(eq->fd_cip);
	ਜੇ (eq->fd_api >= 0)
		close(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

ਸਥਿਰ ਪੂਰਨ init_crypto_api(ਬਣਤਰ ul_fileeq *eq)
{
	ਬਣਤਰ sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	ਪ੍ਰਤਿਜ੍ਞਾ(eq->ਵਿਧੀ);
	ਪ੍ਰਤਿਜ੍ਞਾ(eq->ਵਿਧੀ->kname);
	ਪ੍ਰਤਿਜ੍ਞਾ(eq->fd_api == -1);
	ਪ੍ਰਤਿਜ੍ਞਾ(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->ਵਿਧੀ->kname));

	ਪ੍ਰਤਿਜ੍ਞਾ(ਆਕਾਰ(sa.salg_name) > ਲੜੀ_ਲੰਬਾਈ(eq->ਵਿਧੀ->kname) + 1);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(&sa.salg_name, eq->ਵਿਧੀ->kname, ਲੜੀ_ਲੰਬਾਈ(eq->ਵਿਧੀ->kname) + 1);

	ਜੇ ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		ਜਾਓ fail;
	ਜੇ (bind(eq->fd_api, (ਬਣਤਰ sockaddr *) &sa, ਆਕਾਰ(sa)) != 0)
		ਜਾਓ fail;
	ਜੇ ((eq->fd_cip = accept(eq->fd_api, ਨਿਰਾ, 0)) < 0)
		ਜਾਓ fail;
	ਵਾਪਸ 0;
fail:
	deinit_crypto_api(eq);
	ਵਾਪਸ -1;
}
#ਜੇ_ਅੰਤ

ਪੂਰਨ ul_fileeq_init(ਬਣਤਰ ul_fileeq *eq, ਅਚਲ ਅੱਖਰ *ਵਿਧੀ)
{
	size_t i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", ਵਿਧੀ));

	memset(eq, 0, ਆਕਾਰ(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(ul_eq_methods); i++) {
		ਅਚਲ ਬਣਤਰ ul_fileeq_ਵਿਧੀ *m = &ul_eq_methods[i];

		ਜੇ (strcmp(m->name, ਵਿਧੀ) == 0) {
			eq->ਵਿਧੀ = m;
			ਰੋਕੋ;
		}
	}

	ਜੇ (!eq->ਵਿਧੀ)
		ਵਾਪਸ -1;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ USE_FILEEQ_CRYPTOAPI
	ਜੇ (eq->ਵਿਧੀ->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		ਵਾਪਸ -1;
#ਜੇ_ਅੰਤ
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ reset_fileeq_bufs(ਬਣਤਰ ul_fileeq *eq)
{
	ਮੁਕਤ(eq->buf_a);
	ਮੁਕਤ(eq->buf_b);
	eq->buf_ਆਖਰੀ = eq->buf_a = eq->buf_b = ਨਿਰਾ;
}

ਖਾਲੀ ul_fileeq_deinit(ਬਣਤਰ ul_fileeq *eq)
{
	ਜੇ (!eq)
		ਵਾਪਸ;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#ਜੇ_ਅੰਤ
	reset_fileeq_bufs(eq);
}

ਖਾਲੀ ul_fileeq_ਅੰਕੜੇ_close_file(ਬਣਤਰ ul_fileeq_ਅੰਕੜੇ *ਅੰਕੜੇ)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਕੜੇ);

	ਜੇ (ਅੰਕੜੇ->fd >= 0) {
		DBG_OBJ(ਅੰਕੜੇ_2, ਅੰਕੜੇ, ul_debug("close"));
		close(ਅੰਕੜੇ->fd);
	}
	ਅੰਕੜੇ->fd = -1;
}

ਖਾਲੀ ul_fileeq_ਅੰਕੜੇ_init(ਬਣਤਰ ul_fileeq_ਅੰਕੜੇ *ਅੰਕੜੇ)
{
	DBG_OBJ(ਅੰਕੜੇ_2, ਅੰਕੜੇ, ul_debug("init"));
	memset(ਅੰਕੜੇ, 0, ਆਕਾਰ(*ਅੰਕੜੇ));
	ਅੰਕੜੇ->fd = -1;
}

ਖਾਲੀ ul_fileeq_ਅੰਕੜੇ_deinit(ਬਣਤਰ ul_fileeq_ਅੰਕੜੇ *ਅੰਕੜੇ)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਕੜੇ);

	DBG_OBJ(ਅੰਕੜੇ_2, ਅੰਕੜੇ, ul_debug("deinit"));
	ਮੁਕਤ(ਅੰਕੜੇ->ਖੰਡਾਂ);
	ਅੰਕੜੇ->ਖੰਡਾਂ = ਨਿਰਾ;
	ਅੰਕੜੇ->nblocks = 0;
	ਅੰਕੜੇ->maxblocks = 0;
	ਅੰਕੜੇ->is_eof = 0;
	ਅੰਕੜੇ->name = ਨਿਰਾ;

	ul_fileeq_ਅੰਕੜੇ_close_file(ਅੰਕੜੇ);
}

ਪੂਰਨ ul_fileeq_ਅੰਕੜੇ_associated(ਬਣਤਰ ul_fileeq_ਅੰਕੜੇ *ਅੰਕੜੇ)
{
	ਵਾਪਸ ਅੰਕੜੇ->name != ਨਿਰਾ;
}

ਖਾਲੀ ul_fileeq_ਅੰਕੜੇ_ਸੈੱਟ_file(ਬਣਤਰ ul_fileeq_ਅੰਕੜੇ *ਅੰਕੜੇ, ਅਚਲ ਅੱਖਰ *name)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਕੜੇ);
	ਪ੍ਰਤਿਜ੍ਞਾ(name);

	DBG_OBJ(ਅੰਕੜੇ_2, ਅੰਕੜੇ, ul_debug("set file: %s", name));
	ul_fileeq_ਅੰਕੜੇ_init(ਅੰਕੜੇ);
	ਅੰਕੜੇ->name = name;
}

bool ul_fileeq_ਸੈੱਟ_ਮਾਪ(ਬਣਤਰ ul_fileeq *eq, int64_t st_size,
			size_t readsiz, size_t memsiz)
{
	uint64_t filesiz, nreads, maxdigs;
	size_t digsiz;

	ਪ੍ਰਤਿਜ੍ਞਾ(eq);
	ਪ੍ਰਤਿਜ੍ਞਾ(st_size >= 0);
	ਪ੍ਰਤਿਜ੍ਞਾ(readsiz);

	filesiz = (uint64_t) st_size;

	eq->filesiz = filesiz;

	ਜੇ (filesiz != 0 && readsiz > filesiz)
		readsiz = filesiz;

	ਚੋਣ (eq->ਵਿਧੀ->id) {
	ਹਾਲਤ UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		ਰੋਕੋ;
	ਮੂਲ:
		digsiz = eq->ਵਿਧੀ->digsiz;
		ਜੇ (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		ਜੇ (maxdigs == 0)
			maxdigs = 1;
		ਨਹੀਂਤਾਂ ਜੇ (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		ਜੇ (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			ਜੇ (ceiling / maxdigs > SIZE_MAX)
				ਵਾਪਸ false;
			readsiz = ceiling / maxdigs;
		}
		ਰੋਕੋ;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	ਵਾਪਸ true;
}

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *get_ਬਫਰ(ਬਣਤਰ ul_fileeq *eq)
{
	ਜੇ (!eq->buf_a)
		eq->buf_a = ਸਮ੍ਰਿਤੀ_ਦਿਓ(eq->readsiz);
	ਜੇ (!eq->buf_b)
		eq->buf_b = ਸਮ੍ਰਿਤੀ_ਦਿਓ(eq->readsiz);

	ਜੇ (!eq->buf_a || !eq->buf_b)
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (eq->buf_ਆਖਰੀ == eq->buf_b)
		eq->buf_ਆਖਰੀ = eq->buf_a;
	ਨਹੀਂਤਾਂ
		eq->buf_ਆਖਰੀ = eq->buf_b;

	ਵਾਪਸ eq->buf_ਆਖਰੀ;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ get_cached_ਵਿੱਥ(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				ਆਕਾਰ((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


ਸਥਿਰ ਪੂਰਨ get_fd(ਬਣਤਰ ul_fileeq *eq, ਬਣਤਰ ul_fileeq_ਅੰਕੜੇ *ਅੰਕੜੇ, off_t *off)
{
	off_t o = get_cached_ਵਿੱਥ(eq, ਅੰਕੜੇ);

	ਪ੍ਰਤਿਜ੍ਞਾ(eq);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਕੜੇ);


	ਜੇ (ਅੰਕੜੇ->fd < 0) {
		DBG_OBJ(ਅੰਕੜੇ_2, ਅੰਕੜੇ, ul_debug("open: %s", ਅੰਕੜੇ->name));
		ਅੰਕੜੇ->fd = open(ਅੰਕੜੇ->name, O_RDONLY | O_CLOEXEC);
		ਜੇ (ਅੰਕੜੇ->fd < 0)
			ਵਾਪਸ ਅੰਕੜੇ->fd;

#ਜੇ defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_ਨਤੀਜਾ( posix_fadvise(ਅੰਕੜੇ->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#ਜੇ_ਅੰਤ
		ਜੇ (o) {
			DBG_OBJ(ਅੰਕੜੇ_2, ਅੰਕੜੇ, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(ਅੰਕੜੇ->fd, o, SEEK_SET);
		}
	}

	ਜੇ (off)
		*off = o;

	ਵਾਪਸ ਅੰਕੜੇ->fd;
}

ਸਥਿਰ ਖਾਲੀ memcmp_reset(ਬਣਤਰ ul_fileeq *eq, ਬਣਤਰ ul_fileeq_ਅੰਕੜੇ *ਅੰਕੜੇ)
{
	/* only intro[] is cached */
	ਜੇ (ਅੰਕੜੇ->nblocks)
		ਅੰਕੜੇ->nblocks = 1;
	/* reset file position */
	ਜੇ (ਅੰਕੜੇ->fd >= 0)
		lseek(ਅੰਕੜੇ->fd, get_cached_ਵਿੱਥ(eq, ਅੰਕੜੇ), SEEK_SET);
	ਅੰਕੜੇ->is_eof = 0;
}

ਸਥਿਰ ssize_t read_ਖੰਡ(ਬਣਤਰ ul_fileeq *eq, ਬਣਤਰ ul_fileeq_ਅੰਕੜੇ *ਅੰਕੜੇ,
				uint64_t n, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ **ਖੰਡ)
{
	ਪੂਰਨ fd;
	off_t off = 0;
	ssize_t rsz;

	ਜੇ (ਅੰਕੜੇ->is_eof)
		ਵਾਪਸ 0;

	ਜੇ (n >= eq->blocksmax)
		ਵਾਪਸ -EINVAL;

	fd = get_fd(eq, ਅੰਕੜੇ, &off);
	ਜੇ (fd < 0)
		ਵਾਪਸ fd;

	DBG_OBJ(ਅੰਕੜੇ_2, ਅੰਕੜੇ, ul_debug(" read block off=%ju", (uintmax_t) off));

	*ਖੰਡ = get_ਬਫਰ(eq);
	ਜੇ (!*ਖੰਡ)
		ਵਾਪਸ -ENOMEM;

	rsz = ul_read_all(ਅੰਕੜੇ->fd, (ਅੱਖਰ *) *ਖੰਡ, eq->readsiz);
	ਜੇ (rsz < 0) {
		DBG_OBJ(ਅੰਕੜੇ_2, ਅੰਕੜੇ, ul_debug("  read failed"));
		ਵਾਪਸ rsz;
	}
	off += rsz;
	ਅੰਕੜੇ->nblocks++;

	ਜੇ (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		ਅੰਕੜੇ->is_eof = 1;
		ul_fileeq_ਅੰਕੜੇ_close_file(ਅੰਕੜੇ);
	}

	DBG_OBJ(ਅੰਕੜੇ_2, ਅੰਕੜੇ, ul_debug("  read sz=%zd", rsz));
	ਵਾਪਸ rsz;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ USE_FILEEQ_CRYPTOAPI
ਸਥਿਰ ssize_t get_digest(ਬਣਤਰ ul_fileeq *eq, ਬਣਤਰ ul_fileeq_ਅੰਕੜੇ *ਅੰਕੜੇ,
				uint64_t n, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ **ਖੰਡ)
{
	off_t off = 0;
	ssize_t rsz;
	size_t sz;
	ਪੂਰਨ fd;

	/* return already cached if available */
	ਜੇ (n < get_cached_nblocks(ਅੰਕੜੇ)) {
		DBG_OBJ(ਅੰਕੜੇ_2, ਅੰਕੜੇ, ul_debug(" digest cached"));
		ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਕੜੇ->ਖੰਡਾਂ);
		*ਖੰਡ = ਅੰਕੜੇ->ਖੰਡਾਂ + (n * eq->ਵਿਧੀ->digsiz);
		ਵਾਪਸ eq->ਵਿਧੀ->digsiz;
	}

	ਜੇ (ਅੰਕੜੇ->is_eof) {
		DBG_OBJ(ਅੰਕੜੇ_2, ਅੰਕੜੇ, ul_debug(" file EOF"));
		ਵਾਪਸ 0;
	}

	ਜੇ (n >= eq->blocksmax)
		ਵਾਪਸ -EINVAL;

	/* read new block */
	fd = get_fd(eq, ਅੰਕੜੇ, &off);
	ਜੇ (fd < 0)
		ਵਾਪਸ fd;

	DBG_OBJ(ਅੰਕੜੇ_2, ਅੰਕੜੇ, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->ਵਿਧੀ->digsiz;

	ਜੇ (!ਅੰਕੜੇ->ਖੰਡਾਂ) {
		DBG_OBJ(ਅੰਕੜੇ_2, ਅੰਕੜੇ, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		ਅੰਕੜੇ->ਖੰਡਾਂ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(eq->blocksmax * sz);
		ਜੇ (!ਅੰਕੜੇ->ਖੰਡਾਂ)
			ਵਾਪਸ -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, ਅੰਕੜੇ->fd, ਨਿਰਾ, eq->readsiz);
	DBG_OBJ(ਅੰਕੜੇ_2, ਅੰਕੜੇ, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	ਜੇ (rsz < 0)
		ਵਾਪਸ rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*ਖੰਡ = ਅੰਕੜੇ->ਖੰਡਾਂ + (n * eq->ਵਿਧੀ->digsiz);
	rsz = ul_read_all(eq->fd_cip, (ਅੱਖਰ *) *ਖੰਡ, sz);

	ਜੇ (rsz > 0)
		ਅੰਕੜੇ->nblocks++;
	ਜੇ (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		ਅੰਕੜੇ->is_eof = 1;
		ul_fileeq_ਅੰਕੜੇ_close_file(ਅੰਕੜੇ);
	}
	DBG_OBJ(ਅੰਕੜੇ_2, ਅੰਕੜੇ, ul_debug("  get %zdB digest", rsz));
	ਵਾਪਸ rsz;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ssize_t get_intro(ਬਣਤਰ ul_fileeq *eq, ਬਣਤਰ ul_fileeq_ਅੰਕੜੇ *ਅੰਕੜੇ,
				ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ **ਖੰਡ)
{
	ਜੇ (ਅੰਕੜੇ->nblocks == 0) {
		ਪੂਰਨ fd = get_fd(eq, ਅੰਕੜੇ, ਨਿਰਾ);
		ssize_t rsz;

		ਜੇ (fd < 0)
			ਵਾਪਸ -1;
		rsz = ul_read_all(fd, (ਅੱਖਰ *) ਅੰਕੜੇ->intro, ਆਕਾਰ(ਅੰਕੜੇ->intro));
		DBG_OBJ(ਅੰਕੜੇ_2, ਅੰਕੜੇ, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, ਆਕਾਰ(ਅੰਕੜੇ->intro)));
		ਜੇ (rsz < 0)
			ਵਾਪਸ -1;
		ਅੰਕੜੇ->nblocks = 1;
	}

	DBG_OBJ(ਅੰਕੜੇ_2, ਅੰਕੜੇ, ul_debug(" return intro"));
	*ਖੰਡ = ਅੰਕੜੇ->intro;
	ਵਾਪਸ ਆਕਾਰ(ਅੰਕੜੇ->intro);
}

ਸਥਿਰ ssize_t get_cmp_ਅੰਕੜੇ(ਬਣਤਰ ul_fileeq *eq, ਬਣਤਰ ul_fileeq_ਅੰਕੜੇ *ਅੰਕੜੇ,
				uint64_t blockno, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ **ਖੰਡ)
{
	ਜੇ (blockno == 0)
		ਵਾਪਸ get_intro(eq, ਅੰਕੜੇ, ਖੰਡ);

	blockno--;

	ਚੋਣ (eq->ਵਿਧੀ->id) {
	ਹਾਲਤ UL_FILEEQ_MEMCMP:
		ਵਾਪਸ read_ਖੰਡ(eq, ਅੰਕੜੇ, blockno, ਖੰਡ);
	ਮੂਲ:
		ਰੋਕੋ;
	}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ USE_FILEEQ_CRYPTOAPI
	ਵਾਪਸ get_digest(eq, ਅੰਕੜੇ, blockno, ਖੰਡ);
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ -1;
#ਜੇ_ਅੰਤ
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

ਪੂਰਨ ul_fileeq(ਬਣਤਰ ul_fileeq *eq,
	      ਬਣਤਰ ul_fileeq_ਅੰਕੜੇ *a, ਬਣਤਰ ul_fileeq_ਅੰਕੜੇ *b)
{
	ਪੂਰਨ cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	ਜੇ (eq->ਵਿਧੀ->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	ਕਰੋ {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_ਅੰਕੜੇ(eq, a, n, &da);
		ਜੇ (ca < 0)
			ਜਾਓ ਪੂਰਾ;
		cb = get_cmp_ਅੰਕੜੇ(eq, b, n, &db);
		ਜੇ (cb < 0)
			ਜਾਓ ਪੂਰਾ;
		ਜੇ (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			ਰੋਕੋ;

		}
		cmp = memcmp(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} ਜਦੋਂ (cmp == 0);

	ਜੇ (cmp == 0) {
		ਜੇ (!a->is_eof || !b->is_eof)
			ਜਾਓ ਪੂਰਾ; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		ਵਾਪਸ 1;
	}
ਪੂਰਾ:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_FILEEQ
# include <getopt.h>

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਬਣਤਰ ul_fileeq eq;
	ਬਣਤਰ ul_fileeq_ਅੰਕੜੇ a, b, c;
	ਅਚਲ ਅੱਖਰ *ਵਿਧੀ = "sha1";
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ option longopts[] = {
		{ "method", required_argument, ਨਿਰਾ, 'm' },
		{ "help", no_argument, ਨਿਰਾ, 'h' },
		{ ਨਿਰਾ, 0, ਨਿਰਾ, 0 }
	};
	ਪੂਰਨ ch, rc;
	ਅਚਲ ਅੱਖਰ *file_a = ਨਿਰਾ, *file_b = ਨਿਰਾ, *file_c = ਨਿਰਾ;
	ਬਣਤਰ stat st_a, st_b, st_c;

	ਜਦੋਂ ((ch = getopt_long(argc, argv, "m:", longopts, ਨਿਰਾ)) != -1) {
		ਚੋਣ (ch) {
		ਹਾਲਤ 'm':
			ਵਿਧੀ = optarg;
			ਰੋਕੋ;
		ਹਾਲਤ 'h':
			ਲਿਖੋ("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				ਪ੍ਰੋਗਰਾਮ_invocation_short_name);
			ਵਾਪਸ EXIT_FAILURE;
		}
	}

	ਜੇ (optind < argc)
		file_a = argv[optind++];
	ਜੇ (optind < argc)
		file_b = argv[optind++];
	ਜੇ (optind < argc)
		file_c = argv[optind++];

	ਜੇ (!file_a || !file_b)
		errx(EXIT_FAILURE, "no files specified, see --help");

	ਜੇ (stat(file_a, &st_a) != 0 || !S_ISREG(st_a.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_a);
	ਜੇ (stat(file_b, &st_b) != 0 || !S_ISREG(st_b.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_b);
	ਜੇ (file_c && (stat(file_c, &st_c) != 0 || !S_ISREG(st_c.st_mode)))
		errx(EXIT_FAILURE, "%s: wrong file", file_c);


	ਜੇ (st_a.st_size != st_b.st_size ||
	    (file_c && st_a.st_size != st_c.st_size))
		errx(EXIT_FAILURE, "size of the files does not match");


	rc = ul_fileeq_init(&eq, ਵਿਧੀ);
	ਜੇ (rc != 0 && strcmp(ਵਿਧੀ, "memcmp") != 0) {
		ਵਿਧੀ = "memcmp";
		rc = ul_fileeq_init(&eq, ਵਿਧੀ);
	}
	ਜੇ (rc < 0)
		err(EXIT_FAILURE, "failed to initialize files comparior");

	ul_fileeq_ਅੰਕੜੇ_ਸੈੱਟ_file(&a, file_a);
	ul_fileeq_ਅੰਕੜੇ_ਸੈੱਟ_file(&b, file_b);

	/* 3rd is optional */
	ਜੇ (file_c)
		ul_fileeq_ਅੰਕੜੇ_ਸੈੱਟ_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	ਜੇ (!ul_fileeq_ਸੈੱਟ_ਮਾਪ(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	ਲਿਖੋ("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	ਜੇ (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		ਲਿਖੋ("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		ਲਿਖੋ("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_ਅੰਕੜੇ_deinit(&a);
	ul_fileeq_ਅੰਕੜੇ_deinit(&b);

	ਜੇ (file_c)
		ul_fileeq_ਅੰਕੜੇ_deinit(&c);

	ul_fileeq_deinit(&eq);
	ਵਾਪਸ EXIT_FAILURE;
}
#ਜੇ_ਅੰਤ /* TEST_PROGRAM_FILEEQ */
