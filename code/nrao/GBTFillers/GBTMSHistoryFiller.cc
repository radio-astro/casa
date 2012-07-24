//# GBTMSHistoryFiller.cc: GBTMSHistoryFiller fills the MSHistory table for GBT fillers
//# Copyright (C) 2000,2001
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
//# $Id$

//# Includes

#include <nrao/GBTFillers/GBTMSHistoryFiller.h>

#include <casa/Exceptions/Error.h>
#include <ms/MeasurementSets/MSHistory.h>
#include <ms/MeasurementSets/MSHistoryColumns.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Utilities/Assert.h>

GBTMSHistoryFiller::GBTMSHistoryFiller()
    : msHistory_p(0), msHistoryCols_p(0)
{;}


GBTMSHistoryFiller::GBTMSHistoryFiller(MSHistory &msHistory)
    : msHistory_p(0), msHistoryCols_p(0)
{init(msHistory);}

GBTMSHistoryFiller::~GBTMSHistoryFiller()
{
    delete msHistory_p;
    msHistory_p = 0;

    delete msHistoryCols_p;
    msHistoryCols_p = 0;
}

void GBTMSHistoryFiller::attach(MSHistory &msHistory)
{init(msHistory);}

void GBTMSHistoryFiller::fill(Int observationId, const String &message, 
			      const String &priority, const String &origin,
			      const MVTime &timestamp)
{
    Int rownr = msHistory_p->nrow();
    msHistory_p->addRow();
    msHistoryCols_p->observationId().put(rownr, observationId);
    msHistoryCols_p->time().put(rownr, timestamp.second());
    msHistoryCols_p->message().put(rownr, message);
    msHistoryCols_p->priority().put(rownr, priority);
    msHistoryCols_p->origin().put(rownr, origin);
    msHistoryCols_p->application().put(rownr, "gbtmsfiller");
    // there is no agreed upon format for the contents of these, just
    // leave them empty for now
    msHistoryCols_p->objectId().put(rownr,-1);
    msHistoryCols_p->cliCommand().put(rownr,Vector<String>(1,""));
    msHistoryCols_p->appParams().put(rownr,Vector<String>(1,""));
}

void GBTMSHistoryFiller::init(MSHistory &msHistory)
{
    msHistory_p = new MSHistory(msHistory);
    AlwaysAssert(msHistory_p, AipsError);

    msHistoryCols_p = new MSHistoryColumns(msHistory);
    AlwaysAssert(msHistoryCols_p, AipsError);
}
