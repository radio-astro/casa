//# PlotMSParameters.h: Parameter classes for plotms.
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
#ifndef PLOTMSPARAMETERS_H_
#define PLOTMSPARAMETERS_H_

#include <plotms/PlotMS/PlotMSLogger.h>
#include <plotms/PlotMS/PlotMSWatchedParameters.h>

//#include <casa/namespace.h>

namespace casa {

// Subclass of PlotMSWatchedParameters that hold parameters for the whole
// plotter.  These parameters include:
// * log level.
class PlotMSParameters : public PlotMSWatchedParameters {
public:
    // Constructor.
    PlotMSParameters(PlotMSLogger::Level logLevel = PlotMSLogger::OFF,
            bool debug = false, bool clearSelection = true);
    
    // Copy constructor.  See operator=().
    PlotMSParameters(const PlotMSParameters& copy);
    
    // Destructor.
    ~PlotMSParameters();

    
    // Include overloaded methods.
    using PlotMSWatchedParameters::operator=;

    
    // Returns the current log level.
    PlotMSLogger::Level logLevel() const;
    
    // Returns the current log debug flag.
    bool logDebug() const;
    
    // Sets the current log level.  Note: this will notify any watchers unless
    // notifications are being held.
    void setLogLevel(PlotMSLogger::Level level, bool debug);
    
    // Gets/Sets whether any selections are cleared when plot axes are changed
    // or not.
    // <group>
    bool clearSelectionsOnAxesChange() const;
    void setClearSelectionsOnAxesChange(bool flag);
    // </group>
    
    
    // Implements PlotMSWatchedParameters::equals().  Will return false if the
    // other parameters are not of type PlotMSParameters.
    bool equals(const PlotMSWatchedParameters& other,
                            int updateFlags) const;
    
    // Copy operator.  See PlotMSWatchedParameters::operator=().
    PlotMSParameters& operator=(const PlotMSParameters& copy);
    
private:
    // Log level.
    PlotMSLogger::Level itsLogLevel_;
    
    // Log debug flag.
    bool itsLogDebug_;
    
    // Clear selections on axes change flag.
    bool itsClearSelectionsOnAxesChange_;
};

}

#endif /* PLOTMSPARAMETERS_H_ */
