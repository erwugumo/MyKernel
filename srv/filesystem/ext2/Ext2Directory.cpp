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

#include <Types.h>
#include <dirent.h>
#include "Ext2Directory.h"
#include "Ext2FileSystem.h"
#include "Ext2Inode.h"

Ext2Directory::Ext2Directory(Ext2FileSystem *f,
			     Ext2Inode *i)
    : ext2(f), inode(i)
{
    size = inode->size;
}

Error Ext2Directory::read(FileSystemMessage *msg)
{
    Ext2DirectoryEntry ext2Dent;
    Ext2Inode *ext2Inode;
    Ext2SuperBlock *sb = ext2->getSuperBlock();
    Dirent dent, *buf = (Dirent *) msg->buffer;
    Size bytes = ZERO;
    u64 offset;
    Error e;

    /* Loop all blocks. */
    for (Size blk = 0; blk < inode->blocks - 1; blk++)
    {
        /* We don't support indirect blocks (yet). */
    	if (blk >= EXT2_NDIR_BLOCKS)
    	{
    	    e = ENOTSUP;
    	    break;
    	}
	/* Read directory entries. */
	for (Size ent = 0; ent < EXT2_BLOCK_SIZE(sb); )
	{
	    /* Calculate offset to read. */
	    offset = inode->block[blk] * EXT2_BLOCK_SIZE(sb) + ent;

	    /* Get the next entry. */
	    if (ext2->getStorage()->read(offset, (u8 *) &ext2Dent,
				         sizeof(Ext2DirectoryEntry)) < 0)
	    {
		return EACCES;
	    }
	    /* Is it a valid entry? */
	    else if (!ext2Dent.recordLength || !ext2Dent.inode)
	    {
		break;
	    }
	    /* Can we read another entry? */
            else if (bytes + sizeof(Dirent) <= msg->size)
            {
		/* Fill in the Dirent. */
		if (!(ext2Inode = ext2->getInode(ext2Dent.inode)))
		{
		    return EINVAL;
		}
		strlcpy(dent.d_name, ext2Dent.name, ext2Dent.nameLength + 1);
		dent.d_type = EXT2_FILETYPE(ext2Inode);

		/* Copy to the remote process. */		    
                if ((e = VMCopy(msg->procID, Write, (Address) &dent,
                               (Address) (buf++), sizeof(Dirent))) < 0)
                {
                    return e;
                }
                bytes += e;
		ent   += ext2Dent.recordLength;
            }
	    /* No more buffer space left. */
            else
	    {
		return EFAULT;
	    }
	}
    }
    /* All done. */
    msg->size = bytes;
    return ESUCCESS;
}

Error Ext2Directory::getEntry(Ext2DirectoryEntry *dent, char *name)
{
    Ext2SuperBlock *sb = ext2->getSuperBlock();
    Size offset;

    /* Loop all blocks. */
    for (Size blk = 0; blk < inode->blocks - 1; blk++)
    {
	/* Read directory entries. */
	for (Size ent = 0; ent < EXT2_BLOCK_SIZE(sb); )
	{
	    /* Calculate offset to read. */
	    offset = inode->block[blk] * EXT2_BLOCK_SIZE(sb) + ent;

	    /* Get the next entry. */
	    if (ext2->getStorage()->read(offset, (u8 *) dent,
				         sizeof(Ext2DirectoryEntry)) < 0)
	    {
		return EACCES;
	    }
	    /* Does it have a valid length? */
	    else if (!dent->recordLength)
	    {
		return EINVAL;
	    }
	    /* Is it the entry we are looking for? */
	    if (strncmp(name, dent->name,
			      dent->nameLength < strlen(name) ?
			      dent->nameLength : strlen(name)) == 0)
	    {
		return ESUCCESS;
	    }
	    ent += dent->recordLength;
	}
    }
    return ENOENT;
}
