/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2021]
 */
#شامل <ctype.h>
#شامل <unistd.h>
#شامل <errno.h>

#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif

#شامل "c.h"
#شامل "pathnames.h"
#شامل "procfs.h"
#شامل "fileutils.h"
#شامل "all-io.h"
#شامل "vfs.h"
#شامل "debug.h"
#شامل "strutils.h"
#شامل "statfs_magic.h"

ایستا تهی procfs_process_deinit_path(ساختار path_cxt *pc);

/*
 * Debug stuff (based on include/debug.h)
 */
ایستا UL_DEBUG_DEFINE_MASK(ulprocfs);
UL_DEBUG_DEFINE_MASKNAMES(ulprocfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPROCFS_DEBUG_INIT	(1 << 1)
#define ULPROCFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulprocfs, ULPROCFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulprocfs, ULPROCFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulprocfs, ULPROCFS_DEBUG_, m, x)

تهی ul_procfs_init_debug(تهی)
{
	اگر (ulprocfs_debug_mask)
		بازگشت;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

ساختار path_cxt *ul_new_procfs_path(pid_t pid, ثابت نویسه *prefix)
{
	ساختار path_cxt *pc = ul_new_path(پوچ);

	اگر (!pc)
		بازگشت پوچ;
	اگر (prefix)
		ul_path_set_prefix(pc, prefix);

	اگر (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		بازگشت پوچ;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	بازگشت pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
صحیح procfs_process_init_path(ساختار path_cxt *pc, pid_t pid)
{
	ساختار procfs_process *prc;
	صحیح rc;
	نویسه buf[اندازه(_PATH_PROC) + اندازه(stringify_مقدار(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, اندازه(buf), _PATH_PROC "/%zu", (size_t) pid);
	rc = ul_path_set_dir(pc, buf);
	اگر (rc)
		بازگشت rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	اگر (rc < 0)
		بازگشت rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	اگر (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = calloc(1, اندازه(ساختار procfs_process));
		اگر (!prc)
			بازگشت -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	بازگشت 0;
}

ایستا تهی procfs_process_deinit_path(ساختار path_cxt *pc)
{
	ساختار procfs_process *prc;

	اگر (!pc)
		بازگشت;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	اگر (!prc)
		بازگشت;

	آزاد(prc);
	ul_path_set_dialect(pc, پوچ, پوچ);
}

ایستا ssize_t read_procfs_file(ثابت ساختار ul_vfs_ops *vfs, صحیح fd,
			       نویسه *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	اگر (fd < 0)
		بازگشت -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	اگر (sz <= 0)
		بازگشت sz;

	برای (i = 0; i < (size_t) sz; i++) {
		اگر (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	بازگشت sz;
}

ایستا ssize_t procfs_process_get_data_for(ساختار path_cxt *pc, نویسه *buf, size_t bufsz,
					    ثابت نویسه *fname)
{
	صحیح fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	اگر (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : پوچ, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : پوچ, fd);
		بازگشت sz;
	}
	بازگشت -errno;
}

ssize_t procfs_process_get_cmdline(ساختار path_cxt *pc, نویسه *buf, size_t bufsz)
{
	بازگشت procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(ساختار path_cxt *pc, نویسه *buf, size_t bufsz)
{
	بازگشت procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(ساختار path_cxt *pc, نویسه *buf, size_t bufsz)
{
	بازگشت procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(ساختار path_cxt *pc, نویسه *buf, size_t bufsz)
{
	بازگشت procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

صحیح procfs_process_get_stat_nth(ساختار path_cxt *pc, صحیح n, uint64_t *re)
{
	ssize_t rc;
	نویسه *کلید = پوچ, *tok, *p;
	نویسه buf[BUFSIZ];
	صحیح i;

	اگر (n == 2 || n == 3)		/* process name and status (strings) */
		بازگشت -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, اندازه(buf), "stat");
	اگر (rc < 0)
		بازگشت rc;

	برای (i = 0, tok = strtok_r(buf, " ", &کلید); tok;
	     tok = strtok_r(پوچ, " ", &کلید)) {

		i++;
		اگر (i == n)
			بازگشت ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		اگر (i == 2 && (p = strrchr(کلید, ')')))
			کلید = p + 2;
	}

	بازگشت -EINVAL;
}

صحیح procfs_process_get_uid(ساختار path_cxt *pc, uid_t *uid)
{
	ساختار stat sb;
	صحیح rc;

	اگر ((rc = ul_path_stat(pc, &sb, 0, پوچ)) == 0)
		*uid = sb.st_uid;
	بازگشت rc;
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
صحیح procfs_process_next_tid(ساختار path_cxt *pc, DIR **sub, pid_t *tid)
{
	ساختار dirent *d;

	اگر (!pc || !sub || !tid)
		بازگشت -EINVAL;

	اگر (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		اگر (!*sub)
			بازگشت -errno;
	}

	وقتی ((d = xreaddir(*sub))) {
		اگر (procfs_dirent_get_pid(d, tid) == 0)
			بازگشت 0;
	}

	closedir(*sub);
	*sub = پوچ;
	بازگشت 1;
}

صحیح procfs_process_next_fd(ساختار path_cxt *pc, DIR **sub, صحیح *fd)
{
	ساختار dirent *d;

	اگر (!pc || !sub || !fd)
		بازگشت -EINVAL;

	اگر (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		اگر (!*sub)
			بازگشت -errno;
	}

	وقتی ((d = xreaddir(*sub))) {
		uint64_t رقم;
#ifdef _DIRENT_HAVE_D_TYPE
		اگر (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			ادامه;
#endif
		اگر (ul_strtou64(d->d_name, &رقم, 10) < 0)
			ادامه;
		*fd = رقم;
		بازگشت 0;
	}

	closedir(*sub);
	*sub = پوچ;
	بازگشت 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
صحیح procfs_dirent_is_process(ساختار dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	اگر (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		بازگشت 0;
#endif
	اگر (!isdigit((unsigned نویسه) *d->d_name))
		بازگشت 0;

	بازگشت 1;
}

صحیح procfs_dirent_get_pid(ساختار dirent *d, pid_t *pid)
{
	uint64_t رقم;

	اگر (!procfs_dirent_is_process(d))
		بازگشت -EINVAL;

	اگر (ul_strtou64(d->d_name, &رقم, 10) < 0)
		بازگشت -EINVAL;

	*pid = (pid_t) رقم;
	بازگشت 0;
}

صحیح procfs_dirent_get_uid(DIR *procfs, ساختار dirent *d, uid_t *uid)
{
	ساختار stat st;

	اگر (!procfs_dirent_is_process(d))
		بازگشت -EINVAL;

	اگر (fstatat(dirfd(procfs), d->d_name, &st, 0))
		بازگشت -EINVAL;

	*uid = st.st_uid;
	بازگشت 0;
}

صحیح procfs_dirent_match_uid(DIR *procfs, ساختار dirent *d, uid_t uid)
{
	uid_t x;

	اگر (procfs_dirent_get_uid(procfs, d, &x) == 0)
		بازگشت x == uid;

	بازگشت 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
صحیح procfs_dirent_get_name(DIR *procfs, ساختار dirent *d, نویسه *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	نویسه tmp[1024], *p, *end = پوچ;

	اگر (bufsz < 32)
		بازگشت -EINVAL;
	اگر (!procfs_dirent_is_process(d))
		بازگشت -EINVAL;

	snprintf(tmp, اندازه(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	اگر (!f)
		بازگشت -errno;

	p = fgets(tmp, اندازه(tmp), f);
	fclose(f);
	اگر (!p)
		بازگشت -errno;

	/* skip PID */
	وقتی (*p && *p != '(')
		p++;

	/* skip extra '(' */
	وقتی (*p && *p == '(')
		p++;

	end = p;
	وقتی (*end && *end != ')')
		end++;

	sz = end - p;
	اگر (sz >= bufsz)
		sz = bufsz - 1;

	memcpy(buf, p, sz);
	buf[sz] = '\0';

	بازگشت 0;
}

صحیح procfs_dirent_match_name(DIR *procfs, ساختار dirent *d, ثابت نویسه *name)
{
	نویسه buf[33];

	اگر (procfs_dirent_get_name(procfs, d, buf, اندازه(buf)) == 0)
		بازگشت strcmp(name, buf) == 0;

	بازگشت 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
صحیح fd_is_procfs(صحیح fd)
{
	ساختار statfs st;
	صحیح ret;

	انجامبده {
		errno = 0;
		ret = fstatfs(fd, &st);

		اگر (ret < 0) {
			اگر (errno != EINTR && errno != EAGAIN)
				بازگشت 0;
			xusleep(250000);
		}
	} وقتی (ret != 0);

	بازگشت F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#وگرنه
صحیح fd_is_procfs(صحیح fd __attribute__((__unused__)))
{
	بازگشت 0;
}
#endif

ایستا نویسه *strdup_procfs_file(pid_t pid, ثابت نویسه *name)
{
	نویسه buf[BUFSIZ];
	نویسه *re = پوچ;
	صحیح fd;

	snprintf(buf, اندازه(buf), _PATH_PROC "/%d/%s", (صحیح) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	اگر (fd < 0)
		بازگشت پوچ;

	اگر (read_procfs_file(پوچ, fd, buf, اندازه(buf)) > 0)
		re = strdup(buf);
	close(fd);
	بازگشت re;
}

نویسه *pid_get_cmdname(pid_t pid)
{
	بازگشت strdup_procfs_file(pid, "comm");
}

نویسه *pid_get_cmdline(pid_t pid)
{
	بازگشت strdup_procfs_file(pid, "cmdline");
}

نویسه *pid_get_personality(pid_t pid)
{
	بازگشت strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

ایستا صحیح test_tasks(صحیح argc, نویسه *argv[], ثابت نویسه *prefix)
{
	DIR *sub = پوچ;
	ساختار path_cxt *pc;
	pid_t tid = 0, pid;

	اگر (argc != 2)
		بازگشت EXIT_FAILURE;

	pid = strtol(argv[1], (نویسه **) پوچ, 10);
	printf("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	اگر (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	وقتی (procfs_process_next_tid(pc, &sub, &tid) == 0)
		printf(" %d", tid);

	printf("\n");
	ul_unref_path(pc);
	بازگشت EXIT_SUCCESS;
}

ایستا صحیح test_fds(صحیح argc, نویسه *argv[], ثابت نویسه *prefix)
{
	DIR *sub = پوچ;
	ساختار path_cxt *pc;
	pid_t pid;
	صحیح fd = -1;

	اگر (argc != 2)
		بازگشت EXIT_FAILURE;

	pid = strtol(argv[1], (نویسه **) پوچ, 10);
	printf("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	اگر (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	وقتی (procfs_process_next_fd(pc, &sub, &fd) == 0)
		printf(" %d", fd);

	fputc('\n', stdout);
	ul_unref_path(pc);
	بازگشت EXIT_SUCCESS;
}

ایستا صحیح test_processes(صحیح argc, نویسه *argv[])
{
	DIR *dir;
	ساختار dirent *d;
	نویسه *name = پوچ;
	uid_t uid = (uid_t) -1;
	نویسه buf[128];

	اگر (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	اگر (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(_PATH_PROC);
	اگر (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	وقتی ((d = xreaddir(dir))) {
		pid_t pid = 0;

		اگر (procfs_dirent_get_pid(d, &pid) != 0)
			ادامه;
		اگر (name && !procfs_dirent_match_name(dir, d, name))
			ادامه;
		اگر (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			ادامه;
		procfs_dirent_get_name(dir, d, buf, اندازه(buf));
		printf(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	بازگشت EXIT_SUCCESS;
}

ایستا صحیح test_one_process(صحیح argc, نویسه *argv[], ثابت نویسه *prefix)
{
	pid_t pid;
	ساختار path_cxt *pc;
	نویسه buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	اگر (argc != 2)
		بازگشت EXIT_FAILURE;
	pid = strtol(argv[1], (نویسه **) پوچ, 10);

	pc = ul_new_procfs_path(pid, prefix);
	اگر (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	printf("%d\n", (صحیح) pid);

	procfs_process_get_uid(pc, &uid);
	printf("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, اندازه(buf));
	printf("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, اندازه(buf));
	printf("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	بازگشت EXIT_SUCCESS;
}

ایستا صحیح test_isprocfs(صحیح argc, نویسه *argv[])
{
	ثابت نویسه *name = argc > 1 ? argv[1] : "/proc";
	صحیح fd = open(name, O_RDONLY | O_CLOEXEC);
	صحیح is = 0;

	اگر (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} وگرنه
		err(EXIT_FAILURE, "cannot open %s", name);

	printf("%s: %s procfs\n", name, is ? "is" : "is NOT");
	بازگشت is ? EXIT_SUCCESS : EXIT_FAILURE;
}

ایستا صحیح test_process_stat_nth(صحیح argc, نویسه *argv[], ثابت نویسه *prefix)
{
	pid_t pid;
	ساختار path_cxt *pc;
	uint64_t رقم = 0;
	صحیح n, ret;

	اگر (argc != 3)
		بازگشت EXIT_FAILURE;
	pid = strtol(argv[1], (نویسه **) پوچ, 10);
	n = strtol(argv[2], (نویسه **) پوچ, 10);

	pc = ul_new_procfs_path(pid, prefix);
	اگر (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &رقم);
	اگر (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	printf("%d: %dth %"PRIu64"\n", (صحیح) pid, n, رقم);
	ul_unref_path(pc);
	بازگشت EXIT_SUCCESS;
}

صحیح اصلی(صحیح argc, نویسه *argv[])
{
	ثابت نویسه *prefix = پوچ;

	اگر (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	اگر (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		بازگشت EXIT_FAILURE;
	}

	اگر (strcmp(argv[1], "--tasks") == 0)
		بازگشت test_tasks(argc - 1, argv + 1, prefix);
	اگر (strcmp(argv[1], "--fds") == 0)
		بازگشت test_fds(argc - 1, argv + 1, prefix);
	اگر (strcmp(argv[1], "--processes") == 0)
		بازگشت test_processes(argc - 1, argv + 1);
	اگر (strcmp(argv[1], "--is-procfs") == 0)
		بازگشت test_isprocfs(argc - 1, argv + 1);
	اگر (strcmp(argv[1], "--one") == 0)
		بازگشت test_one_process(argc - 1, argv + 1, prefix);
	اگر (strcmp(argv[1], "--stat-nth") == 0)
		بازگشت test_process_stat_nth(argc - 1, argv + 1, prefix);

	بازگشت EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_PROCUTILS */
