//# BucketCache.cc: File buckets by means of file mapping
//# Copyright (C) 2009
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
//# $Id: BucketMapped.cc 20869 2010-03-19 08:31:45Z gervandiepen $

//# Includes
#include <casa/IO/BucketMapped.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>
#include <cstring>   //# for memset
#include <memory>

namespace casa { //# NAMESPACE CASA - BEGIN


  BucketMapped::BucketMapped (BucketFile* file, Int64 startOffset,
                              uInt bucketSize, uInt nrOfBuckets)
    : BucketBase (file, startOffset, bucketSize, nrOfBuckets)
  {
    AlwaysAssert (itsFile->mappedFile() != 0, AipsError);
  }

  BucketMapped::~BucketMapped()
  {}

  void BucketMapped::doResync()
  {
    ////itsFile->mappedFile()->resync();
  }

  void BucketMapped::doFlush()
  {
    itsFile->mappedFile()->flush();
  }

  void BucketMapped::doExtend (uInt)
  {
    // Extend the file.
    // Earlier versions of this function used to write only the
    // very last byte of the extended file. But that method was
    // found to severely degrade the performance of the following 
    // memory mapping on OS X 10.5 and OS X 10.6 (not on Linux).
    itsFile->mappedFile()->seek(itsStartOffset + 
				Int64(itsNewNrOfBuckets - 1)*itsBucketSize);
    
    uInt n = itsNewNrOfBuckets - itsCurNrOfBuckets;
    std::auto_ptr<char> ch(new char[itsBucketSize * n]());
    itsFile->mappedFile()->write(itsBucketSize, ch.get());
  }

  const char* BucketMapped::getBucket (uInt bucketNr)
  {
    if (bucketNr >= itsCurNrOfBuckets) {
      if (bucketNr >= itsNewNrOfBuckets) {
	throw (indexError<Int> (bucketNr));
      }
      initializeBuckets (bucketNr);
    }
    return static_cast<const char*>(itsFile->mappedFile()->getReadPointer
                                    (itsStartOffset + Int64(bucketNr)*itsBucketSize,
                                     itsBucketSize));
  }

  void BucketMapped::initializeBuckets (uInt bucketNr)
  {
    if (itsCurNrOfBuckets <= bucketNr) {
      doExtend (0);
      // Initialize this bucket and all uninitialized ones before it.
      while (itsCurNrOfBuckets <= bucketNr) {
        char* data = static_cast<char*>(itsFile->mappedFile()->getWritePointer
                                        (itsStartOffset + Int64(itsCurNrOfBuckets)*itsBucketSize,
                                         itsBucketSize));
        memset (data, 0, itsBucketSize);
        itsCurNrOfBuckets++;
        setWritten();
      }
    }
  }


} //# NAMESPACE CASA - END
