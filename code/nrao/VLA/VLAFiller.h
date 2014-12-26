//# VLAFiller.h: 
//# Copyright (C) 1999,2000,2001,2003
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
//# $Id$

#ifndef NRAO_VLAFILLER_H
#define NRAO_VLAFILLER_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/Record.h>
#include <casa/Logging/LogIO.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <measures/Measures/MBaseline.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/Muvw.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/Stokes.h>
#include <tables/DataMan/TiledDataStManAccessor.h>
#include <nrao/VLA/VLAEnum.h>
#include <nrao/VLA/VLAFilterSet.h>
#include <nrao/VLA/VLALogicalRecord.h>
    
#include <casa/namespace.h>
namespace casa { //# NAMESPACE CASA - BEGIN
class Path;
class MPosition;
template <class T> class Vector;
} //# NAMESPACE CASA - END

struct IterationStatus;

// <summary>Functions to convert from VLA archive format to a MeasurementSet</summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> SomeClass
//   <li> SomeOtherClass
//   <li> some concept
// </prerequisite>
//
// <etymology>
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
// <templating arg=T>
//    <li>
//    <li>
// </templating>
//
// <thrown>
//    <li>
//    <li>
// </thrown>
//
// <todo asof="yyyy/mm/dd">
//   <li> add this feature
//   <li> fix this bug
//   <li> start discussion of this possible extension
// </todo>

class VLAFiller: public MSColumns
{
public:
  // Construct a VLAFiller object that will data from the specified input, and
  // writes it to the supplied MeasurementSet.
  // We set the default tolerance for frequency to be 6 times of the
  // channel width and also give user the ability to pass in a tolerance
  // for frequency into vlafillerfromdisk(). The default tolerance of frequency
  // works for dataset G192. But for dataset NGC7538, one has to give a tolerance
  // as larger as 60 times its channel width ( 60000000Hz ). For other dataset, the
  // user has to try out the proper tolerance.
  VLAFiller(MeasurementSet& output, VLALogicalRecord& input, Double freqTolerance=0.0, Bool autocorr=False, const String& antnamescheme="new", const Bool& applyTsys=True);

  // Does nothing special
  ~VLAFiller();

  // Set the filter that will be used to restrict which input records get
  // copied to the output MS.
  void setFilter(const VLAFilterSet& filter);

  // fill the supplied MeasurementSet from the the supplied VLABuffer. The
  // number/type of messages describing the progress sent to the logger are
  // controlled using the verbose argument. If verbose is -1 or less then no
  // messages are sent to the logger. If verbose is zero then a summary is sent
  // to the logger just before this function completes. If verbose is one then
  // a message is sent for every record copied to the output MS. If verbose is
  // two a message is sent every second record and so on.
  void fill(Int verbose=1);

  // Construct an empty MeasurementSet with the supplied table name. Throw
  // an exception (AipsError) if the specified Table already exists unless the
  // overwrite argument is set to True.
  static MeasurementSet emptyMS(const Path& tableName, 
 				const Bool overwrite=False);
  
  // Open the MeasurementSet with the supplied name. Throw an exception
  // (AipsError) if the specified Table does not exist. By default the Table is
  // opened for read/write access. The Type/SubType MUST be "Measurement
  // Set/VLA" ie., the MS must have been created with this class. Otherwise an
  // exception is thrown.
  static MeasurementSet openMS(const Path& tableName, 
 			       const Bool readonly=False);
  
  // Return an empty MeasurementSet with the supplied table name. Creates a
  // empty measurement set unless one with the specified name already exists
  // and overwrite is False. Then it opens the existing measurement set for
  // read/write access.
  static MeasurementSet getMS(const Path& tableName, 
			      const Bool overwrite=False);
  
  // Set the stop Parameters for filling
   void setStopParams(String &, String &);
private:
  // Read one record from the input and copies it to the MS if it is not
  // filtered out. Returns False if a record could not be read.
  Bool fillOne();

  // Send to the logger info about how the filler is going
  void logCurrentRecord(IterationStatus& counts);
  void logChanges(IterationStatus& counts);

  // Send to the logger info about how much data is in the (sub)tables
  void summarise();

  //# Add an entry to the antenna subtable
  uInt addAntenna(const MPosition& antennaPos, uInt whichAnt);

  //# Add an entry to the feed subtable
  void addFeed(uInt whichAnt);

  //# Add an entry to the field subtable
  uInt addField(const MDirection& refDir);

  //# Add an entry to the pointing subtable
  uInt addPointing(const MDirection& antennaDir,
		   const MDirection& fieldDir, uInt whichAnt);
  // # Add an entry to the doppler dubtable
  uInt addDoppler( const VLAEnum::CDA cda ); 
  //# Add an entry to the spectral-window subtable
  uInt addSpectralWindow(const VLAEnum::CDA cda, const MFrequency& refFreq,
			 const uInt nChan,
			 const Double bandwidth,
			 const uInt ifChain);

  //# Add an entry to the polarization subtable
  uInt addPolarization(const Vector<Stokes::StokesTypes>& pol);

  //# Add an entry to the data description subtable
  uInt addDataDescription(uInt spwId, uInt polId);

  //# Add an entry in the source subtable
  uInt addSource(const MDirection& dir );

  //# Add a hypercube to the columns that use the Tiled Data Storage manager.
  void addHypercubes(uInt nPol, uInt nChan);

  //# Add the specified number of rows tp the specified hypercubes
  void extendHypercubes(const Block<uInt>& nPol, const Block<uInt>& nChan, 
			uInt rows);
  
  //# Reconcile duplicate field names
  void fixFieldDuplicates(MSField& msFld);

  //# Determine whether to stop the fill for on-line filling
  String projectCode;
  MVEpoch stopTime;
  Bool checkStop;
  Bool fillStarted;
  Bool stopFilling(VLALogicalRecord &);
  //# pol index RR=0, RL=1, LR=2, LL needed just for index data
  Int polIndexer(Stokes::StokesTypes& stokes);
  //# Contains a logical record
  VLALogicalRecord itsRecord;

  //# Only logical records that match the specified criteria will be used.
  VLAFilterSet itsInputFilters;

  //# The output MS
  MeasurementSet itsMS;
  
  //# The reference frame of the observation is cached here and used when
  //# conversions from the observed frame to the one used in the MS.
  MeasFrame itsFrame;
  //# The reference frame of the observation whose spw is identified different 
  //# from what in the SPECTRAL_WINDOWS table and been added in. So the current
  //# length of Vector equal the number of row in table SPECTRAL WINDOW( no use).
  //# Vector<MeasFrame> theirFrames;
  //# This is the direction reference for the Measurement set. It is cached
  //# here.
  MDirection::Types itsMSDirType;
  //# This is the reference frame of the last record copied. Initially it is
  //# set to a meaningless value.
  MDirection::Types itsDirType;
  //# This converter is used to convert from the direction types on the archive
  //# to the one in the Measurement set. Its is cached to prevent it
  //# continually having to be recreated. It is never used if all the data on
  //# the archive and all the data in the MS have the same direction type.
  MDirection::Convert itsDirCtr;

  //# This converter is used to convert from the direction types on the archive
  //# to Az, El. It is only needed for holography data.
  MDirection::Convert itsAzElCtr;

  //# This converter is used to convert the UVW coordinates from the type on
  //# the archive to the one in the Measurement set. It is cached to
  //# prevent it continually having to be recreated. It is never used if all
  //# the data on the archive and all the data in the MS have the same
  //# direction type.
  Muvw::Convert itsUvwCtr;

  //# This converter is used to convert from the channel frequency types on the
  //# archive to the one in the Measurement set. Its is cached hear to prevent
  //# it continually having to be recreated. It is only used if
  //# data in the archive was observed with online Doppler tracking.
  MFrequency::Convert itsFreqCtr;

  //# This converter is used to the VLA baselines which are in the HADEC frame
  //#to the ITRF frame so that they can be properly added with the VLA reference
  //# position, which is also in the ITRF frame.
  MBaseline::Convert itsBlCtr;

  //# The field Id for the most recently copied record. Negative if no record
  //# has been copied.
  Block<Int> itsFldId;

  //# The antenna Id's for all the antennas in the most recently copied
  //# record. Zero length if no record has been copied.
  Block<Int> itsAntId;

  //# The spectral window Id's for all the CDA's in the most recently copied
  //# record. Always of length 4. Contains -1 is the CDA is unused.
  Block<Int> itsSpId;

  //# The polarization Id for the most recently copied record. Negative if no
  //# record has been copied. There is one for each Spectral Id.
  Block<Int> itsPolId;

  //# The data description Id's for the most recently copied record. Negative
  //# if no record has been copied.
  Block<Block<Int> > itsDataId;

  //# The length of these blocks is set to the maximum number of
  //# subarrays(4). The itsNewScan block is true if the field, spectral window
  //# etc. has changed and the itsScan Block indicates the scan number.
  Bool itsNewScan;
  Block<Int> itsScan;
  //# A string, containing projkect names, that is accumulated when filling and
  //# written to the OBSERVATION subtable.
  String itsProject;
  //# The place to log messages
  LogIO itsLog;

  //# These data members needed because of the use of the TiledData Storage
  //# Manager. It may be possible to remove these when the TiledShape storage
  //# manager can be used (the holdup is currently in VisSet).
  TiledDataStManAccessor itsDataAcc;
  Record itsTileId;
  TiledDataStManAccessor itsSigmaAcc;
  TiledDataStManAccessor itsFlagAcc;
  TiledDataStManAccessor itsImagingWeightAcc;
  TiledDataStManAccessor itsModDataAcc;
  TiledDataStManAccessor itsCorrDataAcc;
  TiledDataStManAccessor itsChanFlagAcc;
  Block<IPosition> itsDataShapes;
  Double itsFreqTolerance;
  Bool itsApplyTsys;
  Bool itsNewAntName;
  Bool itsKeepAutoCorr;
  Bool itsEVLAisOn;
  Bool itsInitEpoch;
  Bool itsRevBeenWarned;
  
};
#endif
