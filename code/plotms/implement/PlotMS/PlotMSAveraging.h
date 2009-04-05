//# PlotMSAveraging.h: Averaging parameters.
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
#ifndef PLOTMSAVERAGING_H_
#define PLOTMSAVERAGING_H_

#include <casa/Containers/Record.h>

#include <casa/namespace.h>

namespace casa {

// Specifies averaging parameters for an MS.
class PlotMSAveraging {
public:
    // Constructor, which uses default values.
    PlotMSAveraging();
    
    // Destructor.
    ~PlotMSAveraging();
    
    
    // Converts this object to/from a record.  The record keys/values currently
    // are:
    // { "channel" => bool, "channelValue" => double,
    //   "time" => bool, "timeValue" => double,
    //   "scan" => bool, "field" => bool, "baseline" => bool }
    void fromRecord(const RecordInterface& record);
    Record toRecord() const;
    // </group>
    
    // Gets/Sets whether channel averaging is turned on or not.
    // <group>
    bool channel() const;
    void setChannel(bool channel);
    // </group>
    
    // Gets/Sets the channel averaging value.
    // <group>
    double channelValue() const;
    void setChannelValue(double value);
    // </group>
    
    // Gets/Sets whether time averaging is turned on or not.
    // <group>
    bool time() const;
    void setTime(bool time);
    // </group>
    
    // Gets/Sets the time averaging value.
    // <group>
    double timeValue() const;
    void setTimeValue(double value);
    // </group>
    
    // Gets/Sets whether scan averaging is turned on or not.
    // <group>
    bool scan() const;
    void setScan(bool scan);
    // </group>
    
    // Gets/Sets whether field averaging is turned on or not.
    // <group>
    bool field() const;
    void setField(bool field);
    // </group>
    
    // Gets/Sets whether baseline averaging is turned on or not.
    // <group>
    bool baseline() const;
    void setBaseline(bool baseline);
    // </group>
    
    
    // Equality operators.
    // <group>
    bool operator==(const PlotMSAveraging& other) const;
    bool operator!=(const PlotMSAveraging& other) const {
        return !(operator==(other)); }
    // </group>
    
private:
    // Channel averaging flag and value, respectively.
    // <group>
    bool itsChannel_;
    double itsChannelValue_;
    // </group>
    
    // Time averaging flag and value, respectively.
    // <group>
    bool itsTime_;
    double itsTimeValue_;
    // </group>
    
    // Scan averaging flag.
    bool itsScan_;
    
    // Field averaging flag.
    bool itsField_;
    
    // Baseline averaging flag.
    bool itsBaseline_;
    
    
    // Sets the default values.
    void setDefaults();
};

}

#endif /* PLOTMSAVERAGING_H_ */
