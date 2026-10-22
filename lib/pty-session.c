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
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <pty.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <poll.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/signalfd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <paths.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/wait.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <inttypes.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "all-io.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ttyutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "pty-session.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "monotonic.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "debug.h"

ਸਥਿਰ UL_DEBUG_DEFINE_ਨਕਾਬ(ulpty);
UL_DEBUG_DEFINE_MASKNAMES(ulpty) = UL_DEBUG_ਖਾਲੀ_ਥਾਵਾਂ_MASKNAMES;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULPTY_DEBUG_INIT	(1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULPTY_DEBUG_ਤਿਆਰੀ	(1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULPTY_DEBUG_SIG		(1 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULPTY_DEBUG_IO		(1 << 4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULPTY_DEBUG_ਪੂਰਾ	(1 << 5)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG(m, x)		__UL_DBG(ulpty, ULPTY_DEBUG_, m, x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpty, ULPTY_DEBUG_, m, h, x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ON_DBG(m, x)		__UL_DBG_CALL(ulpty, ULPTY_DEBUG_, m, x)

ਖਾਲੀ ul_pty_init_debug(ਪੂਰਨ ਨਕਾਬ)
{
	ਜੇ (ulpty_debug_ਨਕਾਬ)
		ਵਾਪਸ;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, ਨਕਾਬ, ULPTY_DEBUG);
}

ਬਣਤਰ ul_pty *ul_new_pty(ਪੂਰਨ is_stdin_tty)
{
	ਬਣਤਰ ul_pty *pty = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*pty));

	ਜੇ (!pty)
		ਵਾਪਸ ਨਿਰਾ;

	DBG_OBJ(ਤਿਆਰੀ, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->ਬੱਚਾ = (pid_t) -1;

	ਵਾਪਸ pty;
}

ਖਾਲੀ ul_free_pty(ਬਣਤਰ ul_pty *pty)
{
	ਬਣਤਰ ul_pty_ਬੱਚਾ_ਬਫਰ *hd;

	ਜਦੋਂ ((hd = pty->ਬੱਚਾ_ਬਫਰ_ਸਿਰ)) {
		pty->ਬੱਚਾ_ਬਫਰ_ਸਿਰ = hd->ਅਗਲਾ;
		ਮੁਕਤ(hd);
	}

	ਜਦੋਂ ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->ਅਗਲਾ;
		ਮੁਕਤ(hd);
	}
	ਮੁਕਤ(pty);
}

ਖਾਲੀ ul_pty_slave_echo(ਬਣਤਰ ul_pty *pty, ਪੂਰਨ enable)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(pty);
	pty->slave_echo = enable ? 1 : 0;
}

ਪੂਰਨ ul_pty_get_delivered_signal(ਬਣਤਰ ul_pty *pty)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(pty);
	ਵਾਪਸ pty->delivered_signal;
}

ਬਣਤਰ ul_pty_callbacks *ul_pty_get_callbacks(ਬਣਤਰ ul_pty *pty)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(pty);
	ਵਾਪਸ &pty->callbacks;
}

ਖਾਲੀ ul_pty_ਸੈੱਟ_callback_ਅੰਕੜੇ(ਬਣਤਰ ul_pty *pty, ਖਾਲੀ *ਅੰਕੜੇ)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(pty);
	pty->callback_ਅੰਕੜੇ = ਅੰਕੜੇ;
}

ਖਾਲੀ ul_pty_ਸੈੱਟ_ਬੱਚਾ(ਬਣਤਰ ul_pty *pty, pid_t ਬੱਚਾ)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(pty);
	pty->ਬੱਚਾ = ਬੱਚਾ;
}

ਪੂਰਨ ul_pty_get_childfd(ਬਣਤਰ ul_pty *pty)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(pty);
	ਵਾਪਸ pty->master;
}

pid_t ul_pty_get_ਬੱਚਾ(ਬਣਤਰ ul_pty *pty)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(pty);
	ਵਾਪਸ pty->ਬੱਚਾ;
}

/* it's active when signals are redirected to sigfd */
ਪੂਰਨ ul_pty_is_running(ਬਣਤਰ ul_pty *pty)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(pty);
	ਵਾਪਸ pty->sigfd >= 0;
}

ਖਾਲੀ ul_pty_ਸੈੱਟ_mainloop_ਸਮਾਂ(ਬਣਤਰ ul_pty *pty, ਬਣਤਰ timeval *tv)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(pty);
	ਜੇ (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->ਅਗਲਾ_callback_ਸਮਾਂ);
	} ਨਹੀਂਤਾਂ {
		pty->ਅਗਲਾ_callback_ਸਮਾਂ.tv_sec = tv->tv_sec;
		pty->ਅਗਲਾ_callback_ਸਮਾਂ.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

ਸਥਿਰ ਖਾਲੀ pty_ਸੰਕੇਤ_cleanup(ਬਣਤਰ ul_pty *pty)
{
	ਜੇ (pty->sigfd != -1)
		close(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, ਨਿਰਾ);
}

/* call me before fork() */
ਪੂਰਨ ul_pty_ਤਿਆਰੀ(ਬਣਤਰ ul_pty *pty)
{
	ਬਣਤਰ termios attrs;
	ਪੂਰਨ rc = 0;

	ਪ੍ਰਤਿਜ੍ਞਾ(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, ਨਿਰਾ, &pty->orgsig);

	ਜੇ (pty->isterm) {
		DBG_OBJ(ਤਿਆਰੀ, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		ਜੇ (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -errno;
			ਜਾਓ ਪੂਰਾ;
		}

		attrs = pty->stdin_attrs;
		ਜੇ (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		ਨਹੀਂਤਾਂ
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (ਅੱਖਰ *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, ਨਿਰਾ, &attrs, &pty->win);
		ਜੇ (rc)
			ਜਾਓ ਪੂਰਾ;

	} ਨਹੀਂਤਾਂ {
		DBG_OBJ(ਤਿਆਰੀ, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ);
		ਜੇ (rc)
			ਜਾਓ ਪੂਰਾ;

		tcgetattr(pty->slave, &attrs);

		ਜੇ (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		ਨਹੀਂਤਾਂ
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

ਪੂਰਾ:
	ਜੇ (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(ਤਿਆਰੀ, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	ਵਾਪਸ rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
ਪੂਰਨ ul_pty_terminal_ਤਿਆਰੀ(ਬਣਤਰ ul_pty *pty)
{
	ਬਣਤਰ termios attrs;

	ਜੇ (!pty->isterm)
		ਵਾਪਸ 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	ਜੇ (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		ਵਾਪਸ -errno;

	DBG_OBJ(ਤਿਆਰੀ, pty, ul_debug("terminal raw mode setup done"));
	ਵਾਪਸ 0;
}

/* call me before fork() */
ਪੂਰਨ ul_pty_ਸੰਕੇਤ_ਤਿਆਰੀ(ਬਣਤਰ ul_pty *pty)
{
	sigset_t ourset;
	ਪੂਰਨ rc = 0;

	ਪ੍ਰਤਿਜ੍ਞਾ(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, ਨਿਰਾ, &pty->orgsig);

	sigfillset(&ourset);
	ਜੇ (sigprocmask(SIG_BLOCK, &ourset, ਨਿਰਾ)) {
		rc = -errno;
		ਜਾਓ ਪੂਰਾ;
	}

	sigemptyset(&ourset);
	sigaddset(&ourset, SIGCHLD);
	sigaddset(&ourset, SIGWINCH);
	sigaddset(&ourset, SIGALRM);
	sigaddset(&ourset, SIGHUP);
	sigaddset(&ourset, SIGTERM);
	sigaddset(&ourset, SIGINT);
	sigaddset(&ourset, SIGQUIT);

	ਜੇ (pty->callbacks.flush_logs)
		sigaddset(&ourset, SIGUSR1);

	ਜੇ ((pty->sigfd = signalfd(-1, &ourset, SFD_CLOEXEC)) < 0)
		rc = -errno;
ਪੂਰਾ:
	ਜੇ (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(ਤਿਆਰੀ, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	ਵਾਪਸ rc;
}

/* cleanup in parent process */
ਖਾਲੀ ul_pty_cleanup(ਬਣਤਰ ul_pty *pty)
{
	ਬਣਤਰ termios rtt;

	pty_ਸੰਕੇਤ_cleanup(pty);

	ਜੇ (pty->master == -1 || !pty->isterm)
		ਵਾਪਸ;

	DBG_OBJ(ਪੂਰਾ_2, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

ਪੂਰਨ ul_pty_chownmod_slave(ਬਣਤਰ ul_pty *pty, uid_t uid, gid_t gid, mode_t ਬਹੁਲਕ)
{
	ਜੇ (fchown(pty->slave, uid, gid))
		ਵਾਪਸ -errno;
	ਜੇ (fchmod(pty->slave, ਬਹੁਲਕ))
		ਵਾਪਸ -errno;
	ਵਾਪਸ 0;
}

/* call me in child process */
ਖਾਲੀ ul_pty_init_slave(ਬਣਤਰ ul_pty *pty)
{
	DBG_OBJ(ਤਿਆਰੀ, pty, ul_debug("initialize slave"));

	setsid();

	ioctl(pty->slave, TIOCSCTTY, 1);
	close(pty->master);

	dup2(pty->slave, STDIN_FILENO);
	dup2(pty->slave, STDOUT_FILENO);
	dup2(pty->slave, STDERR_FILENO);

	close(pty->slave);

	ਜੇ (pty->sigfd >= 0)
		close(pty->sigfd);

	pty->slave = -1;
	pty->master = -1;
	pty->sigfd = -1;

	sigprocmask(SIG_SETMASK, &pty->orgsig, ਨਿਰਾ);

	DBG_OBJ(ਤਿਆਰੀ, pty, ul_debug("... initialize slave done"));
}

ਸਥਿਰ ਪੂਰਨ write_output(ਅੱਖਰ *obuf, ssize_t ਬਾਈਟਾਂ)
{
	DBG(IO, ul_debug(" writing output"));

	ਜੇ (ul_write_all(STDOUT_FILENO, obuf, ਬਾਈਟਾਂ)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		ਵਾਪਸ -errno;
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ ਸਮਾਂ_ਸਾਰਣੀ_ਬੱਚਾ_write(ਬਣਤਰ ul_pty *pty, ਅੱਖਰ *buf, size_t bufsz, ਪੂਰਨ final)
{
	ਬਣਤਰ ul_pty_ਬੱਚਾ_ਬਫਰ *stash;

	ਜੇ (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->ਅਗਲਾ;
		memset(stash, 0, ਆਕਾਰ(*stash));
	} ਨਹੀਂਤਾਂ
		stash = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*stash));
	ਜੇ (!stash)
		ਵਾਪਸ -1;

	ਪ੍ਰਤਿਜ੍ਞਾ(bufsz <= ਆਕਾਰ(stash->buf));

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(stash->buf, buf, bufsz);
	stash->ਮਾਪ = bufsz;
	stash->final_ਨਿਵੇਸ਼ = final ? 1 : 0;

	ਜੇ (pty->ਬੱਚਾ_ਬਫਰ_ਸਿਰ)
		pty->ਬੱਚਾ_ਬਫਰ_ਪੂਛ = pty->ਬੱਚਾ_ਬਫਰ_ਪੂਛ->ਅਗਲਾ = stash;
	ਨਹੀਂਤਾਂ
		pty->ਬੱਚਾ_ਬਫਰ_ਸਿਰ = pty->ਬੱਚਾ_ਬਫਰ_ਪੂਛ = stash;
	ਵਾਪਸ 0;
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
ਸਥਿਰ ਖਾਲੀ drain_ਬੱਚਾ_buffers(ਬਣਤਰ ul_pty *pty)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ tries = 0;
	ਬਣਤਰ pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	ਜਦੋਂ (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	ਜੇ (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

ਸਥਿਰ ਪੂਰਨ flush_ਬੱਚਾ_buffers(ਬਣਤਰ ul_pty *pty, ਪੂਰਨ *anything)
{
	ਪੂਰਨ rc = 0, any = 0;

	ਜਦੋਂ (pty->ਬੱਚਾ_ਬਫਰ_ਸਿਰ) {
		ਬਣਤਰ ul_pty_ਬੱਚਾ_ਬਫਰ *hd = pty->ਬੱਚਾ_ਬਫਰ_ਸਿਰ;
		ssize_t ret;

		ਜੇ (hd->final_ਨਿਵੇਸ਼)
			drain_ਬੱਚਾ_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->ਮਾਪ - hd->cursor));

		ret = ਲਿਖ(pty->master, hd->buf + hd->cursor, hd->ਮਾਪ - hd->cursor);
		ਜੇ (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			ਜੇ (!(errno == EINTR || errno == EAGAIN))
				rc = -errno;
			ਜਾਓ ਬਾਹਰਲਾ;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		ਜੇ (hd->cursor == hd->ਮਾਪ) {
			pty->ਬੱਚਾ_ਬਫਰ_ਸਿਰ = hd->ਅਗਲਾ;
			ਜੇ (!hd->ਅਗਲਾ)
				pty->ਬੱਚਾ_ਬਫਰ_ਪੂਛ = ਨਿਰਾ;

			hd->ਅਗਲਾ = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
ਬਾਹਰਲਾ:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	ਜੇ (any)
		fdatasync(pty->master);

	ਜੇ (anything)
		*anything = any;
	ਵਾਪਸ rc;
}

ਖਾਲੀ ul_pty_write_eof_ਮੰਜ਼ਿਲ_ਬੱਚਾ(ਬਣਤਰ ul_pty *pty)
{
	ਅੱਖਰ c = DEF_EOF;
	ਸਮਾਂ_ਸਾਰਣੀ_ਬੱਚਾ_write(pty, &c, ਆਕਾਰ(ਅੱਖਰ), 1);
}

ਸਥਿਰ ਪੂਰਨ mainloop_callback(ਬਣਤਰ ul_pty *pty)
{
	ਪੂਰਨ rc;

	ਜੇ (!pty->callbacks.mainloop)
		ਵਾਪਸ 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_ਅੰਕੜੇ);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ handle_io(ਬਣਤਰ ul_pty *pty, ਪੂਰਨ fd, ਪੂਰਨ *eof)
{
	ਅੱਖਰ buf[BUFSIZ];
	ssize_t ਬਾਈਟਾਂ;
	ਪੂਰਨ rc = 0;
	sigset_t ਸੈੱਟ;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&ਸੈੱਟ);
	sigaddset(&ਸੈੱਟ, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &ਸੈੱਟ, ਨਿਰਾ);
	/* read from active FD */
	ਬਾਈਟਾਂ = ਪਠ(fd, buf, ਆਕਾਰ(buf));
	sigprocmask(SIG_BLOCK, &ਸੈੱਟ, ਨਿਰਾ);
	ਜੇ (ਬਾਈਟਾਂ == -1) {
		ਜੇ (errno == EAGAIN || errno == EINTR)
			ਵਾਪਸ 0;
		ਵਾਪਸ -errno;
	}

	ਜੇ (ਬਾਈਟਾਂ == 0) {
		*eof = 1;
		ਵਾਪਸ 0;
	}

	/* from stdin (user) to command */
	ਜੇ (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", ਬਾਈਟਾਂ));

		ਜੇ (ਸਮਾਂ_ਸਾਰਣੀ_ਬੱਚਾ_write(pty, buf, ਬਾਈਟਾਂ, 0))
			ਵਾਪਸ -errno;

	/* from command (master) to stdout */
	} ਨਹੀਂਤਾਂ ਜੇ (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", ਬਾਈਟਾਂ));
		write_output(buf, ਬਾਈਟਾਂ);
	}

	ਜੇ (pty->callbacks.log_ਧਾਰਾ_activity)
		rc = pty->callbacks.log_ਧਾਰਾ_activity(
					pty->callback_ਅੰਕੜੇ, fd, buf, ਬਾਈਟਾਂ);

	ਵਾਪਸ rc;
}

ਖਾਲੀ ul_pty_wait_for_ਬੱਚਾ(ਬਣਤਰ ul_pty *pty)
{
	ਪੂਰਨ ਸਥਿਤੀ;
	pid_t pid;
	ਪੂਰਨ options = 0;

	ਜੇ (pty->ਬੱਚਾ == (pid_t) -1)
		ਵਾਪਸ;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (ਪੂਰਨ) pty->ਬੱਚਾ));

	ਜੇ (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		ਲਈ (;;) {
			pid = waitpid(pty->ਬੱਚਾ, &ਸਥਿਤੀ, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (ਪੂਰਨ) pid));
			ਜੇ (pid != (pid_t) - 1) {
				ਜੇ (pty->callbacks.ਬੱਚਾ_die)
					pty->callbacks.ਬੱਚਾ_die(
							pty->callback_ਅੰਕੜੇ,
							pty->ਬੱਚਾ, ਸਥਿਤੀ);
				ul_pty_ਸੈੱਟ_ਬੱਚਾ(pty, (pid_t) -1);
			} ਨਹੀਂਤਾਂ
				ਰੋਕੋ;
		}
	} ਨਹੀਂਤਾਂ {
		/* final wait */
		ਜਦੋਂ ((pid = waitpid(-1, &ਸਥਿਤੀ, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (ਪੂਰਨ) pid));
			ਜੇ (pid == pty->ਬੱਚਾ) {
				ਜੇ (pty->callbacks.ਬੱਚਾ_die)
					pty->callbacks.ਬੱਚਾ_die(
							pty->callback_ਅੰਕੜੇ,
							pty->ਬੱਚਾ, ਸਥਿਤੀ);
				ul_pty_ਸੈੱਟ_ਬੱਚਾ(pty, (pid_t) -1);
			}
		}
	}
}

ਸਥਿਰ ਪੂਰਨ handle_signal(ਬਣਤਰ ul_pty *pty, ਪੂਰਨ fd)
{
	ਬਣਤਰ signalfd_siginfo info;
	ssize_t ਬਾਈਟਾਂ;
	ਪੂਰਨ rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	ਬਾਈਟਾਂ = ਪਠ(fd, &info, ਆਕਾਰ(info));
	ਜੇ (ਬਾਈਟਾਂ != ਆਕਾਰ(info)) {
		ਜੇ (ਬਾਈਟਾਂ < 0 && (errno == EAGAIN || errno == EINTR))
			ਵਾਪਸ 0;
		ਵਾਪਸ -errno;
	}

	ਚੋਣ (info.ssi_signo) {
	ਹਾਲਤ SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		ਜੇ (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			ਜੇ (pty->callbacks.ਬੱਚਾ_wait)
				pty->callbacks.ਬੱਚਾ_wait(pty->callback_ਅੰਕੜੇ,
							  pty->ਬੱਚਾ);
			ਨਹੀਂਤਾਂ
				ul_pty_wait_for_ਬੱਚਾ(pty);

		} ਨਹੀਂਤਾਂ ਜੇ (info.ssi_status == SIGSTOP && pty->ਬੱਚਾ > 0) {
			pty->callbacks.ਬੱਚਾ_sigstop(pty->callback_ਅੰਕੜੇ,
						     pty->ਬੱਚਾ);
		}

		ਜੇ (pty->ਬੱਚਾ <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->ਅਗਲਾ_callback_ਸਮਾਂ);
		}
		ਵਾਪਸ 0;
	ਹਾਲਤ SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		ਜੇ (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (ਅੱਖਰ *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (ਅੱਖਰ *)&pty->win);

			ਜੇ (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_ਅੰਕੜੇ,
							&info, (ਖਾਲੀ *) &pty->win);
		}
		ਰੋਕੋ;
	ਹਾਲਤ SIGALRM:
		FALLTHROUGH;
	ਹਾਲਤ SIGHUP:
		FALLTHROUGH;
	ਹਾਲਤ SIGTERM:
		FALLTHROUGH;
	ਹਾਲਤ SIGINT:
		FALLTHROUGH;
	ਹਾਲਤ SIGQUIT:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIG{ALRM,HUP,TERM,INT,QUIT}"));
		pty->delivered_signal = info.ssi_signo;
		/* Child termination is going to generate SIGCHLD (see above) */
		ਜੇ (pty->ਬੱਚਾ > 0)
			kill(pty->ਬੱਚਾ, SIGTERM);

		ਜੇ (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_ਅੰਕੜੇ,
					&info, (ਖਾਲੀ *) &pty->win);
		ਰੋਕੋ;
	ਹਾਲਤ SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		ਜੇ (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_ਅੰਕੜੇ);
		ਰੋਕੋ;
	ਮੂਲ:
		abort();
	}

	ਵਾਪਸ rc;
}

/* loop in parent */
ਪੂਰਨ ul_pty_proxy_master(ਬਣਤਰ ul_pty *pty)
{
	ਪੂਰਨ rc = 0, ret, eof = 0;
	ਗਿਣਤੀ {
		POLLFD_SIGNAL = 0,
		POLLFD_MASTER,
		POLLFD_STDIN

	};
	ਬਣਤਰ pollfd pfd[] = {
		[POLLFD_SIGNAL] = { .fd = -1,		.events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_MASTER] = { .fd = pty->master,  .events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_STDIN]	= { .fd = STDIN_FILENO, .events = POLLIN | POLLERR | POLLHUP }
	};

	/* We use signalfd, and standard signals by handlers are completely blocked */
	ਪ੍ਰਤਿਜ੍ਞਾ(pty->sigfd >= 0);

	pfd[POLLFD_SIGNAL].fd = pty->sigfd;
	pty->poll_timeout = -1;

	ਜਦੋਂ (!pty->delivered_signal) {
		size_t i;
		ਪੂਰਨ errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		ਜੇ (timerisset(&pty->ਅਗਲਾ_callback_ਸਮਾਂ)) {
			ਬਣਤਰ timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			ਜੇ (timercmp(&now, &pty->ਅਗਲਾ_callback_ਸਮਾਂ, >)) {
				rc = mainloop_callback(pty);
				ਜੇ (rc)
					ਰੋਕੋ;
			}
		}

		/* set timeout */
		ਜੇ (timerisset(&pty->ਅਗਲਾ_callback_ਸਮਾਂ)) {
			ਬਣਤਰ timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->ਅਗਲਾ_callback_ਸਮਾਂ, &now, &rest);
			timeout = (rest.tv_sec * MSEC_ਪ੍ਰਤੀ_SEC) +  (rest.tv_usec / USEC_ਪ੍ਰਤੀ_MSEC);
		} ਨਹੀਂਤਾਂ
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		ਜੇ (pty->ਬੱਚਾ_ਬਫਰ_ਸਿਰ)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		ਨਹੀਂਤਾਂ
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_ਮਾਪ(pfd), timeout);

		errsv = errno;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		ਜੇ (ret < 0) {
			ਜੇ (errsv == EAGAIN)
				ਜਾਰੀ;
			rc = -errno;
			ਰੋਕੋ;
		}

		/* timeout */
		ਜੇ (ret == 0) {
			ਜੇ (timerisset(&pty->ਅਗਲਾ_callback_ਸਮਾਂ)) {
				rc = mainloop_callback(pty);
				ਜੇ (rc == 0)
					ਜਾਰੀ;
			} ਨਹੀਂਤਾਂ {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			ਰੋਕੋ;
		}
		/* event */
		ਲਈ (i = 0; i < ARRAY_ਮਾਪ(pfd); i++) {
			ਜੇ (pfd[i].revents == 0)
				ਜਾਰੀ;

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

			ਜੇ (i == POLLFD_SIGNAL)
				rc = handle_signal(pty, pfd[i].fd);
			ਨਹੀਂਤਾਂ {
				ਜੇ (pfd[i].revents & POLLIN)
					rc = handle_io(pty, pfd[i].fd, &eof); /*ਅੰਕੜੇ*/
				ਜੇ (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_ਬੱਚਾ_buffers(pty, ਨਿਰਾ);
			}

			ਜੇ (rc) {
				ਪੂਰਨ anything = 1;

				ul_pty_write_eof_ਮੰਜ਼ਿਲ_ਬੱਚਾ(pty);
				ਲਈ (anything = 1; anything;)
					flush_ਬੱਚਾ_buffers(pty, &anything);
				ਰੋਕੋ;
			}

			ਜੇ (i == POLLFD_SIGNAL)
				ਜਾਰੀ;

			/* EOF maybe detected in two ways; they are as follows:
			 *	A) poll() return POLLHUP event after close()
			 *	B) read() returns 0 (no data)
			 *
			 * POLLNVAL means that fd is closed.
			 */
			ਜੇ ((pfd[i].revents & POLLHUP) || (pfd[i].revents & POLLNVAL) || eof) {
				DBG_OBJ(IO, pty, ul_debug(" ignore FD"));
				ਜੇ (i == POLLFD_STDIN) {
					pfd[i].fd = -1;
					ul_pty_write_eof_ਮੰਜ਼ਿਲ_ਬੱਚਾ(pty);
				} ਨਹੀਂਤਾਂ /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		ਜੇ (rc)
			ਰੋਕੋ;
	}

	ਜੇ (rc && pty->ਬੱਚਾ && pty->ਬੱਚਾ != (pid_t) -1 && !pty->delivered_signal) {
		kill(pty->ਬੱਚਾ, SIGTERM);
		sleep(2);
		kill(pty->ਬੱਚਾ, SIGKILL);
	}

	pty_ਸੰਕੇਤ_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	ਵਾਪਸ rc;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_PTY
/*
 * $ make test_pty
 * $ ./test_pty
 *
 * ... and see for example tty(1) or "ps afu"
 */
ਸਥਿਰ ਖਾਲੀ ਬੱਚਾ_sigstop(ਖਾਲੀ *ਅੰਕੜੇ __attribute__((__unused__)), pid_t ਬੱਚਾ)
{
	kill(getpid(), SIGSTOP);
	kill(ਬੱਚਾ, SIGCONT);
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਬਣਤਰ ul_pty_callbacks *cb;
	ਅਚਲ ਅੱਖਰ *shell, *command = ਨਿਰਾ, *shname = ਨਿਰਾ;
	ਪੂਰਨ caught_signal = 0;
	pid_t ਬੱਚਾ;
	ਬਣਤਰ ul_pty *pty;

	shell = getenv("SHELL");
	ਜੇ (shell == ਨਿਰਾ)
		shell = _PATH_BSHELL;
	ਜੇ (argc == 2)
		command = argv[1];

	ul_pty_init_debug(0);

	pty = ul_new_pty(isatty(STDIN_FILENO));
	ਜੇ (!pty)
		err(EXIT_FAILURE, "failed to allocate PTY handler");

	cb = ul_pty_get_callbacks(pty);
	cb->ਬੱਚਾ_sigstop = ਬੱਚਾ_sigstop;

	ਜੇ (ul_pty_ਤਿਆਰੀ(pty))
		err(EXIT_FAILURE, "failed to create pseudo-terminal");
	ਜੇ (ul_pty_ਸੰਕੇਤ_ਤਿਆਰੀ(pty))
		err(EXIT_FAILURE, "failed to initialize signals handler");
	ਜੇ (ul_pty_terminal_ਤਿਆਰੀ(pty))
		err(EXIT_FAILURE, "failed to setup terminal");

	fflush(stdout);			/* ??? */

	ਚੋਣ ((ਪੂਰਨ) (ਬੱਚਾ = fork())) {
	ਹਾਲਤ -1: /* error */
		ul_pty_cleanup(pty);
		err(EXIT_FAILURE, "cannot create child process");
		ਰੋਕੋ;

	ਹਾਲਤ 0: /*ਬੱਚਾ*/
		ul_pty_init_slave(pty);

		ਸੰਕੇਤ(SIGTERM, SIG_DFL); /* because /etc/csh.login */

		shname = strrchr(shell, '/');
		shname = shname ? shname + 1 : shell;

		ਜੇ (command)
			execl(shell, shname, "-c", command, (ਅੱਖਰ *)ਨਿਰਾ);
		ਨਹੀਂਤਾਂ
			execl(shell, shname, "-i", (ਅੱਖਰ *)ਨਿਰਾ);
		err(EXIT_FAILURE, "failed to execute %s", shell);
		ਰੋਕੋ;

	ਮੂਲ:
		ਰੋਕੋ;
	}

	/*ਮਾਪਾ*/
	ul_pty_ਸੈੱਟ_ਬੱਚਾ(pty, ਬੱਚਾ);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	ਜੇ (!caught_signal && ul_pty_get_ਬੱਚਾ(pty) != (pid_t)-1)
		ul_pty_wait_for_ਬੱਚਾ(pty);	/* final wait */

	ਜੇ (caught_signal && ul_pty_get_ਬੱਚਾ(pty) != (pid_t)-1) {
		fprintf(stderr, "\nSession terminated, killing shell...");
		kill(ਬੱਚਾ, SIGTERM);
		sleep(2);
		kill(ਬੱਚਾ, SIGKILL);
		fprintf(stderr, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	ਵਾਪਸ EXIT_SUCCESS;
}

#ਜੇ_ਅੰਤ /* TEST_PROGRAM */

