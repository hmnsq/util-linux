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
	ຖ້າ (ulprocfs_debug_mask)
		ສົ່ງຄືນ;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

sa_nracanaa path_cxt *ul_new_procfs_path(pid_t pid, const var_na *prefix)
{
	sa_nracanaa path_cxt *pc = ul_new_path(NULL);

	ຖ້າ (!pc)
		ສົ່ງຄືນ NULL;
	ຖ້າ (prefix)
		ul_path_set_prefix(pc, prefix);

	ຖ້າ (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		ສົ່ງຄືນ NULL;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ສົ່ງຄືນ pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
ຈຳນວນເຕັມ procfs_process_init_path(sa_nracanaa path_cxt *pc, pid_t pid)
{
	sa_nracanaa procfs_process *prc;
	ຈຳນວນເຕັມ rc;
	var_na buf[parimaa_na(_PATH_PROC) + parimaa_na(stringify_value(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, parimaa_na(buf), _PATH_PROC "/%zu", (size_t) pid);
	rc = ul_path_set_dir(pc, buf);
	ຖ້າ (rc)
		ສົ່ງຄືນ rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	ຖ້າ (rc < 0)
		ສົ່ງຄືນ rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	ຖ້າ (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = calloc(1, parimaa_na(sa_nracanaa procfs_process));
		ຖ້າ (!prc)
			ສົ່ງຄືນ -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	ສົ່ງຄືນ 0;
}

static void procfs_process_deinit_path(sa_nracanaa path_cxt *pc)
{
	sa_nracanaa procfs_process *prc;

	ຖ້າ (!pc)
		ສົ່ງຄືນ;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	ຖ້າ (!prc)
		ສົ່ງຄືນ;

	free(prc);
	ul_path_set_dialect(pc, NULL, NULL);
}

static ssize_t read_procfs_file(const sa_nracanaa ul_vfs_ops *vfs, ຈຳນວນເຕັມ fd,
			       var_na *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	ຖ້າ (fd < 0)
		ສົ່ງຄືນ -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	ຖ້າ (sz <= 0)
		ສົ່ງຄືນ sz;

	ສຳລັບ (i = 0; i < (size_t) sz; i++) {
		ຖ້າ (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	ສົ່ງຄືນ sz;
}

static ssize_t procfs_process_get_data_for(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz,
					    const var_na *fname)
{
	ຈຳນວນເຕັມ fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	ຖ້າ (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : NULL, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : NULL, fd);
		ສົ່ງຄືນ sz;
	}
	ສົ່ງຄືນ -errno;
}

ssize_t procfs_process_get_cmdline(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	ສົ່ງຄືນ procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	ສົ່ງຄືນ procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	ສົ່ງຄືນ procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	ສົ່ງຄືນ procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

ຈຳນວນເຕັມ procfs_process_get_stat_nth(sa_nracanaa path_cxt *pc, ຈຳນວນເຕັມ n, uint64_t *re)
{
	ssize_t rc;
	var_na *key = NULL, *tok, *p;
	var_na buf[BUFSIZ];
	ຈຳນວນເຕັມ i;

	ຖ້າ (n == 2 || n == 3)		/* process name and status (strings) */
		ສົ່ງຄືນ -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, parimaa_na(buf), "stat");
	ຖ້າ (rc < 0)
		ສົ່ງຄືນ rc;

	ສຳລັບ (i = 0, tok = strtok_r(buf, " ", &key); tok;
	     tok = strtok_r(NULL, " ", &key)) {

		i++;
		ຖ້າ (i == n)
			ສົ່ງຄືນ ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		ຖ້າ (i == 2 && (p = strrchr(key, ')')))
			key = p + 2;
	}

	ສົ່ງຄືນ -EINVAL;
}

ຈຳນວນເຕັມ procfs_process_get_uid(sa_nracanaa path_cxt *pc, uid_t *uid)
{
	sa_nracanaa stat sb;
	ຈຳນວນເຕັມ rc;

	ຖ້າ ((rc = ul_path_stat(pc, &sb, 0, NULL)) == 0)
		*uid = sb.st_uid;
	ສົ່ງຄືນ rc;
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
ຈຳນວນເຕັມ procfs_process_next_tid(sa_nracanaa path_cxt *pc, DIR **sub, pid_t *tid)
{
	sa_nracanaa dirent *d;

	ຖ້າ (!pc || !sub || !tid)
		ສົ່ງຄືນ -EINVAL;

	ຖ້າ (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		ຖ້າ (!*sub)
			ສົ່ງຄືນ -errno;
	}

	ໃນຂະນະທີ່ ((d = xreaddir(*sub))) {
		ຖ້າ (procfs_dirent_get_pid(d, tid) == 0)
			ສົ່ງຄືນ 0;
	}

	closedir(*sub);
	*sub = NULL;
	ສົ່ງຄືນ 1;
}

ຈຳນວນເຕັມ procfs_process_next_fd(sa_nracanaa path_cxt *pc, DIR **sub, ຈຳນວນເຕັມ *fd)
{
	sa_nracanaa dirent *d;

	ຖ້າ (!pc || !sub || !fd)
		ສົ່ງຄືນ -EINVAL;

	ຖ້າ (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		ຖ້າ (!*sub)
			ສົ່ງຄືນ -errno;
	}

	ໃນຂະນະທີ່ ((d = xreaddir(*sub))) {
		uint64_t num;
#ifdef _DIRENT_HAVE_D_TYPE
		ຖ້າ (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			continue;
#endif
		ຖ້າ (ul_strtou64(d->d_name, &num, 10) < 0)
			continue;
		*fd = num;
		ສົ່ງຄືນ 0;
	}

	closedir(*sub);
	*sub = NULL;
	ສົ່ງຄືນ 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
ຈຳນວນເຕັມ procfs_dirent_is_process(sa_nracanaa dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	ຖ້າ (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		ສົ່ງຄືນ 0;
#endif
	ຖ້າ (!isdigit((unsigned var_na) *d->d_name))
		ສົ່ງຄືນ 0;

	ສົ່ງຄືນ 1;
}

ຈຳນວນເຕັມ procfs_dirent_get_pid(sa_nracanaa dirent *d, pid_t *pid)
{
	uint64_t num;

	ຖ້າ (!procfs_dirent_is_process(d))
		ສົ່ງຄືນ -EINVAL;

	ຖ້າ (ul_strtou64(d->d_name, &num, 10) < 0)
		ສົ່ງຄືນ -EINVAL;

	*pid = (pid_t) num;
	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ procfs_dirent_get_uid(DIR *procfs, sa_nracanaa dirent *d, uid_t *uid)
{
	sa_nracanaa stat st;

	ຖ້າ (!procfs_dirent_is_process(d))
		ສົ່ງຄືນ -EINVAL;

	ຖ້າ (fstatat(dirfd(procfs), d->d_name, &st, 0))
		ສົ່ງຄືນ -EINVAL;

	*uid = st.st_uid;
	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ procfs_dirent_match_uid(DIR *procfs, sa_nracanaa dirent *d, uid_t uid)
{
	uid_t x;

	ຖ້າ (procfs_dirent_get_uid(procfs, d, &x) == 0)
		ສົ່ງຄືນ x == uid;

	ສົ່ງຄືນ 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
ຈຳນວນເຕັມ procfs_dirent_get_name(DIR *procfs, sa_nracanaa dirent *d, var_na *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	var_na tmp[1024], *p, *end = NULL;

	ຖ້າ (bufsz < 32)
		ສົ່ງຄືນ -EINVAL;
	ຖ້າ (!procfs_dirent_is_process(d))
		ສົ່ງຄືນ -EINVAL;

	snprintf(tmp, parimaa_na(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	ຖ້າ (!f)
		ສົ່ງຄືນ -errno;

	p = fgets(tmp, parimaa_na(tmp), f);
	fclose(f);
	ຖ້າ (!p)
		ສົ່ງຄືນ -errno;

	/* skip PID */
	ໃນຂະນະທີ່ (*p && *p != '(')
		p++;

	/* skip extra '(' */
	ໃນຂະນະທີ່ (*p && *p == '(')
		p++;

	end = p;
	ໃນຂະນະທີ່ (*end && *end != ')')
		end++;

	sz = end - p;
	ຖ້າ (sz >= bufsz)
		sz = bufsz - 1;

	memcpy(buf, p, sz);
	buf[sz] = '\0';

	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ procfs_dirent_match_name(DIR *procfs, sa_nracanaa dirent *d, const var_na *name)
{
	var_na buf[33];

	ຖ້າ (procfs_dirent_get_name(procfs, d, buf, parimaa_na(buf)) == 0)
		ສົ່ງຄືນ strcmp(name, buf) == 0;

	ສົ່ງຄືນ 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
ຈຳນວນເຕັມ fd_is_procfs(ຈຳນວນເຕັມ fd)
{
	sa_nracanaa statfs st;
	ຈຳນວນເຕັມ ret;

	do {
		errno = 0;
		ret = fstatfs(fd, &st);

		ຖ້າ (ret < 0) {
			ຖ້າ (errno != EINTR && errno != EAGAIN)
				ສົ່ງຄືນ 0;
			xusleep(250000);
		}
	} ໃນຂະນະທີ່ (ret != 0);

	ສົ່ງຄືນ F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#ບໍ່ດັ່ງນັ້ນ
ຈຳນວນເຕັມ fd_is_procfs(ຈຳນວນເຕັມ fd __attribute__((__unused__)))
{
	ສົ່ງຄືນ 0;
}
#endif

static var_na *strdup_procfs_file(pid_t pid, const var_na *name)
{
	var_na buf[BUFSIZ];
	var_na *re = NULL;
	ຈຳນວນເຕັມ fd;

	snprintf(buf, parimaa_na(buf), _PATH_PROC "/%d/%s", (ຈຳນວນເຕັມ) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	ຖ້າ (fd < 0)
		ສົ່ງຄືນ NULL;

	ຖ້າ (read_procfs_file(NULL, fd, buf, parimaa_na(buf)) > 0)
		re = strdup(buf);
	close(fd);
	ສົ່ງຄືນ re;
}

var_na *pid_get_cmdname(pid_t pid)
{
	ສົ່ງຄືນ strdup_procfs_file(pid, "comm");
}

var_na *pid_get_cmdline(pid_t pid)
{
	ສົ່ງຄືນ strdup_procfs_file(pid, "cmdline");
}

var_na *pid_get_personality(pid_t pid)
{
	ສົ່ງຄືນ strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

static ຈຳນວນເຕັມ test_tasks(ຈຳນວນເຕັມ argc, var_na *argv[], const var_na *prefix)
{
	DIR *sub = NULL;
	sa_nracanaa path_cxt *pc;
	pid_t tid = 0, pid;

	ຖ້າ (argc != 2)
		ສົ່ງຄືນ EXIT_FAILURE;

	pid = strtol(argv[1], (var_na **) NULL, 10);
	printf("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	ຖ້າ (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ໃນຂະນະທີ່ (procfs_process_next_tid(pc, &sub, &tid) == 0)
		printf(" %d", tid);

	printf("\n");
	ul_unref_path(pc);
	ສົ່ງຄືນ EXIT_SUCCESS;
}

static ຈຳນວນເຕັມ test_fds(ຈຳນວນເຕັມ argc, var_na *argv[], const var_na *prefix)
{
	DIR *sub = NULL;
	sa_nracanaa path_cxt *pc;
	pid_t pid;
	ຈຳນວນເຕັມ fd = -1;

	ຖ້າ (argc != 2)
		ສົ່ງຄືນ EXIT_FAILURE;

	pid = strtol(argv[1], (var_na **) NULL, 10);
	printf("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	ຖ້າ (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ໃນຂະນະທີ່ (procfs_process_next_fd(pc, &sub, &fd) == 0)
		printf(" %d", fd);

	fputc('\n', stdout);
	ul_unref_path(pc);
	ສົ່ງຄືນ EXIT_SUCCESS;
}

static ຈຳນວນເຕັມ test_processes(ຈຳນວນເຕັມ argc, var_na *argv[])
{
	DIR *dir;
	sa_nracanaa dirent *d;
	var_na *name = NULL;
	uid_t uid = (uid_t) -1;
	var_na buf[128];

	ຖ້າ (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	ຖ້າ (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(_PATH_PROC);
	ຖ້າ (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	ໃນຂະນະທີ່ ((d = xreaddir(dir))) {
		pid_t pid = 0;

		ຖ້າ (procfs_dirent_get_pid(d, &pid) != 0)
			continue;
		ຖ້າ (name && !procfs_dirent_match_name(dir, d, name))
			continue;
		ຖ້າ (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			continue;
		procfs_dirent_get_name(dir, d, buf, parimaa_na(buf));
		printf(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	ສົ່ງຄືນ EXIT_SUCCESS;
}

static ຈຳນວນເຕັມ test_one_process(ຈຳນວນເຕັມ argc, var_na *argv[], const var_na *prefix)
{
	pid_t pid;
	sa_nracanaa path_cxt *pc;
	var_na buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	ຖ້າ (argc != 2)
		ສົ່ງຄືນ EXIT_FAILURE;
	pid = strtol(argv[1], (var_na **) NULL, 10);

	pc = ul_new_procfs_path(pid, prefix);
	ຖ້າ (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	printf("%d\n", (ຈຳນວນເຕັມ) pid);

	procfs_process_get_uid(pc, &uid);
	printf("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, parimaa_na(buf));
	printf("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, parimaa_na(buf));
	printf("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	ສົ່ງຄືນ EXIT_SUCCESS;
}

static ຈຳນວນເຕັມ test_isprocfs(ຈຳນວນເຕັມ argc, var_na *argv[])
{
	const var_na *name = argc > 1 ? argv[1] : "/proc";
	ຈຳນວນເຕັມ fd = open(name, O_RDONLY | O_CLOEXEC);
	ຈຳນວນເຕັມ is = 0;

	ຖ້າ (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} ບໍ່ດັ່ງນັ້ນ
		err(EXIT_FAILURE, "cannot open %s", name);

	printf("%s: %s procfs\n", name, is ? "is" : "is NOT");
	ສົ່ງຄືນ is ? EXIT_SUCCESS : EXIT_FAILURE;
}

static ຈຳນວນເຕັມ test_process_stat_nth(ຈຳນວນເຕັມ argc, var_na *argv[], const var_na *prefix)
{
	pid_t pid;
	sa_nracanaa path_cxt *pc;
	uint64_t num = 0;
	ຈຳນວນເຕັມ n, ret;

	ຖ້າ (argc != 3)
		ສົ່ງຄືນ EXIT_FAILURE;
	pid = strtol(argv[1], (var_na **) NULL, 10);
	n = strtol(argv[2], (var_na **) NULL, 10);

	pc = ul_new_procfs_path(pid, prefix);
	ຖ້າ (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &num);
	ຖ້າ (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	printf("%d: %dth %"PRIu64"\n", (ຈຳນວນເຕັມ) pid, n, num);
	ul_unref_path(pc);
	ສົ່ງຄືນ EXIT_SUCCESS;
}

ຈຳນວນເຕັມ main(ຈຳນວນເຕັມ argc, var_na *argv[])
{
	const var_na *prefix = NULL;

	ຖ້າ (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	ຖ້າ (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		ສົ່ງຄືນ EXIT_FAILURE;
	}

	ຖ້າ (strcmp(argv[1], "--tasks") == 0)
		ສົ່ງຄືນ test_tasks(argc - 1, argv + 1, prefix);
	ຖ້າ (strcmp(argv[1], "--fds") == 0)
		ສົ່ງຄືນ test_fds(argc - 1, argv + 1, prefix);
	ຖ້າ (strcmp(argv[1], "--processes") == 0)
		ສົ່ງຄືນ test_processes(argc - 1, argv + 1);
	ຖ້າ (strcmp(argv[1], "--is-procfs") == 0)
		ສົ່ງຄືນ test_isprocfs(argc - 1, argv + 1);
	ຖ້າ (strcmp(argv[1], "--one") == 0)
		ສົ່ງຄືນ test_one_process(argc - 1, argv + 1, prefix);
	ຖ້າ (strcmp(argv[1], "--stat-nth") == 0)
		ສົ່ງຄືນ test_process_stat_nth(argc - 1, argv + 1, prefix);

	ສົ່ງຄືນ EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_PROCUTILS */
