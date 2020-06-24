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

#include "MemoryServer.h"
#include "MemoryMessage.h"

MemoryServer::MemoryServer() : IPCServer<MemoryServer, MemoryMessage>(this)
{
    /* Register message handlers. */
    addIPCHandler(HeapGrow,    &MemoryServer::doGrow);
    addIPCHandler(HeapShrink,  &MemoryServer::doShrink);
    addIPCHandler(HeapReset,   &MemoryServer::doReset);
    addIPCHandler(MemoryUsage, &MemoryServer::doUsage);

    /* Initialize heaps. */
    for (Size i = 0; i < MAX_PROCS; i++)
    {
	heaps[i] = HEAP_START;
    }
}

void MemoryServer::doGrow(MemoryMessage *msg)
{
    Size num = 0;

    /* Allocate virtual memory pages. */
    while (num < msg->bytes && heaps[msg->from] < HEAP_END)
    {
	VMCtl(Map, msg->from, ZERO, heaps[msg->from]);
	heaps[msg->from] += PAGESIZE;
	num += PAGESIZE;
    }
    /* Update reply. */
    msg->bytes     = num;
    msg->result    = ESUCCESS;
    msg->startAddr = HEAP_START;
    msg->endAddr   = heaps[msg->from];
}

void MemoryServer::doShrink(MemoryMessage *msg)
{
    Size num = 0;
    
    /* Release virtual memory pages. */
    while (num < msg->bytes && heaps[msg->from] > HEAP_END)
    {
	VMCtl(Map, msg->from, ZERO, heaps[msg->from] - MEMALIGN, ZERO);
	heaps[msg->from] -= PAGESIZE;
	num += PAGESIZE;
    }
    /* Update reply. */
    msg->bytes     = num;
    msg->result    = ESUCCESS;
    msg->startAddr = HEAP_START;
    msg->endAddr   = heaps[msg->from];
}

void MemoryServer::doUsage(MemoryMessage *msg)
{
    SystemInformation info;
    
    /* Fill in the reply. */
    msg->bytes     = info.memorySize;
    msg->bytesFree = info.memoryAvail;
}

void MemoryServer::doReset(MemoryMessage *msg)
{
    if (msg->pid < MAX_PROCS)
    {
	heaps[msg->pid] = HEAP_START;
    }
}
