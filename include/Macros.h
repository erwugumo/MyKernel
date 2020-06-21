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

#ifndef __MACROS_H
#define __MACROS_H

#include "Types.h"

/** Binary 1 means true. */
#define TRUE		1

/** Binary zero means false. */
#define FALSE		0

/** NULL means zero. */
#define NULL		(void *)0

/** Zero value. */
#define ZERO		0

/** Stringfies the given input. */
#define QUOTE(x)	#x

/** Calculates offsets in data structures. */
#define offsetof(TYPE, MEMBER) ((Size) &((TYPE *)0)->MEMBER)

/** Used to define external C functions. */
#ifdef __cplusplus
#define C "C"
#else
#define C
#endif /* c_plusplus */

#endif /* __MACROS_H */
