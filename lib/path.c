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
#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <inttypes.h>
#અન્તર્ભાવ <errno.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "fileutils.h"
#અન્તર્ભાવ "all-io.h"
#અન્તર્ભાવ "vfs.h"
#અન્તર્ભાવ "path.h"
#અન્તર્ભાવ "debug.h"
#અન્તર્ભાવ "strutils.h"

/*
 * Debug stuff (based on include/debug.h)
 */
સ્થિર UL_DEBUG_DEFINE_MASK(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPATH_DEBUG_INIT	(1 << 1)
#define ULPATH_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

શૂન્ય ul_path_init_debug(શૂન્ય)
{
	જો (ulpath_debug_mask)
		પાછા;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

સંરચના path_cxt *ul_new_path(અચલ અક્ષર *dir, ...)
{
	સંરચના path_cxt *pc = calloc(1, કદ(*pc));

	જો (!pc)
		પાછા ખાલી;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	જો (dir) {
		પૂર્ણાંક rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_path, dir, ap);
		va_end(ap);

		જો (rc < 0 || !pc->dir_path)
			જાઓ fail;
	}
	પાછા pc;
fail:
	ul_unref_path(pc);
	પાછા ખાલી;
}

શૂન્ય ul_ref_path(સંરચના path_cxt *pc)
{
	જો (pc)
		pc->refcount++;
}

શૂન્ય ul_unref_path(સંરચના path_cxt *pc)
{
	જો (!pc)
		પાછા;

	pc->refcount--;

	જો (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		જો (pc->dialect)
			pc->free_dialect(pc);
		ul_path_close_dirfd(pc);
		મુક્ત(pc->dir_path);
		મુક્ત(pc->prefix);
		મુક્ત(pc);
	}
}

શૂન્ય ul_path_refer_vfs(સંરચના path_cxt *pc, અચલ સંરચના ul_vfs_ops *vfs)
{
	જો (pc)
		pc->vfs = vfs;
}

પૂર્ણાંક ul_path_set_prefix(સંરચના path_cxt *pc, અચલ અક્ષર *prefix)
{
	અક્ષર *p = ખાલી;

	પ્રતિજ્ઞા(pc->dir_fd < 0);

	જો (prefix) {
		p = strdup(prefix);
		જો (!p)
			પાછા -ENOMEM;
	}

	મુક્ત(pc->prefix);
	pc->prefix = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	પાછા 0;
}

અચલ અક્ષર *ul_path_get_prefix(સંરચના path_cxt *pc)
{
	પાછા pc ? pc->prefix : ખાલી;
}

પૂર્ણાંક ul_path_set_dir(સંરચના path_cxt *pc, અચલ અક્ષર *dir)
{
	અક્ષર *p = ખાલી;

	જો (dir) {
		p = strdup(dir);
		જો (!p)
			પાછા -ENOMEM;
	}

	જો (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	મુક્ત(pc->dir_path);
	pc->dir_path = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	પાછા 0;
}

અચલ અક્ષર *ul_path_get_dir(સંરચના path_cxt *pc)
{
	પાછા pc ? pc->dir_path : ખાલી;
}

પૂર્ણાંક ul_path_set_dialect(સંરચના path_cxt *pc, શૂન્ય *data, શૂન્ય free_data(સંરચના path_cxt *))
{
	pc->dialect = data;
	pc->free_dialect = free_data;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	પાછા 0;
}

શૂન્ય *ul_path_get_dialect(સંરચના path_cxt *pc)
{
	પાછા pc ? pc->dialect : ખાલી;
}

પૂર્ણાંક ul_path_set_enoent_redirect(સંરચના path_cxt *pc, પૂર્ણાંક (*func)(સંરચના path_cxt *, અચલ અક્ષર *, પૂર્ણાંક *))
{
	pc->redirect_on_enoent = func;
	પાછા 0;
}

સ્થિર અચલ અક્ષર *get_absdir(સંરચના path_cxt *pc)
{
	પૂર્ણાંક rc;
	અચલ અક્ષર *dirpath;

	જો (!pc->prefix)
		પાછા pc->dir_path;

	dirpath = pc->dir_path;
	જો (!dirpath)
		પાછા pc->prefix;
	જો (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->path_buffer, કદ(pc->path_buffer), "%s/%s", pc->prefix, dirpath);
	જો (rc < 0)
		પાછા ખાલી;
	જો ((size_t)rc >= કદ(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		પાછા ખાલી;
	}

	પાછા pc->path_buffer;
}

પૂર્ણાંક ul_path_is_accessible(સંરચના path_cxt *pc)
{
	અચલ અક્ષર *path;
	પ્રતિજ્ઞા(pc);

	જો (pc->dir_fd >= 0)
		પાછા 1;

	path = get_absdir(pc);
	જો (!path)
		પાછા 0;
	પાછા access(path, F_OK) == 0;
}

પૂર્ણાંક ul_path_get_dirfd(સંરચના path_cxt *pc)
{
	પ્રતિજ્ઞા(pc);
	પ્રતિજ્ઞા(pc->dir_path);

	જો (pc->dir_fd < 0) {
		અચલ અક્ષર *path = get_absdir(pc);
		જો (!path)
			પાછા -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", path));
		pc->dir_fd = open(path, O_RDONLY|O_CLOEXEC);
	}

	પાછા pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
શૂન્ય ul_path_close_dirfd(સંરચના path_cxt *pc)
{
	પ્રતિજ્ઞા(pc);

	જો (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

પૂર્ણાંક ul_path_isopen_dirfd(સંરચના path_cxt *pc)
{
	પાછા pc && pc->dir_fd >= 0;
}

સ્થિર અચલ અક્ષર *ul_path_mkpath(સંરચના path_cxt *pc, અચલ અક્ષર *path, va_list ap)
{
	પૂર્ણાંક rc;

	errno = 0;

	rc = vsnprintf(pc->path_buffer, કદ(pc->path_buffer), path, ap);
	જો (rc < 0) {
		જો (!errno)
			errno = EINVAL;
		પાછા ખાલી;
	}

	જો ((size_t)rc >= કદ(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		પાછા ખાલી;
	}

	પાછા pc->path_buffer;
}

અક્ષર *ul_path_get_abspath(સંરચના path_cxt *pc, અક્ષર *buf, size_t bufsz, અચલ અક્ષર *path, ...)
{
	જો (path) {
		પૂર્ણાંક rc;
		va_list ap;
		અચલ અક્ષર *tail = ખાલી, *dirpath = pc->dir_path;

		va_start(ap, path);
		tail = ul_path_mkpath(pc, path, ap);
		va_end(ap);

		જો (dirpath && *dirpath == '/')
			dirpath++;
		જો (tail && *tail == '/')
			tail++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->prefix ? pc->prefix : "",
				dirpath ? dirpath : "",
				tail ? tail : "");

		જો ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			પાછા ખાલી;
		}
	} નહીંતર {
		અચલ અક્ષર *tmp = get_absdir(pc);

		જો (!tmp)
			પાછા ખાલી;
		xstrncpy(buf, tmp, bufsz);
	}

	પાછા buf;
}


પૂર્ણાંક ul_path_access(સંરચના path_cxt *pc, પૂર્ણાંક mode, અચલ અક્ષર *path)
{
	પૂર્ણાંક rc;

	જો (!path)
		પાછા -EINVAL;
	જો (!pc) {
		rc = access(path, mode);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", path, rc));
	} નહીંતર {
		પૂર્ણાંક dir = ul_path_get_dirfd(pc);
		જો (dir < 0)
			પાછા dir;
		જો (*path == '/')
			path++;

		rc = faccessat(dir, path, mode, 0);

		જો (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = faccessat(dir, path, mode, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", path, rc));
	}
	પાછા rc;
}

પૂર્ણાંક ul_path_accessf(સંરચના path_cxt *pc, પૂર્ણાંક mode, અચલ અક્ષર *path, ...)
{
	va_list ap;
	અચલ અક્ષર *p;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	પાછા !p ? -errno : ul_path_access(pc, mode, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
પૂર્ણાંક ul_path_stat(સંરચના path_cxt *pc, સંરચના stat *sb, પૂર્ણાંક flags, અચલ અક્ષર *path)
{
	પૂર્ણાંક rc;

	જો (!pc) {
		rc = path ? stat(path, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", path, rc));
	} નહીંતર {
		પૂર્ણાંક dir = ul_path_get_dirfd(pc);
		જો (dir < 0)
			પાછા dir;
		જો (path) {
			જો  (*path == '/')
				path++;
			rc = fstatat(dir, path, sb, flags);

		} નહીંતર
			rc = fstat(dir, sb);	/* dir itself */

		જો (rc && errno == ENOENT
		    && path
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = fstatat(dir, path, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", path, rc));
	}
	પાછા rc;
}

પૂર્ણાંક ul_path_vstatf(સંરચના path_cxt *pc, સંરચના stat *sb, પૂર્ણાંક flags, અચલ અક્ષર *path, va_list ap)
{
	અચલ અક્ષર *p = ul_path_mkpath(pc, path, ap);

	પાછા !p ? -errno : ul_path_stat(pc, sb, flags, p);
}

પૂર્ણાંક ul_path_statf(સંરચના path_cxt *pc, સંરચના stat *sb, પૂર્ણાંક flags, અચલ અક્ષર *path, ...)
{
	va_list ap;
	પૂર્ણાંક rc;

	va_start(ap, path);
	rc = ul_path_vstatf(pc, sb, flags, path, ap);
	va_end(ap);

	પાછા rc;
}
#ifdef HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
પૂર્ણાંક ul_path_statx(સંરચના path_cxt *pc, સંરચના statx *stx, પૂર્ણાંક flags, unsigned પૂર્ણાંક mask,
		  અચલ અક્ષર *path)
{
	પૂર્ણાંક rc;

	જો (!path)
		પાછા -EINVAL;
	જો (!pc)
		rc = path ? statx(AT_FDCWD, path, flags, mask, stx) : - EINVAL;
	નહીંતર {
		પૂર્ણાંક dir = ul_path_get_dirfd(pc);
		જો (dir < 0)
			પાછા dir;
		જો (*path == '/')
			path++;

		rc = statx(dir, path, flags, mask, stx);
	}

	પાછા rc;
}
#નહીંતર
પૂર્ણાંક ul_path_statx(સંરચના path_cxt *pc __attribute__((__unused__)),
		  સંરચના statx *stx __attribute__((__unused__)),
		  પૂર્ણાંક flags __attribute__((__unused__)),
		  unsigned પૂર્ણાંક mask __attribute__((__unused__)),
		  અચલ અક્ષર *path __attribute__((__unused__)))
{
	errno = ENOSYS;
	પાછા -1;
}
#endif /* HAVE_STATX */

પૂર્ણાંક ul_path_vstatxf(સંરચના path_cxt *pc, સંરચના statx *stx, પૂર્ણાંક flags, unsigned પૂર્ણાંક mask,
		    અચલ અક્ષર *path, va_list ap)
{
	અચલ અક્ષર *p = ul_path_mkpath(pc, path, ap);

	પાછા !p ? -errno : ul_path_statx(pc, stx, flags, mask, p);
}

પૂર્ણાંક ul_path_statxf(સંરચના path_cxt *pc, સંરચના statx *stx, પૂર્ણાંક flags, unsigned પૂર્ણાંક mask,
		   અચલ અક્ષર *path, ...)
{
	va_list ap;
	પૂર્ણાંક rc;

	va_start(ap, path);
	rc = ul_path_vstatxf(pc, stx, flags, mask, path, ap);
	va_end(ap);

	પાછા rc;
}

પૂર્ણાંક ul_path_open(સંરચના path_cxt *pc, પૂર્ણાંક flags, અચલ અક્ષર *path)
{
	પૂર્ણાંક fd;

	જો (!path)
		પાછા -EINVAL;
	જો (!pc) {
		fd = open(path, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", path));
	} નહીંતર {
		પૂર્ણાંક fdx;
		પૂર્ણાંક dir = ul_path_get_dirfd(pc);
		જો (dir < 0)
			પાછા dir;

		જો (*path == '/')
			path++;

		fdx = fd = openat(dir, path, flags);

		જો (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			fd = openat(dir, path, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", path, fdx != fd ? " [redirected]" : ""));
	}
	પાછા fd;
}

પૂર્ણાંક ul_path_vopenf(સંરચના path_cxt *pc, પૂર્ણાંક flags, અચલ અક્ષર *path, va_list ap)
{
	અચલ અક્ષર *p = ul_path_mkpath(pc, path, ap);

	પાછા !p ? -errno : ul_path_open(pc, flags, p);
}

પૂર્ણાંક ul_path_openf(સંરચના path_cxt *pc, પૂર્ણાંક flags, અચલ અક્ષર *path, ...)
{
	va_list ap;
	પૂર્ણાંક rc;

	va_start(ap, path);
	rc = ul_path_vopenf(pc, flags, path, ap);
	va_end(ap);

	પાછા rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_path_fopen(સંરચના path_cxt *pc, અચલ અક્ષર *mode, અચલ અક્ષર *path)
{
	પૂર્ણાંક flags = ul_mode_to_flags(mode);
	પૂર્ણાંક fd = ul_path_open(pc, flags, path);

	જો (fd < 0)
		પાછા ખાલી;

	પાછા ul_vfs_fdopen(pc ? pc->vfs : ખાલી, fd, mode);
}


FILE *ul_path_vfopenf(સંરચના path_cxt *pc, અચલ અક્ષર *mode, અચલ અક્ષર *path, va_list ap)
{
	અચલ અક્ષર *p = ul_path_mkpath(pc, path, ap);

	પાછા !p ? ખાલી : ul_path_fopen(pc, mode, p);
}

FILE *ul_path_fopenf(સંરચના path_cxt *pc, અચલ અક્ષર *mode, અચલ અક્ષર *path, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, path);
	f = ul_path_vfopenf(pc, mode, path, ap);
	va_end(ap);

	પાછા f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendir(સંરચના path_cxt *pc, અચલ અક્ષર *path)
{
	DIR *dir;
	પૂર્ણાંક fd = -1;

	જો (path)
		fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	નહીંતર જો (pc->dir_path) {
		પૂર્ણાંક dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_path_get_dirfd(pc);
		જો (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	જો (fd < 0)
		પાછા ખાલી;

	dir = fdopendir(fd);
	જો (!dir) {
		close(fd);
		પાછા ખાલી;
	}
	જો (!path)
		 rewinddir(dir);
	પાછા dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_vopendirf(સંરચના path_cxt *pc, અચલ અક્ષર *path, va_list ap)
{
	અચલ અક્ષર *p = ul_path_mkpath(pc, path, ap);

	પાછા !p ? ખાલી : ul_path_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendirf(સંરચના path_cxt *pc, અચલ અક્ષર *path, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, path);
	dir = ul_path_vopendirf(pc, path, ap);
	va_end(ap);

	પાછા dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlink(સંરચના path_cxt *pc, અક્ષર *buf, size_t bufsiz, અચલ અક્ષર *path)
{
	પૂર્ણાંક dirfd;
	ssize_t ssz;

	જો (!path) {
		અચલ અક્ષર *p = get_absdir(pc);
		જો (!p)
			પાછા -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} નહીંતર {
		dirfd = ul_path_get_dirfd(pc);
		જો (dirfd < 0)
			પાછા dirfd;

		જો (*path == '/')
			path++;

		ssz = readlinkat(dirfd, path, buf, bufsiz - 1);
	}

	જો (ssz >= 0)
		buf[ssz] = '\0';
	પાછા ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlinkf(સંરચના path_cxt *pc, અક્ષર *buf, size_t bufsiz, અચલ અક્ષર *path, ...)
{
	અચલ અક્ષર *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	પાછા !p ? -errno : ul_path_readlink(pc, buf, bufsiz, p);
}

પૂર્ણાંક ul_path_read(સંરચના path_cxt *pc, અક્ષર *buf, size_t len, અચલ અક્ષર *path)
{
	પૂર્ણાંક rc, errsv;
	પૂર્ણાંક fd;

	fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	જો (fd < 0)
		પાછા -errno;

	DBG(CXT, ul_debug(" reading '%s'", path));
	rc = ul_vfs_read_all(pc ? pc->vfs : ખાલી, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ખાલી, fd);
	errno = errsv;
	પાછા rc;
}

પૂર્ણાંક ul_path_vreadf(સંરચના path_cxt *pc, અક્ષર *buf, size_t len, અચલ અક્ષર *path, va_list ap)
{
	અચલ અક્ષર *p = ul_path_mkpath(pc, path, ap);

	પાછા !p ? -errno : ul_path_read(pc, buf, len, p);
}

પૂર્ણાંક ul_path_readf(સંરચના path_cxt *pc, અક્ષર *buf, size_t len, અચલ અક્ષર *path, ...)
{
	va_list ap;
	પૂર્ણાંક rc;

	va_start(ap, path);
	rc = ul_path_vreadf(pc, buf, len, path, ap);
	va_end(ap);

	પાછા rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
પૂર્ણાંક ul_path_read_string(સંરચના path_cxt *pc, અક્ષર **str, અચલ અક્ષર *path)
{
	અક્ષર buf[BUFSIZ];
	પૂર્ણાંક rc;

	જો (!str)
		પાછા -EINVAL;

	*str = ખાલી;

	rc = ul_path_read_buffer(pc, buf, કદ(buf), path);
	જો (rc < 0)
		પાછા rc;

	*str = strdup(buf);
	જો (!*str)
		rc = -ENOMEM;

	પાછા rc;
}

પૂર્ણાંક ul_path_readf_string(સંરચના path_cxt *pc, અક્ષર **str, અચલ અક્ષર *path, ...)
{
	અચલ અક્ષર *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	પાછા !p ? -errno : ul_path_read_string(pc, str, p);
}

પૂર્ણાંક ul_path_read_buffer(સંરચના path_cxt *pc, અક્ષર *buf, size_t bufsz, અચલ અક્ષર *path)
{
	પૂર્ણાંક rc = ul_path_read(pc, buf, bufsz - 1, path);

	જો (rc == 0)
		buf[0] = '\0';

	નહીંતર જો (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		જો (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		નહીંતર
			buf[rc] = '\0';
	}

	પાછા rc;
}

પૂર્ણાંક ul_path_vreadf_buffer(સંરચના path_cxt *pc, અક્ષર *buf, size_t bufsz, અચલ અક્ષર *path, va_list ap)
{
	અચલ અક્ષર *p;

	p = ul_path_mkpath(pc, path, ap);

	પાછા !p ? -errno : ul_path_read_buffer(pc, buf, bufsz, p);
}

પૂર્ણાંક ul_path_readf_buffer(સંરચના path_cxt *pc, અક્ષર *buf, size_t bufsz, અચલ અક્ષર *path, ...)
{
	va_list ap;
	પૂર્ણાંક rc;

	va_start(ap, path);
	rc = ul_path_vreadf_buffer(pc, buf, bufsz, path, ap);
	va_end(ap);

	પાછા rc;
}

પૂર્ણાંક ul_path_scanf(સંરચના path_cxt *pc, અચલ અક્ષર *path, અચલ અક્ષર *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	પૂર્ણાંક rc;

	f = ul_path_fopen(pc, "r" UL_CLOEXECSTR, path);
	જો (!f)
		પાછા -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, path));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	પાછા rc;
}

પૂર્ણાંક ul_path_scanff(સંરચના path_cxt *pc, અચલ અક્ષર *path, va_list ap, અચલ અક્ષર *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	પૂર્ણાંક rc;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	જો (!f)
		પાછા -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	પાછા rc;
}


પૂર્ણાંક ul_path_read_s64(સંરચના path_cxt *pc, int64_t *res, અચલ અક્ષર *path)
{
	int64_t x = 0;
	પૂર્ણાંક rc;

	rc = ul_path_scanf(pc, path, "%"SCNd64, &x);
	જો (rc != 1)
		પાછા -1;
	જો (res)
		*res = x;
	પાછા 0;
}

પૂર્ણાંક ul_path_readf_s64(સંરચના path_cxt *pc, int64_t *res, અચલ અક્ષર *path, ...)
{
	અચલ અક્ષર *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	પાછા !p ? -errno : ul_path_read_s64(pc, res, p);
}

પૂર્ણાંક ul_path_read_u64(સંરચના path_cxt *pc, uint64_t *res, અચલ અક્ષર *path)
{
	uint64_t x = 0;
	પૂર્ણાંક rc;

	rc = ul_path_scanf(pc, path, "%"SCNu64, &x);
	જો (rc != 1)
		પાછા -1;
	જો (res)
		*res = x;
	પાછા 0;
}

પૂર્ણાંક ul_path_readf_u64(સંરચના path_cxt *pc, uint64_t *res, અચલ અક્ષર *path, ...)
{
	અચલ અક્ષર *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	પાછા !p ? -errno : ul_path_read_u64(pc, res, p);
}

પૂર્ણાંક ul_path_read_s32(સંરચના path_cxt *pc, પૂર્ણાંક *res, અચલ અક્ષર *path)
{
	પૂર્ણાંક rc, x = 0;

	rc = ul_path_scanf(pc, path, "%d", &x);
	જો (rc != 1)
		પાછા -1;
	જો (res)
		*res = x;
	પાછા 0;
}

પૂર્ણાંક ul_path_readf_s32(સંરચના path_cxt *pc, પૂર્ણાંક *res, અચલ અક્ષર *path, ...)
{
	અચલ અક્ષર *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	પાછા !p ? -errno : ul_path_read_s32(pc, res, p);
}

પૂર્ણાંક ul_path_read_u32(સંરચના path_cxt *pc, unsigned પૂર્ણાંક *res, અચલ અક્ષર *path)
{
	પૂર્ણાંક rc;
	unsigned પૂર્ણાંક x = 0;

	rc = ul_path_scanf(pc, path, "%u", &x);
	જો (rc != 1)
		પાછા -1;
	જો (res)
		*res = x;
	પાછા 0;
}

પૂર્ણાંક ul_path_readf_u32(સંરચના path_cxt *pc, unsigned પૂર્ણાંક *res, અચલ અક્ષર *path, ...)
{
	અચલ અક્ષર *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	પાછા !p ? -errno : ul_path_read_u32(pc, res, p);
}

પૂર્ણાંક ul_path_read_majmin(સંરચના path_cxt *pc, dev_t *res, અચલ અક્ષર *path)
{
	પૂર્ણાંક rc, maj = 0, min = 0;

	rc = ul_path_scanf(pc, path, "%d:%d", &maj, &min);
	જો (rc != 2)
		પાછા -1;
	જો (res)
		*res = makedev(maj, min);
	પાછા 0;
}

પૂર્ણાંક ul_path_readf_majmin(સંરચના path_cxt *pc, dev_t *res, અચલ અક્ષર *path, ...)
{
	અચલ અક્ષર *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	પાછા !p ? -errno : ul_path_read_majmin(pc, res, p);
}

પૂર્ણાંક ul_path_write_string(સંરચના path_cxt *pc, અચલ અક્ષર *str, અચલ અક્ષર *path)
{
	પૂર્ણાંક rc, errsv;
	પૂર્ણાંક fd;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	જો (fd < 0)
		પાછા -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : ખાલી, fd, str, strlen(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ખાલી, fd);
	errno = errsv;
	પાછા rc;
}

પૂર્ણાંક ul_path_writef_string(સંરચના path_cxt *pc, અચલ અક્ષર *str, અચલ અક્ષર *path, ...)
{
	અચલ અક્ષર *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	પાછા !p ? -errno : ul_path_write_string(pc, str, p);
}

પૂર્ણાંક ul_path_write_s64(સંરચના path_cxt *pc, int64_t અંક, અચલ અક્ષર *path)
{
	અક્ષર buf[કદ(stringify_કિંમત(LLONG_MAX))];
	પૂર્ણાંક rc, errsv;
	પૂર્ણાંક fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	જો (fd < 0)
		પાછા -errno;

	len = snprintf(buf, કદ(buf), "%" PRId64, અંક);
	જો (len < 0 || (size_t) len >= કદ(buf))
		rc = len < 0 ? -errno : -E2BIG;
	નહીંતર
		rc = ul_vfs_write_all(pc ? pc->vfs : ખાલી, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ખાલી, fd);
	errno = errsv;
	પાછા rc;
}

પૂર્ણાંક ul_path_write_u64(સંરચના path_cxt *pc, uint64_t અંક, અચલ અક્ષર *path)
{
	અક્ષર buf[કદ(stringify_કિંમત(ULLONG_MAX))];
	પૂર્ણાંક rc, errsv;
	પૂર્ણાંક fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	જો (fd < 0)
		પાછા -errno;

	len = snprintf(buf, કદ(buf), "%" PRIu64, અંક);
	જો (len < 0 || (size_t) len >= કદ(buf))
		rc = len < 0 ? -errno : -E2BIG;
	નહીંતર
		rc = ul_vfs_write_all(pc ? pc->vfs : ખાલી, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ખાલી, fd);
	errno = errsv;
	પાછા rc;
}

પૂર્ણાંક ul_path_writef_u64(સંરચના path_cxt *pc, uint64_t અંક, અચલ અક્ષર *path, ...)
{
	અચલ અક્ષર *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	પાછા !p ? -errno : ul_path_write_u64(pc, અંક, p);

}

પૂર્ણાંક ul_path_ગણતરી_dirents(સંરચના path_cxt *pc, અચલ અક્ષર *path)
{
	DIR *dir;
	પૂર્ણાંક r = 0;

	dir = ul_path_opendir(pc, path);
	જો (!dir)
		પાછા 0;

	જ્યારે (xreaddir(dir)) r++;

	closedir(dir);
	પાછા r;
}

પૂર્ણાંક ul_path_countf_dirents(સંરચના path_cxt *pc, અચલ અક્ષર *path, ...)
{
	અચલ અક્ષર *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	પાછા !p ? -errno : ul_path_ગણતરી_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
પૂર્ણાંક ul_path_next_dirent(સંરચના path_cxt *pc, DIR **sub, અચલ અક્ષર *dirname, સંરચના dirent **d)
{
	જો (!pc || !sub || !d)
		પાછા -EINVAL;

	જો (!*sub) {
		*sub = ul_path_opendir(pc, dirname);
		જો (!*sub)
			પાછા -errno;
	}

	*d = xreaddir(*sub);
	જો (*d)
		પાછા 0;

	closedir(*sub);
	*sub = ખાલી;
	પાછા 1;
}

#ifdef HAVE_CPU_SET_T
સ્થિર પૂર્ણાંક ul_path_cpuparse(સંરચના path_cxt *pc, cpu_set_t **set, પૂર્ણાંક maxcpus, પૂર્ણાંક islist, અચલ અક્ષર *path, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	અક્ષર *buf;
	પૂર્ણાંક rc;

	*set = ખાલી;

	buf = malloc(len);
	જો (!buf)
		પાછા -ENOMEM;

	rc = ul_path_vreadf_buffer(pc, buf, len, path, ap);
	જો (rc < 0)
		જાઓ out;

	*set = cpuset_alloc(maxcpus, &setsize, ખાલી);
	જો (!*set) {
		rc = -EINVAL;
		જાઓ out;
	}

	જો (islist) {
		જો (cpulist_parse(buf, *set, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			જાઓ out;
		}
	} નહીંતર {
		જો (cpumask_parse(buf, *set, setsize)) {
			errno = EINVAL;
			rc = -errno;
			જાઓ out;
		}
	}
	rc = 0;

out:
	જો (rc) {
		cpuset_free(*set);
		*set = ખાલી;
	}
	મુક્ત(buf);
	પાછા rc;
}

પૂર્ણાંક ul_path_readf_cpuset(સંરચના path_cxt *pc, cpu_set_t **set, પૂર્ણાંક maxcpus, અચલ અક્ષર *path, ...)
{
	va_list ap;
	પૂર્ણાંક rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	va_end(ap);

	પાછા rc;
}

પૂર્ણાંક ul_path_readf_cpulist(સંરચના path_cxt *pc, cpu_set_t **set, પૂર્ણાંક maxcpus, અચલ અક્ષર *path, ...)
{
	va_list ap;
	પૂર્ણાંક rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	va_end(ap);

	પાછા rc;
}

#endif /* HAVE_CPU_SET_T */


#ifdef TEST_PROGRAM_PATH
#અન્તર્ભાવ <getopt.h>

સ્થિર શૂન્ય __attribute__((__noreturn__)) usage(શૂન્ય)
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

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
	પૂર્ણાંક c;
	અચલ અક્ષર *prefix = ખાલી, *dir, *file, *command;
	સંરચના path_cxt *pc = ખાલી;

	સ્થિર અચલ સંરચના option longopts[] = {
		{ "prefix",	1, ખાલી, 'p' },
		{ "help",       0, ખાલી, 'h' },
		{ ખાલી, 0, ખાલી, 0 },
	};

	જ્યારે((c = getopt_long(argc, argv, "p:h", longopts, ખાલી)) != -1) {
		પસંદગી(c) {
		સ્થિતિ 'p':
			prefix = optarg;
			તોડો;
		સ્થિતિ 'h':
			usage();
			તોડો;
		મૂળભૂત:
			err(EXIT_FAILURE, "try --help");
		}
	}

	જો (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_path_init_debug();

	pc = ul_new_path("%s", dir);
	જો (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	જો (prefix)
		ul_path_set_prefix(pc, prefix);

	જો (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	જો (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		જો (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		જો (ul_path_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %u\n", file, res);

		જો (ul_path_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %u\n", file, res);

	} નહીંતર જો (strcmp(command, "read-s32") == 0) {
		int32_t res;

		જો (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		જો (ul_path_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %d\n", file, res);

		જો (ul_path_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %d\n", file, res);

	} નહીંતર જો (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		જો (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		જો (ul_path_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRIu64 "\n", file, res);

		જો (ul_path_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRIu64 "\n", file, res);

	} નહીંતર જો (strcmp(command, "read-s64") == 0) {
		int64_t res;

		જો (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		જો (ul_path_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRId64 "\n", file, res);

		જો (ul_path_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRId64 "\n", file, res);

	} નહીંતર જો (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		જો (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		જો (ul_path_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		printf("read:  %s: %d\n", file, (પૂર્ણાંક) res);

		જો (ul_path_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		printf("readf: %s: %d\n", file, (પૂર્ણાંક) res);

	} નહીંતર જો (strcmp(command, "read-string") == 0) {
		અક્ષર *res;

		જો (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		જો (ul_path_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		printf("read:  %s: %s\n", file, res);

		જો (ul_path_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		printf("readf: %s: %s\n", file, res);

	} નહીંતર જો (strcmp(command, "read-link") == 0) {
		અક્ષર res[PATH_MAX];

		જો (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		જો (ul_path_readlink(pc, res, કદ(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		printf("read:  %s: %s\n", file, res);

		જો (ul_path_readlinkf(pc, res, કદ(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		printf("readf: %s: %s\n", file, res);

	} નહીંતર જો (strcmp(command, "write-string") == 0) {
		અક્ષર *str;

		જો (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		જો (ul_path_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		જો (ul_path_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} નહીંતર જો (strcmp(command, "write-u64") == 0) {
		uint64_t અંક;

		જો (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		અંક = strtoumax(argv[optind++], ખાલી, 0);

		જો (ul_path_write_u64(pc, અંક, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		જો (ul_path_writef_u64(pc, અંક, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_path(pc);
	પાછા EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PATH */
