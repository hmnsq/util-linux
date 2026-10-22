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
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <pty.h>
#ಅನ್ತರ್ಭಾವ <poll.h>
#ಅನ್ತರ್ಭಾವ <sys/signalfd.h>
#ಅನ್ತರ್ಭಾವ <paths.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/wait.h>
#ಅನ್ತರ್ಭಾವ <inttypes.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "all-io.h"
#ಅನ್ತರ್ಭಾವ "ttyutils.h"
#ಅನ್ತರ್ಭಾವ "pty-session.h"
#ಅನ್ತರ್ಭಾವ "monotonic.h"
#ಅನ್ತರ್ಭಾವ "debug.h"

ಸ್ಥಿರ UL_DEBUG_DEFINE_MASK(ulpty);
UL_DEBUG_DEFINE_MASKNAMES(ulpty) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPTY_DEBUG_INIT	(1 << 1)
#define ULPTY_DEBUG_SETUP	(1 << 2)
#define ULPTY_DEBUG_SIG		(1 << 3)
#define ULPTY_DEBUG_IO		(1 << 4)
#define ULPTY_DEBUG_DONE	(1 << 5)

#define DBG(m, x)		__UL_DBG(ulpty, ULPTY_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpty, ULPTY_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpty, ULPTY_DEBUG_, m, x)

ಶೂನ್ಯ ul_pty_init_debug(ಪೂರ್ಣಾಙ್ಕ mask)
{
	ಯದಿ (ulpty_debug_mask)
		ಪ್ರತ್ಯಾಗಮ;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, mask, ULPTY_DEBUG);
}

ಸಂರಚನಾ ul_pty *ul_new_pty(ಪೂರ್ಣಾಙ್ಕ is_stdin_tty)
{
	ಸಂರಚನಾ ul_pty *pty = calloc(1, ಪರಿಮಾಣ(*pty));

	ಯದಿ (!pty)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	DBG_OBJ(SETUP, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->child = (pid_t) -1;

	ಪ್ರತ್ಯಾಗಮ pty;
}

ಶೂನ್ಯ ul_free_pty(ಸಂರಚನಾ ul_pty *pty)
{
	ಸಂರಚನಾ ul_pty_child_buffer *hd;

	ಯಾವತ್ ((hd = pty->child_buffer_head)) {
		pty->child_buffer_head = hd->next;
		ಮುಕ್ತ(hd);
	}

	ಯಾವತ್ ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->next;
		ಮುಕ್ತ(hd);
	}
	ಮುಕ್ತ(pty);
}

ಶೂನ್ಯ ul_pty_slave_echo(ಸಂರಚನಾ ul_pty *pty, ಪೂರ್ಣಾಙ್ಕ enable)
{
	ಪ್ರತಿಜ್ಞಾ(pty);
	pty->slave_echo = enable ? 1 : 0;
}

ಪೂರ್ಣಾಙ್ಕ ul_pty_get_delivered_signal(ಸಂರಚನಾ ul_pty *pty)
{
	ಪ್ರತಿಜ್ಞಾ(pty);
	ಪ್ರತ್ಯಾಗಮ pty->delivered_signal;
}

ಸಂರಚನಾ ul_pty_callbacks *ul_pty_get_callbacks(ಸಂರಚನಾ ul_pty *pty)
{
	ಪ್ರತಿಜ್ಞಾ(pty);
	ಪ್ರತ್ಯಾಗಮ &pty->callbacks;
}

ಶೂನ್ಯ ul_pty_set_callback_data(ಸಂರಚನಾ ul_pty *pty, ಶೂನ್ಯ *data)
{
	ಪ್ರತಿಜ್ಞಾ(pty);
	pty->callback_data = data;
}

ಶೂನ್ಯ ul_pty_set_child(ಸಂರಚನಾ ul_pty *pty, pid_t child)
{
	ಪ್ರತಿಜ್ಞಾ(pty);
	pty->child = child;
}

ಪೂರ್ಣಾಙ್ಕ ul_pty_get_childfd(ಸಂರಚನಾ ul_pty *pty)
{
	ಪ್ರತಿಜ್ಞಾ(pty);
	ಪ್ರತ್ಯಾಗಮ pty->master;
}

pid_t ul_pty_get_child(ಸಂರಚನಾ ul_pty *pty)
{
	ಪ್ರತಿಜ್ಞಾ(pty);
	ಪ್ರತ್ಯಾಗಮ pty->child;
}

/* it's active when signals are redirected to sigfd */
ಪೂರ್ಣಾಙ್ಕ ul_pty_is_running(ಸಂರಚನಾ ul_pty *pty)
{
	ಪ್ರತಿಜ್ಞಾ(pty);
	ಪ್ರತ್ಯಾಗಮ pty->sigfd >= 0;
}

ಶೂನ್ಯ ul_pty_set_mainloop_time(ಸಂರಚನಾ ul_pty *pty, ಸಂರಚನಾ timeval *tv)
{
	ಪ್ರತಿಜ್ಞಾ(pty);
	ಯದಿ (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->next_callback_time);
	} ಅನ್ಯಥಾ {
		pty->next_callback_time.tv_sec = tv->tv_sec;
		pty->next_callback_time.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ pty_signals_cleanup(ಸಂರಚನಾ ul_pty *pty)
{
	ಯದಿ (pty->sigfd != -1)
		close(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, ರಿಕ್ತ);
}

/* call me before fork() */
ಪೂರ್ಣಾಙ್ಕ ul_pty_setup(ಸಂರಚನಾ ul_pty *pty)
{
	ಸಂರಚನಾ termios attrs;
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	ಪ್ರತಿಜ್ಞಾ(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, ರಿಕ್ತ, &pty->orgsig);

	ಯದಿ (pty->isterm) {
		DBG_OBJ(SETUP, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		ಯದಿ (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -errno;
			ಗಚ್ಛ done;
		}

		attrs = pty->stdin_attrs;
		ಯದಿ (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		ಅನ್ಯಥಾ
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (ವರ್ಣ *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, ರಿಕ್ತ, &attrs, &pty->win);
		ಯದಿ (rc)
			ಗಚ್ಛ done;

	} ಅನ್ಯಥಾ {
		DBG_OBJ(SETUP, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ);
		ಯದಿ (rc)
			ಗಚ್ಛ done;

		tcgetattr(pty->slave, &attrs);

		ಯದಿ (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		ಅನ್ಯಥಾ
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

done:
	ಯದಿ (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	ಪ್ರತ್ಯಾಗಮ rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
ಪೂರ್ಣಾಙ್ಕ ul_pty_terminal_setup(ಸಂರಚನಾ ul_pty *pty)
{
	ಸಂರಚನಾ termios attrs;

	ಯದಿ (!pty->isterm)
		ಪ್ರತ್ಯಾಗಮ 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	ಯದಿ (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		ಪ್ರತ್ಯಾಗಮ -errno;

	DBG_OBJ(SETUP, pty, ul_debug("terminal raw mode setup done"));
	ಪ್ರತ್ಯಾಗಮ 0;
}

/* call me before fork() */
ಪೂರ್ಣಾಙ್ಕ ul_pty_signals_setup(ಸಂರಚನಾ ul_pty *pty)
{
	sigset_t ourset;
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	ಪ್ರತಿಜ್ಞಾ(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, ರಿಕ್ತ, &pty->orgsig);

	sigfillset(&ourset);
	ಯದಿ (sigprocmask(SIG_BLOCK, &ourset, ರಿಕ್ತ)) {
		rc = -errno;
		ಗಚ್ಛ done;
	}

	sigemptyset(&ourset);
	sigaddset(&ourset, SIGCHLD);
	sigaddset(&ourset, SIGWINCH);
	sigaddset(&ourset, SIGALRM);
	sigaddset(&ourset, SIGHUP);
	sigaddset(&ourset, SIGTERM);
	sigaddset(&ourset, SIGINT);
	sigaddset(&ourset, SIGQUIT);

	ಯದಿ (pty->callbacks.flush_logs)
		sigaddset(&ourset, SIGUSR1);

	ಯದಿ ((pty->sigfd = signalfd(-1, &ourset, SFD_CLOEXEC)) < 0)
		rc = -errno;
done:
	ಯದಿ (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	ಪ್ರತ್ಯಾಗಮ rc;
}

/* cleanup in parent process */
ಶೂನ್ಯ ul_pty_cleanup(ಸಂರಚನಾ ul_pty *pty)
{
	ಸಂರಚನಾ termios rtt;

	pty_signals_cleanup(pty);

	ಯದಿ (pty->master == -1 || !pty->isterm)
		ಪ್ರತ್ಯಾಗಮ;

	DBG_OBJ(DONE, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

ಪೂರ್ಣಾಙ್ಕ ul_pty_chownmod_slave(ಸಂರಚನಾ ul_pty *pty, uid_t uid, gid_t gid, mode_t mode)
{
	ಯದಿ (fchown(pty->slave, uid, gid))
		ಪ್ರತ್ಯಾಗಮ -errno;
	ಯದಿ (fchmod(pty->slave, mode))
		ಪ್ರತ್ಯಾಗಮ -errno;
	ಪ್ರತ್ಯಾಗಮ 0;
}

/* call me in child process */
ಶೂನ್ಯ ul_pty_init_slave(ಸಂರಚನಾ ul_pty *pty)
{
	DBG_OBJ(SETUP, pty, ul_debug("initialize slave"));

	setsid();

	ioctl(pty->slave, TIOCSCTTY, 1);
	close(pty->master);

	dup2(pty->slave, STDIN_FILENO);
	dup2(pty->slave, STDOUT_FILENO);
	dup2(pty->slave, STDERR_FILENO);

	close(pty->slave);

	ಯದಿ (pty->sigfd >= 0)
		close(pty->sigfd);

	pty->slave = -1;
	pty->master = -1;
	pty->sigfd = -1;

	sigprocmask(SIG_SETMASK, &pty->orgsig, ರಿಕ್ತ);

	DBG_OBJ(SETUP, pty, ul_debug("... initialize slave done"));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ write_output(ವರ್ಣ *obuf, ssize_t bytes)
{
	DBG(IO, ul_debug(" writing output"));

	ಯದಿ (ul_write_all(STDOUT_FILENO, obuf, bytes)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		ಪ್ರತ್ಯಾಗಮ -errno;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ schedule_child_write(ಸಂರಚನಾ ul_pty *pty, ವರ್ಣ *buf, size_t bufsz, ಪೂರ್ಣಾಙ್ಕ final)
{
	ಸಂರಚನಾ ul_pty_child_buffer *stash;

	ಯದಿ (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->next;
		memset(stash, 0, ಪರಿಮಾಣ(*stash));
	} ಅನ್ಯಥಾ
		stash = calloc(1, ಪರಿಮಾಣ(*stash));
	ಯದಿ (!stash)
		ಪ್ರತ್ಯಾಗಮ -1;

	ಪ್ರತಿಜ್ಞಾ(bufsz <= ಪರಿಮಾಣ(stash->buf));

	memcpy(stash->buf, buf, bufsz);
	stash->size = bufsz;
	stash->final_ಒಳಹರಿವು = final ? 1 : 0;

	ಯದಿ (pty->child_buffer_head)
		pty->child_buffer_tail = pty->child_buffer_tail->next = stash;
	ಅನ್ಯಥಾ
		pty->child_buffer_head = pty->child_buffer_tail = stash;
	ಪ್ರತ್ಯಾಗಮ 0;
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
ಸ್ಥಿರ ಶೂನ್ಯ drain_child_buffers(ಸಂರಚನಾ ul_pty *pty)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ tries = 0;
	ಸಂರಚನಾ pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	ಯಾವತ್ (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	ಯದಿ (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ flush_child_buffers(ಸಂರಚನಾ ul_pty *pty, ಪೂರ್ಣಾಙ್ಕ *anything)
{
	ಪೂರ್ಣಾಙ್ಕ rc = 0, any = 0;

	ಯಾವತ್ (pty->child_buffer_head) {
		ಸಂರಚನಾ ul_pty_child_buffer *hd = pty->child_buffer_head;
		ssize_t ret;

		ಯದಿ (hd->final_ಒಳಹರಿವು)
			drain_child_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->size - hd->cursor));

		ret = ಲಿಖ(pty->master, hd->buf + hd->cursor, hd->size - hd->cursor);
		ಯದಿ (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			ಯದಿ (!(errno == EINTR || errno == EAGAIN))
				rc = -errno;
			ಗಚ್ಛ out;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		ಯದಿ (hd->cursor == hd->size) {
			pty->child_buffer_head = hd->next;
			ಯದಿ (!hd->next)
				pty->child_buffer_tail = ರಿಕ್ತ;

			hd->next = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
out:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	ಯದಿ (any)
		fdatasync(pty->master);

	ಯದಿ (anything)
		*anything = any;
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಶೂನ್ಯ ul_pty_write_eof_to_child(ಸಂರಚನಾ ul_pty *pty)
{
	ವರ್ಣ c = DEF_EOF;
	schedule_child_write(pty, &c, ಪರಿಮಾಣ(ವರ್ಣ), 1);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ mainloop_callback(ಸಂರಚನಾ ul_pty *pty)
{
	ಪೂರ್ಣಾಙ್ಕ rc;

	ಯದಿ (!pty->callbacks.mainloop)
		ಪ್ರತ್ಯಾಗಮ 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_data);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ handle_io(ಸಂರಚನಾ ul_pty *pty, ಪೂರ್ಣಾಙ್ಕ fd, ಪೂರ್ಣಾಙ್ಕ *eof)
{
	ವರ್ಣ buf[BUFSIZ];
	ssize_t bytes;
	ಪೂರ್ಣಾಙ್ಕ rc = 0;
	sigset_t set;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&set);
	sigaddset(&set, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &set, ರಿಕ್ತ);
	/* read from active FD */
	bytes = ಪಠ(fd, buf, ಪರಿಮಾಣ(buf));
	sigprocmask(SIG_BLOCK, &set, ರಿಕ್ತ);
	ಯದಿ (bytes == -1) {
		ಯದಿ (errno == EAGAIN || errno == EINTR)
			ಪ್ರತ್ಯಾಗಮ 0;
		ಪ್ರತ್ಯಾಗಮ -errno;
	}

	ಯದಿ (bytes == 0) {
		*eof = 1;
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	/* from stdin (user) to command */
	ಯದಿ (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", bytes));

		ಯದಿ (schedule_child_write(pty, buf, bytes, 0))
			ಪ್ರತ್ಯಾಗಮ -errno;

	/* from command (master) to stdout */
	} ಅನ್ಯಥಾ ಯದಿ (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", bytes));
		write_output(buf, bytes);
	}

	ಯದಿ (pty->callbacks.log_stream_activity)
		rc = pty->callbacks.log_stream_activity(
					pty->callback_data, fd, buf, bytes);

	ಪ್ರತ್ಯಾಗಮ rc;
}

ಶೂನ್ಯ ul_pty_wait_for_child(ಸಂರಚನಾ ul_pty *pty)
{
	ಪೂರ್ಣಾಙ್ಕ status;
	pid_t pid;
	ಪೂರ್ಣಾಙ್ಕ options = 0;

	ಯದಿ (pty->child == (pid_t) -1)
		ಪ್ರತ್ಯಾಗಮ;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (ಪೂರ್ಣಾಙ್ಕ) pty->child));

	ಯದಿ (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		ಪ್ರತಿ (;;) {
			pid = waitpid(pty->child, &status, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (ಪೂರ್ಣಾಙ್ಕ) pid));
			ಯದಿ (pid != (pid_t) - 1) {
				ಯದಿ (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			} ಅನ್ಯಥಾ
				ಭಙ್ಗ;
		}
	} ಅನ್ಯಥಾ {
		/* final wait */
		ಯಾವತ್ ((pid = waitpid(-1, &status, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (ಪೂರ್ಣಾಙ್ಕ) pid));
			ಯದಿ (pid == pty->child) {
				ಯದಿ (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			}
		}
	}
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ handle_signal(ಸಂರಚನಾ ul_pty *pty, ಪೂರ್ಣಾಙ್ಕ fd)
{
	ಸಂರಚನಾ signalfd_siginfo info;
	ssize_t bytes;
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	bytes = ಪಠ(fd, &info, ಪರಿಮಾಣ(info));
	ಯದಿ (bytes != ಪರಿಮಾಣ(info)) {
		ಯದಿ (bytes < 0 && (errno == EAGAIN || errno == EINTR))
			ಪ್ರತ್ಯಾಗಮ 0;
		ಪ್ರತ್ಯಾಗಮ -errno;
	}

	ವಿಕಲ್ಪ (info.ssi_signo) {
	ಪಕ್ಷ SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		ಯದಿ (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			ಯದಿ (pty->callbacks.child_wait)
				pty->callbacks.child_wait(pty->callback_data,
							  pty->child);
			ಅನ್ಯಥಾ
				ul_pty_wait_for_child(pty);

		} ಅನ್ಯಥಾ ಯದಿ (info.ssi_status == SIGSTOP && pty->child > 0) {
			pty->callbacks.child_sigstop(pty->callback_data,
						     pty->child);
		}

		ಯದಿ (pty->child <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->next_callback_time);
		}
		ಪ್ರತ್ಯಾಗಮ 0;
	ಪಕ್ಷ SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		ಯದಿ (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (ವರ್ಣ *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (ವರ್ಣ *)&pty->win);

			ಯದಿ (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_data,
							&info, (ಶೂನ್ಯ *) &pty->win);
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ SIGALRM:
		FALLTHROUGH;
	ಪಕ್ಷ SIGHUP:
		FALLTHROUGH;
	ಪಕ್ಷ SIGTERM:
		FALLTHROUGH;
	ಪಕ್ಷ SIGINT:
		FALLTHROUGH;
	ಪಕ್ಷ SIGQUIT:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIG{ALRM,HUP,TERM,INT,QUIT}"));
		pty->delivered_signal = info.ssi_signo;
		/* Child termination is going to generate SIGCHLD (see above) */
		ಯದಿ (pty->child > 0)
			kill(pty->child, SIGTERM);

		ಯದಿ (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_data,
					&info, (ಶೂನ್ಯ *) &pty->win);
		ಭಙ್ಗ;
	ಪಕ್ಷ SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		ಯದಿ (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_data);
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		abort();
	}

	ಪ್ರತ್ಯಾಗಮ rc;
}

/* loop in parent */
ಪೂರ್ಣಾಙ್ಕ ul_pty_proxy_master(ಸಂರಚನಾ ul_pty *pty)
{
	ಪೂರ್ಣಾಙ್ಕ rc = 0, ret, eof = 0;
	ಗಣನಾ {
		POLLFD_SIGNAL = 0,
		POLLFD_MASTER,
		POLLFD_STDIN

	};
	ಸಂರಚನಾ pollfd pfd[] = {
		[POLLFD_SIGNAL] = { .fd = -1,		.events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_MASTER] = { .fd = pty->master,  .events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_STDIN]	= { .fd = STDIN_FILENO, .events = POLLIN | POLLERR | POLLHUP }
	};

	/* We use signalfd, and standard signals by handlers are completely blocked */
	ಪ್ರತಿಜ್ಞಾ(pty->sigfd >= 0);

	pfd[POLLFD_SIGNAL].fd = pty->sigfd;
	pty->poll_timeout = -1;

	ಯಾವತ್ (!pty->delivered_signal) {
		size_t i;
		ಪೂರ್ಣಾಙ್ಕ errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		ಯದಿ (timerisset(&pty->next_callback_time)) {
			ಸಂರಚನಾ timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			ಯದಿ (timercmp(&now, &pty->next_callback_time, >)) {
				rc = mainloop_callback(pty);
				ಯದಿ (rc)
					ಭಙ್ಗ;
			}
		}

		/* set timeout */
		ಯದಿ (timerisset(&pty->next_callback_time)) {
			ಸಂರಚನಾ timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->next_callback_time, &now, &rest);
			timeout = (rest.tv_sec * MSEC_PER_SEC) +  (rest.tv_usec / USEC_PER_MSEC);
		} ಅನ್ಯಥಾ
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		ಯದಿ (pty->child_buffer_head)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		ಅನ್ಯಥಾ
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_SIZE(pfd), timeout);

		errsv = errno;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		ಯದಿ (ret < 0) {
			ಯದಿ (errsv == EAGAIN)
				ಅನುವರ್ತ;
			rc = -errno;
			ಭಙ್ಗ;
		}

		/* timeout */
		ಯದಿ (ret == 0) {
			ಯದಿ (timerisset(&pty->next_callback_time)) {
				rc = mainloop_callback(pty);
				ಯದಿ (rc == 0)
					ಅನುವರ್ತ;
			} ಅನ್ಯಥಾ {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			ಭಙ್ಗ;
		}
		/* event */
		ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(pfd); i++) {
			ಯದಿ (pfd[i].revents == 0)
				ಅನುವರ್ತ;

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

			ಯದಿ (i == POLLFD_SIGNAL)
				rc = handle_signal(pty, pfd[i].fd);
			ಅನ್ಯಥಾ {
				ಯದಿ (pfd[i].revents & POLLIN)
					rc = handle_io(pty, pfd[i].fd, &eof); /* data */
				ಯದಿ (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_child_buffers(pty, ರಿಕ್ತ);
			}

			ಯದಿ (rc) {
				ಪೂರ್ಣಾಙ್ಕ anything = 1;

				ul_pty_write_eof_to_child(pty);
				ಪ್ರತಿ (anything = 1; anything;)
					flush_child_buffers(pty, &anything);
				ಭಙ್ಗ;
			}

			ಯದಿ (i == POLLFD_SIGNAL)
				ಅನುವರ್ತ;

			/* EOF maybe detected in two ways; they are as follows:
			 *	A) poll() return POLLHUP event after close()
			 *	B) read() returns 0 (no data)
			 *
			 * POLLNVAL means that fd is closed.
			 */
			ಯದಿ ((pfd[i].revents & POLLHUP) || (pfd[i].revents & POLLNVAL) || eof) {
				DBG_OBJ(IO, pty, ul_debug(" ignore FD"));
				ಯದಿ (i == POLLFD_STDIN) {
					pfd[i].fd = -1;
					ul_pty_write_eof_to_child(pty);
				} ಅನ್ಯಥಾ /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		ಯದಿ (rc)
			ಭಙ್ಗ;
	}

	ಯದಿ (rc && pty->child && pty->child != (pid_t) -1 && !pty->delivered_signal) {
		kill(pty->child, SIGTERM);
		sleep(2);
		kill(pty->child, SIGKILL);
	}

	pty_signals_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	ಪ್ರತ್ಯಾಗಮ rc;
}

#ifdef TEST_PROGRAM_PTY
/*
 * $ make test_pty
 * $ ./test_pty
 *
 * ... and see for example tty(1) or "ps afu"
 */
ಸ್ಥಿರ ಶೂನ್ಯ child_sigstop(ಶೂನ್ಯ *data __attribute__((__unused__)), pid_t child)
{
	kill(getpid(), SIGSTOP);
	kill(child, SIGCONT);
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ಸಂರಚನಾ ul_pty_callbacks *cb;
	ಧ್ರುವ ವರ್ಣ *shell, *command = ರಿಕ್ತ, *shname = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ caught_signal = 0;
	pid_t child;
	ಸಂರಚನಾ ul_pty *pty;

	shell = getenv("SHELL");
	ಯದಿ (shell == ರಿಕ್ತ)
		shell = _PATH_BSHELL;
	ಯದಿ (argc == 2)
		command = argv[1];

	ul_pty_init_debug(0);

	pty = ul_new_pty(isatty(STDIN_FILENO));
	ಯದಿ (!pty)
		err(EXIT_FAILURE, "failed to allocate PTY handler");

	cb = ul_pty_get_callbacks(pty);
	cb->child_sigstop = child_sigstop;

	ಯದಿ (ul_pty_setup(pty))
		err(EXIT_FAILURE, "failed to create pseudo-terminal");
	ಯದಿ (ul_pty_signals_setup(pty))
		err(EXIT_FAILURE, "failed to initialize signals handler");
	ಯದಿ (ul_pty_terminal_setup(pty))
		err(EXIT_FAILURE, "failed to setup terminal");

	fflush(stdout);			/* ??? */

	ವಿಕಲ್ಪ ((ಪೂರ್ಣಾಙ್ಕ) (child = fork())) {
	ಪಕ್ಷ -1: /* error */
		ul_pty_cleanup(pty);
		err(EXIT_FAILURE, "cannot create child process");
		ಭಙ್ಗ;

	ಪಕ್ಷ 0: /* child */
		ul_pty_init_slave(pty);

		ಸಙ್ಕೇತ(SIGTERM, SIG_DFL); /* because /etc/csh.login */

		shname = strrchr(shell, '/');
		shname = shname ? shname + 1 : shell;

		ಯದಿ (command)
			execl(shell, shname, "-c", command, (ವರ್ಣ *)ರಿಕ್ತ);
		ಅನ್ಯಥಾ
			execl(shell, shname, "-i", (ವರ್ಣ *)ರಿಕ್ತ);
		err(EXIT_FAILURE, "failed to execute %s", shell);
		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* parent */
	ul_pty_set_child(pty, child);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	ಯದಿ (!caught_signal && ul_pty_get_child(pty) != (pid_t)-1)
		ul_pty_wait_for_child(pty);	/* final wait */

	ಯದಿ (caught_signal && ul_pty_get_child(pty) != (pid_t)-1) {
		fprintf(stderr, "\nSession terminated, killing shell...");
		kill(child, SIGTERM);
		sleep(2);
		kill(child, SIGKILL);
		fprintf(stderr, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM */

