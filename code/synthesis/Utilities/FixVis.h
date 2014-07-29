//# FixVis.h: Does for MSes various fixes which do not involve calibrating.
//# Copyright (C) 2008
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
//# $Id$
//#
#ifndef SYN_FIXVIS_H
#define SYN_FIXVIS_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <synthesis/MSVis/MSUVWGenerator.h>

// UVWMachine Does rotation, including B1950 <-> J2000, refocusing, and maybe
// even SIN <-> (-)NCP reprojection of existing UVWs, but it does not generate
// UVWs from an antenna table.
// FTMachine::rotateUVW(Matrix<Double>& uvw, Vector<Double>& dphase,
//                      const VisBuffer& vb)
#include <synthesis/TransformMachines/FTMachine.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class MeasurementSet;
class LogIO;

// <summary>Performs for MeasurementSets various fixes which do not involve calibrating.
// This includes (in order of implementation):
//   1. Generating and inserting (u, v, w)s into a MS that may or may not
//      already have them.  Includes antenna offsets when known.
//   2. Correcting for differential aberration.
//   3. Changing the phase tracking center.
//   4. Changing the equinox (B1950 -> J2000).
//   5. (maybe never) Changing the projection, i.e. SIN <-> (-)NCP.
// </summary>
// <use visibility=export>
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <prerequisite>
//   <li> <linkto class=MeasurementSet>MeasurementSet</linkto>
// </prerequisite>
//
// <etymology>
// It is a variation on the UVFIX task of AIPS.
// </etymology>
//
// <synopsis>
// (u, v, w)s are needed for imaging, but some data sets may not come with them
// included.
//
// FixVis can also be used to shift the tangent point, and correct for
// differential aberration.
// </synopsis>
//
// <example>
// <srcBlock>
//     MS inMS(fileName);
//     FixVis uvwfixer(inMS);
//     uvwfixer.setDataDescriptionIds(ddIds);
//     uvwfixer.setFields(fieldIds);
//     uvwfixer.fixuvw();
// </srcBlock>
// A <src>FixVis</src> object is constructed 
// and the baselines are calculated and stored in inMS.
// </example>
//
// <motivation>
// Currently (10/30/2008), ASDMs from either the ALMA Test Facility or the
// EVLA do not come with (u, v, w)s, and need to be processed with the UVFIX
// task in AIPS.  It would be preferable to process them entirely inside CASA.
// </motivation>
//
// <todo asof="">
// </todo>

// class FixVis;

class FixVis : public FTMachine
{
public:
// Constructor
  FixVis (MeasurementSet& ms, const String& dataColName=String("all"));

// Assignment (only copies reference to MS, need to reset selection etc)
  FixVis& operator=(FixVis& other);

// Destructor
  ~FixVis();

// Set the required field Ids and return the # of selected fields.
  uInt setFields(const Vector<Int>& fieldIds);

  //// Select by observationIDs (problematic at best)
  //void setObsIDs(const String& obs) {obsString_p = obs;}

  // Specifies new phase tracking centers for the selected fields
  void setPhaseDirs(const Vector<MDirection>& phaseDirs);

  // Specifies distances for each selected field according to distances, which
  // must be in m and the same order as the Vector given to setFields.  Throws
  // an exception if distances.nelements() != nsel_p.
  // Because of the way refocus() works, zeroes are ignored (no refocusing
  // done), but negative distances are accepted!
  void setDistances(const Vector<Double>& distances);

  // Calculate the (u, v, w)s and store them in ms_p.
  Bool calc_uvw(const String& refcode, const Bool reuse=true);

  // Convert the UVW column to a new reference frame by rotating the old
  // baselines instead of calculating fresh ones.
  void rotateUVW(const MDirection &indir, const MDirection::Ref& newref);

  // For things like rotation, differential aberration correction, etc., when
  // there already is a UVW column, using FTMachine.  Returns true if _any_
  // fields are modified.
  Bool fixvis(const String& refcode);
  virtual void setMiscInfo(const Int qualifier){(void)qualifier;};
  virtual void ComputeResiduals(VisBuffer&, Bool //useCorrected
                                ) {}
  virtual String name() const { return "FixVis";};

private:
  // Interpret field indices (MSSelection)
  Vector<Int> getFieldIdx(const String& fields);
  
  // Makes sure msc_p is ready, and returns false if it fails.
  Bool ready_msc_p();
  
  // Convert the directions (phase tracking centers, + DELAY_DIR and
  // REFERENCE_DIR if they start in the same frame) in the FIELD table to
  // newFrame.  Note that each direction column in the table only allows one
  // reference frame for the entire column, so all fields must share the same
  // frame.  Calls ready_msc_p() as a side effect.
  void convertFieldDirs(const MDirection::Types outType);

  // Private worker function for convertFieldDirs().
  void convertFieldCols(MSFieldColumns& msfcs,
                        const MDirection::Ref& newFrame,
                        const Bool doAll3);

  // Calls ready_msc_p() as a side effect.
  Bool makeSelection(const Int selectedField);
  
  // Does phase tracking center and distance adjustment for mssel_p.
  void processSelected(uInt numInSel);

  // FTMachine declares a LOT of pure virtual functions which FixVis does not
  // need.  They are declared as no-ops here for now.
  ImageInterface<Complex>& getImage(Matrix<float>&, Bool) {return *image;}
  virtual void normalizeImage(Lattice<Complex>&,// skyImage,
			      const Matrix<Double>&,// sumOfWts,
			      Lattice<Float>&,// sensitivityImage,
			      Bool //fftNorm
                              )
  {throw(AipsError("normalizeImage::normalizeImage() called"));}

  void getWeightImage(ImageInterface<float>&, Matrix<float>&) {}
  void get(VisBuffer&, Int) {}
  void put(const VisBuffer& , Int , Bool , FTMachine::Type) {};

  // Bool getRestFreq(Vector<Double>& restFreq, const Int spw, const Int fldID);
  //  void setObsInfo(ObsInfo& obsinfo);

  void ok();
  void init();

  // Initialize transform to Visibility plane using the image
  // as a template.
  void initializeToVis(ImageInterface<Complex>& image,
		       const VisBuffer& vb);
  
  // Finalize transform to Visibility plane.
  void finalizeToVis();

  // Initialize transform to Sky plane: initializes the image
  void initializeToSky(ImageInterface<Complex>& image,  Matrix<Float>& weight,
		       const VisBuffer& vb);
  
  // Defined here only because FTMachine declares it purely virtual.
  void finalizeToSky() {}

  // TODO?: trackDir.
  Bool setImageField(const Int fieldid,
                     const Bool dotrackDir=false //, const MDirection& trackDir
                     );

  Bool lock();
  void unlock();

  // Log functions and variables
  LogIO sink_p;
  LogIO& logSink() {return sink_p;}

  // Initialized in ctor.  Make sure the order there matches with the order here.
  MeasurementSet ms_p;			// Input/Output MS
  MSColumns      *msc_p;		// Ptr. to columns of mssel_p.
  uInt           nsel_p;		// Number of selected fields.
  uInt           nAllFields_p;	        // The total # of fields in the MS.
  const uInt       npix_p;              // Not that there are any real pixels.
  const IPosition  cimageShape_p;       // No image will be made, but
  const IPosition  tileShape_p;         // the coords are stored in a fake one.
  const TiledShape tiledShape_p;
  Bool             antennaSel_p;        // Is selection being done by antenna?
  Bool             freqFrameValid_p;    // Freq frame is good and valid
                                        // conversions can be done (or not)
  //String           obsString_p;       // obsID selection
  Vector<Int>      antennaId_p;         // MSSelection::indexExprStr() doesn't
                                        // work with Vector<uInt>.
  Vector<String>   antennaSelStr_p;
  Vector<Double>   distances_p;	        // new distances (m) for each selected
                                        // field
  Vector<MS::PredefinedColumns> dataCols_p;
  uInt             nDataCols_p;
  uInt             nchan_p;
  Vector<Int>      spectralwindowids_p;
  uInt             lockCounter_p;
  
  // Not initialized in ctor.
  MeasurementSet     mssel_p;           // The selected part of ms_p.
  ObsInfo            latestObsInfo_p;  
  Vector<Int>        DDIds_p;	        // DataDescription Ids to process
  Vector<Int>        FieldIds_p;        // Field Ids to process
  Vector<MDirection> phaseDirs_p;       // new phase centers for each selected
                                        // field
  Int                nSpw_p;	        // Number of spws
  MPosition          mLocation_p;
  Bool               doTrackSource_p;
  Int                fieldid_p;
};
  
} //# NAMESPACE CASA - END

#endif
