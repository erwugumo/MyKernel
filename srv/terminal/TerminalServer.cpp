/**
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

#include <api/IPCMessage.h>
#include <api/VMCopy.h>
#include <Types.h>
#include <Version.h>
#include "TerminalServer.h"

/**
 * Short banner to print out startup.
 */
#define BANNER \
    "FreeNOS " RELEASE " (" BUILDUSER "@" BUILDHOST ") (" COMPILER ") " DATETIME "\n"

/**
 * Macro to output a message from inside the MemoryServer.
 * @param str Static character string to print out.
 */
#define OUTPUT(str) \
({ \
    TerminalMessage msg; \
    \
    /* Construct message. */ \
    msg.action = TerminalWrite; \
    msg.buffer = str; \
    msg.size   = strlen(str); \
    \
    /* Output banner. */ \
    IPCMessage(TERMINAL_PID, Send, &msg); \
})

TerminalServer::TerminalServer() : reading(false)
{
    /* Clear VGA. */
    vga.clear();

    /* Output banner and copyrights. */
    OUTPUT(BANNER);
    OUTPUT(COPYRIGHT);
}
	
int TerminalServer::run()
{
    TerminalMessage msg;

    /* Enter loop. */
    while (true)
    {
	/* Now wait for a message. */
	IPCMessage(ZERO, Receive, &msg);

	/* Handle various message types. */
	switch (msg.type)
	{
	    /* Flush I/O buffers. */
	    case IRQType:
		doIRQ((InterruptMessage *) &msg);
		break;

	    /* Incoming user message. */
	    case IPCType:

		/* Perform appropriate action. */
		switch (msg.action)
		{
		    case TerminalRead:
			doRead(&msg);
			break;
		    
		    case TerminalWrite:
			doWrite(&msg);
			break;
		
		    default:
			continue;
		}	
		break;
		
	    default:
		;
	}
    }
    /* Satify compiler. */
    return 0;
}

void TerminalServer::doIRQ(InterruptMessage *msg)
{
    vga.flush();
    ps2.flush();
    uart.flush();
    
    if (reading)
	doRead(&lastRead);
}

void TerminalServer::doRead(TerminalMessage *msg)
{
    TerminalMessage reply;
    s8 buf[BUFFERSIZE];
    Size sz = msg->size > sizeof(buf) ? sizeof(buf) : msg->size;
    
    /* Attempt to read a message. */
    if ((reply.size = uart.bufferedRead(buf, sz)) > 0 ||
        (reply.size = ps2.bufferedRead(buf, sz)) > 0)
    {
	/* Copy to remote process. */
	VMCopy(msg->from, Write, (Address) &buf, (Address) msg->buffer, sz);
    
	/* Success. */
	reply.action = TerminalOK;
	IPCMessage(msg->from, Send, &reply);
	reading      = false;
    }
    /* Save the request. */
    else
    {
	lastRead = msg; 
	reading  = true;
    }
}

void TerminalServer::doWrite(TerminalMessage *msg)
{
    TerminalMessage reply;
    s8 buf[BUFFERSIZE];
    Size sz = msg->size > sizeof(buf) ? sizeof(buf) : msg->size;

    /* Copy the string. */
    VMCopy(msg->from, Read, (Address) &buf, (Address) msg->buffer, sz);
	
    /* Write to our Terminals. */
    vga.bufferedWrite(buf, sz);
    reply.size   = uart.bufferedWrite(buf, sz);
    reply.action = TerminalOK;

    /* Send Reply. */
    IPCMessage(msg->from, Send, &reply);

    /* Flush buffers. */
    vga.flush();
    uart.flush();
}

