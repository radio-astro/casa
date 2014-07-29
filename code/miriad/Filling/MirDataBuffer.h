//# MirDataBuffer.h: this defines a container for filler data buffers
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
//#
//# $Id: MirDataBuffer.h,v 1.1 2009/09/03 18:54:47 pteuben Exp $

#ifndef BIMA_MIRDATABUFFER_H
#define BIMA_MIRDATABUFFER_H

#include <casa/Containers/Block.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>

#include <casa/namespace.h>
//# Forward Declarations
class MirDataBufferIter;

// <summary>
// a buffer for data read in from a Miriad datatset
// </summary>
// 
// <use visibility=local>
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <etymology>
// this class buffers data from a Miriad dataset
// </etymology>
//
// <motivation>
// MSes filled from Miriad data feature multiple windows of differing shapes.  
// Performance during MS reads can be aided by collecting together MS table 
// records of the same shape (e.g. of the same window).  MirFiller accoplishes
// this by reading all baselines from a single timestamp at once, so that 
// they written out in order of spectral window.  This class provides a 
// container for holding the data from different baselines.  
// </motivation>
//
// <synopsis>
// At the moment, this is intended for use only by the MirFiller class.
//
// 
// </synopsis>
//
// <example>
// 
// 
// </example>
//
// <todo asof="2001/02/22">
// 
// </todo>
//
class MirDataBuffer {
private:
    Boolean upd;
    Boolean hasmore;
    
    Block<int> pol;
    Block<double *> preamble;
    Block<float *> narrow;
    Block<float *> wide;
    Int nrec, nnar, nwide;

    MirDataBuffer(MirDataBuffer&);
public:
    // create the container
    MirDataBuffer(...);

    // return true if there is still data to be read from the dataset
    Boolean hasMore() { return hasmore; }

    // set the variable update state.  A value of True means that some
    // variables have changed and should be checked for new values.  False
    // means that the variables have not changed since the last time 
    // they were checked.
    void varUpdated(Boolean val) { upd = val; }

    // return the variable update state.  A value of True means that some
    // variables have changed and should be checked for new values.  False
    // means that the variables have not changed since the last time 
    // they were checked.
    Boolean isVarUpdated() { return upd; }

    // return a pointer to the i-th record in this buffer
    float *operator[](Int i) { return recs[i]; }

    // return the preamble for the i-th record
    double *getPreamble(Int i) { return preamble[i]; }

    // return the current Miriad timestamp
    double getTime() { return preamble[0][3]; }

    // return the baseline number for the i-th record
    Int getBaselineNum(Int i) { return Int(preamble[i][4]); }

    // return the baseline vector as a 3-element array holding u, v, & w
    double *getBaselineVec(Int i) { return preamble[i]; }

    // return the Miriad polarization code for the i-th record
    Int getPol(Int i) { return pol[i]; }

    // return the number of records held in this buffer
    Int size() { nrec; }

    // update the number of records held by the buffer
    
    // reset the buffer for a new time slot
    void reset() {
        if (hasmore) {

            // switch cached record into first position
            double *tmppre = preamble[0];
            float *tmpnar = narrow[0];
            float *tmpwid = wide[0];
            preamble[0] = preamble[n];
            narrow[0] = narrow[n];
            wide[0] = wide[n];
            pol[0] = pol[n];
            preamble[n] = tmppre;
            narrow[n] = tmpnar;
            wide[n] = tmpwid;

            hasmore = False;
            n=1;
        } else {
            n=0;
        }
    }



};

#endif
