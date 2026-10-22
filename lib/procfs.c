/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2021]
 */
#समावेश <ctype.h>
#समावेश <unistd.h>
#समावेश <errno.h>

#अगर_घोषित HAVE_SYS_VFS_H
# include <sys/vfs.h>
#पूर्ण_अगर

#समावेश "c.h"
#समावेश "pathnames.h"
#समावेश "procfs.h"
#समावेश "fileutils.h"
#समावेश "all-io.h"
#समावेश "vfs.h"
#समावेश "debug.h"
#समावेश "strutils.h"
#समावेश "statfs_magic.h"

अटल व्योम procfs_process_deinit_path(काष्ठा path_cxt *pc);

/*
 * Debug stuff (based on include/debug.h)
 */
अटल UL_DEBUG_DEFINE_MASK(ulprocfs);
UL_DEBUG_DEFINE_MASKNAMES(ulprocfs) = UL_DEBUG_EMPTY_MASKNAMES;

#घोषणा ULPROCFS_DEBUG_INIT	(1 << 1)
#घोषणा ULPROCFS_DEBUG_CXT	(1 << 2)

#घोषणा DBG(m, x)		__UL_DBG(ulprocfs, ULPROCFS_DEBUG_, m, x)
#घोषणा DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulprocfs, ULPROCFS_DEBUG_, m, h, x)
#घोषणा ON_DBG(m, x)		__UL_DBG_CALL(ulprocfs, ULPROCFS_DEBUG_, m, x)

व्योम ul_procfs_init_debug(व्योम)
{
	अगर (ulprocfs_debug_mask)
		वापस;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

काष्ठा path_cxt *ul_new_procfs_path(pid_t pid, स्थिर अक्षर *prefix)
{
	काष्ठा path_cxt *pc = ul_new_path(शून्य);

	अगर (!pc)
		वापस शून्य;
	अगर (prefix)
		ul_path_set_prefix(pc, prefix);

	अगर (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		वापस शून्य;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	वापस pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
पूर्णांक procfs_process_init_path(काष्ठा path_cxt *pc, pid_t pid)
{
	काष्ठा procfs_process *prc;
	पूर्णांक rc;
	अक्षर buf[माप(_PATH_PROC) + माप(stringify_मूल्य(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, माप(buf), _PATH_PROC "/%zu", (माप_प्रकार) pid);
	rc = ul_path_set_dir(pc, buf);
	अगर (rc)
		वापस rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	अगर (rc < 0)
		वापस rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	अगर (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = सुस्मृति(1, माप(काष्ठा procfs_process));
		अगर (!prc)
			वापस -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	वापस 0;
}

अटल व्योम procfs_process_deinit_path(काष्ठा path_cxt *pc)
{
	काष्ठा procfs_process *prc;

	अगर (!pc)
		वापस;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	अगर (!prc)
		वापस;

	मुक्त(prc);
	ul_path_set_dialect(pc, शून्य, शून्य);
}

अटल ssize_t read_procfs_file(स्थिर काष्ठा ul_vfs_ops *vfs, पूर्णांक fd,
			       अक्षर *buf, माप_प्रकार bufsz)
{
	ssize_t sz = 0;
	माप_प्रकार i;

	अगर (fd < 0)
		वापस -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	अगर (sz <= 0)
		वापस sz;

	क्रम (i = 0; i < (माप_प्रकार) sz; i++) {
		अगर (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	वापस sz;
}

अटल ssize_t procfs_process_get_तथ्य_for(काष्ठा path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz,
					    स्थिर अक्षर *fname)
{
	पूर्णांक fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	अगर (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : शून्य, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : शून्य, fd);
		वापस sz;
	}
	वापस -त्रुटि_सं;
}

ssize_t procfs_process_get_cmdline(काष्ठा path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz)
{
	वापस procfs_process_get_तथ्य_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(काष्ठा path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz)
{
	वापस procfs_process_get_तथ्य_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(काष्ठा path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz)
{
	वापस procfs_process_get_तथ्य_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(काष्ठा path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz)
{
	वापस procfs_process_get_तथ्य_for(pc, buf, bufsz, "syscall");
}

पूर्णांक procfs_process_get_stat_nth(काष्ठा path_cxt *pc, पूर्णांक n, uint64_t *re)
{
	ssize_t rc;
	अक्षर *कुंजी = शून्य, *tok, *p;
	अक्षर buf[बफ_मान];
	पूर्णांक i;

	अगर (n == 2 || n == 3)		/* process name and status (strings) */
		वापस -EINVAL;

	rc = procfs_process_get_तथ्य_for(pc, buf, माप(buf), "stat");
	अगर (rc < 0)
		वापस rc;

	क्रम (i = 0, tok = strtok_r(buf, " ", &कुंजी); tok;
	     tok = strtok_r(शून्य, " ", &कुंजी)) {

		i++;
		अगर (i == n)
			वापस ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		अगर (i == 2 && (p = म_खोजप(कुंजी, ')')))
			कुंजी = p + 2;
	}

	वापस -EINVAL;
}

पूर्णांक procfs_process_get_uid(काष्ठा path_cxt *pc, uid_t *uid)
{
	काष्ठा stat sb;
	पूर्णांक rc;

	अगर ((rc = ul_path_stat(pc, &sb, 0, शून्य)) == 0)
		*uid = sb.st_uid;
	वापस rc;
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
पूर्णांक procfs_process_अग्र_tid(काष्ठा path_cxt *pc, सूची **sub, pid_t *tid)
{
	काष्ठा dirent *d;

	अगर (!pc || !sub || !tid)
		वापस -EINVAL;

	अगर (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		अगर (!*sub)
			वापस -त्रुटि_सं;
	}

	जबतक ((d = xreaddir(*sub))) {
		अगर (procfs_dirent_get_pid(d, tid) == 0)
			वापस 0;
	}

	बंद_सूची(*sub);
	*sub = शून्य;
	वापस 1;
}

पूर्णांक procfs_process_अग्र_fd(काष्ठा path_cxt *pc, सूची **sub, पूर्णांक *fd)
{
	काष्ठा dirent *d;

	अगर (!pc || !sub || !fd)
		वापस -EINVAL;

	अगर (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		अगर (!*sub)
			वापस -त्रुटि_सं;
	}

	जबतक ((d = xreaddir(*sub))) {
		uint64_t अंक;
#अगर_घोषित _DIRENT_HAVE_D_TYPE
		अगर (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			जारी;
#पूर्ण_अगर
		अगर (ul_strtou64(d->d_name, &अंक, 10) < 0)
			जारी;
		*fd = अंक;
		वापस 0;
	}

	बंद_सूची(*sub);
	*sub = शून्य;
	वापस 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
पूर्णांक procfs_dirent_is_process(काष्ठा dirent *d)
{
#अगर_घोषित _DIRENT_HAVE_D_TYPE
	अगर (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		वापस 0;
#पूर्ण_अगर
	अगर (!है_अंक((अचिन्हित अक्षर) *d->d_name))
		वापस 0;

	वापस 1;
}

पूर्णांक procfs_dirent_get_pid(काष्ठा dirent *d, pid_t *pid)
{
	uint64_t अंक;

	अगर (!procfs_dirent_is_process(d))
		वापस -EINVAL;

	अगर (ul_strtou64(d->d_name, &अंक, 10) < 0)
		वापस -EINVAL;

	*pid = (pid_t) अंक;
	वापस 0;
}

पूर्णांक procfs_dirent_get_uid(सूची *procfs, काष्ठा dirent *d, uid_t *uid)
{
	काष्ठा stat st;

	अगर (!procfs_dirent_is_process(d))
		वापस -EINVAL;

	अगर (fstatat(dirfd(procfs), d->d_name, &st, 0))
		वापस -EINVAL;

	*uid = st.st_uid;
	वापस 0;
}

पूर्णांक procfs_dirent_match_uid(सूची *procfs, काष्ठा dirent *d, uid_t uid)
{
	uid_t x;

	अगर (procfs_dirent_get_uid(procfs, d, &x) == 0)
		वापस x == uid;

	वापस 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
पूर्णांक procfs_dirent_get_name(सूची *procfs, काष्ठा dirent *d, अक्षर *buf, माप_प्रकार bufsz)
{
	खाता *f;
	माप_प्रकार sz;
	अक्षर tmp[1024], *p, *end = शून्य;

	अगर (bufsz < 32)
		वापस -EINVAL;
	अगर (!procfs_dirent_is_process(d))
		वापस -EINVAL;

	snprintf(tmp, माप(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	अगर (!f)
		वापस -त्रुटि_सं;

	p = ख_माला_लो(tmp, माप(tmp), f);
	ख_बंद(f);
	अगर (!p)
		वापस -त्रुटि_सं;

	/* skip PID */
	जबतक (*p && *p != '(')
		p++;

	/* skip extra '(' */
	जबतक (*p && *p == '(')
		p++;

	end = p;
	जबतक (*end && *end != ')')
		end++;

	sz = end - p;
	अगर (sz >= bufsz)
		sz = bufsz - 1;

	स_नकल(buf, p, sz);
	buf[sz] = '\0';

	वापस 0;
}

पूर्णांक procfs_dirent_match_name(सूची *procfs, काष्ठा dirent *d, स्थिर अक्षर *name)
{
	अक्षर buf[33];

	अगर (procfs_dirent_get_name(procfs, d, buf, माप(buf)) == 0)
		वापस म_भेद(name, buf) == 0;

	वापस 0;
}

#अगर_घोषित HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
पूर्णांक fd_is_procfs(पूर्णांक fd)
{
	काष्ठा statfs st;
	पूर्णांक ret;

	करो {
		त्रुटि_सं = 0;
		ret = fstatfs(fd, &st);

		अगर (ret < 0) {
			अगर (त्रुटि_सं != EINTR && त्रुटि_सं != EAGAIN)
				वापस 0;
			xusleep(250000);
		}
	} जबतक (ret != 0);

	वापस F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#अन्यथा
पूर्णांक fd_is_procfs(पूर्णांक fd __attribute__((__unused__)))
{
	वापस 0;
}
#पूर्ण_अगर

अटल अक्षर *strdup_procfs_file(pid_t pid, स्थिर अक्षर *name)
{
	अक्षर buf[बफ_मान];
	अक्षर *re = शून्य;
	पूर्णांक fd;

	snprintf(buf, माप(buf), _PATH_PROC "/%d/%s", (पूर्णांक) pid, name);
	fd = खोलो(buf, O_CLOEXEC|O_RDONLY);
	अगर (fd < 0)
		वापस शून्य;

	अगर (read_procfs_file(शून्य, fd, buf, माप(buf)) > 0)
		re = strdup(buf);
	बंद(fd);
	वापस re;
}

अक्षर *pid_get_cmdname(pid_t pid)
{
	वापस strdup_procfs_file(pid, "comm");
}

अक्षर *pid_get_cmdline(pid_t pid)
{
	वापस strdup_procfs_file(pid, "cmdline");
}

अक्षर *pid_get_personality(pid_t pid)
{
	वापस strdup_procfs_file(pid, "personality");
}

#अगर_घोषित TEST_PROGRAM_PROCFS

अटल पूर्णांक test_tasks(पूर्णांक argc, अक्षर *argv[], स्थिर अक्षर *prefix)
{
	सूची *sub = शून्य;
	काष्ठा path_cxt *pc;
	pid_t tid = 0, pid;

	अगर (argc != 2)
		वापस निकास_त्रुटि;

	pid = म_से_दीर्घ(argv[1], (अक्षर **) शून्य, 10);
	म_लिखो("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	अगर (!pc)
		err(निकास_त्रुटि, "alloc procfs handler failed");

	जबतक (procfs_process_अग्र_tid(pc, &sub, &tid) == 0)
		म_लिखो(" %d", tid);

	म_लिखो("\n");
	ul_unref_path(pc);
	वापस निकास_सफल;
}

अटल पूर्णांक test_fds(पूर्णांक argc, अक्षर *argv[], स्थिर अक्षर *prefix)
{
	सूची *sub = शून्य;
	काष्ठा path_cxt *pc;
	pid_t pid;
	पूर्णांक fd = -1;

	अगर (argc != 2)
		वापस निकास_त्रुटि;

	pid = म_से_दीर्घ(argv[1], (अक्षर **) शून्य, 10);
	म_लिखो("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	अगर (!pc)
		err(निकास_त्रुटि, "alloc procfs handler failed");

	जबतक (procfs_process_अग्र_fd(pc, &sub, &fd) == 0)
		म_लिखो(" %d", fd);

	ख_अक्षर_दो('\n', मानक_निकास);
	ul_unref_path(pc);
	वापस निकास_सफल;
}

अटल पूर्णांक test_processes(पूर्णांक argc, अक्षर *argv[])
{
	सूची *dir;
	काष्ठा dirent *d;
	अक्षर *name = शून्य;
	uid_t uid = (uid_t) -1;
	अक्षर buf[128];

	अगर (argc >= 3 && म_भेद(argv[1], "--name") == 0)
		name = argv[2];
	अगर (argc >= 3 && म_भेद(argv[1], "--uid") == 0)
		uid = (uid_t) म_से_द(argv[2]);

	dir = सूची_खोलो(_PATH_PROC);
	अगर (!dir)
		err(निकास_त्रुटि, "cannot open proc");

	जबतक ((d = xreaddir(dir))) {
		pid_t pid = 0;

		अगर (procfs_dirent_get_pid(d, &pid) != 0)
			जारी;
		अगर (name && !procfs_dirent_match_name(dir, d, name))
			जारी;
		अगर (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			जारी;
		procfs_dirent_get_name(dir, d, buf, माप(buf));
		म_लिखो(" %d [%s]", pid, buf);
	}

	ख_अक्षर_दो('\n', मानक_निकास);
	बंद_सूची(dir);
	वापस निकास_सफल;
}

अटल पूर्णांक test_one_process(पूर्णांक argc, अक्षर *argv[], स्थिर अक्षर *prefix)
{
	pid_t pid;
	काष्ठा path_cxt *pc;
	अक्षर buf[बफ_मान];
	uid_t uid = (uid_t) -1;

	अगर (argc != 2)
		वापस निकास_त्रुटि;
	pid = म_से_दीर्घ(argv[1], (अक्षर **) शून्य, 10);

	pc = ul_new_procfs_path(pid, prefix);
	अगर (!pc)
		err(निकास_त्रुटि, "cannot alloc procfs handler");

	म_लिखो("%d\n", (पूर्णांक) pid);

	procfs_process_get_uid(pc, &uid);
	म_लिखो("   UID: %zu\n", (माप_प्रकार) uid);

	procfs_process_get_cmdline(pc, buf, माप(buf));
	म_लिखो("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, माप(buf));
	म_लिखो("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	वापस निकास_सफल;
}

अटल पूर्णांक test_isprocfs(पूर्णांक argc, अक्षर *argv[])
{
	स्थिर अक्षर *name = argc > 1 ? argv[1] : "/proc";
	पूर्णांक fd = खोलो(name, O_RDONLY | O_CLOEXEC);
	पूर्णांक is = 0;

	अगर (fd >= 0) {
		is = fd_is_procfs(fd);
		बंद(fd);
	} अन्यथा
		err(निकास_त्रुटि, "cannot open %s", name);

	म_लिखो("%s: %s procfs\n", name, is ? "is" : "is NOT");
	वापस is ? निकास_सफल : निकास_त्रुटि;
}

अटल पूर्णांक test_process_stat_nth(पूर्णांक argc, अक्षर *argv[], स्थिर अक्षर *prefix)
{
	pid_t pid;
	काष्ठा path_cxt *pc;
	uint64_t अंक = 0;
	पूर्णांक n, ret;

	अगर (argc != 3)
		वापस निकास_त्रुटि;
	pid = म_से_दीर्घ(argv[1], (अक्षर **) शून्य, 10);
	n = म_से_दीर्घ(argv[2], (अक्षर **) शून्य, 10);

	pc = ul_new_procfs_path(pid, prefix);
	अगर (!pc)
		err(निकास_त्रुटि, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &अंक);
	अगर (ret)
		errx(निकास_त्रुटि, "read %dth number failed: %s", n, म_त्रुटि(-ret));

	म_लिखो("%d: %dth %"PRIu64"\n", (पूर्णांक) pid, n, अंक);
	ul_unref_path(pc);
	वापस निकास_सफल;
}

पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[])
{
	स्थिर अक्षर *prefix = शून्य;

	अगर (argc > 2 && म_भेद(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	अगर (argc < 2) {
		ख_लिखो(मानक_त्रुटि, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		वापस निकास_त्रुटि;
	}

	अगर (म_भेद(argv[1], "--tasks") == 0)
		वापस test_tasks(argc - 1, argv + 1, prefix);
	अगर (म_भेद(argv[1], "--fds") == 0)
		वापस test_fds(argc - 1, argv + 1, prefix);
	अगर (म_भेद(argv[1], "--processes") == 0)
		वापस test_processes(argc - 1, argv + 1);
	अगर (म_भेद(argv[1], "--is-procfs") == 0)
		वापस test_isprocfs(argc - 1, argv + 1);
	अगर (म_भेद(argv[1], "--one") == 0)
		वापस test_one_process(argc - 1, argv + 1, prefix);
	अगर (म_भेद(argv[1], "--stat-nth") == 0)
		वापस test_process_stat_nth(argc - 1, argv + 1, prefix);

	वापस निकास_त्रुटि;
}
#पूर्ण_अगर /* TEST_PROGRAM_PROCUTILS */
