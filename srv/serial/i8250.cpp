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

#include <API/ProcessCtl.h>
#include <FreeNOS/CPU.h>
#include <Macros.h>
#include <Types.h>
#include <Config.h>
#include "i8250.h"

i8250::i8250(u16 b, u16 i)
    : SerialDevice(b, i)
{
    /* Aquire I/O port and IRQ line permissions. */
    ProcessCtl(SELF, AllowIO,  base);
    ProcessCtl(SELF, AllowIO,  base + LINESTATUS);
    ProcessCtl(SELF, AllowIO,  base + LINECONTROL);
    ProcessCtl(SELF, AllowIO,  base + IRQCONTROL);
    ProcessCtl(SELF, AllowIO,  base + FIFOCONTROL);
    ProcessCtl(SELF, AllowIO,  base + MODEMCONTROL);
    ProcessCtl(SELF, AllowIO,  base + DIVISORLOW);
    ProcessCtl(SELF, AllowIO,  base + DIVISORHIGH);
    ProcessCtl(SELF, WatchIRQ, irq);
    
    /* 8bit Words, no parity. */
    outb(base + LINECONTROL, 3);
    
    /* Enable interrupts. */
    outb(base + IRQCONTROL, 1);
    
    /* No FIFO. */
    outb(base + FIFOCONTROL, 0);
    
    /* Data Ready, Request to Send. */
    outb(base + MODEMCONTROL, 3);
    
    /* Set baudrate. */
    outb(base + LINECONTROL, inb(base + LINECONTROL) | DLAB);
    outb(base + DIVISORLOW,  (11500 / BAUDRATE) & 0xff);
    outb(base + DIVISORHIGH, (11500 / BAUDRATE) >> 8);
    outb(base + LINECONTROL, inb(base + LINECONTROL) & ~DLAB);
}

Error i8250::read(s8 *buffer, Size size)
{
    Size bytes = 0;
    
    /* Read as much bytes as possible. */
    while (inb(base + LINESTATUS) & RXREADY && bytes < size)
    {
	buffer[bytes++] = inb(base);
    }
    return (bytes);
}
								     
Error i8250::write(s8 *buffer, Size size)
{
    Size bytes = 0;

    /* Write as much bytes as possible. */
    while (inb(base + LINESTATUS) & TXREADY && bytes < size)
    {
    	outb(base, buffer[bytes++]);
    }
    return (bytes);
}
