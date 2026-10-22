/*
 * plymouth-ctrl.c	Simply communications with plymouthd
 *			to avoid forked sub processes and/or
 *			missed plymouth send commands tool
 *			due a plymouthd replacement.
 *
 * Copyright (c) 2016 SUSE Linux GmbH, All rights reserved.
 * Copyright (c) 2016 Werner Fink <werner@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://gnu.org/licenses/>.
 *
 * Author: Werner Fink <werner@suse.de>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <poll.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/mman.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/socket.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/un.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "all-io.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "nls.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "plymouth-ctrl.h"

ਸਥਿਰ ਪੂਰਨ can_read(ਪੂਰਨ fd, ਅਚਲ ਲੰਮਾ timeout)
{
	ਬਣਤਰ pollfd fds = {
		.fd = fd,
		.events = POLLIN|POLLPRI,
		.revents = 0,
	};
	ਪੂਰਨ ret;

	ਕਰੋ {
		ret = poll(&fds, 1, timeout);
	} ਜਦੋਂ ((ret < 0) && (errno == EINTR));

	ਵਾਪਸ (ret == 1) && (fds.revents & (POLLIN|POLLPRI));
}

ਸਥਿਰ ਪੂਰਨ open_un_socket_and_connect(ਖਾਲੀ)
{
	/* The abstract UNIX socket of plymouth */
	ਬਣਤਰ sockaddr_un su = {
		.sun_family = AF_UNIX,
		.sun_path = PLYMOUTH_SOCKET_ਰਸਤਾ,
	};
	ਅਚਲ ਪੂਰਨ one = 1;
	ਪੂਰਨ fd, ret;

	fd = socket(PF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
	ਜੇ (fd < 0) {
		warnx(_("cannot open UNIX socket"));
		ਜਾਓ err;
	}

	ret = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &one, ਆਕਾਰ(one));
	ਜੇ (ret < 0) {
		warnx(_("cannot set option for UNIX socket"));
		close(fd);
		fd = -1;
		ਜਾਓ err;
	}

	/* Note, the abstract PLYMOUTH_SOCKET_PATH has a leading NULL byte */
	ret = connect(fd, (ਬਣਤਰ sockaddr *) &su,
		offsetof(ਬਣਤਰ sockaddr_un, sun_path) + 1 + ਲੜੀ_ਲੰਬਾਈ(su.sun_path+1));
	ਜੇ (ret < 0) {
		ਜੇ (errno != ECONNREFUSED)
			warnx(_("cannot connect on UNIX socket"));
		close(fd);
		fd = -1;
		ਜਾਓ err;
	}
err:
	ਵਾਪਸ fd;
}

ਪੂਰਨ plymouth_command(ਪੂਰਨ cmd, ...)
{
	uint8_t answer[2], command[2];
	ਬਣਤਰ sigaction sp, op;
	ਪੂਰਨ fdsock = -1, ret = 0;

	sigemptyset (&sp.sa_mask);
	sp.sa_handler = SIG_IGN;
	sp.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sp, &op);

	/* The plymouthd does read at least two bytes. */
	command[1] = '\0';
	ਚੋਣ (cmd) {
	ਹਾਲਤ MAGIC_PING:
		fdsock = open_un_socket_and_connect();
		ਜੇ (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, ਆਕਾਰ(command));
		}
		ਰੋਕੋ;
	ਹਾਲਤ MAGIC_QUIT:
		fdsock = open_un_socket_and_connect();
		ਜੇ (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, ਆਕਾਰ(command));
		}
		ਰੋਕੋ;
	ਮੂਲ:
		warnx(_("the plymouth request %c is not implemented"), cmd);
	ਹਾਲਤ '?':
		ਜਾਓ err;
	}

	answer[0] = '\0';
	ਜੇ (fdsock >= 0) {
		ਜੇ (can_read(fdsock, 1000))
			ul_read_all(fdsock, (ਅੱਖਰ *) &answer[0], ਆਕਾਰ(answer));
		close(fdsock);
	}
	sigaction(SIGPIPE, &op, ਨਿਰਾ);
	ret = (answer[0] == ANSWER_ACK) ? 1 : 0;
err:
	ਵਾਪਸ ret;
}

