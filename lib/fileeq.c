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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "all-io.h"
#ಅನ್ತರ್ಭಾವ "fileeq.h"
#ಅನ್ತರ್ಭಾವ "debug.h"

ಸ್ಥಿರ UL_DEBUG_DEFINE_ಮುಖವಾಡ(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_ಖಾಲಿಗಳು_MASKNAMES;

#ವ್ಯಾಖ್ಯಾನಿಸು ULFILEEQ_DEBUG_INIT	(1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು ULFILEEQ_DEBUG_ದತ್ತಾಂಶ	(1 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು ULFILEEQ_DEBUG_EQ	(1 << 4)

#ವ್ಯಾಖ್ಯಾನಿಸು DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#ವ್ಯಾಖ್ಯಾನಿಸು DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#ವ್ಯಾಖ್ಯಾನಿಸು ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

ಸ್ಥಿರ ಶೂನ್ಯ ul_fileeq_init_debug(ಶೂನ್ಯ)
{
	ಆದರೆ (ulfileeq_debug_ಮುಖವಾಡ)
		ಹಿಂತಿರುಗಿ;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

ಎಣಿಕೆ {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32
};

ರಚನೆ ul_fileeq_ವಿಧಾನ {
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;	/* name used by applications */
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *kname;	/* name used by kernel crypto */
	ಪೂರ್ಣಾಂಕ id;
	ಗಿಡ್ಡ digsiz;
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ ul_fileeq_ವಿಧಾನ ul_eq_methods[] = {
	[UL_FILEEQ_MEMCMP] = {
		.id = UL_FILEEQ_MEMCMP, .name = "memcmp"
	},
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ USE_FILEEQ_CRYPTOAPI
	[UL_FILEEQ_SHA1] = {
		.id = UL_FILEEQ_SHA1, .name = "sha1",
		.digsiz = 20, .kname = "sha1"
	},
	[UL_FILEEQ_SHA256] = {
		.id = UL_FILEEQ_SHA256, .name = "sha256",
		.digsiz = 32, .kname = "sha256"
	},

	[UL_FILEEQ_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32] = {
		.id = UL_FILEEQ_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32, .name = "crc32",
		.digsiz = 4, .kname = "crc32c"
	}
#ಆದರೆ_ಅಂತ್ಯ
};

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ USE_FILEEQ_CRYPTOAPI
ಸ್ಥಿರ ಶೂನ್ಯ deinit_crypto_api(ರಚನೆ ul_fileeq *eq)
{
	ಆದರೆ (!eq)
		ಹಿಂತಿರುಗಿ;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	ಆದರೆ (eq->fd_cip >= 0)
		close(eq->fd_cip);
	ಆದರೆ (eq->fd_api >= 0)
		close(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ init_crypto_api(ರಚನೆ ul_fileeq *eq)
{
	ರಚನೆ sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	ಪ್ರತಿಜ್ಞಾ(eq->ವಿಧಾನ);
	ಪ್ರತಿಜ್ಞಾ(eq->ವಿಧಾನ->kname);
	ಪ್ರತಿಜ್ಞಾ(eq->fd_api == -1);
	ಪ್ರತಿಜ್ಞಾ(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->ವಿಧಾನ->kname));

	ಪ್ರತಿಜ್ಞಾ(ಗಾತ್ರ(sa.salg_name) > ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(eq->ವಿಧಾನ->kname) + 1);
	ಸ್ಮರಣೆ_ನಕಲು(&sa.salg_name, eq->ವಿಧಾನ->kname, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(eq->ವಿಧಾನ->kname) + 1);

	ಆದರೆ ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		ಹೋಗು fail;
	ಆದರೆ (bind(eq->fd_api, (ರಚನೆ sockaddr *) &sa, ಗಾತ್ರ(sa)) != 0)
		ಹೋಗು fail;
	ಆದರೆ ((eq->fd_cip = accept(eq->fd_api, ಖಾಲಿ, 0)) < 0)
		ಹೋಗು fail;
	ಹಿಂತಿರುಗಿ 0;
fail:
	deinit_crypto_api(eq);
	ಹಿಂತಿರುಗಿ -1;
}
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ ul_fileeq_init(ರಚನೆ ul_fileeq *eq, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ವಿಧಾನ)
{
	size_t i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", ವಿಧಾನ));

	memset(eq, 0, ಗಾತ್ರ(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(ul_eq_methods); i++) {
		ಸ್ಥಿರಾಂಕ ರಚನೆ ul_fileeq_ವಿಧಾನ *m = &ul_eq_methods[i];

		ಆದರೆ (strcmp(m->name, ವಿಧಾನ) == 0) {
			eq->ವಿಧಾನ = m;
			ನಿಲ್ಲಿಸು;
		}
	}

	ಆದರೆ (!eq->ವಿಧಾನ)
		ಹಿಂತಿರುಗಿ -1;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ USE_FILEEQ_CRYPTOAPI
	ಆದರೆ (eq->ವಿಧಾನ->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		ಹಿಂತಿರುಗಿ -1;
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ reset_fileeq_bufs(ರಚನೆ ul_fileeq *eq)
{
	ಬಿಡುಗಡೆ(eq->buf_a);
	ಬಿಡುಗಡೆ(eq->buf_b);
	eq->buf_ಕೊನೆಯ = eq->buf_a = eq->buf_b = ಖಾಲಿ;
}

ಶೂನ್ಯ ul_fileeq_deinit(ರಚನೆ ul_fileeq *eq)
{
	ಆದರೆ (!eq)
		ಹಿಂತಿರುಗಿ;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#ಆದರೆ_ಅಂತ್ಯ
	reset_fileeq_bufs(eq);
}

ಶೂನ್ಯ ul_fileeq_ದತ್ತಾಂಶ_close_file(ರಚನೆ ul_fileeq_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ)
{
	ಪ್ರತಿಜ್ಞಾ(ದತ್ತಾಂಶ);

	ಆದರೆ (ದತ್ತಾಂಶ->fd >= 0) {
		DBG_OBJ(ದತ್ತಾಂಶ_2, ದತ್ತಾಂಶ, ul_debug("close"));
		close(ದತ್ತಾಂಶ->fd);
	}
	ದತ್ತಾಂಶ->fd = -1;
}

ಶೂನ್ಯ ul_fileeq_ದತ್ತಾಂಶ_init(ರಚನೆ ul_fileeq_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ)
{
	DBG_OBJ(ದತ್ತಾಂಶ_2, ದತ್ತಾಂಶ, ul_debug("init"));
	memset(ದತ್ತಾಂಶ, 0, ಗಾತ್ರ(*ದತ್ತಾಂಶ));
	ದತ್ತಾಂಶ->fd = -1;
}

ಶೂನ್ಯ ul_fileeq_ದತ್ತಾಂಶ_deinit(ರಚನೆ ul_fileeq_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ)
{
	ಪ್ರತಿಜ್ಞಾ(ದತ್ತಾಂಶ);

	DBG_OBJ(ದತ್ತಾಂಶ_2, ದತ್ತಾಂಶ, ul_debug("deinit"));
	ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ->ಖಂಡಗಳು);
	ದತ್ತಾಂಶ->ಖಂಡಗಳು = ಖಾಲಿ;
	ದತ್ತಾಂಶ->nblocks = 0;
	ದತ್ತಾಂಶ->maxblocks = 0;
	ದತ್ತಾಂಶ->is_eof = 0;
	ದತ್ತಾಂಶ->name = ಖಾಲಿ;

	ul_fileeq_ದತ್ತಾಂಶ_close_file(ದತ್ತಾಂಶ);
}

ಪೂರ್ಣಾಂಕ ul_fileeq_ದತ್ತಾಂಶ_associated(ರಚನೆ ul_fileeq_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ)
{
	ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ->name != ಖಾಲಿ;
}

ಶೂನ್ಯ ul_fileeq_ದತ್ತಾಂಶ_ಗಣ_file(ರಚನೆ ul_fileeq_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಪ್ರತಿಜ್ಞಾ(ದತ್ತಾಂಶ);
	ಪ್ರತಿಜ್ಞಾ(name);

	DBG_OBJ(ದತ್ತಾಂಶ_2, ದತ್ತಾಂಶ, ul_debug("set file: %s", name));
	ul_fileeq_ದತ್ತಾಂಶ_init(ದತ್ತಾಂಶ);
	ದತ್ತಾಂಶ->name = name;
}

bool ul_fileeq_ಗಣ_ಗಾತ್ರ(ರಚನೆ ul_fileeq *eq, int64_t st_size,
			size_t readsiz, size_t memsiz)
{
	uint64_t filesiz, nreads, maxdigs;
	size_t digsiz;

	ಪ್ರತಿಜ್ಞಾ(eq);
	ಪ್ರತಿಜ್ಞಾ(st_size >= 0);
	ಪ್ರತಿಜ್ಞಾ(readsiz);

	filesiz = (uint64_t) st_size;

	eq->filesiz = filesiz;

	ಆದರೆ (filesiz != 0 && readsiz > filesiz)
		readsiz = filesiz;

	ಆಯ್ಕೆ (eq->ವಿಧಾನ->id) {
	ಸ್ಥಿತಿ UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		digsiz = eq->ವಿಧಾನ->digsiz;
		ಆದರೆ (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		ಆದರೆ (maxdigs == 0)
			maxdigs = 1;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		ಆದರೆ (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			ಆದರೆ (ceiling / maxdigs > SIZE_MAX)
				ಹಿಂತಿರುಗಿ false;
			readsiz = ceiling / maxdigs;
		}
		ನಿಲ್ಲಿಸು;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *get_ಬಫರ್(ರಚನೆ ul_fileeq *eq)
{
	ಆದರೆ (!eq->buf_a)
		eq->buf_a = ಸ್ಮರಣೆ_ಕೊಡು(eq->readsiz);
	ಆದರೆ (!eq->buf_b)
		eq->buf_b = ಸ್ಮರಣೆ_ಕೊಡು(eq->readsiz);

	ಆದರೆ (!eq->buf_a || !eq->buf_b)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (eq->buf_ಕೊನೆಯ == eq->buf_b)
		eq->buf_ಕೊನೆಯ = eq->buf_a;
	ಇಲ್ಲದಿದ್ದರೆ
		eq->buf_ಕೊನೆಯ = eq->buf_b;

	ಹಿಂತಿರುಗಿ eq->buf_ಕೊನೆಯ;
}

#ವ್ಯಾಖ್ಯಾನಿಸು get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#ವ್ಯಾಖ್ಯಾನಿಸು get_cached_ಸ್ಥಾನಾಂತರ(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				ಗಾತ್ರ((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_fd(ರಚನೆ ul_fileeq *eq, ರಚನೆ ul_fileeq_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ, off_t *off)
{
	off_t o = get_cached_ಸ್ಥಾನಾಂತರ(eq, ದತ್ತಾಂಶ);

	ಪ್ರತಿಜ್ಞಾ(eq);
	ಪ್ರತಿಜ್ಞಾ(ದತ್ತಾಂಶ);


	ಆದರೆ (ದತ್ತಾಂಶ->fd < 0) {
		DBG_OBJ(ದತ್ತಾಂಶ_2, ದತ್ತಾಂಶ, ul_debug("open: %s", ದತ್ತಾಂಶ->name));
		ದತ್ತಾಂಶ->fd = open(ದತ್ತಾಂಶ->name, O_RDONLY | O_CLOEXEC);
		ಆದರೆ (ದತ್ತಾಂಶ->fd < 0)
			ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ->fd;

#ಆದರೆ defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_ಫಲಿತಾಂಶ( posix_fadvise(ದತ್ತಾಂಶ->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#ಆದರೆ_ಅಂತ್ಯ
		ಆದರೆ (o) {
			DBG_OBJ(ದತ್ತಾಂಶ_2, ದತ್ತಾಂಶ, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(ದತ್ತಾಂಶ->fd, o, SEEK_SET);
		}
	}

	ಆದರೆ (off)
		*off = o;

	ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ->fd;
}

ಸ್ಥಿರ ಶೂನ್ಯ memcmp_reset(ರಚನೆ ul_fileeq *eq, ರಚನೆ ul_fileeq_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ)
{
	/* only intro[] is cached */
	ಆದರೆ (ದತ್ತಾಂಶ->nblocks)
		ದತ್ತಾಂಶ->nblocks = 1;
	/* reset file position */
	ಆದರೆ (ದತ್ತಾಂಶ->fd >= 0)
		lseek(ದತ್ತಾಂಶ->fd, get_cached_ಸ್ಥಾನಾಂತರ(eq, ದತ್ತಾಂಶ), SEEK_SET);
	ದತ್ತಾಂಶ->is_eof = 0;
}

ಸ್ಥಿರ ssize_t read_ಖಂಡ(ರಚನೆ ul_fileeq *eq, ರಚನೆ ul_fileeq_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
				uint64_t n, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ **ಖಂಡ)
{
	ಪೂರ್ಣಾಂಕ fd;
	off_t off = 0;
	ssize_t rsz;

	ಆದರೆ (ದತ್ತಾಂಶ->is_eof)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (n >= eq->blocksmax)
		ಹಿಂತಿರುಗಿ -EINVAL;

	fd = get_fd(eq, ದತ್ತಾಂಶ, &off);
	ಆದರೆ (fd < 0)
		ಹಿಂತಿರುಗಿ fd;

	DBG_OBJ(ದತ್ತಾಂಶ_2, ದತ್ತಾಂಶ, ul_debug(" read block off=%ju", (uintmax_t) off));

	*ಖಂಡ = get_ಬಫರ್(eq);
	ಆದರೆ (!*ಖಂಡ)
		ಹಿಂತಿರುಗಿ -ENOMEM;

	rsz = ul_read_all(ದತ್ತಾಂಶ->fd, (ಅಕ್ಷರ *) *ಖಂಡ, eq->readsiz);
	ಆದರೆ (rsz < 0) {
		DBG_OBJ(ದತ್ತಾಂಶ_2, ದತ್ತಾಂಶ, ul_debug("  read failed"));
		ಹಿಂತಿರುಗಿ rsz;
	}
	off += rsz;
	ದತ್ತಾಂಶ->nblocks++;

	ಆದರೆ (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		ದತ್ತಾಂಶ->is_eof = 1;
		ul_fileeq_ದತ್ತಾಂಶ_close_file(ದತ್ತಾಂಶ);
	}

	DBG_OBJ(ದತ್ತಾಂಶ_2, ದತ್ತಾಂಶ, ul_debug("  read sz=%zd", rsz));
	ಹಿಂತಿರುಗಿ rsz;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ USE_FILEEQ_CRYPTOAPI
ಸ್ಥಿರ ssize_t get_digest(ರಚನೆ ul_fileeq *eq, ರಚನೆ ul_fileeq_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
				uint64_t n, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ **ಖಂಡ)
{
	off_t off = 0;
	ssize_t rsz;
	size_t sz;
	ಪೂರ್ಣಾಂಕ fd;

	/* return already cached if available */
	ಆದರೆ (n < get_cached_nblocks(ದತ್ತಾಂಶ)) {
		DBG_OBJ(ದತ್ತಾಂಶ_2, ದತ್ತಾಂಶ, ul_debug(" digest cached"));
		ಪ್ರತಿಜ್ಞಾ(ದತ್ತಾಂಶ->ಖಂಡಗಳು);
		*ಖಂಡ = ದತ್ತಾಂಶ->ಖಂಡಗಳು + (n * eq->ವಿಧಾನ->digsiz);
		ಹಿಂತಿರುಗಿ eq->ವಿಧಾನ->digsiz;
	}

	ಆದರೆ (ದತ್ತಾಂಶ->is_eof) {
		DBG_OBJ(ದತ್ತಾಂಶ_2, ದತ್ತಾಂಶ, ul_debug(" file EOF"));
		ಹಿಂತಿರುಗಿ 0;
	}

	ಆದರೆ (n >= eq->blocksmax)
		ಹಿಂತಿರುಗಿ -EINVAL;

	/* read new block */
	fd = get_fd(eq, ದತ್ತಾಂಶ, &off);
	ಆದರೆ (fd < 0)
		ಹಿಂತಿರುಗಿ fd;

	DBG_OBJ(ದತ್ತಾಂಶ_2, ದತ್ತಾಂಶ, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->ವಿಧಾನ->digsiz;

	ಆದರೆ (!ದತ್ತಾಂಶ->ಖಂಡಗಳು) {
		DBG_OBJ(ದತ್ತಾಂಶ_2, ದತ್ತಾಂಶ, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		ದತ್ತಾಂಶ->ಖಂಡಗಳು = ಸ್ಮರಣೆ_ಕೊಡು(eq->blocksmax * sz);
		ಆದರೆ (!ದತ್ತಾಂಶ->ಖಂಡಗಳು)
			ಹಿಂತಿರುಗಿ -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, ದತ್ತಾಂಶ->fd, ಖಾಲಿ, eq->readsiz);
	DBG_OBJ(ದತ್ತಾಂಶ_2, ದತ್ತಾಂಶ, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	ಆದರೆ (rsz < 0)
		ಹಿಂತಿರುಗಿ rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*ಖಂಡ = ದತ್ತಾಂಶ->ಖಂಡಗಳು + (n * eq->ವಿಧಾನ->digsiz);
	rsz = ul_read_all(eq->fd_cip, (ಅಕ್ಷರ *) *ಖಂಡ, sz);

	ಆದರೆ (rsz > 0)
		ದತ್ತಾಂಶ->nblocks++;
	ಆದರೆ (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		ದತ್ತಾಂಶ->is_eof = 1;
		ul_fileeq_ದತ್ತಾಂಶ_close_file(ದತ್ತಾಂಶ);
	}
	DBG_OBJ(ದತ್ತಾಂಶ_2, ದತ್ತಾಂಶ, ul_debug("  get %zdB digest", rsz));
	ಹಿಂತಿರುಗಿ rsz;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ssize_t get_intro(ರಚನೆ ul_fileeq *eq, ರಚನೆ ul_fileeq_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
				ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ **ಖಂಡ)
{
	ಆದರೆ (ದತ್ತಾಂಶ->nblocks == 0) {
		ಪೂರ್ಣಾಂಕ fd = get_fd(eq, ದತ್ತಾಂಶ, ಖಾಲಿ);
		ssize_t rsz;

		ಆದರೆ (fd < 0)
			ಹಿಂತಿರುಗಿ -1;
		rsz = ul_read_all(fd, (ಅಕ್ಷರ *) ದತ್ತಾಂಶ->intro, ಗಾತ್ರ(ದತ್ತಾಂಶ->intro));
		DBG_OBJ(ದತ್ತಾಂಶ_2, ದತ್ತಾಂಶ, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, ಗಾತ್ರ(ದತ್ತಾಂಶ->intro)));
		ಆದರೆ (rsz < 0)
			ಹಿಂತಿರುಗಿ -1;
		ದತ್ತಾಂಶ->nblocks = 1;
	}

	DBG_OBJ(ದತ್ತಾಂಶ_2, ದತ್ತಾಂಶ, ul_debug(" return intro"));
	*ಖಂಡ = ದತ್ತಾಂಶ->intro;
	ಹಿಂತಿರುಗಿ ಗಾತ್ರ(ದತ್ತಾಂಶ->intro);
}

ಸ್ಥಿರ ssize_t get_cmp_ದತ್ತಾಂಶ(ರಚನೆ ul_fileeq *eq, ರಚನೆ ul_fileeq_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
				uint64_t blockno, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ **ಖಂಡ)
{
	ಆದರೆ (blockno == 0)
		ಹಿಂತಿರುಗಿ get_intro(eq, ದತ್ತಾಂಶ, ಖಂಡ);

	blockno--;

	ಆಯ್ಕೆ (eq->ವಿಧಾನ->id) {
	ಸ್ಥಿತಿ UL_FILEEQ_MEMCMP:
		ಹಿಂತಿರುಗಿ read_ಖಂಡ(eq, ದತ್ತಾಂಶ, blockno, ಖಂಡ);
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ USE_FILEEQ_CRYPTOAPI
	ಹಿಂತಿರುಗಿ get_digest(eq, ದತ್ತಾಂಶ, blockno, ಖಂಡ);
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ -1;
#ಆದರೆ_ಅಂತ್ಯ
}

#ವ್ಯಾಖ್ಯಾನಿಸು CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

ಪೂರ್ಣಾಂಕ ul_fileeq(ರಚನೆ ul_fileeq *eq,
	      ರಚನೆ ul_fileeq_ದತ್ತಾಂಶ *a, ರಚನೆ ul_fileeq_ದತ್ತಾಂಶ *b)
{
	ಪೂರ್ಣಾಂಕ cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	ಆದರೆ (eq->ವಿಧಾನ->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	ಮಾಡು {
		ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_ದತ್ತಾಂಶ(eq, a, n, &da);
		ಆದರೆ (ca < 0)
			ಹೋಗು ಮುಗಿದ;
		cb = get_cmp_ದತ್ತಾಂಶ(eq, b, n, &db);
		ಆದರೆ (cb < 0)
			ಹೋಗು ಮುಗಿದ;
		ಆದರೆ (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			ನಿಲ್ಲಿಸು;

		}
		cmp = memcmp(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} ಆಗ (cmp == 0);

	ಆದರೆ (cmp == 0) {
		ಆದರೆ (!a->is_eof || !b->is_eof)
			ಹೋಗು ಮುಗಿದ; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		ಹಿಂತಿರುಗಿ 1;
	}
ಮುಗಿದ:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_FILEEQ
# include <getopt.h>

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ರಚನೆ ul_fileeq eq;
	ರಚನೆ ul_fileeq_ದತ್ತಾಂಶ a, b, c;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ವಿಧಾನ = "sha1";
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
		{ "method", required_argument, ಖಾಲಿ, 'm' },
		{ "help", no_argument, ಖಾಲಿ, 'h' },
		{ ಖಾಲಿ, 0, ಖಾಲಿ, 0 }
	};
	ಪೂರ್ಣಾಂಕ ch, rc;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file_a = ಖಾಲಿ, *file_b = ಖಾಲಿ, *file_c = ಖಾಲಿ;
	ರಚನೆ stat st_a, st_b, st_c;

	ಆಗ ((ch = getopt_long(argc, argv, "m:", longopts, ಖಾಲಿ)) != -1) {
		ಆಯ್ಕೆ (ch) {
		ಸ್ಥಿತಿ 'm':
			ವಿಧಾನ = optarg;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'h':
			ಬರೆ("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				ಕಾರ್ಯಕ್ರಮ_invocation_short_name);
			ಹಿಂತಿರುಗಿ EXIT_FAILURE;
		}
	}

	ಆದರೆ (optind < argc)
		file_a = argv[optind++];
	ಆದರೆ (optind < argc)
		file_b = argv[optind++];
	ಆದರೆ (optind < argc)
		file_c = argv[optind++];

	ಆದರೆ (!file_a || !file_b)
		errx(EXIT_FAILURE, "no files specified, see --help");

	ಆದರೆ (stat(file_a, &st_a) != 0 || !S_ISREG(st_a.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_a);
	ಆದರೆ (stat(file_b, &st_b) != 0 || !S_ISREG(st_b.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_b);
	ಆದರೆ (file_c && (stat(file_c, &st_c) != 0 || !S_ISREG(st_c.st_mode)))
		errx(EXIT_FAILURE, "%s: wrong file", file_c);


	ಆದರೆ (st_a.st_size != st_b.st_size ||
	    (file_c && st_a.st_size != st_c.st_size))
		errx(EXIT_FAILURE, "size of the files does not match");


	rc = ul_fileeq_init(&eq, ವಿಧಾನ);
	ಆದರೆ (rc != 0 && strcmp(ವಿಧಾನ, "memcmp") != 0) {
		ವಿಧಾನ = "memcmp";
		rc = ul_fileeq_init(&eq, ವಿಧಾನ);
	}
	ಆದರೆ (rc < 0)
		err(EXIT_FAILURE, "failed to initialize files comparior");

	ul_fileeq_ದತ್ತಾಂಶ_ಗಣ_file(&a, file_a);
	ul_fileeq_ದತ್ತಾಂಶ_ಗಣ_file(&b, file_b);

	/* 3rd is optional */
	ಆದರೆ (file_c)
		ul_fileeq_ದತ್ತಾಂಶ_ಗಣ_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	ಆದರೆ (!ul_fileeq_ಗಣ_ಗಾತ್ರ(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	ಬರೆ("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	ಆದರೆ (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		ಬರೆ("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		ಬರೆ("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_ದತ್ತಾಂಶ_deinit(&a);
	ul_fileeq_ದತ್ತಾಂಶ_deinit(&b);

	ಆದರೆ (file_c)
		ul_fileeq_ದತ್ತಾಂಶ_deinit(&c);

	ul_fileeq_deinit(&eq);
	ಹಿಂತಿರುಗಿ EXIT_FAILURE;
}
#ಆದರೆ_ಅಂತ್ಯ /* TEST_PROGRAM_FILEEQ */
