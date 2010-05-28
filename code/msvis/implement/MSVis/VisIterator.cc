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

namespace casa { //# NAMESPACE CASA - BEGIN

ROVisIterator::ROVisIterator() {}

// const of MS is cast away, but we don't actually change it.
// (just to share code between RO version and RW version of iterator)
ROVisIterator::ROVisIterator(const MeasurementSet &ms,
			     const Block<Int>& sortColumns,
			     Double timeInterval)
  : ROVisibilityIterator(ms,sortColumns,timeInterval),
    useNewSlicer_p(False)
{

  // Initialize multi-slicers with empty slices
  chanSlices_p.resize(numberSpw());
  chanSlices_p.set(Vector<Slice>());
  corrSlices_p.resize(numberPol());
  corrSlices_p.set(Vector<Slice>());

}

ROVisIterator::ROVisIterator(const ROVisIterator& other)
{
    operator=(other);
}

ROVisIterator::~ROVisIterator() {}

ROVisIterator& 
ROVisIterator::operator=(const ROVisIterator& other) 
{
  if (this!=&other) {
    ROVisibilityIterator::operator=(other);

    // TBD: new slicer stuff

  }
  return *this;
}

ROVisIterator & ROVisIterator::operator++(int)
{
  if (!more_p) return *this;
  advance();
  return *this;
}

ROVisIterator & ROVisIterator::operator++()
{
  if (!more_p) return *this;
  advance();
  return *this;
}



void ROVisIterator::updateSlicer()
{

  // Do we need the following here?
  //  if(msIter_p.newMS()){
  //    setTileCache();
  //  }

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

  
  //    cout << "done." << endl;

}

// (Alternative syntax for ROVisIter::chanIds)
Vector<Int>& ROVisIterator::channel(Vector<Int>& chan) const
{ return this->chanIds(chan); }


Vector<Int>& ROVisIterator::chanIds(Vector<Int>& chanids) const
{
  return chanIds(chanids,msIter_p.spectralWindowId());
}

Vector<Int>& ROVisIterator::chanIds(Vector<Int>& chanids, Int spw) const
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

void ROVisIterator::setSelTable()
{
    ROVisibilityIterator::setSelTable();

    // The following code (which uses Table::operator() to create
    // a new RefTable) is computationally expensive. This could
    // be optimized by using the same method as in the
    // VisibilityIterator base class (which is to not create
    // RefTables but instead access the table column directly in
    // msIter_p.table() using VisibilityIterator::selRows_p).

    // Doing so would mean replacing calls like
    //     colSigma.getColumn(newWtSlicer_p,sigmat,True);
    // with
    //     colSigma.getColumnCells(selRows_p,newWtSlicer_p,sigmat,True);
    //
    // However the ArrayColumn class does allow passing both a 
    // Vector<Vector<Slice> > and a RefRows parameter to get-/putColumn.
    // A solution may be to combine selRows_p and newWtSlicer_p
    // to a single object of appropriate type, which can be passed to
    // ArrayColumn::getColumn

    Vector<uInt> rows(curNumRow_p);
    indgen(rows,uInt(curStartRow_p));
    selTable_p=msIter_p.table()(rows);
    this->attachColumns();
}


// Return native correlation _indices_
Vector<Int>& ROVisIterator::corrIds(Vector<Int>& corrids) const
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


Vector<Int>& ROVisIterator::corrType(Vector<Int>& corrTypes) const
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

Cube<Bool>& ROVisIterator::flag(Cube<Bool>& flags) const
{
  if (useNewSlicer_p) colFlag.getColumn(newSlicer_p,flags,True);
  else colFlag.getColumn(flags,True);
  return flags;
}

Vector<Double>& ROVisIterator::frequency(Vector<Double>& freq) const
{
  
  // We need to change some internals transparently
  ROVisIterator* self = const_cast<ROVisIterator*>(this);

  if (!freqCacheOK_p) {
    self->freqCacheOK_p=True;
    const Vector<Double>& chanFreq=msIter_p.frequency();
    Vector<Int> chan;
    self->channel(chan);
    Int nch=chan.nelements();
    self->frequency_p.resize(nch);
    for (Int i=0;i<nch;++i)
      self->frequency_p(i)=chanFreq(chan(i));
  }
  // Always copy to output
  freq.resize(frequency_p.nelements());
  freq=frequency_p;

  return freq;
}

Cube<Complex>& 
ROVisIterator::visibility(Cube<Complex>& vis, DataColumn whichOne) const
{
  if (useNewSlicer_p) getDataColumn(whichOne,newSlicer_p,vis);
  else getDataColumn(whichOne,vis);
  return vis;
}

void ROVisIterator::getDataColumn(DataColumn whichOne, 
				  const Vector<Vector<Slice> >& slices,
				  Cube<Complex>& data) const
{

  //  cout << "Using new getDataColumn. " << endl;
 
  // Return the visibility (observed, model or corrected);
  // deal with DATA and FLOAT_DATA seamlessly for observed data.
  switch (whichOne) {
  case Observed:
    if (floatDataFound_p) {
      Cube<Float> dataFloat;
      colFloatVis.getColumn(slices,dataFloat,True);
      data.resize(dataFloat.shape());
      convertArray(data,dataFloat);
    } else {
      colVis.getColumn(slices,data,True);
    };
    break;
  case Corrected:
    colCorrVis.getColumn(slices,data,True);
    break;
  case Model:
    colModelVis.getColumn(slices,data,True);
    break;
  };
 
};

Matrix<Float>& ROVisIterator::sigmaMat(Matrix<Float>& sigmat) const
{
  if (useNewSlicer_p) colSigma.getColumn(newWtSlicer_p,sigmat,True);
  else {
    sigmat.resize(nPol_p,curNumRow_p);
    colSigma.getColumn(sigmat);
  }
  return sigmat;
}

Matrix<Float>& ROVisIterator::weightMat(Matrix<Float>& wtmat) const
{
  if (useNewSlicer_p) colWeight.getColumn(newWtSlicer_p,wtmat,True);
  else {
    wtmat.resize(nPol_p,curNumRow_p);
    colWeight.getColumn(wtmat);
  }
  return wtmat;
}

Cube<Float>& ROVisIterator::weightSpectrum(Cube<Float>& wtsp) const
{
  if (!colWeightSpectrum.isNull()) {
    if (useNewSlicer_p) colWeightSpectrum.getColumn(newSlicer_p,wtsp,True);
    else {
      wtsp.resize(nPol_p,nChan_p,curNumRow_p);
      colWeightSpectrum.getColumn(wtsp);
    }
  } else {
    wtsp.resize(0,0,0);
  }
  return wtsp;
}

void ROVisIterator::selectChannel(const Vector<Vector<Slice> >& chansel) {
  //  cout << "selectChannel!...vss..." << flush;

  if (chansel.nelements() != uInt(numberSpw()))
    throw(AipsError("Specified channel slices has incorrect number of spws."));
  
  chanSlices_p.resize(numberSpw(),False);
  chanSlices_p=chansel;

  // Enforce use of the new slicer downstream
  useNewSlicer_p=True;
    
  //  cout << "done." << endl;

}

void ROVisIterator::selectCorrelation(const Vector<Vector<Slice> >& corrsel) {

  //  cout << "selectCorrelation!...vvs..." << flush;

  if (corrsel.nelements() != uInt(numberPol()))
    throw(AipsError("Specified correlation slices has incorrect number of polIds."));
  
  corrSlices_p.resize(numberPol(),False);
  corrSlices_p=corrsel;

  // Enforce use of the new slicer downstream
  useNewSlicer_p=True;
    
  //  cout << "done." << endl;

}

Vector<Matrix<Int> >& ROVisIterator::setChanAveBounds(Float factor,
						      Vector<Matrix<Int> >& bounds) {

  if (!useNewSlicer_p) throw(AipsError("Help!"));

  // For every spw...
  bounds.resize(numberSpw());

  // If factor greater than zero, fill the bounds non-trivially
  if (factor>0) {
    
    // Decipher averaging factor
    Int width(1);
    if (factor>1.0) width=Int(factor); // factor supplied in channel units
    
    // Calculate bounds for each spw
    for (Int ispw=0;ispw<numberSpw();++ispw) {
      
      // Number of SELECTED channels PRIOR to averaging
      Int nChan0=numberChan(ispw);
      
      // factor might have been supplied in factional units;
      //  width is then potentially spw-dependent
      if (factor<=1.0)
	width=Int(factor*Float(nChan0));
      
      // Get the selected channel list
      Vector<Int> chans;
      chanIds(chans,ispw);

      // The nominal number of output channels depends on the full
      // range of channels selected (not the number of them)
      //  (will be revised later, if nec.)
      Int nChanOut0((chans(nChan0-1)-chans(0)+1+width)/width);
      
      // Initialize the bounds container for this spw
      Matrix<Int>& currBounds(bounds(ispw));
      currBounds.resize(nChanOut0,4);
      currBounds.set(0);
      
      // Actual output channel count; initialization
      Int nChanOut=1;
      Int lo(chans(0));
      currBounds(0,0)=lo;
      currBounds(0,2)=0;
      for (Int ich=0;ich<nChan0;++ich) 
	if ( (chans(ich)-lo+1)>width ) {
	  currBounds(nChanOut-1,1)=chans(ich-1);   // end of previous
	  currBounds(nChanOut-1,3)=ich-1;
	  lo=currBounds(nChanOut,0)=chans(ich);    // start of next
	  currBounds(nChanOut,2)=ich;    // start of next
	  ++nChanOut;
	}
      currBounds(nChanOut-1,1)=chans(nChan0-1);    // end of final set
      currBounds(nChanOut-1,3)=(nChan0-1);    // end of final set
      
      // contract bounds container, if necessary
      if (nChanOut<nChanOut0)
	currBounds.resize(nChanOut,4,True);
      
    } // ispw


  } // factor > 0

  // Internal reference  (needed?)
  chanAveBounds_p.reference(bounds);

  // Return the bounds Vector reference
  return bounds;

}

Int ROVisIterator::numberChan(Int spw) const {

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


Int ROVisIterator::numberCorr(Int pol) const {

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


VisIterator::VisIterator() {}

VisIterator::VisIterator(MeasurementSet &MS, 
			 const Block<Int>& sortColumns, 
			 Double timeInterval)
  : ROVisIterator(MS, sortColumns, timeInterval)
{}

VisIterator::VisIterator(const VisIterator & other)
{
    operator=(other);
}

VisIterator::~VisIterator() {}

VisIterator& 
VisIterator::operator=(const VisIterator& other)
{
    if (this!=&other) {
	ROVisIterator::operator=(other);
        selTable_p=other.selTable_p;
	RWcolFlag.reference(other.RWcolFlag);
        RWcolFlagRow.reference(other.RWcolFlagRow);
	RWcolVis.reference(other.RWcolVis);
	RWcolFloatVis.reference(other.RWcolFloatVis);
	RWcolModelVis.reference(other.RWcolModelVis);
	RWcolCorrVis.reference(other.RWcolCorrVis);
	RWcolWeight.reference(other.RWcolWeight);
        RWcolWeightSpectrum.reference(other.RWcolWeightSpectrum);
	RWcolSigma.reference(other.RWcolSigma);
	RWcolImagingWeight.reference(other.RWcolImagingWeight);
    }
    return *this;
}

VisIterator & VisIterator::operator++(int)
{
  if (!more_p) return *this;
  advance();
  return *this;
}

VisIterator & VisIterator::operator++()
{
  if (!more_p) return *this;
  advance();
  return *this;
}

void VisIterator::attachColumns()
{
  ROVisibilityIterator::attachColumns();
  //todo: should cache this (update once per ms)
  const ColumnDescSet& cds=selTable_p.tableDesc().columnDescSet();
  if (cds.isDefined(MS::columnName(MS::DATA))) {
      RWcolVis.attach(selTable_p,MS::columnName(MS::DATA));
  };
  if (cds.isDefined(MS::columnName(MS::FLOAT_DATA))) {
    floatDataFound_p=True;
    RWcolFloatVis.attach(selTable_p,MS::columnName(MS::FLOAT_DATA));
  } else {
    floatDataFound_p=False;
  };
  if (cds.isDefined("MODEL_DATA")) 
    RWcolModelVis.attach(selTable_p,"MODEL_DATA");
  if (cds.isDefined("CORRECTED_DATA")) 
    RWcolCorrVis.attach(selTable_p,"CORRECTED_DATA");
  RWcolWeight.attach(selTable_p,MS::columnName(MS::WEIGHT));
  if (cds.isDefined("WEIGHT_SPECTRUM"))
    RWcolWeightSpectrum.attach(selTable_p,"WEIGHT_SPECTRUM");
  RWcolSigma.attach(selTable_p,MS::columnName(MS::SIGMA));
  RWcolFlag.attach(selTable_p,MS::columnName(MS::FLAG));
  RWcolFlagRow.attach(selTable_p,MS::columnName(MS::FLAG_ROW));
  if (cds.isDefined("IMAGING_WEIGHT"))
    RWcolImagingWeight.attach(selTable_p,"IMAGING_WEIGHT");
}

void VisIterator::setFlagRow(const Vector<Bool>& rowflags)
{
  RWcolFlagRow.putColumn(rowflags);
};

void VisIterator::setVis(const Cube<Complex>& vis, DataColumn whichOne)
{
  
  if (useNewSlicer_p) putDataColumn(whichOne,newSlicer_p,vis);
  else putDataColumn(whichOne,vis);

}

void VisIterator::setFlag(const Cube<Bool>& flags)
{
  if (useNewSlicer_p) RWcolFlag.putColumn(newSlicer_p,flags);
  else RWcolFlag.putColumn(flags);
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
  if (useNewSlicer_p) RWcolWeight.putColumn(newWtSlicer_p,weightMat);
  else RWcolWeight.putColumn(weightMat);
}

void VisIterator::setWeightSpectrum(const Cube<Float>& weightSpectrum)
{
  if (!colWeightSpectrum.isNull()) {
    if (useNewSlicer_p) RWcolWeightSpectrum.putColumn(newSlicer_p,weightSpectrum);
    else RWcolWeightSpectrum.putColumn(weightSpectrum);
  }
  else 
    throw(AipsError("Can't set WEIGHT_SPECTRUM -- it doesn't exist!"));
}




void VisIterator::putDataColumn(DataColumn whichOne,
				const Vector<Vector<Slice> >& slices,
				const Cube<Complex>& data)
{
  // Set the visibility (observed, model or corrected);
  // deal with DATA and FLOAT_DATA seamlessly for observed data.
  switch (whichOne) {
  case Observed:
    if (floatDataFound_p) {
      Cube<Float> dataFloat=real(data);
      RWcolFloatVis.putColumn(slices,dataFloat);
    } else {
      RWcolVis.putColumn(slices,data);
    };
    break;
  case Corrected:
    RWcolCorrVis.putColumn(slices,data);
    break;
  case Model:
    RWcolModelVis.putColumn(slices,data);
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
    if (floatDataFound_p) {
      Cube<Float> dataFloat=real(data);
      RWcolFloatVis.putColumn(dataFloat);
    } else {
      RWcolVis.putColumn(data);
    };
    break;
  case Corrected:
    RWcolCorrVis.putColumn(data);
    break;
  case Model:
    RWcolModelVis.putColumn(data);
    break;
  };
};  

Vector<uInt>& ROVisIterator::rowIds(Vector<uInt>& rowids) const
{
  rowids.resize(curNumRow_p);
  rowids=selTable_p.rowNumbers();
  return rowids;
}



} //# NAMESPACE CASA - END

