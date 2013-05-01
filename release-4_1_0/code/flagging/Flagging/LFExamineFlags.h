//# LFDisplayFlags: A lighter flagger - for autoflag
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
#ifndef FLAGGING_LFEXAMINEFLAGS_H
#define FLAGGING_LFEXAMINEFLAGS_H

#include <flagging/Flagging/LFBase.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  
  class LFExamineFlags : public LFBase
  {
  public:  
    // default constructor 
    LFExamineFlags  ();

    // default destructor
    virtual ~LFExamineFlags ();

    // Return method name
    virtual String methodName(){return String("examineflags");};

    // Set autoflag params
    virtual Bool setParameters(Record &parameters);
    
    // Get default autoflag params
    virtual Record getParameters();

    // Run the algorithm
    virtual Bool runMethod(const VisBuffer &inVb, 
                   Cube<Float> &inVisc, Cube<Bool> &inFlagc, Cube<Bool> &inPreFlagc,
			   uInt numT, uInt numAnt, uInt numB, uInt numC, uInt numP);

    virtual Record getStatistics();
    
  protected:

    void ReadMSInfo(const VisBuffer &vb);

    void AccumulateStats(const VisBuffer &vb);

    // Additional private members
    Record allflagcounts;
    Vector<String> antnames_p;
    Vector<String> corrlist_p;
    Vector<String> fieldnames_p;
    //Vector<xxx> spwlist_p;
    Vector<Double> freqlist_p;


    // Counters per chunk
    Vector<Float> chan_count, baseline_count, corr_count;
    Vector<Float> chan_flags, baseline_flags, corr_flags;
    Float chunk_count, chunk_flags;

    // Counters across chunks (can be different shapes
    // Statistics per antenna, baseline, spw, etc.
    // These maps of maps is used e.g. like:
    //
    //        accumflags["baseline"]["2&&7"] == 42
    //        accumflags["spw"     ]["0"   ] == 17
    //
    // which means that there were 42 flags on baseline 2 - 7, etc.
    std::map<std::string, std::map<std::string, float> > allflags;
    std::map<std::string, std::map<std::string, float> > allcounts;

    uInt a1,a2;

  };
  
  
} //# NAMESPACE CASA - END

#endif

