/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2021]
 */
#ضمن <ctype.h>
#ضمن <unistd.h>
#ضمن <errno.h>

#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif

#ضمن "c.h"
#ضمن "pathnames.h"
#ضمن "procfs.h"
#ضمن "fileutils.h"
#ضمن "all-io.h"
#ضمن "vfs.h"
#ضمن "debug.h"
#ضمن "strutils.h"
#ضمن "statfs_magic.h"

ساكن فراغ procfs_process_deinit_path(بنية path_cxt *pc);

/*
 * Debug stuff (based on include/debug.h)
 */
ساكن UL_DEBUG_DEFINE_MASK(ulprocfs);
UL_DEBUG_DEFINE_MASKNAMES(ulprocfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPROCFS_DEBUG_INIT	(1 << 1)
#define ULPROCFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulprocfs, ULPROCFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulprocfs, ULPROCFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulprocfs, ULPROCFS_DEBUG_, m, x)

فراغ ul_procfs_init_debug(فراغ)
{
	إذا (ulprocfs_debug_mask)
		إرجاع;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

بنية path_cxt *ul_new_procfs_path(pid_t pid, ثابت حرف *prefix)
{
	بنية path_cxt *pc = ul_new_path(عدم);

	إذا (!pc)
		إرجاع عدم;
	إذا (prefix)
		ul_path_set_prefix(pc, prefix);

	إذا (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		إرجاع عدم;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	إرجاع pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
صحيح procfs_process_init_path(بنية path_cxt *pc, pid_t pid)
{
	بنية procfs_process *prc;
	صحيح rc;
	حرف buf[حجم(_PATH_PROC) + حجم(stringify_قيمة(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, حجم(buf), _PATH_PROC "/%zu", (size_t) pid);
	rc = ul_path_set_dir(pc, buf);
	إذا (rc)
		إرجاع rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	إذا (rc < 0)
		إرجاع rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	إذا (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = calloc(1, حجم(بنية procfs_process));
		إذا (!prc)
			إرجاع -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	إرجاع 0;
}

ساكن فراغ procfs_process_deinit_path(بنية path_cxt *pc)
{
	بنية procfs_process *prc;

	إذا (!pc)
		إرجاع;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	إذا (!prc)
		إرجاع;

	حرر(prc);
	ul_path_set_dialect(pc, عدم, عدم);
}

ساكن ssize_t read_procfs_file(ثابت بنية ul_vfs_ops *vfs, صحيح fd,
			       حرف *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	إذا (fd < 0)
		إرجاع -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	إذا (sz <= 0)
		إرجاع sz;

	لكل (i = 0; i < (size_t) sz; i++) {
		إذا (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	إرجاع sz;
}

ساكن ssize_t procfs_process_get_data_for(بنية path_cxt *pc, حرف *buf, size_t bufsz,
					    ثابت حرف *fname)
{
	صحيح fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	إذا (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : عدم, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : عدم, fd);
		إرجاع sz;
	}
	إرجاع -errno;
}

ssize_t procfs_process_get_cmdline(بنية path_cxt *pc, حرف *buf, size_t bufsz)
{
	إرجاع procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(بنية path_cxt *pc, حرف *buf, size_t bufsz)
{
	إرجاع procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(بنية path_cxt *pc, حرف *buf, size_t bufsz)
{
	إرجاع procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(بنية path_cxt *pc, حرف *buf, size_t bufsz)
{
	إرجاع procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

صحيح procfs_process_get_stat_nth(بنية path_cxt *pc, صحيح n, uint64_t *re)
{
	ssize_t rc;
	حرف *مفتاح = عدم, *tok, *p;
	حرف buf[BUFSIZ];
	صحيح i;

	إذا (n == 2 || n == 3)		/* process name and status (strings) */
		إرجاع -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, حجم(buf), "stat");
	إذا (rc < 0)
		إرجاع rc;

	لكل (i = 0, tok = strtok_r(buf, " ", &مفتاح); tok;
	     tok = strtok_r(عدم, " ", &مفتاح)) {

		i++;
		إذا (i == n)
			إرجاع ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		إذا (i == 2 && (p = strrchr(مفتاح, ')')))
			مفتاح = p + 2;
	}

	إرجاع -EINVAL;
}

صحيح procfs_process_get_uid(بنية path_cxt *pc, uid_t *uid)
{
	بنية stat sb;
	صحيح rc;

	إذا ((rc = ul_path_stat(pc, &sb, 0, عدم)) == 0)
		*uid = sb.st_uid;
	إرجاع rc;
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
صحيح procfs_process_next_tid(بنية path_cxt *pc, DIR **sub, pid_t *tid)
{
	بنية dirent *d;

	إذا (!pc || !sub || !tid)
		إرجاع -EINVAL;

	إذا (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		إذا (!*sub)
			إرجاع -errno;
	}

	بينما ((d = xreaddir(*sub))) {
		إذا (procfs_dirent_get_pid(d, tid) == 0)
			إرجاع 0;
	}

	closedir(*sub);
	*sub = عدم;
	إرجاع 1;
}

صحيح procfs_process_next_fd(بنية path_cxt *pc, DIR **sub, صحيح *fd)
{
	بنية dirent *d;

	إذا (!pc || !sub || !fd)
		إرجاع -EINVAL;

	إذا (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		إذا (!*sub)
			إرجاع -errno;
	}

	بينما ((d = xreaddir(*sub))) {
		uint64_t رقم;
#ifdef _DIRENT_HAVE_D_TYPE
		إذا (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			استمر;
#endif
		إذا (ul_strtou64(d->d_name, &رقم, 10) < 0)
			استمر;
		*fd = رقم;
		إرجاع 0;
	}

	closedir(*sub);
	*sub = عدم;
	إرجاع 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
صحيح procfs_dirent_is_process(بنية dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	إذا (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		إرجاع 0;
#endif
	إذا (!isdigit((unsigned حرف) *d->d_name))
		إرجاع 0;

	إرجاع 1;
}

صحيح procfs_dirent_get_pid(بنية dirent *d, pid_t *pid)
{
	uint64_t رقم;

	إذا (!procfs_dirent_is_process(d))
		إرجاع -EINVAL;

	إذا (ul_strtou64(d->d_name, &رقم, 10) < 0)
		إرجاع -EINVAL;

	*pid = (pid_t) رقم;
	إرجاع 0;
}

صحيح procfs_dirent_get_uid(DIR *procfs, بنية dirent *d, uid_t *uid)
{
	بنية stat st;

	إذا (!procfs_dirent_is_process(d))
		إرجاع -EINVAL;

	إذا (fstatat(dirfd(procfs), d->d_name, &st, 0))
		إرجاع -EINVAL;

	*uid = st.st_uid;
	إرجاع 0;
}

صحيح procfs_dirent_match_uid(DIR *procfs, بنية dirent *d, uid_t uid)
{
	uid_t x;

	إذا (procfs_dirent_get_uid(procfs, d, &x) == 0)
		إرجاع x == uid;

	إرجاع 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
صحيح procfs_dirent_get_name(DIR *procfs, بنية dirent *d, حرف *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	حرف tmp[1024], *p, *end = عدم;

	إذا (bufsz < 32)
		إرجاع -EINVAL;
	إذا (!procfs_dirent_is_process(d))
		إرجاع -EINVAL;

	snprintf(tmp, حجم(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	إذا (!f)
		إرجاع -errno;

	p = fgets(tmp, حجم(tmp), f);
	fclose(f);
	إذا (!p)
		إرجاع -errno;

	/* skip PID */
	بينما (*p && *p != '(')
		p++;

	/* skip extra '(' */
	بينما (*p && *p == '(')
		p++;

	end = p;
	بينما (*end && *end != ')')
		end++;

	sz = end - p;
	إذا (sz >= bufsz)
		sz = bufsz - 1;

	memcpy(buf, p, sz);
	buf[sz] = '\0';

	إرجاع 0;
}

صحيح procfs_dirent_match_name(DIR *procfs, بنية dirent *d, ثابت حرف *name)
{
	حرف buf[33];

	إذا (procfs_dirent_get_name(procfs, d, buf, حجم(buf)) == 0)
		إرجاع strcmp(name, buf) == 0;

	إرجاع 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
صحيح fd_is_procfs(صحيح fd)
{
	بنية statfs st;
	صحيح ret;

	افعل {
		errno = 0;
		ret = fstatfs(fd, &st);

		إذا (ret < 0) {
			إذا (errno != EINTR && errno != EAGAIN)
				إرجاع 0;
			xusleep(250000);
		}
	} بينما (ret != 0);

	إرجاع F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#وإلا
صحيح fd_is_procfs(صحيح fd __attribute__((__unused__)))
{
	إرجاع 0;
}
#endif

ساكن حرف *strdup_procfs_file(pid_t pid, ثابت حرف *name)
{
	حرف buf[BUFSIZ];
	حرف *re = عدم;
	صحيح fd;

	snprintf(buf, حجم(buf), _PATH_PROC "/%d/%s", (صحيح) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	إذا (fd < 0)
		إرجاع عدم;

	إذا (read_procfs_file(عدم, fd, buf, حجم(buf)) > 0)
		re = strdup(buf);
	close(fd);
	إرجاع re;
}

حرف *pid_get_cmdname(pid_t pid)
{
	إرجاع strdup_procfs_file(pid, "comm");
}

حرف *pid_get_cmdline(pid_t pid)
{
	إرجاع strdup_procfs_file(pid, "cmdline");
}

حرف *pid_get_personality(pid_t pid)
{
	إرجاع strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

ساكن صحيح test_tasks(صحيح argc, حرف *argv[], ثابت حرف *prefix)
{
	DIR *sub = عدم;
	بنية path_cxt *pc;
	pid_t tid = 0, pid;

	إذا (argc != 2)
		إرجاع EXIT_FAILURE;

	pid = strtol(argv[1], (حرف **) عدم, 10);
	printf("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	إذا (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	بينما (procfs_process_next_tid(pc, &sub, &tid) == 0)
		printf(" %d", tid);

	printf("\n");
	ul_unref_path(pc);
	إرجاع EXIT_SUCCESS;
}

ساكن صحيح test_fds(صحيح argc, حرف *argv[], ثابت حرف *prefix)
{
	DIR *sub = عدم;
	بنية path_cxt *pc;
	pid_t pid;
	صحيح fd = -1;

	إذا (argc != 2)
		إرجاع EXIT_FAILURE;

	pid = strtol(argv[1], (حرف **) عدم, 10);
	printf("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	إذا (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	بينما (procfs_process_next_fd(pc, &sub, &fd) == 0)
		printf(" %d", fd);

	fputc('\n', stdout);
	ul_unref_path(pc);
	إرجاع EXIT_SUCCESS;
}

ساكن صحيح test_processes(صحيح argc, حرف *argv[])
{
	DIR *dir;
	بنية dirent *d;
	حرف *name = عدم;
	uid_t uid = (uid_t) -1;
	حرف buf[128];

	إذا (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	إذا (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(_PATH_PROC);
	إذا (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	بينما ((d = xreaddir(dir))) {
		pid_t pid = 0;

		إذا (procfs_dirent_get_pid(d, &pid) != 0)
			استمر;
		إذا (name && !procfs_dirent_match_name(dir, d, name))
			استمر;
		إذا (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			استمر;
		procfs_dirent_get_name(dir, d, buf, حجم(buf));
		printf(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	إرجاع EXIT_SUCCESS;
}

ساكن صحيح test_one_process(صحيح argc, حرف *argv[], ثابت حرف *prefix)
{
	pid_t pid;
	بنية path_cxt *pc;
	حرف buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	إذا (argc != 2)
		إرجاع EXIT_FAILURE;
	pid = strtol(argv[1], (حرف **) عدم, 10);

	pc = ul_new_procfs_path(pid, prefix);
	إذا (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	printf("%d\n", (صحيح) pid);

	procfs_process_get_uid(pc, &uid);
	printf("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, حجم(buf));
	printf("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, حجم(buf));
	printf("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	إرجاع EXIT_SUCCESS;
}

ساكن صحيح test_isprocfs(صحيح argc, حرف *argv[])
{
	ثابت حرف *name = argc > 1 ? argv[1] : "/proc";
	صحيح fd = open(name, O_RDONLY | O_CLOEXEC);
	صحيح is = 0;

	إذا (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} وإلا
		err(EXIT_FAILURE, "cannot open %s", name);

	printf("%s: %s procfs\n", name, is ? "is" : "is NOT");
	إرجاع is ? EXIT_SUCCESS : EXIT_FAILURE;
}

ساكن صحيح test_process_stat_nth(صحيح argc, حرف *argv[], ثابت حرف *prefix)
{
	pid_t pid;
	بنية path_cxt *pc;
	uint64_t رقم = 0;
	صحيح n, ret;

	إذا (argc != 3)
		إرجاع EXIT_FAILURE;
	pid = strtol(argv[1], (حرف **) عدم, 10);
	n = strtol(argv[2], (حرف **) عدم, 10);

	pc = ul_new_procfs_path(pid, prefix);
	إذا (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &رقم);
	إذا (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	printf("%d: %dth %"PRIu64"\n", (صحيح) pid, n, رقم);
	ul_unref_path(pc);
	إرجاع EXIT_SUCCESS;
}

صحيح الرئيسية(صحيح argc, حرف *argv[])
{
	ثابت حرف *prefix = عدم;

	إذا (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	إذا (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		إرجاع EXIT_FAILURE;
	}

	إذا (strcmp(argv[1], "--tasks") == 0)
		إرجاع test_tasks(argc - 1, argv + 1, prefix);
	إذا (strcmp(argv[1], "--fds") == 0)
		إرجاع test_fds(argc - 1, argv + 1, prefix);
	إذا (strcmp(argv[1], "--processes") == 0)
		إرجاع test_processes(argc - 1, argv + 1);
	إذا (strcmp(argv[1], "--is-procfs") == 0)
		إرجاع test_isprocfs(argc - 1, argv + 1);
	إذا (strcmp(argv[1], "--one") == 0)
		إرجاع test_one_process(argc - 1, argv + 1, prefix);
	إذا (strcmp(argv[1], "--stat-nth") == 0)
		إرجاع test_process_stat_nth(argc - 1, argv + 1, prefix);

	إرجاع EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_PROCUTILS */
