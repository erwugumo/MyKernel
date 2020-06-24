/*
 * Copyright (C) 2009 Niek Linnenbank
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <API/IPCMessage.h>
#include <API/ProcessCtl.h>
#include <FreeNOS/Process.h>
#include <FreeNOS/CPU.h>
#include <MemoryServer.h>
#include <FileSystemMessage.h>
#include <VirtualFileSystem.h>
#include <TerminalCodes.h>
#include <Config.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include "Shell.h"

Shell::Shell()
{
    /* TODO: Temporarily solution: wait a while until services have started. */
    for (int i = 0; i < 50000; i++)
    {
	ProcessCtl(ANY, Schedule);
    }
    /* Initialize terminal as standard I/O. */
    for (int i = 0; i < 3; i++)
    {
	while (open("/dev/tty0", ZERO) == -1);
    }
    /* Show the user where to get help. */
    printf("\r\n"
	   "Entering Shell. Type 'help' for the command list.\r\n"
	   "\r\n");
}

int Shell::run()
{
    char *cmdStr, *argv[MAX_ARGV];
    char tmp[128];
    ShellCommand *cmd;
    Size argc;
    int pid, status;

    /* Read commands. */    
    while (true)
    {
	/* Print the prompt. */
	prompt();
	
	/* Wait for a command string. */
	cmdStr = getCommand();
	
	/* Enough input? */
	if (strlen(cmdStr) == 0)
	{
	    continue;
	}
	/* Attempt to extract arguments. */
	argc = parse(cmdStr, argv, MAX_ARGV);
	
	/* Do we have a matching ShellCommand? */
	if (!(cmd = ShellCommand::byName(argv[0])))
	{
	    /* If not, try to execute it as a file directly. */
	    if ((pid = forkexec(argv[0], (const char **) argv)) >= 0)
	    {
		waitpid(pid, &status, 0);
	    }
	    /* Try to find it on the livecd filesystem. (temporary hardcoded PATH) */
	    else if ((snprintf(tmp, sizeof(tmp), "/img/bin/%s/%s",  argv[0], argv[0]) &&
	            ((pid = forkexec(tmp, (const char **) argv)) >= 0)) ||
		     (snprintf(tmp, sizeof(tmp), "/img/sbin/%s/%s", argv[0], argv[0]) &&
	            ((pid = forkexec(tmp, (const char **) argv)) >= 0)))
	    {
		waitpid(pid, &status, 0);
	    }
	    else
		printf("forkexec '%s' failed: %s\r\n", argv[0],
			strerror(errno));
	}
	/* Enough arguments given? */
	else if (argc - 1 < cmd->getMinimumParams())
	{
	    printf("%s: not enough arguments (%u required)\r\n",
		    cmd->getName(), cmd->getMinimumParams());
	}
	/* Execute it. */
	else
	{
	    cmd->execute(argc - 1, argv + 1);
	}
    }
    return 0;
}

char * Shell::getCommand()
{
    static char line[1024];
    Size total = 0;

    /* Read a line. */
    while (total < sizeof(line))
    {
        /* Read a character. */
	read(0, line + total, 1);
	
	/* Process character. */
	switch (line[total])
	{
	    case '\r':
	    case '\n':
	    	printf("\r\n");
		line[total] = ZERO;
		return line;

	    case '\b':
		if (total > 0)
		{
		    total--;
		    printf("\b \b");
		}
		break;
	    
	    default:
		printf("%c", line[total]);
		total++;
		break;
	}
    }
    line[total] = ZERO;
    return line;
}

void Shell::prompt()
{
    char tmp[128];
    
    /* Retrieve current hostname. */
    gethostname(tmp, sizeof(tmp));
    
    /* Print out the prompt. */
    printf(WHITE "(" GREEN "%s" WHITE ") # ", tmp);
}

Size Shell::parse(char *cmdline, char **argv, Size maxArgv)
{
    Size argc;
    
    for (argc = 0; argc < maxArgv && *cmdline; argc++)
    {
	while (*cmdline && *cmdline == ' ')
	    cmdline++;
	
	argv[argc] = cmdline;
	
	while (*cmdline && *cmdline != ' ')
	    cmdline++;
	
	if (*cmdline) *cmdline++ = ZERO;
    }
    return argc;
}
