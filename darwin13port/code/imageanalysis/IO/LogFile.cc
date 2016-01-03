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
//# $Id: $

#include <imageanalysis/IO/LogFile.h>

#include <casa/Exceptions/Error.h>
#include <casa/OS/File.h>
#include <imageanalysis/IO/OutputDestinationChecker.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace casa {

LogFile::LogFile(const String& filename) :
	_filename(filename), _append(False), _fileHandle(-1),
	_fileIO(0) {
	if (filename.empty()) {
		throw AipsError("LogFile::LogFile(): file name cannot be empty");
	}
	OutputDestinationChecker::OutputStruct logFile;
	logFile.label = "log file";
	logFile.outputFile = &_filename;
	logFile.required = False;
	logFile.replaceable = True;
	LogIO log;
	OutputDestinationChecker::checkOutput(logFile, log);
	if (_filename.empty()) {
		throw AipsError("Cannot create log file");
	}
}

LogFile::~LogFile() {}

Bool LogFile::openFile() {

	File log(_filename);
	switch (File::FileWriteStatus status = log.getWriteStatus()) {
	case File::OVERWRITABLE:
		if (_append) {
			_fileHandle = open(_filename.c_str(), O_RDWR | O_APPEND);
		}
		// no break here to fall through to the File::CREATABLE block if logFileAppend is false
	case File::CREATABLE:
		if (status == File::CREATABLE || ! _append) {
			// can fall throw from previous case block so status can be File::OVERWRITABLE
			String action = (status == File::OVERWRITABLE) ? "Overwrote" : "Created";
			_fileHandle = FiledesIO::create(_filename.c_str());
		}
		break;
	default:
		throw AipsError(
			"LogFile::_open(): Programming logic error. This block should never be reached"
		);
		break;
	}
	_fileIO.reset(new FiledesIO(_fileHandle, _filename.c_str()));
	return True;
}

void LogFile::closeFile() const {
	if (_fileHandle > 0) {
		FiledesIO::close(_fileHandle);
	}
}

Bool LogFile::write(
	const String& output, const Bool open, const Bool close
) {
	if (open) {
		if (! openFile()) {
			return False;
		}
	}
	_fileIO->write(output.length(), output.c_str());
	if (close) {
		closeFile();
	}
	return True;
}

void LogFile::setAppend(Bool a) { _append = a; }

}
