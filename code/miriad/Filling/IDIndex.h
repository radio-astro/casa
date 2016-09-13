//# IDIndex.h: this defines IDIndex, which maps one indexing system into another
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
//# $Id: IDIndex.h,v 1.2 2009/09/03 23:28:32 pteuben Exp $

#ifndef BIMA_IDINDEX_H
#define BIMA_IDINDEX_H

#include <casa/Containers/SimOrdMap.h>

#include <casa/namespace.h>
// <summary>
// a simple mapping from one indexing system to another
// </summary>
// 
// <use visibility=export>
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <etymology>
// This class is puts an index on an ordered list of IDs.
// </etymology>
//
// <synopsis>
// Suppose that you have a list of things accessed via an index (i.e. an 
// integer ID), say spectral windows.  Suppose further you want to map them 
// into some other index system, perhaps because you are reordering them.  
// This simple class can keep track of the mapping between the two systems.  <p>
// 
// The mapping is set up through sequenced calls to the <src>add()</src> 
// function, to which indices from the "input" system are passed.  The order 
// that the indices are passed indicate their order in the output system.  
// One can then map to the output system by passing the input index to the 
// () operator.  <p>
//
// One sets the index that the first input index will be mapped to via the 
// constructor, allowing one to map, for example, {1, 3, 2} into {4, 5, 6}.  
// </synopsis>
//
// <example>
// Suppose only selected windows will be written out.  Here's how we can keep
// track of the mapping:
// <srcblock>
//   IDIndex idx(1);   // the first index mapped into is 1
//   for(Int i=0; i < nspect; i++) {
//       if (windowSelected(i)) idx.add(i)   // 0 -> 1 if selected
//   }
//   
//   // which output window is the second window mapped to? 
//   Int outwin = idx(2);     // outwin = -1 if not selected
// </srcblock>
// </example>
//
// <motivation>
// The MirFiller class needs to keep track of which Miriad windows get mapped 
// into which MS spectral windows.  Since the user can select which windows 
// will be loaded, its not easy to predict otherwise how the windows will get 
// mapped.  Furthermore, Miriad wideband channels are loaded as seperate 
// windows in the output MS, thus the mapping from channel number to window 
// will not necessarily start with 0.  
// </motivation>

class IDIndex {
private:
    Int offset;
    SimpleOrderedMap<Int, Int> idmap;
public:    
    // create an ID set.  <src>first</src> is the output index that the first
    // input ID passed to <src>add()</src> will be mapped to.
    //PJT
    //    explicit IDIndex(Int first=0);
    IDIndex(Int first=0);

    // create a copy of another IDIndex
    IDIndex(IDIndex& that);

    // destroy this index
    ~IDIndex();

    // add an ID to the set;
    void add(Int id) { idmap.define(id, next()); }

    // return the ID mapping
    Int operator()(Int id) { return idmap(id); }

    // return the output index that the first input ID is mapped to.  This is the 
    // value returned by operator(0);
    Int first() { return offset; }

    // return the next index to be mapped to when add is next called.
    Int next() { return offset+idmap.ndefined(); }

    // return the number of input IDs mapped
    Int size() { return idmap.ndefined(); }

    // remove all ID mappings.  This returns the index to its state just 
    // after construction.
    void clear() { idmap.clear(); }
};



#endif


