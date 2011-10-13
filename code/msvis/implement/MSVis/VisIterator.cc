//# VisibilityIterator.cc: Step through MeasurementEquation by visibility
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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
//# $Id: VisIterator.cc,v 19.15 2006/02/01 01:25:14 kgolap Exp $

#include <msvis/MSVis/VisIterator.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/UtilJ.h>
#include <scimath/Mathematics/InterpolateArray1D.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/Sort.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <casa/Quanta/MVTime.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ColDescSet.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TiledColumnStMan.h>
#include <tables/Tables/TiledStManAccessor.h>
#include <msvis/MSVis/VisibilityIteratorImpl.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class ROVisIteratorImpl : public VisibilityIteratorReadImpl {

    friend class ROVisIterator;
    friend class VisIterator;

public:

  // Default constructor - useful only to assign another iterator later
  ROVisIteratorImpl();

  // Construct from MS and a Block of MS column enums specifying the iteration
  // order, if none are specified, time iteration is implicit.  An optional
  // timeInterval can be given to iterate through chunks of time.  The default
  // interval of 0 groups all times together.  Every 'chunk' of data contains
  // all data within a certain time interval (in seconds) and with identical
  // values of the other iteration columns (e.g.  DATA_DESC_ID and FIELD_ID).
  // Using selectChannel(), a number of groups of channels can be requested.
  // At present the channel group iteration will always occur before the
  // interval iteration.
  ROVisIteratorImpl(ROVisIterator * rovi,
                    const MeasurementSet& ms,
                    const Block<Int>& sortColumns,
                    Double timeInterval=0);

  // Copy construct. This calls the assignment operator.
  ROVisIteratorImpl(const ROVisIteratorImpl & other);

  // Destructor
  ~ROVisIteratorImpl();

  // Assignment. Any attached VisBuffers are lost in the assign.
  ROVisIteratorImpl & operator=(const ROVisIteratorImpl &other);

  // Members

  // Advance iterator through data
  ROVisIteratorImpl & operator++(int);
  ROVisIteratorImpl & operator++();

  // Return channel numbers in selected VisSet spectrum
  // (i.e. disregarding possible selection on the iterator, but
  //  including the selection set when creating the VisSet)
  Vector<Int>& channel(Vector<Int>& chan) const;
  Vector<Int>& chanIds(Vector<Int>& chanids) const;
  Vector<Int>& chanIds(Vector<Int>& chanids,Int spw) const;

  // Return selected correlation indices
  Vector<Int>& corrIds(Vector<Int>& corrids) const;

  // Return flag for each polarization, channel and row
  Cube<Bool>& flag(Cube<Bool>& flags) const;

  // Return current frequencies
  Vector<Double>& frequency(Vector<Double>& freq) const;

  // Return the correlation type (returns Stokes enums)
  Vector<Int>& corrType(Vector<Int>& corrTypes) const;

  // Return sigma matrix (pol-dep)
  Matrix<Float>& sigmaMat(Matrix<Float>& sigmat) const;

  // Return the visibilities as found in the MS, Cube(npol,nchan,nrow).
  Cube<Complex>& visibility(Cube<Complex>& vis,
			    DataColumn whichOne) const;
  // Return weight matrix
  Matrix<Float>& weightMat(Matrix<Float>& wtmat) const;

  // Return weightspectrum (a weight for each corr & channel)
  Cube<Float>& weightSpectrum(Cube<Float>& wtsp) const;

  // Set up new chan/corr selection via Vector<Slice>
  void selectChannel(const Vector<Vector<Slice> >& chansel);
  void selectCorrelation(const Vector<Vector<Slice> >& corrsel);

  // Set up/return channel averaging bounds
  Vector<Matrix<Int> >& setChanAveBounds(Float factor, Vector<Matrix<Int> >& bounds);

  // Return number of chans/corrs per spw/pol
  Int numberChan(Int spw) const;
  Int numberCorr(Int pol) const;

  // Return the row ids as from the original root table. This is useful
  // to find correspondance between a given row in this iteration to the
  // original ms row
  Vector<uInt>& rowIds(Vector<uInt>& rowids) const;

  // Need to override this and not use getColArray
  Vector<RigidVector<Double,3> >& uvw(Vector<RigidVector<Double,3> >& uvwvec) const;

protected:

  void setSelTable();

  const Table attachTable() const;

  // update the DATA slicer
  void updateSlicer();
  // attach the column objects to the currently selected table

  // The ROVisibilityIterator version of this function sets the tile cache to 1
  // because of a feature in sliced data access that grows memory dramatically in
  // some cases.  However, ROVisibilityIterator, because it uses
  // ROArrayColumn::getColumn(Vector<Vector<Slice> >&), is (1/28/2011) incredibly
  // slow if the tile cache does not span all the selected channels, and it will
  // read orders of magnitude more data than it needs to.  This sets the tile
  // cache to the minimum number of tiles required to span the selected channels.
  // Unlike ROVisibilityIterator, it does it for each hypercube, not just the
  // first one, and it does its work when the DDID has changed.
  void setTileCache();

  void getDataColumn(DataColumn whichOne, const Vector<Vector<Slice> >& slices,
			     Cube<Complex>& data) const;

  // Column access functions
  void getCol(const ROScalarColumn<Bool> &column, Vector<Bool> &array, Bool resize = False) const;
  void getCol(const ROScalarColumn<Int> &column, Vector<Int> &array, Bool resize = False) const;
  void getCol(const ROScalarColumn<Double> &column, Vector<Double> &array, Bool resize = False) const;

  void getCol(const ROArrayColumn<Bool> &column, Array<Bool> &array, Bool resize = False) const;
  void getCol(const ROArrayColumn<Float> &column, Array<Float> &array, Bool resize = False) const;
  void getCol(const ROArrayColumn<Double> &column, Array<Double> &array, Bool resize = False) const;
  void getCol(const ROArrayColumn<Complex> &column, Array<Complex> &array, Bool resize = False) const;

  void getCol(const ROArrayColumn<Bool> &column, const Slicer &slicer, Array<Bool> &array, Bool resize = False) const;
  void getCol(const ROArrayColumn<Float> &column, const Slicer &slicer, Array<Float> &array, Bool resize = False) const;
  void getCol(const ROArrayColumn<Complex> &column, const Slicer &slicer, Array<Complex> &array, Bool resize = False) const;


  // New slicer supports multiple Slices in channel and correlation

  Vector<Matrix<Int> >   chanAveBounds_p;
  Vector<Vector<Slice> > chanSlices_p;
  Vector<Vector<Slice> > corrSlices_p;
  Vector<Vector<Slice> > newSlicer_p;
  Vector<Vector<Slice> > newWtSlicer_p;
  Table                  selTable_p;
  Bool                   useNewSlicer_p;

};


ROVisIterator::ROVisIterator()
{
}

ROVisIterator::ROVisIterator(const MeasurementSet & ms, const Block<Int>& sortColumns, Double timeInterval)
: ROVisibilityIterator (ms, sortColumns, timeInterval, Factory (this))
{}

ROVisIterator::ROVisIterator(const ROVisIterator & other)
: ROVisibilityIterator (other)
{}

ROVisIterator::~ROVisIterator()
{}

ROVisIterator &
ROVisIterator::operator=(const ROVisIterator & other)
{
    ROVisibilityIterator::operator= (other);

    return * this;
}

ROVisIterator &
ROVisIterator::operator++(int)
{
    advance();
    return * this;
}

ROVisIterator &
ROVisIterator::operator++()
{
    advance();
    return * this;
}

Vector<Int>&
ROVisIterator::chanIds(Vector<Int>& chanids) const
{
    return getReadImpl()->chanIds(chanids);
}

Vector<Int>&
ROVisIterator::chanIds(Vector<Int>& chanids, Int spw) const
{
    return getReadImpl()->chanIds(chanids, spw);
}

Vector<Int>&
ROVisIterator::corrIds(Vector<Int>& corrids) const
{
    return getReadImpl()->corrIds(corrids);
}


Vector<Int>&
ROVisIterator::corrType(Vector<Int>& corrTypes) const
{
    return getReadImpl()->corrType(corrTypes);
}

ROVisIteratorImpl *
ROVisIterator::getReadImpl () const
{
    ROVisIteratorImpl * impl = dynamic_cast<ROVisIteratorImpl *> (ROVisibilityIterator::getReadImpl ());

    Assert (impl != NULL);

    return impl;
}

void
ROVisIterator::selectChannel(const Vector<Vector<Slice> >& chansel)
{
    return getReadImpl()->selectChannel(chansel);
}

void
ROVisIterator::selectCorrelation(const Vector<Vector<Slice> >& corrsel)
{
    return getReadImpl()->selectCorrelation(corrsel);
}

Vector<Matrix<Int> >&
ROVisIterator::setChanAveBounds(Float factor, Vector<Matrix<Int> >& bounds)
{
    return getReadImpl()->setChanAveBounds(factor, bounds);
}

Int
ROVisIterator::numberChan(Int spw) const
{
    return getReadImpl()->numberChan(spw);
}

Int
ROVisIterator::numberCorr(Int pol) const
{
    return getReadImpl()->numberCorr(pol);
}

void
ROVisIterator::getDataColumn(DataColumn whichOne, const Vector<Vector<Slice> >& slices, Cube<Complex>& data) const
{
    getReadImpl()->getDataColumn(whichOne, slices, data);
}

VisibilityIteratorReadImpl *
ROVisIterator::Factory::operator() (const asyncio::PrefetchColumns * /*prefetchColumns*/,
                                    const Block<MeasurementSet>& mss,
                                    const Block<Int>& sortColumns,
                                    const Bool /*addDefaultSortCols*/,
                                    Double timeInterval) const
{
    return new ROVisIteratorImpl (vi_p, mss[0], sortColumns, timeInterval);
}

//     *************************
//     *************************
//     **                     **
//     **  ROVisIteratorImpl  **
//     **                     **
//     *************************
//     *************************

ROVisIteratorImpl::ROVisIteratorImpl() {}

ROVisIteratorImpl::ROVisIteratorImpl(ROVisIterator * rovi,
                                     const MeasurementSet &ms,
                                     const Block<Int>& sortColumns,
                                     Double timeInterval)
: VisibilityIteratorReadImpl(rovi, Block<MeasurementSet> (1, ms), sortColumns, True, timeInterval),
  useNewSlicer_p(False)
{

    // Initialize multi-slicers with empty slices
    chanSlices_p.resize(numberSpw());
    chanSlices_p.set(Vector<Slice>());
    corrSlices_p.resize(numberPol());
    corrSlices_p.set(Vector<Slice>());

}

ROVisIteratorImpl::ROVisIteratorImpl(const ROVisIteratorImpl& other)
: VisibilityIteratorReadImpl (other)
{
    operator=(other);
}

ROVisIteratorImpl::~ROVisIteratorImpl() {}

ROVisIteratorImpl&
ROVisIteratorImpl::operator=(const ROVisIteratorImpl& other)
{
  if (this!=&other) {

    VisibilityIteratorReadImpl::operator=(other);

    chanAveBounds_p = other.chanAveBounds_p;
    chanSlices_p = other.chanSlices_p;
    corrSlices_p = other.corrSlices_p;
    newSlicer_p = other.newSlicer_p;
    newWtSlicer_p = other.newWtSlicer_p;
    selTable_p = other.selTable_p;
    useNewSlicer_p = other.useNewSlicer_p;

  }
  return *this;
}

ROVisIteratorImpl & ROVisIteratorImpl::operator++(int)
{
  if (! more ()){
      advance();
  }
  return *this;
}

ROVisIteratorImpl & ROVisIteratorImpl::operator++()
{
  if (! more ()){
      advance();
  }
  return *this;
}



void ROVisIteratorImpl::updateSlicer()
{
  useNewSlicer_p=True;

  //    cout << "Using new slicer!..." << flush;
  
  newSlicer_p.resize(2);
  newWtSlicer_p.resize(1);
  
  //    cout << "newSlicer_p.shape() = " << newSlicer_p.shape() << endl;
  
  useSlicer_p=False;
  
  // Refer to correct slices for the current spwid/polid
  newSlicer_p(0).reference(corrSlices_p(this->polarizationId()));
  newSlicer_p(1).reference(chanSlices_p(this->spectralWindow()));
  
  newWtSlicer_p(0).reference(corrSlices_p(this->polarizationId()));

  setTileCache();
}

// (Alternative syntax for ROVisIter::chanIds)
Vector<Int>& ROVisIteratorImpl::channel(Vector<Int>& chan) const
{ return this->chanIds(chan); }


Vector<Int>& ROVisIteratorImpl::chanIds(Vector<Int>& chanids) const
{
  return chanIds(chanids,msIter_p.spectralWindowId());
}

Vector<Int>& ROVisIteratorImpl::chanIds(Vector<Int>& chanids, Int spw) const
{

  chanids.resize(this->numberChan(spw));

  // if non-trivial slices available
  if (chanSlices_p(spw).nelements() > 0 ) {
    
    Vector<Slice> slices(chanSlices_p(spw));
    Int nslices=slices.nelements();
    
    Int ich0=0;
    for (Int i=0;i<nslices;++i) {
      Int nchan=slices(i).length();
      Int start=slices(i).start();
      for (Int ich=0;ich<nchan;++ich,++ich0) 
	chanids(ich0)=start+ich;
    }
    
  }
  else {
    // all channels selected...
    indgen(chanids);
  }
  return chanids;
}

void ROVisIteratorImpl::setSelTable()
{
    VisibilityIteratorReadImpl::setSelTable();

    // The following code (which uses Table::operator() to create
    // a new RefTable) is computationally expensive. This could
    // be optimized by using the same method as in the
    // VisibilityIterator base class (which is to not create
    // RefTables but instead access the table column directly in
    // getReadImpl()->msIter_p.table() using VisibilityIterator::getReadImpl()->selRows_p).

    // Doing so would mean replacing calls like
    //     colSigma.getColumn(newWtSlicer_p,sigmat,True);
    // with
    //     colSigma.getColumnCells(getReadImpl()->selRows_p,newWtSlicer_p,sigmat,True);
    //
    // However the ArrayColumn class does allow passing both a 
    // Vector<Vector<Slice> > and a RefRows parameter to get-/putColumn.
    // Such put/get functions must be first implemented.

    Vector<uInt> rows(curNumRow_p);
    indgen(rows,uInt(curStartRow_p));
    selTable_p=msIter_p.table()(rows);
    attachColumns(attachTable());
}

// Return native correlation _indices_
Vector<Int>& ROVisIteratorImpl::corrIds(Vector<Int>& corrids) const
{
  Int pol = msIter_p.polarizationId();

  corrids.resize(this->numberCorr(pol));

  
  Vector<Slice> slices(corrSlices_p(pol));
  Int nslices=slices.nelements();
  
  // if non-trivial slices available
  if (nslices > 0 ) {
    
    Int icor0=0;
    for (Int i=0;i<nslices;++i) {
      Int ncorr=slices(i).length();
      Int start=slices(i).start();
      for (Int icor=0;icor<ncorr;++icor,++icor0) 
	corrids(icor0)=start+icor;
    }
    
  }
  else {
    // all corrs selected...
    indgen(corrids);
  }

  return corrids;
}

Vector<Int>&
ROVisIteratorImpl::corrType(Vector<Int>& corrTypes) const
{

  // TBD:  Use corrIds instead of mask

  // Get the nominal corrType list
  Int polId = msIter_p.polarizationId();
  Vector<Int> nomCorrTypes;
  msIter_p.msColumns().polarization().corrType().get(polId,nomCorrTypes,True);

  if (corrSlices_p(polId).nelements() > 0) {
    Vector<Bool> corrmask(nomCorrTypes.nelements(),False);
    Vector<Slice> corrsel(corrSlices_p(polId));
    for (uInt i=0;i<corrsel.nelements();++i)
      corrmask(corrsel(i).start())=True;
    
    // Reference the selected subset
    corrTypes.reference(nomCorrTypes(corrmask).getCompressedArray());

  }
  else
    corrTypes.reference(nomCorrTypes);

  //    cout << "corrTypes = " << corrTypes << endl;
	
  return corrTypes;

}

Cube<Bool>& ROVisIteratorImpl::flag(Cube<Bool>& flags) const
{
  if (useNewSlicer_p){
      columns_p.flag_p.getColumn(newSlicer_p,flags,True);
  }
  else {
      columns_p.flag_p.getColumn(flags,True);
  }
  return flags;
}

Vector<Double>& ROVisIteratorImpl::frequency(Vector<Double>& freq) const
{
  
  // We need to change some internals transparently
  //ROVisIteratorImpl* self = const_cast<ROVisIteratorImpl*>(this);

  if (! cache_p.freqCacheOK_p) {
    cache_p.freqCacheOK_p=True;
    const Vector<Double>& chanFreq=msIter_p.frequency();
    Vector<Int> chan;
    channel(chan);
    Int nch=chan.nelements();
    cache_p.frequency_p.resize(nch);

    for (Int i=0;i<nch;++i){
      cache_p.frequency_p(i)=chanFreq(chan(i));
    }
  }
  // Always copy to output
  freq.resize(cache_p.frequency_p.nelements());
  freq=cache_p.frequency_p;

  return freq;
}

Cube<Complex>& 
ROVisIteratorImpl::visibility(Cube<Complex>& vis, DataColumn whichOne) const
{
  if (useNewSlicer_p){
      getDataColumn(whichOne,newSlicer_p,vis);
  }
  else {
      VisibilityIteratorReadImpl::getDataColumn(whichOne,vis);
  }
  return vis;
}

void ROVisIteratorImpl::getDataColumn(DataColumn whichOne,
				  const Vector<Vector<Slice> >& slices,
				  Cube<Complex>& data) const
{ 
  // Return the visibility (observed, model or corrected);
  // deal with DATA and FLOAT_DATA seamlessly for observed data.
  switch (whichOne) {
  case ROVisIterator::Observed:
    if (floatDataFound_p) {
      Cube<Float> dataFloat;
      columns_p.floatVis_p.getColumn(slices,dataFloat,True);
      data.resize(dataFloat.shape());
      convertArray(data,dataFloat);
    } else {
      columns_p.vis_p.getColumn(slices,data,True);
    };
    break;
  case ROVisIterator::Corrected:
    columns_p.corrVis_p.getColumn(slices,data,True);
    break;
  case ROVisIterator::Model:
    columns_p.modelVis_p.getColumn(slices,data,True);
    break;
  };
 
};

Matrix<Float>& ROVisIteratorImpl::sigmaMat(Matrix<Float>& sigmat) const
{
  if (useNewSlicer_p) columns_p.sigma_p.getColumn(newWtSlicer_p,sigmat,True);
  else {
    sigmat.resize(nPol_p,curNumRow_p);
    columns_p.sigma_p.getColumn(sigmat);
  }
  return sigmat;
}

Matrix<Float>& ROVisIteratorImpl::weightMat(Matrix<Float>& wtmat) const
{
  if (useNewSlicer_p) columns_p.weight_p.getColumn(newWtSlicer_p,wtmat,True);
  else {
    wtmat.resize(nPol_p,curNumRow_p);
    columns_p.weight_p.getColumn(wtmat);
  }
  return wtmat;
}

Cube<Float>& ROVisIteratorImpl::weightSpectrum(Cube<Float>& wtsp) const
{
  if (!columns_p.weightSpectrum_p.isNull()) {
    if (useNewSlicer_p) columns_p.weightSpectrum_p.getColumn(newSlicer_p,wtsp,True);
    else {
      wtsp.resize(nPol_p,nChan_p,curNumRow_p);
      columns_p.weightSpectrum_p.getColumn(wtsp);
    }
  } else {
    wtsp.resize(0,0,0);
  }
  return wtsp;
}

void ROVisIteratorImpl::selectChannel(const Vector<Vector<Slice> >& chansel) {
  //  cout << "selectChannel!...vss..." << flush;

  if (chansel.nelements() != uInt(numberSpw()))
    throw(AipsError("Specified channel slices has incorrect number of spws."));
  
  chanSlices_p.resize(numberSpw(),False);
  chanSlices_p=chansel;

  // Enforce use of the new slicer downstream
  useNewSlicer_p=True;
    
  //  cout << "done." << endl;

}

void ROVisIteratorImpl::selectCorrelation(const Vector<Vector<Slice> >& corrsel) {

  //  cout << "selectCorrelation!...vvs..." << flush;

  if (corrsel.nelements() != uInt(numberPol()))
    throw(AipsError("Specified correlation slices has incorrect number of polIds."));
  
  corrSlices_p.resize(numberPol(),False);
  corrSlices_p=corrsel;

  // Enforce use of the new slicer downstream
  useNewSlicer_p=True;
    
  //  cout << "done." << endl;

}

Vector<Matrix<Int> >& ROVisIteratorImpl::setChanAveBounds(Float factor,
						      Vector<Matrix<Int> >& bounds)
{
  if(!useNewSlicer_p)
    throw(AipsError("Help!"));

  // For every spw...
  bounds.resize(numberSpw());

  // If factor greater than zero, fill the bounds non-trivially
  if (factor > 0) {
 
    // Decipher averaging factor
    Int width = 1;
    if(factor > 1.0)
      width = Int(factor); // factor supplied in channel units
 
    // Calculate bounds for each spw
    for(Int ispw = 0; ispw < numberSpw(); ++ispw){
   
      // Number of SELECTED channels PRIOR to averaging
      Int nChan0 = numberChan(ispw);
   
      // factor might have been supplied as a fraction;
      //  width is then potentially spw-dependent
      if(factor <= 1.0)
	width = Int(factor * Float(nChan0));
   
      // Get the selected channel list
      Vector<Int> chans;
      chanIds(chans, ispw);

      // The nominal number of output channels depends on the full
      // range of channels selected (not the number of them)
      //  (will be revised later, if nec.)
      Int nChanOut0 = 1 + (chans[nChan0 - 1] - chans[0]) / width;
   
      // Initialize the bounds container for this spw
      Matrix<Int>& currBounds(bounds[ispw]);
      currBounds.resize(nChanOut0, 4);
      //currBounds.set(0);
   
      Int outChan = 0;
      Int firstchan = chans[0];
      Int lastchan = chans[nChan0 - 1];

      // Index of input channel in SELECTED list, i.e.
      // ich = vi.chanIds(chanids, spw)[selchanind].
      uInt selchanind = 0;

      for(Int ich = firstchan; ich <= lastchan; ich += width){
        Int w = width;

        // Use a clamped width in case (lastchan - firstchan + 1) % width != 0.
        if(ich + w - 1 > lastchan)
          w = lastchan + 1 - ich;

        // The start and end in MS channel #s.
        currBounds(outChan, 0) = ich;
        currBounds(outChan, 1) = ich + w - 1;

        // The start and end in selected reckoning.
        currBounds(outChan, 2) = selchanind;
        selchanind += w;
        currBounds(outChan, 3) = selchanind - 1;

        // for(uInt ii = 0; ii < 4; ++ii)
        //   cerr << "currBounds(" << outChan << ", " << ii << ") = "
        //        << currBounds(outChan, ii) << endl;
        ++outChan;
      }
    } // ispw

  } // factor > 0

  // Internal reference  (needed?)
  chanAveBounds_p.reference(bounds);

  // Return the bounds Vector reference
  return bounds;
}

// Vector<Matrix<Int> >& ROVisIteratorImpl::setChanAveBounds(Float factor,
// 						      Vector<Matrix<Int> >& bounds) {

//   if (!useNewSlicer_p) throw(AipsError("Help!"));

//   // For every spw...
//   bounds.resize(numberSpw());

//   // If factor greater than zero, fill the bounds non-trivially
//   if (factor>0) {
    
//     // Decipher averaging factor
//     Int width(1);
//     if (factor>1.0) width=Int(factor); // factor supplied in channel units
    
//     // Calculate bounds for each spw
//     for (Int ispw=0;ispw<numberSpw();++ispw) {
      
//       // Number of SELECTED channels PRIOR to averaging
//       Int nChan0=numberChan(ispw);
      
//       // factor might have been supplied in factional units;
//       //  width is then potentially spw-dependent
//       if (factor<=1.0)
// 	width=Int(factor*Float(nChan0));
      
//       // Get the selected channel list
//       Vector<Int> chans;
//       chanIds(chans,ispw);

//       // The nominal number of output channels depends on the full
//       // range of channels selected (not the number of them)
//       //  (will be revised later, if nec.)
//       Int nChanOut0((chans(nChan0-1)-chans(0)+1+width)/width);
      
//       // Initialize the bounds container for this spw
//       Matrix<Int>& currBounds(bounds(ispw));
//       currBounds.resize(nChanOut0,4);
//       currBounds.set(0);
      
//       // Actual output channel count; initialization
//       Int nChanOut=1;
//       Int lo(chans(0));
//       currBounds(0,0)=lo;
//       currBounds(0,2)=0;
//       for (Int ich=0;ich<nChan0;++ich) 
// 	if ( (chans(ich)-lo+1)>width ) {
// 	  currBounds(nChanOut-1,1)=chans(ich-1);   // end of previous
// 	  currBounds(nChanOut-1,3)=ich-1;
// 	  lo=currBounds(nChanOut,0)=chans(ich);    // start of next
// 	  currBounds(nChanOut,2)=ich;    // start of next
// 	  ++nChanOut;
// 	}
//       currBounds(nChanOut-1,1)=chans(nChan0-1);    // end of final set
//       currBounds(nChanOut-1,3)=(nChan0-1);    // end of final set
      
// //       for(uInt ii = 0; ii < 4; ++ii)
// //         cerr << "currBounds(" << nChanOut - 1 << ", " << ii << ") = "
// //              << currBounds(nChanOut - 1, ii) << endl;

//       // contract bounds container, if necessary
//       if (nChanOut<nChanOut0)
// 	currBounds.resize(nChanOut,4,True);
      
//     } // ispw


//   } // factor > 0

//   // Internal reference  (needed?)
//   chanAveBounds_p.reference(bounds);

//   // Return the bounds Vector reference
//   return bounds;

// }

Int ROVisIteratorImpl::numberChan(Int spw) const {

  // Nominally all channels this spw
  Int nchan=msColumns().spectralWindow().numChan()(spw);

  if (useNewSlicer_p) {
    Int nslices=chanSlices_p(spw).nelements();
    if (nslices > 0 ) {
      nchan=0;
      for (Int isl=0;isl<nslices;++isl) 
	nchan+=chanSlices_p(spw)(isl).length();
    }
  }

  return nchan;

}


Int ROVisIteratorImpl::numberCorr(Int pol) const {

  // Nominally all correlations this pol
  Int ncorr=msColumns().polarization().numCorr()(pol);

  if (useNewSlicer_p) {
    Int nslices=corrSlices_p(pol).nelements();
    if (nslices > 0 ) {
      // Accumulate from slice lengths
      ncorr=0;
      for (Int isl=0;isl<nslices;++isl) 
	ncorr+=corrSlices_p(pol)(isl).length();
    }
  }

  return ncorr;

}

void ROVisIteratorImpl::getCol(const ROScalarColumn<Bool> &column, Vector<Bool> &array, Bool resize) const
{
    column.getColumn(array, resize);
}

void ROVisIteratorImpl::getCol(const ROScalarColumn<Int> &column, Vector<Int> &array, Bool resize) const
{
    column.getColumn(array, resize);
}

void ROVisIteratorImpl::getCol(const ROScalarColumn<Double> &column, Vector<Double> &array, Bool resize) const
{
    column.getColumn(array, resize);
}

void ROVisIteratorImpl::getCol(const ROArrayColumn<Bool> &column, Array<Bool> &array, Bool resize) const
{
    column.getColumn(array, resize);
}

void ROVisIteratorImpl::getCol(const ROArrayColumn<Float> &column, Array<Float> &array, Bool resize) const
{
    column.getColumn(array, resize);
}

void ROVisIteratorImpl::getCol(const ROArrayColumn<Double> &column, Array<Double> &array, Bool resize) const
{
    column.getColumn(array, resize);
}

void ROVisIteratorImpl::getCol(const ROArrayColumn<Complex> &column, Array<Complex> &array, Bool resize) const
{
    column.getColumn(array, resize);
}

void ROVisIteratorImpl::getCol(const ROArrayColumn<Bool> &column, const Slicer &slicer, Array<Bool> &array, Bool resize) const
{
    column.getColumn(slicer, array, resize);
}

void ROVisIteratorImpl::getCol(const ROArrayColumn<Float> &column, const Slicer &slicer, Array<Float> &array, Bool resize) const
{
    column.getColumn(slicer, array, resize);
}

void ROVisIteratorImpl::getCol(const ROArrayColumn<Complex> &column, const Slicer &slicer, Array<Complex> &array, Bool resize) const
{
    column.getColumn(slicer, array, resize);
}


Vector<RigidVector<Double,3> >& 
ROVisIteratorImpl::uvw(Vector<RigidVector<Double,3> >& uvwvec) const
{
    uvwvec.resize(curNumRow_p);
    getCol(columns_p.uvw_p, cache_p.uvwMat_p,True);
    // get a pointer to the raw storage for quick access
    Bool deleteIt;
    Double* pmat = cache_p.uvwMat_p.getStorage(deleteIt);
    for (uInt row=0; row<curNumRow_p; row++, pmat+=3) uvwvec(row)=pmat;
    return uvwvec;
}


const Table
ROVisIteratorImpl::attachTable() const
{
    return selTable_p;
}

VisIterator::VisIterator() {}

VisIterator::VisIterator(MeasurementSet &MS, 
			 const Block<Int>& sortColumns, 
			 Double timeInterval)
  : ROVisIterator(MS, sortColumns, timeInterval)
{}

VisIterator::VisIterator(const VisIterator & other)
: ROVisIterator (other)
{
    operator=(other);
}

VisIterator::~VisIterator() {}

VisIterator& 
VisIterator::operator=(const VisIterator& other)
{
    if (this!=&other) {

        ROVisIterator::operator=(other);

        rwColFlag_p.reference(other.rwColFlag_p);
        rwColFlagRow_p.reference(other.rwColFlagRow_p);
        rwColVis_p.reference(other.rwColVis_p);
        rwColFloatVis_p.reference(other.rwColFloatVis_p);
        rwColModelVis_p.reference(other.rwColModelVis_p);
        rwColCorrVis_p.reference(other.rwColCorrVis_p);
        rwColWeight_p.reference(other.rwColWeight_p);
        rwColWeightSpectrum_p.reference(other.rwColWeightSpectrum_p);
        rwColSigma_p.reference(other.rwColSigma_p);

    }
    return *this;
}

VisIterator &
VisIterator::operator++(int)
{
  if (more ()){
      advance();
  }
  return *this;
}

VisIterator &
VisIterator::operator++()
{
  if (more ()){
      advance();
  }
  return *this;
}

void VisIterator::attachColumns(const Table &t)
{
  ROVisIterator::attachColumns(t);

  //todo: should cache this (update once per ms)
  const ColumnDescSet& cds=t.tableDesc().columnDescSet();
  if (cds.isDefined(MS::columnName(MS::DATA))) {
      getReadImpl()->columns_p.vis_p.attach(t,MS::columnName(MS::DATA));
  };
  if (cds.isDefined(MS::columnName(MS::FLOAT_DATA))) {
    getReadImpl()->floatDataFound_p=True;
    getReadImpl()->columns_p.floatVis_p.attach(t,MS::columnName(MS::FLOAT_DATA));
  } else {
    getReadImpl()->floatDataFound_p=False;
  };
  if (cds.isDefined("MODEL_DATA")) 
    getReadImpl()->columns_p.modelVis_p.attach(t,"MODEL_DATA");
  if (cds.isDefined("CORRECTED_DATA")) 
    getReadImpl()->columns_p.corrVis_p.attach(t,"CORRECTED_DATA");
  getReadImpl()->columns_p.weight_p.attach(t,MS::columnName(MS::WEIGHT));
  if (cds.isDefined("WEIGHT_SPECTRUM"))
    getReadImpl()->columns_p.weightSpectrum_p.attach(t,"WEIGHT_SPECTRUM");
  getReadImpl()->columns_p.sigma_p.attach(t,MS::columnName(MS::SIGMA));
  getReadImpl()->columns_p.flag_p.attach(t,MS::columnName(MS::FLAG));
  getReadImpl()->columns_p.flagRow_p.attach(t,MS::columnName(MS::FLAG_ROW));
}

void VisIterator::setFlagRow(const Vector<Bool>& rowflags)
{
  rwColFlagRow_p.putColumn(rowflags);
};

void VisIterator::setVis(const Cube<Complex>& vis, DataColumn whichOne)
{
  
  if (getReadImpl()->useNewSlicer_p){
      putDataColumn (whichOne, getReadImpl()->newSlicer_p, vis);
  }
  else {
      putDataColumn (whichOne, vis);
  }

}

void VisIterator::setFlag(const Cube<Bool>& flags)
{
  if (getReadImpl()->useNewSlicer_p){
      rwColFlag_p.putColumn (getReadImpl()->newSlicer_p, flags);
  }
  else{
      rwColFlag_p.putColumn(flags);
  }
}

void VisIterator::setVisAndFlag(const Cube<Complex>& vis,
				       const Cube<Bool>& flag,
				       DataColumn whichOne)
{
  this->setFlag(flag);
  this->setVis(vis,whichOne);
}

void VisIterator::setWeightMat(const Matrix<Float>& weightMat)
{
  if (getReadImpl()->useNewSlicer_p){
      rwColWeight_p.putColumn (getReadImpl()->newWtSlicer_p, weightMat);
  }
  else{
      rwColWeight_p.putColumn (weightMat);
  }
}

void VisIterator::setWeightSpectrum(const Cube<Float>& weightSpectrum)
{
  if (!getReadImpl()->columns_p.weightSpectrum_p.isNull()) {
    if (getReadImpl()->useNewSlicer_p){
        rwColWeightSpectrum_p.putColumn(getReadImpl()->newSlicer_p,weightSpectrum);
    }
    else{
        rwColWeightSpectrum_p.putColumn(weightSpectrum);
    }
  }
  else {
    throw(AipsError("Can't set WEIGHT_SPECTRUM -- it doesn't exist!"));
  }
}



void VisIterator::putDataColumn(DataColumn whichOne,
				const Vector<Vector<Slice> >& slices,
				const Cube<Complex>& data)
{
  // Set the visibility (observed, model or corrected);
  // deal with DATA and FLOAT_DATA seamlessly for observed data.
  switch (whichOne) {
  case Observed:
    if (getReadImpl()->floatDataFound_p) {
      Cube<Float> dataFloat=real(data);
      rwColFloatVis_p.putColumn(slices,dataFloat);
    } else {
      rwColVis_p.putColumn(slices,data);
    };
    break;
  case Corrected:
    rwColCorrVis_p.putColumn(slices,data);
    break;
  case Model:
    rwColModelVis_p.putColumn(slices,data);
    break;
  };
};  

void VisIterator::putDataColumn(DataColumn whichOne,
				const Cube<Complex>& data)
{
  // Set the visibility (observed, model or corrected);
  // deal with DATA and FLOAT_DATA seamlessly for observed data.
  switch (whichOne) {
  case Observed:
    if (getReadImpl()->floatDataFound_p) {
      Cube<Float> dataFloat=real(data);
      rwColFloatVis_p.putColumn(dataFloat);
    } else {
      rwColVis_p.putColumn(data);
    };
    break;
  case Corrected:
    rwColCorrVis_p.putColumn(data);
    break;
  case Model:
    rwColModelVis_p.putColumn(data);
    break;
  };
};  

Vector<uInt>& ROVisIteratorImpl::rowIds(Vector<uInt>& rowids) const
{
  rowids.resize(curNumRow_p);
  rowids=selTable_p.rowNumbers();
  return rowids;
}

void ROVisIteratorImpl::setTileCache()
{
  // Set the cache size when the DDID changes (as opposed to MS) to avoid 
  // overreading in a case like:
  // hcubes: [2, 256], [4, 64]
  // tileshape: [4, 64]
  // spws (ddids): [4,64], [2, 256], [2, 256], [4,64], [2, 256], [4,64]
  // and spw = '0:1~7,1:1~7,2:100~200,3:20~40,4:200~230,5:40~50'
  //
  // For hypercube 0, spw 2 needs 3 tiles, but spw 4 only needs 1, AND the last
  // tile at that.  So if hypercube 0 used a cache of 3 tiles throughout, every
  // read of 4:200~230 would likely also read the unwanted channels 0~127 of
  // the next row.
  //
  if (curStartRow_p != 0 && ! msIter_p.newDataDescriptionId()){
    return;
  }

  const MeasurementSet& thems = msIter_p.ms();

  if(thems.tableType() == Table::Memory){
    return;
  }

  const ColumnDescSet& cds=thems.tableDesc().columnDescSet();

  // Get the first row number for this DDID.
  Vector<uInt> rownums;
  rowIds(rownums);
  uInt startrow = rownums[0];
  
  Vector<String> columns(8);
  // complex
  columns(0)=MS::columnName(MS::DATA);
  columns(1)=MS::columnName(MS::CORRECTED_DATA);
  columns(2)=MS::columnName(MS::MODEL_DATA);
  // boolean
  columns(3)=MS::columnName(MS::FLAG);
  // float
  columns(4)=MS::columnName(MS::WEIGHT_SPECTRUM);
  columns(5)=MS::columnName(MS::WEIGHT);
  columns(6)=MS::columnName(MS::SIGMA);
  // double
  columns(7)=MS::columnName(MS::UVW);
  //
  for(uInt k = 0; k < columns.nelements(); ++k){
    if(cds.isDefined(columns(k))){
      const ColumnDesc& cdesc = cds[columns(k)];
      String dataManType="";
      
      dataManType = cdesc.dataManagerType();
      // We have to check WEIGHT_SPECTRUM as it tends to exist but not have
      // valid data.
      if(columns[k] == MS::columnName(MS::WEIGHT_SPECTRUM) &&
         !existsWeightSpectrum())
        dataManType="";

      // Sometimes columns may not contain anything yet
      if((columns[k]==MS::columnName(MS::DATA) && (columns_p.vis_p.isNull() ||
                                                   !columns_p.vis_p.isDefined(0))) ||
         (columns[k]==MS::columnName(MS::MODEL_DATA) && (columns_p.modelVis_p.isNull() ||
                                                         !columns_p.modelVis_p.isDefined(0))) ||
         (columns[k]==MS::columnName(MS::CORRECTED_DATA) && (columns_p.corrVis_p.isNull() ||
                                                             !columns_p.corrVis_p.isDefined(0))) ||
         (columns[k]==MS::columnName(MS::FLAG) && (columns_p.flag_p.isNull() ||
                                                   !columns_p.flag_p.isDefined(0))) ||
         (columns[k]==MS::columnName(MS::WEIGHT) && (columns_p.weight_p.isNull() ||
                                                     !columns_p.weight_p.isDefined(0))) ||
         (columns[k]==MS::columnName(MS::SIGMA) && (columns_p.sigma_p.isNull() ||
                                                    !columns_p.sigma_p.isDefined(0))) ||
         (columns[k]==MS::columnName(MS::UVW) && (columns_p.uvw_p.isNull() ||
                                                  !columns_p.uvw_p.isDefined(0))) ){
        dataManType="";
      }
          
      if(dataManType.contains("Tiled") &&
         !String(cdesc.dataManagerGroup()).empty()){
        try {      
          ROTiledStManAccessor tacc=ROTiledStManAccessor(thems, 
                                                         cdesc.dataManagerGroup());

          // This is for the data columns, WEIGHT_SPECTRUM and FLAG only.
          if((columns[k] != MS::columnName(MS::WEIGHT)) && 
             (columns[k] != MS::columnName(MS::UVW))){
            // Figure out how many tiles are needed to span the selected channels.
            const IPosition tileShape(tacc.tileShape(startrow));
            Vector<Int> ids;
            chanIds(ids);
            uInt startTile = ids[0] / tileShape[1];
            uInt endTile = ids[ids.nelements() - 1] / tileShape[1];
            uInt cachesize = endTile - startTile + 1;

            // and the selected correlations.
            corrIds(ids);
            startTile = ids[0] / tileShape[0];
            endTile = ids[ids.nelements() - 1] / tileShape[0];
            cachesize *= endTile - startTile + 1;

            // Safer until I know which of correlations and channels varies faster on
            // disk.
            const IPosition hShape(tacc.hypercubeShape(startrow));
            cachesize = hShape[0] * hShape[1] / (tileShape[0] * tileShape[1]);

            tacc.setCacheSize(startrow, cachesize);
          }
          else
            tacc.setCacheSize(startrow, 1);
        }
        catch (AipsError x) {
          //It failed so leave the caching as is.
          continue;
        }
      }
    }
  }
}

void VisIterator::putCol(ScalarColumn<Bool> &column, const Vector<Bool> &array)
{
    column.putColumn(array);
}

void VisIterator::putCol(ArrayColumn<Bool> &column, const Array<Bool> &array)
{
    column.putColumn(array);
}

void VisIterator::putCol(ArrayColumn<Float> &column, const Array<Float> &array)
{
    column.putColumn(array);
}

void VisIterator::putCol(ArrayColumn<Complex> &column, const Array<Complex> &array)
{
    column.putColumn(array);
}

void VisIterator::putCol(ArrayColumn<Bool> &column, const Slicer &slicer, const Array<Bool> &array)
{
    column.putColumn(slicer, array);
}

void VisIterator::putCol(ArrayColumn<Float> &column, const Slicer &slicer, const Array<Float> &array)
{
    column.putColumn(slicer, array);
}

void VisIterator::putCol(ArrayColumn<Complex> &column, const Slicer &slicer, const Array<Complex> &array)
{
    column.putColumn(slicer, array);
}


} //# NAMESPACE CASA - END

