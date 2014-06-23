//# tSubImage.cc: Test program for class SubImage
//# Copyright (C) 1998,1999,2000,2001,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: tSubImage.cc 20567 2009-04-09 23:12:39Z gervandiepen $

#ifndef IMAGEANALYSIS_LOGFILE_H
#define IMAGEANALYSIS_LOGFILE_H

#include <casa/IO/FiledesIO.h>
#include <casa/namespace.h>
#include <casa/cppconfig.h>

namespace casa {

class LogFile {
    // <summary>
    // Class for operations on a log file
    // </summary>

    // <reviewed reviewer="" date="" tests="" demos="">
    // </reviewed>

    // <prerequisite>
    // </prerequisite>

    // <etymology>
    // Image tasking
    // </etymology>

    // <synopsis>
    // Class for operations on a log file
    // </synopsis>

	// <todo>
	// Reimplement in terms of fstream

public:

	// The constructor does sanity checking such as that the file
	// is writable.
	LogFile(const String& filename);

    virtual ~LogFile();

    // Should file be appended to if it exists?
    void setAppend(const Bool a);

    // returns True if file successfully opened
    Bool open();

    void close() const;

    // Write output to file. If open is False, it is assumed there is apriori information
    // that the file is open and no such check will be done before writing. If close is True,
    // the file will be closed after writing the output, if True it will be left open (which
    // normally means the caller still have more writing to do).
    Bool write(
    	const String& output, const Bool open=True,
    	const Bool close=True
    );

private:

    String _filename;
    Bool _append;
    Int _fileHandle;
    auto_ptr<FiledesIO> _fileIO;

};
}

#endif
