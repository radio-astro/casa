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
      itsTotalMapSize(0),
      itsPosition   (0),
      itsPtr        (NULL),
      itsIsWritable (False)
  {}

    MMapfdIO::MMapfdIO (int fd, const String& fileName)
      :  itsMapOffset  (0),
         itsMapSize    (0),
	 itsTotalMapSize(0),
         itsPtr        (NULL)
  {
    map (fd, fileName);
  }

  void MMapfdIO::map (int fd, const String& fileName)
  {
    attach (fd, fileName);
    // Keep writable switch because it is used quite often.
    itsIsWritable = isWritable();
    itsFileSize   = length();
    itsPosition   = 0;
    if (itsFileSize > 0) {
        mapFile(0, 1);
    }
  }

  MMapfdIO::~MMapfdIO()
  {
    unmapFile();
  }

  void MMapfdIO::mapFile(Int64 offset, Int64 length)
  {
#if defined (AIPS_DARWIN) && defined(AIPS_64B)
      /* On OS X 10.6 do not unmap the previous mapped section. Calling munmap()
         has a cost which is proportional to the file size, even when unmapping
         just a small part of the file. This seems like a problem in munmap()
         which is specific to 10.6, not seen in 10.5 nor on Linux. When the file
         is remapped often, this is a high cost. Instead, postpone all the
         unmapping until the object is destroyed. This has a much lower cost per
         munmap() call.

         The 10.6 approach is a bad approach on non-10.6 because not unmapping
         means spending physical memory, and starting to swap and perform badly
         when out of physical memory. 10.6 is different in this respect too: Not
         unmapping also spends physical memory, but the unused pages get swapped
         out with no significant performance cost.

         Not unmapping costs virtual memory, of which there is theoretically
         256 TB available on 10.6, and in practise more than 100 TB. This is enough
         to hold several TB-sized files.

         For random access patterns the virtual memory usage could grow very
         rapidly (depending on user parameters); therefore, as safety, flush the
         mapped sections once in a while. "Once in a while" should be seldom enough
         to keep the total unmapping costs down, i.e. something that scales with
         the file size. Add 1 GB to the threshold in order to avoid munmap overhead
         for small files.
      */
    if (itsTotalMapSize > 1024*1024*1024 + 10 * itsFileSize) {
      unmapFile();
    }
#else
    unmapFile();
#endif
    int prot = PROT_READ;
    if (itsIsWritable) {
      prot = PROT_READ | PROT_WRITE;
    }
    
    /* Mapping the entire file is runtime expensive on OS X 10.5 and 10.6
       (though it is fine on Linux); therefore map only the section of the
       file which is needed. */

      uInt pageSize = getpagesize();
      Int64 startPage = offset / pageSize;
      Int64 endPage = (offset + length - 1) / pageSize;
      
      /* Empirically, it was found that for a repeated remapping to not 
	 have too much overhead, the mapped size should be in the order of
         MB, i.e. thousands of pages (given the pagesize of 4K used in current
         OSs). Notice that if this code is used by TSM table columns with
         tile sizes >= 1MB, the requested length is already large enough,
         and the following extension of the mapped region is redundant
         (but not harmful) */
      startPage -= 1024;
      endPage += 1024;

      itsMapOffset = pageSize * startPage;
      itsMapSize   = pageSize * (endPage - startPage + 1);
      
      if (itsMapOffset < 0) {
	itsMapOffset = 0;
      }

      if (itsMapOffset + itsMapSize > itsFileSize) {
	itsMapSize = itsFileSize - itsMapOffset;
      }
      
#if (defined(AIPS_LINUX) && !defined(AIPS_64B))
      /*  Linux provides mmap2 to deal with the mapping of large files.

          In principle, this section could be also enabled on 64 bit Linux,
	  which advertises to also provide the mmap2 system call. But in practise,
	  the compile error happened: "'SYS_mmap2' was not declared in this scope".
      
          From man mmap2: "Glibc does not provide a wrapper for this system call; call it using syscall(2)."
	  Okay.
      */

      //itsPtr = static_cast<char *>(::mmap2(0, itsMapSize, prot, MAP_SHARED,
      //                                                fd(), itsMapOffset / pageSize));

      itsPtr = static_cast<char *>((void *)
				   syscall(SYS_mmap2, NULL, (size_t)itsMapSize, prot, MAP_SHARED,
					   fd(), (off_t) (itsMapOffset / pageSize)));
#else 
      /* Not 32-bit Linux */
      
      /* The following two tests should never fail on known OSs, 
	 but rather be safe than sorry */
      if ((uInt64) itsMapSize != (size_t) itsMapSize) {
	/* size_t might be 32-bit, but the map size is in the order of megabytes. */
	throw AipsError("MMapfdIO::MMapfdIO - mmap of " + fileName() +
			" failed, map size " + itsMapSize + " is too large " +
			"for sizeof(size_t) = " + sizeof(size_t));
      }
      if (itsMapOffset != (off_t) itsMapOffset) {
	/* This would fail for large files on 32-bit Linux,
           not on 32-bit builds on Mac though, where sizeof(off_t) = 8. */
	throw AipsError("MMapfdIO::MMapfdIO - mmap of " + fileName() +
			" failed, map offset " + itsMapOffset + " is too large " +
			" for sizeof(off_t) = " + sizeof(off_t));
      }
 
      itsPtr = static_cast<char*>(::mmap (0, itsMapSize, prot, MAP_SHARED,
					  fd(), itsMapOffset));

      /* In the case that the virtual memory space ever becomes too small, one
         could try this,
         if (itsPtr == MAP_FAILED) {
             unmapFile();
             itsPtr = static_cast<char*>(::mmap (0, itsMapSize, prot, MAP_SHARED,
                                               fd(), itsMapOffset));
         }
      */
#endif
      if (itsPtr == MAP_FAILED) {
	itsPtr = NULL;
	itsMapSize = 0;
	throw AipsError("MMapfdIO::MMapfdIO - mmap of " + fileName() +
			" " + itsMapSize + " bytes from " + itsMapOffset +
			" failed: " + strerror(errno));
      }
        
      // Optimize for sequential access.
      ::madvise (itsPtr, itsMapSize, MADV_SEQUENTIAL);

      // Remember this mapping
      itsMappedRegions.insert(std::pair<char *, Int64>(itsPtr, itsMapSize));
      itsTotalMapSize += itsMapSize;

      return;
  }

  void MMapfdIO::unmapFile()
  {
      /* Unmap all sections of the file */
      for (std::set<std::pair<char *, Int64> >::const_iterator i = itsMappedRegions.begin();
           i != itsMappedRegions.end();
           i++) {

          int res = ::munmap (i->first, i->second);
          if (res != 0) {
              throw AipsError ("MMapfdIO::unmapFile - munmap of " + fileName() +
                               " failed: " + strerror(errno));
          }
      }
      itsPtr = NULL;
      itsMapSize = 0;
      itsMappedRegions.clear();
      itsTotalMapSize = 0;
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
        // Remap if required section is not currently mapped
        if (itsPosition < itsMapOffset || itsPosition + size > itsMapOffset + itsMapSize) {
            // If past end-of-file, write the buffer to extend the file.
            if (itsPosition + size > itsFileSize) {
                itsFileSize = itsPosition + size;
                LargeFiledesIO::doSeek(itsPosition, ByteIO::Begin);
                LargeFiledesIO::write(size, buf);
            }
            mapFile(itsPosition, size);
        }
        memcpy(itsPtr + itsPosition - itsMapOffset, buf, size);
        itsPosition += size;
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
