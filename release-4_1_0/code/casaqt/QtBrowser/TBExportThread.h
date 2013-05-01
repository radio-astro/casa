//# TBExportThread.h: Thread that exports a table to VOTable XML format.
//# Copyright (C) 2005
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
//# $Id: $
#ifndef TBEXPORTTHREAD_H_
#define TBEXPORTTHREAD_H_

#include <QtGui>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class TBTable;
class ProgressHelper;

// <summary>
// Thread that exports a table to VOTable XML format.
// <summary>
//
// <synopsis>
// TBExportThread is a subclass of QThread assigned with the task of exporting a
// table into VOTable XML format to a specified file.  This task can work in the
// background to allow the user to continue interacting with the table browser.
// </synopsis>

class TBExportThread : public QThread {
public:
    // Constructor that takes the table to export, the file to export to, and an
	// optional ProgressHelper to show the progress of the export.
    TBExportThread(TBTable* table, String file,
    			   ProgressHelper* progressPanel = NULL);

    ~TBExportThread();

    
    // Overrides QThread::run() which defines the task to be completed by the
    // thread.  See TBTable::exportVOTable().
    void run();

private:
    // Table to export.
    TBTable* table;

    // File to export to.
    String file;

    // (Optional) progress helper.
    ProgressHelper* progressPanel;
};

}

#endif /* TBEXPORTTHREAD_H_ */
