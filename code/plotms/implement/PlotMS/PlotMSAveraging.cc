//# PlotMSAveraging.cc: Averaging parameters.
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
//# $Id: $
#include <plotms/PlotMS/PlotMSAveraging.h>

namespace casa {

/////////////////////////////////
// PLOTMSAVERAGING DEFINITIONS //
/////////////////////////////////

PlotMSAveraging::PlotMSAveraging() { setDefaults(); }
PlotMSAveraging::~PlotMSAveraging() { }


bool PlotMSAveraging::channel() const { return itsChannel_; }
void PlotMSAveraging::setChannel(bool channel) { itsChannel_ = channel; }

double PlotMSAveraging::channelValue() const { return itsChannelValue_; }
void PlotMSAveraging::setChannelValue(double value) {
    itsChannelValue_ = value; }

bool PlotMSAveraging::time() const { return itsTime_; }
void PlotMSAveraging::setTime(bool time) { itsTime_ = time; }

double PlotMSAveraging::timeValue() const { return itsTimeValue_; }
void PlotMSAveraging::setTimeValue(double value) { itsTimeValue_ = value; }

bool PlotMSAveraging::scan() const { return itsScan_; }
void PlotMSAveraging::setScan(bool scan) { itsScan_ = scan; }

bool PlotMSAveraging::field() const { return itsField_; }
void PlotMSAveraging::setField(bool field) { itsField_ = field; }

bool PlotMSAveraging::baseline() const { return itsBaseline_; }
void PlotMSAveraging::setBaseline(bool baseline) { itsBaseline_ = baseline; }


bool PlotMSAveraging::operator==(const PlotMSAveraging& other) const {
    return itsChannel_ == other.itsChannel_ &&
           (!itsChannel_ || itsChannelValue_ == other.itsChannelValue_) &&
           itsTime_ == other.itsTime_ &&
           (!itsTime_ || itsTimeValue_ == other.itsTimeValue_) &&
           itsScan_ == other.itsScan_ && itsField_ == other.itsField_ &&
           itsBaseline_ == other.itsBaseline_;
}


void PlotMSAveraging::setDefaults() {
    itsChannel_ = itsTime_ = itsScan_ = itsField_ = itsBaseline_ = false;
    itsChannelValue_ = itsTimeValue_ = 0;
}

}
