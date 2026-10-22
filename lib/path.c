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
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <inttypes.h>
#अन्तर्भाव <errno.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "fileutils.h"
#अन्तर्भाव "all-io.h"
#अन्तर्भाव "vfs.h"
#अन्तर्भाव "path.h"
#अन्तर्भाव "debug.h"
#अन्तर्भाव "strutils.h"

/*
 * Debug stuff (based on include/debug.h)
 */
स्थिर UL_DEBUG_DEFINE_MASK(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPATH_DEBUG_INIT	(1 << 1)
#define ULPATH_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

शून्य ul_path_init_debug(शून्य)
{
	यदि (ulpath_debug_mask)
		प्रत्यागम;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

संरचना path_cxt *ul_new_path(ध्रुव वर्ण *dir, ...)
{
	संरचना path_cxt *pc = calloc(1, परिमाण(*pc));

	यदि (!pc)
		प्रत्यागम रिक्त;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	यदि (dir) {
		पूर्णाङ्क rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_path, dir, ap);
		va_end(ap);

		यदि (rc < 0 || !pc->dir_path)
			गच्छ fail;
	}
	प्रत्यागम pc;
fail:
	ul_unref_path(pc);
	प्रत्यागम रिक्त;
}

शून्य ul_ref_path(संरचना path_cxt *pc)
{
	यदि (pc)
		pc->refcount++;
}

शून्य ul_unref_path(संरचना path_cxt *pc)
{
	यदि (!pc)
		प्रत्यागम;

	pc->refcount--;

	यदि (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		यदि (pc->dialect)
			pc->free_dialect(pc);
		ul_path_close_dirfd(pc);
		मुक्त(pc->dir_path);
		मुक्त(pc->prefix);
		मुक्त(pc);
	}
}

शून्य ul_path_refer_vfs(संरचना path_cxt *pc, ध्रुव संरचना ul_vfs_ops *vfs)
{
	यदि (pc)
		pc->vfs = vfs;
}

पूर्णाङ्क ul_path_set_prefix(संरचना path_cxt *pc, ध्रुव वर्ण *prefix)
{
	वर्ण *p = रिक्त;

	प्रतिज्ञा(pc->dir_fd < 0);

	यदि (prefix) {
		p = strdup(prefix);
		यदि (!p)
			प्रत्यागम -ENOMEM;
	}

	मुक्त(pc->prefix);
	pc->prefix = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	प्रत्यागम 0;
}

ध्रुव वर्ण *ul_path_get_prefix(संरचना path_cxt *pc)
{
	प्रत्यागम pc ? pc->prefix : रिक्त;
}

पूर्णाङ्क ul_path_set_dir(संरचना path_cxt *pc, ध्रुव वर्ण *dir)
{
	वर्ण *p = रिक्त;

	यदि (dir) {
		p = strdup(dir);
		यदि (!p)
			प्रत्यागम -ENOMEM;
	}

	यदि (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	मुक्त(pc->dir_path);
	pc->dir_path = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	प्रत्यागम 0;
}

ध्रुव वर्ण *ul_path_get_dir(संरचना path_cxt *pc)
{
	प्रत्यागम pc ? pc->dir_path : रिक्त;
}

पूर्णाङ्क ul_path_set_dialect(संरचना path_cxt *pc, शून्य *data, शून्य free_data(संरचना path_cxt *))
{
	pc->dialect = data;
	pc->free_dialect = free_data;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	प्रत्यागम 0;
}

शून्य *ul_path_get_dialect(संरचना path_cxt *pc)
{
	प्रत्यागम pc ? pc->dialect : रिक्त;
}

पूर्णाङ्क ul_path_set_enoent_redirect(संरचना path_cxt *pc, पूर्णाङ्क (*func)(संरचना path_cxt *, ध्रुव वर्ण *, पूर्णाङ्क *))
{
	pc->redirect_on_enoent = func;
	प्रत्यागम 0;
}

स्थिर ध्रुव वर्ण *get_absdir(संरचना path_cxt *pc)
{
	पूर्णाङ्क rc;
	ध्रुव वर्ण *dirpath;

	यदि (!pc->prefix)
		प्रत्यागम pc->dir_path;

	dirpath = pc->dir_path;
	यदि (!dirpath)
		प्रत्यागम pc->prefix;
	यदि (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->path_buffer, परिमाण(pc->path_buffer), "%s/%s", pc->prefix, dirpath);
	यदि (rc < 0)
		प्रत्यागम रिक्त;
	यदि ((size_t)rc >= परिमाण(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		प्रत्यागम रिक्त;
	}

	प्रत्यागम pc->path_buffer;
}

पूर्णाङ्क ul_path_is_accessible(संरचना path_cxt *pc)
{
	ध्रुव वर्ण *path;
	प्रतिज्ञा(pc);

	यदि (pc->dir_fd >= 0)
		प्रत्यागम 1;

	path = get_absdir(pc);
	यदि (!path)
		प्रत्यागम 0;
	प्रत्यागम access(path, F_OK) == 0;
}

पूर्णाङ्क ul_path_get_dirfd(संरचना path_cxt *pc)
{
	प्रतिज्ञा(pc);
	प्रतिज्ञा(pc->dir_path);

	यदि (pc->dir_fd < 0) {
		ध्रुव वर्ण *path = get_absdir(pc);
		यदि (!path)
			प्रत्यागम -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", path));
		pc->dir_fd = open(path, O_RDONLY|O_CLOEXEC);
	}

	प्रत्यागम pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
शून्य ul_path_close_dirfd(संरचना path_cxt *pc)
{
	प्रतिज्ञा(pc);

	यदि (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

पूर्णाङ्क ul_path_isopen_dirfd(संरचना path_cxt *pc)
{
	प्रत्यागम pc && pc->dir_fd >= 0;
}

स्थिर ध्रुव वर्ण *ul_path_mkpath(संरचना path_cxt *pc, ध्रुव वर्ण *path, va_list ap)
{
	पूर्णाङ्क rc;

	errno = 0;

	rc = vsnprintf(pc->path_buffer, परिमाण(pc->path_buffer), path, ap);
	यदि (rc < 0) {
		यदि (!errno)
			errno = EINVAL;
		प्रत्यागम रिक्त;
	}

	यदि ((size_t)rc >= परिमाण(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		प्रत्यागम रिक्त;
	}

	प्रत्यागम pc->path_buffer;
}

वर्ण *ul_path_get_abspath(संरचना path_cxt *pc, वर्ण *buf, size_t bufsz, ध्रुव वर्ण *path, ...)
{
	यदि (path) {
		पूर्णाङ्क rc;
		va_list ap;
		ध्रुव वर्ण *tail = रिक्त, *dirpath = pc->dir_path;

		va_start(ap, path);
		tail = ul_path_mkpath(pc, path, ap);
		va_end(ap);

		यदि (dirpath && *dirpath == '/')
			dirpath++;
		यदि (tail && *tail == '/')
			tail++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->prefix ? pc->prefix : "",
				dirpath ? dirpath : "",
				tail ? tail : "");

		यदि ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			प्रत्यागम रिक्त;
		}
	} अन्यथा {
		ध्रुव वर्ण *tmp = get_absdir(pc);

		यदि (!tmp)
			प्रत्यागम रिक्त;
		xstrncpy(buf, tmp, bufsz);
	}

	प्रत्यागम buf;
}


पूर्णाङ्क ul_path_access(संरचना path_cxt *pc, पूर्णाङ्क mode, ध्रुव वर्ण *path)
{
	पूर्णाङ्क rc;

	यदि (!path)
		प्रत्यागम -EINVAL;
	यदि (!pc) {
		rc = access(path, mode);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", path, rc));
	} अन्यथा {
		पूर्णाङ्क dir = ul_path_get_dirfd(pc);
		यदि (dir < 0)
			प्रत्यागम dir;
		यदि (*path == '/')
			path++;

		rc = faccessat(dir, path, mode, 0);

		यदि (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = faccessat(dir, path, mode, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", path, rc));
	}
	प्रत्यागम rc;
}

पूर्णाङ्क ul_path_accessf(संरचना path_cxt *pc, पूर्णाङ्क mode, ध्रुव वर्ण *path, ...)
{
	va_list ap;
	ध्रुव वर्ण *p;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_path_access(pc, mode, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
पूर्णाङ्क ul_path_stat(संरचना path_cxt *pc, संरचना stat *sb, पूर्णाङ्क flags, ध्रुव वर्ण *path)
{
	पूर्णाङ्क rc;

	यदि (!pc) {
		rc = path ? stat(path, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", path, rc));
	} अन्यथा {
		पूर्णाङ्क dir = ul_path_get_dirfd(pc);
		यदि (dir < 0)
			प्रत्यागम dir;
		यदि (path) {
			यदि  (*path == '/')
				path++;
			rc = fstatat(dir, path, sb, flags);

		} अन्यथा
			rc = fstat(dir, sb);	/* dir itself */

		यदि (rc && errno == ENOENT
		    && path
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = fstatat(dir, path, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", path, rc));
	}
	प्रत्यागम rc;
}

पूर्णाङ्क ul_path_vstatf(संरचना path_cxt *pc, संरचना stat *sb, पूर्णाङ्क flags, ध्रुव वर्ण *path, va_list ap)
{
	ध्रुव वर्ण *p = ul_path_mkpath(pc, path, ap);

	प्रत्यागम !p ? -errno : ul_path_stat(pc, sb, flags, p);
}

पूर्णाङ्क ul_path_statf(संरचना path_cxt *pc, संरचना stat *sb, पूर्णाङ्क flags, ध्रुव वर्ण *path, ...)
{
	va_list ap;
	पूर्णाङ्क rc;

	va_start(ap, path);
	rc = ul_path_vstatf(pc, sb, flags, path, ap);
	va_end(ap);

	प्रत्यागम rc;
}
#ifdef HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
पूर्णाङ्क ul_path_statx(संरचना path_cxt *pc, संरचना statx *stx, पूर्णाङ्क flags, unsigned पूर्णाङ्क mask,
		  ध्रुव वर्ण *path)
{
	पूर्णाङ्क rc;

	यदि (!path)
		प्रत्यागम -EINVAL;
	यदि (!pc)
		rc = path ? statx(AT_FDCWD, path, flags, mask, stx) : - EINVAL;
	अन्यथा {
		पूर्णाङ्क dir = ul_path_get_dirfd(pc);
		यदि (dir < 0)
			प्रत्यागम dir;
		यदि (*path == '/')
			path++;

		rc = statx(dir, path, flags, mask, stx);
	}

	प्रत्यागम rc;
}
#अन्यथा
पूर्णाङ्क ul_path_statx(संरचना path_cxt *pc __attribute__((__unused__)),
		  संरचना statx *stx __attribute__((__unused__)),
		  पूर्णाङ्क flags __attribute__((__unused__)),
		  unsigned पूर्णाङ्क mask __attribute__((__unused__)),
		  ध्रुव वर्ण *path __attribute__((__unused__)))
{
	errno = ENOSYS;
	प्रत्यागम -1;
}
#endif /* HAVE_STATX */

पूर्णाङ्क ul_path_vstatxf(संरचना path_cxt *pc, संरचना statx *stx, पूर्णाङ्क flags, unsigned पूर्णाङ्क mask,
		    ध्रुव वर्ण *path, va_list ap)
{
	ध्रुव वर्ण *p = ul_path_mkpath(pc, path, ap);

	प्रत्यागम !p ? -errno : ul_path_statx(pc, stx, flags, mask, p);
}

पूर्णाङ्क ul_path_statxf(संरचना path_cxt *pc, संरचना statx *stx, पूर्णाङ्क flags, unsigned पूर्णाङ्क mask,
		   ध्रुव वर्ण *path, ...)
{
	va_list ap;
	पूर्णाङ्क rc;

	va_start(ap, path);
	rc = ul_path_vstatxf(pc, stx, flags, mask, path, ap);
	va_end(ap);

	प्रत्यागम rc;
}

पूर्णाङ्क ul_path_open(संरचना path_cxt *pc, पूर्णाङ्क flags, ध्रुव वर्ण *path)
{
	पूर्णाङ्क fd;

	यदि (!path)
		प्रत्यागम -EINVAL;
	यदि (!pc) {
		fd = open(path, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", path));
	} अन्यथा {
		पूर्णाङ्क fdx;
		पूर्णाङ्क dir = ul_path_get_dirfd(pc);
		यदि (dir < 0)
			प्रत्यागम dir;

		यदि (*path == '/')
			path++;

		fdx = fd = openat(dir, path, flags);

		यदि (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			fd = openat(dir, path, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", path, fdx != fd ? " [redirected]" : ""));
	}
	प्रत्यागम fd;
}

पूर्णाङ्क ul_path_vopenf(संरचना path_cxt *pc, पूर्णाङ्क flags, ध्रुव वर्ण *path, va_list ap)
{
	ध्रुव वर्ण *p = ul_path_mkpath(pc, path, ap);

	प्रत्यागम !p ? -errno : ul_path_open(pc, flags, p);
}

पूर्णाङ्क ul_path_openf(संरचना path_cxt *pc, पूर्णाङ्क flags, ध्रुव वर्ण *path, ...)
{
	va_list ap;
	पूर्णाङ्क rc;

	va_start(ap, path);
	rc = ul_path_vopenf(pc, flags, path, ap);
	va_end(ap);

	प्रत्यागम rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_path_fopen(संरचना path_cxt *pc, ध्रुव वर्ण *mode, ध्रुव वर्ण *path)
{
	पूर्णाङ्क flags = ul_mode_to_flags(mode);
	पूर्णाङ्क fd = ul_path_open(pc, flags, path);

	यदि (fd < 0)
		प्रत्यागम रिक्त;

	प्रत्यागम ul_vfs_fdopen(pc ? pc->vfs : रिक्त, fd, mode);
}


FILE *ul_path_vfopenf(संरचना path_cxt *pc, ध्रुव वर्ण *mode, ध्रुव वर्ण *path, va_list ap)
{
	ध्रुव वर्ण *p = ul_path_mkpath(pc, path, ap);

	प्रत्यागम !p ? रिक्त : ul_path_fopen(pc, mode, p);
}

FILE *ul_path_fopenf(संरचना path_cxt *pc, ध्रुव वर्ण *mode, ध्रुव वर्ण *path, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, path);
	f = ul_path_vfopenf(pc, mode, path, ap);
	va_end(ap);

	प्रत्यागम f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendir(संरचना path_cxt *pc, ध्रुव वर्ण *path)
{
	DIR *dir;
	पूर्णाङ्क fd = -1;

	यदि (path)
		fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	अन्यथा यदि (pc->dir_path) {
		पूर्णाङ्क dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_path_get_dirfd(pc);
		यदि (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	यदि (fd < 0)
		प्रत्यागम रिक्त;

	dir = fdopendir(fd);
	यदि (!dir) {
		close(fd);
		प्रत्यागम रिक्त;
	}
	यदि (!path)
		 rewinddir(dir);
	प्रत्यागम dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_vopendirf(संरचना path_cxt *pc, ध्रुव वर्ण *path, va_list ap)
{
	ध्रुव वर्ण *p = ul_path_mkpath(pc, path, ap);

	प्रत्यागम !p ? रिक्त : ul_path_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendirf(संरचना path_cxt *pc, ध्रुव वर्ण *path, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, path);
	dir = ul_path_vopendirf(pc, path, ap);
	va_end(ap);

	प्रत्यागम dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlink(संरचना path_cxt *pc, वर्ण *buf, size_t bufsiz, ध्रुव वर्ण *path)
{
	पूर्णाङ्क dirfd;
	ssize_t ssz;

	यदि (!path) {
		ध्रुव वर्ण *p = get_absdir(pc);
		यदि (!p)
			प्रत्यागम -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} अन्यथा {
		dirfd = ul_path_get_dirfd(pc);
		यदि (dirfd < 0)
			प्रत्यागम dirfd;

		यदि (*path == '/')
			path++;

		ssz = readlinkat(dirfd, path, buf, bufsiz - 1);
	}

	यदि (ssz >= 0)
		buf[ssz] = '\0';
	प्रत्यागम ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlinkf(संरचना path_cxt *pc, वर्ण *buf, size_t bufsiz, ध्रुव वर्ण *path, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_path_readlink(pc, buf, bufsiz, p);
}

पूर्णाङ्क ul_path_read(संरचना path_cxt *pc, वर्ण *buf, size_t len, ध्रुव वर्ण *path)
{
	पूर्णाङ्क rc, errsv;
	पूर्णाङ्क fd;

	fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	यदि (fd < 0)
		प्रत्यागम -errno;

	DBG(CXT, ul_debug(" reading '%s'", path));
	rc = ul_vfs_read_all(pc ? pc->vfs : रिक्त, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : रिक्त, fd);
	errno = errsv;
	प्रत्यागम rc;
}

पूर्णाङ्क ul_path_vreadf(संरचना path_cxt *pc, वर्ण *buf, size_t len, ध्रुव वर्ण *path, va_list ap)
{
	ध्रुव वर्ण *p = ul_path_mkpath(pc, path, ap);

	प्रत्यागम !p ? -errno : ul_path_read(pc, buf, len, p);
}

पूर्णाङ्क ul_path_readf(संरचना path_cxt *pc, वर्ण *buf, size_t len, ध्रुव वर्ण *path, ...)
{
	va_list ap;
	पूर्णाङ्क rc;

	va_start(ap, path);
	rc = ul_path_vreadf(pc, buf, len, path, ap);
	va_end(ap);

	प्रत्यागम rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
पूर्णाङ्क ul_path_read_string(संरचना path_cxt *pc, वर्ण **str, ध्रुव वर्ण *path)
{
	वर्ण buf[BUFSIZ];
	पूर्णाङ्क rc;

	यदि (!str)
		प्रत्यागम -EINVAL;

	*str = रिक्त;

	rc = ul_path_read_buffer(pc, buf, परिमाण(buf), path);
	यदि (rc < 0)
		प्रत्यागम rc;

	*str = strdup(buf);
	यदि (!*str)
		rc = -ENOMEM;

	प्रत्यागम rc;
}

पूर्णाङ्क ul_path_readf_string(संरचना path_cxt *pc, वर्ण **str, ध्रुव वर्ण *path, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_path_read_string(pc, str, p);
}

पूर्णाङ्क ul_path_read_buffer(संरचना path_cxt *pc, वर्ण *buf, size_t bufsz, ध्रुव वर्ण *path)
{
	पूर्णाङ्क rc = ul_path_read(pc, buf, bufsz - 1, path);

	यदि (rc == 0)
		buf[0] = '\0';

	अन्यथा यदि (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		यदि (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		अन्यथा
			buf[rc] = '\0';
	}

	प्रत्यागम rc;
}

पूर्णाङ्क ul_path_vreadf_buffer(संरचना path_cxt *pc, वर्ण *buf, size_t bufsz, ध्रुव वर्ण *path, va_list ap)
{
	ध्रुव वर्ण *p;

	p = ul_path_mkpath(pc, path, ap);

	प्रत्यागम !p ? -errno : ul_path_read_buffer(pc, buf, bufsz, p);
}

पूर्णाङ्क ul_path_readf_buffer(संरचना path_cxt *pc, वर्ण *buf, size_t bufsz, ध्रुव वर्ण *path, ...)
{
	va_list ap;
	पूर्णाङ्क rc;

	va_start(ap, path);
	rc = ul_path_vreadf_buffer(pc, buf, bufsz, path, ap);
	va_end(ap);

	प्रत्यागम rc;
}

पूर्णाङ्क ul_path_scanf(संरचना path_cxt *pc, ध्रुव वर्ण *path, ध्रुव वर्ण *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	पूर्णाङ्क rc;

	f = ul_path_fopen(pc, "r" UL_CLOEXECSTR, path);
	यदि (!f)
		प्रत्यागम -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, path));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	प्रत्यागम rc;
}

पूर्णाङ्क ul_path_scanff(संरचना path_cxt *pc, ध्रुव वर्ण *path, va_list ap, ध्रुव वर्ण *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	पूर्णाङ्क rc;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	यदि (!f)
		प्रत्यागम -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	प्रत्यागम rc;
}


पूर्णाङ्क ul_path_read_s64(संरचना path_cxt *pc, int64_t *res, ध्रुव वर्ण *path)
{
	int64_t x = 0;
	पूर्णाङ्क rc;

	rc = ul_path_scanf(pc, path, "%"SCNd64, &x);
	यदि (rc != 1)
		प्रत्यागम -1;
	यदि (res)
		*res = x;
	प्रत्यागम 0;
}

पूर्णाङ्क ul_path_readf_s64(संरचना path_cxt *pc, int64_t *res, ध्रुव वर्ण *path, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_path_read_s64(pc, res, p);
}

पूर्णाङ्क ul_path_read_u64(संरचना path_cxt *pc, uint64_t *res, ध्रुव वर्ण *path)
{
	uint64_t x = 0;
	पूर्णाङ्क rc;

	rc = ul_path_scanf(pc, path, "%"SCNu64, &x);
	यदि (rc != 1)
		प्रत्यागम -1;
	यदि (res)
		*res = x;
	प्रत्यागम 0;
}

पूर्णाङ्क ul_path_readf_u64(संरचना path_cxt *pc, uint64_t *res, ध्रुव वर्ण *path, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_path_read_u64(pc, res, p);
}

पूर्णाङ्क ul_path_read_s32(संरचना path_cxt *pc, पूर्णाङ्क *res, ध्रुव वर्ण *path)
{
	पूर्णाङ्क rc, x = 0;

	rc = ul_path_scanf(pc, path, "%d", &x);
	यदि (rc != 1)
		प्रत्यागम -1;
	यदि (res)
		*res = x;
	प्रत्यागम 0;
}

पूर्णाङ्क ul_path_readf_s32(संरचना path_cxt *pc, पूर्णाङ्क *res, ध्रुव वर्ण *path, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_path_read_s32(pc, res, p);
}

पूर्णाङ्क ul_path_read_u32(संरचना path_cxt *pc, unsigned पूर्णाङ्क *res, ध्रुव वर्ण *path)
{
	पूर्णाङ्क rc;
	unsigned पूर्णाङ्क x = 0;

	rc = ul_path_scanf(pc, path, "%u", &x);
	यदि (rc != 1)
		प्रत्यागम -1;
	यदि (res)
		*res = x;
	प्रत्यागम 0;
}

पूर्णाङ्क ul_path_readf_u32(संरचना path_cxt *pc, unsigned पूर्णाङ्क *res, ध्रुव वर्ण *path, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_path_read_u32(pc, res, p);
}

पूर्णाङ्क ul_path_read_majmin(संरचना path_cxt *pc, dev_t *res, ध्रुव वर्ण *path)
{
	पूर्णाङ्क rc, maj = 0, min = 0;

	rc = ul_path_scanf(pc, path, "%d:%d", &maj, &min);
	यदि (rc != 2)
		प्रत्यागम -1;
	यदि (res)
		*res = makedev(maj, min);
	प्रत्यागम 0;
}

पूर्णाङ्क ul_path_readf_majmin(संरचना path_cxt *pc, dev_t *res, ध्रुव वर्ण *path, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_path_read_majmin(pc, res, p);
}

पूर्णाङ्क ul_path_write_string(संरचना path_cxt *pc, ध्रुव वर्ण *str, ध्रुव वर्ण *path)
{
	पूर्णाङ्क rc, errsv;
	पूर्णाङ्क fd;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	यदि (fd < 0)
		प्रत्यागम -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : रिक्त, fd, str, strlen(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : रिक्त, fd);
	errno = errsv;
	प्रत्यागम rc;
}

पूर्णाङ्क ul_path_writef_string(संरचना path_cxt *pc, ध्रुव वर्ण *str, ध्रुव वर्ण *path, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_path_write_string(pc, str, p);
}

पूर्णाङ्क ul_path_write_s64(संरचना path_cxt *pc, int64_t अंक, ध्रुव वर्ण *path)
{
	वर्ण buf[परिमाण(stringify_मूल्य(LLONG_MAX))];
	पूर्णाङ्क rc, errsv;
	पूर्णाङ्क fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	यदि (fd < 0)
		प्रत्यागम -errno;

	len = snprintf(buf, परिमाण(buf), "%" PRId64, अंक);
	यदि (len < 0 || (size_t) len >= परिमाण(buf))
		rc = len < 0 ? -errno : -E2BIG;
	अन्यथा
		rc = ul_vfs_write_all(pc ? pc->vfs : रिक्त, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : रिक्त, fd);
	errno = errsv;
	प्रत्यागम rc;
}

पूर्णाङ्क ul_path_write_u64(संरचना path_cxt *pc, uint64_t अंक, ध्रुव वर्ण *path)
{
	वर्ण buf[परिमाण(stringify_मूल्य(ULLONG_MAX))];
	पूर्णाङ्क rc, errsv;
	पूर्णाङ्क fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	यदि (fd < 0)
		प्रत्यागम -errno;

	len = snprintf(buf, परिमाण(buf), "%" PRIu64, अंक);
	यदि (len < 0 || (size_t) len >= परिमाण(buf))
		rc = len < 0 ? -errno : -E2BIG;
	अन्यथा
		rc = ul_vfs_write_all(pc ? pc->vfs : रिक्त, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : रिक्त, fd);
	errno = errsv;
	प्रत्यागम rc;
}

पूर्णाङ्क ul_path_writef_u64(संरचना path_cxt *pc, uint64_t अंक, ध्रुव वर्ण *path, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_path_write_u64(pc, अंक, p);

}

पूर्णाङ्क ul_path_मोजणी_dirents(संरचना path_cxt *pc, ध्रुव वर्ण *path)
{
	DIR *dir;
	पूर्णाङ्क r = 0;

	dir = ul_path_opendir(pc, path);
	यदि (!dir)
		प्रत्यागम 0;

	यावत् (xreaddir(dir)) r++;

	closedir(dir);
	प्रत्यागम r;
}

पूर्णाङ्क ul_path_countf_dirents(संरचना path_cxt *pc, ध्रुव वर्ण *path, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_path_मोजणी_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
पूर्णाङ्क ul_path_next_dirent(संरचना path_cxt *pc, DIR **sub, ध्रुव वर्ण *dirname, संरचना dirent **d)
{
	यदि (!pc || !sub || !d)
		प्रत्यागम -EINVAL;

	यदि (!*sub) {
		*sub = ul_path_opendir(pc, dirname);
		यदि (!*sub)
			प्रत्यागम -errno;
	}

	*d = xreaddir(*sub);
	यदि (*d)
		प्रत्यागम 0;

	closedir(*sub);
	*sub = रिक्त;
	प्रत्यागम 1;
}

#ifdef HAVE_CPU_SET_T
स्थिर पूर्णाङ्क ul_path_cpuparse(संरचना path_cxt *pc, cpu_set_t **set, पूर्णाङ्क maxcpus, पूर्णाङ्क islist, ध्रुव वर्ण *path, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	वर्ण *buf;
	पूर्णाङ्क rc;

	*set = रिक्त;

	buf = malloc(len);
	यदि (!buf)
		प्रत्यागम -ENOMEM;

	rc = ul_path_vreadf_buffer(pc, buf, len, path, ap);
	यदि (rc < 0)
		गच्छ out;

	*set = cpuset_alloc(maxcpus, &setsize, रिक्त);
	यदि (!*set) {
		rc = -EINVAL;
		गच्छ out;
	}

	यदि (islist) {
		यदि (cpulist_parse(buf, *set, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			गच्छ out;
		}
	} अन्यथा {
		यदि (cpumask_parse(buf, *set, setsize)) {
			errno = EINVAL;
			rc = -errno;
			गच्छ out;
		}
	}
	rc = 0;

out:
	यदि (rc) {
		cpuset_free(*set);
		*set = रिक्त;
	}
	मुक्त(buf);
	प्रत्यागम rc;
}

पूर्णाङ्क ul_path_readf_cpuset(संरचना path_cxt *pc, cpu_set_t **set, पूर्णाङ्क maxcpus, ध्रुव वर्ण *path, ...)
{
	va_list ap;
	पूर्णाङ्क rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	va_end(ap);

	प्रत्यागम rc;
}

पूर्णाङ्क ul_path_readf_cpulist(संरचना path_cxt *pc, cpu_set_t **set, पूर्णाङ्क maxcpus, ध्रुव वर्ण *path, ...)
{
	va_list ap;
	पूर्णाङ्क rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	va_end(ap);

	प्रत्यागम rc;
}

#endif /* HAVE_CPU_SET_T */


#ifdef TEST_PROGRAM_PATH
#अन्तर्भाव <getopt.h>

स्थिर शून्य __attribute__((__noreturn__)) usage(शून्य)
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

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	पूर्णाङ्क c;
	ध्रुव वर्ण *prefix = रिक्त, *dir, *file, *command;
	संरचना path_cxt *pc = रिक्त;

	स्थिर ध्रुव संरचना option longopts[] = {
		{ "prefix",	1, रिक्त, 'p' },
		{ "help",       0, रिक्त, 'h' },
		{ रिक्त, 0, रिक्त, 0 },
	};

	यावत्((c = getopt_long(argc, argv, "p:h", longopts, रिक्त)) != -1) {
		विकल्प(c) {
		पक्ष 'p':
			prefix = optarg;
			भङ्ग;
		पक्ष 'h':
			usage();
			भङ्ग;
		स्वभाव:
			err(EXIT_FAILURE, "try --help");
		}
	}

	यदि (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_path_init_debug();

	pc = ul_new_path("%s", dir);
	यदि (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	यदि (prefix)
		ul_path_set_prefix(pc, prefix);

	यदि (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	यदि (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		यदि (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		यदि (ul_path_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %u\n", file, res);

		यदि (ul_path_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %u\n", file, res);

	} अन्यथा यदि (strcmp(command, "read-s32") == 0) {
		int32_t res;

		यदि (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		यदि (ul_path_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %d\n", file, res);

		यदि (ul_path_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %d\n", file, res);

	} अन्यथा यदि (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		यदि (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		यदि (ul_path_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRIu64 "\n", file, res);

		यदि (ul_path_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRIu64 "\n", file, res);

	} अन्यथा यदि (strcmp(command, "read-s64") == 0) {
		int64_t res;

		यदि (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		यदि (ul_path_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRId64 "\n", file, res);

		यदि (ul_path_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRId64 "\n", file, res);

	} अन्यथा यदि (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		यदि (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		यदि (ul_path_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		printf("read:  %s: %d\n", file, (पूर्णाङ्क) res);

		यदि (ul_path_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		printf("readf: %s: %d\n", file, (पूर्णाङ्क) res);

	} अन्यथा यदि (strcmp(command, "read-string") == 0) {
		वर्ण *res;

		यदि (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		यदि (ul_path_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		printf("read:  %s: %s\n", file, res);

		यदि (ul_path_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		printf("readf: %s: %s\n", file, res);

	} अन्यथा यदि (strcmp(command, "read-link") == 0) {
		वर्ण res[PATH_MAX];

		यदि (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		यदि (ul_path_readlink(pc, res, परिमाण(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		printf("read:  %s: %s\n", file, res);

		यदि (ul_path_readlinkf(pc, res, परिमाण(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		printf("readf: %s: %s\n", file, res);

	} अन्यथा यदि (strcmp(command, "write-string") == 0) {
		वर्ण *str;

		यदि (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		यदि (ul_path_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		यदि (ul_path_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} अन्यथा यदि (strcmp(command, "write-u64") == 0) {
		uint64_t अंक;

		यदि (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		अंक = strtoumax(argv[optind++], रिक्त, 0);

		यदि (ul_path_write_u64(pc, अंक, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		यदि (ul_path_writef_u64(pc, अंक, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_path(pc);
	प्रत्यागम EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PATH */
