//# PlotMSConstants.cc: Constants and useful methods for plotms.
//# Copyright (C) 2008
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
//#
//# $Id:  $
#include <msvis/PlotMS/PlotMSConstants.h>

#include <ctype.h>

namespace casa {

/////////////////////
// PMS DEFINITIONS //
/////////////////////

PlotAxisScale PMS::axisScale(Axis axis) {
    switch(axis) {
    
    case TIME: return DATE_MJ_SEC;
    
    default: return NORMAL;
    }
}

bool PMS::axisIsData(Axis axis) {
    switch(axis) {
    case AMP: case PHASE: case REAL: case IMAG: return true;
    default: return false;
    }
}

bool PMS::strEq(const String& str1, const String& str2, bool ignoreCase) {
    if(str1.size() != str2.size()) return false;
    if(!ignoreCase) return str1 == str2;
    for(unsigned int i = 0; i < str1.size(); i++)
        if(tolower(str1[i]) != tolower(str2[i])) return false;
    
    return true;
}

}
