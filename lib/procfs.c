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

static void procfs_process_deinit_path(struct path_cxt *pc);

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
	nếu (ulprocfs_debug_mask)
		trả_về;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

struct path_cxt *ul_new_procfs_path(pid_t pid, const char *prefix)
{
	struct path_cxt *pc = ul_new_path(NULL);

	nếu (!pc)
		trả_về NULL;
	nếu (prefix)
		ul_path_set_prefix(pc, prefix);

	nếu (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		trả_về NULL;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	trả_về pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
số_nguyên procfs_process_init_path(struct path_cxt *pc, pid_t pid)
{
	struct procfs_process *prc;
	số_nguyên rc;
	char buf[sizeof(_PATH_PROC) + sizeof(stringify_value(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, sizeof(buf), _PATH_PROC "/%zu", (size_t) pid);
	rc = ul_path_set_dir(pc, buf);
	nếu (rc)
		trả_về rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	nếu (rc < 0)
		trả_về rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	nếu (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = calloc(1, sizeof(struct procfs_process));
		nếu (!prc)
			trả_về -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	trả_về 0;
}

static void procfs_process_deinit_path(struct path_cxt *pc)
{
	struct procfs_process *prc;

	nếu (!pc)
		trả_về;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	nếu (!prc)
		trả_về;

	free(prc);
	ul_path_set_dialect(pc, NULL, NULL);
}

static ssize_t read_procfs_file(const struct ul_vfs_ops *vfs, số_nguyên fd,
			       char *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	nếu (fd < 0)
		trả_về -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	nếu (sz <= 0)
		trả_về sz;

	cho (i = 0; i < (size_t) sz; i++) {
		nếu (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	trả_về sz;
}

static ssize_t procfs_process_get_data_for(struct path_cxt *pc, char *buf, size_t bufsz,
					    const char *fname)
{
	số_nguyên fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	nếu (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : NULL, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : NULL, fd);
		trả_về sz;
	}
	trả_về -errno;
}

ssize_t procfs_process_get_cmdline(struct path_cxt *pc, char *buf, size_t bufsz)
{
	trả_về procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(struct path_cxt *pc, char *buf, size_t bufsz)
{
	trả_về procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(struct path_cxt *pc, char *buf, size_t bufsz)
{
	trả_về procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(struct path_cxt *pc, char *buf, size_t bufsz)
{
	trả_về procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

số_nguyên procfs_process_get_stat_nth(struct path_cxt *pc, số_nguyên n, uint64_t *re)
{
	ssize_t rc;
	char *key = NULL, *tok, *p;
	char buf[BUFSIZ];
	số_nguyên i;

	nếu (n == 2 || n == 3)		/* process name and status (strings) */
		trả_về -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, sizeof(buf), "stat");
	nếu (rc < 0)
		trả_về rc;

	cho (i = 0, tok = strtok_r(buf, " ", &key); tok;
	     tok = strtok_r(NULL, " ", &key)) {

		i++;
		nếu (i == n)
			trả_về ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		nếu (i == 2 && (p = strrchr(key, ')')))
			key = p + 2;
	}

	trả_về -EINVAL;
}

số_nguyên procfs_process_get_uid(struct path_cxt *pc, uid_t *uid)
{
	struct stat sb;
	số_nguyên rc;

	nếu ((rc = ul_path_stat(pc, &sb, 0, NULL)) == 0)
		*uid = sb.st_uid;
	trả_về rc;
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
số_nguyên procfs_process_next_tid(struct path_cxt *pc, DIR **sub, pid_t *tid)
{
	struct dirent *d;

	nếu (!pc || !sub || !tid)
		trả_về -EINVAL;

	nếu (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		nếu (!*sub)
			trả_về -errno;
	}

	trong_khi ((d = xreaddir(*sub))) {
		nếu (procfs_dirent_get_pid(d, tid) == 0)
			trả_về 0;
	}

	closedir(*sub);
	*sub = NULL;
	trả_về 1;
}

số_nguyên procfs_process_next_fd(struct path_cxt *pc, DIR **sub, số_nguyên *fd)
{
	struct dirent *d;

	nếu (!pc || !sub || !fd)
		trả_về -EINVAL;

	nếu (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		nếu (!*sub)
			trả_về -errno;
	}

	trong_khi ((d = xreaddir(*sub))) {
		uint64_t num;
#ifdef _DIRENT_HAVE_D_TYPE
		nếu (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			continue;
#endif
		nếu (ul_strtou64(d->d_name, &num, 10) < 0)
			continue;
		*fd = num;
		trả_về 0;
	}

	closedir(*sub);
	*sub = NULL;
	trả_về 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
số_nguyên procfs_dirent_is_process(struct dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	nếu (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		trả_về 0;
#endif
	nếu (!isdigit((unsigned char) *d->d_name))
		trả_về 0;

	trả_về 1;
}

số_nguyên procfs_dirent_get_pid(struct dirent *d, pid_t *pid)
{
	uint64_t num;

	nếu (!procfs_dirent_is_process(d))
		trả_về -EINVAL;

	nếu (ul_strtou64(d->d_name, &num, 10) < 0)
		trả_về -EINVAL;

	*pid = (pid_t) num;
	trả_về 0;
}

số_nguyên procfs_dirent_get_uid(DIR *procfs, struct dirent *d, uid_t *uid)
{
	struct stat st;

	nếu (!procfs_dirent_is_process(d))
		trả_về -EINVAL;

	nếu (fstatat(dirfd(procfs), d->d_name, &st, 0))
		trả_về -EINVAL;

	*uid = st.st_uid;
	trả_về 0;
}

số_nguyên procfs_dirent_match_uid(DIR *procfs, struct dirent *d, uid_t uid)
{
	uid_t x;

	nếu (procfs_dirent_get_uid(procfs, d, &x) == 0)
		trả_về x == uid;

	trả_về 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
số_nguyên procfs_dirent_get_name(DIR *procfs, struct dirent *d, char *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	char tmp[1024], *p, *end = NULL;

	nếu (bufsz < 32)
		trả_về -EINVAL;
	nếu (!procfs_dirent_is_process(d))
		trả_về -EINVAL;

	snprintf(tmp, sizeof(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	nếu (!f)
		trả_về -errno;

	p = fgets(tmp, sizeof(tmp), f);
	fclose(f);
	nếu (!p)
		trả_về -errno;

	/* skip PID */
	trong_khi (*p && *p != '(')
		p++;

	/* skip extra '(' */
	trong_khi (*p && *p == '(')
		p++;

	end = p;
	trong_khi (*end && *end != ')')
		end++;

	sz = end - p;
	nếu (sz >= bufsz)
		sz = bufsz - 1;

	memcpy(buf, p, sz);
	buf[sz] = '\0';

	trả_về 0;
}

số_nguyên procfs_dirent_match_name(DIR *procfs, struct dirent *d, const char *name)
{
	char buf[33];

	nếu (procfs_dirent_get_name(procfs, d, buf, sizeof(buf)) == 0)
		trả_về strcmp(name, buf) == 0;

	trả_về 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
số_nguyên fd_is_procfs(số_nguyên fd)
{
	struct statfs st;
	số_nguyên ret;

	do {
		errno = 0;
		ret = fstatfs(fd, &st);

		nếu (ret < 0) {
			nếu (errno != EINTR && errno != EAGAIN)
				trả_về 0;
			xusleep(250000);
		}
	} trong_khi (ret != 0);

	trả_về F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#không_thì
số_nguyên fd_is_procfs(số_nguyên fd __attribute__((__unused__)))
{
	trả_về 0;
}
#endif

static char *strdup_procfs_file(pid_t pid, const char *name)
{
	char buf[BUFSIZ];
	char *re = NULL;
	số_nguyên fd;

	snprintf(buf, sizeof(buf), _PATH_PROC "/%d/%s", (số_nguyên) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	nếu (fd < 0)
		trả_về NULL;

	nếu (read_procfs_file(NULL, fd, buf, sizeof(buf)) > 0)
		re = strdup(buf);
	close(fd);
	trả_về re;
}

char *pid_get_cmdname(pid_t pid)
{
	trả_về strdup_procfs_file(pid, "comm");
}

char *pid_get_cmdline(pid_t pid)
{
	trả_về strdup_procfs_file(pid, "cmdline");
}

char *pid_get_personality(pid_t pid)
{
	trả_về strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

static số_nguyên test_tasks(số_nguyên argc, char *argv[], const char *prefix)
{
	DIR *sub = NULL;
	struct path_cxt *pc;
	pid_t tid = 0, pid;

	nếu (argc != 2)
		trả_về EXIT_FAILURE;

	pid = strtol(argv[1], (char **) NULL, 10);
	printf("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	nếu (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	trong_khi (procfs_process_next_tid(pc, &sub, &tid) == 0)
		printf(" %d", tid);

	printf("\n");
	ul_unref_path(pc);
	trả_về EXIT_SUCCESS;
}

static số_nguyên test_fds(số_nguyên argc, char *argv[], const char *prefix)
{
	DIR *sub = NULL;
	struct path_cxt *pc;
	pid_t pid;
	số_nguyên fd = -1;

	nếu (argc != 2)
		trả_về EXIT_FAILURE;

	pid = strtol(argv[1], (char **) NULL, 10);
	printf("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	nếu (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	trong_khi (procfs_process_next_fd(pc, &sub, &fd) == 0)
		printf(" %d", fd);

	fputc('\n', stdout);
	ul_unref_path(pc);
	trả_về EXIT_SUCCESS;
}

static số_nguyên test_processes(số_nguyên argc, char *argv[])
{
	DIR *dir;
	struct dirent *d;
	char *name = NULL;
	uid_t uid = (uid_t) -1;
	char buf[128];

	nếu (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	nếu (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(_PATH_PROC);
	nếu (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	trong_khi ((d = xreaddir(dir))) {
		pid_t pid = 0;

		nếu (procfs_dirent_get_pid(d, &pid) != 0)
			continue;
		nếu (name && !procfs_dirent_match_name(dir, d, name))
			continue;
		nếu (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			continue;
		procfs_dirent_get_name(dir, d, buf, sizeof(buf));
		printf(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	trả_về EXIT_SUCCESS;
}

static số_nguyên test_one_process(số_nguyên argc, char *argv[], const char *prefix)
{
	pid_t pid;
	struct path_cxt *pc;
	char buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	nếu (argc != 2)
		trả_về EXIT_FAILURE;
	pid = strtol(argv[1], (char **) NULL, 10);

	pc = ul_new_procfs_path(pid, prefix);
	nếu (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	printf("%d\n", (số_nguyên) pid);

	procfs_process_get_uid(pc, &uid);
	printf("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, sizeof(buf));
	printf("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, sizeof(buf));
	printf("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	trả_về EXIT_SUCCESS;
}

static số_nguyên test_isprocfs(số_nguyên argc, char *argv[])
{
	const char *name = argc > 1 ? argv[1] : "/proc";
	số_nguyên fd = open(name, O_RDONLY | O_CLOEXEC);
	số_nguyên is = 0;

	nếu (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} không_thì
		err(EXIT_FAILURE, "cannot open %s", name);

	printf("%s: %s procfs\n", name, is ? "is" : "is NOT");
	trả_về is ? EXIT_SUCCESS : EXIT_FAILURE;
}

static số_nguyên test_process_stat_nth(số_nguyên argc, char *argv[], const char *prefix)
{
	pid_t pid;
	struct path_cxt *pc;
	uint64_t num = 0;
	số_nguyên n, ret;

	nếu (argc != 3)
		trả_về EXIT_FAILURE;
	pid = strtol(argv[1], (char **) NULL, 10);
	n = strtol(argv[2], (char **) NULL, 10);

	pc = ul_new_procfs_path(pid, prefix);
	nếu (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &num);
	nếu (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	printf("%d: %dth %"PRIu64"\n", (số_nguyên) pid, n, num);
	ul_unref_path(pc);
	trả_về EXIT_SUCCESS;
}

số_nguyên main(số_nguyên argc, char *argv[])
{
	const char *prefix = NULL;

	nếu (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	nếu (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		trả_về EXIT_FAILURE;
	}

	nếu (strcmp(argv[1], "--tasks") == 0)
		trả_về test_tasks(argc - 1, argv + 1, prefix);
	nếu (strcmp(argv[1], "--fds") == 0)
		trả_về test_fds(argc - 1, argv + 1, prefix);
	nếu (strcmp(argv[1], "--processes") == 0)
		trả_về test_processes(argc - 1, argv + 1);
	nếu (strcmp(argv[1], "--is-procfs") == 0)
		trả_về test_isprocfs(argc - 1, argv + 1);
	nếu (strcmp(argv[1], "--one") == 0)
		trả_về test_one_process(argc - 1, argv + 1, prefix);
	nếu (strcmp(argv[1], "--stat-nth") == 0)
		trả_về test_process_stat_nth(argc - 1, argv + 1, prefix);

	trả_về EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_PROCUTILS */
