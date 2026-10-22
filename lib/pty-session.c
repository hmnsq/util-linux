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
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <pty.h>
#অন্তর্ভাৱ <poll.h>
#অন্তর্ভাৱ <sys/signalfd.h>
#অন্তর্ভাৱ <paths.h>
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <sys/wait.h>
#অন্তর্ভাৱ <inttypes.h>

#অন্তর্ভাৱ "c.h"
#অন্তর্ভাৱ "all-io.h"
#অন্তর্ভাৱ "ttyutils.h"
#অন্তর্ভাৱ "pty-session.h"
#অন্তর্ভাৱ "monotonic.h"
#অন্তর্ভাৱ "debug.h"

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

শূন্য ul_pty_init_debug(পূর্ণাঙ্ক mask)
{
	যদি (ulpty_debug_mask)
		ঘূৰাই;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, mask, ULPTY_DEBUG);
}

সংরচনা ul_pty *ul_new_pty(পূর্ণাঙ্ক is_stdin_tty)
{
	সংরচনা ul_pty *pty = calloc(1, পরিমাণ(*pty));

	যদি (!pty)
		ঘূৰাই রিক্ত;

	DBG_OBJ(SETUP, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->child = (pid_t) -1;

	ঘূৰাই pty;
}

শূন্য ul_free_pty(সংরচনা ul_pty *pty)
{
	সংরচনা ul_pty_child_buffer *hd;

	যেতিয়ালৈ ((hd = pty->child_buffer_head)) {
		pty->child_buffer_head = hd->next;
		মুক্ত(hd);
	}

	যেতিয়ালৈ ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->next;
		মুক্ত(hd);
	}
	মুক্ত(pty);
}

শূন্য ul_pty_slave_echo(সংরচনা ul_pty *pty, পূর্ণাঙ্ক enable)
{
	প্রতিজ্ঞা(pty);
	pty->slave_echo = enable ? 1 : 0;
}

পূর্ণাঙ্ক ul_pty_get_delivered_signal(সংরচনা ul_pty *pty)
{
	প্রতিজ্ঞা(pty);
	ঘূৰাই pty->delivered_signal;
}

সংরচনা ul_pty_callbacks *ul_pty_get_callbacks(সংরচনা ul_pty *pty)
{
	প্রতিজ্ঞা(pty);
	ঘূৰাই &pty->callbacks;
}

শূন্য ul_pty_set_callback_data(সংরচনা ul_pty *pty, শূন্য *data)
{
	প্রতিজ্ঞা(pty);
	pty->callback_data = data;
}

শূন্য ul_pty_set_child(সংরচনা ul_pty *pty, pid_t child)
{
	প্রতিজ্ঞা(pty);
	pty->child = child;
}

পূর্ণাঙ্ক ul_pty_get_childfd(সংরচনা ul_pty *pty)
{
	প্রতিজ্ঞা(pty);
	ঘূৰাই pty->master;
}

pid_t ul_pty_get_child(সংরচনা ul_pty *pty)
{
	প্রতিজ্ঞা(pty);
	ঘূৰাই pty->child;
}

/* it's active when signals are redirected to sigfd */
পূর্ণাঙ্ক ul_pty_is_running(সংরচনা ul_pty *pty)
{
	প্রতিজ্ঞা(pty);
	ঘূৰাই pty->sigfd >= 0;
}

শূন্য ul_pty_set_mainloop_time(সংরচনা ul_pty *pty, সংরচনা timeval *tv)
{
	প্রতিজ্ঞা(pty);
	যদি (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->next_callback_time);
	} অন্যথা {
		pty->next_callback_time.tv_sec = tv->tv_sec;
		pty->next_callback_time.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

স্থির শূন্য pty_signals_cleanup(সংরচনা ul_pty *pty)
{
	যদি (pty->sigfd != -1)
		close(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, রিক্ত);
}

/* call me before fork() */
পূর্ণাঙ্ক ul_pty_setup(সংরচনা ul_pty *pty)
{
	সংরচনা termios attrs;
	পূর্ণাঙ্ক rc = 0;

	প্রতিজ্ঞা(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, রিক্ত, &pty->orgsig);

	যদি (pty->isterm) {
		DBG_OBJ(SETUP, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		যদি (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -errno;
			গচ্ছ done;
		}

		attrs = pty->stdin_attrs;
		যদি (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		অন্যথা
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (ৱর্ণ *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, রিক্ত, &attrs, &pty->win);
		যদি (rc)
			গচ্ছ done;

	} অন্যথা {
		DBG_OBJ(SETUP, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, রিক্ত, রিক্ত, রিক্ত);
		যদি (rc)
			গচ্ছ done;

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
	ঘূৰাই rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
পূর্ণাঙ্ক ul_pty_terminal_setup(সংরচনা ul_pty *pty)
{
	সংরচনা termios attrs;

	যদি (!pty->isterm)
		ঘূৰাই 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	যদি (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		ঘূৰাই -errno;

	DBG_OBJ(SETUP, pty, ul_debug("terminal raw mode setup done"));
	ঘূৰাই 0;
}

/* call me before fork() */
পূর্ণাঙ্ক ul_pty_signals_setup(সংরচনা ul_pty *pty)
{
	sigset_t ourset;
	পূর্ণাঙ্ক rc = 0;

	প্রতিজ্ঞা(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, রিক্ত, &pty->orgsig);

	sigfillset(&ourset);
	যদি (sigprocmask(SIG_BLOCK, &ourset, রিক্ত)) {
		rc = -errno;
		গচ্ছ done;
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
	ঘূৰাই rc;
}

/* cleanup in parent process */
শূন্য ul_pty_cleanup(সংরচনা ul_pty *pty)
{
	সংরচনা termios rtt;

	pty_signals_cleanup(pty);

	যদি (pty->master == -1 || !pty->isterm)
		ঘূৰাই;

	DBG_OBJ(DONE, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

পূর্ণাঙ্ক ul_pty_chownmod_slave(সংরচনা ul_pty *pty, uid_t uid, gid_t gid, mode_t mode)
{
	যদি (fchown(pty->slave, uid, gid))
		ঘূৰাই -errno;
	যদি (fchmod(pty->slave, mode))
		ঘূৰাই -errno;
	ঘূৰাই 0;
}

/* call me in child process */
শূন্য ul_pty_init_slave(সংরচনা ul_pty *pty)
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

	sigprocmask(SIG_SETMASK, &pty->orgsig, রিক্ত);

	DBG_OBJ(SETUP, pty, ul_debug("... initialize slave done"));
}

স্থির পূর্ণাঙ্ক write_output(ৱর্ণ *obuf, ssize_t bytes)
{
	DBG(IO, ul_debug(" writing output"));

	যদি (ul_write_all(STDOUT_FILENO, obuf, bytes)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		ঘূৰাই -errno;
	}

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক schedule_child_write(সংরচনা ul_pty *pty, ৱর্ণ *buf, size_t bufsz, পূর্ণাঙ্ক final)
{
	সংরচনা ul_pty_child_buffer *stash;

	যদি (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->next;
		memset(stash, 0, পরিমাণ(*stash));
	} অন্যথা
		stash = calloc(1, পরিমাণ(*stash));
	যদি (!stash)
		ঘূৰাই -1;

	প্রতিজ্ঞা(bufsz <= পরিমাণ(stash->buf));

	memcpy(stash->buf, buf, bufsz);
	stash->size = bufsz;
	stash->final_প্ৰবেশ = final ? 1 : 0;

	যদি (pty->child_buffer_head)
		pty->child_buffer_tail = pty->child_buffer_tail->next = stash;
	অন্যথা
		pty->child_buffer_head = pty->child_buffer_tail = stash;
	ঘূৰাই 0;
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
স্থির শূন্য drain_child_buffers(সংরচনা ul_pty *pty)
{
	unsigned পূর্ণাঙ্ক tries = 0;
	সংরচনা pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	যেতিয়ালৈ (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	যদি (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

স্থির পূর্ণাঙ্ক flush_child_buffers(সংরচনা ul_pty *pty, পূর্ণাঙ্ক *anything)
{
	পূর্ণাঙ্ক rc = 0, any = 0;

	যেতিয়ালৈ (pty->child_buffer_head) {
		সংরচনা ul_pty_child_buffer *hd = pty->child_buffer_head;
		ssize_t ret;

		যদি (hd->final_প্ৰবেশ)
			drain_child_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->size - hd->cursor));

		ret = লিখ(pty->master, hd->buf + hd->cursor, hd->size - hd->cursor);
		যদি (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			যদি (!(errno == EINTR || errno == EAGAIN))
				rc = -errno;
			গচ্ছ out;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		যদি (hd->cursor == hd->size) {
			pty->child_buffer_head = hd->next;
			যদি (!hd->next)
				pty->child_buffer_tail = রিক্ত;

			hd->next = pty->free_buffers;
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
	ঘূৰাই rc;
}

শূন্য ul_pty_write_eof_to_child(সংরচনা ul_pty *pty)
{
	ৱর্ণ c = DEF_EOF;
	schedule_child_write(pty, &c, পরিমাণ(ৱর্ণ), 1);
}

স্থির পূর্ণাঙ্ক mainloop_callback(সংরচনা ul_pty *pty)
{
	পূর্ণাঙ্ক rc;

	যদি (!pty->callbacks.mainloop)
		ঘূৰাই 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_data);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	ঘূৰাই rc;
}

স্থির পূর্ণাঙ্ক handle_io(সংরচনা ul_pty *pty, পূর্ণাঙ্ক fd, পূর্ণাঙ্ক *eof)
{
	ৱর্ণ buf[BUFSIZ];
	ssize_t bytes;
	পূর্ণাঙ্ক rc = 0;
	sigset_t set;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&set);
	sigaddset(&set, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &set, রিক্ত);
	/* read from active FD */
	bytes = পঠ(fd, buf, পরিমাণ(buf));
	sigprocmask(SIG_BLOCK, &set, রিক্ত);
	যদি (bytes == -1) {
		যদি (errno == EAGAIN || errno == EINTR)
			ঘূৰাই 0;
		ঘূৰাই -errno;
	}

	যদি (bytes == 0) {
		*eof = 1;
		ঘূৰাই 0;
	}

	/* from stdin (user) to command */
	যদি (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", bytes));

		যদি (schedule_child_write(pty, buf, bytes, 0))
			ঘূৰাই -errno;

	/* from command (master) to stdout */
	} অন্যথা যদি (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", bytes));
		write_output(buf, bytes);
	}

	যদি (pty->callbacks.log_stream_activity)
		rc = pty->callbacks.log_stream_activity(
					pty->callback_data, fd, buf, bytes);

	ঘূৰাই rc;
}

শূন্য ul_pty_wait_for_child(সংরচনা ul_pty *pty)
{
	পূর্ণাঙ্ক status;
	pid_t pid;
	পূর্ণাঙ্ক options = 0;

	যদি (pty->child == (pid_t) -1)
		ঘূৰাই;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (পূর্ণাঙ্ক) pty->child));

	যদি (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		বাবে (;;) {
			pid = waitpid(pty->child, &status, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (পূর্ণাঙ্ক) pid));
			যদি (pid != (pid_t) - 1) {
				যদি (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			} অন্যথা
				ভঙ্গ;
		}
	} অন্যথা {
		/* final wait */
		যেতিয়ালৈ ((pid = waitpid(-1, &status, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (পূর্ণাঙ্ক) pid));
			যদি (pid == pty->child) {
				যদি (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			}
		}
	}
}

স্থির পূর্ণাঙ্ক handle_signal(সংরচনা ul_pty *pty, পূর্ণাঙ্ক fd)
{
	সংরচনা signalfd_siginfo info;
	ssize_t bytes;
	পূর্ণাঙ্ক rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	bytes = পঠ(fd, &info, পরিমাণ(info));
	যদি (bytes != পরিমাণ(info)) {
		যদি (bytes < 0 && (errno == EAGAIN || errno == EINTR))
			ঘূৰাই 0;
		ঘূৰাই -errno;
	}

	ৱিকল্প (info.ssi_signo) {
	পক্ষ SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		যদি (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			যদি (pty->callbacks.child_wait)
				pty->callbacks.child_wait(pty->callback_data,
							  pty->child);
			অন্যথা
				ul_pty_wait_for_child(pty);

		} অন্যথা যদি (info.ssi_status == SIGSTOP && pty->child > 0) {
			pty->callbacks.child_sigstop(pty->callback_data,
						     pty->child);
		}

		যদি (pty->child <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->next_callback_time);
		}
		ঘূৰাই 0;
	পক্ষ SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		যদি (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (ৱর্ণ *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (ৱর্ণ *)&pty->win);

			যদি (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_data,
							&info, (শূন্য *) &pty->win);
		}
		ভঙ্গ;
	পক্ষ SIGALRM:
		FALLTHROUGH;
	পক্ষ SIGHUP:
		FALLTHROUGH;
	পক্ষ SIGTERM:
		FALLTHROUGH;
	পক্ষ SIGINT:
		FALLTHROUGH;
	পক্ষ SIGQUIT:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIG{ALRM,HUP,TERM,INT,QUIT}"));
		pty->delivered_signal = info.ssi_signo;
		/* Child termination is going to generate SIGCHLD (see above) */
		যদি (pty->child > 0)
			kill(pty->child, SIGTERM);

		যদি (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_data,
					&info, (শূন্য *) &pty->win);
		ভঙ্গ;
	পক্ষ SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		যদি (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_data);
		ভঙ্গ;
	স্ৱভাৱ:
		abort();
	}

	ঘূৰাই rc;
}

/* loop in parent */
পূর্ণাঙ্ক ul_pty_proxy_master(সংরচনা ul_pty *pty)
{
	পূর্ণাঙ্ক rc = 0, ret, eof = 0;
	গণনা {
		POLLFD_SIGNAL = 0,
		POLLFD_MASTER,
		POLLFD_STDIN

	};
	সংরচনা pollfd pfd[] = {
		[POLLFD_SIGNAL] = { .fd = -1,		.events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_MASTER] = { .fd = pty->master,  .events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_STDIN]	= { .fd = STDIN_FILENO, .events = POLLIN | POLLERR | POLLHUP }
	};

	/* We use signalfd, and standard signals by handlers are completely blocked */
	প্রতিজ্ঞা(pty->sigfd >= 0);

	pfd[POLLFD_SIGNAL].fd = pty->sigfd;
	pty->poll_timeout = -1;

	যেতিয়ালৈ (!pty->delivered_signal) {
		size_t i;
		পূর্ণাঙ্ক errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		যদি (timerisset(&pty->next_callback_time)) {
			সংরচনা timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			যদি (timercmp(&now, &pty->next_callback_time, >)) {
				rc = mainloop_callback(pty);
				যদি (rc)
					ভঙ্গ;
			}
		}

		/* set timeout */
		যদি (timerisset(&pty->next_callback_time)) {
			সংরচনা timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->next_callback_time, &now, &rest);
			timeout = (rest.tv_sec * MSEC_PER_SEC) +  (rest.tv_usec / USEC_PER_MSEC);
		} অন্যথা
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		যদি (pty->child_buffer_head)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		অন্যথা
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_SIZE(pfd), timeout);

		errsv = errno;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		যদি (ret < 0) {
			যদি (errsv == EAGAIN)
				অনুৱর্ত;
			rc = -errno;
			ভঙ্গ;
		}

		/* timeout */
		যদি (ret == 0) {
			যদি (timerisset(&pty->next_callback_time)) {
				rc = mainloop_callback(pty);
				যদি (rc == 0)
					অনুৱর্ত;
			} অন্যথা {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			ভঙ্গ;
		}
		/* event */
		বাবে (i = 0; i < ARRAY_SIZE(pfd); i++) {
			যদি (pfd[i].revents == 0)
				অনুৱর্ত;

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
					rc = handle_io(pty, pfd[i].fd, &eof); /* data */
				যদি (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_child_buffers(pty, রিক্ত);
			}

			যদি (rc) {
				পূর্ণাঙ্ক anything = 1;

				ul_pty_write_eof_to_child(pty);
				বাবে (anything = 1; anything;)
					flush_child_buffers(pty, &anything);
				ভঙ্গ;
			}

			যদি (i == POLLFD_SIGNAL)
				অনুৱর্ত;

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
			ভঙ্গ;
	}

	যদি (rc && pty->child && pty->child != (pid_t) -1 && !pty->delivered_signal) {
		kill(pty->child, SIGTERM);
		sleep(2);
		kill(pty->child, SIGKILL);
	}

	pty_signals_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	ঘূৰাই rc;
}

#ifdef TEST_PROGRAM_PTY
/*
 * $ make test_pty
 * $ ./test_pty
 *
 * ... and see for example tty(1) or "ps afu"
 */
স্থির শূন্য child_sigstop(শূন্য *data __attribute__((__unused__)), pid_t child)
{
	kill(getpid(), SIGSTOP);
	kill(child, SIGCONT);
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ *argv[])
{
	সংরচনা ul_pty_callbacks *cb;
	ধ্রুৱ ৱর্ণ *shell, *command = রিক্ত, *shname = রিক্ত;
	পূর্ণাঙ্ক caught_signal = 0;
	pid_t child;
	সংরচনা ul_pty *pty;

	shell = getenv("SHELL");
	যদি (shell == রিক্ত)
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

	ৱিকল্প ((পূর্ণাঙ্ক) (child = fork())) {
	পক্ষ -1: /* error */
		ul_pty_cleanup(pty);
		err(EXIT_FAILURE, "cannot create child process");
		ভঙ্গ;

	পক্ষ 0: /* child */
		ul_pty_init_slave(pty);

		সঙ্কেত(SIGTERM, SIG_DFL); /* because /etc/csh.login */

		shname = strrchr(shell, '/');
		shname = shname ? shname + 1 : shell;

		যদি (command)
			execl(shell, shname, "-c", command, (ৱর্ণ *)রিক্ত);
		অন্যথা
			execl(shell, shname, "-i", (ৱর্ণ *)রিক্ত);
		err(EXIT_FAILURE, "failed to execute %s", shell);
		ভঙ্গ;

	স্ৱভাৱ:
		ভঙ্গ;
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
	ঘূৰাই EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM */

