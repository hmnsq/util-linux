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
	ако (ulprocfs_debug_mask)
		върни;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

struct path_cxt *ul_new_procfs_path(pid_t pid, const char *prefix)
{
	struct path_cxt *pc = ul_new_path(NULL);

	ако (!pc)
		върни NULL;
	ако (prefix)
		ul_path_set_prefix(pc, prefix);

	ако (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		върни NULL;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	върни pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
цяло procfs_process_init_path(struct path_cxt *pc, pid_t pid)
{
	struct procfs_process *prc;
	цяло rc;
	char buf[sizeof(_PATH_PROC) + sizeof(stringify_value(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, sizeof(buf), _PATH_PROC "/%zu", (size_t) pid);
	rc = ul_path_set_dir(pc, buf);
	ако (rc)
		върни rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	ако (rc < 0)
		върни rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	ако (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = calloc(1, sizeof(struct procfs_process));
		ако (!prc)
			върни -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	върни 0;
}

static void procfs_process_deinit_path(struct path_cxt *pc)
{
	struct procfs_process *prc;

	ако (!pc)
		върни;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	ако (!prc)
		върни;

	free(prc);
	ul_path_set_dialect(pc, NULL, NULL);
}

static ssize_t read_procfs_file(const struct ul_vfs_ops *vfs, цяло fd,
			       char *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	ако (fd < 0)
		върни -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	ако (sz <= 0)
		върни sz;

	за (i = 0; i < (size_t) sz; i++) {
		ако (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	върни sz;
}

static ssize_t procfs_process_get_data_for(struct path_cxt *pc, char *buf, size_t bufsz,
					    const char *fname)
{
	цяло fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	ако (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : NULL, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : NULL, fd);
		върни sz;
	}
	върни -errno;
}

ssize_t procfs_process_get_cmdline(struct path_cxt *pc, char *buf, size_t bufsz)
{
	върни procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(struct path_cxt *pc, char *buf, size_t bufsz)
{
	върни procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(struct path_cxt *pc, char *buf, size_t bufsz)
{
	върни procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(struct path_cxt *pc, char *buf, size_t bufsz)
{
	върни procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

цяло procfs_process_get_stat_nth(struct path_cxt *pc, цяло n, uint64_t *re)
{
	ssize_t rc;
	char *key = NULL, *tok, *p;
	char buf[BUFSIZ];
	цяло i;

	ако (n == 2 || n == 3)		/* process name and status (strings) */
		върни -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, sizeof(buf), "stat");
	ако (rc < 0)
		върни rc;

	за (i = 0, tok = strtok_r(buf, " ", &key); tok;
	     tok = strtok_r(NULL, " ", &key)) {

		i++;
		ако (i == n)
			върни ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		ако (i == 2 && (p = strrchr(key, ')')))
			key = p + 2;
	}

	върни -EINVAL;
}

цяло procfs_process_get_uid(struct path_cxt *pc, uid_t *uid)
{
	struct stat sb;
	цяло rc;

	ако ((rc = ul_path_stat(pc, &sb, 0, NULL)) == 0)
		*uid = sb.st_uid;
	върни rc;
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
цяло procfs_process_next_tid(struct path_cxt *pc, DIR **sub, pid_t *tid)
{
	struct dirent *d;

	ако (!pc || !sub || !tid)
		върни -EINVAL;

	ако (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		ако (!*sub)
			върни -errno;
	}

	докато ((d = xreaddir(*sub))) {
		ако (procfs_dirent_get_pid(d, tid) == 0)
			върни 0;
	}

	closedir(*sub);
	*sub = NULL;
	върни 1;
}

цяло procfs_process_next_fd(struct path_cxt *pc, DIR **sub, цяло *fd)
{
	struct dirent *d;

	ако (!pc || !sub || !fd)
		върни -EINVAL;

	ако (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		ако (!*sub)
			върни -errno;
	}

	докато ((d = xreaddir(*sub))) {
		uint64_t num;
#ifdef _DIRENT_HAVE_D_TYPE
		ако (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			continue;
#endif
		ако (ul_strtou64(d->d_name, &num, 10) < 0)
			continue;
		*fd = num;
		върни 0;
	}

	closedir(*sub);
	*sub = NULL;
	върни 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
цяло procfs_dirent_is_process(struct dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	ако (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		върни 0;
#endif
	ако (!isdigit((unsigned char) *d->d_name))
		върни 0;

	върни 1;
}

цяло procfs_dirent_get_pid(struct dirent *d, pid_t *pid)
{
	uint64_t num;

	ако (!procfs_dirent_is_process(d))
		върни -EINVAL;

	ако (ul_strtou64(d->d_name, &num, 10) < 0)
		върни -EINVAL;

	*pid = (pid_t) num;
	върни 0;
}

цяло procfs_dirent_get_uid(DIR *procfs, struct dirent *d, uid_t *uid)
{
	struct stat st;

	ако (!procfs_dirent_is_process(d))
		върни -EINVAL;

	ако (fstatat(dirfd(procfs), d->d_name, &st, 0))
		върни -EINVAL;

	*uid = st.st_uid;
	върни 0;
}

цяло procfs_dirent_match_uid(DIR *procfs, struct dirent *d, uid_t uid)
{
	uid_t x;

	ако (procfs_dirent_get_uid(procfs, d, &x) == 0)
		върни x == uid;

	върни 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
цяло procfs_dirent_get_name(DIR *procfs, struct dirent *d, char *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	char tmp[1024], *p, *end = NULL;

	ако (bufsz < 32)
		върни -EINVAL;
	ако (!procfs_dirent_is_process(d))
		върни -EINVAL;

	snprintf(tmp, sizeof(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	ако (!f)
		върни -errno;

	p = fgets(tmp, sizeof(tmp), f);
	fclose(f);
	ако (!p)
		върни -errno;

	/* skip PID */
	докато (*p && *p != '(')
		p++;

	/* skip extra '(' */
	докато (*p && *p == '(')
		p++;

	end = p;
	докато (*end && *end != ')')
		end++;

	sz = end - p;
	ако (sz >= bufsz)
		sz = bufsz - 1;

	memcpy(buf, p, sz);
	buf[sz] = '\0';

	върни 0;
}

цяло procfs_dirent_match_name(DIR *procfs, struct dirent *d, const char *name)
{
	char buf[33];

	ако (procfs_dirent_get_name(procfs, d, buf, sizeof(buf)) == 0)
		върни strcmp(name, buf) == 0;

	върни 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
цяло fd_is_procfs(цяло fd)
{
	struct statfs st;
	цяло ret;

	do {
		errno = 0;
		ret = fstatfs(fd, &st);

		ако (ret < 0) {
			ако (errno != EINTR && errno != EAGAIN)
				върни 0;
			xusleep(250000);
		}
	} докато (ret != 0);

	върни F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#иначе
цяло fd_is_procfs(цяло fd __attribute__((__unused__)))
{
	върни 0;
}
#endif

static char *strdup_procfs_file(pid_t pid, const char *name)
{
	char buf[BUFSIZ];
	char *re = NULL;
	цяло fd;

	snprintf(buf, sizeof(buf), _PATH_PROC "/%d/%s", (цяло) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	ако (fd < 0)
		върни NULL;

	ако (read_procfs_file(NULL, fd, buf, sizeof(buf)) > 0)
		re = strdup(buf);
	close(fd);
	върни re;
}

char *pid_get_cmdname(pid_t pid)
{
	върни strdup_procfs_file(pid, "comm");
}

char *pid_get_cmdline(pid_t pid)
{
	върни strdup_procfs_file(pid, "cmdline");
}

char *pid_get_personality(pid_t pid)
{
	върни strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

static цяло test_tasks(цяло argc, char *argv[], const char *prefix)
{
	DIR *sub = NULL;
	struct path_cxt *pc;
	pid_t tid = 0, pid;

	ако (argc != 2)
		върни EXIT_FAILURE;

	pid = strtol(argv[1], (char **) NULL, 10);
	printf("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	ако (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	докато (procfs_process_next_tid(pc, &sub, &tid) == 0)
		printf(" %d", tid);

	printf("\n");
	ul_unref_path(pc);
	върни EXIT_SUCCESS;
}

static цяло test_fds(цяло argc, char *argv[], const char *prefix)
{
	DIR *sub = NULL;
	struct path_cxt *pc;
	pid_t pid;
	цяло fd = -1;

	ако (argc != 2)
		върни EXIT_FAILURE;

	pid = strtol(argv[1], (char **) NULL, 10);
	printf("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	ако (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	докато (procfs_process_next_fd(pc, &sub, &fd) == 0)
		printf(" %d", fd);

	fputc('\n', stdout);
	ul_unref_path(pc);
	върни EXIT_SUCCESS;
}

static цяло test_processes(цяло argc, char *argv[])
{
	DIR *dir;
	struct dirent *d;
	char *name = NULL;
	uid_t uid = (uid_t) -1;
	char buf[128];

	ако (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	ако (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(_PATH_PROC);
	ако (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	докато ((d = xreaddir(dir))) {
		pid_t pid = 0;

		ако (procfs_dirent_get_pid(d, &pid) != 0)
			continue;
		ако (name && !procfs_dirent_match_name(dir, d, name))
			continue;
		ако (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			continue;
		procfs_dirent_get_name(dir, d, buf, sizeof(buf));
		printf(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	върни EXIT_SUCCESS;
}

static цяло test_one_process(цяло argc, char *argv[], const char *prefix)
{
	pid_t pid;
	struct path_cxt *pc;
	char buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	ако (argc != 2)
		върни EXIT_FAILURE;
	pid = strtol(argv[1], (char **) NULL, 10);

	pc = ul_new_procfs_path(pid, prefix);
	ако (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	printf("%d\n", (цяло) pid);

	procfs_process_get_uid(pc, &uid);
	printf("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, sizeof(buf));
	printf("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, sizeof(buf));
	printf("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	върни EXIT_SUCCESS;
}

static цяло test_isprocfs(цяло argc, char *argv[])
{
	const char *name = argc > 1 ? argv[1] : "/proc";
	цяло fd = open(name, O_RDONLY | O_CLOEXEC);
	цяло is = 0;

	ако (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} иначе
		err(EXIT_FAILURE, "cannot open %s", name);

	printf("%s: %s procfs\n", name, is ? "is" : "is NOT");
	върни is ? EXIT_SUCCESS : EXIT_FAILURE;
}

static цяло test_process_stat_nth(цяло argc, char *argv[], const char *prefix)
{
	pid_t pid;
	struct path_cxt *pc;
	uint64_t num = 0;
	цяло n, ret;

	ако (argc != 3)
		върни EXIT_FAILURE;
	pid = strtol(argv[1], (char **) NULL, 10);
	n = strtol(argv[2], (char **) NULL, 10);

	pc = ul_new_procfs_path(pid, prefix);
	ако (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &num);
	ако (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	printf("%d: %dth %"PRIu64"\n", (цяло) pid, n, num);
	ul_unref_path(pc);
	върни EXIT_SUCCESS;
}

цяло main(цяло argc, char *argv[])
{
	const char *prefix = NULL;

	ако (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	ако (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		върни EXIT_FAILURE;
	}

	ако (strcmp(argv[1], "--tasks") == 0)
		върни test_tasks(argc - 1, argv + 1, prefix);
	ако (strcmp(argv[1], "--fds") == 0)
		върни test_fds(argc - 1, argv + 1, prefix);
	ако (strcmp(argv[1], "--processes") == 0)
		върни test_processes(argc - 1, argv + 1);
	ако (strcmp(argv[1], "--is-procfs") == 0)
		върни test_isprocfs(argc - 1, argv + 1);
	ако (strcmp(argv[1], "--one") == 0)
		върни test_one_process(argc - 1, argv + 1, prefix);
	ако (strcmp(argv[1], "--stat-nth") == 0)
		върни test_process_stat_nth(argc - 1, argv + 1, prefix);

	върни EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_PROCUTILS */
