//# <MirExceptions.h>: this defines exception classes associated with filling errors
//# Copyright (C) 2000,2001,2003
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
//#
//# $Id: MirExceptions.h,v 1.1 2009/09/03 18:54:47 pteuben Exp $

#ifndef BIMA_MIREXCEPTIONS_H
#define BIMA_MIREXCEPTIONS_H

#include <casa/Exceptions/Error.h>

#include <casa/namespace.h>
//# Forward Declarations

//# UnavailableMiriadDataSelectionError
// <summary>
// an exception indicating that a requested data selection from a Miriad
// dataset is not available.
// </summary>
// 
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <synopsis>
// This class can be thrown as an exception to indicate that a requested 
// data selection from a Miriad dataset is not available, usually because 
// the selection is out of the bounds of the dataset's contents.
// </synopsis>
//
// <example>
// <srcblock>
// if (win > maxwin || win <= 0) 
//     throw UnavailableMiriadDataSelectionError("No such window", win);
// }
// </srcblock>
// </example>
//
// <motivation>
// Users can potentially make illegal data selection requests.
// </motivation>
//
class UnavailableMiriadDataSelectionError : public AipsError { 
public:

    // An exception with a message.  win usually is a window number.
    UnavailableMiriadDataSelectionError(const String& msg, Int win=0);

    // An exception with a message.  sel is the data selection value (e.g. 
    // requested source name).  
    UnavailableMiriadDataSelectionError(const String& msg, const String& sel);

    virtual ~UnavailableMiriadDataSelectionError() throw();

protected:
    // Message-less exceptions should not be created excepted possibly by 
    // subclasses
    UnavailableMiriadDataSelectionError();

    // format a message from a string and the illegal integer selection.
    // This class is used as a convenience for the instantiating the 
    // parent class, AipsError, which can only take a simple String.
    static String makeMsg(const String& msg, Int selval=0);
};

//# MiriadFormatError
// <summary>
// an exception indicating that a non-recoverable format error was detected
// in the input Miriad dataset
// </summary>
// 
// <use visibility=export>
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <synopsis>
// This exception can be thrown when a Miriad dataset contains some unexpected 
// and/or illegal structures.  This may happen if the dataset is corrupted or 
// otherwise does not conform to the legal format specification.  
// </synopsis>
//
// <example>
// <srcblock>
// uvprobvr_c(mirds_handle, "sfreq", vtype, &vlen, &vupd);
// if (vlen != nspect) {
//     throw MiriadFormatError(String("Wrong number of values for variable ") +
//                             "sfreq: got " + vlen + "; expected " + nspect);
// }
// </srcblock>
// </example>
//
// <motivation>
// This class handles the possibility that a non-conforming or corrupted 
// Miriad dataset is encountered.
// </motivation>
//
class MiriadFormatError : public AipsError { 
public:
    // create the exception with a message describing the format error
    MiriadFormatError(const String &msg);

    virtual ~MiriadFormatError() throw();

protected:
    // Message-less exceptions should not be created excepted possibly by 
    // subclasses
    MiriadFormatError();
};


//# MiriadUnsupportedFeatureError
// <summary>
// an exception indicating that further processing is dependent on an 
// unsupported feature of a Miriad dataset
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <etymology>
// Not all features of a Miriad dataset are necessarily supported by the filler
// classes.
// </etymology>
//
// <synopsis>
// This exception can be thrown during the processing of a Miriad dataset when 
// an unsupported feature is encountered.  For example, early versions of the 
// Miriad filler do not support multiple polarizations, although future 
// support is expected.  When multiple polarizations are found in the dataset, 
// this class is thrown as an exception.
// </synopsis>
//
// <example>
// <srcblock>
// if (! supported(feature)) {
//     throw MiriadUnsupportedFeatureError(String("Feature ") + feature.name()
//                                         + " not yet supported");
// }
// </srcblock>
// </example>
//
// <motivation>
// This class allows features of a Miriad dataset to be supported incrementally
// with new versions of the filler classes.
// </motivation>
//
class MiriadUnsupportedFeatureError : public AipsError { 
public:

    // An exception with a message indicating the unsupported feature
    MiriadUnsupportedFeatureError(const String &msg);

    virtual ~MiriadUnsupportedFeatureError() throw();

protected:
    // Message-less exceptions should not be created excepted possibly by 
    // subclasses
    MiriadUnsupportedFeatureError();
};


#endif


