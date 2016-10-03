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
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MDoppler.h>

namespace casa {

// Specifies averaging parameters for an MS.
class PlotMSTransformations {
public:
  // Static //
  
  // Enum and methods to define the different fields for an casacore::MS averaging.
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
  void fromRecord(const casacore::RecordInterface& record);
  casacore::Record toRecord() const;
  // </group>


  // If any transformations are specified, return true
  casacore::Bool anyTransform() const { return ((frameStr()!="") ||
				(veldefStr()!="RADIO") ||
				(xpcOffset()!=0.0) ||
				(ypcOffset()!=0.0) ||
				(formStokes()) ); };
  
  // Convenience methods for returning the standard field values.
  // <group>
  casacore::MFrequency::Types frame() { return mfreqType_; };
  casacore::MDoppler::Types veldef()  { return mdoppType_; };
  casacore::String frameStr() const   { return (mfreqType_==casacore::MFrequency::N_Types ? 
				      "" : casacore::MFrequency::showType(mfreqType_)); };
  casacore::String veldefStr() const  { return casacore::MDoppler::showType(mdoppType_); };
  casacore::Double restFreq() const   { return restFreq_; };  // In MHz
  casacore::Double restFreqHz() const   { return restFreq_*1.0e6; };
  casacore::Double xpcOffset() const  { return XpcOffset_; };
  casacore::Double ypcOffset() const  { return YpcOffset_; };
  casacore::Bool formStokes() const   { return formStokes_; };

  // </group>
  
  
  // Convenience methods for setting the standard field values.
  // <group>
  void setFrame(casacore::MFrequency::Types type) { mfreqType_=type; };
  void setFrame(const casacore::String& typeStr)  { 
    if (typeStr=="") mfreqType_=casacore::MFrequency::N_Types;
    else casacore::MFrequency::getType(mfreqType_,typeStr); };
  void setVelDef(casacore::MDoppler::Types type)  { mdoppType_=type; };
  void setVelDef(const casacore::String& typeStr) { casacore::MDoppler::getType(mdoppType_,typeStr); };
  void setRestFreq(casacore::Double restfreq)     { restFreq_ = restfreq; };  // in MHz
  void setRestFreq(casacore::Quantity restfreq)   { restFreq_ = restfreq.getValue("MHz"); };
  void setXpcOffset(casacore::Double dx)          { XpcOffset_ = dx; };
  void setYpcOffset(casacore::Double dy)          { YpcOffset_ = dy; };
  void setFormStokes(casacore::Bool formstokes)   { formStokes_ = formstokes; };
  // </group>
  
  
  // Equality operators.
  // <group>
  bool operator==(const PlotMSTransformations& other) const;
  bool operator!=(const PlotMSTransformations& other) const {
    return !(operator==(other)); }
  // </group>

  // Print out a summary
  casacore::String summary() const;
  
private:

  // The Frequency frame
  casacore::MFrequency::Types mfreqType_;
  
  // The Velocity Defn
  casacore::MDoppler::Types mdoppType_;
  
  // The rest frequency (MHz)
  casacore::Double restFreq_;

  // Phase center offsets
  casacore::Double XpcOffset_, YpcOffset_;

  // Form casacore::Stokes from correlations
  casacore::Bool formStokes_;

  // Sets the default values.
  void setDefaults();
    
};

}

#endif /* PLOTMSTRANSFORMATIONS_H_ */
