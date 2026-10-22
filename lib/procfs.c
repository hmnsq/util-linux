/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2021]
 */
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <errno.h>

#నిర్వచితమైతే HAVE_SYS_VFS_H
# include <sys/vfs.h>
#అయితే_ముగింపు

#అన్తర్భావ "c.h"
#అన్తర్భావ "pathnames.h"
#అన్తర్భావ "procfs.h"
#అన్తర్భావ "fileutils.h"
#అన్తర్భావ "all-io.h"
#అన్తర్భావ "vfs.h"
#అన్తర్భావ "debug.h"
#అన్తర్భావ "strutils.h"
#అన్తర్భావ "statfs_magic.h"

స్థిర శూన్య procfs_process_deinit_మార్గం(నిర్మాణం మార్గం_cxt *pc);

/*
 * Debug stuff (based on include/debug.h)
 */
స్థిర UL_DEBUG_DEFINE_ముసుగు(ulprocfs);
UL_DEBUG_DEFINE_MASKNAMES(ulprocfs) = UL_DEBUG_ఖాళీలు_MASKNAMES;

#నిర్వచించు ULPROCFS_DEBUG_INIT	(1 << 1)
#నిర్వచించు ULPROCFS_DEBUG_CXT	(1 << 2)

#నిర్వచించు DBG(m, x)		__UL_DBG(ulprocfs, ULPROCFS_DEBUG_, m, x)
#నిర్వచించు DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulprocfs, ULPROCFS_DEBUG_, m, h, x)
#నిర్వచించు ON_DBG(m, x)		__UL_DBG_CALL(ulprocfs, ULPROCFS_DEBUG_, m, x)

శూన్య ul_procfs_init_debug(శూన్య)
{
	ఒకవేళ (ulprocfs_debug_ముసుగు)
		తిరిగి;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

నిర్మాణం మార్గం_cxt *ul_new_procfs_మార్గం(pid_t pid, స్థిరరాశి అక్షరం *ఉపసర్గ)
{
	నిర్మాణం మార్గం_cxt *pc = ul_new_మార్గం(ఖాళీ);

	ఒకవేళ (!pc)
		తిరిగి ఖాళీ;
	ఒకవేళ (ఉపసర్గ)
		ul_మార్గం_సమితి_ఉపసర్గ(pc, ఉపసర్గ);

	ఒకవేళ (procfs_process_init_మార్గం(pc, pid) != 0) {
		ul_unref_మార్గం(pc);
		తిరిగి ఖాళీ;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	తిరిగి pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
పూర్ణాంకం procfs_process_init_మార్గం(నిర్మాణం మార్గం_cxt *pc, pid_t pid)
{
	నిర్మాణం procfs_process *prc;
	పూర్ణాంకం rc;
	అక్షరం buf[పరిమాణం(మార్గం_PROC) + పరిమాణం(stringify_విలువ(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, పరిమాణం(buf), మార్గం_PROC "/%zu", (size_t) pid);
	rc = ul_మార్గం_సమితి_dir(pc, buf);
	ఒకవేళ (rc)
		తిరిగి rc;

	/* make sure path exists */
	rc = ul_మార్గం_get_dirfd(pc);
	ఒకవేళ (rc < 0)
		తిరిగి rc;

	/* initialize procfs specific stuff */
	prc = ul_మార్గం_get_dialect(pc);
	ఒకవేళ (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం procfs_process));
		ఒకవేళ (!prc)
			తిరిగి -ENOMEM;

		ul_మార్గం_సమితి_dialect(pc, prc, procfs_process_deinit_మార్గం);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	తిరిగి 0;
}

స్థిర శూన్య procfs_process_deinit_మార్గం(నిర్మాణం మార్గం_cxt *pc)
{
	నిర్మాణం procfs_process *prc;

	ఒకవేళ (!pc)
		తిరిగి;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_మార్గం_get_dialect(pc);
	ఒకవేళ (!prc)
		తిరిగి;

	విడుదల(prc);
	ul_మార్గం_సమితి_dialect(pc, ఖాళీ, ఖాళీ);
}

స్థిర ssize_t read_procfs_file(స్థిరరాశి నిర్మాణం ul_vfs_ops *vfs, పూర్ణాంకం fd,
			       అక్షరం *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	ఒకవేళ (fd < 0)
		తిరిగి -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	ఒకవేళ (sz <= 0)
		తిరిగి sz;

	కొరకు (i = 0; i < (size_t) sz; i++) {
		ఒకవేళ (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	తిరిగి sz;
}

స్థిర ssize_t procfs_process_get_దత్తాంశం_for(నిర్మాణం మార్గం_cxt *pc, అక్షరం *buf, size_t bufsz,
					    స్థిరరాశి అక్షరం *fname)
{
	పూర్ణాంకం fd = ul_మార్గం_open(pc, O_RDONLY|O_CLOEXEC, fname);

	ఒకవేళ (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : ఖాళీ, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : ఖాళీ, fd);
		తిరిగి sz;
	}
	తిరిగి -errno;
}

ssize_t procfs_process_get_cmdline(నిర్మాణం మార్గం_cxt *pc, అక్షరం *buf, size_t bufsz)
{
	తిరిగి procfs_process_get_దత్తాంశం_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(నిర్మాణం మార్గం_cxt *pc, అక్షరం *buf, size_t bufsz)
{
	తిరిగి procfs_process_get_దత్తాంశం_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(నిర్మాణం మార్గం_cxt *pc, అక్షరం *buf, size_t bufsz)
{
	తిరిగి procfs_process_get_దత్తాంశం_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(నిర్మాణం మార్గం_cxt *pc, అక్షరం *buf, size_t bufsz)
{
	తిరిగి procfs_process_get_దత్తాంశం_for(pc, buf, bufsz, "syscall");
}

పూర్ణాంకం procfs_process_get_stat_nth(నిర్మాణం మార్గం_cxt *pc, పూర్ణాంకం n, uint64_t *re)
{
	ssize_t rc;
	అక్షరం *కీలకం = ఖాళీ, *tok, *p;
	అక్షరం buf[BUFSIZ];
	పూర్ణాంకం i;

	ఒకవేళ (n == 2 || n == 3)		/* process name and status (strings) */
		తిరిగి -EINVAL;

	rc = procfs_process_get_దత్తాంశం_for(pc, buf, పరిమాణం(buf), "stat");
	ఒకవేళ (rc < 0)
		తిరిగి rc;

	కొరకు (i = 0, tok = strtok_r(buf, " ", &కీలకం); tok;
	     tok = strtok_r(ఖాళీ, " ", &కీలకం)) {

		i++;
		ఒకవేళ (i == n)
			తిరిగి ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		ఒకవేళ (i == 2 && (p = strrchr(కీలకం, ')')))
			కీలకం = p + 2;
	}

	తిరిగి -EINVAL;
}

పూర్ణాంకం procfs_process_get_uid(నిర్మాణం మార్గం_cxt *pc, uid_t *uid)
{
	నిర్మాణం stat sb;
	పూర్ణాంకం rc;

	ఒకవేళ ((rc = ul_మార్గం_stat(pc, &sb, 0, ఖాళీ)) == 0)
		*uid = sb.st_uid;
	తిరిగి rc;
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
పూర్ణాంకం procfs_process_తదుపరి_tid(నిర్మాణం మార్గం_cxt *pc, DIR **sub, pid_t *tid)
{
	నిర్మాణం dirent *d;

	ఒకవేళ (!pc || !sub || !tid)
		తిరిగి -EINVAL;

	ఒకవేళ (!*sub) {
		*sub = ul_మార్గం_opendir(pc, "task");
		ఒకవేళ (!*sub)
			తిరిగి -errno;
	}

	అయితే ((d = xreaddir(*sub))) {
		ఒకవేళ (procfs_dirent_get_pid(d, tid) == 0)
			తిరిగి 0;
	}

	closedir(*sub);
	*sub = ఖాళీ;
	తిరిగి 1;
}

పూర్ణాంకం procfs_process_తదుపరి_fd(నిర్మాణం మార్గం_cxt *pc, DIR **sub, పూర్ణాంకం *fd)
{
	నిర్మాణం dirent *d;

	ఒకవేళ (!pc || !sub || !fd)
		తిరిగి -EINVAL;

	ఒకవేళ (!*sub) {
		*sub = ul_మార్గం_opendir(pc, "fd");
		ఒకవేళ (!*sub)
			తిరిగి -errno;
	}

	అయితే ((d = xreaddir(*sub))) {
		uint64_t అంకె;
#నిర్వచితమైతే _DIRENT_HAVE_D_TYPE
		ఒకవేళ (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			కొనసాగు;
#అయితే_ముగింపు
		ఒకవేళ (ul_strtou64(d->d_name, &అంకె, 10) < 0)
			కొనసాగు;
		*fd = అంకె;
		తిరిగి 0;
	}

	closedir(*sub);
	*sub = ఖాళీ;
	తిరిగి 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
పూర్ణాంకం procfs_dirent_is_process(నిర్మాణం dirent *d)
{
#నిర్వచితమైతే _DIRENT_HAVE_D_TYPE
	ఒకవేళ (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		తిరిగి 0;
#అయితే_ముగింపు
	ఒకవేళ (!అంకెనా((గుర్తులేని అక్షరం) *d->d_name))
		తిరిగి 0;

	తిరిగి 1;
}

పూర్ణాంకం procfs_dirent_get_pid(నిర్మాణం dirent *d, pid_t *pid)
{
	uint64_t అంకె;

	ఒకవేళ (!procfs_dirent_is_process(d))
		తిరిగి -EINVAL;

	ఒకవేళ (ul_strtou64(d->d_name, &అంకె, 10) < 0)
		తిరిగి -EINVAL;

	*pid = (pid_t) అంకె;
	తిరిగి 0;
}

పూర్ణాంకం procfs_dirent_get_uid(DIR *procfs, నిర్మాణం dirent *d, uid_t *uid)
{
	నిర్మాణం stat st;

	ఒకవేళ (!procfs_dirent_is_process(d))
		తిరిగి -EINVAL;

	ఒకవేళ (fstatat(dirfd(procfs), d->d_name, &st, 0))
		తిరిగి -EINVAL;

	*uid = st.st_uid;
	తిరిగి 0;
}

పూర్ణాంకం procfs_dirent_సరిపోలిక_uid(DIR *procfs, నిర్మాణం dirent *d, uid_t uid)
{
	uid_t x;

	ఒకవేళ (procfs_dirent_get_uid(procfs, d, &x) == 0)
		తిరిగి x == uid;

	తిరిగి 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
పూర్ణాంకం procfs_dirent_get_name(DIR *procfs, నిర్మాణం dirent *d, అక్షరం *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	అక్షరం tmp[1024], *p, *చివర = ఖాళీ;

	ఒకవేళ (bufsz < 32)
		తిరిగి -EINVAL;
	ఒకవేళ (!procfs_dirent_is_process(d))
		తిరిగి -EINVAL;

	snprintf(tmp, పరిమాణం(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	ఒకవేళ (!f)
		తిరిగి -errno;

	p = fgets(tmp, పరిమాణం(tmp), f);
	fclose(f);
	ఒకవేళ (!p)
		తిరిగి -errno;

	/* skip PID */
	అయితే (*p && *p != '(')
		p++;

	/* skip extra '(' */
	అయితే (*p && *p == '(')
		p++;

	చివర = p;
	అయితే (*చివర && *చివర != ')')
		చివర++;

	sz = చివర - p;
	ఒకవేళ (sz >= bufsz)
		sz = bufsz - 1;

	జ్ఞాపకం_నకలు(buf, p, sz);
	buf[sz] = '\0';

	తిరిగి 0;
}

పూర్ణాంకం procfs_dirent_సరిపోలిక_name(DIR *procfs, నిర్మాణం dirent *d, స్థిరరాశి అక్షరం *name)
{
	అక్షరం buf[33];

	ఒకవేళ (procfs_dirent_get_name(procfs, d, buf, పరిమాణం(buf)) == 0)
		తిరిగి strcmp(name, buf) == 0;

	తిరిగి 0;
}

#నిర్వచితమైతే HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
పూర్ణాంకం fd_is_procfs(పూర్ణాంకం fd)
{
	నిర్మాణం statfs st;
	పూర్ణాంకం ret;

	చేయి {
		errno = 0;
		ret = fstatfs(fd, &st);

		ఒకవేళ (ret < 0) {
			ఒకవేళ (errno != EINTR && errno != EAGAIN)
				తిరిగి 0;
			xusleep(250000);
		}
	} అయితే (ret != 0);

	తిరిగి F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#లేకపోతే
పూర్ణాంకం fd_is_procfs(పూర్ణాంకం fd __attribute__((__unused__)))
{
	తిరిగి 0;
}
#అయితే_ముగింపు

స్థిర అక్షరం *strdup_procfs_file(pid_t pid, స్థిరరాశి అక్షరం *name)
{
	అక్షరం buf[BUFSIZ];
	అక్షరం *re = ఖాళీ;
	పూర్ణాంకం fd;

	snprintf(buf, పరిమాణం(buf), మార్గం_PROC "/%d/%s", (పూర్ణాంకం) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	ఒకవేళ (fd < 0)
		తిరిగి ఖాళీ;

	ఒకవేళ (read_procfs_file(ఖాళీ, fd, buf, పరిమాణం(buf)) > 0)
		re = strdup(buf);
	close(fd);
	తిరిగి re;
}

అక్షరం *pid_get_cmdname(pid_t pid)
{
	తిరిగి strdup_procfs_file(pid, "comm");
}

అక్షరం *pid_get_cmdline(pid_t pid)
{
	తిరిగి strdup_procfs_file(pid, "cmdline");
}

అక్షరం *pid_get_personality(pid_t pid)
{
	తిరిగి strdup_procfs_file(pid, "personality");
}

#నిర్వచితమైతే TEST_కార్యక్రమం_PROCFS

స్థిర పూర్ణాంకం test_tasks(పూర్ణాంకం argc, అక్షరం *argv[], స్థిరరాశి అక్షరం *ఉపసర్గ)
{
	DIR *sub = ఖాళీ;
	నిర్మాణం మార్గం_cxt *pc;
	pid_t tid = 0, pid;

	ఒకవేళ (argc != 2)
		తిరిగి EXIT_FAILURE;

	pid = strtol(argv[1], (అక్షరం **) ఖాళీ, 10);
	రాయి("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_మార్గం(pid, ఉపసర్గ);
	ఒకవేళ (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	అయితే (procfs_process_తదుపరి_tid(pc, &sub, &tid) == 0)
		రాయి(" %d", tid);

	రాయి("\n");
	ul_unref_మార్గం(pc);
	తిరిగి EXIT_SUCCESS;
}

స్థిర పూర్ణాంకం test_fds(పూర్ణాంకం argc, అక్షరం *argv[], స్థిరరాశి అక్షరం *ఉపసర్గ)
{
	DIR *sub = ఖాళీ;
	నిర్మాణం మార్గం_cxt *pc;
	pid_t pid;
	పూర్ణాంకం fd = -1;

	ఒకవేళ (argc != 2)
		తిరిగి EXIT_FAILURE;

	pid = strtol(argv[1], (అక్షరం **) ఖాళీ, 10);
	రాయి("PID=%d, FDs:", pid);

	pc = ul_new_procfs_మార్గం(pid, ఉపసర్గ);
	ఒకవేళ (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	అయితే (procfs_process_తదుపరి_fd(pc, &sub, &fd) == 0)
		రాయి(" %d", fd);

	fputc('\n', stdout);
	ul_unref_మార్గం(pc);
	తిరిగి EXIT_SUCCESS;
}

స్థిర పూర్ణాంకం test_processes(పూర్ణాంకం argc, అక్షరం *argv[])
{
	DIR *dir;
	నిర్మాణం dirent *d;
	అక్షరం *name = ఖాళీ;
	uid_t uid = (uid_t) -1;
	అక్షరం buf[128];

	ఒకవేళ (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	ఒకవేళ (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(మార్గం_PROC);
	ఒకవేళ (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	అయితే ((d = xreaddir(dir))) {
		pid_t pid = 0;

		ఒకవేళ (procfs_dirent_get_pid(d, &pid) != 0)
			కొనసాగు;
		ఒకవేళ (name && !procfs_dirent_సరిపోలిక_name(dir, d, name))
			కొనసాగు;
		ఒకవేళ (uid != (uid_t) -1 && !procfs_dirent_సరిపోలిక_uid(dir, d, uid))
			కొనసాగు;
		procfs_dirent_get_name(dir, d, buf, పరిమాణం(buf));
		రాయి(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	తిరిగి EXIT_SUCCESS;
}

స్థిర పూర్ణాంకం test_one_process(పూర్ణాంకం argc, అక్షరం *argv[], స్థిరరాశి అక్షరం *ఉపసర్గ)
{
	pid_t pid;
	నిర్మాణం మార్గం_cxt *pc;
	అక్షరం buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	ఒకవేళ (argc != 2)
		తిరిగి EXIT_FAILURE;
	pid = strtol(argv[1], (అక్షరం **) ఖాళీ, 10);

	pc = ul_new_procfs_మార్గం(pid, ఉపసర్గ);
	ఒకవేళ (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	రాయి("%d\n", (పూర్ణాంకం) pid);

	procfs_process_get_uid(pc, &uid);
	రాయి("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, పరిమాణం(buf));
	రాయి("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, పరిమాణం(buf));
	రాయి("   COMM: '%s'\n", buf);

	ul_unref_మార్గం(pc);
	తిరిగి EXIT_SUCCESS;
}

స్థిర పూర్ణాంకం test_isprocfs(పూర్ణాంకం argc, అక్షరం *argv[])
{
	స్థిరరాశి అక్షరం *name = argc > 1 ? argv[1] : "/proc";
	పూర్ణాంకం fd = open(name, O_RDONLY | O_CLOEXEC);
	పూర్ణాంకం is = 0;

	ఒకవేళ (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} లేకపోతే
		err(EXIT_FAILURE, "cannot open %s", name);

	రాయి("%s: %s procfs\n", name, is ? "is" : "is NOT");
	తిరిగి is ? EXIT_SUCCESS : EXIT_FAILURE;
}

స్థిర పూర్ణాంకం test_process_stat_nth(పూర్ణాంకం argc, అక్షరం *argv[], స్థిరరాశి అక్షరం *ఉపసర్గ)
{
	pid_t pid;
	నిర్మాణం మార్గం_cxt *pc;
	uint64_t అంకె = 0;
	పూర్ణాంకం n, ret;

	ఒకవేళ (argc != 3)
		తిరిగి EXIT_FAILURE;
	pid = strtol(argv[1], (అక్షరం **) ఖాళీ, 10);
	n = strtol(argv[2], (అక్షరం **) ఖాళీ, 10);

	pc = ul_new_procfs_మార్గం(pid, ఉపసర్గ);
	ఒకవేళ (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &అంకె);
	ఒకవేళ (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	రాయి("%d: %dth %"PRIu64"\n", (పూర్ణాంకం) pid, n, అంకె);
	ul_unref_మార్గం(pc);
	తిరిగి EXIT_SUCCESS;
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
	స్థిరరాశి అక్షరం *ఉపసర్గ = ఖాళీ;

	ఒకవేళ (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		ఉపసర్గ = argv[2];
		argc -= 2;
		argv += 2;
	}

	ఒకవేళ (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				కార్యక్రమం_invocation_short_name);
		తిరిగి EXIT_FAILURE;
	}

	ఒకవేళ (strcmp(argv[1], "--tasks") == 0)
		తిరిగి test_tasks(argc - 1, argv + 1, ఉపసర్గ);
	ఒకవేళ (strcmp(argv[1], "--fds") == 0)
		తిరిగి test_fds(argc - 1, argv + 1, ఉపసర్గ);
	ఒకవేళ (strcmp(argv[1], "--processes") == 0)
		తిరిగి test_processes(argc - 1, argv + 1);
	ఒకవేళ (strcmp(argv[1], "--is-procfs") == 0)
		తిరిగి test_isprocfs(argc - 1, argv + 1);
	ఒకవేళ (strcmp(argv[1], "--one") == 0)
		తిరిగి test_one_process(argc - 1, argv + 1, ఉపసర్గ);
	ఒకవేళ (strcmp(argv[1], "--stat-nth") == 0)
		తిరిగి test_process_stat_nth(argc - 1, argv + 1, ఉపసర్గ);

	తిరిగి EXIT_FAILURE;
}
#అయితే_ముగింపు /* TEST_PROGRAM_PROCUTILS */
