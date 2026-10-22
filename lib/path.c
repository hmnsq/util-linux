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
#অন্তর্ভাৱ <stdarg.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <inttypes.h>
#অন্তর্ভাৱ <errno.h>

#অন্তর্ভাৱ "c.h"
#অন্তর্ভাৱ "fileutils.h"
#অন্তর্ভাৱ "all-io.h"
#অন্তর্ভাৱ "vfs.h"
#অন্তর্ভাৱ "path.h"
#অন্তর্ভাৱ "debug.h"
#অন্তর্ভাৱ "strutils.h"

/*
 * Debug stuff (based on include/debug.h)
 */
স্থির UL_DEBUG_DEFINE_MASK(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPATH_DEBUG_INIT	(1 << 1)
#define ULPATH_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

শূন্য ul_path_init_debug(শূন্য)
{
	যদি (ulpath_debug_mask)
		ঘূৰাই;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

সংরচনা path_cxt *ul_new_path(ধ্রুৱ ৱর্ণ *dir, ...)
{
	সংরচনা path_cxt *pc = calloc(1, পরিমাণ(*pc));

	যদি (!pc)
		ঘূৰাই রিক্ত;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	যদি (dir) {
		পূর্ণাঙ্ক rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_path, dir, ap);
		va_end(ap);

		যদি (rc < 0 || !pc->dir_path)
			গচ্ছ fail;
	}
	ঘূৰাই pc;
fail:
	ul_unref_path(pc);
	ঘূৰাই রিক্ত;
}

শূন্য ul_ref_path(সংরচনা path_cxt *pc)
{
	যদি (pc)
		pc->refcount++;
}

শূন্য ul_unref_path(সংরচনা path_cxt *pc)
{
	যদি (!pc)
		ঘূৰাই;

	pc->refcount--;

	যদি (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		যদি (pc->dialect)
			pc->free_dialect(pc);
		ul_path_close_dirfd(pc);
		মুক্ত(pc->dir_path);
		মুক্ত(pc->prefix);
		মুক্ত(pc);
	}
}

শূন্য ul_path_refer_vfs(সংরচনা path_cxt *pc, ধ্রুৱ সংরচনা ul_vfs_ops *vfs)
{
	যদি (pc)
		pc->vfs = vfs;
}

পূর্ণাঙ্ক ul_path_set_prefix(সংরচনা path_cxt *pc, ধ্রুৱ ৱর্ণ *prefix)
{
	ৱর্ণ *p = রিক্ত;

	প্রতিজ্ঞা(pc->dir_fd < 0);

	যদি (prefix) {
		p = strdup(prefix);
		যদি (!p)
			ঘূৰাই -ENOMEM;
	}

	মুক্ত(pc->prefix);
	pc->prefix = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	ঘূৰাই 0;
}

ধ্রুৱ ৱর্ণ *ul_path_get_prefix(সংরচনা path_cxt *pc)
{
	ঘূৰাই pc ? pc->prefix : রিক্ত;
}

পূর্ণাঙ্ক ul_path_set_dir(সংরচনা path_cxt *pc, ধ্রুৱ ৱর্ণ *dir)
{
	ৱর্ণ *p = রিক্ত;

	যদি (dir) {
		p = strdup(dir);
		যদি (!p)
			ঘূৰাই -ENOMEM;
	}

	যদি (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	মুক্ত(pc->dir_path);
	pc->dir_path = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	ঘূৰাই 0;
}

ধ্রুৱ ৱর্ণ *ul_path_get_dir(সংরচনা path_cxt *pc)
{
	ঘূৰাই pc ? pc->dir_path : রিক্ত;
}

পূর্ণাঙ্ক ul_path_set_dialect(সংরচনা path_cxt *pc, শূন্য *data, শূন্য free_data(সংরচনা path_cxt *))
{
	pc->dialect = data;
	pc->free_dialect = free_data;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	ঘূৰাই 0;
}

শূন্য *ul_path_get_dialect(সংরচনা path_cxt *pc)
{
	ঘূৰাই pc ? pc->dialect : রিক্ত;
}

পূর্ণাঙ্ক ul_path_set_enoent_redirect(সংরচনা path_cxt *pc, পূর্ণাঙ্ক (*func)(সংরচনা path_cxt *, ধ্রুৱ ৱর্ণ *, পূর্ণাঙ্ক *))
{
	pc->redirect_on_enoent = func;
	ঘূৰাই 0;
}

স্থির ধ্রুৱ ৱর্ণ *get_absdir(সংরচনা path_cxt *pc)
{
	পূর্ণাঙ্ক rc;
	ধ্রুৱ ৱর্ণ *dirpath;

	যদি (!pc->prefix)
		ঘূৰাই pc->dir_path;

	dirpath = pc->dir_path;
	যদি (!dirpath)
		ঘূৰাই pc->prefix;
	যদি (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->path_buffer, পরিমাণ(pc->path_buffer), "%s/%s", pc->prefix, dirpath);
	যদি (rc < 0)
		ঘূৰাই রিক্ত;
	যদি ((size_t)rc >= পরিমাণ(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		ঘূৰাই রিক্ত;
	}

	ঘূৰাই pc->path_buffer;
}

পূর্ণাঙ্ক ul_path_is_accessible(সংরচনা path_cxt *pc)
{
	ধ্রুৱ ৱর্ণ *path;
	প্রতিজ্ঞা(pc);

	যদি (pc->dir_fd >= 0)
		ঘূৰাই 1;

	path = get_absdir(pc);
	যদি (!path)
		ঘূৰাই 0;
	ঘূৰাই access(path, F_OK) == 0;
}

পূর্ণাঙ্ক ul_path_get_dirfd(সংরচনা path_cxt *pc)
{
	প্রতিজ্ঞা(pc);
	প্রতিজ্ঞা(pc->dir_path);

	যদি (pc->dir_fd < 0) {
		ধ্রুৱ ৱর্ণ *path = get_absdir(pc);
		যদি (!path)
			ঘূৰাই -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", path));
		pc->dir_fd = open(path, O_RDONLY|O_CLOEXEC);
	}

	ঘূৰাই pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
শূন্য ul_path_close_dirfd(সংরচনা path_cxt *pc)
{
	প্রতিজ্ঞা(pc);

	যদি (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

পূর্ণাঙ্ক ul_path_isopen_dirfd(সংরচনা path_cxt *pc)
{
	ঘূৰাই pc && pc->dir_fd >= 0;
}

স্থির ধ্রুৱ ৱর্ণ *ul_path_mkpath(সংরচনা path_cxt *pc, ধ্রুৱ ৱর্ণ *path, va_list ap)
{
	পূর্ণাঙ্ক rc;

	errno = 0;

	rc = vsnprintf(pc->path_buffer, পরিমাণ(pc->path_buffer), path, ap);
	যদি (rc < 0) {
		যদি (!errno)
			errno = EINVAL;
		ঘূৰাই রিক্ত;
	}

	যদি ((size_t)rc >= পরিমাণ(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		ঘূৰাই রিক্ত;
	}

	ঘূৰাই pc->path_buffer;
}

ৱর্ণ *ul_path_get_abspath(সংরচনা path_cxt *pc, ৱর্ণ *buf, size_t bufsz, ধ্রুৱ ৱর্ণ *path, ...)
{
	যদি (path) {
		পূর্ণাঙ্ক rc;
		va_list ap;
		ধ্রুৱ ৱর্ণ *tail = রিক্ত, *dirpath = pc->dir_path;

		va_start(ap, path);
		tail = ul_path_mkpath(pc, path, ap);
		va_end(ap);

		যদি (dirpath && *dirpath == '/')
			dirpath++;
		যদি (tail && *tail == '/')
			tail++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->prefix ? pc->prefix : "",
				dirpath ? dirpath : "",
				tail ? tail : "");

		যদি ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			ঘূৰাই রিক্ত;
		}
	} অন্যথা {
		ধ্রুৱ ৱর্ণ *tmp = get_absdir(pc);

		যদি (!tmp)
			ঘূৰাই রিক্ত;
		xstrncpy(buf, tmp, bufsz);
	}

	ঘূৰাই buf;
}


পূর্ণাঙ্ক ul_path_access(সংরচনা path_cxt *pc, পূর্ণাঙ্ক mode, ধ্রুৱ ৱর্ণ *path)
{
	পূর্ণাঙ্ক rc;

	যদি (!path)
		ঘূৰাই -EINVAL;
	যদি (!pc) {
		rc = access(path, mode);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", path, rc));
	} অন্যথা {
		পূর্ণাঙ্ক dir = ul_path_get_dirfd(pc);
		যদি (dir < 0)
			ঘূৰাই dir;
		যদি (*path == '/')
			path++;

		rc = faccessat(dir, path, mode, 0);

		যদি (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = faccessat(dir, path, mode, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", path, rc));
	}
	ঘূৰাই rc;
}

পূর্ণাঙ্ক ul_path_accessf(সংরচনা path_cxt *pc, পূর্ণাঙ্ক mode, ধ্রুৱ ৱর্ণ *path, ...)
{
	va_list ap;
	ধ্রুৱ ৱর্ণ *p;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ঘূৰাই !p ? -errno : ul_path_access(pc, mode, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
পূর্ণাঙ্ক ul_path_stat(সংরচনা path_cxt *pc, সংরচনা stat *sb, পূর্ণাঙ্ক flags, ধ্রুৱ ৱর্ণ *path)
{
	পূর্ণাঙ্ক rc;

	যদি (!pc) {
		rc = path ? stat(path, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", path, rc));
	} অন্যথা {
		পূর্ণাঙ্ক dir = ul_path_get_dirfd(pc);
		যদি (dir < 0)
			ঘূৰাই dir;
		যদি (path) {
			যদি  (*path == '/')
				path++;
			rc = fstatat(dir, path, sb, flags);

		} অন্যথা
			rc = fstat(dir, sb);	/* dir itself */

		যদি (rc && errno == ENOENT
		    && path
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = fstatat(dir, path, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", path, rc));
	}
	ঘূৰাই rc;
}

পূর্ণাঙ্ক ul_path_vstatf(সংরচনা path_cxt *pc, সংরচনা stat *sb, পূর্ণাঙ্ক flags, ধ্রুৱ ৱর্ণ *path, va_list ap)
{
	ধ্রুৱ ৱর্ণ *p = ul_path_mkpath(pc, path, ap);

	ঘূৰাই !p ? -errno : ul_path_stat(pc, sb, flags, p);
}

পূর্ণাঙ্ক ul_path_statf(সংরচনা path_cxt *pc, সংরচনা stat *sb, পূর্ণাঙ্ক flags, ধ্রুৱ ৱর্ণ *path, ...)
{
	va_list ap;
	পূর্ণাঙ্ক rc;

	va_start(ap, path);
	rc = ul_path_vstatf(pc, sb, flags, path, ap);
	va_end(ap);

	ঘূৰাই rc;
}
#ifdef HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
পূর্ণাঙ্ক ul_path_statx(সংরচনা path_cxt *pc, সংরচনা statx *stx, পূর্ণাঙ্ক flags, unsigned পূর্ণাঙ্ক mask,
		  ধ্রুৱ ৱর্ণ *path)
{
	পূর্ণাঙ্ক rc;

	যদি (!path)
		ঘূৰাই -EINVAL;
	যদি (!pc)
		rc = path ? statx(AT_FDCWD, path, flags, mask, stx) : - EINVAL;
	অন্যথা {
		পূর্ণাঙ্ক dir = ul_path_get_dirfd(pc);
		যদি (dir < 0)
			ঘূৰাই dir;
		যদি (*path == '/')
			path++;

		rc = statx(dir, path, flags, mask, stx);
	}

	ঘূৰাই rc;
}
#অন্যথা
পূর্ণাঙ্ক ul_path_statx(সংরচনা path_cxt *pc __attribute__((__unused__)),
		  সংরচনা statx *stx __attribute__((__unused__)),
		  পূর্ণাঙ্ক flags __attribute__((__unused__)),
		  unsigned পূর্ণাঙ্ক mask __attribute__((__unused__)),
		  ধ্রুৱ ৱর্ণ *path __attribute__((__unused__)))
{
	errno = ENOSYS;
	ঘূৰাই -1;
}
#endif /* HAVE_STATX */

পূর্ণাঙ্ক ul_path_vstatxf(সংরচনা path_cxt *pc, সংরচনা statx *stx, পূর্ণাঙ্ক flags, unsigned পূর্ণাঙ্ক mask,
		    ধ্রুৱ ৱর্ণ *path, va_list ap)
{
	ধ্রুৱ ৱর্ণ *p = ul_path_mkpath(pc, path, ap);

	ঘূৰাই !p ? -errno : ul_path_statx(pc, stx, flags, mask, p);
}

পূর্ণাঙ্ক ul_path_statxf(সংরচনা path_cxt *pc, সংরচনা statx *stx, পূর্ণাঙ্ক flags, unsigned পূর্ণাঙ্ক mask,
		   ধ্রুৱ ৱর্ণ *path, ...)
{
	va_list ap;
	পূর্ণাঙ্ক rc;

	va_start(ap, path);
	rc = ul_path_vstatxf(pc, stx, flags, mask, path, ap);
	va_end(ap);

	ঘূৰাই rc;
}

পূর্ণাঙ্ক ul_path_open(সংরচনা path_cxt *pc, পূর্ণাঙ্ক flags, ধ্রুৱ ৱর্ণ *path)
{
	পূর্ণাঙ্ক fd;

	যদি (!path)
		ঘূৰাই -EINVAL;
	যদি (!pc) {
		fd = open(path, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", path));
	} অন্যথা {
		পূর্ণাঙ্ক fdx;
		পূর্ণাঙ্ক dir = ul_path_get_dirfd(pc);
		যদি (dir < 0)
			ঘূৰাই dir;

		যদি (*path == '/')
			path++;

		fdx = fd = openat(dir, path, flags);

		যদি (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			fd = openat(dir, path, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", path, fdx != fd ? " [redirected]" : ""));
	}
	ঘূৰাই fd;
}

পূর্ণাঙ্ক ul_path_vopenf(সংরচনা path_cxt *pc, পূর্ণাঙ্ক flags, ধ্রুৱ ৱর্ণ *path, va_list ap)
{
	ধ্রুৱ ৱর্ণ *p = ul_path_mkpath(pc, path, ap);

	ঘূৰাই !p ? -errno : ul_path_open(pc, flags, p);
}

পূর্ণাঙ্ক ul_path_openf(সংরচনা path_cxt *pc, পূর্ণাঙ্ক flags, ধ্রুৱ ৱর্ণ *path, ...)
{
	va_list ap;
	পূর্ণাঙ্ক rc;

	va_start(ap, path);
	rc = ul_path_vopenf(pc, flags, path, ap);
	va_end(ap);

	ঘূৰাই rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_path_fopen(সংরচনা path_cxt *pc, ধ্রুৱ ৱর্ণ *mode, ধ্রুৱ ৱর্ণ *path)
{
	পূর্ণাঙ্ক flags = ul_mode_to_flags(mode);
	পূর্ণাঙ্ক fd = ul_path_open(pc, flags, path);

	যদি (fd < 0)
		ঘূৰাই রিক্ত;

	ঘূৰাই ul_vfs_fdopen(pc ? pc->vfs : রিক্ত, fd, mode);
}


FILE *ul_path_vfopenf(সংরচনা path_cxt *pc, ধ্রুৱ ৱর্ণ *mode, ধ্রুৱ ৱর্ণ *path, va_list ap)
{
	ধ্রুৱ ৱর্ণ *p = ul_path_mkpath(pc, path, ap);

	ঘূৰাই !p ? রিক্ত : ul_path_fopen(pc, mode, p);
}

FILE *ul_path_fopenf(সংরচনা path_cxt *pc, ধ্রুৱ ৱর্ণ *mode, ধ্রুৱ ৱর্ণ *path, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, path);
	f = ul_path_vfopenf(pc, mode, path, ap);
	va_end(ap);

	ঘূৰাই f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendir(সংরচনা path_cxt *pc, ধ্রুৱ ৱর্ণ *path)
{
	DIR *dir;
	পূর্ণাঙ্ক fd = -1;

	যদি (path)
		fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	অন্যথা যদি (pc->dir_path) {
		পূর্ণাঙ্ক dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_path_get_dirfd(pc);
		যদি (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	যদি (fd < 0)
		ঘূৰাই রিক্ত;

	dir = fdopendir(fd);
	যদি (!dir) {
		close(fd);
		ঘূৰাই রিক্ত;
	}
	যদি (!path)
		 rewinddir(dir);
	ঘূৰাই dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_vopendirf(সংরচনা path_cxt *pc, ধ্রুৱ ৱর্ণ *path, va_list ap)
{
	ধ্রুৱ ৱর্ণ *p = ul_path_mkpath(pc, path, ap);

	ঘূৰাই !p ? রিক্ত : ul_path_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendirf(সংরচনা path_cxt *pc, ধ্রুৱ ৱর্ণ *path, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, path);
	dir = ul_path_vopendirf(pc, path, ap);
	va_end(ap);

	ঘূৰাই dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlink(সংরচনা path_cxt *pc, ৱর্ণ *buf, size_t bufsiz, ধ্রুৱ ৱর্ণ *path)
{
	পূর্ণাঙ্ক dirfd;
	ssize_t ssz;

	যদি (!path) {
		ধ্রুৱ ৱর্ণ *p = get_absdir(pc);
		যদি (!p)
			ঘূৰাই -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} অন্যথা {
		dirfd = ul_path_get_dirfd(pc);
		যদি (dirfd < 0)
			ঘূৰাই dirfd;

		যদি (*path == '/')
			path++;

		ssz = readlinkat(dirfd, path, buf, bufsiz - 1);
	}

	যদি (ssz >= 0)
		buf[ssz] = '\0';
	ঘূৰাই ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlinkf(সংরচনা path_cxt *pc, ৱর্ণ *buf, size_t bufsiz, ধ্রুৱ ৱর্ণ *path, ...)
{
	ধ্রুৱ ৱর্ণ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ঘূৰাই !p ? -errno : ul_path_readlink(pc, buf, bufsiz, p);
}

পূর্ণাঙ্ক ul_path_read(সংরচনা path_cxt *pc, ৱর্ণ *buf, size_t len, ধ্রুৱ ৱর্ণ *path)
{
	পূর্ণাঙ্ক rc, errsv;
	পূর্ণাঙ্ক fd;

	fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	যদি (fd < 0)
		ঘূৰাই -errno;

	DBG(CXT, ul_debug(" reading '%s'", path));
	rc = ul_vfs_read_all(pc ? pc->vfs : রিক্ত, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : রিক্ত, fd);
	errno = errsv;
	ঘূৰাই rc;
}

পূর্ণাঙ্ক ul_path_vreadf(সংরচনা path_cxt *pc, ৱর্ণ *buf, size_t len, ধ্রুৱ ৱর্ণ *path, va_list ap)
{
	ধ্রুৱ ৱর্ণ *p = ul_path_mkpath(pc, path, ap);

	ঘূৰাই !p ? -errno : ul_path_read(pc, buf, len, p);
}

পূর্ণাঙ্ক ul_path_readf(সংরচনা path_cxt *pc, ৱর্ণ *buf, size_t len, ধ্রুৱ ৱর্ণ *path, ...)
{
	va_list ap;
	পূর্ণাঙ্ক rc;

	va_start(ap, path);
	rc = ul_path_vreadf(pc, buf, len, path, ap);
	va_end(ap);

	ঘূৰাই rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
পূর্ণাঙ্ক ul_path_read_string(সংরচনা path_cxt *pc, ৱর্ণ **str, ধ্রুৱ ৱর্ণ *path)
{
	ৱর্ণ buf[BUFSIZ];
	পূর্ণাঙ্ক rc;

	যদি (!str)
		ঘূৰাই -EINVAL;

	*str = রিক্ত;

	rc = ul_path_read_buffer(pc, buf, পরিমাণ(buf), path);
	যদি (rc < 0)
		ঘূৰাই rc;

	*str = strdup(buf);
	যদি (!*str)
		rc = -ENOMEM;

	ঘূৰাই rc;
}

পূর্ণাঙ্ক ul_path_readf_string(সংরচনা path_cxt *pc, ৱর্ণ **str, ধ্রুৱ ৱর্ণ *path, ...)
{
	ধ্রুৱ ৱর্ণ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ঘূৰাই !p ? -errno : ul_path_read_string(pc, str, p);
}

পূর্ণাঙ্ক ul_path_read_buffer(সংরচনা path_cxt *pc, ৱর্ণ *buf, size_t bufsz, ধ্রুৱ ৱর্ণ *path)
{
	পূর্ণাঙ্ক rc = ul_path_read(pc, buf, bufsz - 1, path);

	যদি (rc == 0)
		buf[0] = '\0';

	অন্যথা যদি (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		যদি (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		অন্যথা
			buf[rc] = '\0';
	}

	ঘূৰাই rc;
}

পূর্ণাঙ্ক ul_path_vreadf_buffer(সংরচনা path_cxt *pc, ৱর্ণ *buf, size_t bufsz, ধ্রুৱ ৱর্ণ *path, va_list ap)
{
	ধ্রুৱ ৱর্ণ *p;

	p = ul_path_mkpath(pc, path, ap);

	ঘূৰাই !p ? -errno : ul_path_read_buffer(pc, buf, bufsz, p);
}

পূর্ণাঙ্ক ul_path_readf_buffer(সংরচনা path_cxt *pc, ৱর্ণ *buf, size_t bufsz, ধ্রুৱ ৱর্ণ *path, ...)
{
	va_list ap;
	পূর্ণাঙ্ক rc;

	va_start(ap, path);
	rc = ul_path_vreadf_buffer(pc, buf, bufsz, path, ap);
	va_end(ap);

	ঘূৰাই rc;
}

পূর্ণাঙ্ক ul_path_scanf(সংরচনা path_cxt *pc, ধ্রুৱ ৱর্ণ *path, ধ্রুৱ ৱর্ণ *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	পূর্ণাঙ্ক rc;

	f = ul_path_fopen(pc, "r" UL_CLOEXECSTR, path);
	যদি (!f)
		ঘূৰাই -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, path));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	ঘূৰাই rc;
}

পূর্ণাঙ্ক ul_path_scanff(সংরচনা path_cxt *pc, ধ্রুৱ ৱর্ণ *path, va_list ap, ধ্রুৱ ৱর্ণ *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	পূর্ণাঙ্ক rc;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	যদি (!f)
		ঘূৰাই -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	ঘূৰাই rc;
}


পূর্ণাঙ্ক ul_path_read_s64(সংরচনা path_cxt *pc, int64_t *res, ধ্রুৱ ৱর্ণ *path)
{
	int64_t x = 0;
	পূর্ণাঙ্ক rc;

	rc = ul_path_scanf(pc, path, "%"SCNd64, &x);
	যদি (rc != 1)
		ঘূৰাই -1;
	যদি (res)
		*res = x;
	ঘূৰাই 0;
}

পূর্ণাঙ্ক ul_path_readf_s64(সংরচনা path_cxt *pc, int64_t *res, ধ্রুৱ ৱর্ণ *path, ...)
{
	ধ্রুৱ ৱর্ণ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ঘূৰাই !p ? -errno : ul_path_read_s64(pc, res, p);
}

পূর্ণাঙ্ক ul_path_read_u64(সংরচনা path_cxt *pc, uint64_t *res, ধ্রুৱ ৱর্ণ *path)
{
	uint64_t x = 0;
	পূর্ণাঙ্ক rc;

	rc = ul_path_scanf(pc, path, "%"SCNu64, &x);
	যদি (rc != 1)
		ঘূৰাই -1;
	যদি (res)
		*res = x;
	ঘূৰাই 0;
}

পূর্ণাঙ্ক ul_path_readf_u64(সংরচনা path_cxt *pc, uint64_t *res, ধ্রুৱ ৱর্ণ *path, ...)
{
	ধ্রুৱ ৱর্ণ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ঘূৰাই !p ? -errno : ul_path_read_u64(pc, res, p);
}

পূর্ণাঙ্ক ul_path_read_s32(সংরচনা path_cxt *pc, পূর্ণাঙ্ক *res, ধ্রুৱ ৱর্ণ *path)
{
	পূর্ণাঙ্ক rc, x = 0;

	rc = ul_path_scanf(pc, path, "%d", &x);
	যদি (rc != 1)
		ঘূৰাই -1;
	যদি (res)
		*res = x;
	ঘূৰাই 0;
}

পূর্ণাঙ্ক ul_path_readf_s32(সংরচনা path_cxt *pc, পূর্ণাঙ্ক *res, ধ্রুৱ ৱর্ণ *path, ...)
{
	ধ্রুৱ ৱর্ণ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ঘূৰাই !p ? -errno : ul_path_read_s32(pc, res, p);
}

পূর্ণাঙ্ক ul_path_read_u32(সংরচনা path_cxt *pc, unsigned পূর্ণাঙ্ক *res, ধ্রুৱ ৱর্ণ *path)
{
	পূর্ণাঙ্ক rc;
	unsigned পূর্ণাঙ্ক x = 0;

	rc = ul_path_scanf(pc, path, "%u", &x);
	যদি (rc != 1)
		ঘূৰাই -1;
	যদি (res)
		*res = x;
	ঘূৰাই 0;
}

পূর্ণাঙ্ক ul_path_readf_u32(সংরচনা path_cxt *pc, unsigned পূর্ণাঙ্ক *res, ধ্রুৱ ৱর্ণ *path, ...)
{
	ধ্রুৱ ৱর্ণ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ঘূৰাই !p ? -errno : ul_path_read_u32(pc, res, p);
}

পূর্ণাঙ্ক ul_path_read_majmin(সংরচনা path_cxt *pc, dev_t *res, ধ্রুৱ ৱর্ণ *path)
{
	পূর্ণাঙ্ক rc, maj = 0, min = 0;

	rc = ul_path_scanf(pc, path, "%d:%d", &maj, &min);
	যদি (rc != 2)
		ঘূৰাই -1;
	যদি (res)
		*res = makedev(maj, min);
	ঘূৰাই 0;
}

পূর্ণাঙ্ক ul_path_readf_majmin(সংরচনা path_cxt *pc, dev_t *res, ধ্রুৱ ৱর্ণ *path, ...)
{
	ধ্রুৱ ৱর্ণ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ঘূৰাই !p ? -errno : ul_path_read_majmin(pc, res, p);
}

পূর্ণাঙ্ক ul_path_write_string(সংরচনা path_cxt *pc, ধ্রুৱ ৱর্ণ *str, ধ্রুৱ ৱর্ণ *path)
{
	পূর্ণাঙ্ক rc, errsv;
	পূর্ণাঙ্ক fd;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	যদি (fd < 0)
		ঘূৰাই -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : রিক্ত, fd, str, strlen(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : রিক্ত, fd);
	errno = errsv;
	ঘূৰাই rc;
}

পূর্ণাঙ্ক ul_path_writef_string(সংরচনা path_cxt *pc, ধ্রুৱ ৱর্ণ *str, ধ্রুৱ ৱর্ণ *path, ...)
{
	ধ্রুৱ ৱর্ণ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ঘূৰাই !p ? -errno : ul_path_write_string(pc, str, p);
}

পূর্ণাঙ্ক ul_path_write_s64(সংরচনা path_cxt *pc, int64_t অঙ্ক, ধ্রুৱ ৱর্ণ *path)
{
	ৱর্ণ buf[পরিমাণ(stringify_মান(LLONG_MAX))];
	পূর্ণাঙ্ক rc, errsv;
	পূর্ণাঙ্ক fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	যদি (fd < 0)
		ঘূৰাই -errno;

	len = snprintf(buf, পরিমাণ(buf), "%" PRId64, অঙ্ক);
	যদি (len < 0 || (size_t) len >= পরিমাণ(buf))
		rc = len < 0 ? -errno : -E2BIG;
	অন্যথা
		rc = ul_vfs_write_all(pc ? pc->vfs : রিক্ত, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : রিক্ত, fd);
	errno = errsv;
	ঘূৰাই rc;
}

পূর্ণাঙ্ক ul_path_write_u64(সংরচনা path_cxt *pc, uint64_t অঙ্ক, ধ্রুৱ ৱর্ণ *path)
{
	ৱর্ণ buf[পরিমাণ(stringify_মান(ULLONG_MAX))];
	পূর্ণাঙ্ক rc, errsv;
	পূর্ণাঙ্ক fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	যদি (fd < 0)
		ঘূৰাই -errno;

	len = snprintf(buf, পরিমাণ(buf), "%" PRIu64, অঙ্ক);
	যদি (len < 0 || (size_t) len >= পরিমাণ(buf))
		rc = len < 0 ? -errno : -E2BIG;
	অন্যথা
		rc = ul_vfs_write_all(pc ? pc->vfs : রিক্ত, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : রিক্ত, fd);
	errno = errsv;
	ঘূৰাই rc;
}

পূর্ণাঙ্ক ul_path_writef_u64(সংরচনা path_cxt *pc, uint64_t অঙ্ক, ধ্রুৱ ৱর্ণ *path, ...)
{
	ধ্রুৱ ৱর্ণ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ঘূৰাই !p ? -errno : ul_path_write_u64(pc, অঙ্ক, p);

}

পূর্ণাঙ্ক ul_path_গুনতি_dirents(সংরচনা path_cxt *pc, ধ্রুৱ ৱর্ণ *path)
{
	DIR *dir;
	পূর্ণাঙ্ক r = 0;

	dir = ul_path_opendir(pc, path);
	যদি (!dir)
		ঘূৰাই 0;

	যেতিয়ালৈ (xreaddir(dir)) r++;

	closedir(dir);
	ঘূৰাই r;
}

পূর্ণাঙ্ক ul_path_countf_dirents(সংরচনা path_cxt *pc, ধ্রুৱ ৱর্ণ *path, ...)
{
	ধ্রুৱ ৱর্ণ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ঘূৰাই !p ? -errno : ul_path_গুনতি_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
পূর্ণাঙ্ক ul_path_next_dirent(সংরচনা path_cxt *pc, DIR **sub, ধ্রুৱ ৱর্ণ *dirname, সংরচনা dirent **d)
{
	যদি (!pc || !sub || !d)
		ঘূৰাই -EINVAL;

	যদি (!*sub) {
		*sub = ul_path_opendir(pc, dirname);
		যদি (!*sub)
			ঘূৰাই -errno;
	}

	*d = xreaddir(*sub);
	যদি (*d)
		ঘূৰাই 0;

	closedir(*sub);
	*sub = রিক্ত;
	ঘূৰাই 1;
}

#ifdef HAVE_CPU_SET_T
স্থির পূর্ণাঙ্ক ul_path_cpuparse(সংরচনা path_cxt *pc, cpu_set_t **set, পূর্ণাঙ্ক maxcpus, পূর্ণাঙ্ক islist, ধ্রুৱ ৱর্ণ *path, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	ৱর্ণ *buf;
	পূর্ণাঙ্ক rc;

	*set = রিক্ত;

	buf = malloc(len);
	যদি (!buf)
		ঘূৰাই -ENOMEM;

	rc = ul_path_vreadf_buffer(pc, buf, len, path, ap);
	যদি (rc < 0)
		গচ্ছ out;

	*set = cpuset_alloc(maxcpus, &setsize, রিক্ত);
	যদি (!*set) {
		rc = -EINVAL;
		গচ্ছ out;
	}

	যদি (islist) {
		যদি (cpulist_parse(buf, *set, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			গচ্ছ out;
		}
	} অন্যথা {
		যদি (cpumask_parse(buf, *set, setsize)) {
			errno = EINVAL;
			rc = -errno;
			গচ্ছ out;
		}
	}
	rc = 0;

out:
	যদি (rc) {
		cpuset_free(*set);
		*set = রিক্ত;
	}
	মুক্ত(buf);
	ঘূৰাই rc;
}

পূর্ণাঙ্ক ul_path_readf_cpuset(সংরচনা path_cxt *pc, cpu_set_t **set, পূর্ণাঙ্ক maxcpus, ধ্রুৱ ৱর্ণ *path, ...)
{
	va_list ap;
	পূর্ণাঙ্ক rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	va_end(ap);

	ঘূৰাই rc;
}

পূর্ণাঙ্ক ul_path_readf_cpulist(সংরচনা path_cxt *pc, cpu_set_t **set, পূর্ণাঙ্ক maxcpus, ধ্রুৱ ৱর্ণ *path, ...)
{
	va_list ap;
	পূর্ণাঙ্ক rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	va_end(ap);

	ঘূৰাই rc;
}

#endif /* HAVE_CPU_SET_T */


#ifdef TEST_PROGRAM_PATH
#অন্তর্ভাৱ <getopt.h>

স্থির শূন্য __attribute__((__noreturn__)) usage(শূন্য)
{
	fprintf(stdout, " %s [options] <dir> <command>\n\n", program_invocation_short_name);
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

	exit(EXIT_SUCCESS);
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ *argv[])
{
	পূর্ণাঙ্ক c;
	ধ্রুৱ ৱর্ণ *prefix = রিক্ত, *dir, *file, *command;
	সংরচনা path_cxt *pc = রিক্ত;

	স্থির ধ্রুৱ সংরচনা option longopts[] = {
		{ "prefix",	1, রিক্ত, 'p' },
		{ "help",       0, রিক্ত, 'h' },
		{ রিক্ত, 0, রিক্ত, 0 },
	};

	যেতিয়ালৈ((c = getopt_long(argc, argv, "p:h", longopts, রিক্ত)) != -1) {
		ৱিকল্প(c) {
		পক্ষ 'p':
			prefix = optarg;
			ভঙ্গ;
		পক্ষ 'h':
			usage();
			ভঙ্গ;
		স্ৱভাৱ:
			err(EXIT_FAILURE, "try --help");
		}
	}

	যদি (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_path_init_debug();

	pc = ul_new_path("%s", dir);
	যদি (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	যদি (prefix)
		ul_path_set_prefix(pc, prefix);

	যদি (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	যদি (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		যদি (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		যদি (ul_path_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %u\n", file, res);

		যদি (ul_path_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %u\n", file, res);

	} অন্যথা যদি (strcmp(command, "read-s32") == 0) {
		int32_t res;

		যদি (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		যদি (ul_path_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %d\n", file, res);

		যদি (ul_path_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %d\n", file, res);

	} অন্যথা যদি (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		যদি (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		যদি (ul_path_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRIu64 "\n", file, res);

		যদি (ul_path_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRIu64 "\n", file, res);

	} অন্যথা যদি (strcmp(command, "read-s64") == 0) {
		int64_t res;

		যদি (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		যদি (ul_path_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRId64 "\n", file, res);

		যদি (ul_path_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRId64 "\n", file, res);

	} অন্যথা যদি (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		যদি (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		যদি (ul_path_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		printf("read:  %s: %d\n", file, (পূর্ণাঙ্ক) res);

		যদি (ul_path_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		printf("readf: %s: %d\n", file, (পূর্ণাঙ্ক) res);

	} অন্যথা যদি (strcmp(command, "read-string") == 0) {
		ৱর্ণ *res;

		যদি (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		যদি (ul_path_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		printf("read:  %s: %s\n", file, res);

		যদি (ul_path_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		printf("readf: %s: %s\n", file, res);

	} অন্যথা যদি (strcmp(command, "read-link") == 0) {
		ৱর্ণ res[PATH_MAX];

		যদি (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		যদি (ul_path_readlink(pc, res, পরিমাণ(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		printf("read:  %s: %s\n", file, res);

		যদি (ul_path_readlinkf(pc, res, পরিমাণ(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		printf("readf: %s: %s\n", file, res);

	} অন্যথা যদি (strcmp(command, "write-string") == 0) {
		ৱর্ণ *str;

		যদি (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		যদি (ul_path_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		যদি (ul_path_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} অন্যথা যদি (strcmp(command, "write-u64") == 0) {
		uint64_t অঙ্ক;

		যদি (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		অঙ্ক = strtoumax(argv[optind++], রিক্ত, 0);

		যদি (ul_path_write_u64(pc, অঙ্ক, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		যদি (ul_path_writef_u64(pc, অঙ্ক, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_path(pc);
	ঘূৰাই EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PATH */
