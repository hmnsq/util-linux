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

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <poll.h>
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <sys/mman.h>
#அந்தர்பாவ <sys/socket.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/un.h>
#அந்தர்பாவ <unistd.h>

#அந்தர்பாவ "all-io.h"
#அந்தர்பாவ "c.h"
#அந்தர்பாவ "nls.h"
#அந்தர்பாவ "plymouth-ctrl.h"

நிலையான முழு can_read(முழு fd, மாறிலி long timeout)
{
	கட்டமைப்பு pollfd fds = {
		.fd = fd,
		.events = POLLIN|POLLPRI,
		.revents = 0,
	};
	முழு ret;

	செய் {
		ret = poll(&fds, 1, timeout);
	} போது ((ret < 0) && (errno == EINTR));

	திரும்பு (ret == 1) && (fds.revents & (POLLIN|POLLPRI));
}

நிலையான முழு open_un_socket_and_connect(வெறுமை)
{
	/* The abstract UNIX socket of plymouth */
	கட்டமைப்பு sockaddr_un su = {
		.sun_family = AF_UNIX,
		.sun_path = PLYMOUTH_SOCKET_PATH,
	};
	மாறிலி முழு one = 1;
	முழு fd, ret;

	fd = socket(PF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
	எனில் (fd < 0) {
		warnx(_("cannot open UNIX socket"));
		செல் err;
	}

	ret = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &one, அளவு(one));
	எனில் (ret < 0) {
		warnx(_("cannot set option for UNIX socket"));
		close(fd);
		fd = -1;
		செல் err;
	}

	/* Note, the abstract PLYMOUTH_SOCKET_PATH has a leading NULL byte */
	ret = connect(fd, (கட்டமைப்பு sockaddr *) &su,
		offsetof(கட்டமைப்பு sockaddr_un, sun_path) + 1 + strlen(su.sun_path+1));
	எனில் (ret < 0) {
		எனில் (errno != ECONNREFUSED)
			warnx(_("cannot connect on UNIX socket"));
		close(fd);
		fd = -1;
		செல் err;
	}
err:
	திரும்பு fd;
}

முழு plymouth_command(முழு cmd, ...)
{
	uint8_t answer[2], command[2];
	கட்டமைப்பு sigaction sp, op;
	முழு fdsock = -1, ret = 0;

	sigemptyset (&sp.sa_mask);
	sp.sa_handler = SIG_IGN;
	sp.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sp, &op);

	/* The plymouthd does read at least two bytes. */
	command[1] = '\0';
	தேர்வு (cmd) {
	நிலை MAGIC_PING:
		fdsock = open_un_socket_and_connect();
		எனில் (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, அளவு(command));
		}
		நிறுத்து;
	நிலை MAGIC_QUIT:
		fdsock = open_un_socket_and_connect();
		எனில் (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, அளவு(command));
		}
		நிறுத்து;
	இயல்பு:
		warnx(_("the plymouth request %c is not implemented"), cmd);
	நிலை '?':
		செல் err;
	}

	answer[0] = '\0';
	எனில் (fdsock >= 0) {
		எனில் (can_read(fdsock, 1000))
			ul_read_all(fdsock, (எழுத்து *) &answer[0], அளவு(answer));
		close(fdsock);
	}
	sigaction(SIGPIPE, &op, வெற்று);
	ret = (answer[0] == ANSWER_ACK) ? 1 : 0;
err:
	திரும்பு ret;
}

