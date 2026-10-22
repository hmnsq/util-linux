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
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <pty.h>
#અન્તર્ભાવ <poll.h>
#અન્તર્ભાવ <sys/signalfd.h>
#અન્તર્ભાવ <paths.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/wait.h>
#અન્તર્ભાવ <inttypes.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "all-io.h"
#અન્તર્ભાવ "ttyutils.h"
#અન્તર્ભાવ "pty-session.h"
#અન્તર્ભાવ "monotonic.h"
#અન્તર્ભાવ "debug.h"

સ્થિર UL_DEBUG_DEFINE_MASK(ulpty);
UL_DEBUG_DEFINE_MASKNAMES(ulpty) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPTY_DEBUG_INIT	(1 << 1)
#define ULPTY_DEBUG_SETUP	(1 << 2)
#define ULPTY_DEBUG_SIG		(1 << 3)
#define ULPTY_DEBUG_IO		(1 << 4)
#define ULPTY_DEBUG_DONE	(1 << 5)

#define DBG(m, x)		__UL_DBG(ulpty, ULPTY_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpty, ULPTY_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpty, ULPTY_DEBUG_, m, x)

શૂન્ય ul_pty_init_debug(પૂર્ણાંક mask)
{
	જો (ulpty_debug_mask)
		પાછા;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, mask, ULPTY_DEBUG);
}

સંરચના ul_pty *ul_new_pty(પૂર્ણાંક is_stdin_tty)
{
	સંરચના ul_pty *pty = calloc(1, કદ(*pty));

	જો (!pty)
		પાછા ખાલી;

	DBG_OBJ(SETUP, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->child = (pid_t) -1;

	પાછા pty;
}

શૂન્ય ul_free_pty(સંરચના ul_pty *pty)
{
	સંરચના ul_pty_child_buffer *hd;

	જ્યારે ((hd = pty->child_buffer_head)) {
		pty->child_buffer_head = hd->next;
		મુક્ત(hd);
	}

	જ્યારે ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->next;
		મુક્ત(hd);
	}
	મુક્ત(pty);
}

શૂન્ય ul_pty_slave_echo(સંરચના ul_pty *pty, પૂર્ણાંક enable)
{
	પ્રતિજ્ઞા(pty);
	pty->slave_echo = enable ? 1 : 0;
}

પૂર્ણાંક ul_pty_get_delivered_signal(સંરચના ul_pty *pty)
{
	પ્રતિજ્ઞા(pty);
	પાછા pty->delivered_signal;
}

સંરચના ul_pty_callbacks *ul_pty_get_callbacks(સંરચના ul_pty *pty)
{
	પ્રતિજ્ઞા(pty);
	પાછા &pty->callbacks;
}

શૂન્ય ul_pty_set_callback_data(સંરચના ul_pty *pty, શૂન્ય *data)
{
	પ્રતિજ્ઞા(pty);
	pty->callback_data = data;
}

શૂન્ય ul_pty_set_child(સંરચના ul_pty *pty, pid_t child)
{
	પ્રતિજ્ઞા(pty);
	pty->child = child;
}

પૂર્ણાંક ul_pty_get_childfd(સંરચના ul_pty *pty)
{
	પ્રતિજ્ઞા(pty);
	પાછા pty->master;
}

pid_t ul_pty_get_child(સંરચના ul_pty *pty)
{
	પ્રતિજ્ઞા(pty);
	પાછા pty->child;
}

/* it's active when signals are redirected to sigfd */
પૂર્ણાંક ul_pty_is_running(સંરચના ul_pty *pty)
{
	પ્રતિજ્ઞા(pty);
	પાછા pty->sigfd >= 0;
}

શૂન્ય ul_pty_set_mainloop_time(સંરચના ul_pty *pty, સંરચના timeval *tv)
{
	પ્રતિજ્ઞા(pty);
	જો (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->next_callback_time);
	} નહીંતર {
		pty->next_callback_time.tv_sec = tv->tv_sec;
		pty->next_callback_time.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

સ્થિર શૂન્ય pty_signals_cleanup(સંરચના ul_pty *pty)
{
	જો (pty->sigfd != -1)
		close(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, ખાલી);
}

/* call me before fork() */
પૂર્ણાંક ul_pty_setup(સંરચના ul_pty *pty)
{
	સંરચના termios attrs;
	પૂર્ણાંક rc = 0;

	પ્રતિજ્ઞા(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, ખાલી, &pty->orgsig);

	જો (pty->isterm) {
		DBG_OBJ(SETUP, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		જો (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -errno;
			જાઓ done;
		}

		attrs = pty->stdin_attrs;
		જો (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		નહીંતર
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (અક્ષર *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, ખાલી, &attrs, &pty->win);
		જો (rc)
			જાઓ done;

	} નહીંતર {
		DBG_OBJ(SETUP, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, ખાલી, ખાલી, ખાલી);
		જો (rc)
			જાઓ done;

		tcgetattr(pty->slave, &attrs);

		જો (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		નહીંતર
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

done:
	જો (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	પાછા rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
પૂર્ણાંક ul_pty_terminal_setup(સંરચના ul_pty *pty)
{
	સંરચના termios attrs;

	જો (!pty->isterm)
		પાછા 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	જો (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		પાછા -errno;

	DBG_OBJ(SETUP, pty, ul_debug("terminal raw mode setup done"));
	પાછા 0;
}

/* call me before fork() */
પૂર્ણાંક ul_pty_signals_setup(સંરચના ul_pty *pty)
{
	sigset_t ourset;
	પૂર્ણાંક rc = 0;

	પ્રતિજ્ઞા(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, ખાલી, &pty->orgsig);

	sigfillset(&ourset);
	જો (sigprocmask(SIG_BLOCK, &ourset, ખાલી)) {
		rc = -errno;
		જાઓ done;
	}

	sigemptyset(&ourset);
	sigaddset(&ourset, SIGCHLD);
	sigaddset(&ourset, SIGWINCH);
	sigaddset(&ourset, SIGALRM);
	sigaddset(&ourset, SIGHUP);
	sigaddset(&ourset, SIGTERM);
	sigaddset(&ourset, SIGINT);
	sigaddset(&ourset, SIGQUIT);

	જો (pty->callbacks.flush_logs)
		sigaddset(&ourset, SIGUSR1);

	જો ((pty->sigfd = signalfd(-1, &ourset, SFD_CLOEXEC)) < 0)
		rc = -errno;
done:
	જો (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	પાછા rc;
}

/* cleanup in parent process */
શૂન્ય ul_pty_cleanup(સંરચના ul_pty *pty)
{
	સંરચના termios rtt;

	pty_signals_cleanup(pty);

	જો (pty->master == -1 || !pty->isterm)
		પાછા;

	DBG_OBJ(DONE, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

પૂર્ણાંક ul_pty_chownmod_slave(સંરચના ul_pty *pty, uid_t uid, gid_t gid, mode_t mode)
{
	જો (fchown(pty->slave, uid, gid))
		પાછા -errno;
	જો (fchmod(pty->slave, mode))
		પાછા -errno;
	પાછા 0;
}

/* call me in child process */
શૂન્ય ul_pty_init_slave(સંરચના ul_pty *pty)
{
	DBG_OBJ(SETUP, pty, ul_debug("initialize slave"));

	setsid();

	ioctl(pty->slave, TIOCSCTTY, 1);
	close(pty->master);

	dup2(pty->slave, STDIN_FILENO);
	dup2(pty->slave, STDOUT_FILENO);
	dup2(pty->slave, STDERR_FILENO);

	close(pty->slave);

	જો (pty->sigfd >= 0)
		close(pty->sigfd);

	pty->slave = -1;
	pty->master = -1;
	pty->sigfd = -1;

	sigprocmask(SIG_SETMASK, &pty->orgsig, ખાલી);

	DBG_OBJ(SETUP, pty, ul_debug("... initialize slave done"));
}

સ્થિર પૂર્ણાંક write_output(અક્ષર *obuf, ssize_t bytes)
{
	DBG(IO, ul_debug(" writing output"));

	જો (ul_write_all(STDOUT_FILENO, obuf, bytes)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		પાછા -errno;
	}

	પાછા 0;
}

સ્થિર પૂર્ણાંક schedule_child_write(સંરચના ul_pty *pty, અક્ષર *buf, size_t bufsz, પૂર્ણાંક final)
{
	સંરચના ul_pty_child_buffer *stash;

	જો (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->next;
		memset(stash, 0, કદ(*stash));
	} નહીંતર
		stash = calloc(1, કદ(*stash));
	જો (!stash)
		પાછા -1;

	પ્રતિજ્ઞા(bufsz <= કદ(stash->buf));

	memcpy(stash->buf, buf, bufsz);
	stash->size = bufsz;
	stash->final_આગત = final ? 1 : 0;

	જો (pty->child_buffer_head)
		pty->child_buffer_tail = pty->child_buffer_tail->next = stash;
	નહીંતર
		pty->child_buffer_head = pty->child_buffer_tail = stash;
	પાછા 0;
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
સ્થિર શૂન્ય drain_child_buffers(સંરચના ul_pty *pty)
{
	unsigned પૂર્ણાંક tries = 0;
	સંરચના pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	જ્યારે (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	જો (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

સ્થિર પૂર્ણાંક flush_child_buffers(સંરચના ul_pty *pty, પૂર્ણાંક *anything)
{
	પૂર્ણાંક rc = 0, any = 0;

	જ્યારે (pty->child_buffer_head) {
		સંરચના ul_pty_child_buffer *hd = pty->child_buffer_head;
		ssize_t ret;

		જો (hd->final_આગત)
			drain_child_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->size - hd->cursor));

		ret = લિખ(pty->master, hd->buf + hd->cursor, hd->size - hd->cursor);
		જો (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			જો (!(errno == EINTR || errno == EAGAIN))
				rc = -errno;
			જાઓ out;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		જો (hd->cursor == hd->size) {
			pty->child_buffer_head = hd->next;
			જો (!hd->next)
				pty->child_buffer_tail = ખાલી;

			hd->next = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
out:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	જો (any)
		fdatasync(pty->master);

	જો (anything)
		*anything = any;
	પાછા rc;
}

શૂન્ય ul_pty_write_eof_to_child(સંરચના ul_pty *pty)
{
	અક્ષર c = DEF_EOF;
	schedule_child_write(pty, &c, કદ(અક્ષર), 1);
}

સ્થિર પૂર્ણાંક mainloop_callback(સંરચના ul_pty *pty)
{
	પૂર્ણાંક rc;

	જો (!pty->callbacks.mainloop)
		પાછા 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_data);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	પાછા rc;
}

સ્થિર પૂર્ણાંક handle_io(સંરચના ul_pty *pty, પૂર્ણાંક fd, પૂર્ણાંક *eof)
{
	અક્ષર buf[BUFSIZ];
	ssize_t bytes;
	પૂર્ણાંક rc = 0;
	sigset_t set;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&set);
	sigaddset(&set, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &set, ખાલી);
	/* read from active FD */
	bytes = પઠ(fd, buf, કદ(buf));
	sigprocmask(SIG_BLOCK, &set, ખાલી);
	જો (bytes == -1) {
		જો (errno == EAGAIN || errno == EINTR)
			પાછા 0;
		પાછા -errno;
	}

	જો (bytes == 0) {
		*eof = 1;
		પાછા 0;
	}

	/* from stdin (user) to command */
	જો (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", bytes));

		જો (schedule_child_write(pty, buf, bytes, 0))
			પાછા -errno;

	/* from command (master) to stdout */
	} નહીંતર જો (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", bytes));
		write_output(buf, bytes);
	}

	જો (pty->callbacks.log_stream_activity)
		rc = pty->callbacks.log_stream_activity(
					pty->callback_data, fd, buf, bytes);

	પાછા rc;
}

શૂન્ય ul_pty_wait_for_child(સંરચના ul_pty *pty)
{
	પૂર્ણાંક status;
	pid_t pid;
	પૂર્ણાંક options = 0;

	જો (pty->child == (pid_t) -1)
		પાછા;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (પૂર્ણાંક) pty->child));

	જો (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		માટે (;;) {
			pid = waitpid(pty->child, &status, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (પૂર્ણાંક) pid));
			જો (pid != (pid_t) - 1) {
				જો (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			} નહીંતર
				તોડો;
		}
	} નહીંતર {
		/* final wait */
		જ્યારે ((pid = waitpid(-1, &status, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (પૂર્ણાંક) pid));
			જો (pid == pty->child) {
				જો (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			}
		}
	}
}

સ્થિર પૂર્ણાંક handle_signal(સંરચના ul_pty *pty, પૂર્ણાંક fd)
{
	સંરચના signalfd_siginfo info;
	ssize_t bytes;
	પૂર્ણાંક rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	bytes = પઠ(fd, &info, કદ(info));
	જો (bytes != કદ(info)) {
		જો (bytes < 0 && (errno == EAGAIN || errno == EINTR))
			પાછા 0;
		પાછા -errno;
	}

	પસંદગી (info.ssi_signo) {
	સ્થિતિ SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		જો (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			જો (pty->callbacks.child_wait)
				pty->callbacks.child_wait(pty->callback_data,
							  pty->child);
			નહીંતર
				ul_pty_wait_for_child(pty);

		} નહીંતર જો (info.ssi_status == SIGSTOP && pty->child > 0) {
			pty->callbacks.child_sigstop(pty->callback_data,
						     pty->child);
		}

		જો (pty->child <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->next_callback_time);
		}
		પાછા 0;
	સ્થિતિ SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		જો (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (અક્ષર *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (અક્ષર *)&pty->win);

			જો (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_data,
							&info, (શૂન્ય *) &pty->win);
		}
		તોડો;
	સ્થિતિ SIGALRM:
		FALLTHROUGH;
	સ્થિતિ SIGHUP:
		FALLTHROUGH;
	સ્થિતિ SIGTERM:
		FALLTHROUGH;
	સ્થિતિ SIGINT:
		FALLTHROUGH;
	સ્થિતિ SIGQUIT:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIG{ALRM,HUP,TERM,INT,QUIT}"));
		pty->delivered_signal = info.ssi_signo;
		/* Child termination is going to generate SIGCHLD (see above) */
		જો (pty->child > 0)
			kill(pty->child, SIGTERM);

		જો (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_data,
					&info, (શૂન્ય *) &pty->win);
		તોડો;
	સ્થિતિ SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		જો (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_data);
		તોડો;
	મૂળભૂત:
		abort();
	}

	પાછા rc;
}

/* loop in parent */
પૂર્ણાંક ul_pty_proxy_master(સંરચના ul_pty *pty)
{
	પૂર્ણાંક rc = 0, ret, eof = 0;
	ગણના {
		POLLFD_SIGNAL = 0,
		POLLFD_MASTER,
		POLLFD_STDIN

	};
	સંરચના pollfd pfd[] = {
		[POLLFD_SIGNAL] = { .fd = -1,		.events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_MASTER] = { .fd = pty->master,  .events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_STDIN]	= { .fd = STDIN_FILENO, .events = POLLIN | POLLERR | POLLHUP }
	};

	/* We use signalfd, and standard signals by handlers are completely blocked */
	પ્રતિજ્ઞા(pty->sigfd >= 0);

	pfd[POLLFD_SIGNAL].fd = pty->sigfd;
	pty->poll_timeout = -1;

	જ્યારે (!pty->delivered_signal) {
		size_t i;
		પૂર્ણાંક errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		જો (timerisset(&pty->next_callback_time)) {
			સંરચના timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			જો (timercmp(&now, &pty->next_callback_time, >)) {
				rc = mainloop_callback(pty);
				જો (rc)
					તોડો;
			}
		}

		/* set timeout */
		જો (timerisset(&pty->next_callback_time)) {
			સંરચના timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->next_callback_time, &now, &rest);
			timeout = (rest.tv_sec * MSEC_PER_SEC) +  (rest.tv_usec / USEC_PER_MSEC);
		} નહીંતર
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		જો (pty->child_buffer_head)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		નહીંતર
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_SIZE(pfd), timeout);

		errsv = errno;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		જો (ret < 0) {
			જો (errsv == EAGAIN)
				ચાલુ;
			rc = -errno;
			તોડો;
		}

		/* timeout */
		જો (ret == 0) {
			જો (timerisset(&pty->next_callback_time)) {
				rc = mainloop_callback(pty);
				જો (rc == 0)
					ચાલુ;
			} નહીંતર {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			તોડો;
		}
		/* event */
		માટે (i = 0; i < ARRAY_SIZE(pfd); i++) {
			જો (pfd[i].revents == 0)
				ચાલુ;

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

			જો (i == POLLFD_SIGNAL)
				rc = handle_signal(pty, pfd[i].fd);
			નહીંતર {
				જો (pfd[i].revents & POLLIN)
					rc = handle_io(pty, pfd[i].fd, &eof); /* data */
				જો (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_child_buffers(pty, ખાલી);
			}

			જો (rc) {
				પૂર્ણાંક anything = 1;

				ul_pty_write_eof_to_child(pty);
				માટે (anything = 1; anything;)
					flush_child_buffers(pty, &anything);
				તોડો;
			}

			જો (i == POLLFD_SIGNAL)
				ચાલુ;

			/* EOF maybe detected in two ways; they are as follows:
			 *	A) poll() return POLLHUP event after close()
			 *	B) read() returns 0 (no data)
			 *
			 * POLLNVAL means that fd is closed.
			 */
			જો ((pfd[i].revents & POLLHUP) || (pfd[i].revents & POLLNVAL) || eof) {
				DBG_OBJ(IO, pty, ul_debug(" ignore FD"));
				જો (i == POLLFD_STDIN) {
					pfd[i].fd = -1;
					ul_pty_write_eof_to_child(pty);
				} નહીંતર /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		જો (rc)
			તોડો;
	}

	જો (rc && pty->child && pty->child != (pid_t) -1 && !pty->delivered_signal) {
		kill(pty->child, SIGTERM);
		sleep(2);
		kill(pty->child, SIGKILL);
	}

	pty_signals_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	પાછા rc;
}

#ifdef TEST_PROGRAM_PTY
/*
 * $ make test_pty
 * $ ./test_pty
 *
 * ... and see for example tty(1) or "ps afu"
 */
સ્થિર શૂન્ય child_sigstop(શૂન્ય *data __attribute__((__unused__)), pid_t child)
{
	kill(getpid(), SIGSTOP);
	kill(child, SIGCONT);
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
	સંરચના ul_pty_callbacks *cb;
	અચલ અક્ષર *shell, *command = ખાલી, *shname = ખાલી;
	પૂર્ણાંક caught_signal = 0;
	pid_t child;
	સંરચના ul_pty *pty;

	shell = getenv("SHELL");
	જો (shell == ખાલી)
		shell = _PATH_BSHELL;
	જો (argc == 2)
		command = argv[1];

	ul_pty_init_debug(0);

	pty = ul_new_pty(isatty(STDIN_FILENO));
	જો (!pty)
		err(EXIT_FAILURE, "failed to allocate PTY handler");

	cb = ul_pty_get_callbacks(pty);
	cb->child_sigstop = child_sigstop;

	જો (ul_pty_setup(pty))
		err(EXIT_FAILURE, "failed to create pseudo-terminal");
	જો (ul_pty_signals_setup(pty))
		err(EXIT_FAILURE, "failed to initialize signals handler");
	જો (ul_pty_terminal_setup(pty))
		err(EXIT_FAILURE, "failed to setup terminal");

	fflush(stdout);			/* ??? */

	પસંદગી ((પૂર્ણાંક) (child = fork())) {
	સ્થિતિ -1: /* error */
		ul_pty_cleanup(pty);
		err(EXIT_FAILURE, "cannot create child process");
		તોડો;

	સ્થિતિ 0: /* child */
		ul_pty_init_slave(pty);

		સંકેત(SIGTERM, SIG_DFL); /* because /etc/csh.login */

		shname = strrchr(shell, '/');
		shname = shname ? shname + 1 : shell;

		જો (command)
			execl(shell, shname, "-c", command, (અક્ષર *)ખાલી);
		નહીંતર
			execl(shell, shname, "-i", (અક્ષર *)ખાલી);
		err(EXIT_FAILURE, "failed to execute %s", shell);
		તોડો;

	મૂળભૂત:
		તોડો;
	}

	/* parent */
	ul_pty_set_child(pty, child);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	જો (!caught_signal && ul_pty_get_child(pty) != (pid_t)-1)
		ul_pty_wait_for_child(pty);	/* final wait */

	જો (caught_signal && ul_pty_get_child(pty) != (pid_t)-1) {
		fprintf(stderr, "\nSession terminated, killing shell...");
		kill(child, SIGTERM);
		sleep(2);
		kill(child, SIGKILL);
		fprintf(stderr, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	પાછા EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM */

