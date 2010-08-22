//# MMapfdIO.cc: Memory-mapped IO on a file
//#
//#  Copyright (C) 2009
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#  $Id: MMapfdIO.cc 20859 2010-02-03 13:14:15Z gervandiepen $

#include <casa/IO/MMapfdIO.h>
#include <casa/IO/LargeRegularFileIO.h>
#include <casa/Exceptions/Error.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <cassert>

namespace casa
{
  
  MMapfdIO::MMapfdIO()
    : itsFileSize   (0),
      itsMapOffset  (0),
      itsMapSize    (0),
      itsPosition   (0),
      itsPtr        (NULL),
      itsIsWritable (False),
	itsRealFileSize(0)
  {}

    MMapfdIO::MMapfdIO (int fd, const String& fileName)
      :  itsMapOffset  (0),
         itsMapSize    (0),
         itsPtr        (NULL),
	 itsRealFileSize(0)
  {
    map (fd, fileName);
  }

  void MMapfdIO::map (int fd, const String& fileName)
  {
    attach (fd, fileName);
    // Keep writable switch because it is used quite often.
    itsIsWritable = isWritable();
    itsRealFileSize = itsFileSize   = length();
    itsPosition   = 0;
    if (itsFileSize > 0) {
        mapFile(0, 1);
    }
  }

  MMapfdIO::~MMapfdIO()
  {
    unmapFile();
    //
    // this maybe dangerouse because we assume the actual file length was
    if(itsRealFileSize)
	    ftruncate(fd(), itsRealFileSize); 
  }

  void MMapfdIO::mapFile(Int64 offset, Int64 length)
  {
    // Unmap file if still mapped.
    if (itsPtr != NULL) {
      unmapFile();
    }
    int prot = PROT_READ;
    if (itsIsWritable) {
      prot = PROT_READ | PROT_WRITE;
    }

    // Attempt to map the entire file.
    itsMapSize = itsFileSize;
    itsMapOffset = 0;

    if ((uInt64) itsMapSize == (size_t) itsMapSize) {
        /* This condition fails for example if sizeof(size_t) = 4 and itsFileSize > 2^32,
           in which case there is no reason to even attempt to mmap the entire file. */

        itsPtr = static_cast<char*>(::mmap (0, itsMapSize, prot, MAP_SHARED,
                                            fd(), itsMapOffset));
        if (itsPtr == MAP_FAILED) {
	    itsPtr = NULL;
	    itsMapSize = 0;
	}
    }

    if (itsPtr == NULL) {

        /* It did not work to mmap the entire file. Try to map a smaller and smaller sections,
           until the memory map either succeeds, or the section gets smaller than the required
           length. */
        
        uInt pageSize = getpagesize();
	itsMapSize = itsFileSize;

        do {
            itsMapSize /= 2;

            itsMapOffset = offset - itsMapSize / 2;
            // Round down to nearest page start, as required by mmap
            itsMapOffset = itsMapOffset - itsMapOffset % pageSize;

            if (itsMapOffset < 0) {
                itsMapOffset = 0;
            }
            if (itsMapOffset + itsMapSize > itsFileSize) {
                itsMapSize = itsFileSize - itsMapOffset;
            }

            if (itsMapOffset + itsMapSize < offset + length) {
                stringstream s;
                s << "MMapfdIO::MMapfdIO - mmap " << length << " bytes from " << offset <<
                    " of " << fileName() << " failed";
                throw AipsError (s.str());
            }

	    //	    cout << "mapping " << itsMapSize << " from " << itsMapOffset << " to " << itsMapOffset + itsMapSize 
	    //		 << " (requested " << offset << " to " << offset + length << ")" << endl;

	    errno = 0;

#if (defined(AIPS_LINUX) && !defined(AIPS_64B))
	    /*
              In principle, this section should also be enabled on 64 bit Linux,
              which advertises to also provide the mmap2 system call. But in practise,
              the compile error happened: "'SYS_mmap2' was not declared in this scope".
              (This section is redundant though if mapping the entire file succeeded.)
	    */
 

	    // From man mmap2: "Glibc does not provide a wrapper for this system call; call it using syscall(2)."
            //itsPtr = static_cast<char *>(::mmap2(0, itsMapSize, prot, MAP_SHARED,
            //                                                fd(), itsMapOffset / pageSize));

            itsPtr = static_cast<char *>((void *)syscall(SYS_mmap2, NULL, (size_t)itsMapSize, prot, MAP_SHARED,
                                                         fd(), (off_t) (itsMapOffset / pageSize)));

#else
            // On some systems with sizeof(size_t) == 4 and without mmap2
            // (in particular 64-bit Macs running 32-bit executables)
            // the off_t argument is wide enough to hold a 64-bit offset
            if (itsMapOffset == (off_t) itsMapOffset) {
	        itsPtr = static_cast<char *>(::mmap(0, itsMapSize, prot, MAP_SHARED,
                                                    fd(), itsMapOffset));
	    }
            else {
	        // no mmap2 available, and size_t and off_t are too narrow, give up...

                throw AipsError("MMapfdIO::MMapfdIO - " + fileName() +
		                " is too large (" + itsFileSize + " bytes)" +
			        " to memory map on non-Linux with sizeof(size_t) = " + sizeof(size_t));
	    }
#endif
            if (itsPtr == MAP_FAILED && errno != ENOMEM) {
                itsPtr = NULL;
		itsMapSize = 0;
                throw AipsError ("MMapfdIO::MMapfdIO - mmap of " + fileName() +
                                 " failed: " + strerror(errno));
            } 
            
        } while (errno == ENOMEM);

    }
   
    assert( itsPtr != MAP_FAILED && itsPtr != NULL );

    // Optimize for sequential access.
    ::madvise (itsPtr, itsMapSize, MADV_SEQUENTIAL);
  }

  void MMapfdIO::unmapFile()
  {
    if (itsPtr != NULL) {
      int res = ::munmap (itsPtr, itsMapSize);
      itsMapSize = 0;
      if (res != 0) {
        throw AipsError ("MMapfdIO::unmapFile - munmap of " + fileName() +
                         " failed: " + strerror(errno));
      }
      itsPtr = NULL;
    }
  }

  void MMapfdIO::flush()
  {
    if (itsIsWritable  &&  itsPtr != NULL) {
      int res = ::msync (itsPtr, itsMapSize, MS_SYNC);
      if (res != 0) {
        throw AipsError ("MMapfdIO::flush - msync of " + fileName() +
                         " failed: " + strerror(errno));
      }
    }
  }

  void MMapfdIO::write (uInt size, const void* buf)
  {
    if (!itsIsWritable) {
      throw AipsError ("MMapfdIO file " + fileName() + " is not writable");
    }
    if (size > 0) {
	    // 
	    // OK, master of the kludge here, on the mac writing to extend the file or
	    // using ftruncate is pretty slow, ok painfully slow, so rather than continually
	    // extend the file with each write, lets ask for twice as much space each time
	    // this avoids a lot of remapping and speeds things up quite a bit, not as fast
	    // as the original cache scheme but we can tune later. When we're all done we 
	    // trim back the file to desired size. anyway seems to work.
	    //
        // Remap if required section is not currently mapped
        if (itsPosition < itsMapOffset || itsPosition + size > itsMapOffset + itsMapSize) {
            unmapFile();
            // If past end-of-file, write the last byte to extend the file.
            if (itsPosition + size > itsFileSize) {
                itsFileSize = itsPosition + size;
		/*
                LargeFiledesIO::doSeek (itsFileSize-1, ByteIO::Begin);
                char b=0;
                LargeFiledesIO::write (1, &b);
		*/
		itsFileSize += (1024*1024*20);
		ftruncate(fd(), itsFileSize);
            }
            mapFile(itsPosition, size);
        }
        memcpy(itsPtr + itsPosition - itsMapOffset, buf, size);
        itsPosition += size;
	itsRealFileSize = (itsRealFileSize < itsPosition) ? itsPosition : itsRealFileSize;
    }
  }

  Int MMapfdIO::read (uInt size, void* buf, Bool throwException)
  {
    Int szrd = size;
    if (itsPosition >= itsFileSize) {
      szrd = 0;
    } else if (itsPosition+size > itsFileSize) {
      szrd = itsFileSize - itsPosition;
    }
    if (szrd > 0) {
        // Remap if required
        if (itsPosition < itsMapOffset || itsPosition + szrd > itsMapOffset + itsMapSize) {
            mapFile(itsPosition, szrd);
        }
      memcpy (buf, itsPtr + itsPosition - itsMapOffset, szrd);
      itsPosition += szrd;
      if (throwException  &&  szrd < Int(size)) {
        throw AipsError ("MMapfdIO::read - " + fileName() +
                         " incorrect number of bytes read");
      }
    }
    return szrd;
  }

  Int64 MMapfdIO::doSeek (Int64 offset, ByteIO::SeekOption dir)
  {
    itsPosition = LargeFiledesIO::doSeek (offset, dir);
    return itsPosition;
  }

    const void* MMapfdIO::getReadPointer (Int64 offset, Int64 length) const
  {
    if (offset >= itsFileSize) {
      throw AipsError ("MMapfdIO::getReadPointer: beyond EOF of "
                       + fileName());
    }

    // Remap if required
    if (offset < itsMapOffset || offset + length > itsMapOffset + itsMapSize) {
        const_cast<MMapfdIO *>(this)->mapFile(offset, length);
    }
   
    return itsPtr + (offset - itsMapOffset);
  }

  void* MMapfdIO::getWritePointer (Int64 offset, Int64 length)
  {
    if (!itsIsWritable) {
      throw AipsError ("MMapfdIO file " + fileName() + " is not writable");
    }
    if (offset >= itsFileSize) {
      throw AipsError ("MMapfdIO::getWritePointer: beyond EOF of "
                       + fileName());
    }
    // Remap if required
    if (offset < itsMapOffset || offset + length > itsMapOffset + itsMapSize) {
        mapFile(offset, length);
    }

    return itsPtr + offset - itsMapOffset;
  }

} // end namespace
