/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2021]
 */
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <errno.h>

#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "pathnames.h"
#অন্তর্ভাব "procfs.h"
#অন্তর্ভাব "fileutils.h"
#অন্তর্ভাব "all-io.h"
#অন্তর্ভাব "vfs.h"
#অন্তর্ভাব "debug.h"
#অন্তর্ভাব "strutils.h"
#অন্তর্ভাব "statfs_magic.h"

স্থির শূন্য procfs_process_deinit_path(কাঠামো path_cxt *pc);

/*
 * Debug stuff (based on include/debug.h)
 */
স্থির UL_DEBUG_DEFINE_MASK(ulprocfs);
UL_DEBUG_DEFINE_MASKNAMES(ulprocfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPROCFS_DEBUG_INIT	(1 << 1)
#define ULPROCFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulprocfs, ULPROCFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulprocfs, ULPROCFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulprocfs, ULPROCFS_DEBUG_, m, x)

শূন্য ul_procfs_init_debug(শূন্য)
{
	যদি (ulprocfs_debug_mask)
		ফেরত;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

কাঠামো path_cxt *ul_new_procfs_path(pid_t pid, ধ্রুবক অক্ষর *prefix)
{
	কাঠামো path_cxt *pc = ul_new_path(ফাঁকা);

	যদি (!pc)
		ফেরত ফাঁকা;
	যদি (prefix)
		ul_path_set_prefix(pc, prefix);

	যদি (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		ফেরত ফাঁকা;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ফেরত pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
পূর্ণসংখ্যা procfs_process_init_path(কাঠামো path_cxt *pc, pid_t pid)
{
	কাঠামো procfs_process *prc;
	পূর্ণসংখ্যা rc;
	অক্ষর buf[আকার(_PATH_PROC) + আকার(stringify_মান(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, আকার(buf), _PATH_PROC "/%zu", (size_t) pid);
	rc = ul_path_set_dir(pc, buf);
	যদি (rc)
		ফেরত rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	যদি (rc < 0)
		ফেরত rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	যদি (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = calloc(1, আকার(কাঠামো procfs_process));
		যদি (!prc)
			ফেরত -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	ফেরত 0;
}

স্থির শূন্য procfs_process_deinit_path(কাঠামো path_cxt *pc)
{
	কাঠামো procfs_process *prc;

	যদি (!pc)
		ফেরত;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	যদি (!prc)
		ফেরত;

	মুক্ত(prc);
	ul_path_set_dialect(pc, ফাঁকা, ফাঁকা);
}

স্থির ssize_t read_procfs_file(ধ্রুবক কাঠামো ul_vfs_ops *vfs, পূর্ণসংখ্যা fd,
			       অক্ষর *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	যদি (fd < 0)
		ফেরত -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	যদি (sz <= 0)
		ফেরত sz;

	জন্য (i = 0; i < (size_t) sz; i++) {
		যদি (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	ফেরত sz;
}

স্থির ssize_t procfs_process_get_তথ্য_for(কাঠামো path_cxt *pc, অক্ষর *buf, size_t bufsz,
					    ধ্রুবক অক্ষর *fname)
{
	পূর্ণসংখ্যা fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	যদি (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : ফাঁকা, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : ফাঁকা, fd);
		ফেরত sz;
	}
	ফেরত -errno;
}

ssize_t procfs_process_get_cmdline(কাঠামো path_cxt *pc, অক্ষর *buf, size_t bufsz)
{
	ফেরত procfs_process_get_তথ্য_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(কাঠামো path_cxt *pc, অক্ষর *buf, size_t bufsz)
{
	ফেরত procfs_process_get_তথ্য_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(কাঠামো path_cxt *pc, অক্ষর *buf, size_t bufsz)
{
	ফেরত procfs_process_get_তথ্য_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(কাঠামো path_cxt *pc, অক্ষর *buf, size_t bufsz)
{
	ফেরত procfs_process_get_তথ্য_for(pc, buf, bufsz, "syscall");
}

পূর্ণসংখ্যা procfs_process_get_stat_nth(কাঠামো path_cxt *pc, পূর্ণসংখ্যা n, uint64_t *re)
{
	ssize_t rc;
	অক্ষর *চাবি = ফাঁকা, *tok, *p;
	অক্ষর buf[BUFSIZ];
	পূর্ণসংখ্যা i;

	যদি (n == 2 || n == 3)		/* process name and status (strings) */
		ফেরত -EINVAL;

	rc = procfs_process_get_তথ্য_for(pc, buf, আকার(buf), "stat");
	যদি (rc < 0)
		ফেরত rc;

	জন্য (i = 0, tok = strtok_r(buf, " ", &চাবি); tok;
	     tok = strtok_r(ফাঁকা, " ", &চাবি)) {

		i++;
		যদি (i == n)
			ফেরত ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		যদি (i == 2 && (p = strrchr(চাবি, ')')))
			চাবি = p + 2;
	}

	ফেরত -EINVAL;
}

পূর্ণসংখ্যা procfs_process_get_uid(কাঠামো path_cxt *pc, uid_t *uid)
{
	কাঠামো stat sb;
	পূর্ণসংখ্যা rc;

	যদি ((rc = ul_path_stat(pc, &sb, 0, ফাঁকা)) == 0)
		*uid = sb.st_uid;
	ফেরত rc;
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
পূর্ণসংখ্যা procfs_process_পরবর্তী_tid(কাঠামো path_cxt *pc, DIR **sub, pid_t *tid)
{
	কাঠামো dirent *d;

	যদি (!pc || !sub || !tid)
		ফেরত -EINVAL;

	যদি (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		যদি (!*sub)
			ফেরত -errno;
	}

	যখন ((d = xreaddir(*sub))) {
		যদি (procfs_dirent_get_pid(d, tid) == 0)
			ফেরত 0;
	}

	closedir(*sub);
	*sub = ফাঁকা;
	ফেরত 1;
}

পূর্ণসংখ্যা procfs_process_পরবর্তী_fd(কাঠামো path_cxt *pc, DIR **sub, পূর্ণসংখ্যা *fd)
{
	কাঠামো dirent *d;

	যদি (!pc || !sub || !fd)
		ফেরত -EINVAL;

	যদি (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		যদি (!*sub)
			ফেরত -errno;
	}

	যখন ((d = xreaddir(*sub))) {
		uint64_t অঙ্ক;
#ifdef _DIRENT_HAVE_D_TYPE
		যদি (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			চালু;
#endif
		যদি (ul_strtou64(d->d_name, &অঙ্ক, 10) < 0)
			চালু;
		*fd = অঙ্ক;
		ফেরত 0;
	}

	closedir(*sub);
	*sub = ফাঁকা;
	ফেরত 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
পূর্ণসংখ্যা procfs_dirent_is_process(কাঠামো dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	যদি (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		ফেরত 0;
#endif
	যদি (!isdigit((unsigned অক্ষর) *d->d_name))
		ফেরত 0;

	ফেরত 1;
}

পূর্ণসংখ্যা procfs_dirent_get_pid(কাঠামো dirent *d, pid_t *pid)
{
	uint64_t অঙ্ক;

	যদি (!procfs_dirent_is_process(d))
		ফেরত -EINVAL;

	যদি (ul_strtou64(d->d_name, &অঙ্ক, 10) < 0)
		ফেরত -EINVAL;

	*pid = (pid_t) অঙ্ক;
	ফেরত 0;
}

পূর্ণসংখ্যা procfs_dirent_get_uid(DIR *procfs, কাঠামো dirent *d, uid_t *uid)
{
	কাঠামো stat st;

	যদি (!procfs_dirent_is_process(d))
		ফেরত -EINVAL;

	যদি (fstatat(dirfd(procfs), d->d_name, &st, 0))
		ফেরত -EINVAL;

	*uid = st.st_uid;
	ফেরত 0;
}

পূর্ণসংখ্যা procfs_dirent_match_uid(DIR *procfs, কাঠামো dirent *d, uid_t uid)
{
	uid_t x;

	যদি (procfs_dirent_get_uid(procfs, d, &x) == 0)
		ফেরত x == uid;

	ফেরত 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
পূর্ণসংখ্যা procfs_dirent_get_name(DIR *procfs, কাঠামো dirent *d, অক্ষর *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	অক্ষর tmp[1024], *p, *end = ফাঁকা;

	যদি (bufsz < 32)
		ফেরত -EINVAL;
	যদি (!procfs_dirent_is_process(d))
		ফেরত -EINVAL;

	snprintf(tmp, আকার(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	যদি (!f)
		ফেরত -errno;

	p = fgets(tmp, আকার(tmp), f);
	fclose(f);
	যদি (!p)
		ফেরত -errno;

	/* skip PID */
	যখন (*p && *p != '(')
		p++;

	/* skip extra '(' */
	যখন (*p && *p == '(')
		p++;

	end = p;
	যখন (*end && *end != ')')
		end++;

	sz = end - p;
	যদি (sz >= bufsz)
		sz = bufsz - 1;

	memcpy(buf, p, sz);
	buf[sz] = '\0';

	ফেরত 0;
}

পূর্ণসংখ্যা procfs_dirent_match_name(DIR *procfs, কাঠামো dirent *d, ধ্রুবক অক্ষর *name)
{
	অক্ষর buf[33];

	যদি (procfs_dirent_get_name(procfs, d, buf, আকার(buf)) == 0)
		ফেরত strcmp(name, buf) == 0;

	ফেরত 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
পূর্ণসংখ্যা fd_is_procfs(পূর্ণসংখ্যা fd)
{
	কাঠামো statfs st;
	পূর্ণসংখ্যা ret;

	করো {
		errno = 0;
		ret = fstatfs(fd, &st);

		যদি (ret < 0) {
			যদি (errno != EINTR && errno != EAGAIN)
				ফেরত 0;
			xusleep(250000);
		}
	} যখন (ret != 0);

	ফেরত F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#অন্যথা
পূর্ণসংখ্যা fd_is_procfs(পূর্ণসংখ্যা fd __attribute__((__unused__)))
{
	ফেরত 0;
}
#endif

স্থির অক্ষর *strdup_procfs_file(pid_t pid, ধ্রুবক অক্ষর *name)
{
	অক্ষর buf[BUFSIZ];
	অক্ষর *re = ফাঁকা;
	পূর্ণসংখ্যা fd;

	snprintf(buf, আকার(buf), _PATH_PROC "/%d/%s", (পূর্ণসংখ্যা) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	যদি (fd < 0)
		ফেরত ফাঁকা;

	যদি (read_procfs_file(ফাঁকা, fd, buf, আকার(buf)) > 0)
		re = strdup(buf);
	close(fd);
	ফেরত re;
}

অক্ষর *pid_get_cmdname(pid_t pid)
{
	ফেরত strdup_procfs_file(pid, "comm");
}

অক্ষর *pid_get_cmdline(pid_t pid)
{
	ফেরত strdup_procfs_file(pid, "cmdline");
}

অক্ষর *pid_get_personality(pid_t pid)
{
	ফেরত strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

স্থির পূর্ণসংখ্যা test_tasks(পূর্ণসংখ্যা argc, অক্ষর *argv[], ধ্রুবক অক্ষর *prefix)
{
	DIR *sub = ফাঁকা;
	কাঠামো path_cxt *pc;
	pid_t tid = 0, pid;

	যদি (argc != 2)
		ফেরত EXIT_FAILURE;

	pid = strtol(argv[1], (অক্ষর **) ফাঁকা, 10);
	printf("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	যদি (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	যখন (procfs_process_পরবর্তী_tid(pc, &sub, &tid) == 0)
		printf(" %d", tid);

	printf("\n");
	ul_unref_path(pc);
	ফেরত EXIT_SUCCESS;
}

স্থির পূর্ণসংখ্যা test_fds(পূর্ণসংখ্যা argc, অক্ষর *argv[], ধ্রুবক অক্ষর *prefix)
{
	DIR *sub = ফাঁকা;
	কাঠামো path_cxt *pc;
	pid_t pid;
	পূর্ণসংখ্যা fd = -1;

	যদি (argc != 2)
		ফেরত EXIT_FAILURE;

	pid = strtol(argv[1], (অক্ষর **) ফাঁকা, 10);
	printf("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	যদি (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	যখন (procfs_process_পরবর্তী_fd(pc, &sub, &fd) == 0)
		printf(" %d", fd);

	fputc('\n', stdout);
	ul_unref_path(pc);
	ফেরত EXIT_SUCCESS;
}

স্থির পূর্ণসংখ্যা test_processes(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	DIR *dir;
	কাঠামো dirent *d;
	অক্ষর *name = ফাঁকা;
	uid_t uid = (uid_t) -1;
	অক্ষর buf[128];

	যদি (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	যদি (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(_PATH_PROC);
	যদি (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	যখন ((d = xreaddir(dir))) {
		pid_t pid = 0;

		যদি (procfs_dirent_get_pid(d, &pid) != 0)
			চালু;
		যদি (name && !procfs_dirent_match_name(dir, d, name))
			চালু;
		যদি (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			চালু;
		procfs_dirent_get_name(dir, d, buf, আকার(buf));
		printf(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	ফেরত EXIT_SUCCESS;
}

স্থির পূর্ণসংখ্যা test_one_process(পূর্ণসংখ্যা argc, অক্ষর *argv[], ধ্রুবক অক্ষর *prefix)
{
	pid_t pid;
	কাঠামো path_cxt *pc;
	অক্ষর buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	যদি (argc != 2)
		ফেরত EXIT_FAILURE;
	pid = strtol(argv[1], (অক্ষর **) ফাঁকা, 10);

	pc = ul_new_procfs_path(pid, prefix);
	যদি (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	printf("%d\n", (পূর্ণসংখ্যা) pid);

	procfs_process_get_uid(pc, &uid);
	printf("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, আকার(buf));
	printf("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, আকার(buf));
	printf("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	ফেরত EXIT_SUCCESS;
}

স্থির পূর্ণসংখ্যা test_isprocfs(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	ধ্রুবক অক্ষর *name = argc > 1 ? argv[1] : "/proc";
	পূর্ণসংখ্যা fd = open(name, O_RDONLY | O_CLOEXEC);
	পূর্ণসংখ্যা is = 0;

	যদি (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} অন্যথা
		err(EXIT_FAILURE, "cannot open %s", name);

	printf("%s: %s procfs\n", name, is ? "is" : "is NOT");
	ফেরত is ? EXIT_SUCCESS : EXIT_FAILURE;
}

স্থির পূর্ণসংখ্যা test_process_stat_nth(পূর্ণসংখ্যা argc, অক্ষর *argv[], ধ্রুবক অক্ষর *prefix)
{
	pid_t pid;
	কাঠামো path_cxt *pc;
	uint64_t অঙ্ক = 0;
	পূর্ণসংখ্যা n, ret;

	যদি (argc != 3)
		ফেরত EXIT_FAILURE;
	pid = strtol(argv[1], (অক্ষর **) ফাঁকা, 10);
	n = strtol(argv[2], (অক্ষর **) ফাঁকা, 10);

	pc = ul_new_procfs_path(pid, prefix);
	যদি (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &অঙ্ক);
	যদি (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	printf("%d: %dth %"PRIu64"\n", (পূর্ণসংখ্যা) pid, n, অঙ্ক);
	ul_unref_path(pc);
	ফেরত EXIT_SUCCESS;
}

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	ধ্রুবক অক্ষর *prefix = ফাঁকা;

	যদি (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	যদি (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		ফেরত EXIT_FAILURE;
	}

	যদি (strcmp(argv[1], "--tasks") == 0)
		ফেরত test_tasks(argc - 1, argv + 1, prefix);
	যদি (strcmp(argv[1], "--fds") == 0)
		ফেরত test_fds(argc - 1, argv + 1, prefix);
	যদি (strcmp(argv[1], "--processes") == 0)
		ফেরত test_processes(argc - 1, argv + 1);
	যদি (strcmp(argv[1], "--is-procfs") == 0)
		ফেরত test_isprocfs(argc - 1, argv + 1);
	যদি (strcmp(argv[1], "--one") == 0)
		ফেরত test_one_process(argc - 1, argv + 1, prefix);
	যদি (strcmp(argv[1], "--stat-nth") == 0)
		ফেরত test_process_stat_nth(argc - 1, argv + 1, prefix);

	ফেরত EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_PROCUTILS */
