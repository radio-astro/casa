//# PlotMSAveraging.h: Transformation parameters.
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
#ifndef PLOTMSTRANSFORMATIONS_H_
#define PLOTMSTRANSFORMATIONS_H_

#include <plotms/PlotMS/PlotMSConstants.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MDoppler.h>

#include <casa/namespace.h>

namespace casa {

// Specifies averaging parameters for an MS.
class PlotMSTransformations {
public:
  // Static //
  
  // Enum and methods to define the different fields for an MS averaging.
  // <group>
  PMS_ENUM1(Field, fields, fieldStrings, field,
	    FRAME, VELDEF)
  PMS_ENUM2(Field, fields, fieldStrings, field,
	    "Frame", "veldef")
  // </group>
              
  // Non-Static //
    
  // Constructor, which uses default values.
  PlotMSTransformations();
    
  // Destructor.
  ~PlotMSTransformations();
    
    
  // Converts this object to/from a record.  Each field will have a key that
  // is its enum name
  // <group>
  void fromRecord(const RecordInterface& record);
  Record toRecord() const;
  // </group>
  
  // Convenience methods for returning the standard field values.
  // <group>
  MFrequency::Types frame() { return mfreqType_; };
  MDoppler::Types veldef()  { return mdoppType_; };
  String frameStr() const   { return (mfreqType_==MFrequency::N_Types ? 
				      "" : MFrequency::showType(mfreqType_)); };
  String veldefStr() const  { return MDoppler::showType(mdoppType_); };
  Double restFreq() const   { return restFreq_; };  // In MHz
  Double restFreqHz() const   { return restFreq_*1.0e6; };
  Double xpcOffset() const  { return XpcOffset_; };
  Double ypcOffset() const  { return YpcOffset_; };
  // </group>
  
  
  // Convenience methods for setting the standard field values.
  // <group>
  void setFrame(MFrequency::Types type) { mfreqType_=type; };
  void setFrame(const String& typeStr)  { 
    if (typeStr=="") mfreqType_=MFrequency::N_Types;
    else MFrequency::getType(mfreqType_,typeStr); };
  void setVelDef(MDoppler::Types type)  { mdoppType_=type; };
  void setVelDef(const String& typeStr) { MDoppler::getType(mdoppType_,typeStr); };
  void setRestFreq(Double restfreq)     { restFreq_ = restfreq; };  // in MHz
  void setXpcOffset(Double dx)          { XpcOffset_ = dx; };
  void setYpcOffset(Double dy)          { YpcOffset_ = dy; };
  // </group>
  
  
  // Equality operators.
  // <group>
  bool operator==(const PlotMSTransformations& other) const;
  bool operator!=(const PlotMSTransformations& other) const {
    return !(operator==(other)); }
  // </group>

  // Print out a summary
  String summary() const;
  
private:

  // The Frequency frame
  MFrequency::Types mfreqType_;
  
  // The Velocity Defn
  MDoppler::Types mdoppType_;
  
  // The rest frequency (MHz)
  Double restFreq_;

  // Phase center offsets
  Double XpcOffset_, YpcOffset_;

  // Sets the default values.
  void setDefaults();
    
};

}

#endif /* PLOTMSTRANSFORMATIONS_H_ */
