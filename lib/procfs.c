/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2021]
 */
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <errno.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_SYS_VFS_H
# include <sys/vfs.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "pathnames.h"
#ಅನ್ತರ್ಭಾವ "procfs.h"
#ಅನ್ತರ್ಭಾವ "fileutils.h"
#ಅನ್ತರ್ಭಾವ "all-io.h"
#ಅನ್ತರ್ಭಾವ "vfs.h"
#ಅನ್ತರ್ಭಾವ "debug.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"
#ಅನ್ತರ್ಭಾವ "statfs_magic.h"

ಸ್ಥಿರ ಶೂನ್ಯ procfs_process_deinit_ಮಾರ್ಗ(ರಚನೆ ಮಾರ್ಗ_cxt *pc);

/*
 * Debug stuff (based on include/debug.h)
 */
ಸ್ಥಿರ UL_DEBUG_DEFINE_ಮುಖವಾಡ(ulprocfs);
UL_DEBUG_DEFINE_MASKNAMES(ulprocfs) = UL_DEBUG_ಖಾಲಿಗಳು_MASKNAMES;

#ವ್ಯಾಖ್ಯಾನಿಸು ULPROCFS_DEBUG_INIT	(1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು ULPROCFS_DEBUG_CXT	(1 << 2)

#ವ್ಯಾಖ್ಯಾನಿಸು DBG(m, x)		__UL_DBG(ulprocfs, ULPROCFS_DEBUG_, m, x)
#ವ್ಯಾಖ್ಯಾನಿಸು DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulprocfs, ULPROCFS_DEBUG_, m, h, x)
#ವ್ಯಾಖ್ಯಾನಿಸು ON_DBG(m, x)		__UL_DBG_CALL(ulprocfs, ULPROCFS_DEBUG_, m, x)

ಶೂನ್ಯ ul_procfs_init_debug(ಶೂನ್ಯ)
{
	ಆದರೆ (ulprocfs_debug_ಮುಖವಾಡ)
		ಹಿಂತಿರುಗಿ;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

ರಚನೆ ಮಾರ್ಗ_cxt *ul_new_procfs_ಮಾರ್ಗ(pid_t pid, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ)
{
	ರಚನೆ ಮಾರ್ಗ_cxt *pc = ul_new_ಮಾರ್ಗ(ಖಾಲಿ);

	ಆದರೆ (!pc)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಆದರೆ (ಪೂರ್ವಪ್ರತ್ಯಯ)
		ul_ಮಾರ್ಗ_ಗಣ_ಪೂರ್ವಪ್ರತ್ಯಯ(pc, ಪೂರ್ವಪ್ರತ್ಯಯ);

	ಆದರೆ (procfs_process_init_ಮಾರ್ಗ(pc, pid) != 0) {
		ul_unref_ಮಾರ್ಗ(pc);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ಹಿಂತಿರುಗಿ pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
ಪೂರ್ಣಾಂಕ procfs_process_init_ಮಾರ್ಗ(ರಚನೆ ಮಾರ್ಗ_cxt *pc, pid_t pid)
{
	ರಚನೆ procfs_process *prc;
	ಪೂರ್ಣಾಂಕ rc;
	ಅಕ್ಷರ buf[ಗಾತ್ರ(ಮಾರ್ಗ_PROC) + ಗಾತ್ರ(stringify_ಮೌಲ್ಯ(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, ಗಾತ್ರ(buf), ಮಾರ್ಗ_PROC "/%zu", (size_t) pid);
	rc = ul_ಮಾರ್ಗ_ಗಣ_dir(pc, buf);
	ಆದರೆ (rc)
		ಹಿಂತಿರುಗಿ rc;

	/* make sure path exists */
	rc = ul_ಮಾರ್ಗ_get_dirfd(pc);
	ಆದರೆ (rc < 0)
		ಹಿಂತಿರುಗಿ rc;

	/* initialize procfs specific stuff */
	prc = ul_ಮಾರ್ಗ_get_dialect(pc);
	ಆದರೆ (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ procfs_process));
		ಆದರೆ (!prc)
			ಹಿಂತಿರುಗಿ -ENOMEM;

		ul_ಮಾರ್ಗ_ಗಣ_dialect(pc, prc, procfs_process_deinit_ಮಾರ್ಗ);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ procfs_process_deinit_ಮಾರ್ಗ(ರಚನೆ ಮಾರ್ಗ_cxt *pc)
{
	ರಚನೆ procfs_process *prc;

	ಆದರೆ (!pc)
		ಹಿಂತಿರುಗಿ;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_ಮಾರ್ಗ_get_dialect(pc);
	ಆದರೆ (!prc)
		ಹಿಂತಿರುಗಿ;

	ಬಿಡುಗಡೆ(prc);
	ul_ಮಾರ್ಗ_ಗಣ_dialect(pc, ಖಾಲಿ, ಖಾಲಿ);
}

ಸ್ಥಿರ ssize_t read_procfs_file(ಸ್ಥಿರಾಂಕ ರಚನೆ ul_vfs_ops *vfs, ಪೂರ್ಣಾಂಕ fd,
			       ಅಕ್ಷರ *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	ಆದರೆ (fd < 0)
		ಹಿಂತಿರುಗಿ -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	ಆದರೆ (sz <= 0)
		ಹಿಂತಿರುಗಿ sz;

	ಗಾಗಿ (i = 0; i < (size_t) sz; i++) {
		ಆದರೆ (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	ಹಿಂತಿರುಗಿ sz;
}

ಸ್ಥಿರ ssize_t procfs_process_get_ದತ್ತಾಂಶ_for(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ *buf, size_t bufsz,
					    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname)
{
	ಪೂರ್ಣಾಂಕ fd = ul_ಮಾರ್ಗ_open(pc, O_RDONLY|O_CLOEXEC, fname);

	ಆದರೆ (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : ಖಾಲಿ, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : ಖಾಲಿ, fd);
		ಹಿಂತಿರುಗಿ sz;
	}
	ಹಿಂತಿರುಗಿ -errno;
}

ssize_t procfs_process_get_cmdline(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ *buf, size_t bufsz)
{
	ಹಿಂತಿರುಗಿ procfs_process_get_ದತ್ತಾಂಶ_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ *buf, size_t bufsz)
{
	ಹಿಂತಿರುಗಿ procfs_process_get_ದತ್ತಾಂಶ_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ *buf, size_t bufsz)
{
	ಹಿಂತಿರುಗಿ procfs_process_get_ದತ್ತಾಂಶ_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ *buf, size_t bufsz)
{
	ಹಿಂತಿರುಗಿ procfs_process_get_ದತ್ತಾಂಶ_for(pc, buf, bufsz, "syscall");
}

ಪೂರ್ಣಾಂಕ procfs_process_get_stat_nth(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಪೂರ್ಣಾಂಕ n, uint64_t *re)
{
	ssize_t rc;
	ಅಕ್ಷರ *ಕೀಲಿ = ಖಾಲಿ, *tok, *p;
	ಅಕ್ಷರ buf[BUFSIZ];
	ಪೂರ್ಣಾಂಕ i;

	ಆದರೆ (n == 2 || n == 3)		/* process name and status (strings) */
		ಹಿಂತಿರುಗಿ -EINVAL;

	rc = procfs_process_get_ದತ್ತಾಂಶ_for(pc, buf, ಗಾತ್ರ(buf), "stat");
	ಆದರೆ (rc < 0)
		ಹಿಂತಿರುಗಿ rc;

	ಗಾಗಿ (i = 0, tok = strtok_r(buf, " ", &ಕೀಲಿ); tok;
	     tok = strtok_r(ಖಾಲಿ, " ", &ಕೀಲಿ)) {

		i++;
		ಆದರೆ (i == n)
			ಹಿಂತಿರುಗಿ ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		ಆದರೆ (i == 2 && (p = strrchr(ಕೀಲಿ, ')')))
			ಕೀಲಿ = p + 2;
	}

	ಹಿಂತಿರುಗಿ -EINVAL;
}

ಪೂರ್ಣಾಂಕ procfs_process_get_uid(ರಚನೆ ಮಾರ್ಗ_cxt *pc, uid_t *uid)
{
	ರಚನೆ stat sb;
	ಪೂರ್ಣಾಂಕ rc;

	ಆದರೆ ((rc = ul_ಮಾರ್ಗ_stat(pc, &sb, 0, ಖಾಲಿ)) == 0)
		*uid = sb.st_uid;
	ಹಿಂತಿರುಗಿ rc;
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
ಪೂರ್ಣಾಂಕ procfs_process_ಮುಂದಿನ_tid(ರಚನೆ ಮಾರ್ಗ_cxt *pc, DIR **sub, pid_t *tid)
{
	ರಚನೆ dirent *d;

	ಆದರೆ (!pc || !sub || !tid)
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಆದರೆ (!*sub) {
		*sub = ul_ಮಾರ್ಗ_opendir(pc, "task");
		ಆದರೆ (!*sub)
			ಹಿಂತಿರುಗಿ -errno;
	}

	ಆಗ ((d = xreaddir(*sub))) {
		ಆದರೆ (procfs_dirent_get_pid(d, tid) == 0)
			ಹಿಂತಿರುಗಿ 0;
	}

	closedir(*sub);
	*sub = ಖಾಲಿ;
	ಹಿಂತಿರುಗಿ 1;
}

ಪೂರ್ಣಾಂಕ procfs_process_ಮುಂದಿನ_fd(ರಚನೆ ಮಾರ್ಗ_cxt *pc, DIR **sub, ಪೂರ್ಣಾಂಕ *fd)
{
	ರಚನೆ dirent *d;

	ಆದರೆ (!pc || !sub || !fd)
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಆದರೆ (!*sub) {
		*sub = ul_ಮಾರ್ಗ_opendir(pc, "fd");
		ಆದರೆ (!*sub)
			ಹಿಂತಿರುಗಿ -errno;
	}

	ಆಗ ((d = xreaddir(*sub))) {
		uint64_t ಅಂಕೆ;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _DIRENT_HAVE_D_TYPE
		ಆದರೆ (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			ಮುಂದುವರಿ;
#ಆದರೆ_ಅಂತ್ಯ
		ಆದರೆ (ul_strtou64(d->d_name, &ಅಂಕೆ, 10) < 0)
			ಮುಂದುವರಿ;
		*fd = ಅಂಕೆ;
		ಹಿಂತಿರುಗಿ 0;
	}

	closedir(*sub);
	*sub = ಖಾಲಿ;
	ಹಿಂತಿರುಗಿ 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
ಪೂರ್ಣಾಂಕ procfs_dirent_is_process(ರಚನೆ dirent *d)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _DIRENT_HAVE_D_TYPE
	ಆದರೆ (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (!ಅಂಕಿಯೇ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *d->d_name))
		ಹಿಂತಿರುಗಿ 0;

	ಹಿಂತಿರುಗಿ 1;
}

ಪೂರ್ಣಾಂಕ procfs_dirent_get_pid(ರಚನೆ dirent *d, pid_t *pid)
{
	uint64_t ಅಂಕೆ;

	ಆದರೆ (!procfs_dirent_is_process(d))
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಆದರೆ (ul_strtou64(d->d_name, &ಅಂಕೆ, 10) < 0)
		ಹಿಂತಿರುಗಿ -EINVAL;

	*pid = (pid_t) ಅಂಕೆ;
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ procfs_dirent_get_uid(DIR *procfs, ರಚನೆ dirent *d, uid_t *uid)
{
	ರಚನೆ stat st;

	ಆದರೆ (!procfs_dirent_is_process(d))
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಆದರೆ (fstatat(dirfd(procfs), d->d_name, &st, 0))
		ಹಿಂತಿರುಗಿ -EINVAL;

	*uid = st.st_uid;
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ procfs_dirent_ಹೊಂದಿಕೆ_uid(DIR *procfs, ರಚನೆ dirent *d, uid_t uid)
{
	uid_t x;

	ಆದರೆ (procfs_dirent_get_uid(procfs, d, &x) == 0)
		ಹಿಂತಿರುಗಿ x == uid;

	ಹಿಂತಿರುಗಿ 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
ಪೂರ್ಣಾಂಕ procfs_dirent_get_name(DIR *procfs, ರಚನೆ dirent *d, ಅಕ್ಷರ *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	ಅಕ್ಷರ tmp[1024], *p, *ಕೊನೆ = ಖಾಲಿ;

	ಆದರೆ (bufsz < 32)
		ಹಿಂತಿರುಗಿ -EINVAL;
	ಆದರೆ (!procfs_dirent_is_process(d))
		ಹಿಂತಿರುಗಿ -EINVAL;

	snprintf(tmp, ಗಾತ್ರ(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	ಆದರೆ (!f)
		ಹಿಂತಿರುಗಿ -errno;

	p = fgets(tmp, ಗಾತ್ರ(tmp), f);
	fclose(f);
	ಆದರೆ (!p)
		ಹಿಂತಿರುಗಿ -errno;

	/* skip PID */
	ಆಗ (*p && *p != '(')
		p++;

	/* skip extra '(' */
	ಆಗ (*p && *p == '(')
		p++;

	ಕೊನೆ = p;
	ಆಗ (*ಕೊನೆ && *ಕೊನೆ != ')')
		ಕೊನೆ++;

	sz = ಕೊನೆ - p;
	ಆದರೆ (sz >= bufsz)
		sz = bufsz - 1;

	ಸ್ಮರಣೆ_ನಕಲು(buf, p, sz);
	buf[sz] = '\0';

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ procfs_dirent_ಹೊಂದಿಕೆ_name(DIR *procfs, ರಚನೆ dirent *d, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಅಕ್ಷರ buf[33];

	ಆದರೆ (procfs_dirent_get_name(procfs, d, buf, ಗಾತ್ರ(buf)) == 0)
		ಹಿಂತಿರುಗಿ strcmp(name, buf) == 0;

	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
ಪೂರ್ಣಾಂಕ fd_is_procfs(ಪೂರ್ಣಾಂಕ fd)
{
	ರಚನೆ statfs st;
	ಪೂರ್ಣಾಂಕ ret;

	ಮಾಡು {
		errno = 0;
		ret = fstatfs(fd, &st);

		ಆದರೆ (ret < 0) {
			ಆದರೆ (errno != EINTR && errno != EAGAIN)
				ಹಿಂತಿರುಗಿ 0;
			xusleep(250000);
		}
	} ಆಗ (ret != 0);

	ಹಿಂತಿರುಗಿ F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ fd_is_procfs(ಪೂರ್ಣಾಂಕ fd __attribute__((__unused__)))
{
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಅಕ್ಷರ *strdup_procfs_file(pid_t pid, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಅಕ್ಷರ buf[BUFSIZ];
	ಅಕ್ಷರ *re = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ fd;

	snprintf(buf, ಗಾತ್ರ(buf), ಮಾರ್ಗ_PROC "/%d/%s", (ಪೂರ್ಣಾಂಕ) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	ಆದರೆ (fd < 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (read_procfs_file(ಖಾಲಿ, fd, buf, ಗಾತ್ರ(buf)) > 0)
		re = strdup(buf);
	close(fd);
	ಹಿಂತಿರುಗಿ re;
}

ಅಕ್ಷರ *pid_get_cmdname(pid_t pid)
{
	ಹಿಂತಿರುಗಿ strdup_procfs_file(pid, "comm");
}

ಅಕ್ಷರ *pid_get_cmdline(pid_t pid)
{
	ಹಿಂತಿರುಗಿ strdup_procfs_file(pid, "cmdline");
}

ಅಕ್ಷರ *pid_get_personality(pid_t pid)
{
	ಹಿಂತಿರುಗಿ strdup_procfs_file(pid, "personality");
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_PROCFS

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test_tasks(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[], ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ)
{
	DIR *sub = ಖಾಲಿ;
	ರಚನೆ ಮಾರ್ಗ_cxt *pc;
	pid_t tid = 0, pid;

	ಆದರೆ (argc != 2)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;

	pid = strtol(argv[1], (ಅಕ್ಷರ **) ಖಾಲಿ, 10);
	ಬರೆ("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_ಮಾರ್ಗ(pid, ಪೂರ್ವಪ್ರತ್ಯಯ);
	ಆದರೆ (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ಆಗ (procfs_process_ಮುಂದಿನ_tid(pc, &sub, &tid) == 0)
		ಬರೆ(" %d", tid);

	ಬರೆ("\n");
	ul_unref_ಮಾರ್ಗ(pc);
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test_fds(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[], ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ)
{
	DIR *sub = ಖಾಲಿ;
	ರಚನೆ ಮಾರ್ಗ_cxt *pc;
	pid_t pid;
	ಪೂರ್ಣಾಂಕ fd = -1;

	ಆದರೆ (argc != 2)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;

	pid = strtol(argv[1], (ಅಕ್ಷರ **) ಖಾಲಿ, 10);
	ಬರೆ("PID=%d, FDs:", pid);

	pc = ul_new_procfs_ಮಾರ್ಗ(pid, ಪೂರ್ವಪ್ರತ್ಯಯ);
	ಆದರೆ (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ಆಗ (procfs_process_ಮುಂದಿನ_fd(pc, &sub, &fd) == 0)
		ಬರೆ(" %d", fd);

	fputc('\n', stdout);
	ul_unref_ಮಾರ್ಗ(pc);
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test_processes(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	DIR *dir;
	ರಚನೆ dirent *d;
	ಅಕ್ಷರ *name = ಖಾಲಿ;
	uid_t uid = (uid_t) -1;
	ಅಕ್ಷರ buf[128];

	ಆದರೆ (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	ಆದರೆ (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(ಮಾರ್ಗ_PROC);
	ಆದರೆ (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	ಆಗ ((d = xreaddir(dir))) {
		pid_t pid = 0;

		ಆದರೆ (procfs_dirent_get_pid(d, &pid) != 0)
			ಮುಂದುವರಿ;
		ಆದರೆ (name && !procfs_dirent_ಹೊಂದಿಕೆ_name(dir, d, name))
			ಮುಂದುವರಿ;
		ಆದರೆ (uid != (uid_t) -1 && !procfs_dirent_ಹೊಂದಿಕೆ_uid(dir, d, uid))
			ಮುಂದುವರಿ;
		procfs_dirent_get_name(dir, d, buf, ಗಾತ್ರ(buf));
		ಬರೆ(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test_one_process(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[], ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ)
{
	pid_t pid;
	ರಚನೆ ಮಾರ್ಗ_cxt *pc;
	ಅಕ್ಷರ buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	ಆದರೆ (argc != 2)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	pid = strtol(argv[1], (ಅಕ್ಷರ **) ಖಾಲಿ, 10);

	pc = ul_new_procfs_ಮಾರ್ಗ(pid, ಪೂರ್ವಪ್ರತ್ಯಯ);
	ಆದರೆ (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ಬರೆ("%d\n", (ಪೂರ್ಣಾಂಕ) pid);

	procfs_process_get_uid(pc, &uid);
	ಬರೆ("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, ಗಾತ್ರ(buf));
	ಬರೆ("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, ಗಾತ್ರ(buf));
	ಬರೆ("   COMM: '%s'\n", buf);

	ul_unref_ಮಾರ್ಗ(pc);
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test_isprocfs(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = argc > 1 ? argv[1] : "/proc";
	ಪೂರ್ಣಾಂಕ fd = open(name, O_RDONLY | O_CLOEXEC);
	ಪೂರ್ಣಾಂಕ is = 0;

	ಆದರೆ (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} ಇಲ್ಲದಿದ್ದರೆ
		err(EXIT_FAILURE, "cannot open %s", name);

	ಬರೆ("%s: %s procfs\n", name, is ? "is" : "is NOT");
	ಹಿಂತಿರುಗಿ is ? EXIT_SUCCESS : EXIT_FAILURE;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test_process_stat_nth(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[], ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ)
{
	pid_t pid;
	ರಚನೆ ಮಾರ್ಗ_cxt *pc;
	uint64_t ಅಂಕೆ = 0;
	ಪೂರ್ಣಾಂಕ n, ret;

	ಆದರೆ (argc != 3)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	pid = strtol(argv[1], (ಅಕ್ಷರ **) ಖಾಲಿ, 10);
	n = strtol(argv[2], (ಅಕ್ಷರ **) ಖಾಲಿ, 10);

	pc = ul_new_procfs_ಮಾರ್ಗ(pid, ಪೂರ್ವಪ್ರತ್ಯಯ);
	ಆದರೆ (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &ಅಂಕೆ);
	ಆದರೆ (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	ಬರೆ("%d: %dth %"PRIu64"\n", (ಪೂರ್ಣಾಂಕ) pid, n, ಅಂಕೆ);
	ul_unref_ಮಾರ್ಗ(pc);
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ = ಖಾಲಿ;

	ಆದರೆ (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		ಪೂರ್ವಪ್ರತ್ಯಯ = argv[2];
		argc -= 2;
		argv += 2;
	}

	ಆದರೆ (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				ಕಾರ್ಯಕ್ರಮ_invocation_short_name);
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	}

	ಆದರೆ (strcmp(argv[1], "--tasks") == 0)
		ಹಿಂತಿರುಗಿ test_tasks(argc - 1, argv + 1, ಪೂರ್ವಪ್ರತ್ಯಯ);
	ಆದರೆ (strcmp(argv[1], "--fds") == 0)
		ಹಿಂತಿರುಗಿ test_fds(argc - 1, argv + 1, ಪೂರ್ವಪ್ರತ್ಯಯ);
	ಆದರೆ (strcmp(argv[1], "--processes") == 0)
		ಹಿಂತಿರುಗಿ test_processes(argc - 1, argv + 1);
	ಆದರೆ (strcmp(argv[1], "--is-procfs") == 0)
		ಹಿಂತಿರುಗಿ test_isprocfs(argc - 1, argv + 1);
	ಆದರೆ (strcmp(argv[1], "--one") == 0)
		ಹಿಂತಿರುಗಿ test_one_process(argc - 1, argv + 1, ಪೂರ್ವಪ್ರತ್ಯಯ);
	ಆದರೆ (strcmp(argv[1], "--stat-nth") == 0)
		ಹಿಂತಿರುಗಿ test_process_stat_nth(argc - 1, argv + 1, ಪೂರ್ವಪ್ರತ್ಯಯ);

	ಹಿಂತಿರುಗಿ EXIT_FAILURE;
}
#ಆದರೆ_ಅಂತ್ಯ /* TEST_PROGRAM_PROCUTILS */
