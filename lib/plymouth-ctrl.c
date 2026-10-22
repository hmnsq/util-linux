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

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <poll.h>
#അന്തര്ഭാവ <signal.h>
#അന്തര്ഭാവ <stdarg.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <sys/mman.h>
#അന്തര്ഭാവ <sys/socket.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/un.h>
#അന്തര്ഭാവ <unistd.h>

#അന്തര്ഭാവ "all-io.h"
#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "nls.h"
#അന്തര്ഭാവ "plymouth-ctrl.h"

സ്ഥിതം പൂർണ്ണസംഖ്യ can_read(പൂർണ്ണസംഖ്യ fd, സ്ഥിരം long timeout)
{
	ഘടന pollfd fds = {
		.fd = fd,
		.events = POLLIN|POLLPRI,
		.revents = 0,
	};
	പൂർണ്ണസംഖ്യ ret;

	ചെയ്യുക {
		ret = poll(&fds, 1, timeout);
	} ആകുമ്പോൾ ((ret < 0) && (errno == EINTR));

	തിരികെ (ret == 1) && (fds.revents & (POLLIN|POLLPRI));
}

സ്ഥിതം പൂർണ്ണസംഖ്യ open_un_socket_and_connect(ശൂന്യം)
{
	/* The abstract UNIX socket of plymouth */
	ഘടന sockaddr_un su = {
		.sun_family = AF_UNIX,
		.sun_path = PLYMOUTH_SOCKET_PATH,
	};
	സ്ഥിരം പൂർണ്ണസംഖ്യ one = 1;
	പൂർണ്ണസംഖ്യ fd, ret;

	fd = socket(PF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
	എങ്കിൽ (fd < 0) {
		warnx(_("cannot open UNIX socket"));
		പോകുക err;
	}

	ret = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &one, വലുപ്പം(one));
	എങ്കിൽ (ret < 0) {
		warnx(_("cannot set option for UNIX socket"));
		close(fd);
		fd = -1;
		പോകുക err;
	}

	/* Note, the abstract PLYMOUTH_SOCKET_PATH has a leading NULL byte */
	ret = connect(fd, (ഘടന sockaddr *) &su,
		offsetof(ഘടന sockaddr_un, sun_path) + 1 + strlen(su.sun_path+1));
	എങ്കിൽ (ret < 0) {
		എങ്കിൽ (errno != ECONNREFUSED)
			warnx(_("cannot connect on UNIX socket"));
		close(fd);
		fd = -1;
		പോകുക err;
	}
err:
	തിരികെ fd;
}

പൂർണ്ണസംഖ്യ plymouth_command(പൂർണ്ണസംഖ്യ cmd, ...)
{
	uint8_t answer[2], command[2];
	ഘടന sigaction sp, op;
	പൂർണ്ണസംഖ്യ fdsock = -1, ret = 0;

	sigemptyset (&sp.sa_mask);
	sp.sa_handler = SIG_IGN;
	sp.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sp, &op);

	/* The plymouthd does read at least two bytes. */
	command[1] = '\0';
	തിരഞ്ഞെടുപ്പ് (cmd) {
	അവസ്ഥ MAGIC_PING:
		fdsock = open_un_socket_and_connect();
		എങ്കിൽ (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, വലുപ്പം(command));
		}
		നിർത്തുക;
	അവസ്ഥ MAGIC_QUIT:
		fdsock = open_un_socket_and_connect();
		എങ്കിൽ (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, വലുപ്പം(command));
		}
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		warnx(_("the plymouth request %c is not implemented"), cmd);
	അവസ്ഥ '?':
		പോകുക err;
	}

	answer[0] = '\0';
	എങ്കിൽ (fdsock >= 0) {
		എങ്കിൽ (can_read(fdsock, 1000))
			ul_read_all(fdsock, (അക്ഷരം *) &answer[0], വലുപ്പം(answer));
		close(fdsock);
	}
	sigaction(SIGPIPE, &op, ശൂന്യ);
	ret = (answer[0] == ANSWER_ACK) ? 1 : 0;
err:
	തിരികെ ret;
}

