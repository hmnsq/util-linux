/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2021]
 */
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <errno.h>

#यदि_परिभाषितम् HAVE_SYS_VFS_H
# include <sys/vfs.h>
#यदि_अन्तः

#अन्तर्भाव "c.h"
#अन्तर्भाव "pathnames.h"
#अन्तर्भाव "procfs.h"
#अन्तर्भाव "fileutils.h"
#अन्तर्भाव "all-io.h"
#अन्तर्भाव "vfs.h"
#अन्तर्भाव "debug.h"
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "statfs_magic.h"

स्थिर शून्य procfs_process_deinit_मार्गः(संरचना मार्गः_cxt *pc);

/*
 * Debug stuff (based on include/debug.h)
 */
स्थिर UL_DEBUG_DEFINE_आवरणम्(ulprocfs);
UL_DEBUG_DEFINE_MASKNAMES(ulprocfs) = UL_DEBUG_रिक्तानि_MASKNAMES;

#परिभाषय ULPROCFS_DEBUG_INIT	(1 << 1)
#परिभाषय ULPROCFS_DEBUG_CXT	(1 << 2)

#परिभाषय DBG(m, x)		__UL_DBG(ulprocfs, ULPROCFS_DEBUG_, m, x)
#परिभाषय DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulprocfs, ULPROCFS_DEBUG_, m, h, x)
#परिभाषय ON_DBG(m, x)		__UL_DBG_CALL(ulprocfs, ULPROCFS_DEBUG_, m, x)

शून्य ul_procfs_init_debug(शून्य)
{
	यदि (ulprocfs_debug_आवरणम्)
		प्रत्यागम;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

संरचना मार्गः_cxt *ul_new_procfs_मार्गः(pid_t pid, ध्रुव वर्ण *उपसर्गः)
{
	संरचना मार्गः_cxt *pc = ul_new_मार्गः(रिक्त);

	यदि (!pc)
		प्रत्यागम रिक्त;
	यदि (उपसर्गः)
		ul_मार्गः_समुच्चयः_उपसर्गः(pc, उपसर्गः);

	यदि (procfs_process_init_मार्गः(pc, pid) != 0) {
		ul_unref_मार्गः(pc);
		प्रत्यागम रिक्त;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	प्रत्यागम pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
पूर्णाङ्क procfs_process_init_मार्गः(संरचना मार्गः_cxt *pc, pid_t pid)
{
	संरचना procfs_process *prc;
	पूर्णाङ्क rc;
	वर्ण buf[परिमाण(मार्गः_PROC) + परिमाण(stringify_मानम्(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, परिमाण(buf), मार्गः_PROC "/%zu", (size_t) pid);
	rc = ul_मार्गः_समुच्चयः_dir(pc, buf);
	यदि (rc)
		प्रत्यागम rc;

	/* make sure path exists */
	rc = ul_मार्गः_get_dirfd(pc);
	यदि (rc < 0)
		प्रत्यागम rc;

	/* initialize procfs specific stuff */
	prc = ul_मार्गः_get_dialect(pc);
	यदि (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = शून्यस्मृतिं_देहि(1, परिमाण(संरचना procfs_process));
		यदि (!prc)
			प्रत्यागम -ENOMEM;

		ul_मार्गः_समुच्चयः_dialect(pc, prc, procfs_process_deinit_मार्गः);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	प्रत्यागम 0;
}

स्थिर शून्य procfs_process_deinit_मार्गः(संरचना मार्गः_cxt *pc)
{
	संरचना procfs_process *prc;

	यदि (!pc)
		प्रत्यागम;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_मार्गः_get_dialect(pc);
	यदि (!prc)
		प्रत्यागम;

	मुक्त(prc);
	ul_मार्गः_समुच्चयः_dialect(pc, रिक्त, रिक्त);
}

स्थिर ssize_t read_procfs_file(ध्रुव संरचना ul_vfs_ops *vfs, पूर्णाङ्क fd,
			       वर्ण *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	यदि (fd < 0)
		प्रत्यागम -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	यदि (sz <= 0)
		प्रत्यागम sz;

	प्रति (i = 0; i < (size_t) sz; i++) {
		यदि (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	प्रत्यागम sz;
}

स्थिर ssize_t procfs_process_get_दत्त_for(संरचना मार्गः_cxt *pc, वर्ण *buf, size_t bufsz,
					    ध्रुव वर्ण *fname)
{
	पूर्णाङ्क fd = ul_मार्गः_open(pc, O_RDONLY|O_CLOEXEC, fname);

	यदि (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : रिक्त, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : रिक्त, fd);
		प्रत्यागम sz;
	}
	प्रत्यागम -errno;
}

ssize_t procfs_process_get_cmdline(संरचना मार्गः_cxt *pc, वर्ण *buf, size_t bufsz)
{
	प्रत्यागम procfs_process_get_दत्त_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(संरचना मार्गः_cxt *pc, वर्ण *buf, size_t bufsz)
{
	प्रत्यागम procfs_process_get_दत्त_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(संरचना मार्गः_cxt *pc, वर्ण *buf, size_t bufsz)
{
	प्रत्यागम procfs_process_get_दत्त_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(संरचना मार्गः_cxt *pc, वर्ण *buf, size_t bufsz)
{
	प्रत्यागम procfs_process_get_दत्त_for(pc, buf, bufsz, "syscall");
}

पूर्णाङ्क procfs_process_get_stat_nth(संरचना मार्गः_cxt *pc, पूर्णाङ्क n, uint64_t *re)
{
	ssize_t rc;
	वर्ण *कुञ्चिका = रिक्त, *tok, *p;
	वर्ण buf[BUFSIZ];
	पूर्णाङ्क i;

	यदि (n == 2 || n == 3)		/* process name and status (strings) */
		प्रत्यागम -EINVAL;

	rc = procfs_process_get_दत्त_for(pc, buf, परिमाण(buf), "stat");
	यदि (rc < 0)
		प्रत्यागम rc;

	प्रति (i = 0, tok = strtok_r(buf, " ", &कुञ्चिका); tok;
	     tok = strtok_r(रिक्त, " ", &कुञ्चिका)) {

		i++;
		यदि (i == n)
			प्रत्यागम ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		यदि (i == 2 && (p = strrchr(कुञ्चिका, ')')))
			कुञ्चिका = p + 2;
	}

	प्रत्यागम -EINVAL;
}

पूर्णाङ्क procfs_process_get_uid(संरचना मार्गः_cxt *pc, uid_t *uid)
{
	संरचना stat sb;
	पूर्णाङ्क rc;

	यदि ((rc = ul_मार्गः_stat(pc, &sb, 0, रिक्त)) == 0)
		*uid = sb.st_uid;
	प्रत्यागम rc;
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
पूर्णाङ्क procfs_process_अग्र_tid(संरचना मार्गः_cxt *pc, DIR **sub, pid_t *tid)
{
	संरचना dirent *d;

	यदि (!pc || !sub || !tid)
		प्रत्यागम -EINVAL;

	यदि (!*sub) {
		*sub = ul_मार्गः_opendir(pc, "task");
		यदि (!*sub)
			प्रत्यागम -errno;
	}

	यावत् ((d = xreaddir(*sub))) {
		यदि (procfs_dirent_get_pid(d, tid) == 0)
			प्रत्यागम 0;
	}

	closedir(*sub);
	*sub = रिक्त;
	प्रत्यागम 1;
}

पूर्णाङ्क procfs_process_अग्र_fd(संरचना मार्गः_cxt *pc, DIR **sub, पूर्णाङ्क *fd)
{
	संरचना dirent *d;

	यदि (!pc || !sub || !fd)
		प्रत्यागम -EINVAL;

	यदि (!*sub) {
		*sub = ul_मार्गः_opendir(pc, "fd");
		यदि (!*sub)
			प्रत्यागम -errno;
	}

	यावत् ((d = xreaddir(*sub))) {
		uint64_t अङ्कः;
#यदि_परिभाषितम् _DIRENT_HAVE_D_TYPE
		यदि (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			अनुवर्त;
#यदि_अन्तः
		यदि (ul_strtou64(d->d_name, &अङ्कः, 10) < 0)
			अनुवर्त;
		*fd = अङ्कः;
		प्रत्यागम 0;
	}

	closedir(*sub);
	*sub = रिक्त;
	प्रत्यागम 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
पूर्णाङ्क procfs_dirent_is_process(संरचना dirent *d)
{
#यदि_परिभाषितम् _DIRENT_HAVE_D_TYPE
	यदि (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		प्रत्यागम 0;
#यदि_अन्तः
	यदि (!अङ्कः_वा((अचिह्नित वर्ण) *d->d_name))
		प्रत्यागम 0;

	प्रत्यागम 1;
}

पूर्णाङ्क procfs_dirent_get_pid(संरचना dirent *d, pid_t *pid)
{
	uint64_t अङ्कः;

	यदि (!procfs_dirent_is_process(d))
		प्रत्यागम -EINVAL;

	यदि (ul_strtou64(d->d_name, &अङ्कः, 10) < 0)
		प्रत्यागम -EINVAL;

	*pid = (pid_t) अङ्कः;
	प्रत्यागम 0;
}

पूर्णाङ्क procfs_dirent_get_uid(DIR *procfs, संरचना dirent *d, uid_t *uid)
{
	संरचना stat st;

	यदि (!procfs_dirent_is_process(d))
		प्रत्यागम -EINVAL;

	यदि (fstatat(dirfd(procfs), d->d_name, &st, 0))
		प्रत्यागम -EINVAL;

	*uid = st.st_uid;
	प्रत्यागम 0;
}

पूर्णाङ्क procfs_dirent_मेलनम्_uid(DIR *procfs, संरचना dirent *d, uid_t uid)
{
	uid_t x;

	यदि (procfs_dirent_get_uid(procfs, d, &x) == 0)
		प्रत्यागम x == uid;

	प्रत्यागम 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
पूर्णाङ्क procfs_dirent_get_name(DIR *procfs, संरचना dirent *d, वर्ण *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	वर्ण tmp[1024], *p, *अन्तः = रिक्त;

	यदि (bufsz < 32)
		प्रत्यागम -EINVAL;
	यदि (!procfs_dirent_is_process(d))
		प्रत्यागम -EINVAL;

	snprintf(tmp, परिमाण(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	यदि (!f)
		प्रत्यागम -errno;

	p = fgets(tmp, परिमाण(tmp), f);
	fclose(f);
	यदि (!p)
		प्रत्यागम -errno;

	/* skip PID */
	यावत् (*p && *p != '(')
		p++;

	/* skip extra '(' */
	यावत् (*p && *p == '(')
		p++;

	अन्तः = p;
	यावत् (*अन्तः && *अन्तः != ')')
		अन्तः++;

	sz = अन्तः - p;
	यदि (sz >= bufsz)
		sz = bufsz - 1;

	स्मृतिं_प्रतिलिख(buf, p, sz);
	buf[sz] = '\0';

	प्रत्यागम 0;
}

पूर्णाङ्क procfs_dirent_मेलनम्_name(DIR *procfs, संरचना dirent *d, ध्रुव वर्ण *name)
{
	वर्ण buf[33];

	यदि (procfs_dirent_get_name(procfs, d, buf, परिमाण(buf)) == 0)
		प्रत्यागम वाक्यं_तुलय(name, buf) == 0;

	प्रत्यागम 0;
}

#यदि_परिभाषितम् HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
पूर्णाङ्क fd_is_procfs(पूर्णाङ्क fd)
{
	संरचना statfs st;
	पूर्णाङ्क ret;

	कुरु {
		errno = 0;
		ret = fstatfs(fd, &st);

		यदि (ret < 0) {
			यदि (errno != EINTR && errno != EAGAIN)
				प्रत्यागम 0;
			xusleep(250000);
		}
	} यावत् (ret != 0);

	प्रत्यागम F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#अन्यथा
पूर्णाङ्क fd_is_procfs(पूर्णाङ्क fd __attribute__((__unused__)))
{
	प्रत्यागम 0;
}
#यदि_अन्तः

स्थिर वर्ण *strdup_procfs_file(pid_t pid, ध्रुव वर्ण *name)
{
	वर्ण buf[BUFSIZ];
	वर्ण *re = रिक्त;
	पूर्णाङ्क fd;

	snprintf(buf, परिमाण(buf), मार्गः_PROC "/%d/%s", (पूर्णाङ्क) pid, name);
	fd = उद्घाटय(buf, O_CLOEXEC|O_RDONLY);
	यदि (fd < 0)
		प्रत्यागम रिक्त;

	यदि (read_procfs_file(रिक्त, fd, buf, परिमाण(buf)) > 0)
		re = strdup(buf);
	close(fd);
	प्रत्यागम re;
}

वर्ण *pid_get_cmdname(pid_t pid)
{
	प्रत्यागम strdup_procfs_file(pid, "comm");
}

वर्ण *pid_get_cmdline(pid_t pid)
{
	प्रत्यागम strdup_procfs_file(pid, "cmdline");
}

वर्ण *pid_get_personality(pid_t pid)
{
	प्रत्यागम strdup_procfs_file(pid, "personality");
}

#यदि_परिभाषितम् TEST_क्रमादेशः_PROCFS

स्थिर पूर्णाङ्क test_tasks(पूर्णाङ्क argc, वर्ण *argv[], ध्रुव वर्ण *उपसर्गः)
{
	DIR *sub = रिक्त;
	संरचना मार्गः_cxt *pc;
	pid_t tid = 0, pid;

	यदि (argc != 2)
		प्रत्यागम EXIT_FAILURE;

	pid = strtol(argv[1], (वर्ण **) रिक्त, 10);
	प्रारूप_लिख("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_मार्गः(pid, उपसर्गः);
	यदि (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	यावत् (procfs_process_अग्र_tid(pc, &sub, &tid) == 0)
		प्रारूप_लिख(" %d", tid);

	प्रारूप_लिख("\n");
	ul_unref_मार्गः(pc);
	प्रत्यागम EXIT_SUCCESS;
}

स्थिर पूर्णाङ्क test_fds(पूर्णाङ्क argc, वर्ण *argv[], ध्रुव वर्ण *उपसर्गः)
{
	DIR *sub = रिक्त;
	संरचना मार्गः_cxt *pc;
	pid_t pid;
	पूर्णाङ्क fd = -1;

	यदि (argc != 2)
		प्रत्यागम EXIT_FAILURE;

	pid = strtol(argv[1], (वर्ण **) रिक्त, 10);
	प्रारूप_लिख("PID=%d, FDs:", pid);

	pc = ul_new_procfs_मार्गः(pid, उपसर्गः);
	यदि (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	यावत् (procfs_process_अग्र_fd(pc, &sub, &fd) == 0)
		प्रारूप_लिख(" %d", fd);

	fputc('\n', stdout);
	ul_unref_मार्गः(pc);
	प्रत्यागम EXIT_SUCCESS;
}

स्थिर पूर्णाङ्क test_processes(पूर्णाङ्क argc, वर्ण *argv[])
{
	DIR *dir;
	संरचना dirent *d;
	वर्ण *name = रिक्त;
	uid_t uid = (uid_t) -1;
	वर्ण buf[128];

	यदि (argc >= 3 && वाक्यं_तुलय(argv[1], "--name") == 0)
		name = argv[2];
	यदि (argc >= 3 && वाक्यं_तुलय(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(मार्गः_PROC);
	यदि (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	यावत् ((d = xreaddir(dir))) {
		pid_t pid = 0;

		यदि (procfs_dirent_get_pid(d, &pid) != 0)
			अनुवर्त;
		यदि (name && !procfs_dirent_मेलनम्_name(dir, d, name))
			अनुवर्त;
		यदि (uid != (uid_t) -1 && !procfs_dirent_मेलनम्_uid(dir, d, uid))
			अनुवर्त;
		procfs_dirent_get_name(dir, d, buf, परिमाण(buf));
		प्रारूप_लिख(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	प्रत्यागम EXIT_SUCCESS;
}

स्थिर पूर्णाङ्क test_one_process(पूर्णाङ्क argc, वर्ण *argv[], ध्रुव वर्ण *उपसर्गः)
{
	pid_t pid;
	संरचना मार्गः_cxt *pc;
	वर्ण buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	यदि (argc != 2)
		प्रत्यागम EXIT_FAILURE;
	pid = strtol(argv[1], (वर्ण **) रिक्त, 10);

	pc = ul_new_procfs_मार्गः(pid, उपसर्गः);
	यदि (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	प्रारूप_लिख("%d\n", (पूर्णाङ्क) pid);

	procfs_process_get_uid(pc, &uid);
	प्रारूप_लिख("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, परिमाण(buf));
	प्रारूप_लिख("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, परिमाण(buf));
	प्रारूप_लिख("   COMM: '%s'\n", buf);

	ul_unref_मार्गः(pc);
	प्रत्यागम EXIT_SUCCESS;
}

स्थिर पूर्णाङ्क test_isprocfs(पूर्णाङ्क argc, वर्ण *argv[])
{
	ध्रुव वर्ण *name = argc > 1 ? argv[1] : "/proc";
	पूर्णाङ्क fd = उद्घाटय(name, O_RDONLY | O_CLOEXEC);
	पूर्णाङ्क is = 0;

	यदि (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} अन्यथा
		err(EXIT_FAILURE, "cannot open %s", name);

	प्रारूप_लिख("%s: %s procfs\n", name, is ? "is" : "is NOT");
	प्रत्यागम is ? EXIT_SUCCESS : EXIT_FAILURE;
}

स्थिर पूर्णाङ्क test_process_stat_nth(पूर्णाङ्क argc, वर्ण *argv[], ध्रुव वर्ण *उपसर्गः)
{
	pid_t pid;
	संरचना मार्गः_cxt *pc;
	uint64_t अङ्कः = 0;
	पूर्णाङ्क n, ret;

	यदि (argc != 3)
		प्रत्यागम EXIT_FAILURE;
	pid = strtol(argv[1], (वर्ण **) रिक्त, 10);
	n = strtol(argv[2], (वर्ण **) रिक्त, 10);

	pc = ul_new_procfs_मार्गः(pid, उपसर्गः);
	यदि (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &अङ्कः);
	यदि (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	प्रारूप_लिख("%d: %dth %"PRIu64"\n", (पूर्णाङ्क) pid, n, अङ्कः);
	ul_unref_मार्गः(pc);
	प्रत्यागम EXIT_SUCCESS;
}

पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण *argv[])
{
	ध्रुव वर्ण *उपसर्गः = रिक्त;

	यदि (argc > 2 && वाक्यं_तुलय(argv[1], "--prefix") == 0) {
		उपसर्गः = argv[2];
		argc -= 2;
		argv += 2;
	}

	यदि (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				क्रमादेशः_invocation_short_name);
		प्रत्यागम EXIT_FAILURE;
	}

	यदि (वाक्यं_तुलय(argv[1], "--tasks") == 0)
		प्रत्यागम test_tasks(argc - 1, argv + 1, उपसर्गः);
	यदि (वाक्यं_तुलय(argv[1], "--fds") == 0)
		प्रत्यागम test_fds(argc - 1, argv + 1, उपसर्गः);
	यदि (वाक्यं_तुलय(argv[1], "--processes") == 0)
		प्रत्यागम test_processes(argc - 1, argv + 1);
	यदि (वाक्यं_तुलय(argv[1], "--is-procfs") == 0)
		प्रत्यागम test_isprocfs(argc - 1, argv + 1);
	यदि (वाक्यं_तुलय(argv[1], "--one") == 0)
		प्रत्यागम test_one_process(argc - 1, argv + 1, उपसर्गः);
	यदि (वाक्यं_तुलय(argv[1], "--stat-nth") == 0)
		प्रत्यागम test_process_stat_nth(argc - 1, argv + 1, उपसर्गः);

	प्रत्यागम EXIT_FAILURE;
}
#यदि_अन्तः /* TEST_PROGRAM_PROCUTILS */
