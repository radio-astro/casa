//# CalSolVi2Organizer.h: MS Meta info services in Calibraiton context
//# Copyright (C) 2016
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

#ifndef SYNTHESIS_CALSOLVI2ORGANIZER_H
#define SYNTHESIS_CALSOLVI2ORGANIZER_H

#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/BasicSL/String.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <msvis/MSVis/SimpleSimVi2.h>
#include <mstransform/TVI/ChannelAverageTVI.h>
#include <synthesis/MeasurementComponents/CalibratingVi2.h>


namespace casa { //# NAMESPACE CASA - BEGIN

class VisEquation;

class CalSolVi2Organizer {
  
public:

  // Construct 
  CalSolVi2Organizer();

  // Dtor
  ~CalSolVi2Organizer();

  Int nLayers() const { return factories_.nelements(); };

  // Build and Return a reference to the fully-stacked VI
  vi::VisibilityIterator2& makeFullVI();

  // Count solutions
  Int countSolutions(Vector<Int>& nChunkPerSolve);

  // Add disk-accesing layer factory
  //  void addDiskIO();

  // Add spoofed data layer factory
  void addSimIO();
  void addSimIO(const vi::SimpleSimVi2Parameters& ss);

  // Add calibrating layer factory (one way or another)
  void addCalForSolving(Float calfactor);
  void addCalForSolving(VisEquation& ve);

  // Add chan-averaging layer factory
  void addChanAve(Int chanbin);

  // Add time-averaging layer factory
  void addTimeAve(Float timebin);

private:

  void barf();

  // Add a factory to the internal list
  void appendFactory(vi::ViiLayerFactory* f);

  // Delete everything
  void cleanUp();

  // Derive MS iteration sort order, based on comb*
  void deriveVI2Sort(Block<Int>& sortcols, // Double& iterInterval
		     Bool combobs,Bool combscan,
		     Bool combfld,Bool combspw);

  // Pointers to the various kinds of layer factories we may use
  vi::ViiLayerFactory *ss_, *cal_, *chanave_, *timeave_;

  //vi::SimpleSimVi2LayerFactory *ss_;
  //vi::CalSolvingVi2LayerFactoryByVE *cal_;
  //vi::ChannelAverageTVILayerFactory *chanave_;
  //  AveragingVi2LayerFactory *timeave_;

  Vector<vi::ViiLayerFactory*> factories_;

  vi::VisibilityIterator2* vi_;

};


} //# NAMESPACE CASA - END

#endif


