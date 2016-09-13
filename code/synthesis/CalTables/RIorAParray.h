//# RIorAParray.h: Definition for RI/AP on-demand converter
//# Copyright (C) 2012                                     
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
//# Correspondence concerning AIPS++ should be adressed as follows:                           
//#        Internet email: aips2-request@nrao.edu.                                            
//#        Postal address: AIPS++ Project Office                                              
//#                        National Radio Astronomy Observatory                               
//#                        520 Edgemont Road                                                  
//#                        Charlottesville, VA 22903-2475 USA                                 
//#                                                                                           
//#                                                                                           

#ifndef CALTABLES_RIORAPARRAY_H
#define CALTABLES_RIORAPARRAY_H

#include <casa/aips.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Arrays/Array.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>

namespace casa { //# NAMESPACE CASA - BEGIN                                                   


class RIorAPArray
{
public:
  // Construct empty
  RIorAPArray();

  // Construct from external Complex Array
  RIorAPArray(const Array<Complex>& c);

  // Construct from external Float Array
  RIorAPArray(const Array<Float>& f);

  // Destructor
  ~RIorAPArray();

  // Set data
  void setData(const Array<Complex>& c);
  void setData(const Array<Float>& f);

  // State
  void state(Bool verbose=False);

  // Render Complex version (calc from Float, if necessary)
  Array<Complex> c();

  // Render Float version (calc from Complex, if necessary)
  Array<Float> f(Bool trackphase);

private:

  // Resize internal target arrays
  void resizec_();
  void resizef_();

  // Perform the actual calculation
  void calc_c();
  void calc_f(Bool trackphase);

  // Unwind phase
  void trackPhase(Array<Float>& ph);

  // State of Complex and Float versions
  Bool c_ok_, f_ok_;

  Bool phaseTracked_;

  // Internal Arrays for Complex and Float versions
  Array<Complex> c_;
  Array<Float> f_;

};

} //# NAMESPACE CASA - END

#endif

