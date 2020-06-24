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

#include <FreeNOS/CPU.h>
#include <API/ProcessCtl.h>
#include <API/IPCMessage.h>
#include <API/VMCtl.h>
#include <MemoryMessage.h>
#include <Config.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    u64 t1 = 0, t2 = 0;
    ProcessID pid = 0;
    ProcessInfo info;
    char *foo[128];

    /* Initialize terminal as standard I/O. */
    for (int i = 0; i < 3; i++)
    {
        while (open("/dev/tty0", ZERO) < 0);
    }
    t1 = timestamp();
    pid = ProcessCtl(SELF, GetPID);
    t2 = timestamp();
	
    printf("SystemCall (GetPID) Ticks: %u\r\n", t2 - t1);
	
    t1 = timestamp();
    ProcessCtl(SELF, InfoPID, (Address) &info);
    t2 = timestamp();

    printf("SystemCall (InfoPID) Ticks: %u\r\n", t2 - t1);

    t1 = timestamp();
    ProcessCtl(SELF, Schedule);
    t2 = timestamp();

    printf("SystemCall (Schedule) Ticks: %u\r\n", t2 - t1);
	
    t1 = timestamp();
    VMCtl(Lookup, SELF, ZERO, 0x80000000);
    t2 = timestamp();
	
    printf("SystemCall (VMCtl) Ticks: %u\r\n", t2 - t1);

    t1 = timestamp();
    getpid();
    t2 = timestamp();

    printf("IPC Ticks: %u\r\n", t2 - t1);
	
    t1 = timestamp();
    for (int i = 0; i < 128; i++)
        foo[i] = new char[16];
    t2 = timestamp();
	
    printf("allocate() Ticks: %u (%u AVG)\r\n",
    	    (u32)(t2 - t1), (u32)(t2 - t1) / 128);
	
    t1 = timestamp();
    for (int i = 0; i < 128; i++)
        delete foo[i];
    t2 = timestamp();
    printf("release() Ticks: %u (%u AVG)\r\n",
	(u32)(t2 - t1), (u32)(t2 - t1) / 128);

    /* Done. */
    return EXIT_SUCCESS;
}
