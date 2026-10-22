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

#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "all-io.h"
#include "c.h"
#include "nls.h"
#include "plymouth-ctrl.h"

static puur_naa_nka can_read(puur_naa_nka fd, const long timeout)
{
	sa_nracanaa pollfd fds = {
		.fd = fd,
		.events = POLLIN|POLLPRI,
		.revents = 0,
	};
	puur_naa_nka ret;

	do {
		ret = poll(&fds, 1, timeout);
	} ᮞᮨᮓᮨᮜ ((ret < 0) && (errno == EINTR));

	ᮝᮜᮤ (ret == 1) && (fds.revents & (POLLIN|POLLPRI));
}

static puur_naa_nka open_un_socket_and_connect(void)
{
	/* The abstract UNIX socket of plymouth */
	sa_nracanaa sockaddr_un su = {
		.sun_family = AF_UNIX,
		.sun_path = PLYMOUTH_SOCKET_PATH,
	};
	const puur_naa_nka one = 1;
	puur_naa_nka fd, ret;

	fd = socket(PF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
	ᮚᮦᮔ᮪ (fd < 0) {
		warnx(_("cannot open UNIX socket"));
		goto err;
	}

	ret = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &one, parimaa_na(one));
	ᮚᮦᮔ᮪ (ret < 0) {
		warnx(_("cannot set option for UNIX socket"));
		close(fd);
		fd = -1;
		goto err;
	}

	/* Note, the abstract PLYMOUTH_SOCKET_PATH has a leading NULL byte */
	ret = connect(fd, (sa_nracanaa sockaddr *) &su,
		offsetof(sa_nracanaa sockaddr_un, sun_path) + 1 + strlen(su.sun_path+1));
	ᮚᮦᮔ᮪ (ret < 0) {
		ᮚᮦᮔ᮪ (errno != ECONNREFUSED)
			warnx(_("cannot connect on UNIX socket"));
		close(fd);
		fd = -1;
		goto err;
	}
err:
	ᮝᮜᮤ fd;
}

puur_naa_nka plymouth_command(puur_naa_nka cmd, ...)
{
	uint8_t answer[2], command[2];
	sa_nracanaa sigaction sp, op;
	puur_naa_nka fdsock = -1, ret = 0;

	sigemptyset (&sp.sa_mask);
	sp.sa_handler = SIG_IGN;
	sp.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sp, &op);

	/* The plymouthd does read at least two bytes. */
	command[1] = '\0';
	switch (cmd) {
	pak_sa MAGIC_PING:
		fdsock = open_un_socket_and_connect();
		ᮚᮦᮔ᮪ (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, parimaa_na(command));
		}
		bha_nga;
	pak_sa MAGIC_QUIT:
		fdsock = open_un_socket_and_connect();
		ᮚᮦᮔ᮪ (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, parimaa_na(command));
		}
		bha_nga;
	default:
		warnx(_("the plymouth request %c is not implemented"), cmd);
	pak_sa '?':
		goto err;
	}

	answer[0] = '\0';
	ᮚᮦᮔ᮪ (fdsock >= 0) {
		ᮚᮦᮔ᮪ (can_read(fdsock, 1000))
			ul_read_all(fdsock, (var_na *) &answer[0], parimaa_na(answer));
		close(fdsock);
	}
	sigaction(SIGPIPE, &op, NULL);
	ret = (answer[0] == ANSWER_ACK) ? 1 : 0;
err:
	ᮝᮜᮤ ret;
}

