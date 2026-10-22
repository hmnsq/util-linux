/*
 * exec_shell() - launch a shell, else exit!
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://gnu.org/licenses/>.
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <libgen.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "nls.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "xalloc.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "exec_shell.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "default_shell.h"

ਖਾਲੀ __attribute__((__noreturn__)) exec_shell(ਖਾਲੀ)
{
	ਅਚਲ ਅੱਖਰ *shell = ul_default_shell(0, ਨਿਰਾ);
	ਅੱਖਰ *shellc;
	ਅਚਲ ਅੱਖਰ *shell_basename;
	ਅੱਖਰ *arg0;

	shellc = xstrdup(shell);
	shell_basename = basename(shellc);
	xasprintf(&arg0, "-%s", shell_basename);

	execl(shell, arg0, (ਅੱਖਰ *)ਨਿਰਾ);
	errexec(shell);
}
