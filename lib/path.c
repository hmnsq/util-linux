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
#ଅନ୍ତର୍ଭାଵ <stdarg.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <inttypes.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "fileutils.h"
#ଅନ୍ତର୍ଭାଵ "all-io.h"
#ଅନ୍ତର୍ଭାଵ "vfs.h"
#ଅନ୍ତର୍ଭାଵ "path.h"
#ଅନ୍ତର୍ଭାଵ "debug.h"
#ଅନ୍ତର୍ଭାଵ "strutils.h"

/*
 * Debug stuff (based on include/debug.h)
 */
ସ୍ଥିର UL_DEBUG_DEFINE_MASK(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPATH_DEBUG_INIT	(1 << 1)
#define ULPATH_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

ଶୂନ୍ୟ ul_path_init_debug(ଶୂନ୍ୟ)
{
	ଯଦି (ulpath_debug_mask)
		ଫେରନ୍ତୁ;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

ସଂରଚନା path_cxt *ul_new_path(ଅଚଳ ଅକ୍ଷର *dir, ...)
{
	ସଂରଚନା path_cxt *pc = calloc(1, ଆକାର(*pc));

	ଯଦି (!pc)
		ଫେରନ୍ତୁ ଖାଲି;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	ଯଦି (dir) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_path, dir, ap);
		va_end(ap);

		ଯଦି (rc < 0 || !pc->dir_path)
			ଯାଅ fail;
	}
	ଫେରନ୍ତୁ pc;
fail:
	ul_unref_path(pc);
	ଫେରନ୍ତୁ ଖାଲି;
}

ଶୂନ୍ୟ ul_ref_path(ସଂରଚନା path_cxt *pc)
{
	ଯଦି (pc)
		pc->refcount++;
}

ଶୂନ୍ୟ ul_unref_path(ସଂରଚନା path_cxt *pc)
{
	ଯଦି (!pc)
		ଫେରନ୍ତୁ;

	pc->refcount--;

	ଯଦି (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		ଯଦି (pc->dialect)
			pc->free_dialect(pc);
		ul_path_close_dirfd(pc);
		ମୁକ୍ତ(pc->dir_path);
		ମୁକ୍ତ(pc->prefix);
		ମୁକ୍ତ(pc);
	}
}

ଶୂନ୍ୟ ul_path_refer_vfs(ସଂରଚନା path_cxt *pc, ଅଚଳ ସଂରଚନା ul_vfs_ops *vfs)
{
	ଯଦି (pc)
		pc->vfs = vfs;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_set_prefix(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *prefix)
{
	ଅକ୍ଷର *p = ଖାଲି;

	ପ୍ରତିଜ୍ଞା(pc->dir_fd < 0);

	ଯଦି (prefix) {
		p = strdup(prefix);
		ଯଦି (!p)
			ଫେରନ୍ତୁ -ENOMEM;
	}

	ମୁକ୍ତ(pc->prefix);
	pc->prefix = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	ଫେରନ୍ତୁ 0;
}

ଅଚଳ ଅକ୍ଷର *ul_path_get_prefix(ସଂରଚନା path_cxt *pc)
{
	ଫେରନ୍ତୁ pc ? pc->prefix : ଖାଲି;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_set_dir(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *dir)
{
	ଅକ୍ଷର *p = ଖାଲି;

	ଯଦି (dir) {
		p = strdup(dir);
		ଯଦି (!p)
			ଫେରନ୍ତୁ -ENOMEM;
	}

	ଯଦି (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	ମୁକ୍ତ(pc->dir_path);
	pc->dir_path = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	ଫେରନ୍ତୁ 0;
}

ଅଚଳ ଅକ୍ଷର *ul_path_get_dir(ସଂରଚନା path_cxt *pc)
{
	ଫେରନ୍ତୁ pc ? pc->dir_path : ଖାଲି;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_set_dialect(ସଂରଚନା path_cxt *pc, ଶୂନ୍ୟ *data, ଶୂନ୍ୟ free_data(ସଂରଚନା path_cxt *))
{
	pc->dialect = data;
	pc->free_dialect = free_data;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ *ul_path_get_dialect(ସଂରଚନା path_cxt *pc)
{
	ଫେରନ୍ତୁ pc ? pc->dialect : ଖାଲି;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_set_enoent_redirect(ସଂରଚନା path_cxt *pc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*func)(ସଂରଚନା path_cxt *, ଅଚଳ ଅକ୍ଷର *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *))
{
	pc->redirect_on_enoent = func;
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *get_absdir(ସଂରଚନା path_cxt *pc)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	ଅଚଳ ଅକ୍ଷର *dirpath;

	ଯଦି (!pc->prefix)
		ଫେରନ୍ତୁ pc->dir_path;

	dirpath = pc->dir_path;
	ଯଦି (!dirpath)
		ଫେରନ୍ତୁ pc->prefix;
	ଯଦି (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->path_buffer, ଆକାର(pc->path_buffer), "%s/%s", pc->prefix, dirpath);
	ଯଦି (rc < 0)
		ଫେରନ୍ତୁ ଖାଲି;
	ଯଦି ((size_t)rc >= ଆକାର(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ pc->path_buffer;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_is_accessible(ସଂରଚନା path_cxt *pc)
{
	ଅଚଳ ଅକ୍ଷର *path;
	ପ୍ରତିଜ୍ଞା(pc);

	ଯଦି (pc->dir_fd >= 0)
		ଫେରନ୍ତୁ 1;

	path = get_absdir(pc);
	ଯଦି (!path)
		ଫେରନ୍ତୁ 0;
	ଫେରନ୍ତୁ access(path, F_OK) == 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_get_dirfd(ସଂରଚନା path_cxt *pc)
{
	ପ୍ରତିଜ୍ଞା(pc);
	ପ୍ରତିଜ୍ଞା(pc->dir_path);

	ଯଦି (pc->dir_fd < 0) {
		ଅଚଳ ଅକ୍ଷର *path = get_absdir(pc);
		ଯଦି (!path)
			ଫେରନ୍ତୁ -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", path));
		pc->dir_fd = open(path, O_RDONLY|O_CLOEXEC);
	}

	ଫେରନ୍ତୁ pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
ଶୂନ୍ୟ ul_path_close_dirfd(ସଂରଚନା path_cxt *pc)
{
	ପ୍ରତିଜ୍ଞା(pc);

	ଯଦି (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_isopen_dirfd(ସଂରଚନା path_cxt *pc)
{
	ଫେରନ୍ତୁ pc && pc->dir_fd >= 0;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ul_path_mkpath(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *path, va_list ap)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	errno = 0;

	rc = vsnprintf(pc->path_buffer, ଆକାର(pc->path_buffer), path, ap);
	ଯଦି (rc < 0) {
		ଯଦି (!errno)
			errno = EINVAL;
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଯଦି ((size_t)rc >= ଆକାର(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ pc->path_buffer;
}

ଅକ୍ଷର *ul_path_get_abspath(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର *buf, size_t bufsz, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	ଯଦି (path) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
		va_list ap;
		ଅଚଳ ଅକ୍ଷର *tail = ଖାଲି, *dirpath = pc->dir_path;

		va_start(ap, path);
		tail = ul_path_mkpath(pc, path, ap);
		va_end(ap);

		ଯଦି (dirpath && *dirpath == '/')
			dirpath++;
		ଯଦି (tail && *tail == '/')
			tail++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->prefix ? pc->prefix : "",
				dirpath ? dirpath : "",
				tail ? tail : "");

		ଯଦି ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			ଫେରନ୍ତୁ ଖାଲି;
		}
	} ଅନ୍ୟଥା {
		ଅଚଳ ଅକ୍ଷର *tmp = get_absdir(pc);

		ଯଦି (!tmp)
			ଫେରନ୍ତୁ ଖାଲି;
		xstrncpy(buf, tmp, bufsz);
	}

	ଫେରନ୍ତୁ buf;
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_access(ସଂରଚନା path_cxt *pc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode, ଅଚଳ ଅକ୍ଷର *path)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	ଯଦି (!path)
		ଫେରନ୍ତୁ -EINVAL;
	ଯଦି (!pc) {
		rc = access(path, mode);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", path, rc));
	} ଅନ୍ୟଥା {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir = ul_path_get_dirfd(pc);
		ଯଦି (dir < 0)
			ଫେରନ୍ତୁ dir;
		ଯଦି (*path == '/')
			path++;

		rc = faccessat(dir, path, mode, 0);

		ଯଦି (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = faccessat(dir, path, mode, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", path, rc));
	}
	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_accessf(ସଂରଚନା path_cxt *pc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	va_list ap;
	ଅଚଳ ଅକ୍ଷର *p;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ !p ? -errno : ul_path_access(pc, mode, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_stat(ସଂରଚନା path_cxt *pc, ସଂରଚନା stat *sb, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ଅଚଳ ଅକ୍ଷର *path)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	ଯଦି (!pc) {
		rc = path ? stat(path, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", path, rc));
	} ଅନ୍ୟଥା {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir = ul_path_get_dirfd(pc);
		ଯଦି (dir < 0)
			ଫେରନ୍ତୁ dir;
		ଯଦି (path) {
			ଯଦି  (*path == '/')
				path++;
			rc = fstatat(dir, path, sb, flags);

		} ଅନ୍ୟଥା
			rc = fstat(dir, sb);	/* dir itself */

		ଯଦି (rc && errno == ENOENT
		    && path
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = fstatat(dir, path, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", path, rc));
	}
	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_vstatf(ସଂରଚନା path_cxt *pc, ସଂରଚନା stat *sb, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ଅଚଳ ଅକ୍ଷର *path, va_list ap)
{
	ଅଚଳ ଅକ୍ଷର *p = ul_path_mkpath(pc, path, ap);

	ଫେରନ୍ତୁ !p ? -errno : ul_path_stat(pc, sb, flags, p);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_statf(ସଂରଚନା path_cxt *pc, ସଂରଚନା stat *sb, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	va_list ap;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	va_start(ap, path);
	rc = ul_path_vstatf(pc, sb, flags, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ rc;
}
#ifdef HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_statx(ସଂରଚନା path_cxt *pc, ସଂରଚନା statx *stx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask,
		  ଅଚଳ ଅକ୍ଷର *path)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	ଯଦି (!path)
		ଫେରନ୍ତୁ -EINVAL;
	ଯଦି (!pc)
		rc = path ? statx(AT_FDCWD, path, flags, mask, stx) : - EINVAL;
	ଅନ୍ୟଥା {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir = ul_path_get_dirfd(pc);
		ଯଦି (dir < 0)
			ଫେରନ୍ତୁ dir;
		ଯଦି (*path == '/')
			path++;

		rc = statx(dir, path, flags, mask, stx);
	}

	ଫେରନ୍ତୁ rc;
}
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_statx(ସଂରଚନା path_cxt *pc __attribute__((__unused__)),
		  ସଂରଚନା statx *stx __attribute__((__unused__)),
		  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags __attribute__((__unused__)),
		  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask __attribute__((__unused__)),
		  ଅଚଳ ଅକ୍ଷର *path __attribute__((__unused__)))
{
	errno = ENOSYS;
	ଫେରନ୍ତୁ -1;
}
#endif /* HAVE_STATX */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_vstatxf(ସଂରଚନା path_cxt *pc, ସଂରଚନା statx *stx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask,
		    ଅଚଳ ଅକ୍ଷର *path, va_list ap)
{
	ଅଚଳ ଅକ୍ଷର *p = ul_path_mkpath(pc, path, ap);

	ଫେରନ୍ତୁ !p ? -errno : ul_path_statx(pc, stx, flags, mask, p);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_statxf(ସଂରଚନା path_cxt *pc, ସଂରଚନା statx *stx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask,
		   ଅଚଳ ଅକ୍ଷର *path, ...)
{
	va_list ap;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	va_start(ap, path);
	rc = ul_path_vstatxf(pc, stx, flags, mask, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_open(ସଂରଚନା path_cxt *pc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ଅଚଳ ଅକ୍ଷର *path)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;

	ଯଦି (!path)
		ଫେରନ୍ତୁ -EINVAL;
	ଯଦି (!pc) {
		fd = open(path, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", path));
	} ଅନ୍ୟଥା {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fdx;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir = ul_path_get_dirfd(pc);
		ଯଦି (dir < 0)
			ଫେରନ୍ତୁ dir;

		ଯଦି (*path == '/')
			path++;

		fdx = fd = openat(dir, path, flags);

		ଯଦି (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			fd = openat(dir, path, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", path, fdx != fd ? " [redirected]" : ""));
	}
	ଫେରନ୍ତୁ fd;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_vopenf(ସଂରଚନା path_cxt *pc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ଅଚଳ ଅକ୍ଷର *path, va_list ap)
{
	ଅଚଳ ଅକ୍ଷର *p = ul_path_mkpath(pc, path, ap);

	ଫେରନ୍ତୁ !p ? -errno : ul_path_open(pc, flags, p);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_openf(ସଂରଚନା path_cxt *pc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	va_list ap;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	va_start(ap, path);
	rc = ul_path_vopenf(pc, flags, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_path_fopen(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *mode, ଅଚଳ ଅକ୍ଷର *path)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = ul_mode_to_flags(mode);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = ul_path_open(pc, flags, path);

	ଯଦି (fd < 0)
		ଫେରନ୍ତୁ ଖାଲି;

	ଫେରନ୍ତୁ ul_vfs_fdopen(pc ? pc->vfs : ଖାଲି, fd, mode);
}


FILE *ul_path_vfopenf(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *mode, ଅଚଳ ଅକ୍ଷର *path, va_list ap)
{
	ଅଚଳ ଅକ୍ଷର *p = ul_path_mkpath(pc, path, ap);

	ଫେରନ୍ତୁ !p ? ଖାଲି : ul_path_fopen(pc, mode, p);
}

FILE *ul_path_fopenf(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *mode, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, path);
	f = ul_path_vfopenf(pc, mode, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendir(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *path)
{
	DIR *dir;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = -1;

	ଯଦି (path)
		fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	ଅନ୍ୟଥା ଯଦି (pc->dir_path) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_path_get_dirfd(pc);
		ଯଦି (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	ଯଦି (fd < 0)
		ଫେରନ୍ତୁ ଖାଲି;

	dir = fdopendir(fd);
	ଯଦି (!dir) {
		close(fd);
		ଫେରନ୍ତୁ ଖାଲି;
	}
	ଯଦି (!path)
		 rewinddir(dir);
	ଫେରନ୍ତୁ dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_vopendirf(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *path, va_list ap)
{
	ଅଚଳ ଅକ୍ଷର *p = ul_path_mkpath(pc, path, ap);

	ଫେରନ୍ତୁ !p ? ଖାଲି : ul_path_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendirf(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, path);
	dir = ul_path_vopendirf(pc, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlink(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର *buf, size_t bufsiz, ଅଚଳ ଅକ୍ଷର *path)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirfd;
	ssize_t ssz;

	ଯଦି (!path) {
		ଅଚଳ ଅକ୍ଷର *p = get_absdir(pc);
		ଯଦି (!p)
			ଫେରନ୍ତୁ -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} ଅନ୍ୟଥା {
		dirfd = ul_path_get_dirfd(pc);
		ଯଦି (dirfd < 0)
			ଫେରନ୍ତୁ dirfd;

		ଯଦି (*path == '/')
			path++;

		ssz = readlinkat(dirfd, path, buf, bufsiz - 1);
	}

	ଯଦି (ssz >= 0)
		buf[ssz] = '\0';
	ଫେରନ୍ତୁ ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlinkf(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର *buf, size_t bufsiz, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	ଅଚଳ ଅକ୍ଷର *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ !p ? -errno : ul_path_readlink(pc, buf, bufsiz, p);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_read(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର *buf, size_t len, ଅଚଳ ଅକ୍ଷର *path)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, errsv;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;

	fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	ଯଦି (fd < 0)
		ଫେରନ୍ତୁ -errno;

	DBG(CXT, ul_debug(" reading '%s'", path));
	rc = ul_vfs_read_all(pc ? pc->vfs : ଖାଲି, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ଖାଲି, fd);
	errno = errsv;
	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_vreadf(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର *buf, size_t len, ଅଚଳ ଅକ୍ଷର *path, va_list ap)
{
	ଅଚଳ ଅକ୍ଷର *p = ul_path_mkpath(pc, path, ap);

	ଫେରନ୍ତୁ !p ? -errno : ul_path_read(pc, buf, len, p);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_readf(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର *buf, size_t len, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	va_list ap;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	va_start(ap, path);
	rc = ul_path_vreadf(pc, buf, len, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_read_string(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର **str, ଅଚଳ ଅକ୍ଷର *path)
{
	ଅକ୍ଷର buf[BUFSIZ];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	ଯଦି (!str)
		ଫେରନ୍ତୁ -EINVAL;

	*str = ଖାଲି;

	rc = ul_path_read_buffer(pc, buf, ଆକାର(buf), path);
	ଯଦି (rc < 0)
		ଫେରନ୍ତୁ rc;

	*str = strdup(buf);
	ଯଦି (!*str)
		rc = -ENOMEM;

	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_readf_string(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର **str, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	ଅଚଳ ଅକ୍ଷର *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ !p ? -errno : ul_path_read_string(pc, str, p);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_read_buffer(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର *buf, size_t bufsz, ଅଚଳ ଅକ୍ଷର *path)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = ul_path_read(pc, buf, bufsz - 1, path);

	ଯଦି (rc == 0)
		buf[0] = '\0';

	ଅନ୍ୟଥା ଯଦି (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		ଯଦି (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		ଅନ୍ୟଥା
			buf[rc] = '\0';
	}

	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_vreadf_buffer(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର *buf, size_t bufsz, ଅଚଳ ଅକ୍ଷର *path, va_list ap)
{
	ଅଚଳ ଅକ୍ଷର *p;

	p = ul_path_mkpath(pc, path, ap);

	ଫେରନ୍ତୁ !p ? -errno : ul_path_read_buffer(pc, buf, bufsz, p);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_readf_buffer(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର *buf, size_t bufsz, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	va_list ap;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	va_start(ap, path);
	rc = ul_path_vreadf_buffer(pc, buf, bufsz, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_scanf(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *path, ଅଚଳ ଅକ୍ଷର *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	f = ul_path_fopen(pc, "r" UL_CLOEXECSTR, path);
	ଯଦି (!f)
		ଫେରନ୍ତୁ -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, path));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_scanff(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *path, va_list ap, ଅଚଳ ଅକ୍ଷର *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	ଯଦି (!f)
		ଫେରନ୍ତୁ -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	ଫେରନ୍ତୁ rc;
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_read_s64(ସଂରଚନା path_cxt *pc, int64_t *res, ଅଚଳ ଅକ୍ଷର *path)
{
	int64_t x = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	rc = ul_path_scanf(pc, path, "%"SCNd64, &x);
	ଯଦି (rc != 1)
		ଫେରନ୍ତୁ -1;
	ଯଦି (res)
		*res = x;
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_readf_s64(ସଂରଚନା path_cxt *pc, int64_t *res, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	ଅଚଳ ଅକ୍ଷର *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ !p ? -errno : ul_path_read_s64(pc, res, p);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_read_u64(ସଂରଚନା path_cxt *pc, uint64_t *res, ଅଚଳ ଅକ୍ଷର *path)
{
	uint64_t x = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	rc = ul_path_scanf(pc, path, "%"SCNu64, &x);
	ଯଦି (rc != 1)
		ଫେରନ୍ତୁ -1;
	ଯଦି (res)
		*res = x;
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_readf_u64(ସଂରଚନା path_cxt *pc, uint64_t *res, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	ଅଚଳ ଅକ୍ଷର *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ !p ? -errno : ul_path_read_u64(pc, res, p);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_read_s32(ସଂରଚନା path_cxt *pc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *res, ଅଚଳ ଅକ୍ଷର *path)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, x = 0;

	rc = ul_path_scanf(pc, path, "%d", &x);
	ଯଦି (rc != 1)
		ଫେରନ୍ତୁ -1;
	ଯଦି (res)
		*res = x;
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_readf_s32(ସଂରଚନା path_cxt *pc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *res, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	ଅଚଳ ଅକ୍ଷର *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ !p ? -errno : ul_path_read_s32(pc, res, p);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_read_u32(ସଂରଚନା path_cxt *pc, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *res, ଅଚଳ ଅକ୍ଷର *path)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା x = 0;

	rc = ul_path_scanf(pc, path, "%u", &x);
	ଯଦି (rc != 1)
		ଫେରନ୍ତୁ -1;
	ଯଦି (res)
		*res = x;
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_readf_u32(ସଂରଚନା path_cxt *pc, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *res, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	ଅଚଳ ଅକ୍ଷର *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ !p ? -errno : ul_path_read_u32(pc, res, p);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_read_majmin(ସଂରଚନା path_cxt *pc, dev_t *res, ଅଚଳ ଅକ୍ଷର *path)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, maj = 0, min = 0;

	rc = ul_path_scanf(pc, path, "%d:%d", &maj, &min);
	ଯଦି (rc != 2)
		ଫେରନ୍ତୁ -1;
	ଯଦି (res)
		*res = makedev(maj, min);
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_readf_majmin(ସଂରଚନା path_cxt *pc, dev_t *res, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	ଅଚଳ ଅକ୍ଷର *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ !p ? -errno : ul_path_read_majmin(pc, res, p);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_write_string(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *str, ଅଚଳ ଅକ୍ଷର *path)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, errsv;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	ଯଦି (fd < 0)
		ଫେରନ୍ତୁ -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : ଖାଲି, fd, str, strlen(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ଖାଲି, fd);
	errno = errsv;
	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_writef_string(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *str, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	ଅଚଳ ଅକ୍ଷର *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ !p ? -errno : ul_path_write_string(pc, str, p);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_write_s64(ସଂରଚନା path_cxt *pc, int64_t ଅଙ୍କ, ଅଚଳ ଅକ୍ଷର *path)
{
	ଅକ୍ଷର buf[ଆକାର(stringify_ମୂଲ୍ୟ(LLONG_MAX))];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, errsv;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	ଯଦି (fd < 0)
		ଫେରନ୍ତୁ -errno;

	len = snprintf(buf, ଆକାର(buf), "%" PRId64, ଅଙ୍କ);
	ଯଦି (len < 0 || (size_t) len >= ଆକାର(buf))
		rc = len < 0 ? -errno : -E2BIG;
	ଅନ୍ୟଥା
		rc = ul_vfs_write_all(pc ? pc->vfs : ଖାଲି, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ଖାଲି, fd);
	errno = errsv;
	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_write_u64(ସଂରଚନା path_cxt *pc, uint64_t ଅଙ୍କ, ଅଚଳ ଅକ୍ଷର *path)
{
	ଅକ୍ଷର buf[ଆକାର(stringify_ମୂଲ୍ୟ(ULLONG_MAX))];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, errsv;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	ଯଦି (fd < 0)
		ଫେରନ୍ତୁ -errno;

	len = snprintf(buf, ଆକାର(buf), "%" PRIu64, ଅଙ୍କ);
	ଯଦି (len < 0 || (size_t) len >= ଆକାର(buf))
		rc = len < 0 ? -errno : -E2BIG;
	ଅନ୍ୟଥା
		rc = ul_vfs_write_all(pc ? pc->vfs : ଖାଲି, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ଖାଲି, fd);
	errno = errsv;
	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_writef_u64(ସଂରଚନା path_cxt *pc, uint64_t ଅଙ୍କ, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	ଅଚଳ ଅକ୍ଷର *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ !p ? -errno : ul_path_write_u64(pc, ଅଙ୍କ, p);

}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_ଗଣନା_dirents(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *path)
{
	DIR *dir;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା r = 0;

	dir = ul_path_opendir(pc, path);
	ଯଦି (!dir)
		ଫେରନ୍ତୁ 0;

	ଯେତେବେଳେ (xreaddir(dir)) r++;

	closedir(dir);
	ଫେରନ୍ତୁ r;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_countf_dirents(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	ଅଚଳ ଅକ୍ଷର *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ !p ? -errno : ul_path_ଗଣନା_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_next_dirent(ସଂରଚନା path_cxt *pc, DIR **sub, ଅଚଳ ଅକ୍ଷର *dirname, ସଂରଚନା dirent **d)
{
	ଯଦି (!pc || !sub || !d)
		ଫେରନ୍ତୁ -EINVAL;

	ଯଦି (!*sub) {
		*sub = ul_path_opendir(pc, dirname);
		ଯଦି (!*sub)
			ଫେରନ୍ତୁ -errno;
	}

	*d = xreaddir(*sub);
	ଯଦି (*d)
		ଫେରନ୍ତୁ 0;

	closedir(*sub);
	*sub = ଖାଲି;
	ଫେରନ୍ତୁ 1;
}

#ifdef HAVE_CPU_SET_T
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_cpuparse(ସଂରଚନା path_cxt *pc, cpu_set_t **set, ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxcpus, ପୂର୍ଣ୍ଣସଂଖ୍ୟା islist, ଅଚଳ ଅକ୍ଷର *path, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	ଅକ୍ଷର *buf;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	*set = ଖାଲି;

	buf = malloc(len);
	ଯଦି (!buf)
		ଫେରନ୍ତୁ -ENOMEM;

	rc = ul_path_vreadf_buffer(pc, buf, len, path, ap);
	ଯଦି (rc < 0)
		ଯାଅ out;

	*set = cpuset_alloc(maxcpus, &setsize, ଖାଲି);
	ଯଦି (!*set) {
		rc = -EINVAL;
		ଯାଅ out;
	}

	ଯଦି (islist) {
		ଯଦି (cpulist_parse(buf, *set, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			ଯାଅ out;
		}
	} ଅନ୍ୟଥା {
		ଯଦି (cpumask_parse(buf, *set, setsize)) {
			errno = EINVAL;
			rc = -errno;
			ଯାଅ out;
		}
	}
	rc = 0;

out:
	ଯଦି (rc) {
		cpuset_free(*set);
		*set = ଖାଲି;
	}
	ମୁକ୍ତ(buf);
	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_readf_cpuset(ସଂରଚନା path_cxt *pc, cpu_set_t **set, ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxcpus, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	va_list ap;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_readf_cpulist(ସଂରଚନା path_cxt *pc, cpu_set_t **set, ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxcpus, ଅଚଳ ଅକ୍ଷର *path, ...)
{
	va_list ap;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	va_end(ap);

	ଫେରନ୍ତୁ rc;
}

#endif /* HAVE_CPU_SET_T */


#ifdef TEST_PROGRAM_PATH
#ଅନ୍ତର୍ଭାଵ <getopt.h>

ସ୍ଥିର ଶୂନ୍ୟ __attribute__((__noreturn__)) usage(ଶୂନ୍ୟ)
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

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;
	ଅଚଳ ଅକ୍ଷର *prefix = ଖାଲି, *dir, *file, *command;
	ସଂରଚନା path_cxt *pc = ଖାଲି;

	ସ୍ଥିର ଅଚଳ ସଂରଚନା option longopts[] = {
		{ "prefix",	1, ଖାଲି, 'p' },
		{ "help",       0, ଖାଲି, 'h' },
		{ ଖାଲି, 0, ଖାଲି, 0 },
	};

	ଯେତେବେଳେ((c = getopt_long(argc, argv, "p:h", longopts, ଖାଲି)) != -1) {
		ବାଛ(c) {
		ସ୍ଥିତି 'p':
			prefix = optarg;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'h':
			usage();
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			err(EXIT_FAILURE, "try --help");
		}
	}

	ଯଦି (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_path_init_debug();

	pc = ul_new_path("%s", dir);
	ଯଦି (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	ଯଦି (prefix)
		ul_path_set_prefix(pc, prefix);

	ଯଦି (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	ଯଦି (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		ଯଦି (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ଯଦି (ul_path_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %u\n", file, res);

		ଯଦି (ul_path_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %u\n", file, res);

	} ଅନ୍ୟଥା ଯଦି (strcmp(command, "read-s32") == 0) {
		int32_t res;

		ଯଦି (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ଯଦି (ul_path_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %d\n", file, res);

		ଯଦି (ul_path_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %d\n", file, res);

	} ଅନ୍ୟଥା ଯଦି (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		ଯଦି (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ଯଦି (ul_path_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRIu64 "\n", file, res);

		ଯଦି (ul_path_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRIu64 "\n", file, res);

	} ଅନ୍ୟଥା ଯଦି (strcmp(command, "read-s64") == 0) {
		int64_t res;

		ଯଦି (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ଯଦି (ul_path_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRId64 "\n", file, res);

		ଯଦି (ul_path_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRId64 "\n", file, res);

	} ଅନ୍ୟଥା ଯଦି (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		ଯଦି (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ଯଦି (ul_path_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		printf("read:  %s: %d\n", file, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) res);

		ଯଦି (ul_path_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		printf("readf: %s: %d\n", file, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) res);

	} ଅନ୍ୟଥା ଯଦି (strcmp(command, "read-string") == 0) {
		ଅକ୍ଷର *res;

		ଯଦି (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ଯଦି (ul_path_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		printf("read:  %s: %s\n", file, res);

		ଯଦି (ul_path_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		printf("readf: %s: %s\n", file, res);

	} ଅନ୍ୟଥା ଯଦି (strcmp(command, "read-link") == 0) {
		ଅକ୍ଷର res[PATH_MAX];

		ଯଦି (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ଯଦି (ul_path_readlink(pc, res, ଆକାର(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		printf("read:  %s: %s\n", file, res);

		ଯଦି (ul_path_readlinkf(pc, res, ଆକାର(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		printf("readf: %s: %s\n", file, res);

	} ଅନ୍ୟଥା ଯଦି (strcmp(command, "write-string") == 0) {
		ଅକ୍ଷର *str;

		ଯଦି (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		ଯଦି (ul_path_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		ଯଦି (ul_path_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} ଅନ୍ୟଥା ଯଦି (strcmp(command, "write-u64") == 0) {
		uint64_t ଅଙ୍କ;

		ଯଦି (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		ଅଙ୍କ = strtoumax(argv[optind++], ଖାଲି, 0);

		ଯଦି (ul_path_write_u64(pc, ଅଙ୍କ, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		ଯଦି (ul_path_writef_u64(pc, ଅଙ୍କ, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_path(pc);
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PATH */
