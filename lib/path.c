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
#അന്തര്ഭാവ <stdarg.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <inttypes.h>
#അന്തര്ഭാവ <errno.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "fileutils.h"
#അന്തര്ഭാവ "all-io.h"
#അന്തര്ഭാവ "vfs.h"
#അന്തര്ഭാവ "path.h"
#അന്തര്ഭാവ "debug.h"
#അന്തര്ഭാവ "strutils.h"

/*
 * Debug stuff (based on include/debug.h)
 */
സ്ഥിതം UL_DEBUG_DEFINE_MASK(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPATH_DEBUG_INIT	(1 << 1)
#define ULPATH_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

ശൂന്യം ul_path_init_debug(ശൂന്യം)
{
	എങ്കിൽ (ulpath_debug_mask)
		തിരികെ;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

ഘടന path_cxt *ul_new_path(സ്ഥിരം അക്ഷരം *dir, ...)
{
	ഘടന path_cxt *pc = calloc(1, വലുപ്പം(*pc));

	എങ്കിൽ (!pc)
		തിരികെ ശൂന്യ;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	എങ്കിൽ (dir) {
		പൂർണ്ണസംഖ്യ rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_path, dir, ap);
		va_end(ap);

		എങ്കിൽ (rc < 0 || !pc->dir_path)
			പോകുക fail;
	}
	തിരികെ pc;
fail:
	ul_unref_path(pc);
	തിരികെ ശൂന്യ;
}

ശൂന്യം ul_ref_path(ഘടന path_cxt *pc)
{
	എങ്കിൽ (pc)
		pc->refcount++;
}

ശൂന്യം ul_unref_path(ഘടന path_cxt *pc)
{
	എങ്കിൽ (!pc)
		തിരികെ;

	pc->refcount--;

	എങ്കിൽ (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		എങ്കിൽ (pc->dialect)
			pc->free_dialect(pc);
		ul_path_close_dirfd(pc);
		സ്വതന്ത്രം(pc->dir_path);
		സ്വതന്ത്രം(pc->prefix);
		സ്വതന്ത്രം(pc);
	}
}

ശൂന്യം ul_path_refer_vfs(ഘടന path_cxt *pc, സ്ഥിരം ഘടന ul_vfs_ops *vfs)
{
	എങ്കിൽ (pc)
		pc->vfs = vfs;
}

പൂർണ്ണസംഖ്യ ul_path_set_prefix(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *prefix)
{
	അക്ഷരം *p = ശൂന്യ;

	പ്രതിജ്ഞാ(pc->dir_fd < 0);

	എങ്കിൽ (prefix) {
		p = strdup(prefix);
		എങ്കിൽ (!p)
			തിരികെ -ENOMEM;
	}

	സ്വതന്ത്രം(pc->prefix);
	pc->prefix = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	തിരികെ 0;
}

സ്ഥിരം അക്ഷരം *ul_path_get_prefix(ഘടന path_cxt *pc)
{
	തിരികെ pc ? pc->prefix : ശൂന്യ;
}

പൂർണ്ണസംഖ്യ ul_path_set_dir(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *dir)
{
	അക്ഷരം *p = ശൂന്യ;

	എങ്കിൽ (dir) {
		p = strdup(dir);
		എങ്കിൽ (!p)
			തിരികെ -ENOMEM;
	}

	എങ്കിൽ (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	സ്വതന്ത്രം(pc->dir_path);
	pc->dir_path = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	തിരികെ 0;
}

സ്ഥിരം അക്ഷരം *ul_path_get_dir(ഘടന path_cxt *pc)
{
	തിരികെ pc ? pc->dir_path : ശൂന്യ;
}

പൂർണ്ണസംഖ്യ ul_path_set_dialect(ഘടന path_cxt *pc, ശൂന്യം *data, ശൂന്യം free_data(ഘടന path_cxt *))
{
	pc->dialect = data;
	pc->free_dialect = free_data;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	തിരികെ 0;
}

ശൂന്യം *ul_path_get_dialect(ഘടന path_cxt *pc)
{
	തിരികെ pc ? pc->dialect : ശൂന്യ;
}

പൂർണ്ണസംഖ്യ ul_path_set_enoent_redirect(ഘടന path_cxt *pc, പൂർണ്ണസംഖ്യ (*func)(ഘടന path_cxt *, സ്ഥിരം അക്ഷരം *, പൂർണ്ണസംഖ്യ *))
{
	pc->redirect_on_enoent = func;
	തിരികെ 0;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *get_absdir(ഘടന path_cxt *pc)
{
	പൂർണ്ണസംഖ്യ rc;
	സ്ഥിരം അക്ഷരം *dirpath;

	എങ്കിൽ (!pc->prefix)
		തിരികെ pc->dir_path;

	dirpath = pc->dir_path;
	എങ്കിൽ (!dirpath)
		തിരികെ pc->prefix;
	എങ്കിൽ (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->path_buffer, വലുപ്പം(pc->path_buffer), "%s/%s", pc->prefix, dirpath);
	എങ്കിൽ (rc < 0)
		തിരികെ ശൂന്യ;
	എങ്കിൽ ((size_t)rc >= വലുപ്പം(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		തിരികെ ശൂന്യ;
	}

	തിരികെ pc->path_buffer;
}

പൂർണ്ണസംഖ്യ ul_path_is_accessible(ഘടന path_cxt *pc)
{
	സ്ഥിരം അക്ഷരം *path;
	പ്രതിജ്ഞാ(pc);

	എങ്കിൽ (pc->dir_fd >= 0)
		തിരികെ 1;

	path = get_absdir(pc);
	എങ്കിൽ (!path)
		തിരികെ 0;
	തിരികെ access(path, F_OK) == 0;
}

പൂർണ്ണസംഖ്യ ul_path_get_dirfd(ഘടന path_cxt *pc)
{
	പ്രതിജ്ഞാ(pc);
	പ്രതിജ്ഞാ(pc->dir_path);

	എങ്കിൽ (pc->dir_fd < 0) {
		സ്ഥിരം അക്ഷരം *path = get_absdir(pc);
		എങ്കിൽ (!path)
			തിരികെ -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", path));
		pc->dir_fd = open(path, O_RDONLY|O_CLOEXEC);
	}

	തിരികെ pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
ശൂന്യം ul_path_close_dirfd(ഘടന path_cxt *pc)
{
	പ്രതിജ്ഞാ(pc);

	എങ്കിൽ (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

പൂർണ്ണസംഖ്യ ul_path_isopen_dirfd(ഘടന path_cxt *pc)
{
	തിരികെ pc && pc->dir_fd >= 0;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *ul_path_mkpath(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *path, va_list ap)
{
	പൂർണ്ണസംഖ്യ rc;

	errno = 0;

	rc = vsnprintf(pc->path_buffer, വലുപ്പം(pc->path_buffer), path, ap);
	എങ്കിൽ (rc < 0) {
		എങ്കിൽ (!errno)
			errno = EINVAL;
		തിരികെ ശൂന്യ;
	}

	എങ്കിൽ ((size_t)rc >= വലുപ്പം(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		തിരികെ ശൂന്യ;
	}

	തിരികെ pc->path_buffer;
}

അക്ഷരം *ul_path_get_abspath(ഘടന path_cxt *pc, അക്ഷരം *buf, size_t bufsz, സ്ഥിരം അക്ഷരം *path, ...)
{
	എങ്കിൽ (path) {
		പൂർണ്ണസംഖ്യ rc;
		va_list ap;
		സ്ഥിരം അക്ഷരം *tail = ശൂന്യ, *dirpath = pc->dir_path;

		va_start(ap, path);
		tail = ul_path_mkpath(pc, path, ap);
		va_end(ap);

		എങ്കിൽ (dirpath && *dirpath == '/')
			dirpath++;
		എങ്കിൽ (tail && *tail == '/')
			tail++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->prefix ? pc->prefix : "",
				dirpath ? dirpath : "",
				tail ? tail : "");

		എങ്കിൽ ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			തിരികെ ശൂന്യ;
		}
	} അല്ലെങ്കിൽ {
		സ്ഥിരം അക്ഷരം *tmp = get_absdir(pc);

		എങ്കിൽ (!tmp)
			തിരികെ ശൂന്യ;
		xstrncpy(buf, tmp, bufsz);
	}

	തിരികെ buf;
}


പൂർണ്ണസംഖ്യ ul_path_access(ഘടന path_cxt *pc, പൂർണ്ണസംഖ്യ mode, സ്ഥിരം അക്ഷരം *path)
{
	പൂർണ്ണസംഖ്യ rc;

	എങ്കിൽ (!path)
		തിരികെ -EINVAL;
	എങ്കിൽ (!pc) {
		rc = access(path, mode);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", path, rc));
	} അല്ലെങ്കിൽ {
		പൂർണ്ണസംഖ്യ dir = ul_path_get_dirfd(pc);
		എങ്കിൽ (dir < 0)
			തിരികെ dir;
		എങ്കിൽ (*path == '/')
			path++;

		rc = faccessat(dir, path, mode, 0);

		എങ്കിൽ (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = faccessat(dir, path, mode, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", path, rc));
	}
	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ ul_path_accessf(ഘടന path_cxt *pc, പൂർണ്ണസംഖ്യ mode, സ്ഥിരം അക്ഷരം *path, ...)
{
	va_list ap;
	സ്ഥിരം അക്ഷരം *p;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	തിരികെ !p ? -errno : ul_path_access(pc, mode, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
പൂർണ്ണസംഖ്യ ul_path_stat(ഘടന path_cxt *pc, ഘടന stat *sb, പൂർണ്ണസംഖ്യ flags, സ്ഥിരം അക്ഷരം *path)
{
	പൂർണ്ണസംഖ്യ rc;

	എങ്കിൽ (!pc) {
		rc = path ? stat(path, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", path, rc));
	} അല്ലെങ്കിൽ {
		പൂർണ്ണസംഖ്യ dir = ul_path_get_dirfd(pc);
		എങ്കിൽ (dir < 0)
			തിരികെ dir;
		എങ്കിൽ (path) {
			എങ്കിൽ  (*path == '/')
				path++;
			rc = fstatat(dir, path, sb, flags);

		} അല്ലെങ്കിൽ
			rc = fstat(dir, sb);	/* dir itself */

		എങ്കിൽ (rc && errno == ENOENT
		    && path
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = fstatat(dir, path, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", path, rc));
	}
	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ ul_path_vstatf(ഘടന path_cxt *pc, ഘടന stat *sb, പൂർണ്ണസംഖ്യ flags, സ്ഥിരം അക്ഷരം *path, va_list ap)
{
	സ്ഥിരം അക്ഷരം *p = ul_path_mkpath(pc, path, ap);

	തിരികെ !p ? -errno : ul_path_stat(pc, sb, flags, p);
}

പൂർണ്ണസംഖ്യ ul_path_statf(ഘടന path_cxt *pc, ഘടന stat *sb, പൂർണ്ണസംഖ്യ flags, സ്ഥിരം അക്ഷരം *path, ...)
{
	va_list ap;
	പൂർണ്ണസംഖ്യ rc;

	va_start(ap, path);
	rc = ul_path_vstatf(pc, sb, flags, path, ap);
	va_end(ap);

	തിരികെ rc;
}
#ifdef HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
പൂർണ്ണസംഖ്യ ul_path_statx(ഘടന path_cxt *pc, ഘടന statx *stx, പൂർണ്ണസംഖ്യ flags, unsigned പൂർണ്ണസംഖ്യ mask,
		  സ്ഥിരം അക്ഷരം *path)
{
	പൂർണ്ണസംഖ്യ rc;

	എങ്കിൽ (!path)
		തിരികെ -EINVAL;
	എങ്കിൽ (!pc)
		rc = path ? statx(AT_FDCWD, path, flags, mask, stx) : - EINVAL;
	അല്ലെങ്കിൽ {
		പൂർണ്ണസംഖ്യ dir = ul_path_get_dirfd(pc);
		എങ്കിൽ (dir < 0)
			തിരികെ dir;
		എങ്കിൽ (*path == '/')
			path++;

		rc = statx(dir, path, flags, mask, stx);
	}

	തിരികെ rc;
}
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ ul_path_statx(ഘടന path_cxt *pc __attribute__((__unused__)),
		  ഘടന statx *stx __attribute__((__unused__)),
		  പൂർണ്ണസംഖ്യ flags __attribute__((__unused__)),
		  unsigned പൂർണ്ണസംഖ്യ mask __attribute__((__unused__)),
		  സ്ഥിരം അക്ഷരം *path __attribute__((__unused__)))
{
	errno = ENOSYS;
	തിരികെ -1;
}
#endif /* HAVE_STATX */

പൂർണ്ണസംഖ്യ ul_path_vstatxf(ഘടന path_cxt *pc, ഘടന statx *stx, പൂർണ്ണസംഖ്യ flags, unsigned പൂർണ്ണസംഖ്യ mask,
		    സ്ഥിരം അക്ഷരം *path, va_list ap)
{
	സ്ഥിരം അക്ഷരം *p = ul_path_mkpath(pc, path, ap);

	തിരികെ !p ? -errno : ul_path_statx(pc, stx, flags, mask, p);
}

പൂർണ്ണസംഖ്യ ul_path_statxf(ഘടന path_cxt *pc, ഘടന statx *stx, പൂർണ്ണസംഖ്യ flags, unsigned പൂർണ്ണസംഖ്യ mask,
		   സ്ഥിരം അക്ഷരം *path, ...)
{
	va_list ap;
	പൂർണ്ണസംഖ്യ rc;

	va_start(ap, path);
	rc = ul_path_vstatxf(pc, stx, flags, mask, path, ap);
	va_end(ap);

	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ ul_path_open(ഘടന path_cxt *pc, പൂർണ്ണസംഖ്യ flags, സ്ഥിരം അക്ഷരം *path)
{
	പൂർണ്ണസംഖ്യ fd;

	എങ്കിൽ (!path)
		തിരികെ -EINVAL;
	എങ്കിൽ (!pc) {
		fd = open(path, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", path));
	} അല്ലെങ്കിൽ {
		പൂർണ്ണസംഖ്യ fdx;
		പൂർണ്ണസംഖ്യ dir = ul_path_get_dirfd(pc);
		എങ്കിൽ (dir < 0)
			തിരികെ dir;

		എങ്കിൽ (*path == '/')
			path++;

		fdx = fd = openat(dir, path, flags);

		എങ്കിൽ (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			fd = openat(dir, path, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", path, fdx != fd ? " [redirected]" : ""));
	}
	തിരികെ fd;
}

പൂർണ്ണസംഖ്യ ul_path_vopenf(ഘടന path_cxt *pc, പൂർണ്ണസംഖ്യ flags, സ്ഥിരം അക്ഷരം *path, va_list ap)
{
	സ്ഥിരം അക്ഷരം *p = ul_path_mkpath(pc, path, ap);

	തിരികെ !p ? -errno : ul_path_open(pc, flags, p);
}

പൂർണ്ണസംഖ്യ ul_path_openf(ഘടന path_cxt *pc, പൂർണ്ണസംഖ്യ flags, സ്ഥിരം അക്ഷരം *path, ...)
{
	va_list ap;
	പൂർണ്ണസംഖ്യ rc;

	va_start(ap, path);
	rc = ul_path_vopenf(pc, flags, path, ap);
	va_end(ap);

	തിരികെ rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_path_fopen(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *mode, സ്ഥിരം അക്ഷരം *path)
{
	പൂർണ്ണസംഖ്യ flags = ul_mode_to_flags(mode);
	പൂർണ്ണസംഖ്യ fd = ul_path_open(pc, flags, path);

	എങ്കിൽ (fd < 0)
		തിരികെ ശൂന്യ;

	തിരികെ ul_vfs_fdopen(pc ? pc->vfs : ശൂന്യ, fd, mode);
}


FILE *ul_path_vfopenf(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *mode, സ്ഥിരം അക്ഷരം *path, va_list ap)
{
	സ്ഥിരം അക്ഷരം *p = ul_path_mkpath(pc, path, ap);

	തിരികെ !p ? ശൂന്യ : ul_path_fopen(pc, mode, p);
}

FILE *ul_path_fopenf(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *mode, സ്ഥിരം അക്ഷരം *path, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, path);
	f = ul_path_vfopenf(pc, mode, path, ap);
	va_end(ap);

	തിരികെ f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendir(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *path)
{
	DIR *dir;
	പൂർണ്ണസംഖ്യ fd = -1;

	എങ്കിൽ (path)
		fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	അല്ലെങ്കിൽ എങ്കിൽ (pc->dir_path) {
		പൂർണ്ണസംഖ്യ dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_path_get_dirfd(pc);
		എങ്കിൽ (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	എങ്കിൽ (fd < 0)
		തിരികെ ശൂന്യ;

	dir = fdopendir(fd);
	എങ്കിൽ (!dir) {
		close(fd);
		തിരികെ ശൂന്യ;
	}
	എങ്കിൽ (!path)
		 rewinddir(dir);
	തിരികെ dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_vopendirf(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *path, va_list ap)
{
	സ്ഥിരം അക്ഷരം *p = ul_path_mkpath(pc, path, ap);

	തിരികെ !p ? ശൂന്യ : ul_path_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendirf(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *path, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, path);
	dir = ul_path_vopendirf(pc, path, ap);
	va_end(ap);

	തിരികെ dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlink(ഘടന path_cxt *pc, അക്ഷരം *buf, size_t bufsiz, സ്ഥിരം അക്ഷരം *path)
{
	പൂർണ്ണസംഖ്യ dirfd;
	ssize_t ssz;

	എങ്കിൽ (!path) {
		സ്ഥിരം അക്ഷരം *p = get_absdir(pc);
		എങ്കിൽ (!p)
			തിരികെ -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} അല്ലെങ്കിൽ {
		dirfd = ul_path_get_dirfd(pc);
		എങ്കിൽ (dirfd < 0)
			തിരികെ dirfd;

		എങ്കിൽ (*path == '/')
			path++;

		ssz = readlinkat(dirfd, path, buf, bufsiz - 1);
	}

	എങ്കിൽ (ssz >= 0)
		buf[ssz] = '\0';
	തിരികെ ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlinkf(ഘടന path_cxt *pc, അക്ഷരം *buf, size_t bufsiz, സ്ഥിരം അക്ഷരം *path, ...)
{
	സ്ഥിരം അക്ഷരം *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	തിരികെ !p ? -errno : ul_path_readlink(pc, buf, bufsiz, p);
}

പൂർണ്ണസംഖ്യ ul_path_read(ഘടന path_cxt *pc, അക്ഷരം *buf, size_t len, സ്ഥിരം അക്ഷരം *path)
{
	പൂർണ്ണസംഖ്യ rc, errsv;
	പൂർണ്ണസംഖ്യ fd;

	fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	എങ്കിൽ (fd < 0)
		തിരികെ -errno;

	DBG(CXT, ul_debug(" reading '%s'", path));
	rc = ul_vfs_read_all(pc ? pc->vfs : ശൂന്യ, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ശൂന്യ, fd);
	errno = errsv;
	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ ul_path_vreadf(ഘടന path_cxt *pc, അക്ഷരം *buf, size_t len, സ്ഥിരം അക്ഷരം *path, va_list ap)
{
	സ്ഥിരം അക്ഷരം *p = ul_path_mkpath(pc, path, ap);

	തിരികെ !p ? -errno : ul_path_read(pc, buf, len, p);
}

പൂർണ്ണസംഖ്യ ul_path_readf(ഘടന path_cxt *pc, അക്ഷരം *buf, size_t len, സ്ഥിരം അക്ഷരം *path, ...)
{
	va_list ap;
	പൂർണ്ണസംഖ്യ rc;

	va_start(ap, path);
	rc = ul_path_vreadf(pc, buf, len, path, ap);
	va_end(ap);

	തിരികെ rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
പൂർണ്ണസംഖ്യ ul_path_read_string(ഘടന path_cxt *pc, അക്ഷരം **str, സ്ഥിരം അക്ഷരം *path)
{
	അക്ഷരം buf[BUFSIZ];
	പൂർണ്ണസംഖ്യ rc;

	എങ്കിൽ (!str)
		തിരികെ -EINVAL;

	*str = ശൂന്യ;

	rc = ul_path_read_buffer(pc, buf, വലുപ്പം(buf), path);
	എങ്കിൽ (rc < 0)
		തിരികെ rc;

	*str = strdup(buf);
	എങ്കിൽ (!*str)
		rc = -ENOMEM;

	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ ul_path_readf_string(ഘടന path_cxt *pc, അക്ഷരം **str, സ്ഥിരം അക്ഷരം *path, ...)
{
	സ്ഥിരം അക്ഷരം *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	തിരികെ !p ? -errno : ul_path_read_string(pc, str, p);
}

പൂർണ്ണസംഖ്യ ul_path_read_buffer(ഘടന path_cxt *pc, അക്ഷരം *buf, size_t bufsz, സ്ഥിരം അക്ഷരം *path)
{
	പൂർണ്ണസംഖ്യ rc = ul_path_read(pc, buf, bufsz - 1, path);

	എങ്കിൽ (rc == 0)
		buf[0] = '\0';

	അല്ലെങ്കിൽ എങ്കിൽ (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		എങ്കിൽ (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		അല്ലെങ്കിൽ
			buf[rc] = '\0';
	}

	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ ul_path_vreadf_buffer(ഘടന path_cxt *pc, അക്ഷരം *buf, size_t bufsz, സ്ഥിരം അക്ഷരം *path, va_list ap)
{
	സ്ഥിരം അക്ഷരം *p;

	p = ul_path_mkpath(pc, path, ap);

	തിരികെ !p ? -errno : ul_path_read_buffer(pc, buf, bufsz, p);
}

പൂർണ്ണസംഖ്യ ul_path_readf_buffer(ഘടന path_cxt *pc, അക്ഷരം *buf, size_t bufsz, സ്ഥിരം അക്ഷരം *path, ...)
{
	va_list ap;
	പൂർണ്ണസംഖ്യ rc;

	va_start(ap, path);
	rc = ul_path_vreadf_buffer(pc, buf, bufsz, path, ap);
	va_end(ap);

	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ ul_path_scanf(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *path, സ്ഥിരം അക്ഷരം *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	പൂർണ്ണസംഖ്യ rc;

	f = ul_path_fopen(pc, "r" UL_CLOEXECSTR, path);
	എങ്കിൽ (!f)
		തിരികെ -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, path));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ ul_path_scanff(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *path, va_list ap, സ്ഥിരം അക്ഷരം *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	പൂർണ്ണസംഖ്യ rc;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	എങ്കിൽ (!f)
		തിരികെ -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	തിരികെ rc;
}


പൂർണ്ണസംഖ്യ ul_path_read_s64(ഘടന path_cxt *pc, int64_t *res, സ്ഥിരം അക്ഷരം *path)
{
	int64_t x = 0;
	പൂർണ്ണസംഖ്യ rc;

	rc = ul_path_scanf(pc, path, "%"SCNd64, &x);
	എങ്കിൽ (rc != 1)
		തിരികെ -1;
	എങ്കിൽ (res)
		*res = x;
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ ul_path_readf_s64(ഘടന path_cxt *pc, int64_t *res, സ്ഥിരം അക്ഷരം *path, ...)
{
	സ്ഥിരം അക്ഷരം *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	തിരികെ !p ? -errno : ul_path_read_s64(pc, res, p);
}

പൂർണ്ണസംഖ്യ ul_path_read_u64(ഘടന path_cxt *pc, uint64_t *res, സ്ഥിരം അക്ഷരം *path)
{
	uint64_t x = 0;
	പൂർണ്ണസംഖ്യ rc;

	rc = ul_path_scanf(pc, path, "%"SCNu64, &x);
	എങ്കിൽ (rc != 1)
		തിരികെ -1;
	എങ്കിൽ (res)
		*res = x;
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ ul_path_readf_u64(ഘടന path_cxt *pc, uint64_t *res, സ്ഥിരം അക്ഷരം *path, ...)
{
	സ്ഥിരം അക്ഷരം *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	തിരികെ !p ? -errno : ul_path_read_u64(pc, res, p);
}

പൂർണ്ണസംഖ്യ ul_path_read_s32(ഘടന path_cxt *pc, പൂർണ്ണസംഖ്യ *res, സ്ഥിരം അക്ഷരം *path)
{
	പൂർണ്ണസംഖ്യ rc, x = 0;

	rc = ul_path_scanf(pc, path, "%d", &x);
	എങ്കിൽ (rc != 1)
		തിരികെ -1;
	എങ്കിൽ (res)
		*res = x;
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ ul_path_readf_s32(ഘടന path_cxt *pc, പൂർണ്ണസംഖ്യ *res, സ്ഥിരം അക്ഷരം *path, ...)
{
	സ്ഥിരം അക്ഷരം *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	തിരികെ !p ? -errno : ul_path_read_s32(pc, res, p);
}

പൂർണ്ണസംഖ്യ ul_path_read_u32(ഘടന path_cxt *pc, unsigned പൂർണ്ണസംഖ്യ *res, സ്ഥിരം അക്ഷരം *path)
{
	പൂർണ്ണസംഖ്യ rc;
	unsigned പൂർണ്ണസംഖ്യ x = 0;

	rc = ul_path_scanf(pc, path, "%u", &x);
	എങ്കിൽ (rc != 1)
		തിരികെ -1;
	എങ്കിൽ (res)
		*res = x;
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ ul_path_readf_u32(ഘടന path_cxt *pc, unsigned പൂർണ്ണസംഖ്യ *res, സ്ഥിരം അക്ഷരം *path, ...)
{
	സ്ഥിരം അക്ഷരം *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	തിരികെ !p ? -errno : ul_path_read_u32(pc, res, p);
}

പൂർണ്ണസംഖ്യ ul_path_read_majmin(ഘടന path_cxt *pc, dev_t *res, സ്ഥിരം അക്ഷരം *path)
{
	പൂർണ്ണസംഖ്യ rc, maj = 0, min = 0;

	rc = ul_path_scanf(pc, path, "%d:%d", &maj, &min);
	എങ്കിൽ (rc != 2)
		തിരികെ -1;
	എങ്കിൽ (res)
		*res = makedev(maj, min);
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ ul_path_readf_majmin(ഘടന path_cxt *pc, dev_t *res, സ്ഥിരം അക്ഷരം *path, ...)
{
	സ്ഥിരം അക്ഷരം *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	തിരികെ !p ? -errno : ul_path_read_majmin(pc, res, p);
}

പൂർണ്ണസംഖ്യ ul_path_write_string(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *str, സ്ഥിരം അക്ഷരം *path)
{
	പൂർണ്ണസംഖ്യ rc, errsv;
	പൂർണ്ണസംഖ്യ fd;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	എങ്കിൽ (fd < 0)
		തിരികെ -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : ശൂന്യ, fd, str, strlen(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ശൂന്യ, fd);
	errno = errsv;
	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ ul_path_writef_string(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *str, സ്ഥിരം അക്ഷരം *path, ...)
{
	സ്ഥിരം അക്ഷരം *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	തിരികെ !p ? -errno : ul_path_write_string(pc, str, p);
}

പൂർണ്ണസംഖ്യ ul_path_write_s64(ഘടന path_cxt *pc, int64_t അക്കം, സ്ഥിരം അക്ഷരം *path)
{
	അക്ഷരം buf[വലുപ്പം(stringify_മൂല്യം(LLONG_MAX))];
	പൂർണ്ണസംഖ്യ rc, errsv;
	പൂർണ്ണസംഖ്യ fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	എങ്കിൽ (fd < 0)
		തിരികെ -errno;

	len = snprintf(buf, വലുപ്പം(buf), "%" PRId64, അക്കം);
	എങ്കിൽ (len < 0 || (size_t) len >= വലുപ്പം(buf))
		rc = len < 0 ? -errno : -E2BIG;
	അല്ലെങ്കിൽ
		rc = ul_vfs_write_all(pc ? pc->vfs : ശൂന്യ, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ശൂന്യ, fd);
	errno = errsv;
	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ ul_path_write_u64(ഘടന path_cxt *pc, uint64_t അക്കം, സ്ഥിരം അക്ഷരം *path)
{
	അക്ഷരം buf[വലുപ്പം(stringify_മൂല്യം(ULLONG_MAX))];
	പൂർണ്ണസംഖ്യ rc, errsv;
	പൂർണ്ണസംഖ്യ fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	എങ്കിൽ (fd < 0)
		തിരികെ -errno;

	len = snprintf(buf, വലുപ്പം(buf), "%" PRIu64, അക്കം);
	എങ്കിൽ (len < 0 || (size_t) len >= വലുപ്പം(buf))
		rc = len < 0 ? -errno : -E2BIG;
	അല്ലെങ്കിൽ
		rc = ul_vfs_write_all(pc ? pc->vfs : ശൂന്യ, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ശൂന്യ, fd);
	errno = errsv;
	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ ul_path_writef_u64(ഘടന path_cxt *pc, uint64_t അക്കം, സ്ഥിരം അക്ഷരം *path, ...)
{
	സ്ഥിരം അക്ഷരം *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	തിരികെ !p ? -errno : ul_path_write_u64(pc, അക്കം, p);

}

പൂർണ്ണസംഖ്യ ul_path_എണ്ണം_dirents(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *path)
{
	DIR *dir;
	പൂർണ്ണസംഖ്യ r = 0;

	dir = ul_path_opendir(pc, path);
	എങ്കിൽ (!dir)
		തിരികെ 0;

	ആകുമ്പോൾ (xreaddir(dir)) r++;

	closedir(dir);
	തിരികെ r;
}

പൂർണ്ണസംഖ്യ ul_path_countf_dirents(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *path, ...)
{
	സ്ഥിരം അക്ഷരം *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	തിരികെ !p ? -errno : ul_path_എണ്ണം_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
പൂർണ്ണസംഖ്യ ul_path_next_dirent(ഘടന path_cxt *pc, DIR **sub, സ്ഥിരം അക്ഷരം *dirname, ഘടന dirent **d)
{
	എങ്കിൽ (!pc || !sub || !d)
		തിരികെ -EINVAL;

	എങ്കിൽ (!*sub) {
		*sub = ul_path_opendir(pc, dirname);
		എങ്കിൽ (!*sub)
			തിരികെ -errno;
	}

	*d = xreaddir(*sub);
	എങ്കിൽ (*d)
		തിരികെ 0;

	closedir(*sub);
	*sub = ശൂന്യ;
	തിരികെ 1;
}

#ifdef HAVE_CPU_SET_T
സ്ഥിതം പൂർണ്ണസംഖ്യ ul_path_cpuparse(ഘടന path_cxt *pc, cpu_set_t **set, പൂർണ്ണസംഖ്യ maxcpus, പൂർണ്ണസംഖ്യ islist, സ്ഥിരം അക്ഷരം *path, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	അക്ഷരം *buf;
	പൂർണ്ണസംഖ്യ rc;

	*set = ശൂന്യ;

	buf = malloc(len);
	എങ്കിൽ (!buf)
		തിരികെ -ENOMEM;

	rc = ul_path_vreadf_buffer(pc, buf, len, path, ap);
	എങ്കിൽ (rc < 0)
		പോകുക out;

	*set = cpuset_alloc(maxcpus, &setsize, ശൂന്യ);
	എങ്കിൽ (!*set) {
		rc = -EINVAL;
		പോകുക out;
	}

	എങ്കിൽ (islist) {
		എങ്കിൽ (cpulist_parse(buf, *set, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			പോകുക out;
		}
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (cpumask_parse(buf, *set, setsize)) {
			errno = EINVAL;
			rc = -errno;
			പോകുക out;
		}
	}
	rc = 0;

out:
	എങ്കിൽ (rc) {
		cpuset_free(*set);
		*set = ശൂന്യ;
	}
	സ്വതന്ത്രം(buf);
	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ ul_path_readf_cpuset(ഘടന path_cxt *pc, cpu_set_t **set, പൂർണ്ണസംഖ്യ maxcpus, സ്ഥിരം അക്ഷരം *path, ...)
{
	va_list ap;
	പൂർണ്ണസംഖ്യ rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	va_end(ap);

	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ ul_path_readf_cpulist(ഘടന path_cxt *pc, cpu_set_t **set, പൂർണ്ണസംഖ്യ maxcpus, സ്ഥിരം അക്ഷരം *path, ...)
{
	va_list ap;
	പൂർണ്ണസംഖ്യ rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	va_end(ap);

	തിരികെ rc;
}

#endif /* HAVE_CPU_SET_T */


#ifdef TEST_PROGRAM_PATH
#അന്തര്ഭാവ <getopt.h>

സ്ഥിതം ശൂന്യം __attribute__((__noreturn__)) usage(ശൂന്യം)
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

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	പൂർണ്ണസംഖ്യ c;
	സ്ഥിരം അക്ഷരം *prefix = ശൂന്യ, *dir, *file, *command;
	ഘടന path_cxt *pc = ശൂന്യ;

	സ്ഥിതം സ്ഥിരം ഘടന option longopts[] = {
		{ "prefix",	1, ശൂന്യ, 'p' },
		{ "help",       0, ശൂന്യ, 'h' },
		{ ശൂന്യ, 0, ശൂന്യ, 0 },
	};

	ആകുമ്പോൾ((c = getopt_long(argc, argv, "p:h", longopts, ശൂന്യ)) != -1) {
		തിരഞ്ഞെടുപ്പ്(c) {
		അവസ്ഥ 'p':
			prefix = optarg;
			നിർത്തുക;
		അവസ്ഥ 'h':
			usage();
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			err(EXIT_FAILURE, "try --help");
		}
	}

	എങ്കിൽ (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_path_init_debug();

	pc = ul_new_path("%s", dir);
	എങ്കിൽ (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	എങ്കിൽ (prefix)
		ul_path_set_prefix(pc, prefix);

	എങ്കിൽ (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	എങ്കിൽ (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		എങ്കിൽ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		എങ്കിൽ (ul_path_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %u\n", file, res);

		എങ്കിൽ (ul_path_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %u\n", file, res);

	} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(command, "read-s32") == 0) {
		int32_t res;

		എങ്കിൽ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		എങ്കിൽ (ul_path_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %d\n", file, res);

		എങ്കിൽ (ul_path_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %d\n", file, res);

	} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		എങ്കിൽ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		എങ്കിൽ (ul_path_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRIu64 "\n", file, res);

		എങ്കിൽ (ul_path_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRIu64 "\n", file, res);

	} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(command, "read-s64") == 0) {
		int64_t res;

		എങ്കിൽ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		എങ്കിൽ (ul_path_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRId64 "\n", file, res);

		എങ്കിൽ (ul_path_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRId64 "\n", file, res);

	} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		എങ്കിൽ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		എങ്കിൽ (ul_path_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		printf("read:  %s: %d\n", file, (പൂർണ്ണസംഖ്യ) res);

		എങ്കിൽ (ul_path_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		printf("readf: %s: %d\n", file, (പൂർണ്ണസംഖ്യ) res);

	} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(command, "read-string") == 0) {
		അക്ഷരം *res;

		എങ്കിൽ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		എങ്കിൽ (ul_path_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		printf("read:  %s: %s\n", file, res);

		എങ്കിൽ (ul_path_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		printf("readf: %s: %s\n", file, res);

	} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(command, "read-link") == 0) {
		അക്ഷരം res[PATH_MAX];

		എങ്കിൽ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		എങ്കിൽ (ul_path_readlink(pc, res, വലുപ്പം(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		printf("read:  %s: %s\n", file, res);

		എങ്കിൽ (ul_path_readlinkf(pc, res, വലുപ്പം(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		printf("readf: %s: %s\n", file, res);

	} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(command, "write-string") == 0) {
		അക്ഷരം *str;

		എങ്കിൽ (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		എങ്കിൽ (ul_path_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		എങ്കിൽ (ul_path_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(command, "write-u64") == 0) {
		uint64_t അക്കം;

		എങ്കിൽ (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		അക്കം = strtoumax(argv[optind++], ശൂന്യ, 0);

		എങ്കിൽ (ul_path_write_u64(pc, അക്കം, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		എങ്കിൽ (ul_path_writef_u64(pc, അക്കം, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_path(pc);
	തിരികെ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PATH */
