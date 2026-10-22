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

ಸ್ಥಿರ UL_DEBUG_DEFINE_ಮುಖವಾಡ(ulpty);
UL_DEBUG_DEFINE_MASKNAMES(ulpty) = UL_DEBUG_ಖಾಲಿಗಳು_MASKNAMES;

#ವ್ಯಾಖ್ಯಾನಿಸು ULPTY_DEBUG_INIT	(1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು ULPTY_DEBUG_ಸಿದ್ಧತೆ	(1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು ULPTY_DEBUG_SIG		(1 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು ULPTY_DEBUG_IO		(1 << 4)
#ವ್ಯಾಖ್ಯಾನಿಸು ULPTY_DEBUG_ಮುಗಿದ	(1 << 5)

#ವ್ಯಾಖ್ಯಾನಿಸು DBG(m, x)		__UL_DBG(ulpty, ULPTY_DEBUG_, m, x)
#ವ್ಯಾಖ್ಯಾನಿಸು DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpty, ULPTY_DEBUG_, m, h, x)
#ವ್ಯಾಖ್ಯಾನಿಸು ON_DBG(m, x)		__UL_DBG_CALL(ulpty, ULPTY_DEBUG_, m, x)

ಶೂನ್ಯ ul_pty_init_debug(ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ)
{
	ಆದರೆ (ulpty_debug_ಮುಖವಾಡ)
		ಹಿಂತಿರುಗಿ;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, ಮುಖವಾಡ, ULPTY_DEBUG);
}

ರಚನೆ ul_pty *ul_new_pty(ಪೂರ್ಣಾಂಕ is_stdin_tty)
{
	ರಚನೆ ul_pty *pty = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*pty));

	ಆದರೆ (!pty)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	DBG_OBJ(ಸಿದ್ಧತೆ, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->ಮಗು = (pid_t) -1;

	ಹಿಂತಿರುಗಿ pty;
}

ಶೂನ್ಯ ul_free_pty(ರಚನೆ ul_pty *pty)
{
	ರಚನೆ ul_pty_ಮಗು_ಬಫರ್ *hd;

	ಆಗ ((hd = pty->ಮಗು_ಬಫರ್_ತಲೆ)) {
		pty->ಮಗು_ಬಫರ್_ತಲೆ = hd->ಮುಂದಿನ;
		ಬಿಡುಗಡೆ(hd);
	}

	ಆಗ ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->ಮುಂದಿನ;
		ಬಿಡುಗಡೆ(hd);
	}
	ಬಿಡುಗಡೆ(pty);
}

ಶೂನ್ಯ ul_pty_slave_echo(ರಚನೆ ul_pty *pty, ಪೂರ್ಣಾಂಕ enable)
{
	ಪ್ರತಿಜ್ಞಾ(pty);
	pty->slave_echo = enable ? 1 : 0;
}

ಪೂರ್ಣಾಂಕ ul_pty_get_delivered_signal(ರಚನೆ ul_pty *pty)
{
	ಪ್ರತಿಜ್ಞಾ(pty);
	ಹಿಂತಿರುಗಿ pty->delivered_signal;
}

ರಚನೆ ul_pty_callbacks *ul_pty_get_callbacks(ರಚನೆ ul_pty *pty)
{
	ಪ್ರತಿಜ್ಞಾ(pty);
	ಹಿಂತಿರುಗಿ &pty->callbacks;
}

ಶೂನ್ಯ ul_pty_ಗಣ_callback_ದತ್ತಾಂಶ(ರಚನೆ ul_pty *pty, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	ಪ್ರತಿಜ್ಞಾ(pty);
	pty->callback_ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;
}

ಶೂನ್ಯ ul_pty_ಗಣ_ಮಗು(ರಚನೆ ul_pty *pty, pid_t ಮಗು)
{
	ಪ್ರತಿಜ್ಞಾ(pty);
	pty->ಮಗು = ಮಗು;
}

ಪೂರ್ಣಾಂಕ ul_pty_get_childfd(ರಚನೆ ul_pty *pty)
{
	ಪ್ರತಿಜ್ಞಾ(pty);
	ಹಿಂತಿರುಗಿ pty->master;
}

pid_t ul_pty_get_ಮಗು(ರಚನೆ ul_pty *pty)
{
	ಪ್ರತಿಜ್ಞಾ(pty);
	ಹಿಂತಿರುಗಿ pty->ಮಗು;
}

/* it's active when signals are redirected to sigfd */
ಪೂರ್ಣಾಂಕ ul_pty_is_running(ರಚನೆ ul_pty *pty)
{
	ಪ್ರತಿಜ್ಞಾ(pty);
	ಹಿಂತಿರುಗಿ pty->sigfd >= 0;
}

ಶೂನ್ಯ ul_pty_ಗಣ_mainloop_ಸಮಯ(ರಚನೆ ul_pty *pty, ರಚನೆ timeval *tv)
{
	ಪ್ರತಿಜ್ಞಾ(pty);
	ಆದರೆ (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->ಮುಂದಿನ_callback_ಸಮಯ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		pty->ಮುಂದಿನ_callback_ಸಮಯ.tv_sec = tv->tv_sec;
		pty->ಮುಂದಿನ_callback_ಸಮಯ.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ pty_ಸಂಕೇತಗಳು_cleanup(ರಚನೆ ul_pty *pty)
{
	ಆದರೆ (pty->sigfd != -1)
		close(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, ಖಾಲಿ);
}

/* call me before fork() */
ಪೂರ್ಣಾಂಕ ul_pty_ಸಿದ್ಧತೆ(ರಚನೆ ul_pty *pty)
{
	ರಚನೆ termios attrs;
	ಪೂರ್ಣಾಂಕ rc = 0;

	ಪ್ರತಿಜ್ಞಾ(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, ಖಾಲಿ, &pty->orgsig);

	ಆದರೆ (pty->isterm) {
		DBG_OBJ(ಸಿದ್ಧತೆ, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		ಆದರೆ (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -errno;
			ಹೋಗು ಮುಗಿದ;
		}

		attrs = pty->stdin_attrs;
		ಆದರೆ (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		ಇಲ್ಲದಿದ್ದರೆ
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (ಅಕ್ಷರ *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, ಖಾಲಿ, &attrs, &pty->win);
		ಆದರೆ (rc)
			ಹೋಗು ಮುಗಿದ;

	} ಇಲ್ಲದಿದ್ದರೆ {
		DBG_OBJ(ಸಿದ್ಧತೆ, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ);
		ಆದರೆ (rc)
			ಹೋಗು ಮುಗಿದ;

		tcgetattr(pty->slave, &attrs);

		ಆದರೆ (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		ಇಲ್ಲದಿದ್ದರೆ
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

ಮುಗಿದ:
	ಆದರೆ (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(ಸಿದ್ಧತೆ, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	ಹಿಂತಿರುಗಿ rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
ಪೂರ್ಣಾಂಕ ul_pty_terminal_ಸಿದ್ಧತೆ(ರಚನೆ ul_pty *pty)
{
	ರಚನೆ termios attrs;

	ಆದರೆ (!pty->isterm)
		ಹಿಂತಿರುಗಿ 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	ಆದರೆ (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		ಹಿಂತಿರುಗಿ -errno;

	DBG_OBJ(ಸಿದ್ಧತೆ, pty, ul_debug("terminal raw mode setup done"));
	ಹಿಂತಿರುಗಿ 0;
}

/* call me before fork() */
ಪೂರ್ಣಾಂಕ ul_pty_ಸಂಕೇತಗಳು_ಸಿದ್ಧತೆ(ರಚನೆ ul_pty *pty)
{
	sigset_t ourset;
	ಪೂರ್ಣಾಂಕ rc = 0;

	ಪ್ರತಿಜ್ಞಾ(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, ಖಾಲಿ, &pty->orgsig);

	sigfillset(&ourset);
	ಆದರೆ (sigprocmask(SIG_BLOCK, &ourset, ಖಾಲಿ)) {
		rc = -errno;
		ಹೋಗು ಮುಗಿದ;
	}

	sigemptyset(&ourset);
	sigaddset(&ourset, SIGCHLD);
	sigaddset(&ourset, SIGWINCH);
	sigaddset(&ourset, SIGALRM);
	sigaddset(&ourset, SIGHUP);
	sigaddset(&ourset, SIGTERM);
	sigaddset(&ourset, SIGINT);
	sigaddset(&ourset, SIGQUIT);

	ಆದರೆ (pty->callbacks.flush_logs)
		sigaddset(&ourset, SIGUSR1);

	ಆದರೆ ((pty->sigfd = signalfd(-1, &ourset, SFD_CLOEXEC)) < 0)
		rc = -errno;
ಮುಗಿದ:
	ಆದರೆ (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(ಸಿದ್ಧತೆ, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	ಹಿಂತಿರುಗಿ rc;
}

/* cleanup in parent process */
ಶೂನ್ಯ ul_pty_cleanup(ರಚನೆ ul_pty *pty)
{
	ರಚನೆ termios rtt;

	pty_ಸಂಕೇತಗಳು_cleanup(pty);

	ಆದರೆ (pty->master == -1 || !pty->isterm)
		ಹಿಂತಿರುಗಿ;

	DBG_OBJ(ಮುಗಿದ_2, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

ಪೂರ್ಣಾಂಕ ul_pty_chownmod_slave(ರಚನೆ ul_pty *pty, uid_t uid, gid_t gid, mode_t ಬಹುಲಕ)
{
	ಆದರೆ (fchown(pty->slave, uid, gid))
		ಹಿಂತಿರುಗಿ -errno;
	ಆದರೆ (fchmod(pty->slave, ಬಹುಲಕ))
		ಹಿಂತಿರುಗಿ -errno;
	ಹಿಂತಿರುಗಿ 0;
}

/* call me in child process */
ಶೂನ್ಯ ul_pty_init_slave(ರಚನೆ ul_pty *pty)
{
	DBG_OBJ(ಸಿದ್ಧತೆ, pty, ul_debug("initialize slave"));

	setsid();

	ioctl(pty->slave, TIOCSCTTY, 1);
	close(pty->master);

	dup2(pty->slave, STDIN_FILENO);
	dup2(pty->slave, STDOUT_FILENO);
	dup2(pty->slave, STDERR_FILENO);

	close(pty->slave);

	ಆದರೆ (pty->sigfd >= 0)
		close(pty->sigfd);

	pty->slave = -1;
	pty->master = -1;
	pty->sigfd = -1;

	sigprocmask(SIG_SETMASK, &pty->orgsig, ಖಾಲಿ);

	DBG_OBJ(ಸಿದ್ಧತೆ, pty, ul_debug("... initialize slave done"));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ write_output(ಅಕ್ಷರ *obuf, ssize_t ಬೈಟುಗಳು)
{
	DBG(IO, ul_debug(" writing output"));

	ಆದರೆ (ul_write_all(STDOUT_FILENO, obuf, ಬೈಟುಗಳು)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		ಹಿಂತಿರುಗಿ -errno;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ವೇಳಾಪಟ್ಟಿ_ಮಗು_write(ರಚನೆ ul_pty *pty, ಅಕ್ಷರ *buf, size_t bufsz, ಪೂರ್ಣಾಂಕ final)
{
	ರಚನೆ ul_pty_ಮಗು_ಬಫರ್ *stash;

	ಆದರೆ (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->ಮುಂದಿನ;
		memset(stash, 0, ಗಾತ್ರ(*stash));
	} ಇಲ್ಲದಿದ್ದರೆ
		stash = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*stash));
	ಆದರೆ (!stash)
		ಹಿಂತಿರುಗಿ -1;

	ಪ್ರತಿಜ್ಞಾ(bufsz <= ಗಾತ್ರ(stash->buf));

	ಸ್ಮರಣೆ_ನಕಲು(stash->buf, buf, bufsz);
	stash->ಗಾತ್ರ_2 = bufsz;
	stash->final_ಒಳಹರಿವು = final ? 1 : 0;

	ಆದರೆ (pty->ಮಗು_ಬಫರ್_ತಲೆ)
		pty->ಮಗು_ಬಫರ್_ಬಾಲ = pty->ಮಗು_ಬಫರ್_ಬಾಲ->ಮುಂದಿನ = stash;
	ಇಲ್ಲದಿದ್ದರೆ
		pty->ಮಗು_ಬಫರ್_ತಲೆ = pty->ಮಗು_ಬಫರ್_ಬಾಲ = stash;
	ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ ಶೂನ್ಯ drain_ಮಗು_buffers(ರಚನೆ ul_pty *pty)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ tries = 0;
	ರಚನೆ pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	ಆಗ (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	ಆದರೆ (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ flush_ಮಗು_buffers(ರಚನೆ ul_pty *pty, ಪೂರ್ಣಾಂಕ *anything)
{
	ಪೂರ್ಣಾಂಕ rc = 0, any = 0;

	ಆಗ (pty->ಮಗು_ಬಫರ್_ತಲೆ) {
		ರಚನೆ ul_pty_ಮಗು_ಬಫರ್ *hd = pty->ಮಗು_ಬಫರ್_ತಲೆ;
		ssize_t ret;

		ಆದರೆ (hd->final_ಒಳಹರಿವು)
			drain_ಮಗು_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->ಗಾತ್ರ_2 - hd->cursor));

		ret = ಲಿಖ(pty->master, hd->buf + hd->cursor, hd->ಗಾತ್ರ_2 - hd->cursor);
		ಆದರೆ (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			ಆದರೆ (!(errno == EINTR || errno == EAGAIN))
				rc = -errno;
			ಹೋಗು ಹೊರಹೋಗು;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		ಆದರೆ (hd->cursor == hd->ಗಾತ್ರ_2) {
			pty->ಮಗು_ಬಫರ್_ತಲೆ = hd->ಮುಂದಿನ;
			ಆದರೆ (!hd->ಮುಂದಿನ)
				pty->ಮಗು_ಬಫರ್_ಬಾಲ = ಖಾಲಿ;

			hd->ಮುಂದಿನ = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
ಹೊರಹೋಗು:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	ಆದರೆ (any)
		fdatasync(pty->master);

	ಆದರೆ (anything)
		*anything = any;
	ಹಿಂತಿರುಗಿ rc;
}

ಶೂನ್ಯ ul_pty_write_eof_ಗಮ್ಯ_ಮಗು(ರಚನೆ ul_pty *pty)
{
	ಅಕ್ಷರ c = DEF_EOF;
	ವೇಳಾಪಟ್ಟಿ_ಮಗು_write(pty, &c, ಗಾತ್ರ(ಅಕ್ಷರ), 1);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ mainloop_callback(ರಚನೆ ul_pty *pty)
{
	ಪೂರ್ಣಾಂಕ rc;

	ಆದರೆ (!pty->callbacks.mainloop)
		ಹಿಂತಿರುಗಿ 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_ದತ್ತಾಂಶ);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ handle_io(ರಚನೆ ul_pty *pty, ಪೂರ್ಣಾಂಕ fd, ಪೂರ್ಣಾಂಕ *eof)
{
	ಅಕ್ಷರ buf[BUFSIZ];
	ssize_t ಬೈಟುಗಳು;
	ಪೂರ್ಣಾಂಕ rc = 0;
	sigset_t ಗಣ;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&ಗಣ);
	sigaddset(&ಗಣ, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &ಗಣ, ಖಾಲಿ);
	/* read from active FD */
	ಬೈಟುಗಳು = ಪಠ(fd, buf, ಗಾತ್ರ(buf));
	sigprocmask(SIG_BLOCK, &ಗಣ, ಖಾಲಿ);
	ಆದರೆ (ಬೈಟುಗಳು == -1) {
		ಆದರೆ (errno == EAGAIN || errno == EINTR)
			ಹಿಂತಿರುಗಿ 0;
		ಹಿಂತಿರುಗಿ -errno;
	}

	ಆದರೆ (ಬೈಟುಗಳು == 0) {
		*eof = 1;
		ಹಿಂತಿರುಗಿ 0;
	}

	/* from stdin (user) to command */
	ಆದರೆ (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", ಬೈಟುಗಳು));

		ಆದರೆ (ವೇಳಾಪಟ್ಟಿ_ಮಗು_write(pty, buf, ಬೈಟುಗಳು, 0))
			ಹಿಂತಿರುಗಿ -errno;

	/* from command (master) to stdout */
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", ಬೈಟುಗಳು));
		write_output(buf, ಬೈಟುಗಳು);
	}

	ಆದರೆ (pty->callbacks.log_ಹರಿವು_activity)
		rc = pty->callbacks.log_ಹರಿವು_activity(
					pty->callback_ದತ್ತಾಂಶ, fd, buf, ಬೈಟುಗಳು);

	ಹಿಂತಿರುಗಿ rc;
}

ಶೂನ್ಯ ul_pty_wait_for_ಮಗು(ರಚನೆ ul_pty *pty)
{
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
	pid_t pid;
	ಪೂರ್ಣಾಂಕ options = 0;

	ಆದರೆ (pty->ಮಗು == (pid_t) -1)
		ಹಿಂತಿರುಗಿ;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (ಪೂರ್ಣಾಂಕ) pty->ಮಗು));

	ಆದರೆ (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		ಗಾಗಿ (;;) {
			pid = waitpid(pty->ಮಗು, &ಸ್ಥಿತಿ_2, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (ಪೂರ್ಣಾಂಕ) pid));
			ಆದರೆ (pid != (pid_t) - 1) {
				ಆದರೆ (pty->callbacks.ಮಗು_die)
					pty->callbacks.ಮಗು_die(
							pty->callback_ದತ್ತಾಂಶ,
							pty->ಮಗು, ಸ್ಥಿತಿ_2);
				ul_pty_ಗಣ_ಮಗು(pty, (pid_t) -1);
			} ಇಲ್ಲದಿದ್ದರೆ
				ನಿಲ್ಲಿಸು;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* final wait */
		ಆಗ ((pid = waitpid(-1, &ಸ್ಥಿತಿ_2, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (ಪೂರ್ಣಾಂಕ) pid));
			ಆದರೆ (pid == pty->ಮಗು) {
				ಆದರೆ (pty->callbacks.ಮಗು_die)
					pty->callbacks.ಮಗು_die(
							pty->callback_ದತ್ತಾಂಶ,
							pty->ಮಗು, ಸ್ಥಿತಿ_2);
				ul_pty_ಗಣ_ಮಗು(pty, (pid_t) -1);
			}
		}
	}
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ handle_signal(ರಚನೆ ul_pty *pty, ಪೂರ್ಣಾಂಕ fd)
{
	ರಚನೆ signalfd_siginfo info;
	ssize_t ಬೈಟುಗಳು;
	ಪೂರ್ಣಾಂಕ rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	ಬೈಟುಗಳು = ಪಠ(fd, &info, ಗಾತ್ರ(info));
	ಆದರೆ (ಬೈಟುಗಳು != ಗಾತ್ರ(info)) {
		ಆದರೆ (ಬೈಟುಗಳು < 0 && (errno == EAGAIN || errno == EINTR))
			ಹಿಂತಿರುಗಿ 0;
		ಹಿಂತಿರುಗಿ -errno;
	}

	ಆಯ್ಕೆ (info.ssi_signo) {
	ಸ್ಥಿತಿ SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		ಆದರೆ (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			ಆದರೆ (pty->callbacks.ಮಗು_wait)
				pty->callbacks.ಮಗು_wait(pty->callback_ದತ್ತಾಂಶ,
							  pty->ಮಗು);
			ಇಲ್ಲದಿದ್ದರೆ
				ul_pty_wait_for_ಮಗು(pty);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (info.ssi_status == SIGSTOP && pty->ಮಗು > 0) {
			pty->callbacks.ಮಗು_sigstop(pty->callback_ದತ್ತಾಂಶ,
						     pty->ಮಗು);
		}

		ಆದರೆ (pty->ಮಗು <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->ಮುಂದಿನ_callback_ಸಮಯ);
		}
		ಹಿಂತಿರುಗಿ 0;
	ಸ್ಥಿತಿ SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		ಆದರೆ (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (ಅಕ್ಷರ *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (ಅಕ್ಷರ *)&pty->win);

			ಆದರೆ (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_ದತ್ತಾಂಶ,
							&info, (ಶೂನ್ಯ *) &pty->win);
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ SIGALRM:
		FALLTHROUGH;
	ಸ್ಥಿತಿ SIGHUP:
		FALLTHROUGH;
	ಸ್ಥಿತಿ SIGTERM:
		FALLTHROUGH;
	ಸ್ಥಿತಿ SIGINT:
		FALLTHROUGH;
	ಸ್ಥಿತಿ SIGQUIT:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIG{ALRM,HUP,TERM,INT,QUIT}"));
		pty->delivered_signal = info.ssi_signo;
		/* Child termination is going to generate SIGCHLD (see above) */
		ಆದರೆ (pty->ಮಗು > 0)
			kill(pty->ಮಗು, SIGTERM);

		ಆದರೆ (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_ದತ್ತಾಂಶ,
					&info, (ಶೂನ್ಯ *) &pty->win);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		ಆದರೆ (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_ದತ್ತಾಂಶ);
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		abort();
	}

	ಹಿಂತಿರುಗಿ rc;
}

/* loop in parent */
ಪೂರ್ಣಾಂಕ ul_pty_proxy_master(ರಚನೆ ul_pty *pty)
{
	ಪೂರ್ಣಾಂಕ rc = 0, ret, eof = 0;
	ಎಣಿಕೆ {
		POLLFD_SIGNAL = 0,
		POLLFD_MASTER,
		POLLFD_STDIN

	};
	ರಚನೆ pollfd pfd[] = {
		[POLLFD_SIGNAL] = { .fd = -1,		.events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_MASTER] = { .fd = pty->master,  .events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_STDIN]	= { .fd = STDIN_FILENO, .events = POLLIN | POLLERR | POLLHUP }
	};

	/* We use signalfd, and standard signals by handlers are completely blocked */
	ಪ್ರತಿಜ್ಞಾ(pty->sigfd >= 0);

	pfd[POLLFD_SIGNAL].fd = pty->sigfd;
	pty->poll_timeout = -1;

	ಆಗ (!pty->delivered_signal) {
		size_t i;
		ಪೂರ್ಣಾಂಕ errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		ಆದರೆ (timerisset(&pty->ಮುಂದಿನ_callback_ಸಮಯ)) {
			ರಚನೆ timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			ಆದರೆ (timercmp(&now, &pty->ಮುಂದಿನ_callback_ಸಮಯ, >)) {
				rc = mainloop_callback(pty);
				ಆದರೆ (rc)
					ನಿಲ್ಲಿಸು;
			}
		}

		/* set timeout */
		ಆದರೆ (timerisset(&pty->ಮುಂದಿನ_callback_ಸಮಯ)) {
			ರಚನೆ timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->ಮುಂದಿನ_callback_ಸಮಯ, &now, &rest);
			timeout = (rest.tv_sec * MSEC_ಪ್ರತಿ_SEC) +  (rest.tv_usec / USEC_ಪ್ರತಿ_MSEC);
		} ಇಲ್ಲದಿದ್ದರೆ
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		ಆದರೆ (pty->ಮಗು_ಬಫರ್_ತಲೆ)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		ಇಲ್ಲದಿದ್ದರೆ
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_ಗಾತ್ರ(pfd), timeout);

		errsv = errno;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		ಆದರೆ (ret < 0) {
			ಆದರೆ (errsv == EAGAIN)
				ಮುಂದುವರಿ;
			rc = -errno;
			ನಿಲ್ಲಿಸು;
		}

		/* timeout */
		ಆದರೆ (ret == 0) {
			ಆದರೆ (timerisset(&pty->ಮುಂದಿನ_callback_ಸಮಯ)) {
				rc = mainloop_callback(pty);
				ಆದರೆ (rc == 0)
					ಮುಂದುವರಿ;
			} ಇಲ್ಲದಿದ್ದರೆ {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			ನಿಲ್ಲಿಸು;
		}
		/* event */
		ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(pfd); i++) {
			ಆದರೆ (pfd[i].revents == 0)
				ಮುಂದುವರಿ;

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

			ಆದರೆ (i == POLLFD_SIGNAL)
				rc = handle_signal(pty, pfd[i].fd);
			ಇಲ್ಲದಿದ್ದರೆ {
				ಆದರೆ (pfd[i].revents & POLLIN)
					rc = handle_io(pty, pfd[i].fd, &eof); /*ದತ್ತಾಂಶ*/
				ಆದರೆ (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_ಮಗು_buffers(pty, ಖಾಲಿ);
			}

			ಆದರೆ (rc) {
				ಪೂರ್ಣಾಂಕ anything = 1;

				ul_pty_write_eof_ಗಮ್ಯ_ಮಗು(pty);
				ಗಾಗಿ (anything = 1; anything;)
					flush_ಮಗು_buffers(pty, &anything);
				ನಿಲ್ಲಿಸು;
			}

			ಆದರೆ (i == POLLFD_SIGNAL)
				ಮುಂದುವರಿ;

			/* EOF maybe detected in two ways; they are as follows:
			 *	A) poll() return POLLHUP event after close()
			 *	B) read() returns 0 (no data)
			 *
			 * POLLNVAL means that fd is closed.
			 */
			ಆದರೆ ((pfd[i].revents & POLLHUP) || (pfd[i].revents & POLLNVAL) || eof) {
				DBG_OBJ(IO, pty, ul_debug(" ignore FD"));
				ಆದರೆ (i == POLLFD_STDIN) {
					pfd[i].fd = -1;
					ul_pty_write_eof_ಗಮ್ಯ_ಮಗು(pty);
				} ಇಲ್ಲದಿದ್ದರೆ /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		ಆದರೆ (rc)
			ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (rc && pty->ಮಗು && pty->ಮಗು != (pid_t) -1 && !pty->delivered_signal) {
		kill(pty->ಮಗು, SIGTERM);
		sleep(2);
		kill(pty->ಮಗು, SIGKILL);
	}

	pty_ಸಂಕೇತಗಳು_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	ಹಿಂತಿರುಗಿ rc;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_PTY
/*
 * $ make test_pty
 * $ ./test_pty
 *
 * ... and see for example tty(1) or "ps afu"
 */
ಸ್ಥಿರ ಶೂನ್ಯ ಮಗು_sigstop(ಶೂನ್ಯ *ದತ್ತಾಂಶ __attribute__((__unused__)), pid_t ಮಗು)
{
	kill(getpid(), SIGSTOP);
	kill(ಮಗು, SIGCONT);
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ರಚನೆ ul_pty_callbacks *cb;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *shell, *command = ಖಾಲಿ, *shname = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ caught_signal = 0;
	pid_t ಮಗು;
	ರಚನೆ ul_pty *pty;

	shell = getenv("SHELL");
	ಆದರೆ (shell == ಖಾಲಿ)
		shell = _PATH_BSHELL;
	ಆದರೆ (argc == 2)
		command = argv[1];

	ul_pty_init_debug(0);

	pty = ul_new_pty(isatty(STDIN_FILENO));
	ಆದರೆ (!pty)
		err(EXIT_FAILURE, "failed to allocate PTY handler");

	cb = ul_pty_get_callbacks(pty);
	cb->ಮಗು_sigstop = ಮಗು_sigstop;

	ಆದರೆ (ul_pty_ಸಿದ್ಧತೆ(pty))
		err(EXIT_FAILURE, "failed to create pseudo-terminal");
	ಆದರೆ (ul_pty_ಸಂಕೇತಗಳು_ಸಿದ್ಧತೆ(pty))
		err(EXIT_FAILURE, "failed to initialize signals handler");
	ಆದರೆ (ul_pty_terminal_ಸಿದ್ಧತೆ(pty))
		err(EXIT_FAILURE, "failed to setup terminal");

	fflush(stdout);			/* ??? */

	ಆಯ್ಕೆ ((ಪೂರ್ಣಾಂಕ) (ಮಗು = fork())) {
	ಸ್ಥಿತಿ -1: /* error */
		ul_pty_cleanup(pty);
		err(EXIT_FAILURE, "cannot create child process");
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ 0: /*ಮಗು*/
		ul_pty_init_slave(pty);

		ಸಂಕೇತ(SIGTERM, SIG_DFL); /* because /etc/csh.login */

		shname = strrchr(shell, '/');
		shname = shname ? shname + 1 : shell;

		ಆದರೆ (command)
			execl(shell, shname, "-c", command, (ಅಕ್ಷರ *)ಖಾಲಿ);
		ಇಲ್ಲದಿದ್ದರೆ
			execl(shell, shname, "-i", (ಅಕ್ಷರ *)ಖಾಲಿ);
		err(EXIT_FAILURE, "failed to execute %s", shell);
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/*ಪೋಷಕ*/
	ul_pty_ಗಣ_ಮಗು(pty, ಮಗು);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	ಆದರೆ (!caught_signal && ul_pty_get_ಮಗು(pty) != (pid_t)-1)
		ul_pty_wait_for_ಮಗು(pty);	/* final wait */

	ಆದರೆ (caught_signal && ul_pty_get_ಮಗು(pty) != (pid_t)-1) {
		fprintf(stderr, "\nSession terminated, killing shell...");
		kill(ಮಗು, SIGTERM);
		sleep(2);
		kill(ಮಗು, SIGKILL);
		fprintf(stderr, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

#ಆದರೆ_ಅಂತ್ಯ /* TEST_PROGRAM */

