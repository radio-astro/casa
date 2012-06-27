//# GBTMSObservationFiller.cc: GBTMSObservationFiller fills the MSObservation table for GBT fillers
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

#include <nrao/GBTFillers/GBTMSObservationFiller.h>

#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <ms/MeasurementSets/MSObservation.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Utilities/Assert.h>

GBTMSObservationFiller::GBTMSObservationFiller()
    : msObservation_p(0), msObsCols_p(0), observationId_p(-1), project_p(""),
      observer_p(""), telescope_p("")
{;}


GBTMSObservationFiller::GBTMSObservationFiller(MSObservation &msObservation)
    : msObservation_p(0), msObsCols_p(0), observationId_p(-1), project_p(""),
      observer_p(""), telescope_p("")
{init(msObservation);}

GBTMSObservationFiller::~GBTMSObservationFiller()
{
    delete msObservation_p;
    msObservation_p = 0;

    delete msObsCols_p;
    msObsCols_p = 0;
}

void GBTMSObservationFiller::attach(MSObservation &msObservation)
{init(msObservation);}

void GBTMSObservationFiller::fill(const String &project, const String &observer,
				  const MVTime &timestamp, Double duration,
				  const String &telescope)
{
    if (observationId_p < 0 || project_p != project ||
	observer_p != observer || telescope_p != telescope) {
	// we need a new row
	observationId_p = msObservation_p->nrow();
	msObservation_p->addRow();
	msObsCols_p->telescopeName().put(observationId_p, telescope);
	Vector<Double> timeRange(2);
	timeRange(0) = timestamp.second();
	timeRange(1) = timeRange(0) + duration;
	msObsCols_p->timeRange().put(observationId_p, timeRange);
	msObsCols_p->observer().put(observationId_p,observer);
	Vector<String> emptyString(1);
	emptyString = "";
	msObsCols_p->log().put(observationId_p, emptyString);
	msObsCols_p->scheduleType().put(observationId_p, "");
	msObsCols_p->schedule().put(observationId_p, emptyString);
	msObsCols_p->project().put(observationId_p, project); 
	msObsCols_p->releaseDate().put(observationId_p, 0.0);
	msObsCols_p->flagRow().put(observationId_p, False);
	project_p = project;
	observer_p = observer;
	telescope_p = telescope;
    } else {
	// reuse the existing row, but update the time range
	Vector<Double> timeRange = msObsCols_p->timeRange()(observationId_p);
	timeRange(1) = timestamp.second() + duration;
	msObsCols_p->timeRange().put(observationId_p, timeRange);
    }
}

void GBTMSObservationFiller::init(MSObservation &msObservation)
{
    msObservation_p = new MSObservation(msObservation);
    AlwaysAssert(msObservation_p, AipsError);

    msObsCols_p = new MSObservationColumns(msObservation);
    AlwaysAssert(msObsCols_p, AipsError);

    // set the observationId to the last row filled, if there are any
    observationId_p = msObservation_p->nrow()-1;
    if (observationId_p < 0) {
	observationId_p = -1;
	project_p = "";
	observer_p = "";
	telescope_p = "";
    } else {
	project_p = msObsCols_p->project()(observationId_p);
	observer_p = msObsCols_p->observer()(observationId_p);
	telescope_p = msObsCols_p->telescopeName()(observationId_p);
    }
}
