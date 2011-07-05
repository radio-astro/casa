//# LFExtendFlags: A lighter flagger - for autoflag
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
//# $Jan 28 2011 rurvashi Id$
#ifndef FLAGGING_LFEXTENDFLAGS_H
#define FLAGGING_LFEXTENDFLAGS_H

#include <flagging/Flagging/LFExamineFlags.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  
  class LFExtendFlags : public LFExamineFlags
  {
  public:  
    // default constructor 
    LFExtendFlags  ();
    // default destructor
    ~LFExtendFlags ();

    // Return method name
    String methodName(){return String("extendflags");};

    // Set autoflag params
    Bool setParameters(Record &parameters);
    
    // Get default autoflag params
    Record getParameters();

    // Run the algorithm
    Bool runMethod(const VisBuffer &inVb, Cube<Float> &inVisc, Cube<Bool> &inFlagc, Cube<Bool> &inPreFlagc, 
		   uInt numT, uInt numAnt, uInt numB, uInt numC, uInt numP);    
    
  private:

    void GrowFlags();
    void FlagBaselinesFromAntennas();
    void ExtendFlagsAcrossPols();
    void FlagAntennasFromBaselines();
    //    void ExtendAcrossBaselines();
    //    void MakeFlagCmds();

    Int flaglevel_p;

    Bool flagcrossfromauto_p;
    Bool flagbothants_p;

    Bool extendacrosspols_p;

    Float grow_in_time_p;
    Float grow_in_freq_p;
    Bool grow_around_p;

    Bool flag_prev_next_time_p;
    Bool flag_prev_next_freq_p;

    Vector<Int> ant1,ant2;
    
  };
  
  
} //# NAMESPACE CASA - END

#endif

