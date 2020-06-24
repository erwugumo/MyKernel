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
#include <ProcessMessage.h>
#include "wait.h"
#include "types.h"

pid_t waitpid(pid_t pid, int *stat_loc, int options)
{
    ProcessMessage msg;
    
    /* Fill in the message. */
    msg.action = WaitProcess;
    msg.number = pid;
    
    /* Ask ProcessServer. */
    IPCMessage(PROCSRV_PID, SendReceive, &msg, sizeof(msg));

    /* Did we succeed? */
    if (msg.result == ESUCCESS)
    {
	if (stat_loc)
	{
	    *stat_loc = msg.number;
	}
	return pid;
    }
    else
	return msg.result;
}
