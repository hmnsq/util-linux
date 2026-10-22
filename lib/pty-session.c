/*
 * This is pseudo-terminal container for child process where parent creates a
 * proxy between the current std{in,out,etrr} and the child's pty. Advantages:
 *
 * - child has no access to parent's terminal (e.g. su --pty)
 * - parent can log all traffic between user and child's terminal (e.g. script(1))
 * - it's possible to start commands on terminal although parent has no terminal
 *
 * This code is in the public domain; do with it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> in Jul 2019
 */
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <pty.h>
#অন্তর্ভাব <poll.h>
#অন্তর্ভাব <sys/signalfd.h>
#অন্তর্ভাব <paths.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/wait.h>
#অন্তর্ভাব <inttypes.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "all-io.h"
#অন্তর্ভাব "ttyutils.h"
#অন্তর্ভাব "pty-session.h"
#অন্তর্ভাব "monotonic.h"
#অন্তর্ভাব "debug.h"

স্থির UL_DEBUG_DEFINE_MASK(ulpty);
UL_DEBUG_DEFINE_MASKNAMES(ulpty) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPTY_DEBUG_INIT	(1 << 1)
#define ULPTY_DEBUG_SETUP	(1 << 2)
#define ULPTY_DEBUG_SIG		(1 << 3)
#define ULPTY_DEBUG_IO		(1 << 4)
#define ULPTY_DEBUG_DONE	(1 << 5)

#define DBG(m, x)		__UL_DBG(ulpty, ULPTY_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpty, ULPTY_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpty, ULPTY_DEBUG_, m, x)

শূন্য ul_pty_init_debug(পূর্ণসংখ্যা mask)
{
	যদি (ulpty_debug_mask)
		ফেরত;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, mask, ULPTY_DEBUG);
}

কাঠামো ul_pty *ul_new_pty(পূর্ণসংখ্যা is_stdin_tty)
{
	কাঠামো ul_pty *pty = calloc(1, আকার(*pty));

	যদি (!pty)
		ফেরত ফাঁকা;

	DBG_OBJ(SETUP, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->child = (pid_t) -1;

	ফেরত pty;
}

শূন্য ul_free_pty(কাঠামো ul_pty *pty)
{
	কাঠামো ul_pty_child_বাফার *hd;

	যখন ((hd = pty->child_বাফার_শীর্ষ)) {
		pty->child_বাফার_শীর্ষ = hd->পরবর্তী;
		মুক্ত(hd);
	}

	যখন ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->পরবর্তী;
		মুক্ত(hd);
	}
	মুক্ত(pty);
}

শূন্য ul_pty_slave_echo(কাঠামো ul_pty *pty, পূর্ণসংখ্যা enable)
{
	প্রতিজ্ঞা(pty);
	pty->slave_echo = enable ? 1 : 0;
}

পূর্ণসংখ্যা ul_pty_get_delivered_signal(কাঠামো ul_pty *pty)
{
	প্রতিজ্ঞা(pty);
	ফেরত pty->delivered_signal;
}

কাঠামো ul_pty_callbacks *ul_pty_get_callbacks(কাঠামো ul_pty *pty)
{
	প্রতিজ্ঞা(pty);
	ফেরত &pty->callbacks;
}

শূন্য ul_pty_set_callback_তথ্য(কাঠামো ul_pty *pty, শূন্য *তথ্য)
{
	প্রতিজ্ঞা(pty);
	pty->callback_তথ্য = তথ্য;
}

শূন্য ul_pty_set_child(কাঠামো ul_pty *pty, pid_t child)
{
	প্রতিজ্ঞা(pty);
	pty->child = child;
}

পূর্ণসংখ্যা ul_pty_get_childfd(কাঠামো ul_pty *pty)
{
	প্রতিজ্ঞা(pty);
	ফেরত pty->master;
}

pid_t ul_pty_get_child(কাঠামো ul_pty *pty)
{
	প্রতিজ্ঞা(pty);
	ফেরত pty->child;
}

/* it's active when signals are redirected to sigfd */
পূর্ণসংখ্যা ul_pty_is_running(কাঠামো ul_pty *pty)
{
	প্রতিজ্ঞা(pty);
	ফেরত pty->sigfd >= 0;
}

শূন্য ul_pty_set_mainloop_time(কাঠামো ul_pty *pty, কাঠামো timeval *tv)
{
	প্রতিজ্ঞা(pty);
	যদি (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->পরবর্তী_callback_time);
	} অন্যথা {
		pty->পরবর্তী_callback_time.tv_sec = tv->tv_sec;
		pty->পরবর্তী_callback_time.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

স্থির শূন্য pty_signals_cleanup(কাঠামো ul_pty *pty)
{
	যদি (pty->sigfd != -1)
		close(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, ফাঁকা);
}

/* call me before fork() */
পূর্ণসংখ্যা ul_pty_setup(কাঠামো ul_pty *pty)
{
	কাঠামো termios attrs;
	পূর্ণসংখ্যা rc = 0;

	প্রতিজ্ঞা(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, ফাঁকা, &pty->orgsig);

	যদি (pty->isterm) {
		DBG_OBJ(SETUP, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		যদি (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -errno;
			যাও done;
		}

		attrs = pty->stdin_attrs;
		যদি (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		অন্যথা
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (অক্ষর *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, ফাঁকা, &attrs, &pty->win);
		যদি (rc)
			যাও done;

	} অন্যথা {
		DBG_OBJ(SETUP, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, ফাঁকা, ফাঁকা, ফাঁকা);
		যদি (rc)
			যাও done;

		tcgetattr(pty->slave, &attrs);

		যদি (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		অন্যথা
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

done:
	যদি (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	ফেরত rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
পূর্ণসংখ্যা ul_pty_terminal_setup(কাঠামো ul_pty *pty)
{
	কাঠামো termios attrs;

	যদি (!pty->isterm)
		ফেরত 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	যদি (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		ফেরত -errno;

	DBG_OBJ(SETUP, pty, ul_debug("terminal raw mode setup done"));
	ফেরত 0;
}

/* call me before fork() */
পূর্ণসংখ্যা ul_pty_signals_setup(কাঠামো ul_pty *pty)
{
	sigset_t ourset;
	পূর্ণসংখ্যা rc = 0;

	প্রতিজ্ঞা(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, ফাঁকা, &pty->orgsig);

	sigfillset(&ourset);
	যদি (sigprocmask(SIG_BLOCK, &ourset, ফাঁকা)) {
		rc = -errno;
		যাও done;
	}

	sigemptyset(&ourset);
	sigaddset(&ourset, SIGCHLD);
	sigaddset(&ourset, SIGWINCH);
	sigaddset(&ourset, SIGALRM);
	sigaddset(&ourset, SIGHUP);
	sigaddset(&ourset, SIGTERM);
	sigaddset(&ourset, SIGINT);
	sigaddset(&ourset, SIGQUIT);

	যদি (pty->callbacks.flush_logs)
		sigaddset(&ourset, SIGUSR1);

	যদি ((pty->sigfd = signalfd(-1, &ourset, SFD_CLOEXEC)) < 0)
		rc = -errno;
done:
	যদি (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	ফেরত rc;
}

/* cleanup in parent process */
শূন্য ul_pty_cleanup(কাঠামো ul_pty *pty)
{
	কাঠামো termios rtt;

	pty_signals_cleanup(pty);

	যদি (pty->master == -1 || !pty->isterm)
		ফেরত;

	DBG_OBJ(DONE, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

পূর্ণসংখ্যা ul_pty_chownmod_slave(কাঠামো ul_pty *pty, uid_t uid, gid_t gid, mode_t mode)
{
	যদি (fchown(pty->slave, uid, gid))
		ফেরত -errno;
	যদি (fchmod(pty->slave, mode))
		ফেরত -errno;
	ফেরত 0;
}

/* call me in child process */
শূন্য ul_pty_init_slave(কাঠামো ul_pty *pty)
{
	DBG_OBJ(SETUP, pty, ul_debug("initialize slave"));

	setsid();

	ioctl(pty->slave, TIOCSCTTY, 1);
	close(pty->master);

	dup2(pty->slave, STDIN_FILENO);
	dup2(pty->slave, STDOUT_FILENO);
	dup2(pty->slave, STDERR_FILENO);

	close(pty->slave);

	যদি (pty->sigfd >= 0)
		close(pty->sigfd);

	pty->slave = -1;
	pty->master = -1;
	pty->sigfd = -1;

	sigprocmask(SIG_SETMASK, &pty->orgsig, ফাঁকা);

	DBG_OBJ(SETUP, pty, ul_debug("... initialize slave done"));
}

স্থির পূর্ণসংখ্যা write_output(অক্ষর *obuf, ssize_t bytes)
{
	DBG(IO, ul_debug(" writing output"));

	যদি (ul_write_all(STDOUT_FILENO, obuf, bytes)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		ফেরত -errno;
	}

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা schedule_child_write(কাঠামো ul_pty *pty, অক্ষর *buf, size_t bufsz, পূর্ণসংখ্যা final)
{
	কাঠামো ul_pty_child_বাফার *stash;

	যদি (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->পরবর্তী;
		memset(stash, 0, আকার(*stash));
	} অন্যথা
		stash = calloc(1, আকার(*stash));
	যদি (!stash)
		ফেরত -1;

	প্রতিজ্ঞা(bufsz <= আকার(stash->buf));

	memcpy(stash->buf, buf, bufsz);
	stash->আকার_2 = bufsz;
	stash->final_প্রবেশ = final ? 1 : 0;

	যদি (pty->child_বাফার_শীর্ষ)
		pty->child_বাফার_লেজ = pty->child_বাফার_লেজ->পরবর্তী = stash;
	অন্যথা
		pty->child_বাফার_শীর্ষ = pty->child_বাফার_লেজ = stash;
	ফেরত 0;
}

/*
 * The pty is usually faster than shell, so it's a good idea to wait until
 * the previous message has been already read by shell from slave before we
 * write to master. This is necessary especially for EOF situation when we can
 * send EOF to master before shell is fully initialized, to workaround this
 * problem we wait until slave is empty. For example:
 *
 *   echo "date" | su --pty
 *
 * Unfortunately, the child (usually shell) can ignore stdin at all, so we
 * don't wait forever to avoid dead locks...
 *
 * Note that su --pty is primarily designed for interactive sessions as it
 * maintains master+slave tty stuff within the session. Use pipe to write to
 * pty and assume non-interactive (tee-like) behavior is NOT well supported.
 */
স্থির শূন্য drain_child_buffers(কাঠামো ul_pty *pty)
{
	unsigned পূর্ণসংখ্যা tries = 0;
	কাঠামো pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	যখন (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	যদি (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

স্থির পূর্ণসংখ্যা flush_child_buffers(কাঠামো ul_pty *pty, পূর্ণসংখ্যা *anything)
{
	পূর্ণসংখ্যা rc = 0, any = 0;

	যখন (pty->child_বাফার_শীর্ষ) {
		কাঠামো ul_pty_child_বাফার *hd = pty->child_বাফার_শীর্ষ;
		ssize_t ret;

		যদি (hd->final_প্রবেশ)
			drain_child_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->আকার_2 - hd->cursor));

		ret = লিখ(pty->master, hd->buf + hd->cursor, hd->আকার_2 - hd->cursor);
		যদি (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			যদি (!(errno == EINTR || errno == EAGAIN))
				rc = -errno;
			যাও out;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		যদি (hd->cursor == hd->আকার_2) {
			pty->child_বাফার_শীর্ষ = hd->পরবর্তী;
			যদি (!hd->পরবর্তী)
				pty->child_বাফার_লেজ = ফাঁকা;

			hd->পরবর্তী = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
out:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	যদি (any)
		fdatasync(pty->master);

	যদি (anything)
		*anything = any;
	ফেরত rc;
}

শূন্য ul_pty_write_eof_to_child(কাঠামো ul_pty *pty)
{
	অক্ষর c = DEF_EOF;
	schedule_child_write(pty, &c, আকার(অক্ষর), 1);
}

স্থির পূর্ণসংখ্যা mainloop_callback(কাঠামো ul_pty *pty)
{
	পূর্ণসংখ্যা rc;

	যদি (!pty->callbacks.mainloop)
		ফেরত 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_তথ্য);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	ফেরত rc;
}

স্থির পূর্ণসংখ্যা handle_io(কাঠামো ul_pty *pty, পূর্ণসংখ্যা fd, পূর্ণসংখ্যা *eof)
{
	অক্ষর buf[BUFSIZ];
	ssize_t bytes;
	পূর্ণসংখ্যা rc = 0;
	sigset_t set;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&set);
	sigaddset(&set, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &set, ফাঁকা);
	/* read from active FD */
	bytes = পঠ(fd, buf, আকার(buf));
	sigprocmask(SIG_BLOCK, &set, ফাঁকা);
	যদি (bytes == -1) {
		যদি (errno == EAGAIN || errno == EINTR)
			ফেরত 0;
		ফেরত -errno;
	}

	যদি (bytes == 0) {
		*eof = 1;
		ফেরত 0;
	}

	/* from stdin (user) to command */
	যদি (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", bytes));

		যদি (schedule_child_write(pty, buf, bytes, 0))
			ফেরত -errno;

	/* from command (master) to stdout */
	} অন্যথা যদি (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", bytes));
		write_output(buf, bytes);
	}

	যদি (pty->callbacks.log_stream_activity)
		rc = pty->callbacks.log_stream_activity(
					pty->callback_তথ্য, fd, buf, bytes);

	ফেরত rc;
}

শূন্য ul_pty_wait_for_child(কাঠামো ul_pty *pty)
{
	পূর্ণসংখ্যা status;
	pid_t pid;
	পূর্ণসংখ্যা options = 0;

	যদি (pty->child == (pid_t) -1)
		ফেরত;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (পূর্ণসংখ্যা) pty->child));

	যদি (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		জন্য (;;) {
			pid = waitpid(pty->child, &status, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (পূর্ণসংখ্যা) pid));
			যদি (pid != (pid_t) - 1) {
				যদি (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_তথ্য,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			} অন্যথা
				বিরতি;
		}
	} অন্যথা {
		/* final wait */
		যখন ((pid = waitpid(-1, &status, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (পূর্ণসংখ্যা) pid));
			যদি (pid == pty->child) {
				যদি (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_তথ্য,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			}
		}
	}
}

স্থির পূর্ণসংখ্যা handle_signal(কাঠামো ul_pty *pty, পূর্ণসংখ্যা fd)
{
	কাঠামো signalfd_siginfo info;
	ssize_t bytes;
	পূর্ণসংখ্যা rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	bytes = পঠ(fd, &info, আকার(info));
	যদি (bytes != আকার(info)) {
		যদি (bytes < 0 && (errno == EAGAIN || errno == EINTR))
			ফেরত 0;
		ফেরত -errno;
	}

	নির্বাচন (info.ssi_signo) {
	ক্ষেত্র SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		যদি (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			যদি (pty->callbacks.child_wait)
				pty->callbacks.child_wait(pty->callback_তথ্য,
							  pty->child);
			অন্যথা
				ul_pty_wait_for_child(pty);

		} অন্যথা যদি (info.ssi_status == SIGSTOP && pty->child > 0) {
			pty->callbacks.child_sigstop(pty->callback_তথ্য,
						     pty->child);
		}

		যদি (pty->child <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->পরবর্তী_callback_time);
		}
		ফেরত 0;
	ক্ষেত্র SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		যদি (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (অক্ষর *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (অক্ষর *)&pty->win);

			যদি (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_তথ্য,
							&info, (শূন্য *) &pty->win);
		}
		বিরতি;
	ক্ষেত্র SIGALRM:
		FALLTHROUGH;
	ক্ষেত্র SIGHUP:
		FALLTHROUGH;
	ক্ষেত্র SIGTERM:
		FALLTHROUGH;
	ক্ষেত্র SIGINT:
		FALLTHROUGH;
	ক্ষেত্র SIGQUIT:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIG{ALRM,HUP,TERM,INT,QUIT}"));
		pty->delivered_signal = info.ssi_signo;
		/* Child termination is going to generate SIGCHLD (see above) */
		যদি (pty->child > 0)
			kill(pty->child, SIGTERM);

		যদি (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_তথ্য,
					&info, (শূন্য *) &pty->win);
		বিরতি;
	ক্ষেত্র SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		যদি (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_তথ্য);
		বিরতি;
	পূর্বনির্ধারিত:
		abort();
	}

	ফেরত rc;
}

/* loop in parent */
পূর্ণসংখ্যা ul_pty_proxy_master(কাঠামো ul_pty *pty)
{
	পূর্ণসংখ্যা rc = 0, ret, eof = 0;
	গণনা {
		POLLFD_SIGNAL = 0,
		POLLFD_MASTER,
		POLLFD_STDIN

	};
	কাঠামো pollfd pfd[] = {
		[POLLFD_SIGNAL] = { .fd = -1,		.events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_MASTER] = { .fd = pty->master,  .events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_STDIN]	= { .fd = STDIN_FILENO, .events = POLLIN | POLLERR | POLLHUP }
	};

	/* We use signalfd, and standard signals by handlers are completely blocked */
	প্রতিজ্ঞা(pty->sigfd >= 0);

	pfd[POLLFD_SIGNAL].fd = pty->sigfd;
	pty->poll_timeout = -1;

	যখন (!pty->delivered_signal) {
		size_t i;
		পূর্ণসংখ্যা errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		যদি (timerisset(&pty->পরবর্তী_callback_time)) {
			কাঠামো timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			যদি (timercmp(&now, &pty->পরবর্তী_callback_time, >)) {
				rc = mainloop_callback(pty);
				যদি (rc)
					বিরতি;
			}
		}

		/* set timeout */
		যদি (timerisset(&pty->পরবর্তী_callback_time)) {
			কাঠামো timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->পরবর্তী_callback_time, &now, &rest);
			timeout = (rest.tv_sec * MSEC_PER_SEC) +  (rest.tv_usec / USEC_PER_MSEC);
		} অন্যথা
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		যদি (pty->child_বাফার_শীর্ষ)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		অন্যথা
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_আকার(pfd), timeout);

		errsv = errno;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		যদি (ret < 0) {
			যদি (errsv == EAGAIN)
				চালু;
			rc = -errno;
			বিরতি;
		}

		/* timeout */
		যদি (ret == 0) {
			যদি (timerisset(&pty->পরবর্তী_callback_time)) {
				rc = mainloop_callback(pty);
				যদি (rc == 0)
					চালু;
			} অন্যথা {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			বিরতি;
		}
		/* event */
		জন্য (i = 0; i < ARRAY_আকার(pfd); i++) {
			যদি (pfd[i].revents == 0)
				চালু;

			DBG_OBJ(IO, pty, ul_debug(" active pfd[%s].fd=%d %s %s %s %s %s",
						i == POLLFD_STDIN  ? "stdin" :
						i == POLLFD_MASTER ? "master" :
						i == POLLFD_SIGNAL ? "signal" : "???",
						pfd[i].fd,
						pfd[i].revents & POLLIN  ? "POLLIN" : "",
						pfd[i].revents & POLLOUT ? "POLLOUT" : "",
						pfd[i].revents & POLLHUP ? "POLLHUP" : "",
						pfd[i].revents & POLLERR ? "POLLERR" : "",
						pfd[i].revents & POLLNVAL ? "POLLNVAL" : ""));

			যদি (i == POLLFD_SIGNAL)
				rc = handle_signal(pty, pfd[i].fd);
			অন্যথা {
				যদি (pfd[i].revents & POLLIN)
					rc = handle_io(pty, pfd[i].fd, &eof); /*তথ্য*/
				যদি (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_child_buffers(pty, ফাঁকা);
			}

			যদি (rc) {
				পূর্ণসংখ্যা anything = 1;

				ul_pty_write_eof_to_child(pty);
				জন্য (anything = 1; anything;)
					flush_child_buffers(pty, &anything);
				বিরতি;
			}

			যদি (i == POLLFD_SIGNAL)
				চালু;

			/* EOF maybe detected in two ways; they are as follows:
			 *	A) poll() return POLLHUP event after close()
			 *	B) read() returns 0 (no data)
			 *
			 * POLLNVAL means that fd is closed.
			 */
			যদি ((pfd[i].revents & POLLHUP) || (pfd[i].revents & POLLNVAL) || eof) {
				DBG_OBJ(IO, pty, ul_debug(" ignore FD"));
				যদি (i == POLLFD_STDIN) {
					pfd[i].fd = -1;
					ul_pty_write_eof_to_child(pty);
				} অন্যথা /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		যদি (rc)
			বিরতি;
	}

	যদি (rc && pty->child && pty->child != (pid_t) -1 && !pty->delivered_signal) {
		kill(pty->child, SIGTERM);
		sleep(2);
		kill(pty->child, SIGKILL);
	}

	pty_signals_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	ফেরত rc;
}

#ifdef TEST_PROGRAM_PTY
/*
 * $ make test_pty
 * $ ./test_pty
 *
 * ... and see for example tty(1) or "ps afu"
 */
স্থির শূন্য child_sigstop(শূন্য *তথ্য __attribute__((__unused__)), pid_t child)
{
	kill(getpid(), SIGSTOP);
	kill(child, SIGCONT);
}

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	কাঠামো ul_pty_callbacks *cb;
	ধ্রুবক অক্ষর *shell, *command = ফাঁকা, *shname = ফাঁকা;
	পূর্ণসংখ্যা caught_signal = 0;
	pid_t child;
	কাঠামো ul_pty *pty;

	shell = getenv("SHELL");
	যদি (shell == ফাঁকা)
		shell = _PATH_BSHELL;
	যদি (argc == 2)
		command = argv[1];

	ul_pty_init_debug(0);

	pty = ul_new_pty(isatty(STDIN_FILENO));
	যদি (!pty)
		err(EXIT_FAILURE, "failed to allocate PTY handler");

	cb = ul_pty_get_callbacks(pty);
	cb->child_sigstop = child_sigstop;

	যদি (ul_pty_setup(pty))
		err(EXIT_FAILURE, "failed to create pseudo-terminal");
	যদি (ul_pty_signals_setup(pty))
		err(EXIT_FAILURE, "failed to initialize signals handler");
	যদি (ul_pty_terminal_setup(pty))
		err(EXIT_FAILURE, "failed to setup terminal");

	fflush(stdout);			/* ??? */

	নির্বাচন ((পূর্ণসংখ্যা) (child = fork())) {
	ক্ষেত্র -1: /* error */
		ul_pty_cleanup(pty);
		err(EXIT_FAILURE, "cannot create child process");
		বিরতি;

	ক্ষেত্র 0: /* child */
		ul_pty_init_slave(pty);

		সংকেত(SIGTERM, SIG_DFL); /* because /etc/csh.login */

		shname = strrchr(shell, '/');
		shname = shname ? shname + 1 : shell;

		যদি (command)
			execl(shell, shname, "-c", command, (অক্ষর *)ফাঁকা);
		অন্যথা
			execl(shell, shname, "-i", (অক্ষর *)ফাঁকা);
		err(EXIT_FAILURE, "failed to execute %s", shell);
		বিরতি;

	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* parent */
	ul_pty_set_child(pty, child);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	যদি (!caught_signal && ul_pty_get_child(pty) != (pid_t)-1)
		ul_pty_wait_for_child(pty);	/* final wait */

	যদি (caught_signal && ul_pty_get_child(pty) != (pid_t)-1) {
		fprintf(stderr, "\nSession terminated, killing shell...");
		kill(child, SIGTERM);
		sleep(2);
		kill(child, SIGKILL);
		fprintf(stderr, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	ফেরত EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM */

