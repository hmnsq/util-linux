/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2021]
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_SYS_VFS_H
# include <sys/vfs.h>
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "pathnames.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "procfs.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "fileutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "all-io.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "vfs.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "strutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "statfs_magic.h"

ਸਥਿਰ ਖਾਲੀ procfs_process_deinit_ਰਸਤਾ(ਬਣਤਰ ਰਸਤਾ_cxt *pc);

/*
 * Debug stuff (based on include/debug.h)
 */
ਸਥਿਰ UL_DEBUG_DEFINE_ਨਕਾਬ(ulprocfs);
UL_DEBUG_DEFINE_MASKNAMES(ulprocfs) = UL_DEBUG_ਖਾਲੀ_ਥਾਵਾਂ_MASKNAMES;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULPROCFS_DEBUG_INIT	(1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULPROCFS_DEBUG_CXT	(1 << 2)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG(m, x)		__UL_DBG(ulprocfs, ULPROCFS_DEBUG_, m, x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulprocfs, ULPROCFS_DEBUG_, m, h, x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ON_DBG(m, x)		__UL_DBG_CALL(ulprocfs, ULPROCFS_DEBUG_, m, x)

ਖਾਲੀ ul_procfs_init_debug(ਖਾਲੀ)
{
	ਜੇ (ulprocfs_debug_ਨਕਾਬ)
		ਵਾਪਸ;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

ਬਣਤਰ ਰਸਤਾ_cxt *ul_new_procfs_ਰਸਤਾ(pid_t pid, ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ)
{
	ਬਣਤਰ ਰਸਤਾ_cxt *pc = ul_new_ਰਸਤਾ(ਨਿਰਾ);

	ਜੇ (!pc)
		ਵਾਪਸ ਨਿਰਾ;
	ਜੇ (ਅਗੇਤਰ)
		ul_ਰਸਤਾ_ਸੈੱਟ_ਅਗੇਤਰ(pc, ਅਗੇਤਰ);

	ਜੇ (procfs_process_init_ਰਸਤਾ(pc, pid) != 0) {
		ul_unref_ਰਸਤਾ(pc);
		ਵਾਪਸ ਨਿਰਾ;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ਵਾਪਸ pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
ਪੂਰਨ procfs_process_init_ਰਸਤਾ(ਬਣਤਰ ਰਸਤਾ_cxt *pc, pid_t pid)
{
	ਬਣਤਰ procfs_process *prc;
	ਪੂਰਨ rc;
	ਅੱਖਰ buf[ਆਕਾਰ(ਰਸਤਾ_PROC) + ਆਕਾਰ(stringify_ਮੁੱਲ(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, ਆਕਾਰ(buf), ਰਸਤਾ_PROC "/%zu", (size_t) pid);
	rc = ul_ਰਸਤਾ_ਸੈੱਟ_dir(pc, buf);
	ਜੇ (rc)
		ਵਾਪਸ rc;

	/* make sure path exists */
	rc = ul_ਰਸਤਾ_get_dirfd(pc);
	ਜੇ (rc < 0)
		ਵਾਪਸ rc;

	/* initialize procfs specific stuff */
	prc = ul_ਰਸਤਾ_get_dialect(pc);
	ਜੇ (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ procfs_process));
		ਜੇ (!prc)
			ਵਾਪਸ -ENOMEM;

		ul_ਰਸਤਾ_ਸੈੱਟ_dialect(pc, prc, procfs_process_deinit_ਰਸਤਾ);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ procfs_process_deinit_ਰਸਤਾ(ਬਣਤਰ ਰਸਤਾ_cxt *pc)
{
	ਬਣਤਰ procfs_process *prc;

	ਜੇ (!pc)
		ਵਾਪਸ;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_ਰਸਤਾ_get_dialect(pc);
	ਜੇ (!prc)
		ਵਾਪਸ;

	ਮੁਕਤ(prc);
	ul_ਰਸਤਾ_ਸੈੱਟ_dialect(pc, ਨਿਰਾ, ਨਿਰਾ);
}

ਸਥਿਰ ssize_t read_procfs_file(ਅਚਲ ਬਣਤਰ ul_vfs_ops *vfs, ਪੂਰਨ fd,
			       ਅੱਖਰ *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	ਜੇ (fd < 0)
		ਵਾਪਸ -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	ਜੇ (sz <= 0)
		ਵਾਪਸ sz;

	ਲਈ (i = 0; i < (size_t) sz; i++) {
		ਜੇ (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	ਵਾਪਸ sz;
}

ਸਥਿਰ ssize_t procfs_process_get_ਅੰਕੜੇ_for(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ *buf, size_t bufsz,
					    ਅਚਲ ਅੱਖਰ *fname)
{
	ਪੂਰਨ fd = ul_ਰਸਤਾ_open(pc, O_RDONLY|O_CLOEXEC, fname);

	ਜੇ (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : ਨਿਰਾ, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : ਨਿਰਾ, fd);
		ਵਾਪਸ sz;
	}
	ਵਾਪਸ -errno;
}

ssize_t procfs_process_get_cmdline(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ *buf, size_t bufsz)
{
	ਵਾਪਸ procfs_process_get_ਅੰਕੜੇ_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ *buf, size_t bufsz)
{
	ਵਾਪਸ procfs_process_get_ਅੰਕੜੇ_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ *buf, size_t bufsz)
{
	ਵਾਪਸ procfs_process_get_ਅੰਕੜੇ_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ *buf, size_t bufsz)
{
	ਵਾਪਸ procfs_process_get_ਅੰਕੜੇ_for(pc, buf, bufsz, "syscall");
}

ਪੂਰਨ procfs_process_get_stat_nth(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਪੂਰਨ n, uint64_t *re)
{
	ssize_t rc;
	ਅੱਖਰ *ਕੁੰਜੀ = ਨਿਰਾ, *tok, *p;
	ਅੱਖਰ buf[BUFSIZ];
	ਪੂਰਨ i;

	ਜੇ (n == 2 || n == 3)		/* process name and status (strings) */
		ਵਾਪਸ -EINVAL;

	rc = procfs_process_get_ਅੰਕੜੇ_for(pc, buf, ਆਕਾਰ(buf), "stat");
	ਜੇ (rc < 0)
		ਵਾਪਸ rc;

	ਲਈ (i = 0, tok = strtok_r(buf, " ", &ਕੁੰਜੀ); tok;
	     tok = strtok_r(ਨਿਰਾ, " ", &ਕੁੰਜੀ)) {

		i++;
		ਜੇ (i == n)
			ਵਾਪਸ ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		ਜੇ (i == 2 && (p = strrchr(ਕੁੰਜੀ, ')')))
			ਕੁੰਜੀ = p + 2;
	}

	ਵਾਪਸ -EINVAL;
}

ਪੂਰਨ procfs_process_get_uid(ਬਣਤਰ ਰਸਤਾ_cxt *pc, uid_t *uid)
{
	ਬਣਤਰ stat sb;
	ਪੂਰਨ rc;

	ਜੇ ((rc = ul_ਰਸਤਾ_stat(pc, &sb, 0, ਨਿਰਾ)) == 0)
		*uid = sb.st_uid;
	ਵਾਪਸ rc;
}

/*
 * returns the next task TID, the @sub is automatically initialized
 * when called first time and closed after last call or you can
 * call closedir()* when you need to break the loop.
 *
 * Returns: <0 on error, 0 on success, >1 done
 *
 * Example:
 *
 * pid_t tid;
 * DIR *sub = NULL;
 * path_cxt *pc = ul_new_procfs_path(123, NULL);
 *
 * while (procfs_process_next_tid(pc, &sub, &tid) == 0)
 *	printf("task: %d", (int) tid);
 *
 */
ਪੂਰਨ procfs_process_ਅਗਲਾ_tid(ਬਣਤਰ ਰਸਤਾ_cxt *pc, DIR **sub, pid_t *tid)
{
	ਬਣਤਰ dirent *d;

	ਜੇ (!pc || !sub || !tid)
		ਵਾਪਸ -EINVAL;

	ਜੇ (!*sub) {
		*sub = ul_ਰਸਤਾ_opendir(pc, "task");
		ਜੇ (!*sub)
			ਵਾਪਸ -errno;
	}

	ਜਦੋਂ ((d = xreaddir(*sub))) {
		ਜੇ (procfs_dirent_get_pid(d, tid) == 0)
			ਵਾਪਸ 0;
	}

	closedir(*sub);
	*sub = ਨਿਰਾ;
	ਵਾਪਸ 1;
}

ਪੂਰਨ procfs_process_ਅਗਲਾ_fd(ਬਣਤਰ ਰਸਤਾ_cxt *pc, DIR **sub, ਪੂਰਨ *fd)
{
	ਬਣਤਰ dirent *d;

	ਜੇ (!pc || !sub || !fd)
		ਵਾਪਸ -EINVAL;

	ਜੇ (!*sub) {
		*sub = ul_ਰਸਤਾ_opendir(pc, "fd");
		ਜੇ (!*sub)
			ਵਾਪਸ -errno;
	}

	ਜਦੋਂ ((d = xreaddir(*sub))) {
		uint64_t ਅੰਕ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _DIRENT_HAVE_D_TYPE
		ਜੇ (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			ਜਾਰੀ;
#ਜੇ_ਅੰਤ
		ਜੇ (ul_strtou64(d->d_name, &ਅੰਕ, 10) < 0)
			ਜਾਰੀ;
		*fd = ਅੰਕ;
		ਵਾਪਸ 0;
	}

	closedir(*sub);
	*sub = ਨਿਰਾ;
	ਵਾਪਸ 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
ਪੂਰਨ procfs_dirent_is_process(ਬਣਤਰ dirent *d)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _DIRENT_HAVE_D_TYPE
	ਜੇ (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
	ਜੇ (!ਅੰਕ_ਹੈ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *d->d_name))
		ਵਾਪਸ 0;

	ਵਾਪਸ 1;
}

ਪੂਰਨ procfs_dirent_get_pid(ਬਣਤਰ dirent *d, pid_t *pid)
{
	uint64_t ਅੰਕ;

	ਜੇ (!procfs_dirent_is_process(d))
		ਵਾਪਸ -EINVAL;

	ਜੇ (ul_strtou64(d->d_name, &ਅੰਕ, 10) < 0)
		ਵਾਪਸ -EINVAL;

	*pid = (pid_t) ਅੰਕ;
	ਵਾਪਸ 0;
}

ਪੂਰਨ procfs_dirent_get_uid(DIR *procfs, ਬਣਤਰ dirent *d, uid_t *uid)
{
	ਬਣਤਰ stat st;

	ਜੇ (!procfs_dirent_is_process(d))
		ਵਾਪਸ -EINVAL;

	ਜੇ (fstatat(dirfd(procfs), d->d_name, &st, 0))
		ਵਾਪਸ -EINVAL;

	*uid = st.st_uid;
	ਵਾਪਸ 0;
}

ਪੂਰਨ procfs_dirent_ਮੇਲ_uid(DIR *procfs, ਬਣਤਰ dirent *d, uid_t uid)
{
	uid_t x;

	ਜੇ (procfs_dirent_get_uid(procfs, d, &x) == 0)
		ਵਾਪਸ x == uid;

	ਵਾਪਸ 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
ਪੂਰਨ procfs_dirent_get_name(DIR *procfs, ਬਣਤਰ dirent *d, ਅੱਖਰ *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	ਅੱਖਰ tmp[1024], *p, *ਸਿਰਾ = ਨਿਰਾ;

	ਜੇ (bufsz < 32)
		ਵਾਪਸ -EINVAL;
	ਜੇ (!procfs_dirent_is_process(d))
		ਵਾਪਸ -EINVAL;

	snprintf(tmp, ਆਕਾਰ(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	ਜੇ (!f)
		ਵਾਪਸ -errno;

	p = fgets(tmp, ਆਕਾਰ(tmp), f);
	fclose(f);
	ਜੇ (!p)
		ਵਾਪਸ -errno;

	/* skip PID */
	ਜਦੋਂ (*p && *p != '(')
		p++;

	/* skip extra '(' */
	ਜਦੋਂ (*p && *p == '(')
		p++;

	ਸਿਰਾ = p;
	ਜਦੋਂ (*ਸਿਰਾ && *ਸਿਰਾ != ')')
		ਸਿਰਾ++;

	sz = ਸਿਰਾ - p;
	ਜੇ (sz >= bufsz)
		sz = bufsz - 1;

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(buf, p, sz);
	buf[sz] = '\0';

	ਵਾਪਸ 0;
}

ਪੂਰਨ procfs_dirent_ਮੇਲ_name(DIR *procfs, ਬਣਤਰ dirent *d, ਅਚਲ ਅੱਖਰ *name)
{
	ਅੱਖਰ buf[33];

	ਜੇ (procfs_dirent_get_name(procfs, d, buf, ਆਕਾਰ(buf)) == 0)
		ਵਾਪਸ strcmp(name, buf) == 0;

	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
ਪੂਰਨ fd_is_procfs(ਪੂਰਨ fd)
{
	ਬਣਤਰ statfs st;
	ਪੂਰਨ ret;

	ਕਰੋ {
		errno = 0;
		ret = fstatfs(fd, &st);

		ਜੇ (ret < 0) {
			ਜੇ (errno != EINTR && errno != EAGAIN)
				ਵਾਪਸ 0;
			xusleep(250000);
		}
	} ਜਦੋਂ (ret != 0);

	ਵਾਪਸ F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#ਨਹੀਂਤਾਂ
ਪੂਰਨ fd_is_procfs(ਪੂਰਨ fd __attribute__((__unused__)))
{
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਅੱਖਰ *strdup_procfs_file(pid_t pid, ਅਚਲ ਅੱਖਰ *name)
{
	ਅੱਖਰ buf[BUFSIZ];
	ਅੱਖਰ *re = ਨਿਰਾ;
	ਪੂਰਨ fd;

	snprintf(buf, ਆਕਾਰ(buf), ਰਸਤਾ_PROC "/%d/%s", (ਪੂਰਨ) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	ਜੇ (fd < 0)
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (read_procfs_file(ਨਿਰਾ, fd, buf, ਆਕਾਰ(buf)) > 0)
		re = strdup(buf);
	close(fd);
	ਵਾਪਸ re;
}

ਅੱਖਰ *pid_get_cmdname(pid_t pid)
{
	ਵਾਪਸ strdup_procfs_file(pid, "comm");
}

ਅੱਖਰ *pid_get_cmdline(pid_t pid)
{
	ਵਾਪਸ strdup_procfs_file(pid, "cmdline");
}

ਅੱਖਰ *pid_get_personality(pid_t pid)
{
	ਵਾਪਸ strdup_procfs_file(pid, "personality");
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_PROCFS

ਸਥਿਰ ਪੂਰਨ test_tasks(ਪੂਰਨ argc, ਅੱਖਰ *argv[], ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ)
{
	DIR *sub = ਨਿਰਾ;
	ਬਣਤਰ ਰਸਤਾ_cxt *pc;
	pid_t tid = 0, pid;

	ਜੇ (argc != 2)
		ਵਾਪਸ EXIT_FAILURE;

	pid = strtol(argv[1], (ਅੱਖਰ **) ਨਿਰਾ, 10);
	ਲਿਖੋ("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_ਰਸਤਾ(pid, ਅਗੇਤਰ);
	ਜੇ (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ਜਦੋਂ (procfs_process_ਅਗਲਾ_tid(pc, &sub, &tid) == 0)
		ਲਿਖੋ(" %d", tid);

	ਲਿਖੋ("\n");
	ul_unref_ਰਸਤਾ(pc);
	ਵਾਪਸ EXIT_SUCCESS;
}

ਸਥਿਰ ਪੂਰਨ test_fds(ਪੂਰਨ argc, ਅੱਖਰ *argv[], ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ)
{
	DIR *sub = ਨਿਰਾ;
	ਬਣਤਰ ਰਸਤਾ_cxt *pc;
	pid_t pid;
	ਪੂਰਨ fd = -1;

	ਜੇ (argc != 2)
		ਵਾਪਸ EXIT_FAILURE;

	pid = strtol(argv[1], (ਅੱਖਰ **) ਨਿਰਾ, 10);
	ਲਿਖੋ("PID=%d, FDs:", pid);

	pc = ul_new_procfs_ਰਸਤਾ(pid, ਅਗੇਤਰ);
	ਜੇ (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ਜਦੋਂ (procfs_process_ਅਗਲਾ_fd(pc, &sub, &fd) == 0)
		ਲਿਖੋ(" %d", fd);

	fputc('\n', stdout);
	ul_unref_ਰਸਤਾ(pc);
	ਵਾਪਸ EXIT_SUCCESS;
}

ਸਥਿਰ ਪੂਰਨ test_processes(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	DIR *dir;
	ਬਣਤਰ dirent *d;
	ਅੱਖਰ *name = ਨਿਰਾ;
	uid_t uid = (uid_t) -1;
	ਅੱਖਰ buf[128];

	ਜੇ (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	ਜੇ (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(ਰਸਤਾ_PROC);
	ਜੇ (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	ਜਦੋਂ ((d = xreaddir(dir))) {
		pid_t pid = 0;

		ਜੇ (procfs_dirent_get_pid(d, &pid) != 0)
			ਜਾਰੀ;
		ਜੇ (name && !procfs_dirent_ਮੇਲ_name(dir, d, name))
			ਜਾਰੀ;
		ਜੇ (uid != (uid_t) -1 && !procfs_dirent_ਮੇਲ_uid(dir, d, uid))
			ਜਾਰੀ;
		procfs_dirent_get_name(dir, d, buf, ਆਕਾਰ(buf));
		ਲਿਖੋ(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	ਵਾਪਸ EXIT_SUCCESS;
}

ਸਥਿਰ ਪੂਰਨ test_one_process(ਪੂਰਨ argc, ਅੱਖਰ *argv[], ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ)
{
	pid_t pid;
	ਬਣਤਰ ਰਸਤਾ_cxt *pc;
	ਅੱਖਰ buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	ਜੇ (argc != 2)
		ਵਾਪਸ EXIT_FAILURE;
	pid = strtol(argv[1], (ਅੱਖਰ **) ਨਿਰਾ, 10);

	pc = ul_new_procfs_ਰਸਤਾ(pid, ਅਗੇਤਰ);
	ਜੇ (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ਲਿਖੋ("%d\n", (ਪੂਰਨ) pid);

	procfs_process_get_uid(pc, &uid);
	ਲਿਖੋ("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, ਆਕਾਰ(buf));
	ਲਿਖੋ("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, ਆਕਾਰ(buf));
	ਲਿਖੋ("   COMM: '%s'\n", buf);

	ul_unref_ਰਸਤਾ(pc);
	ਵਾਪਸ EXIT_SUCCESS;
}

ਸਥਿਰ ਪੂਰਨ test_isprocfs(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਅਚਲ ਅੱਖਰ *name = argc > 1 ? argv[1] : "/proc";
	ਪੂਰਨ fd = open(name, O_RDONLY | O_CLOEXEC);
	ਪੂਰਨ is = 0;

	ਜੇ (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} ਨਹੀਂਤਾਂ
		err(EXIT_FAILURE, "cannot open %s", name);

	ਲਿਖੋ("%s: %s procfs\n", name, is ? "is" : "is NOT");
	ਵਾਪਸ is ? EXIT_SUCCESS : EXIT_FAILURE;
}

ਸਥਿਰ ਪੂਰਨ test_process_stat_nth(ਪੂਰਨ argc, ਅੱਖਰ *argv[], ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ)
{
	pid_t pid;
	ਬਣਤਰ ਰਸਤਾ_cxt *pc;
	uint64_t ਅੰਕ = 0;
	ਪੂਰਨ n, ret;

	ਜੇ (argc != 3)
		ਵਾਪਸ EXIT_FAILURE;
	pid = strtol(argv[1], (ਅੱਖਰ **) ਨਿਰਾ, 10);
	n = strtol(argv[2], (ਅੱਖਰ **) ਨਿਰਾ, 10);

	pc = ul_new_procfs_ਰਸਤਾ(pid, ਅਗੇਤਰ);
	ਜੇ (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &ਅੰਕ);
	ਜੇ (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	ਲਿਖੋ("%d: %dth %"PRIu64"\n", (ਪੂਰਨ) pid, n, ਅੰਕ);
	ul_unref_ਰਸਤਾ(pc);
	ਵਾਪਸ EXIT_SUCCESS;
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ = ਨਿਰਾ;

	ਜੇ (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		ਅਗੇਤਰ = argv[2];
		argc -= 2;
		argv += 2;
	}

	ਜੇ (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				ਪ੍ਰੋਗਰਾਮ_invocation_short_name);
		ਵਾਪਸ EXIT_FAILURE;
	}

	ਜੇ (strcmp(argv[1], "--tasks") == 0)
		ਵਾਪਸ test_tasks(argc - 1, argv + 1, ਅਗੇਤਰ);
	ਜੇ (strcmp(argv[1], "--fds") == 0)
		ਵਾਪਸ test_fds(argc - 1, argv + 1, ਅਗੇਤਰ);
	ਜੇ (strcmp(argv[1], "--processes") == 0)
		ਵਾਪਸ test_processes(argc - 1, argv + 1);
	ਜੇ (strcmp(argv[1], "--is-procfs") == 0)
		ਵਾਪਸ test_isprocfs(argc - 1, argv + 1);
	ਜੇ (strcmp(argv[1], "--one") == 0)
		ਵਾਪਸ test_one_process(argc - 1, argv + 1, ਅਗੇਤਰ);
	ਜੇ (strcmp(argv[1], "--stat-nth") == 0)
		ਵਾਪਸ test_process_stat_nth(argc - 1, argv + 1, ਅਗੇਤਰ);

	ਵਾਪਸ EXIT_FAILURE;
}
#ਜੇ_ਅੰਤ /* TEST_PROGRAM_PROCUTILS */
