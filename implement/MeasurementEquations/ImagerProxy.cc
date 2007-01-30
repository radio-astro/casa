//# ImagerProxy.cc: 
//# Copyright (C) 2006
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
//# $Id: ImagerProxy.cc,v 1.1 2007/01/29 04:18:28 mmarquar Exp $
#include <casa/aips.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Exceptions/Error.h>
#include <casa/Arrays/Vector.h>
#include <measures/Measures/MeasureHolder.h>
#include <synthesis/MeasurementEquations/Imager.h>
#include <synthesis/MeasurementEquations/ImagerMultiMS.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Containers/Record.h>
#include <casa/BasicSL/String.h>
#include <tables/Tables/Table.h>
#include <synthesis/MeasurementEquations/Imager.h>

#include <synthesis/MeasurementEquations/ImagerProxy.h>

//#include <casa/namespace.h>


namespace casa {

ImagerProxy::ImagerProxy()
{
  itsImager = new ImagerMultiMS();
}

ImagerProxy::ImagerProxy(const String& thems, Bool compress)
{
  const Table::TableOption openOption = Table::Update;
  MeasurementSet ms = MeasurementSet(thems,TableLock(TableLock::UserLocking),
				     openOption);
  itsImager = new Imager(ms, compress);
}

ImagerProxy::~ImagerProxy()
{
  if ( itsImager != 0 ) delete itsImager;
}

Bool ImagerProxy::setimage(Int nx, Int ny,
			   const Record& cellx, const Record& celly,
			   const String& stokes,
			   Bool doShift,
			   const Record& phaseCenter, 
			   const Record& shiftx, const Record& shifty,
			   const String& mode, Int nchan,
			   Int start, Int step,
			   const Record& mStart, 
			   const Record& mStep,
			   const Vector<Int>& spectralwindowids,
			   Int fieldid,
			   Int facets,
			   const Record& distance,
			   Float paStep, Float pbLimit)
{
  QuantumHolder qh0,qh1,qh2,qh3,qh4;
  String err;
  if (!qh0.fromRecord(err, cellx)) throw AipsError(err);
  if (!qh1.fromRecord(err, celly)) throw AipsError(err);
  if (!qh2.fromRecord(err, shiftx)) throw AipsError(err);
  if (!qh3.fromRecord(err, shifty)) throw AipsError(err);
  if (!qh4.fromRecord(err, distance)) throw AipsError(err);
  MeasureHolder mh0,mh1,mh2;
  if (!mh0.fromRecord(err, phaseCenter)) throw AipsError(err);
  if (!mh1.fromRecord(err, mStart)) throw AipsError(err);
  if (!mh2.fromRecord(err, mStep)) throw AipsError(err);

  Quantity q0 = qh0.asQuantity();
  Quantity q1 = qh1.asQuantity();
  Quantity q2 = qh2.asQuantity();
  Quantity q3 = qh3.asQuantity();
  Quantity q4 = qh4.asQuantity();
  MDirection md0(mh0.asMDirection());
  MRadialVelocity mrv1 = mh1.asMRadialVelocity();
  MRadialVelocity mrv2 = mh2.asMRadialVelocity();

  return itsImager->setimage(nx, ny, q0, q1,
			     stokes, doShift,
                             md0,
			     q2, q3,
			     mode,nchan,
                             start, step, mrv1,
			     mrv2, spectralwindowids, 
                             fieldid, facets, q4);
}

Bool ImagerProxy::setdata(const String& mode, const Vector<Int>& nchan,
			  const Vector<Int>& start, const Vector<Int>& step,
			  const Vector<Int>& spectralwindowids,
			  const Vector<Int>& fieldids,
			  const String& msSelect, const String& msname)
  
{
  return itsImager->setDataPerMS(msname, mode, nchan, start, step, 
				 spectralwindowids, fieldids,
				 msSelect);
}

Bool ImagerProxy::setoptions(const String& ftmachine, Int cache, 
			       Int tile,
			       const String& gridfunction, 
			       const Record& mLocation,
			       Float padding, Bool usemodelcol, 
			       Int wprojplanes,
			       const String& epJTableName,
			       Bool applyPointingOffsets,
			       Bool doPointingCorrection,
			       const String &cfCache)
{
  MeasureHolder mh;
  String err;
  if (!mh.fromRecord(err, mLocation)) throw AipsError(err);
  return itsImager->setoptions(ftmachine, cache, tile, gridfunction, 
                               mh.asMPosition(), padding,usemodelcol,
			       wprojplanes,
                               epJTableName,
			       applyPointingOffsets,doPointingCorrection,
                               cfCache);
}

Bool ImagerProxy::weight(const String& type, const String& rmode,
			 const Record& noise, const Double robust,
			 const Record& fieldofview,
			 const Int npixels)
{
  QuantumHolder qh0,qh1;
  String err;
  if (!qh0.fromRecord(err, noise)) throw AipsError(err);
  if (!qh1.fromRecord(err, fieldofview)) throw AipsError(err);

  return itsImager->weight(type, rmode, qh0.asQuantity(), robust, 
			   qh1.asQuantity(), npixels);
}

Bool ImagerProxy::makeimage(const String& type, const String& imageName)
{
  return itsImager->makeimage(type, imageName, "");
}


Bool ImagerProxy::setmfcontrol(Float cyclefactor,
			       Float cyclespeedup,
			       Int stoplargenegatives, 
			       Int stoppointmode,
			       const String& scaleType,
			       Float minPB,
			       Float constPB,
			       const Vector<String>& fluxscale)
{
  return itsImager->setmfcontrol(cyclefactor, cyclespeedup,
                                 stoplargenegatives, stoppointmode,
                                 scaleType, minPB, constPB, fluxscale);
}

Bool ImagerProxy::setscales(const String& scaleMethod,
			    Int inscales,
			    const Vector<Float>& userScaleSizes)
{
  return itsImager->setscales(scaleMethod, inscales, userScaleSizes);
}

Bool ImagerProxy::clean(const String& algorithm,
			Int niter, 
			Float gain,
			const Record& threshold,//quant 
			const Bool displayProgress, 
			const Vector<String>& model, const Vector<Bool>& fixed,
			const String& complist,
			const Vector<String>& mask,
			const Vector<String>& image,
			const Vector<String>& residual)
{
  QuantumHolder qh;
  String err;
  if (!qh.fromRecord(err, threshold)) throw AipsError(err);
  return itsImager->clean(algorithm, niter, gain, qh.asQuantity(), 
			  displayProgress, 
			  model, fixed, complist, mask, image, residual);
}

Bool ImagerProxy::filter(const String& type, const Record& bmaj,
			 const Record& bmin, const Record& bpa)
{
  QuantumHolder qh0,qh1,qh2,qh3;
  String err;
  if (!qh0.fromRecord(err, bmaj)) throw AipsError(err);
  if (!qh1.fromRecord(err, bmin)) throw AipsError(err);
  if (!qh2.fromRecord(err, bpa)) throw AipsError(err);
  
  return itsImager->filter(type, qh0.asQuantity(), qh1.asQuantity(),
			   qh2.asQuantity());
}


}
