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
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <pty.h>
#அந்தர்பாவ <poll.h>
#அந்தர்பாவ <sys/signalfd.h>
#அந்தர்பாவ <paths.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/wait.h>
#அந்தர்பாவ <inttypes.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "all-io.h"
#அந்தர்பாவ "ttyutils.h"
#அந்தர்பாவ "pty-session.h"
#அந்தர்பாவ "monotonic.h"
#அந்தர்பாவ "debug.h"

நிலையான UL_DEBUG_DEFINE_MASK(ulpty);
UL_DEBUG_DEFINE_MASKNAMES(ulpty) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPTY_DEBUG_INIT	(1 << 1)
#define ULPTY_DEBUG_SETUP	(1 << 2)
#define ULPTY_DEBUG_SIG		(1 << 3)
#define ULPTY_DEBUG_IO		(1 << 4)
#define ULPTY_DEBUG_DONE	(1 << 5)

#define DBG(m, x)		__UL_DBG(ulpty, ULPTY_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpty, ULPTY_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpty, ULPTY_DEBUG_, m, x)

வெறுமை ul_pty_init_debug(முழு mask)
{
	எனில் (ulpty_debug_mask)
		திரும்பு;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, mask, ULPTY_DEBUG);
}

கட்டமைப்பு ul_pty *ul_new_pty(முழு is_stdin_tty)
{
	கட்டமைப்பு ul_pty *pty = calloc(1, அளவு(*pty));

	எனில் (!pty)
		திரும்பு வெற்று;

	DBG_OBJ(SETUP, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->child = (pid_t) -1;

	திரும்பு pty;
}

வெறுமை ul_free_pty(கட்டமைப்பு ul_pty *pty)
{
	கட்டமைப்பு ul_pty_child_buffer *hd;

	போது ((hd = pty->child_buffer_head)) {
		pty->child_buffer_head = hd->next;
		விடுவி(hd);
	}

	போது ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->next;
		விடுவி(hd);
	}
	விடுவி(pty);
}

வெறுமை ul_pty_slave_echo(கட்டமைப்பு ul_pty *pty, முழு enable)
{
	ப்ரதிஜ்ஞா(pty);
	pty->slave_echo = enable ? 1 : 0;
}

முழு ul_pty_get_delivered_signal(கட்டமைப்பு ul_pty *pty)
{
	ப்ரதிஜ்ஞா(pty);
	திரும்பு pty->delivered_signal;
}

கட்டமைப்பு ul_pty_callbacks *ul_pty_get_callbacks(கட்டமைப்பு ul_pty *pty)
{
	ப்ரதிஜ்ஞா(pty);
	திரும்பு &pty->callbacks;
}

வெறுமை ul_pty_set_callback_data(கட்டமைப்பு ul_pty *pty, வெறுமை *data)
{
	ப்ரதிஜ்ஞா(pty);
	pty->callback_data = data;
}

வெறுமை ul_pty_set_child(கட்டமைப்பு ul_pty *pty, pid_t child)
{
	ப்ரதிஜ்ஞா(pty);
	pty->child = child;
}

முழு ul_pty_get_childfd(கட்டமைப்பு ul_pty *pty)
{
	ப்ரதிஜ்ஞா(pty);
	திரும்பு pty->master;
}

pid_t ul_pty_get_child(கட்டமைப்பு ul_pty *pty)
{
	ப்ரதிஜ்ஞா(pty);
	திரும்பு pty->child;
}

/* it's active when signals are redirected to sigfd */
முழு ul_pty_is_running(கட்டமைப்பு ul_pty *pty)
{
	ப்ரதிஜ்ஞா(pty);
	திரும்பு pty->sigfd >= 0;
}

வெறுமை ul_pty_set_mainloop_time(கட்டமைப்பு ul_pty *pty, கட்டமைப்பு timeval *tv)
{
	ப்ரதிஜ்ஞா(pty);
	எனில் (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->next_callback_time);
	} இல்லையெனில் {
		pty->next_callback_time.tv_sec = tv->tv_sec;
		pty->next_callback_time.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

நிலையான வெறுமை pty_signals_cleanup(கட்டமைப்பு ul_pty *pty)
{
	எனில் (pty->sigfd != -1)
		close(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, வெற்று);
}

/* call me before fork() */
முழு ul_pty_setup(கட்டமைப்பு ul_pty *pty)
{
	கட்டமைப்பு termios attrs;
	முழு rc = 0;

	ப்ரதிஜ்ஞா(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, வெற்று, &pty->orgsig);

	எனில் (pty->isterm) {
		DBG_OBJ(SETUP, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		எனில் (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -errno;
			செல் done;
		}

		attrs = pty->stdin_attrs;
		எனில் (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		இல்லையெனில்
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (எழுத்து *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, வெற்று, &attrs, &pty->win);
		எனில் (rc)
			செல் done;

	} இல்லையெனில் {
		DBG_OBJ(SETUP, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, வெற்று, வெற்று, வெற்று);
		எனில் (rc)
			செல் done;

		tcgetattr(pty->slave, &attrs);

		எனில் (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		இல்லையெனில்
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

done:
	எனில் (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	திரும்பு rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
முழு ul_pty_terminal_setup(கட்டமைப்பு ul_pty *pty)
{
	கட்டமைப்பு termios attrs;

	எனில் (!pty->isterm)
		திரும்பு 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	எனில் (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		திரும்பு -errno;

	DBG_OBJ(SETUP, pty, ul_debug("terminal raw mode setup done"));
	திரும்பு 0;
}

/* call me before fork() */
முழு ul_pty_signals_setup(கட்டமைப்பு ul_pty *pty)
{
	sigset_t ourset;
	முழு rc = 0;

	ப்ரதிஜ்ஞா(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, வெற்று, &pty->orgsig);

	sigfillset(&ourset);
	எனில் (sigprocmask(SIG_BLOCK, &ourset, வெற்று)) {
		rc = -errno;
		செல் done;
	}

	sigemptyset(&ourset);
	sigaddset(&ourset, SIGCHLD);
	sigaddset(&ourset, SIGWINCH);
	sigaddset(&ourset, SIGALRM);
	sigaddset(&ourset, SIGHUP);
	sigaddset(&ourset, SIGTERM);
	sigaddset(&ourset, SIGINT);
	sigaddset(&ourset, SIGQUIT);

	எனில் (pty->callbacks.flush_logs)
		sigaddset(&ourset, SIGUSR1);

	எனில் ((pty->sigfd = signalfd(-1, &ourset, SFD_CLOEXEC)) < 0)
		rc = -errno;
done:
	எனில் (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	திரும்பு rc;
}

/* cleanup in parent process */
வெறுமை ul_pty_cleanup(கட்டமைப்பு ul_pty *pty)
{
	கட்டமைப்பு termios rtt;

	pty_signals_cleanup(pty);

	எனில் (pty->master == -1 || !pty->isterm)
		திரும்பு;

	DBG_OBJ(DONE, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

முழு ul_pty_chownmod_slave(கட்டமைப்பு ul_pty *pty, uid_t uid, gid_t gid, mode_t mode)
{
	எனில் (fchown(pty->slave, uid, gid))
		திரும்பு -errno;
	எனில் (fchmod(pty->slave, mode))
		திரும்பு -errno;
	திரும்பு 0;
}

/* call me in child process */
வெறுமை ul_pty_init_slave(கட்டமைப்பு ul_pty *pty)
{
	DBG_OBJ(SETUP, pty, ul_debug("initialize slave"));

	setsid();

	ioctl(pty->slave, TIOCSCTTY, 1);
	close(pty->master);

	dup2(pty->slave, STDIN_FILENO);
	dup2(pty->slave, STDOUT_FILENO);
	dup2(pty->slave, STDERR_FILENO);

	close(pty->slave);

	எனில் (pty->sigfd >= 0)
		close(pty->sigfd);

	pty->slave = -1;
	pty->master = -1;
	pty->sigfd = -1;

	sigprocmask(SIG_SETMASK, &pty->orgsig, வெற்று);

	DBG_OBJ(SETUP, pty, ul_debug("... initialize slave done"));
}

நிலையான முழு write_output(எழுத்து *obuf, ssize_t bytes)
{
	DBG(IO, ul_debug(" writing output"));

	எனில் (ul_write_all(STDOUT_FILENO, obuf, bytes)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		திரும்பு -errno;
	}

	திரும்பு 0;
}

நிலையான முழு schedule_child_write(கட்டமைப்பு ul_pty *pty, எழுத்து *buf, size_t bufsz, முழு final)
{
	கட்டமைப்பு ul_pty_child_buffer *stash;

	எனில் (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->next;
		memset(stash, 0, அளவு(*stash));
	} இல்லையெனில்
		stash = calloc(1, அளவு(*stash));
	எனில் (!stash)
		திரும்பு -1;

	ப்ரதிஜ்ஞா(bufsz <= அளவு(stash->buf));

	memcpy(stash->buf, buf, bufsz);
	stash->size = bufsz;
	stash->final_உள்ளீடு = final ? 1 : 0;

	எனில் (pty->child_buffer_head)
		pty->child_buffer_tail = pty->child_buffer_tail->next = stash;
	இல்லையெனில்
		pty->child_buffer_head = pty->child_buffer_tail = stash;
	திரும்பு 0;
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
நிலையான வெறுமை drain_child_buffers(கட்டமைப்பு ul_pty *pty)
{
	unsigned முழு tries = 0;
	கட்டமைப்பு pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	போது (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	எனில் (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

நிலையான முழு flush_child_buffers(கட்டமைப்பு ul_pty *pty, முழு *anything)
{
	முழு rc = 0, any = 0;

	போது (pty->child_buffer_head) {
		கட்டமைப்பு ul_pty_child_buffer *hd = pty->child_buffer_head;
		ssize_t ret;

		எனில் (hd->final_உள்ளீடு)
			drain_child_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->size - hd->cursor));

		ret = லிக(pty->master, hd->buf + hd->cursor, hd->size - hd->cursor);
		எனில் (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			எனில் (!(errno == EINTR || errno == EAGAIN))
				rc = -errno;
			செல் out;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		எனில் (hd->cursor == hd->size) {
			pty->child_buffer_head = hd->next;
			எனில் (!hd->next)
				pty->child_buffer_tail = வெற்று;

			hd->next = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
out:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	எனில் (any)
		fdatasync(pty->master);

	எனில் (anything)
		*anything = any;
	திரும்பு rc;
}

வெறுமை ul_pty_write_eof_to_child(கட்டமைப்பு ul_pty *pty)
{
	எழுத்து c = DEF_EOF;
	schedule_child_write(pty, &c, அளவு(எழுத்து), 1);
}

நிலையான முழு mainloop_callback(கட்டமைப்பு ul_pty *pty)
{
	முழு rc;

	எனில் (!pty->callbacks.mainloop)
		திரும்பு 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_data);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	திரும்பு rc;
}

நிலையான முழு handle_io(கட்டமைப்பு ul_pty *pty, முழு fd, முழு *eof)
{
	எழுத்து buf[BUFSIZ];
	ssize_t bytes;
	முழு rc = 0;
	sigset_t set;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&set);
	sigaddset(&set, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &set, வெற்று);
	/* read from active FD */
	bytes = பட(fd, buf, அளவு(buf));
	sigprocmask(SIG_BLOCK, &set, வெற்று);
	எனில் (bytes == -1) {
		எனில் (errno == EAGAIN || errno == EINTR)
			திரும்பு 0;
		திரும்பு -errno;
	}

	எனில் (bytes == 0) {
		*eof = 1;
		திரும்பு 0;
	}

	/* from stdin (user) to command */
	எனில் (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", bytes));

		எனில் (schedule_child_write(pty, buf, bytes, 0))
			திரும்பு -errno;

	/* from command (master) to stdout */
	} இல்லையெனில் எனில் (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", bytes));
		write_output(buf, bytes);
	}

	எனில் (pty->callbacks.log_stream_activity)
		rc = pty->callbacks.log_stream_activity(
					pty->callback_data, fd, buf, bytes);

	திரும்பு rc;
}

வெறுமை ul_pty_wait_for_child(கட்டமைப்பு ul_pty *pty)
{
	முழு status;
	pid_t pid;
	முழு options = 0;

	எனில் (pty->child == (pid_t) -1)
		திரும்பு;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (முழு) pty->child));

	எனில் (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		க்கு (;;) {
			pid = waitpid(pty->child, &status, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (முழு) pid));
			எனில் (pid != (pid_t) - 1) {
				எனில் (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			} இல்லையெனில்
				நிறுத்து;
		}
	} இல்லையெனில் {
		/* final wait */
		போது ((pid = waitpid(-1, &status, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (முழு) pid));
			எனில் (pid == pty->child) {
				எனில் (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			}
		}
	}
}

நிலையான முழு handle_signal(கட்டமைப்பு ul_pty *pty, முழு fd)
{
	கட்டமைப்பு signalfd_siginfo info;
	ssize_t bytes;
	முழு rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	bytes = பட(fd, &info, அளவு(info));
	எனில் (bytes != அளவு(info)) {
		எனில் (bytes < 0 && (errno == EAGAIN || errno == EINTR))
			திரும்பு 0;
		திரும்பு -errno;
	}

	தேர்வு (info.ssi_signo) {
	நிலை SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		எனில் (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			எனில் (pty->callbacks.child_wait)
				pty->callbacks.child_wait(pty->callback_data,
							  pty->child);
			இல்லையெனில்
				ul_pty_wait_for_child(pty);

		} இல்லையெனில் எனில் (info.ssi_status == SIGSTOP && pty->child > 0) {
			pty->callbacks.child_sigstop(pty->callback_data,
						     pty->child);
		}

		எனில் (pty->child <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->next_callback_time);
		}
		திரும்பு 0;
	நிலை SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		எனில் (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (எழுத்து *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (எழுத்து *)&pty->win);

			எனில் (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_data,
							&info, (வெறுமை *) &pty->win);
		}
		நிறுத்து;
	நிலை SIGALRM:
		FALLTHROUGH;
	நிலை SIGHUP:
		FALLTHROUGH;
	நிலை SIGTERM:
		FALLTHROUGH;
	நிலை SIGINT:
		FALLTHROUGH;
	நிலை SIGQUIT:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIG{ALRM,HUP,TERM,INT,QUIT}"));
		pty->delivered_signal = info.ssi_signo;
		/* Child termination is going to generate SIGCHLD (see above) */
		எனில் (pty->child > 0)
			kill(pty->child, SIGTERM);

		எனில் (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_data,
					&info, (வெறுமை *) &pty->win);
		நிறுத்து;
	நிலை SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		எனில் (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_data);
		நிறுத்து;
	இயல்பு:
		abort();
	}

	திரும்பு rc;
}

/* loop in parent */
முழு ul_pty_proxy_master(கட்டமைப்பு ul_pty *pty)
{
	முழு rc = 0, ret, eof = 0;
	எண்ணிக்கை {
		POLLFD_SIGNAL = 0,
		POLLFD_MASTER,
		POLLFD_STDIN

	};
	கட்டமைப்பு pollfd pfd[] = {
		[POLLFD_SIGNAL] = { .fd = -1,		.events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_MASTER] = { .fd = pty->master,  .events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_STDIN]	= { .fd = STDIN_FILENO, .events = POLLIN | POLLERR | POLLHUP }
	};

	/* We use signalfd, and standard signals by handlers are completely blocked */
	ப்ரதிஜ்ஞா(pty->sigfd >= 0);

	pfd[POLLFD_SIGNAL].fd = pty->sigfd;
	pty->poll_timeout = -1;

	போது (!pty->delivered_signal) {
		size_t i;
		முழு errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		எனில் (timerisset(&pty->next_callback_time)) {
			கட்டமைப்பு timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			எனில் (timercmp(&now, &pty->next_callback_time, >)) {
				rc = mainloop_callback(pty);
				எனில் (rc)
					நிறுத்து;
			}
		}

		/* set timeout */
		எனில் (timerisset(&pty->next_callback_time)) {
			கட்டமைப்பு timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->next_callback_time, &now, &rest);
			timeout = (rest.tv_sec * MSEC_PER_SEC) +  (rest.tv_usec / USEC_PER_MSEC);
		} இல்லையெனில்
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		எனில் (pty->child_buffer_head)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		இல்லையெனில்
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_SIZE(pfd), timeout);

		errsv = errno;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		எனில் (ret < 0) {
			எனில் (errsv == EAGAIN)
				தொடர்;
			rc = -errno;
			நிறுத்து;
		}

		/* timeout */
		எனில் (ret == 0) {
			எனில் (timerisset(&pty->next_callback_time)) {
				rc = mainloop_callback(pty);
				எனில் (rc == 0)
					தொடர்;
			} இல்லையெனில் {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			நிறுத்து;
		}
		/* event */
		க்கு (i = 0; i < ARRAY_SIZE(pfd); i++) {
			எனில் (pfd[i].revents == 0)
				தொடர்;

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

			எனில் (i == POLLFD_SIGNAL)
				rc = handle_signal(pty, pfd[i].fd);
			இல்லையெனில் {
				எனில் (pfd[i].revents & POLLIN)
					rc = handle_io(pty, pfd[i].fd, &eof); /* data */
				எனில் (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_child_buffers(pty, வெற்று);
			}

			எனில் (rc) {
				முழு anything = 1;

				ul_pty_write_eof_to_child(pty);
				க்கு (anything = 1; anything;)
					flush_child_buffers(pty, &anything);
				நிறுத்து;
			}

			எனில் (i == POLLFD_SIGNAL)
				தொடர்;

			/* EOF maybe detected in two ways; they are as follows:
			 *	A) poll() return POLLHUP event after close()
			 *	B) read() returns 0 (no data)
			 *
			 * POLLNVAL means that fd is closed.
			 */
			எனில் ((pfd[i].revents & POLLHUP) || (pfd[i].revents & POLLNVAL) || eof) {
				DBG_OBJ(IO, pty, ul_debug(" ignore FD"));
				எனில் (i == POLLFD_STDIN) {
					pfd[i].fd = -1;
					ul_pty_write_eof_to_child(pty);
				} இல்லையெனில் /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		எனில் (rc)
			நிறுத்து;
	}

	எனில் (rc && pty->child && pty->child != (pid_t) -1 && !pty->delivered_signal) {
		kill(pty->child, SIGTERM);
		sleep(2);
		kill(pty->child, SIGKILL);
	}

	pty_signals_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	திரும்பு rc;
}

#ifdef TEST_PROGRAM_PTY
/*
 * $ make test_pty
 * $ ./test_pty
 *
 * ... and see for example tty(1) or "ps afu"
 */
நிலையான வெறுமை child_sigstop(வெறுமை *data __attribute__((__unused__)), pid_t child)
{
	kill(getpid(), SIGSTOP);
	kill(child, SIGCONT);
}

முழு முதன்மை(முழு argc, எழுத்து *argv[])
{
	கட்டமைப்பு ul_pty_callbacks *cb;
	மாறிலி எழுத்து *shell, *command = வெற்று, *shname = வெற்று;
	முழு caught_signal = 0;
	pid_t child;
	கட்டமைப்பு ul_pty *pty;

	shell = getenv("SHELL");
	எனில் (shell == வெற்று)
		shell = _PATH_BSHELL;
	எனில் (argc == 2)
		command = argv[1];

	ul_pty_init_debug(0);

	pty = ul_new_pty(isatty(STDIN_FILENO));
	எனில் (!pty)
		err(EXIT_FAILURE, "failed to allocate PTY handler");

	cb = ul_pty_get_callbacks(pty);
	cb->child_sigstop = child_sigstop;

	எனில் (ul_pty_setup(pty))
		err(EXIT_FAILURE, "failed to create pseudo-terminal");
	எனில் (ul_pty_signals_setup(pty))
		err(EXIT_FAILURE, "failed to initialize signals handler");
	எனில் (ul_pty_terminal_setup(pty))
		err(EXIT_FAILURE, "failed to setup terminal");

	fflush(stdout);			/* ??? */

	தேர்வு ((முழு) (child = fork())) {
	நிலை -1: /* error */
		ul_pty_cleanup(pty);
		err(EXIT_FAILURE, "cannot create child process");
		நிறுத்து;

	நிலை 0: /* child */
		ul_pty_init_slave(pty);

		சமிக்ஞை(SIGTERM, SIG_DFL); /* because /etc/csh.login */

		shname = strrchr(shell, '/');
		shname = shname ? shname + 1 : shell;

		எனில் (command)
			execl(shell, shname, "-c", command, (எழுத்து *)வெற்று);
		இல்லையெனில்
			execl(shell, shname, "-i", (எழுத்து *)வெற்று);
		err(EXIT_FAILURE, "failed to execute %s", shell);
		நிறுத்து;

	இயல்பு:
		நிறுத்து;
	}

	/* parent */
	ul_pty_set_child(pty, child);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	எனில் (!caught_signal && ul_pty_get_child(pty) != (pid_t)-1)
		ul_pty_wait_for_child(pty);	/* final wait */

	எனில் (caught_signal && ul_pty_get_child(pty) != (pid_t)-1) {
		fprintf(stderr, "\nSession terminated, killing shell...");
		kill(child, SIGTERM);
		sleep(2);
		kill(child, SIGKILL);
		fprintf(stderr, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	திரும்பு EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM */

