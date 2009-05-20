//# PlotMSPlotParameters.h: Parameter classes for PlotMSPlot classes.
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
#ifndef PLOTMSPLOTPARAMETERS_H_
#define PLOTMSPLOTPARAMETERS_H_

#include <plotms/PlotMS/PlotMSAveraging.h>
#include <plotms/PlotMS/PlotMSSelection.h>
#include <plotms/PlotMS/PlotMSWatchedParameters.h>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations.
class PlotMS;


// Parameters for a PlotMSPlot.  Includes the following parameters:
// * MS filename, and
// * MS selection.
class PlotMSPlotParameters : public PlotMSWatchedParameters {
public:
    // Constructor which sets default values and the optional MS filename.
    PlotMSPlotParameters(const String& filename = "");
    
    // Copy constructor.  See operator=().
    PlotMSPlotParameters(const PlotMSPlotParameters& copy);
    
    // Destructor.
    virtual ~PlotMSPlotParameters();
    
    
    // Include overloaded methods.
    using PlotMSWatchedParameters::operator=;
    
    
    // Implements PlotMSWatchedParameters::equals().  Will return false if the
    // other parameters are not of type PlotMSPlotParameters.  Checks MS
    // filename and selection.
    virtual bool equals(const PlotMSWatchedParameters& other,
                        int updateFlags) const;
    
    
    // Indicates whether or not parameters have been set or not.  Unset
    // parameters should lead to blank plot canvases.  Parameters are unset if
    // the filename is empty.
    virtual bool isSet() const;
    
    // Gets/Sets MS filename.
    // <group>
    virtual const String& filename() const;
    virtual void setFilename(const String& filename);
    // </group>
    
    // Gets/Sets MS selection.
    // <group>
    virtual const PlotMSSelection& selection() const;
    virtual void setSelection(const PlotMSSelection& sel);
    // </group>
    
    // Gets/Sets MS averaging.
    // <group>
    virtual const PlotMSAveraging& averaging() const;
    virtual void setAveraging(const PlotMSAveraging& avg);
    // </group>
    
    // Copy operator.  See PlotMSWatchedParameters::operator=().
    virtual PlotMSPlotParameters& operator=(const PlotMSPlotParameters& copy);
    
protected:
    // MS filename.
    String itsMSFilename_;
    
    // MS selection.
    PlotMSSelection itsMSSelection_;
    
    // MS averaging.
    PlotMSAveraging itsMSAveraging_;
    
    // Flag for avoiding multiple calls to PlotMSWatchedParameters::updateFlag.
    bool updateFlag_;
};

}

#endif /* PLOTMSPLOTPARAMETERS_H_ */
