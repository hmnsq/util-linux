/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2021]
 */
#شامل <ctype.h>
#شامل <unistd.h>
#شامل <errno.h>

#اگر_تعریف_شدہ HAVE_SYS_VFS_H
# include <sys/vfs.h>
#اگر_ختم

#شامل "c.h"
#شامل "pathnames.h"
#شامل "procfs.h"
#شامل "fileutils.h"
#شامل "all-io.h"
#شامل "vfs.h"
#شامل "debug.h"
#شامل "strutils.h"
#شامل "statfs_magic.h"

ساکن خالی procfs_process_deinit_راستہ(ساخت راستہ_cxt *pc);

/*
 * Debug stuff (based on include/debug.h)
 */
ساکن UL_DEBUG_DEFINE_نقاب(ulprocfs);
UL_DEBUG_DEFINE_MASKNAMES(ulprocfs) = UL_DEBUG_خالی_خانے_MASKNAMES;

#تعریف_کرو ULPROCFS_DEBUG_INIT	(1 << 1)
#تعریف_کرو ULPROCFS_DEBUG_CXT	(1 << 2)

#تعریف_کرو DBG(m, x)		__UL_DBG(ulprocfs, ULPROCFS_DEBUG_, m, x)
#تعریف_کرو DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulprocfs, ULPROCFS_DEBUG_, m, h, x)
#تعریف_کرو ON_DBG(m, x)		__UL_DBG_CALL(ulprocfs, ULPROCFS_DEBUG_, m, x)

خالی ul_procfs_init_debug(خالی)
{
	اگر (ulprocfs_debug_نقاب)
		واپس;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

ساخت راستہ_cxt *ul_new_procfs_راستہ(pid_t pid, ثابت حرف *سابقہ)
{
	ساخت راستہ_cxt *pc = ul_new_راستہ(تہی);

	اگر (!pc)
		واپس تہی;
	اگر (سابقہ)
		ul_راستہ_سیٹ_سابقہ(pc, سابقہ);

	اگر (procfs_process_init_راستہ(pc, pid) != 0) {
		ul_unref_راستہ(pc);
		واپس تہی;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	واپس pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
صحیح procfs_process_init_راستہ(ساخت راستہ_cxt *pc, pid_t pid)
{
	ساخت procfs_process *prc;
	صحیح rc;
	حرف buf[حجم(راستہ_PROC) + حجم(stringify_قدر(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, حجم(buf), راستہ_PROC "/%zu", (size_t) pid);
	rc = ul_راستہ_سیٹ_dir(pc, buf);
	اگر (rc)
		واپس rc;

	/* make sure path exists */
	rc = ul_راستہ_get_dirfd(pc);
	اگر (rc < 0)
		واپس rc;

	/* initialize procfs specific stuff */
	prc = ul_راستہ_get_dialect(pc);
	اگر (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = صفر_حافظہ_دو(1, حجم(ساخت procfs_process));
		اگر (!prc)
			واپس -ENOMEM;

		ul_راستہ_سیٹ_dialect(pc, prc, procfs_process_deinit_راستہ);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	واپس 0;
}

ساکن خالی procfs_process_deinit_راستہ(ساخت راستہ_cxt *pc)
{
	ساخت procfs_process *prc;

	اگر (!pc)
		واپس;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_راستہ_get_dialect(pc);
	اگر (!prc)
		واپس;

	آزاد(prc);
	ul_راستہ_سیٹ_dialect(pc, تہی, تہی);
}

ساکن ssize_t read_procfs_file(ثابت ساخت ul_vfs_ops *vfs, صحیح fd,
			       حرف *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	اگر (fd < 0)
		واپس -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	اگر (sz <= 0)
		واپس sz;

	برائے (i = 0; i < (size_t) sz; i++) {
		اگر (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	واپس sz;
}

ساکن ssize_t procfs_process_get_ڈیٹا_for(ساخت راستہ_cxt *pc, حرف *buf, size_t bufsz,
					    ثابت حرف *fname)
{
	صحیح fd = ul_راستہ_open(pc, O_RDONLY|O_CLOEXEC, fname);

	اگر (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : تہی, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : تہی, fd);
		واپس sz;
	}
	واپس -errno;
}

ssize_t procfs_process_get_cmdline(ساخت راستہ_cxt *pc, حرف *buf, size_t bufsz)
{
	واپس procfs_process_get_ڈیٹا_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(ساخت راستہ_cxt *pc, حرف *buf, size_t bufsz)
{
	واپس procfs_process_get_ڈیٹا_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(ساخت راستہ_cxt *pc, حرف *buf, size_t bufsz)
{
	واپس procfs_process_get_ڈیٹا_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(ساخت راستہ_cxt *pc, حرف *buf, size_t bufsz)
{
	واپس procfs_process_get_ڈیٹا_for(pc, buf, bufsz, "syscall");
}

صحیح procfs_process_get_stat_nth(ساخت راستہ_cxt *pc, صحیح n, uint64_t *re)
{
	ssize_t rc;
	حرف *کلید = تہی, *tok, *p;
	حرف buf[BUFSIZ];
	صحیح i;

	اگر (n == 2 || n == 3)		/* process name and status (strings) */
		واپس -EINVAL;

	rc = procfs_process_get_ڈیٹا_for(pc, buf, حجم(buf), "stat");
	اگر (rc < 0)
		واپس rc;

	برائے (i = 0, tok = strtok_r(buf, " ", &کلید); tok;
	     tok = strtok_r(تہی, " ", &کلید)) {

		i++;
		اگر (i == n)
			واپس ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		اگر (i == 2 && (p = strrchr(کلید, ')')))
			کلید = p + 2;
	}

	واپس -EINVAL;
}

صحیح procfs_process_get_uid(ساخت راستہ_cxt *pc, uid_t *uid)
{
	ساخت stat sb;
	صحیح rc;

	اگر ((rc = ul_راستہ_stat(pc, &sb, 0, تہی)) == 0)
		*uid = sb.st_uid;
	واپس rc;
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
صحیح procfs_process_اگلا_tid(ساخت راستہ_cxt *pc, DIR **sub, pid_t *tid)
{
	ساخت dirent *d;

	اگر (!pc || !sub || !tid)
		واپس -EINVAL;

	اگر (!*sub) {
		*sub = ul_راستہ_opendir(pc, "task");
		اگر (!*sub)
			واپس -errno;
	}

	جب ((d = xreaddir(*sub))) {
		اگر (procfs_dirent_get_pid(d, tid) == 0)
			واپس 0;
	}

	closedir(*sub);
	*sub = تہی;
	واپس 1;
}

صحیح procfs_process_اگلا_fd(ساخت راستہ_cxt *pc, DIR **sub, صحیح *fd)
{
	ساخت dirent *d;

	اگر (!pc || !sub || !fd)
		واپس -EINVAL;

	اگر (!*sub) {
		*sub = ul_راستہ_opendir(pc, "fd");
		اگر (!*sub)
			واپس -errno;
	}

	جب ((d = xreaddir(*sub))) {
		uint64_t عدد;
#اگر_تعریف_شدہ _DIRENT_HAVE_D_TYPE
		اگر (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			جاری;
#اگر_ختم
		اگر (ul_strtou64(d->d_name, &عدد, 10) < 0)
			جاری;
		*fd = عدد;
		واپس 0;
	}

	closedir(*sub);
	*sub = تہی;
	واپس 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
صحیح procfs_dirent_is_process(ساخت dirent *d)
{
#اگر_تعریف_شدہ _DIRENT_HAVE_D_TYPE
	اگر (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		واپس 0;
#اگر_ختم
	اگر (!ہندسہ_ہے((غیرعلامتی حرف) *d->d_name))
		واپس 0;

	واپس 1;
}

صحیح procfs_dirent_get_pid(ساخت dirent *d, pid_t *pid)
{
	uint64_t عدد;

	اگر (!procfs_dirent_is_process(d))
		واپس -EINVAL;

	اگر (ul_strtou64(d->d_name, &عدد, 10) < 0)
		واپس -EINVAL;

	*pid = (pid_t) عدد;
	واپس 0;
}

صحیح procfs_dirent_get_uid(DIR *procfs, ساخت dirent *d, uid_t *uid)
{
	ساخت stat st;

	اگر (!procfs_dirent_is_process(d))
		واپس -EINVAL;

	اگر (fstatat(dirfd(procfs), d->d_name, &st, 0))
		واپس -EINVAL;

	*uid = st.st_uid;
	واپس 0;
}

صحیح procfs_dirent_مطابقت_uid(DIR *procfs, ساخت dirent *d, uid_t uid)
{
	uid_t x;

	اگر (procfs_dirent_get_uid(procfs, d, &x) == 0)
		واپس x == uid;

	واپس 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
صحیح procfs_dirent_get_name(DIR *procfs, ساخت dirent *d, حرف *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	حرف tmp[1024], *p, *انتہا = تہی;

	اگر (bufsz < 32)
		واپس -EINVAL;
	اگر (!procfs_dirent_is_process(d))
		واپس -EINVAL;

	snprintf(tmp, حجم(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	اگر (!f)
		واپس -errno;

	p = fgets(tmp, حجم(tmp), f);
	fclose(f);
	اگر (!p)
		واپس -errno;

	/* skip PID */
	جب (*p && *p != '(')
		p++;

	/* skip extra '(' */
	جب (*p && *p == '(')
		p++;

	انتہا = p;
	جب (*انتہا && *انتہا != ')')
		انتہا++;

	sz = انتہا - p;
	اگر (sz >= bufsz)
		sz = bufsz - 1;

	حافظہ_نقل(buf, p, sz);
	buf[sz] = '\0';

	واپس 0;
}

صحیح procfs_dirent_مطابقت_name(DIR *procfs, ساخت dirent *d, ثابت حرف *name)
{
	حرف buf[33];

	اگر (procfs_dirent_get_name(procfs, d, buf, حجم(buf)) == 0)
		واپس strcmp(name, buf) == 0;

	واپس 0;
}

#اگر_تعریف_شدہ HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
صحیح fd_is_procfs(صحیح fd)
{
	ساخت statfs st;
	صحیح ret;

	کرو {
		errno = 0;
		ret = fstatfs(fd, &st);

		اگر (ret < 0) {
			اگر (errno != EINTR && errno != EAGAIN)
				واپس 0;
			xusleep(250000);
		}
	} جب (ret != 0);

	واپس F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#ورنہ
صحیح fd_is_procfs(صحیح fd __attribute__((__unused__)))
{
	واپس 0;
}
#اگر_ختم

ساکن حرف *strdup_procfs_file(pid_t pid, ثابت حرف *name)
{
	حرف buf[BUFSIZ];
	حرف *re = تہی;
	صحیح fd;

	snprintf(buf, حجم(buf), راستہ_PROC "/%d/%s", (صحیح) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	اگر (fd < 0)
		واپس تہی;

	اگر (read_procfs_file(تہی, fd, buf, حجم(buf)) > 0)
		re = strdup(buf);
	close(fd);
	واپس re;
}

حرف *pid_get_cmdname(pid_t pid)
{
	واپس strdup_procfs_file(pid, "comm");
}

حرف *pid_get_cmdline(pid_t pid)
{
	واپس strdup_procfs_file(pid, "cmdline");
}

حرف *pid_get_personality(pid_t pid)
{
	واپس strdup_procfs_file(pid, "personality");
}

#اگر_تعریف_شدہ TEST_پروگرام_PROCFS

ساکن صحیح test_tasks(صحیح argc, حرف *argv[], ثابت حرف *سابقہ)
{
	DIR *sub = تہی;
	ساخت راستہ_cxt *pc;
	pid_t tid = 0, pid;

	اگر (argc != 2)
		واپس EXIT_FAILURE;

	pid = strtol(argv[1], (حرف **) تہی, 10);
	لکھو("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_راستہ(pid, سابقہ);
	اگر (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	جب (procfs_process_اگلا_tid(pc, &sub, &tid) == 0)
		لکھو(" %d", tid);

	لکھو("\n");
	ul_unref_راستہ(pc);
	واپس EXIT_SUCCESS;
}

ساکن صحیح test_fds(صحیح argc, حرف *argv[], ثابت حرف *سابقہ)
{
	DIR *sub = تہی;
	ساخت راستہ_cxt *pc;
	pid_t pid;
	صحیح fd = -1;

	اگر (argc != 2)
		واپس EXIT_FAILURE;

	pid = strtol(argv[1], (حرف **) تہی, 10);
	لکھو("PID=%d, FDs:", pid);

	pc = ul_new_procfs_راستہ(pid, سابقہ);
	اگر (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	جب (procfs_process_اگلا_fd(pc, &sub, &fd) == 0)
		لکھو(" %d", fd);

	fputc('\n', stdout);
	ul_unref_راستہ(pc);
	واپس EXIT_SUCCESS;
}

ساکن صحیح test_processes(صحیح argc, حرف *argv[])
{
	DIR *dir;
	ساخت dirent *d;
	حرف *name = تہی;
	uid_t uid = (uid_t) -1;
	حرف buf[128];

	اگر (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	اگر (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(راستہ_PROC);
	اگر (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	جب ((d = xreaddir(dir))) {
		pid_t pid = 0;

		اگر (procfs_dirent_get_pid(d, &pid) != 0)
			جاری;
		اگر (name && !procfs_dirent_مطابقت_name(dir, d, name))
			جاری;
		اگر (uid != (uid_t) -1 && !procfs_dirent_مطابقت_uid(dir, d, uid))
			جاری;
		procfs_dirent_get_name(dir, d, buf, حجم(buf));
		لکھو(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	واپس EXIT_SUCCESS;
}

ساکن صحیح test_one_process(صحیح argc, حرف *argv[], ثابت حرف *سابقہ)
{
	pid_t pid;
	ساخت راستہ_cxt *pc;
	حرف buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	اگر (argc != 2)
		واپس EXIT_FAILURE;
	pid = strtol(argv[1], (حرف **) تہی, 10);

	pc = ul_new_procfs_راستہ(pid, سابقہ);
	اگر (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	لکھو("%d\n", (صحیح) pid);

	procfs_process_get_uid(pc, &uid);
	لکھو("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, حجم(buf));
	لکھو("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, حجم(buf));
	لکھو("   COMM: '%s'\n", buf);

	ul_unref_راستہ(pc);
	واپس EXIT_SUCCESS;
}

ساکن صحیح test_isprocfs(صحیح argc, حرف *argv[])
{
	ثابت حرف *name = argc > 1 ? argv[1] : "/proc";
	صحیح fd = open(name, O_RDONLY | O_CLOEXEC);
	صحیح is = 0;

	اگر (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} ورنہ
		err(EXIT_FAILURE, "cannot open %s", name);

	لکھو("%s: %s procfs\n", name, is ? "is" : "is NOT");
	واپس is ? EXIT_SUCCESS : EXIT_FAILURE;
}

ساکن صحیح test_process_stat_nth(صحیح argc, حرف *argv[], ثابت حرف *سابقہ)
{
	pid_t pid;
	ساخت راستہ_cxt *pc;
	uint64_t عدد = 0;
	صحیح n, ret;

	اگر (argc != 3)
		واپس EXIT_FAILURE;
	pid = strtol(argv[1], (حرف **) تہی, 10);
	n = strtol(argv[2], (حرف **) تہی, 10);

	pc = ul_new_procfs_راستہ(pid, سابقہ);
	اگر (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &عدد);
	اگر (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	لکھو("%d: %dth %"PRIu64"\n", (صحیح) pid, n, عدد);
	ul_unref_راستہ(pc);
	واپس EXIT_SUCCESS;
}

صحیح مین(صحیح argc, حرف *argv[])
{
	ثابت حرف *سابقہ = تہی;

	اگر (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		سابقہ = argv[2];
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
				پروگرام_invocation_short_name);
		واپس EXIT_FAILURE;
	}

	اگر (strcmp(argv[1], "--tasks") == 0)
		واپس test_tasks(argc - 1, argv + 1, سابقہ);
	اگر (strcmp(argv[1], "--fds") == 0)
		واپس test_fds(argc - 1, argv + 1, سابقہ);
	اگر (strcmp(argv[1], "--processes") == 0)
		واپس test_processes(argc - 1, argv + 1);
	اگر (strcmp(argv[1], "--is-procfs") == 0)
		واپس test_isprocfs(argc - 1, argv + 1);
	اگر (strcmp(argv[1], "--one") == 0)
		واپس test_one_process(argc - 1, argv + 1, سابقہ);
	اگر (strcmp(argv[1], "--stat-nth") == 0)
		واپس test_process_stat_nth(argc - 1, argv + 1, سابقہ);

	واپس EXIT_FAILURE;
}
#اگر_ختم /* TEST_PROGRAM_PROCUTILS */
