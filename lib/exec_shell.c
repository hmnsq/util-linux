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

#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <libgen.h>

#అన్తర్భావ "nls.h"
#అన్తర్భావ "c.h"
#అన్తర్భావ "xalloc.h"

#అన్తర్భావ "exec_shell.h"
#అన్తర్భావ "default_shell.h"

శూన్య __attribute__((__noreturn__)) exec_shell(శూన్య)
{
	స్థిరరాశి అక్షరం *shell = ul_default_shell(0, ఖాళీ);
	అక్షరం *shellc;
	స్థిరరాశి అక్షరం *shell_basename;
	అక్షరం *arg0;

	shellc = xstrdup(shell);
	shell_basename = basename(shellc);
	xasprintf(&arg0, "-%s", shell_basename);

	execl(shell, arg0, (అక్షరం *)ఖాళీ);
	errexec(shell);
}
