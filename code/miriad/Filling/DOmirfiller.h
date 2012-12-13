//# DOmirfiller.h: a DO for filling MIRIAD uv data into an MS
//# Copyright (C) 2000,2001,2002
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
//# $Id: DOmirfiller.h,v 1.2 2009/09/03 23:28:32 pteuben Exp $

#ifndef BIMA_DOMIRFILLER_H
#define BIMA_DOMIRFILLER_H

#include <miriad/Filling/MirFiller.h>
#include <miriad/Filling/MirExceptions.h>
#include <tasking/Tasking/ApplicationObject.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>
#include <tasking/Glish/GlishRecord.h>

#include <casa/namespace.h>
//# Forward Declarations

// <summary>
// A Distributed Object for filling MIRIAD uv data into an MS
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class="MirFiller">MirFiller</linkto>
//   <li> the AIPS++ DO/Tasking system
// </prerequisite>
//
// <etymology>
// DO means it's a distributed object; mirfiller means its a filler of 
// miriad data
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//
// <thrown>
//    <li>
//    <li>
// </thrown>
//
// <todo asof="yyyy/mm/dd">
//   <li> 
//   <li> 
//   <li> 
// </todo>

class mirfiller: public ApplicationObject {
private:
    mirfiller();
    mirfiller(const mirfiller& other);

    MirFiller filler_p;
public:
    // create the filler, attaching it to a Miriad dataset.  <src>mirfile</src>
    // is the dataset filename
    mirfiller(const String& mirfile);

    // destruct this filler
    ~mirfiller();

    // fill the output MS according to the current selections and options.
    // msfile is the output name to use; it should not already exist.
    void fill(const String& msfile) { filler_p.fill(msfile); }

    // select spectra from the input Miriad dataset.  wideChans is a list of 
    // one-based channel indices.  narrowWins is a list of one-based
    // window indices.  The default is to choose all available wide channels
    // and windows.
    void selectspectra(const Vector<Int> &wideChans, 
                       const Vector<Int> &narrowWins) 
	throw(UnavailableMiriadDataSelectionError) 
    { 
        filler_p.selectSpectra(wideChans, narrowWins); 
    }

    // select polarization setups from the input Miriad dataset.  
    void selectpols(const Vector<Int> pols);

    // return a description of the input Miriad dataset's contents as a Record
    // object.  If scan is True, the dataset will be implicitly scanned in 
    // its entirety to extract all the needed information; otherwise, just the
    // first timestep will be read.
    GlishRecord summary(Bool verbose=True, Bool scan=True);

    // set the options for filling.  The options are set via a Record object
    // for which the following keys are recognized:
    //
    void setoptions(const Record &opts) { filler_p.setOptions(opts); }

    // get the current filling options
    Record getoptions() { return filler_p.getOptions(); }

    virtual String className() const;
    virtual Vector<String> methods() const;
    virtual MethodResult runMethod(uInt which,     
                                   ParameterSet &parameters,
                                   Bool runMethod);
    virtual Vector<String> noTraceMethods() const;
};

#endif


