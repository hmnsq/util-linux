/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2021]
 */
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif

#include "c.h"
#include "pathnames.h"
#include "procfs.h"
#include "fileutils.h"
#include "all-io.h"
#include "vfs.h"
#include "debug.h"
#include "strutils.h"
#include "statfs_magic.h"

static void procfs_process_deinit_path(sa_nracanaa path_cxt *pc);

/*
 * Debug stuff (based on include/debug.h)
 */
static UL_DEBUG_DEFINE_MASK(ulprocfs);
UL_DEBUG_DEFINE_MASKNAMES(ulprocfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPROCFS_DEBUG_INIT	(1 << 1)
#define ULPROCFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulprocfs, ULPROCFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulprocfs, ULPROCFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulprocfs, ULPROCFS_DEBUG_, m, x)

void ul_procfs_init_debug(void)
{
	ถ้า (ulprocfs_debug_mask)
		คืน;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

sa_nracanaa path_cxt *ul_new_procfs_path(pid_t pid, const var_na *prefix)
{
	sa_nracanaa path_cxt *pc = ul_new_path(NULL);

	ถ้า (!pc)
		คืน NULL;
	ถ้า (prefix)
		ul_path_set_prefix(pc, prefix);

	ถ้า (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		คืน NULL;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	คืน pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
จำนวนเต็ม procfs_process_init_path(sa_nracanaa path_cxt *pc, pid_t pid)
{
	sa_nracanaa procfs_process *prc;
	จำนวนเต็ม rc;
	var_na buf[parimaa_na(_PATH_PROC) + parimaa_na(stringify_value(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, parimaa_na(buf), _PATH_PROC "/%zu", (size_t) pid);
	rc = ul_path_set_dir(pc, buf);
	ถ้า (rc)
		คืน rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	ถ้า (rc < 0)
		คืน rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	ถ้า (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = calloc(1, parimaa_na(sa_nracanaa procfs_process));
		ถ้า (!prc)
			คืน -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	คืน 0;
}

static void procfs_process_deinit_path(sa_nracanaa path_cxt *pc)
{
	sa_nracanaa procfs_process *prc;

	ถ้า (!pc)
		คืน;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	ถ้า (!prc)
		คืน;

	free(prc);
	ul_path_set_dialect(pc, NULL, NULL);
}

static ssize_t read_procfs_file(const sa_nracanaa ul_vfs_ops *vfs, จำนวนเต็ม fd,
			       var_na *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	ถ้า (fd < 0)
		คืน -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	ถ้า (sz <= 0)
		คืน sz;

	สำหรับ (i = 0; i < (size_t) sz; i++) {
		ถ้า (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	คืน sz;
}

static ssize_t procfs_process_get_data_for(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz,
					    const var_na *fname)
{
	จำนวนเต็ม fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	ถ้า (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : NULL, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : NULL, fd);
		คืน sz;
	}
	คืน -errno;
}

ssize_t procfs_process_get_cmdline(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	คืน procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	คืน procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	คืน procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	คืน procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

จำนวนเต็ม procfs_process_get_stat_nth(sa_nracanaa path_cxt *pc, จำนวนเต็ม n, uint64_t *re)
{
	ssize_t rc;
	var_na *key = NULL, *tok, *p;
	var_na buf[BUFSIZ];
	จำนวนเต็ม i;

	ถ้า (n == 2 || n == 3)		/* process name and status (strings) */
		คืน -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, parimaa_na(buf), "stat");
	ถ้า (rc < 0)
		คืน rc;

	สำหรับ (i = 0, tok = strtok_r(buf, " ", &key); tok;
	     tok = strtok_r(NULL, " ", &key)) {

		i++;
		ถ้า (i == n)
			คืน ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		ถ้า (i == 2 && (p = strrchr(key, ')')))
			key = p + 2;
	}

	คืน -EINVAL;
}

จำนวนเต็ม procfs_process_get_uid(sa_nracanaa path_cxt *pc, uid_t *uid)
{
	sa_nracanaa stat sb;
	จำนวนเต็ม rc;

	ถ้า ((rc = ul_path_stat(pc, &sb, 0, NULL)) == 0)
		*uid = sb.st_uid;
	คืน rc;
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
จำนวนเต็ม procfs_process_next_tid(sa_nracanaa path_cxt *pc, DIR **sub, pid_t *tid)
{
	sa_nracanaa dirent *d;

	ถ้า (!pc || !sub || !tid)
		คืน -EINVAL;

	ถ้า (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		ถ้า (!*sub)
			คืน -errno;
	}

	ขณะ ((d = xreaddir(*sub))) {
		ถ้า (procfs_dirent_get_pid(d, tid) == 0)
			คืน 0;
	}

	closedir(*sub);
	*sub = NULL;
	คืน 1;
}

จำนวนเต็ม procfs_process_next_fd(sa_nracanaa path_cxt *pc, DIR **sub, จำนวนเต็ม *fd)
{
	sa_nracanaa dirent *d;

	ถ้า (!pc || !sub || !fd)
		คืน -EINVAL;

	ถ้า (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		ถ้า (!*sub)
			คืน -errno;
	}

	ขณะ ((d = xreaddir(*sub))) {
		uint64_t num;
#ifdef _DIRENT_HAVE_D_TYPE
		ถ้า (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			continue;
#endif
		ถ้า (ul_strtou64(d->d_name, &num, 10) < 0)
			continue;
		*fd = num;
		คืน 0;
	}

	closedir(*sub);
	*sub = NULL;
	คืน 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
จำนวนเต็ม procfs_dirent_is_process(sa_nracanaa dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	ถ้า (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		คืน 0;
#endif
	ถ้า (!isdigit((unsigned var_na) *d->d_name))
		คืน 0;

	คืน 1;
}

จำนวนเต็ม procfs_dirent_get_pid(sa_nracanaa dirent *d, pid_t *pid)
{
	uint64_t num;

	ถ้า (!procfs_dirent_is_process(d))
		คืน -EINVAL;

	ถ้า (ul_strtou64(d->d_name, &num, 10) < 0)
		คืน -EINVAL;

	*pid = (pid_t) num;
	คืน 0;
}

จำนวนเต็ม procfs_dirent_get_uid(DIR *procfs, sa_nracanaa dirent *d, uid_t *uid)
{
	sa_nracanaa stat st;

	ถ้า (!procfs_dirent_is_process(d))
		คืน -EINVAL;

	ถ้า (fstatat(dirfd(procfs), d->d_name, &st, 0))
		คืน -EINVAL;

	*uid = st.st_uid;
	คืน 0;
}

จำนวนเต็ม procfs_dirent_match_uid(DIR *procfs, sa_nracanaa dirent *d, uid_t uid)
{
	uid_t x;

	ถ้า (procfs_dirent_get_uid(procfs, d, &x) == 0)
		คืน x == uid;

	คืน 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
จำนวนเต็ม procfs_dirent_get_name(DIR *procfs, sa_nracanaa dirent *d, var_na *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	var_na tmp[1024], *p, *end = NULL;

	ถ้า (bufsz < 32)
		คืน -EINVAL;
	ถ้า (!procfs_dirent_is_process(d))
		คืน -EINVAL;

	snprintf(tmp, parimaa_na(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	ถ้า (!f)
		คืน -errno;

	p = fgets(tmp, parimaa_na(tmp), f);
	fclose(f);
	ถ้า (!p)
		คืน -errno;

	/* skip PID */
	ขณะ (*p && *p != '(')
		p++;

	/* skip extra '(' */
	ขณะ (*p && *p == '(')
		p++;

	end = p;
	ขณะ (*end && *end != ')')
		end++;

	sz = end - p;
	ถ้า (sz >= bufsz)
		sz = bufsz - 1;

	memcpy(buf, p, sz);
	buf[sz] = '\0';

	คืน 0;
}

จำนวนเต็ม procfs_dirent_match_name(DIR *procfs, sa_nracanaa dirent *d, const var_na *name)
{
	var_na buf[33];

	ถ้า (procfs_dirent_get_name(procfs, d, buf, parimaa_na(buf)) == 0)
		คืน strcmp(name, buf) == 0;

	คืน 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
จำนวนเต็ม fd_is_procfs(จำนวนเต็ม fd)
{
	sa_nracanaa statfs st;
	จำนวนเต็ม ret;

	do {
		errno = 0;
		ret = fstatfs(fd, &st);

		ถ้า (ret < 0) {
			ถ้า (errno != EINTR && errno != EAGAIN)
				คืน 0;
			xusleep(250000);
		}
	} ขณะ (ret != 0);

	คืน F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#มิฉะนั้น
จำนวนเต็ม fd_is_procfs(จำนวนเต็ม fd __attribute__((__unused__)))
{
	คืน 0;
}
#endif

static var_na *strdup_procfs_file(pid_t pid, const var_na *name)
{
	var_na buf[BUFSIZ];
	var_na *re = NULL;
	จำนวนเต็ม fd;

	snprintf(buf, parimaa_na(buf), _PATH_PROC "/%d/%s", (จำนวนเต็ม) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	ถ้า (fd < 0)
		คืน NULL;

	ถ้า (read_procfs_file(NULL, fd, buf, parimaa_na(buf)) > 0)
		re = strdup(buf);
	close(fd);
	คืน re;
}

var_na *pid_get_cmdname(pid_t pid)
{
	คืน strdup_procfs_file(pid, "comm");
}

var_na *pid_get_cmdline(pid_t pid)
{
	คืน strdup_procfs_file(pid, "cmdline");
}

var_na *pid_get_personality(pid_t pid)
{
	คืน strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

static จำนวนเต็ม test_tasks(จำนวนเต็ม argc, var_na *argv[], const var_na *prefix)
{
	DIR *sub = NULL;
	sa_nracanaa path_cxt *pc;
	pid_t tid = 0, pid;

	ถ้า (argc != 2)
		คืน EXIT_FAILURE;

	pid = strtol(argv[1], (var_na **) NULL, 10);
	printf("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	ถ้า (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ขณะ (procfs_process_next_tid(pc, &sub, &tid) == 0)
		printf(" %d", tid);

	printf("\n");
	ul_unref_path(pc);
	คืน EXIT_SUCCESS;
}

static จำนวนเต็ม test_fds(จำนวนเต็ม argc, var_na *argv[], const var_na *prefix)
{
	DIR *sub = NULL;
	sa_nracanaa path_cxt *pc;
	pid_t pid;
	จำนวนเต็ม fd = -1;

	ถ้า (argc != 2)
		คืน EXIT_FAILURE;

	pid = strtol(argv[1], (var_na **) NULL, 10);
	printf("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	ถ้า (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ขณะ (procfs_process_next_fd(pc, &sub, &fd) == 0)
		printf(" %d", fd);

	fputc('\n', stdout);
	ul_unref_path(pc);
	คืน EXIT_SUCCESS;
}

static จำนวนเต็ม test_processes(จำนวนเต็ม argc, var_na *argv[])
{
	DIR *dir;
	sa_nracanaa dirent *d;
	var_na *name = NULL;
	uid_t uid = (uid_t) -1;
	var_na buf[128];

	ถ้า (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	ถ้า (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(_PATH_PROC);
	ถ้า (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	ขณะ ((d = xreaddir(dir))) {
		pid_t pid = 0;

		ถ้า (procfs_dirent_get_pid(d, &pid) != 0)
			continue;
		ถ้า (name && !procfs_dirent_match_name(dir, d, name))
			continue;
		ถ้า (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			continue;
		procfs_dirent_get_name(dir, d, buf, parimaa_na(buf));
		printf(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	คืน EXIT_SUCCESS;
}

static จำนวนเต็ม test_one_process(จำนวนเต็ม argc, var_na *argv[], const var_na *prefix)
{
	pid_t pid;
	sa_nracanaa path_cxt *pc;
	var_na buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	ถ้า (argc != 2)
		คืน EXIT_FAILURE;
	pid = strtol(argv[1], (var_na **) NULL, 10);

	pc = ul_new_procfs_path(pid, prefix);
	ถ้า (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	printf("%d\n", (จำนวนเต็ม) pid);

	procfs_process_get_uid(pc, &uid);
	printf("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, parimaa_na(buf));
	printf("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, parimaa_na(buf));
	printf("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	คืน EXIT_SUCCESS;
}

static จำนวนเต็ม test_isprocfs(จำนวนเต็ม argc, var_na *argv[])
{
	const var_na *name = argc > 1 ? argv[1] : "/proc";
	จำนวนเต็ม fd = open(name, O_RDONLY | O_CLOEXEC);
	จำนวนเต็ม is = 0;

	ถ้า (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} มิฉะนั้น
		err(EXIT_FAILURE, "cannot open %s", name);

	printf("%s: %s procfs\n", name, is ? "is" : "is NOT");
	คืน is ? EXIT_SUCCESS : EXIT_FAILURE;
}

static จำนวนเต็ม test_process_stat_nth(จำนวนเต็ม argc, var_na *argv[], const var_na *prefix)
{
	pid_t pid;
	sa_nracanaa path_cxt *pc;
	uint64_t num = 0;
	จำนวนเต็ม n, ret;

	ถ้า (argc != 3)
		คืน EXIT_FAILURE;
	pid = strtol(argv[1], (var_na **) NULL, 10);
	n = strtol(argv[2], (var_na **) NULL, 10);

	pc = ul_new_procfs_path(pid, prefix);
	ถ้า (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &num);
	ถ้า (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	printf("%d: %dth %"PRIu64"\n", (จำนวนเต็ม) pid, n, num);
	ul_unref_path(pc);
	คืน EXIT_SUCCESS;
}

จำนวนเต็ม หลัก(จำนวนเต็ม argc, var_na *argv[])
{
	const var_na *prefix = NULL;

	ถ้า (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	ถ้า (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		คืน EXIT_FAILURE;
	}

	ถ้า (strcmp(argv[1], "--tasks") == 0)
		คืน test_tasks(argc - 1, argv + 1, prefix);
	ถ้า (strcmp(argv[1], "--fds") == 0)
		คืน test_fds(argc - 1, argv + 1, prefix);
	ถ้า (strcmp(argv[1], "--processes") == 0)
		คืน test_processes(argc - 1, argv + 1);
	ถ้า (strcmp(argv[1], "--is-procfs") == 0)
		คืน test_isprocfs(argc - 1, argv + 1);
	ถ้า (strcmp(argv[1], "--one") == 0)
		คืน test_one_process(argc - 1, argv + 1, prefix);
	ถ้า (strcmp(argv[1], "--stat-nth") == 0)
		คืน test_process_stat_nth(argc - 1, argv + 1, prefix);

	คืน EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_PROCUTILS */
