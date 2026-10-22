/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2021]
 */
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>

#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "pathnames.h"
#ଅନ୍ତର୍ଭାଵ "procfs.h"
#ଅନ୍ତର୍ଭାଵ "fileutils.h"
#ଅନ୍ତର୍ଭାଵ "all-io.h"
#ଅନ୍ତର୍ଭାଵ "vfs.h"
#ଅନ୍ତର୍ଭାଵ "debug.h"
#ଅନ୍ତର୍ଭାଵ "strutils.h"
#ଅନ୍ତର୍ଭାଵ "statfs_magic.h"

ସ୍ଥିର ଶୂନ୍ୟ procfs_process_deinit_path(ସଂରଚନା path_cxt *pc);

/*
 * Debug stuff (based on include/debug.h)
 */
ସ୍ଥିର UL_DEBUG_DEFINE_MASK(ulprocfs);
UL_DEBUG_DEFINE_MASKNAMES(ulprocfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPROCFS_DEBUG_INIT	(1 << 1)
#define ULPROCFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulprocfs, ULPROCFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulprocfs, ULPROCFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulprocfs, ULPROCFS_DEBUG_, m, x)

ଶୂନ୍ୟ ul_procfs_init_debug(ଶୂନ୍ୟ)
{
	ଯଦି (ulprocfs_debug_mask)
		ଫେରନ୍ତୁ;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

ସଂରଚନା path_cxt *ul_new_procfs_path(pid_t pid, ଅଚଳ ଅକ୍ଷର *prefix)
{
	ସଂରଚନା path_cxt *pc = ul_new_path(ଖାଲି);

	ଯଦି (!pc)
		ଫେରନ୍ତୁ ଖାଲି;
	ଯଦି (prefix)
		ul_path_set_prefix(pc, prefix);

	ଯଦି (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ଫେରନ୍ତୁ pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା procfs_process_init_path(ସଂରଚନା path_cxt *pc, pid_t pid)
{
	ସଂରଚନା procfs_process *prc;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	ଅକ୍ଷର buf[ଆକାର(_PATH_PROC) + ଆକାର(stringify_ମୂଲ୍ୟ(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, ଆକାର(buf), _PATH_PROC "/%zu", (size_t) pid);
	rc = ul_path_set_dir(pc, buf);
	ଯଦି (rc)
		ଫେରନ୍ତୁ rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	ଯଦି (rc < 0)
		ଫେରନ୍ତୁ rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	ଯଦି (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = calloc(1, ଆକାର(ସଂରଚନା procfs_process));
		ଯଦି (!prc)
			ଫେରନ୍ତୁ -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ procfs_process_deinit_path(ସଂରଚନା path_cxt *pc)
{
	ସଂରଚନା procfs_process *prc;

	ଯଦି (!pc)
		ଫେରନ୍ତୁ;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	ଯଦି (!prc)
		ଫେରନ୍ତୁ;

	ମୁକ୍ତ(prc);
	ul_path_set_dialect(pc, ଖାଲି, ଖାଲି);
}

ସ୍ଥିର ssize_t read_procfs_file(ଅଚଳ ସଂରଚନା ul_vfs_ops *vfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd,
			       ଅକ୍ଷର *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	ଯଦି (fd < 0)
		ଫେରନ୍ତୁ -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	ଯଦି (sz <= 0)
		ଫେରନ୍ତୁ sz;

	ପାଇଁ (i = 0; i < (size_t) sz; i++) {
		ଯଦି (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	ଫେରନ୍ତୁ sz;
}

ସ୍ଥିର ssize_t procfs_process_get_data_for(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର *buf, size_t bufsz,
					    ଅଚଳ ଅକ୍ଷର *fname)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	ଯଦି (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : ଖାଲି, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : ଖାଲି, fd);
		ଫେରନ୍ତୁ sz;
	}
	ଫେରନ୍ତୁ -errno;
}

ssize_t procfs_process_get_cmdline(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର *buf, size_t bufsz)
{
	ଫେରନ୍ତୁ procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର *buf, size_t bufsz)
{
	ଫେରନ୍ତୁ procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର *buf, size_t bufsz)
{
	ଫେରନ୍ତୁ procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର *buf, size_t bufsz)
{
	ଫେରନ୍ତୁ procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା procfs_process_get_stat_nth(ସଂରଚନା path_cxt *pc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, uint64_t *re)
{
	ssize_t rc;
	ଅକ୍ଷର *ଚାବି = ଖାଲି, *tok, *p;
	ଅକ୍ଷର buf[BUFSIZ];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ଯଦି (n == 2 || n == 3)		/* process name and status (strings) */
		ଫେରନ୍ତୁ -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, ଆକାର(buf), "stat");
	ଯଦି (rc < 0)
		ଫେରନ୍ତୁ rc;

	ପାଇଁ (i = 0, tok = strtok_r(buf, " ", &ଚାବି); tok;
	     tok = strtok_r(ଖାଲି, " ", &ଚାବି)) {

		i++;
		ଯଦି (i == n)
			ଫେରନ୍ତୁ ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		ଯଦି (i == 2 && (p = strrchr(ଚାବି, ')')))
			ଚାବି = p + 2;
	}

	ଫେରନ୍ତୁ -EINVAL;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା procfs_process_get_uid(ସଂରଚନା path_cxt *pc, uid_t *uid)
{
	ସଂରଚନା stat sb;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	ଯଦି ((rc = ul_path_stat(pc, &sb, 0, ଖାଲି)) == 0)
		*uid = sb.st_uid;
	ଫେରନ୍ତୁ rc;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା procfs_process_next_tid(ସଂରଚନା path_cxt *pc, DIR **sub, pid_t *tid)
{
	ସଂରଚନା dirent *d;

	ଯଦି (!pc || !sub || !tid)
		ଫେରନ୍ତୁ -EINVAL;

	ଯଦି (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		ଯଦି (!*sub)
			ଫେରନ୍ତୁ -errno;
	}

	ଯେତେବେଳେ ((d = xreaddir(*sub))) {
		ଯଦି (procfs_dirent_get_pid(d, tid) == 0)
			ଫେରନ୍ତୁ 0;
	}

	closedir(*sub);
	*sub = ଖାଲି;
	ଫେରନ୍ତୁ 1;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା procfs_process_next_fd(ସଂରଚନା path_cxt *pc, DIR **sub, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *fd)
{
	ସଂରଚନା dirent *d;

	ଯଦି (!pc || !sub || !fd)
		ଫେରନ୍ତୁ -EINVAL;

	ଯଦି (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		ଯଦି (!*sub)
			ଫେରନ୍ତୁ -errno;
	}

	ଯେତେବେଳେ ((d = xreaddir(*sub))) {
		uint64_t ଅଙ୍କ;
#ifdef _DIRENT_HAVE_D_TYPE
		ଯଦି (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			ଜାରି;
#endif
		ଯଦି (ul_strtou64(d->d_name, &ଅଙ୍କ, 10) < 0)
			ଜାରି;
		*fd = ଅଙ୍କ;
		ଫେରନ୍ତୁ 0;
	}

	closedir(*sub);
	*sub = ଖାଲି;
	ଫେରନ୍ତୁ 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା procfs_dirent_is_process(ସଂରଚନା dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	ଯଦି (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		ଫେରନ୍ତୁ 0;
#endif
	ଯଦି (!isdigit((unsigned ଅକ୍ଷର) *d->d_name))
		ଫେରନ୍ତୁ 0;

	ଫେରନ୍ତୁ 1;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା procfs_dirent_get_pid(ସଂରଚନା dirent *d, pid_t *pid)
{
	uint64_t ଅଙ୍କ;

	ଯଦି (!procfs_dirent_is_process(d))
		ଫେରନ୍ତୁ -EINVAL;

	ଯଦି (ul_strtou64(d->d_name, &ଅଙ୍କ, 10) < 0)
		ଫେରନ୍ତୁ -EINVAL;

	*pid = (pid_t) ଅଙ୍କ;
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା procfs_dirent_get_uid(DIR *procfs, ସଂରଚନା dirent *d, uid_t *uid)
{
	ସଂରଚନା stat st;

	ଯଦି (!procfs_dirent_is_process(d))
		ଫେରନ୍ତୁ -EINVAL;

	ଯଦି (fstatat(dirfd(procfs), d->d_name, &st, 0))
		ଫେରନ୍ତୁ -EINVAL;

	*uid = st.st_uid;
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା procfs_dirent_match_uid(DIR *procfs, ସଂରଚନା dirent *d, uid_t uid)
{
	uid_t x;

	ଯଦି (procfs_dirent_get_uid(procfs, d, &x) == 0)
		ଫେରନ୍ତୁ x == uid;

	ଫେରନ୍ତୁ 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା procfs_dirent_get_name(DIR *procfs, ସଂରଚନା dirent *d, ଅକ୍ଷର *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	ଅକ୍ଷର tmp[1024], *p, *end = ଖାଲି;

	ଯଦି (bufsz < 32)
		ଫେରନ୍ତୁ -EINVAL;
	ଯଦି (!procfs_dirent_is_process(d))
		ଫେରନ୍ତୁ -EINVAL;

	snprintf(tmp, ଆକାର(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	ଯଦି (!f)
		ଫେରନ୍ତୁ -errno;

	p = fgets(tmp, ଆକାର(tmp), f);
	fclose(f);
	ଯଦି (!p)
		ଫେରନ୍ତୁ -errno;

	/* skip PID */
	ଯେତେବେଳେ (*p && *p != '(')
		p++;

	/* skip extra '(' */
	ଯେତେବେଳେ (*p && *p == '(')
		p++;

	end = p;
	ଯେତେବେଳେ (*end && *end != ')')
		end++;

	sz = end - p;
	ଯଦି (sz >= bufsz)
		sz = bufsz - 1;

	memcpy(buf, p, sz);
	buf[sz] = '\0';

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା procfs_dirent_match_name(DIR *procfs, ସଂରଚନା dirent *d, ଅଚଳ ଅକ୍ଷର *name)
{
	ଅକ୍ଷର buf[33];

	ଯଦି (procfs_dirent_get_name(procfs, d, buf, ଆକାର(buf)) == 0)
		ଫେରନ୍ତୁ strcmp(name, buf) == 0;

	ଫେରନ୍ତୁ 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd_is_procfs(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd)
{
	ସଂରଚନା statfs st;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	କର {
		errno = 0;
		ret = fstatfs(fd, &st);

		ଯଦି (ret < 0) {
			ଯଦି (errno != EINTR && errno != EAGAIN)
				ଫେରନ୍ତୁ 0;
			xusleep(250000);
		}
	} ଯେତେବେଳେ (ret != 0);

	ଫେରନ୍ତୁ F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd_is_procfs(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd __attribute__((__unused__)))
{
	ଫେରନ୍ତୁ 0;
}
#endif

ସ୍ଥିର ଅକ୍ଷର *strdup_procfs_file(pid_t pid, ଅଚଳ ଅକ୍ଷର *name)
{
	ଅକ୍ଷର buf[BUFSIZ];
	ଅକ୍ଷର *re = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;

	snprintf(buf, ଆକାର(buf), _PATH_PROC "/%d/%s", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	ଯଦି (fd < 0)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (read_procfs_file(ଖାଲି, fd, buf, ଆକାର(buf)) > 0)
		re = strdup(buf);
	close(fd);
	ଫେରନ୍ତୁ re;
}

ଅକ୍ଷର *pid_get_cmdname(pid_t pid)
{
	ଫେରନ୍ତୁ strdup_procfs_file(pid, "comm");
}

ଅକ୍ଷର *pid_get_cmdline(pid_t pid)
{
	ଫେରନ୍ତୁ strdup_procfs_file(pid, "cmdline");
}

ଅକ୍ଷର *pid_get_personality(pid_t pid)
{
	ଫେରନ୍ତୁ strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_tasks(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[], ଅଚଳ ଅକ୍ଷର *prefix)
{
	DIR *sub = ଖାଲି;
	ସଂରଚନା path_cxt *pc;
	pid_t tid = 0, pid;

	ଯଦି (argc != 2)
		ଫେରନ୍ତୁ EXIT_FAILURE;

	pid = strtol(argv[1], (ଅକ୍ଷର **) ଖାଲି, 10);
	printf("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	ଯଦି (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ଯେତେବେଳେ (procfs_process_next_tid(pc, &sub, &tid) == 0)
		printf(" %d", tid);

	printf("\n");
	ul_unref_path(pc);
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_fds(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[], ଅଚଳ ଅକ୍ଷର *prefix)
{
	DIR *sub = ଖାଲି;
	ସଂରଚନା path_cxt *pc;
	pid_t pid;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = -1;

	ଯଦି (argc != 2)
		ଫେରନ୍ତୁ EXIT_FAILURE;

	pid = strtol(argv[1], (ଅକ୍ଷର **) ଖାଲି, 10);
	printf("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	ଯଦି (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ଯେତେବେଳେ (procfs_process_next_fd(pc, &sub, &fd) == 0)
		printf(" %d", fd);

	fputc('\n', stdout);
	ul_unref_path(pc);
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_processes(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	DIR *dir;
	ସଂରଚନା dirent *d;
	ଅକ୍ଷର *name = ଖାଲି;
	uid_t uid = (uid_t) -1;
	ଅକ୍ଷର buf[128];

	ଯଦି (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	ଯଦି (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(_PATH_PROC);
	ଯଦି (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	ଯେତେବେଳେ ((d = xreaddir(dir))) {
		pid_t pid = 0;

		ଯଦି (procfs_dirent_get_pid(d, &pid) != 0)
			ଜାରି;
		ଯଦି (name && !procfs_dirent_match_name(dir, d, name))
			ଜାରି;
		ଯଦି (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			ଜାରି;
		procfs_dirent_get_name(dir, d, buf, ଆକାର(buf));
		printf(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_one_process(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[], ଅଚଳ ଅକ୍ଷର *prefix)
{
	pid_t pid;
	ସଂରଚନା path_cxt *pc;
	ଅକ୍ଷର buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	ଯଦି (argc != 2)
		ଫେରନ୍ତୁ EXIT_FAILURE;
	pid = strtol(argv[1], (ଅକ୍ଷର **) ଖାଲି, 10);

	pc = ul_new_procfs_path(pid, prefix);
	ଯଦି (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	printf("%d\n", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) pid);

	procfs_process_get_uid(pc, &uid);
	printf("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, ଆକାର(buf));
	printf("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, ଆକାର(buf));
	printf("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_isprocfs(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ଅଚଳ ଅକ୍ଷର *name = argc > 1 ? argv[1] : "/proc";
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = open(name, O_RDONLY | O_CLOEXEC);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା is = 0;

	ଯଦି (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} ଅନ୍ୟଥା
		err(EXIT_FAILURE, "cannot open %s", name);

	printf("%s: %s procfs\n", name, is ? "is" : "is NOT");
	ଫେରନ୍ତୁ is ? EXIT_SUCCESS : EXIT_FAILURE;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_process_stat_nth(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[], ଅଚଳ ଅକ୍ଷର *prefix)
{
	pid_t pid;
	ସଂରଚନା path_cxt *pc;
	uint64_t ଅଙ୍କ = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, ret;

	ଯଦି (argc != 3)
		ଫେରନ୍ତୁ EXIT_FAILURE;
	pid = strtol(argv[1], (ଅକ୍ଷର **) ଖାଲି, 10);
	n = strtol(argv[2], (ଅକ୍ଷର **) ଖାଲି, 10);

	pc = ul_new_procfs_path(pid, prefix);
	ଯଦି (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &ଅଙ୍କ);
	ଯଦି (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	printf("%d: %dth %"PRIu64"\n", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) pid, n, ଅଙ୍କ);
	ul_unref_path(pc);
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ଅଚଳ ଅକ୍ଷର *prefix = ଖାଲି;

	ଯଦି (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	ଯଦି (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		ଫେରନ୍ତୁ EXIT_FAILURE;
	}

	ଯଦି (strcmp(argv[1], "--tasks") == 0)
		ଫେରନ୍ତୁ test_tasks(argc - 1, argv + 1, prefix);
	ଯଦି (strcmp(argv[1], "--fds") == 0)
		ଫେରନ୍ତୁ test_fds(argc - 1, argv + 1, prefix);
	ଯଦି (strcmp(argv[1], "--processes") == 0)
		ଫେରନ୍ତୁ test_processes(argc - 1, argv + 1);
	ଯଦି (strcmp(argv[1], "--is-procfs") == 0)
		ଫେରନ୍ତୁ test_isprocfs(argc - 1, argv + 1);
	ଯଦି (strcmp(argv[1], "--one") == 0)
		ଫେରନ୍ତୁ test_one_process(argc - 1, argv + 1, prefix);
	ଯଦି (strcmp(argv[1], "--stat-nth") == 0)
		ଫେରନ୍ତୁ test_process_stat_nth(argc - 1, argv + 1, prefix);

	ଫେରନ୍ତୁ EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_PROCUTILS */
