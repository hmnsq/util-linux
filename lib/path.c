/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2018]
 *
 *
 * Simple functions to access files. Paths can be globally prefixed to read
 * data from an alternative source (e.g. a /proc dump for regression tests).
 *
 * The paths is possible to format by printf-like way for functions with "f"
 * postfix in the name (e.g. readf, openf, ... ul_path_readf_u64()).
 *
 * The ul_path_read_* API is possible to use without path_cxt handler. In this
 * case is not possible to use global prefix and printf-like formatting.
 */
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <inttypes.h>
#ಅನ್ತರ್ಭಾವ <errno.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "fileutils.h"
#ಅನ್ತರ್ಭಾವ "all-io.h"
#ಅನ್ತರ್ಭಾವ "vfs.h"
#ಅನ್ತರ್ಭಾವ "path.h"
#ಅನ್ತರ್ಭಾವ "debug.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"

/*
 * Debug stuff (based on include/debug.h)
 */
ಸ್ಥಿರ UL_DEBUG_DEFINE_ಮುಖವಾಡ(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_ಖಾಲಿಗಳು_MASKNAMES;

#ವ್ಯಾಖ್ಯಾನಿಸು ULPATH_DEBUG_INIT	(1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು ULPATH_DEBUG_CXT	(1 << 2)

#ವ್ಯಾಖ್ಯಾನಿಸು DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#ವ್ಯಾಖ್ಯಾನಿಸು DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#ವ್ಯಾಖ್ಯಾನಿಸು ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

ಶೂನ್ಯ ul_ಮಾರ್ಗ_init_debug(ಶೂನ್ಯ)
{
	ಆದರೆ (ulpath_debug_ಮುಖವಾಡ)
		ಹಿಂತಿರುಗಿ;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

ರಚನೆ ಮಾರ್ಗ_cxt *ul_new_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dir, ...)
{
	ರಚನೆ ಮಾರ್ಗ_cxt *pc = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*pc));

	ಆದರೆ (!pc)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	ಆದರೆ (dir) {
		ಪೂರ್ಣಾಂಕ rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_ಮಾರ್ಗ, dir, ap);
		va_end(ap);

		ಆದರೆ (rc < 0 || !pc->dir_ಮಾರ್ಗ)
			ಹೋಗು fail;
	}
	ಹಿಂತಿರುಗಿ pc;
fail:
	ul_unref_ಮಾರ್ಗ(pc);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಶೂನ್ಯ ul_ref_ಮಾರ್ಗ(ರಚನೆ ಮಾರ್ಗ_cxt *pc)
{
	ಆದರೆ (pc)
		pc->refcount++;
}

ಶೂನ್ಯ ul_unref_ಮಾರ್ಗ(ರಚನೆ ಮಾರ್ಗ_cxt *pc)
{
	ಆದರೆ (!pc)
		ಹಿಂತಿರುಗಿ;

	pc->refcount--;

	ಆದರೆ (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		ಆದರೆ (pc->dialect)
			pc->free_dialect(pc);
		ul_ಮಾರ್ಗ_close_dirfd(pc);
		ಬಿಡುಗಡೆ(pc->dir_ಮಾರ್ಗ);
		ಬಿಡುಗಡೆ(pc->ಪೂರ್ವಪ್ರತ್ಯಯ);
		ಬಿಡುಗಡೆ(pc);
	}
}

ಶೂನ್ಯ ul_ಮಾರ್ಗ_refer_vfs(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ರಚನೆ ul_vfs_ops *vfs)
{
	ಆದರೆ (pc)
		pc->vfs = vfs;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_ಗಣ_ಪೂರ್ವಪ್ರತ್ಯಯ(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ)
{
	ಅಕ್ಷರ *p = ಖಾಲಿ;

	ಪ್ರತಿಜ್ಞಾ(pc->dir_fd < 0);

	ಆದರೆ (ಪೂರ್ವಪ್ರತ್ಯಯ) {
		p = strdup(ಪೂರ್ವಪ್ರತ್ಯಯ);
		ಆದರೆ (!p)
			ಹಿಂತಿರುಗಿ -ENOMEM;
	}

	ಬಿಡುಗಡೆ(pc->ಪೂರ್ವಪ್ರತ್ಯಯ);
	pc->ಪೂರ್ವಪ್ರತ್ಯಯ = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ul_ಮಾರ್ಗ_get_ಪೂರ್ವಪ್ರತ್ಯಯ(ರಚನೆ ಮಾರ್ಗ_cxt *pc)
{
	ಹಿಂತಿರುಗಿ pc ? pc->ಪೂರ್ವಪ್ರತ್ಯಯ : ಖಾಲಿ;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_ಗಣ_dir(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dir)
{
	ಅಕ್ಷರ *p = ಖಾಲಿ;

	ಆದರೆ (dir) {
		p = strdup(dir);
		ಆದರೆ (!p)
			ಹಿಂತಿರುಗಿ -ENOMEM;
	}

	ಆದರೆ (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	ಬಿಡುಗಡೆ(pc->dir_ಮಾರ್ಗ);
	pc->dir_ಮಾರ್ಗ = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ul_ಮಾರ್ಗ_get_dir(ರಚನೆ ಮಾರ್ಗ_cxt *pc)
{
	ಹಿಂತಿರುಗಿ pc ? pc->dir_ಮಾರ್ಗ : ಖಾಲಿ;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_ಗಣ_dialect(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಶೂನ್ಯ free_ದತ್ತಾಂಶ(ರಚನೆ ಮಾರ್ಗ_cxt *))
{
	pc->dialect = ದತ್ತಾಂಶ;
	pc->free_dialect = free_ದತ್ತಾಂಶ;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ *ul_ಮಾರ್ಗ_get_dialect(ರಚನೆ ಮಾರ್ಗ_cxt *pc)
{
	ಹಿಂತಿರುಗಿ pc ? pc->dialect : ಖಾಲಿ;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_ಗಣ_enoent_redirect(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಪೂರ್ಣಾಂಕ (*func)(ರಚನೆ ಮಾರ್ಗ_cxt *, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, ಪೂರ್ಣಾಂಕ *))
{
	pc->redirect_on_enoent = func;
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *get_absdir(ರಚನೆ ಮಾರ್ಗ_cxt *pc)
{
	ಪೂರ್ಣಾಂಕ rc;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dirpath;

	ಆದರೆ (!pc->ಪೂರ್ವಪ್ರತ್ಯಯ)
		ಹಿಂತಿರುಗಿ pc->dir_ಮಾರ್ಗ;

	dirpath = pc->dir_ಮಾರ್ಗ;
	ಆದರೆ (!dirpath)
		ಹಿಂತಿರುಗಿ pc->ಪೂರ್ವಪ್ರತ್ಯಯ;
	ಆದರೆ (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->ಮಾರ್ಗ_ಬಫರ್, ಗಾತ್ರ(pc->ಮಾರ್ಗ_ಬಫರ್), "%s/%s", pc->ಪೂರ್ವಪ್ರತ್ಯಯ, dirpath);
	ಆದರೆ (rc < 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಆದರೆ ((size_t)rc >= ಗಾತ್ರ(pc->ಮಾರ್ಗ_ಬಫರ್)) {
		errno = ENAMETOOLONG;
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ pc->ಮಾರ್ಗ_ಬಫರ್;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_is_accessible(ರಚನೆ ಮಾರ್ಗ_cxt *pc)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ;
	ಪ್ರತಿಜ್ಞಾ(pc);

	ಆದರೆ (pc->dir_fd >= 0)
		ಹಿಂತಿರುಗಿ 1;

	ಮಾರ್ಗ = get_absdir(pc);
	ಆದರೆ (!ಮಾರ್ಗ)
		ಹಿಂತಿರುಗಿ 0;
	ಹಿಂತಿರುಗಿ access(ಮಾರ್ಗ, F_OK) == 0;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_get_dirfd(ರಚನೆ ಮಾರ್ಗ_cxt *pc)
{
	ಪ್ರತಿಜ್ಞಾ(pc);
	ಪ್ರತಿಜ್ಞಾ(pc->dir_ಮಾರ್ಗ);

	ಆದರೆ (pc->dir_fd < 0) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ = get_absdir(pc);
		ಆದರೆ (!ಮಾರ್ಗ)
			ಹಿಂತಿರುಗಿ -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", ಮಾರ್ಗ));
		pc->dir_fd = open(ಮಾರ್ಗ, O_RDONLY|O_CLOEXEC);
	}

	ಹಿಂತಿರುಗಿ pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
ಶೂನ್ಯ ul_ಮಾರ್ಗ_close_dirfd(ರಚನೆ ಮಾರ್ಗ_cxt *pc)
{
	ಪ್ರತಿಜ್ಞಾ(pc);

	ಆದರೆ (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_isopen_dirfd(ರಚನೆ ಮಾರ್ಗ_cxt *pc)
{
	ಹಿಂತಿರುಗಿ pc && pc->dir_fd >= 0;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ul_ಮಾರ್ಗ_mkpath(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, va_list ap)
{
	ಪೂರ್ಣಾಂಕ rc;

	errno = 0;

	rc = vsnprintf(pc->ಮಾರ್ಗ_ಬಫರ್, ಗಾತ್ರ(pc->ಮಾರ್ಗ_ಬಫರ್), ಮಾರ್ಗ, ap);
	ಆದರೆ (rc < 0) {
		ಆದರೆ (!errno)
			errno = EINVAL;
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಆದರೆ ((size_t)rc >= ಗಾತ್ರ(pc->ಮಾರ್ಗ_ಬಫರ್)) {
		errno = ENAMETOOLONG;
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ pc->ಮಾರ್ಗ_ಬಫರ್;
}

ಅಕ್ಷರ *ul_ಮಾರ್ಗ_get_abspath(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ *buf, size_t bufsz, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	ಆದರೆ (ಮಾರ್ಗ) {
		ಪೂರ್ಣಾಂಕ rc;
		va_list ap;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಾಲ = ಖಾಲಿ, *dirpath = pc->dir_ಮಾರ್ಗ;

		va_start(ap, ಮಾರ್ಗ);
		ಬಾಲ = ul_ಮಾರ್ಗ_mkpath(pc, ಮಾರ್ಗ, ap);
		va_end(ap);

		ಆದರೆ (dirpath && *dirpath == '/')
			dirpath++;
		ಆದರೆ (ಬಾಲ && *ಬಾಲ == '/')
			ಬಾಲ++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->ಪೂರ್ವಪ್ರತ್ಯಯ ? pc->ಪೂರ್ವಪ್ರತ್ಯಯ : "",
				dirpath ? dirpath : "",
				ಬಾಲ ? ಬಾಲ : "");

		ಆದರೆ ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tmp = get_absdir(pc);

		ಆದರೆ (!tmp)
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		xstrncpy(buf, tmp, bufsz);
	}

	ಹಿಂತಿರುಗಿ buf;
}


ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_access(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಪೂರ್ಣಾಂಕ ಬಹುಲಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಪೂರ್ಣಾಂಕ rc;

	ಆದರೆ (!ಮಾರ್ಗ)
		ಹಿಂತಿರುಗಿ -EINVAL;
	ಆದರೆ (!pc) {
		rc = access(ಮಾರ್ಗ, ಬಹುಲಕ);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", ಮಾರ್ಗ, rc));
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಪೂರ್ಣಾಂಕ dir = ul_ಮಾರ್ಗ_get_dirfd(pc);
		ಆದರೆ (dir < 0)
			ಹಿಂತಿರುಗಿ dir;
		ಆದರೆ (*ಮಾರ್ಗ == '/')
			ಮಾರ್ಗ++;

		rc = faccessat(dir, ಮಾರ್ಗ, ಬಹುಲಕ, 0);

		ಆದರೆ (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, ಮಾರ್ಗ, &dir) == 0)
			rc = faccessat(dir, ಮಾರ್ಗ, ಬಹುಲಕ, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", ಮಾರ್ಗ, rc));
	}
	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_accessf(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಪೂರ್ಣಾಂಕ ಬಹುಲಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	va_list ap;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;

	va_start(ap, ಮಾರ್ಗ);
	p = ul_ಮಾರ್ಗ_mkpath(pc, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ !p ? -errno : ul_ಮಾರ್ಗ_access(pc, ಬಹುಲಕ, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_stat(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ರಚನೆ stat *sb, ಪೂರ್ಣಾಂಕ flags, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಪೂರ್ಣಾಂಕ rc;

	ಆದರೆ (!pc) {
		rc = ಮಾರ್ಗ ? stat(ಮಾರ್ಗ, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", ಮಾರ್ಗ, rc));
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಪೂರ್ಣಾಂಕ dir = ul_ಮಾರ್ಗ_get_dirfd(pc);
		ಆದರೆ (dir < 0)
			ಹಿಂತಿರುಗಿ dir;
		ಆದರೆ (ಮಾರ್ಗ) {
			ಆದರೆ  (*ಮಾರ್ಗ == '/')
				ಮಾರ್ಗ++;
			rc = fstatat(dir, ಮಾರ್ಗ, sb, flags);

		} ಇಲ್ಲದಿದ್ದರೆ
			rc = fstat(dir, sb);	/* dir itself */

		ಆದರೆ (rc && errno == ENOENT
		    && ಮಾರ್ಗ
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, ಮಾರ್ಗ, &dir) == 0)
			rc = fstatat(dir, ಮಾರ್ಗ, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", ಮಾರ್ಗ, rc));
	}
	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_vstatf(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ರಚನೆ stat *sb, ಪೂರ್ಣಾಂಕ flags, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, va_list ap)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = ul_ಮಾರ್ಗ_mkpath(pc, ಮಾರ್ಗ, ap);

	ಹಿಂತಿರುಗಿ !p ? -errno : ul_ಮಾರ್ಗ_stat(pc, sb, flags, p);
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_statf(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ರಚನೆ stat *sb, ಪೂರ್ಣಾಂಕ flags, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಂಕ rc;

	va_start(ap, ಮಾರ್ಗ);
	rc = ul_ಮಾರ್ಗ_vstatf(pc, sb, flags, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ rc;
}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_statx(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ರಚನೆ statx *stx, ಪೂರ್ಣಾಂಕ flags, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ,
		  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಪೂರ್ಣಾಂಕ rc;

	ಆದರೆ (!ಮಾರ್ಗ)
		ಹಿಂತಿರುಗಿ -EINVAL;
	ಆದರೆ (!pc)
		rc = ಮಾರ್ಗ ? statx(AT_FDCWD, ಮಾರ್ಗ, flags, ಮುಖವಾಡ, stx) : - EINVAL;
	ಇಲ್ಲದಿದ್ದರೆ {
		ಪೂರ್ಣಾಂಕ dir = ul_ಮಾರ್ಗ_get_dirfd(pc);
		ಆದರೆ (dir < 0)
			ಹಿಂತಿರುಗಿ dir;
		ಆದರೆ (*ಮಾರ್ಗ == '/')
			ಮಾರ್ಗ++;

		rc = statx(dir, ಮಾರ್ಗ, flags, ಮುಖವಾಡ, stx);
	}

	ಹಿಂತಿರುಗಿ rc;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_statx(ರಚನೆ ಮಾರ್ಗ_cxt *pc __attribute__((__unused__)),
		  ರಚನೆ statx *stx __attribute__((__unused__)),
		  ಪೂರ್ಣಾಂಕ flags __attribute__((__unused__)),
		  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ __attribute__((__unused__)),
		  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ __attribute__((__unused__)))
{
	errno = ENOSYS;
	ಹಿಂತಿರುಗಿ -1;
}
#ಆದರೆ_ಅಂತ್ಯ /* HAVE_STATX */

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_vstatxf(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ರಚನೆ statx *stx, ಪೂರ್ಣಾಂಕ flags, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ,
		    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, va_list ap)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = ul_ಮಾರ್ಗ_mkpath(pc, ಮಾರ್ಗ, ap);

	ಹಿಂತಿರುಗಿ !p ? -errno : ul_ಮಾರ್ಗ_statx(pc, stx, flags, ಮುಖವಾಡ, p);
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_statxf(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ರಚನೆ statx *stx, ಪೂರ್ಣಾಂಕ flags, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ,
		   ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಂಕ rc;

	va_start(ap, ಮಾರ್ಗ);
	rc = ul_ಮಾರ್ಗ_vstatxf(pc, stx, flags, ಮುಖವಾಡ, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_open(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಪೂರ್ಣಾಂಕ flags, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಪೂರ್ಣಾಂಕ fd;

	ಆದರೆ (!ಮಾರ್ಗ)
		ಹಿಂತಿರುಗಿ -EINVAL;
	ಆದರೆ (!pc) {
		fd = open(ಮಾರ್ಗ, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", ಮಾರ್ಗ));
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಪೂರ್ಣಾಂಕ fdx;
		ಪೂರ್ಣಾಂಕ dir = ul_ಮಾರ್ಗ_get_dirfd(pc);
		ಆದರೆ (dir < 0)
			ಹಿಂತಿರುಗಿ dir;

		ಆದರೆ (*ಮಾರ್ಗ == '/')
			ಮಾರ್ಗ++;

		fdx = fd = openat(dir, ಮಾರ್ಗ, flags);

		ಆದರೆ (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, ಮಾರ್ಗ, &dir) == 0)
			fd = openat(dir, ಮಾರ್ಗ, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", ಮಾರ್ಗ, fdx != fd ? " [redirected]" : ""));
	}
	ಹಿಂತಿರುಗಿ fd;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_vopenf(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಪೂರ್ಣಾಂಕ flags, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, va_list ap)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = ul_ಮಾರ್ಗ_mkpath(pc, ಮಾರ್ಗ, ap);

	ಹಿಂತಿರುಗಿ !p ? -errno : ul_ಮಾರ್ಗ_open(pc, flags, p);
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_openf(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಪೂರ್ಣಾಂಕ flags, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಂಕ rc;

	va_start(ap, ಮಾರ್ಗ);
	rc = ul_ಮಾರ್ಗ_vopenf(pc, flags, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_ಮಾರ್ಗ_fopen(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಪೂರ್ಣಾಂಕ flags = ul_ಬಹುಲಕ_ಗಮ್ಯ_flags(ಬಹುಲಕ);
	ಪೂರ್ಣಾಂಕ fd = ul_ಮಾರ್ಗ_open(pc, flags, ಮಾರ್ಗ);

	ಆದರೆ (fd < 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ ul_vfs_fdopen(pc ? pc->vfs : ಖಾಲಿ, fd, ಬಹುಲಕ);
}


FILE *ul_ಮಾರ್ಗ_vfopenf(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, va_list ap)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = ul_ಮಾರ್ಗ_mkpath(pc, ಮಾರ್ಗ, ap);

	ಹಿಂತಿರುಗಿ !p ? ಖಾಲಿ : ul_ಮಾರ್ಗ_fopen(pc, ಬಹುಲಕ, p);
}

FILE *ul_ಮಾರ್ಗ_fopenf(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, ಮಾರ್ಗ);
	f = ul_ಮಾರ್ಗ_vfopenf(pc, ಬಹುಲಕ, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_ಮಾರ್ಗ_opendir(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	DIR *dir;
	ಪೂರ್ಣಾಂಕ fd = -1;

	ಆದರೆ (ಮಾರ್ಗ)
		fd = ul_ಮಾರ್ಗ_open(pc, O_RDONLY|O_CLOEXEC, ಮಾರ್ಗ);
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pc->dir_ಮಾರ್ಗ) {
		ಪೂರ್ಣಾಂಕ dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_ಮಾರ್ಗ_get_dirfd(pc);
		ಆದರೆ (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	ಆದರೆ (fd < 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	dir = fdopendir(fd);
	ಆದರೆ (!dir) {
		close(fd);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}
	ಆದರೆ (!ಮಾರ್ಗ)
		 rewinddir(dir);
	ಹಿಂತಿರುಗಿ dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_ಮಾರ್ಗ_vopendirf(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, va_list ap)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = ul_ಮಾರ್ಗ_mkpath(pc, ಮಾರ್ಗ, ap);

	ಹಿಂತಿರುಗಿ !p ? ಖಾಲಿ : ul_ಮಾರ್ಗ_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_ಮಾರ್ಗ_opendirf(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, ಮಾರ್ಗ);
	dir = ul_ಮಾರ್ಗ_vopendirf(pc, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_ಮಾರ್ಗ_readlink(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ *buf, size_t bufsiz, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಪೂರ್ಣಾಂಕ dirfd;
	ssize_t ssz;

	ಆದರೆ (!ಮಾರ್ಗ) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = get_absdir(pc);
		ಆದರೆ (!p)
			ಹಿಂತಿರುಗಿ -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} ಇಲ್ಲದಿದ್ದರೆ {
		dirfd = ul_ಮಾರ್ಗ_get_dirfd(pc);
		ಆದರೆ (dirfd < 0)
			ಹಿಂತಿರುಗಿ dirfd;

		ಆದರೆ (*ಮಾರ್ಗ == '/')
			ಮಾರ್ಗ++;

		ssz = readlinkat(dirfd, ಮಾರ್ಗ, buf, bufsiz - 1);
	}

	ಆದರೆ (ssz >= 0)
		buf[ssz] = '\0';
	ಹಿಂತಿರುಗಿ ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_ಮಾರ್ಗ_readlinkf(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ *buf, size_t bufsiz, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	va_list ap;

	va_start(ap, ಮಾರ್ಗ);
	p = ul_ಮಾರ್ಗ_mkpath(pc, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ !p ? -errno : ul_ಮಾರ್ಗ_readlink(pc, buf, bufsiz, p);
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_read(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ *buf, size_t len, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಪೂರ್ಣಾಂಕ rc, errsv;
	ಪೂರ್ಣಾಂಕ fd;

	fd = ul_ಮಾರ್ಗ_open(pc, O_RDONLY|O_CLOEXEC, ಮಾರ್ಗ);
	ಆದರೆ (fd < 0)
		ಹಿಂತಿರುಗಿ -errno;

	DBG(CXT, ul_debug(" reading '%s'", ಮಾರ್ಗ));
	rc = ul_vfs_read_all(pc ? pc->vfs : ಖಾಲಿ, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ಖಾಲಿ, fd);
	errno = errsv;
	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_vreadf(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ *buf, size_t len, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, va_list ap)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = ul_ಮಾರ್ಗ_mkpath(pc, ಮಾರ್ಗ, ap);

	ಹಿಂತಿರುಗಿ !p ? -errno : ul_ಮಾರ್ಗ_read(pc, buf, len, p);
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_readf(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ *buf, size_t len, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಂಕ rc;

	va_start(ap, ಮಾರ್ಗ);
	rc = ul_ಮಾರ್ಗ_vreadf(pc, buf, len, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_read_string(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ **str, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಅಕ್ಷರ buf[BUFSIZ];
	ಪೂರ್ಣಾಂಕ rc;

	ಆದರೆ (!str)
		ಹಿಂತಿರುಗಿ -EINVAL;

	*str = ಖಾಲಿ;

	rc = ul_ಮಾರ್ಗ_read_ಬಫರ್(pc, buf, ಗಾತ್ರ(buf), ಮಾರ್ಗ);
	ಆದರೆ (rc < 0)
		ಹಿಂತಿರುಗಿ rc;

	*str = strdup(buf);
	ಆದರೆ (!*str)
		rc = -ENOMEM;

	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_readf_string(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ **str, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	va_list ap;

	va_start(ap, ಮಾರ್ಗ);
	p = ul_ಮಾರ್ಗ_mkpath(pc, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ !p ? -errno : ul_ಮಾರ್ಗ_read_string(pc, str, p);
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_read_ಬಫರ್(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ *buf, size_t bufsz, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಪೂರ್ಣಾಂಕ rc = ul_ಮಾರ್ಗ_read(pc, buf, bufsz - 1, ಮಾರ್ಗ);

	ಆದರೆ (rc == 0)
		buf[0] = '\0';

	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		ಆದರೆ (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		ಇಲ್ಲದಿದ್ದರೆ
			buf[rc] = '\0';
	}

	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_vreadf_ಬಫರ್(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ *buf, size_t bufsz, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, va_list ap)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;

	p = ul_ಮಾರ್ಗ_mkpath(pc, ಮಾರ್ಗ, ap);

	ಹಿಂತಿರುಗಿ !p ? -errno : ul_ಮಾರ್ಗ_read_ಬಫರ್(pc, buf, bufsz, p);
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_readf_ಬಫರ್(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ *buf, size_t bufsz, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಂಕ rc;

	va_start(ap, ಮಾರ್ಗ);
	rc = ul_ಮಾರ್ಗ_vreadf_ಬಫರ್(pc, buf, bufsz, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_scanf(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	ಪೂರ್ಣಾಂಕ rc;

	f = ul_ಮಾರ್ಗ_fopen(pc, "r" UL_CLOEXECSTR, ಮಾರ್ಗ);
	ಆದರೆ (!f)
		ಹಿಂತಿರುಗಿ -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, ಮಾರ್ಗ));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_scanff(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, va_list ap, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	ಪೂರ್ಣಾಂಕ rc;

	f = ul_ಮಾರ್ಗ_vfopenf(pc, "r" UL_CLOEXECSTR, ಮಾರ್ಗ, ap);
	ಆದರೆ (!f)
		ಹಿಂತಿರುಗಿ -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	ಹಿಂತಿರುಗಿ rc;
}


ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_read_s64(ರಚನೆ ಮಾರ್ಗ_cxt *pc, int64_t *res, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	int64_t x = 0;
	ಪೂರ್ಣಾಂಕ rc;

	rc = ul_ಮಾರ್ಗ_scanf(pc, ಮಾರ್ಗ, "%"SCNd64, &x);
	ಆದರೆ (rc != 1)
		ಹಿಂತಿರುಗಿ -1;
	ಆದರೆ (res)
		*res = x;
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_readf_s64(ರಚನೆ ಮಾರ್ಗ_cxt *pc, int64_t *res, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	va_list ap;

	va_start(ap, ಮಾರ್ಗ);
	p = ul_ಮಾರ್ಗ_mkpath(pc, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ !p ? -errno : ul_ಮಾರ್ಗ_read_s64(pc, res, p);
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_read_u64(ರಚನೆ ಮಾರ್ಗ_cxt *pc, uint64_t *res, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	uint64_t x = 0;
	ಪೂರ್ಣಾಂಕ rc;

	rc = ul_ಮಾರ್ಗ_scanf(pc, ಮಾರ್ಗ, "%"SCNu64, &x);
	ಆದರೆ (rc != 1)
		ಹಿಂತಿರುಗಿ -1;
	ಆದರೆ (res)
		*res = x;
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_readf_u64(ರಚನೆ ಮಾರ್ಗ_cxt *pc, uint64_t *res, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	va_list ap;

	va_start(ap, ಮಾರ್ಗ);
	p = ul_ಮಾರ್ಗ_mkpath(pc, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ !p ? -errno : ul_ಮಾರ್ಗ_read_u64(pc, res, p);
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_read_s32(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಪೂರ್ಣಾಂಕ *res, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಪೂರ್ಣಾಂಕ rc, x = 0;

	rc = ul_ಮಾರ್ಗ_scanf(pc, ಮಾರ್ಗ, "%d", &x);
	ಆದರೆ (rc != 1)
		ಹಿಂತಿರುಗಿ -1;
	ಆದರೆ (res)
		*res = x;
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_readf_s32(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಪೂರ್ಣಾಂಕ *res, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	va_list ap;

	va_start(ap, ಮಾರ್ಗ);
	p = ul_ಮಾರ್ಗ_mkpath(pc, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ !p ? -errno : ul_ಮಾರ್ಗ_read_s32(pc, res, p);
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_read_u32(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *res, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಪೂರ್ಣಾಂಕ rc;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ x = 0;

	rc = ul_ಮಾರ್ಗ_scanf(pc, ಮಾರ್ಗ, "%u", &x);
	ಆದರೆ (rc != 1)
		ಹಿಂತಿರುಗಿ -1;
	ಆದರೆ (res)
		*res = x;
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_readf_u32(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *res, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	va_list ap;

	va_start(ap, ಮಾರ್ಗ);
	p = ul_ಮಾರ್ಗ_mkpath(pc, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ !p ? -errno : ul_ಮಾರ್ಗ_read_u32(pc, res, p);
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_read_majmin(ರಚನೆ ಮಾರ್ಗ_cxt *pc, dev_t *res, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಪೂರ್ಣಾಂಕ rc, maj = 0, min = 0;

	rc = ul_ಮಾರ್ಗ_scanf(pc, ಮಾರ್ಗ, "%d:%d", &maj, &min);
	ಆದರೆ (rc != 2)
		ಹಿಂತಿರುಗಿ -1;
	ಆದರೆ (res)
		*res = makedev(maj, min);
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_readf_majmin(ರಚನೆ ಮಾರ್ಗ_cxt *pc, dev_t *res, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	va_list ap;

	va_start(ap, ಮಾರ್ಗ);
	p = ul_ಮಾರ್ಗ_mkpath(pc, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ !p ? -errno : ul_ಮಾರ್ಗ_read_majmin(pc, res, p);
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_write_string(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಪೂರ್ಣಾಂಕ rc, errsv;
	ಪೂರ್ಣಾಂಕ fd;

	fd = ul_ಮಾರ್ಗ_open(pc, O_WRONLY|O_CLOEXEC, ಮಾರ್ಗ);
	ಆದರೆ (fd < 0)
		ಹಿಂತಿರುಗಿ -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : ಖಾಲಿ, fd, str, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ಖಾಲಿ, fd);
	errno = errsv;
	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_writef_string(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	va_list ap;

	va_start(ap, ಮಾರ್ಗ);
	p = ul_ಮಾರ್ಗ_mkpath(pc, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ !p ? -errno : ul_ಮಾರ್ಗ_write_string(pc, str, p);
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_write_s64(ರಚನೆ ಮಾರ್ಗ_cxt *pc, int64_t ಅಂಕೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಅಕ್ಷರ buf[ಗಾತ್ರ(stringify_ಮೌಲ್ಯ(LLONG_MAX))];
	ಪೂರ್ಣಾಂಕ rc, errsv;
	ಪೂರ್ಣಾಂಕ fd, len;

	fd = ul_ಮಾರ್ಗ_open(pc, O_WRONLY|O_CLOEXEC, ಮಾರ್ಗ);
	ಆದರೆ (fd < 0)
		ಹಿಂತಿರುಗಿ -errno;

	len = snprintf(buf, ಗಾತ್ರ(buf), "%" PRId64, ಅಂಕೆ);
	ಆದರೆ (len < 0 || (size_t) len >= ಗಾತ್ರ(buf))
		rc = len < 0 ? -errno : -E2BIG;
	ಇಲ್ಲದಿದ್ದರೆ
		rc = ul_vfs_write_all(pc ? pc->vfs : ಖಾಲಿ, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ಖಾಲಿ, fd);
	errno = errsv;
	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_write_u64(ರಚನೆ ಮಾರ್ಗ_cxt *pc, uint64_t ಅಂಕೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಅಕ್ಷರ buf[ಗಾತ್ರ(stringify_ಮೌಲ್ಯ(ULLONG_MAX))];
	ಪೂರ್ಣಾಂಕ rc, errsv;
	ಪೂರ್ಣಾಂಕ fd, len;

	fd = ul_ಮಾರ್ಗ_open(pc, O_WRONLY|O_CLOEXEC, ಮಾರ್ಗ);
	ಆದರೆ (fd < 0)
		ಹಿಂತಿರುಗಿ -errno;

	len = snprintf(buf, ಗಾತ್ರ(buf), "%" PRIu64, ಅಂಕೆ);
	ಆದರೆ (len < 0 || (size_t) len >= ಗಾತ್ರ(buf))
		rc = len < 0 ? -errno : -E2BIG;
	ಇಲ್ಲದಿದ್ದರೆ
		rc = ul_vfs_write_all(pc ? pc->vfs : ಖಾಲಿ, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ಖಾಲಿ, fd);
	errno = errsv;
	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_writef_u64(ರಚನೆ ಮಾರ್ಗ_cxt *pc, uint64_t ಅಂಕೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	va_list ap;

	va_start(ap, ಮಾರ್ಗ);
	p = ul_ಮಾರ್ಗ_mkpath(pc, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ !p ? -errno : ul_ಮಾರ್ಗ_write_u64(pc, ಅಂಕೆ, p);

}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_ಎಣಿಕೆ_dirents(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	DIR *dir;
	ಪೂರ್ಣಾಂಕ r = 0;

	dir = ul_ಮಾರ್ಗ_opendir(pc, ಮಾರ್ಗ);
	ಆದರೆ (!dir)
		ಹಿಂತಿರುಗಿ 0;

	ಆಗ (xreaddir(dir)) r++;

	closedir(dir);
	ಹಿಂತಿರುಗಿ r;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_countf_dirents(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	va_list ap;

	va_start(ap, ಮಾರ್ಗ);
	p = ul_ಮಾರ್ಗ_mkpath(pc, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ !p ? -errno : ul_ಮಾರ್ಗ_ಎಣಿಕೆ_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_ಮುಂದಿನ_dirent(ರಚನೆ ಮಾರ್ಗ_cxt *pc, DIR **sub, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dirname, ರಚನೆ dirent **d)
{
	ಆದರೆ (!pc || !sub || !d)
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಆದರೆ (!*sub) {
		*sub = ul_ಮಾರ್ಗ_opendir(pc, dirname);
		ಆದರೆ (!*sub)
			ಹಿಂತಿರುಗಿ -errno;
	}

	*d = xreaddir(*sub);
	ಆದರೆ (*d)
		ಹಿಂತಿರುಗಿ 0;

	closedir(*sub);
	*sub = ಖಾಲಿ;
	ಹಿಂತಿರುಗಿ 1;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_CPU_ಗಣ_T
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_cpuparse(ರಚನೆ ಮಾರ್ಗ_cxt *pc, cpu_set_t **ಗಣ, ಪೂರ್ಣಾಂಕ maxcpus, ಪೂರ್ಣಾಂಕ islist, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	ಅಕ್ಷರ *buf;
	ಪೂರ್ಣಾಂಕ rc;

	*ಗಣ = ಖಾಲಿ;

	buf = ಸ್ಮರಣೆ_ಕೊಡು(len);
	ಆದರೆ (!buf)
		ಹಿಂತಿರುಗಿ -ENOMEM;

	rc = ul_ಮಾರ್ಗ_vreadf_ಬಫರ್(pc, buf, len, ಮಾರ್ಗ, ap);
	ಆದರೆ (rc < 0)
		ಹೋಗು ಹೊರಹೋಗು;

	*ಗಣ = cpuset_alloc(maxcpus, &setsize, ಖಾಲಿ);
	ಆದರೆ (!*ಗಣ) {
		rc = -EINVAL;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಆದರೆ (islist) {
		ಆದರೆ (cpulist_parse(buf, *ಗಣ, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			ಹೋಗು ಹೊರಹೋಗು;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (cpumask_parse(buf, *ಗಣ, setsize)) {
			errno = EINVAL;
			rc = -errno;
			ಹೋಗು ಹೊರಹೋಗು;
		}
	}
	rc = 0;

ಹೊರಹೋಗು:
	ಆದರೆ (rc) {
		cpuset_free(*ಗಣ);
		*ಗಣ = ಖಾಲಿ;
	}
	ಬಿಡುಗಡೆ(buf);
	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_readf_cpuset(ರಚನೆ ಮಾರ್ಗ_cxt *pc, cpu_set_t **ಗಣ, ಪೂರ್ಣಾಂಕ maxcpus, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಂಕ rc = 0;

	va_start(ap, ಮಾರ್ಗ);
	rc = ul_ಮಾರ್ಗ_cpuparse(pc, ಗಣ, maxcpus, 0, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_readf_cpulist(ರಚನೆ ಮಾರ್ಗ_cxt *pc, cpu_set_t **ಗಣ, ಪೂರ್ಣಾಂಕ maxcpus, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಂಕ rc = 0;

	va_start(ap, ಮಾರ್ಗ);
	rc = ul_ಮಾರ್ಗ_cpuparse(pc, ಗಣ, maxcpus, 1, ಮಾರ್ಗ, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ rc;
}

#ಆದರೆ_ಅಂತ್ಯ /* HAVE_CPU_SET_T */


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_ಮಾರ್ಗ
#ಅನ್ತರ್ಭಾವ <getopt.h>

ಸ್ಥಿರ ಶೂನ್ಯ __attribute__((__noreturn__)) usage(ಶೂನ್ಯ)
{
	fprintf(stdout, " %s [options] <dir> <command>\n\n", ಕಾರ್ಯಕ್ರಮ_invocation_short_name);
	fputs(" -p, --prefix <dir>      redirect hardcoded paths to <dir>\n", stdout);

	fputs(" Commands:\n", stdout);
	fputs(" read-u64 <file>            read uint64_t from file\n", stdout);
	fputs(" read-s64 <file>            read  int64_t from file\n", stdout);
	fputs(" read-u32 <file>            read uint32_t from file\n", stdout);
	fputs(" read-s32 <file>            read  int32_t from file\n", stdout);
	fputs(" read-string <file>         read string  from file\n", stdout);
	fputs(" read-majmin <file>         read devno from file\n", stdout);
	fputs(" read-link <file>           read symlink\n", stdout);
	fputs(" write-string <file> <str>  write string from file\n", stdout);
	fputs(" write-u64 <file> <str>     write uint64_t from file\n", stdout);

	ನಿರ್ಗಮಿಸು(EXIT_SUCCESS);
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ಪೂರ್ಣಾಂಕ c;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ = ಖಾಲಿ, *dir, *file, *command;
	ರಚನೆ ಮಾರ್ಗ_cxt *pc = ಖಾಲಿ;

	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
		{ "prefix",	1, ಖಾಲಿ, 'p' },
		{ "help",       0, ಖಾಲಿ, 'h' },
		{ ಖಾಲಿ, 0, ಖಾಲಿ, 0 },
	};

	ಆಗ((c = getopt_long(argc, argv, "p:h", longopts, ಖಾಲಿ)) != -1) {
		ಆಯ್ಕೆ(c) {
		ಸ್ಥಿತಿ 'p':
			ಪೂರ್ವಪ್ರತ್ಯಯ = optarg;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'h':
			usage();
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			err(EXIT_FAILURE, "try --help");
		}
	}

	ಆದರೆ (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_ಮಾರ್ಗ_init_debug();

	pc = ul_new_ಮಾರ್ಗ("%s", dir);
	ಆದರೆ (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	ಆದರೆ (ಪೂರ್ವಪ್ರತ್ಯಯ)
		ul_ಮಾರ್ಗ_ಗಣ_ಪೂರ್ವಪ್ರತ್ಯಯ(pc, ಪೂರ್ವಪ್ರತ್ಯಯ);

	ಆದರೆ (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	ಆದರೆ (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		ಆದರೆ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ಆದರೆ (ul_ಮಾರ್ಗ_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		ಬರೆ("read:  %s: %u\n", file, res);

		ಆದರೆ (ul_ಮಾರ್ಗ_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		ಬರೆ("readf: %s: %u\n", file, res);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(command, "read-s32") == 0) {
		int32_t res;

		ಆದರೆ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ಆದರೆ (ul_ಮಾರ್ಗ_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		ಬರೆ("read:  %s: %d\n", file, res);

		ಆದರೆ (ul_ಮಾರ್ಗ_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		ಬರೆ("readf: %s: %d\n", file, res);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		ಆದರೆ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ಆದರೆ (ul_ಮಾರ್ಗ_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		ಬರೆ("read:  %s: %" PRIu64 "\n", file, res);

		ಆದರೆ (ul_ಮಾರ್ಗ_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		ಬರೆ("readf: %s: %" PRIu64 "\n", file, res);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(command, "read-s64") == 0) {
		int64_t res;

		ಆದರೆ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ಆದರೆ (ul_ಮಾರ್ಗ_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		ಬರೆ("read:  %s: %" PRId64 "\n", file, res);

		ಆದರೆ (ul_ಮಾರ್ಗ_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		ಬರೆ("readf: %s: %" PRId64 "\n", file, res);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		ಆದರೆ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ಆದರೆ (ul_ಮಾರ್ಗ_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		ಬರೆ("read:  %s: %d\n", file, (ಪೂರ್ಣಾಂಕ) res);

		ಆದರೆ (ul_ಮಾರ್ಗ_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		ಬರೆ("readf: %s: %d\n", file, (ಪೂರ್ಣಾಂಕ) res);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(command, "read-string") == 0) {
		ಅಕ್ಷರ *res;

		ಆದರೆ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ಆದರೆ (ul_ಮಾರ್ಗ_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		ಬರೆ("read:  %s: %s\n", file, res);

		ಆದರೆ (ul_ಮಾರ್ಗ_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		ಬರೆ("readf: %s: %s\n", file, res);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(command, "read-link") == 0) {
		ಅಕ್ಷರ res[PATH_MAX];

		ಆದರೆ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ಆದರೆ (ul_ಮಾರ್ಗ_readlink(pc, res, ಗಾತ್ರ(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		ಬರೆ("read:  %s: %s\n", file, res);

		ಆದರೆ (ul_ಮಾರ್ಗ_readlinkf(pc, res, ಗಾತ್ರ(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		ಬರೆ("readf: %s: %s\n", file, res);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(command, "write-string") == 0) {
		ಅಕ್ಷರ *str;

		ಆದರೆ (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		ಆದರೆ (ul_ಮಾರ್ಗ_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		ಆದರೆ (ul_ಮಾರ್ಗ_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(command, "write-u64") == 0) {
		uint64_t ಅಂಕೆ;

		ಆದರೆ (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		ಅಂಕೆ = strtoumax(argv[optind++], ಖಾಲಿ, 0);

		ಆದರೆ (ul_ಮಾರ್ಗ_write_u64(pc, ಅಂಕೆ, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		ಆದರೆ (ul_ಮಾರ್ಗ_writef_u64(pc, ಅಂಕೆ, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_ಮಾರ್ಗ(pc);
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ /* TEST_PROGRAM_PATH */
