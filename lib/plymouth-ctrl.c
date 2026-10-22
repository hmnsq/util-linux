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

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <poll.h>
#અન્તર્ભાવ <signal.h>
#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <sys/mman.h>
#અન્તર્ભાવ <sys/socket.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/un.h>
#અન્તર્ભાવ <unistd.h>

#અન્તર્ભાવ "all-io.h"
#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "nls.h"
#અન્તર્ભાવ "plymouth-ctrl.h"

સ્થિર પૂર્ણાંક can_read(પૂર્ણાંક fd, અચલ long timeout)
{
	સંરચના pollfd fds = {
		.fd = fd,
		.events = POLLIN|POLLPRI,
		.revents = 0,
	};
	પૂર્ણાંક ret;

	કરો {
		ret = poll(&fds, 1, timeout);
	} જ્યારે ((ret < 0) && (errno == EINTR));

	પાછા (ret == 1) && (fds.revents & (POLLIN|POLLPRI));
}

સ્થિર પૂર્ણાંક open_un_socket_and_connect(શૂન્ય)
{
	/* The abstract UNIX socket of plymouth */
	સંરચના sockaddr_un su = {
		.sun_family = AF_UNIX,
		.sun_path = PLYMOUTH_SOCKET_PATH,
	};
	અચલ પૂર્ણાંક one = 1;
	પૂર્ણાંક fd, ret;

	fd = socket(PF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
	જો (fd < 0) {
		warnx(_("cannot open UNIX socket"));
		જાઓ err;
	}

	ret = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &one, કદ(one));
	જો (ret < 0) {
		warnx(_("cannot set option for UNIX socket"));
		close(fd);
		fd = -1;
		જાઓ err;
	}

	/* Note, the abstract PLYMOUTH_SOCKET_PATH has a leading NULL byte */
	ret = connect(fd, (સંરચના sockaddr *) &su,
		offsetof(સંરચના sockaddr_un, sun_path) + 1 + strlen(su.sun_path+1));
	જો (ret < 0) {
		જો (errno != ECONNREFUSED)
			warnx(_("cannot connect on UNIX socket"));
		close(fd);
		fd = -1;
		જાઓ err;
	}
err:
	પાછા fd;
}

પૂર્ણાંક plymouth_command(પૂર્ણાંક cmd, ...)
{
	uint8_t answer[2], command[2];
	સંરચના sigaction sp, op;
	પૂર્ણાંક fdsock = -1, ret = 0;

	sigemptyset (&sp.sa_mask);
	sp.sa_handler = SIG_IGN;
	sp.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sp, &op);

	/* The plymouthd does read at least two bytes. */
	command[1] = '\0';
	પસંદગી (cmd) {
	સ્થિતિ MAGIC_PING:
		fdsock = open_un_socket_and_connect();
		જો (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, કદ(command));
		}
		તોડો;
	સ્થિતિ MAGIC_QUIT:
		fdsock = open_un_socket_and_connect();
		જો (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, કદ(command));
		}
		તોડો;
	મૂળભૂત:
		warnx(_("the plymouth request %c is not implemented"), cmd);
	સ્થિતિ '?':
		જાઓ err;
	}

	answer[0] = '\0';
	જો (fdsock >= 0) {
		જો (can_read(fdsock, 1000))
			ul_read_all(fdsock, (અક્ષર *) &answer[0], કદ(answer));
		close(fdsock);
	}
	sigaction(SIGPIPE, &op, ખાલી);
	ret = (answer[0] == ANSWER_ACK) ? 1 : 0;
err:
	પાછા ret;
}

