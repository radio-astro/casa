//# DataLoadingBuf.h: this defines a container for filler data buffers
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
//# $Id: DataLoadingBuf.h,v 1.1 2009/09/03 18:54:47 pteuben Exp $

#ifndef BIMA_DATALOADINGBUF_H
#define BIMA_DATALOADINGBUF_H

#include <casa/Containers/Block.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Vector.h>

#include <casa/namespace.h>
//# Forward Declarations

// <summary>
// a container for data buffers used to fill a measurement set
// </summary>
// 
// <use visibility=export>
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <etymology>
// class holds buffers used to load data into a Measurment Set 
// </etymology>
//
// <motivation>
// Miriad data support multiple spectral windows for a given correllator
// setup.  Furthermore, a dataset can contain multiple setups.  This class
// attempts to avoid having to repeatedly create a buffer inside a loading
// loop.
// </motivation>
//
// <synopsis>
// This class allows one to create reusable buffers for each of the spectral 
// windows that can be passed to an 
// <linkto class="ArrayColumn">ArrayColumn's</linkto>
// <src>put()</src> function.  This avoids the cost of recreating the buffers
// inside a loading loop that cycles through the input Miriad windows.  The 
// buffers can be resized if necessary if the Miriad correllator setup 
// changes.  <p>
//
// This class stores for each window a Matrix<Complex> to store the visibility data
// and a Cube<Bool> to hold the flags.  Normally, you can have all wide-band windows 
// share a single buffer
// (since they are all the same size, namely having 1 channel).  The buffers are 
// access via their (zero-base) window index, starting with the narrow band windows.  
// A window index greater than or equal to the number of narrow band windows (set 
// via <src>resizeForNspec()</src>) is assumed to refer a wide-band window and the 
// last buffer is returned.  
//
// If the size of a buffer needs to be changed (e.g. because the correllator setup
// changed such that the number of channels in a window subsequently changed), it 
// is most convenient to do the resizing via <src>resizeForSpWin()</src>.
// </synopsis>
//
// <example>
// To set up the buffers, you need to know how many windows you have, and the 
// number of channels in each window.
// <srcblock>
//     Int nspec = 6
//     Int *nchan = { 512, 256, 256, 512, 256, 256 };
//     DataLoadingBuf buf();
//     buf.resizeForNspect(nspec+1)  // to hold both narrow and wide band data
//     for(Int i=0; i < nspec; i++)
//         buf.resizeForSpWin(i, nchan[i]);
//     buf.resizeForSpWin(nspec, 1);
// </srcblock>
//
// Now to use the buffers...
// <srcblock>
//     for(Int i=0; i < 2*nspec; i++) {
//
//         // get data-loading buffers
//         Matrix<Complex> &vis = buf.visForSpWin(i);
//         Cube<Bool> &flgs = buf.flagsForSpWin(i);
// 
//         // load in the data
//         Int n = (i < nspec) ? nchan[i] : 1;
//         for(Int j=0; j < n; j++) {
//             vis(0,j) = Complex( corr[2*(j+offset)], 
//                                -corr[2*(j+offset)+1] );
//             flgs(0,j,1) = (flags[j+offset] == 0);
//         }
//     
//         // load it into MS column
//         msc->data().put(row, vis);
//         msc->flagCategory().put(row, flgs);
//     }
// </srcblock>
// </example>
//
// <todo asof="2001/02/22">
//   <li> the most important functions in this class are inlined even though 
//        they are a tad long (4-6 lines).  This is because it is expected 
//        that they are called once within a loop.  Under this assumption, 
//        the benefit probably out-weighs the cost.  If this assumption 
//        changes, this code should probably be moved out of line.
// </todo>
//
class DataLoadingBuf {
private:
    Block< Matrix<Complex>* > vislist;
    Block< Cube<Bool>* > flgslist;
    Vector<Float> wt;
    Vector<Float> rms;

    DataLoadingBuf(DataLoadingBuf&);
public:

    // create the container
    DataLoadingBuf();

    // delete the container
    ~DataLoadingBuf();

    // resize our arrays of containers to hold data for wide band data 
    // and a given number of narrow band spectral windows.  <src>i</src>
    // is the number of narrow band spectral windows.  
    void resizeForNspect(Int i);

    // return the number of channels that can be stored in a given window.
    // A value for <src>i</src> greater than or equal to the number of 
    // narrow band windows is assumed to refer to a wideband window; thus, 
    // 1 will be returned.
    Int nchanForSpWin(Int i) { return visForSpWin(i).ncolumn(); }

    // return a reference to the flags Cube for a given window.
    // A value for <src>i</src> greater than or equal to the number of 
    // narrow band windows is assumed to refer to a wideband window.
    Cube<Bool>& flagsForSpWin(Int winid) {
        if ((uInt)winid >= flgslist.nelements()) 
            winid = flgslist.nelements()-1;
        if (flgslist[winid] == NULL) flgslist[winid] = new Cube<Bool>();
        return *(flgslist[winid]);
    }

    // return a reference to the vis Matrix for a given window
    Matrix<Complex>& visForSpWin(Int winid) {
        if ((uInt)winid >= vislist.nelements()) winid = vislist.nelements()-1;
        if (vislist[winid] == NULL) vislist[winid] = new Matrix<Complex>();
        return *(vislist[winid]);
    }

    // return a reference to the weight vector
    Vector<Float>& weight() { return wt; }

    // return a reference to the sigma vector
    Vector<Float>& sigma() { return rms; }

    // resize the containers for a given window to the given number of channels
    void resizeForSpWin(Int winid, Int nchan) {
	Int npol;

	if (winid > Int(vislist.nelements())) winid = vislist.nelements();
        if (vislist[winid] == NULL ||
            visForSpWin(winid).ncolumn() != (uInt)nchan) 
        {
	    npol = (vislist[winid] == NULL) ? 1 : vislist[winid]->nrow();
            if (vislist[winid] != NULL) delete vislist[winid];
            vislist[winid] = new Matrix<Complex>(npol, nchan);
            if (flgslist[winid] != NULL) delete flgslist[winid];
            flgslist[winid] = new Cube<Bool>(npol, nchan, 2, False);
        }
    }

    // resize the containers for a given number of polarization correlations
    void resizeForNpol(Int npol) {
	for(Int i=0; i < Int(vislist.nelements()); i++) {
	    if (vislist[i] != NULL) {
		Int nchan = vislist[i]->ncolumn();
		vislist[i]->resize(npol, nchan);
		flgslist[i]->resize(npol, nchan, 2);
		*(flgslist[i]) = False;
	    }
	}
	wt.resize(npol);
	rms.resize(npol);
    }

    
};

#endif


