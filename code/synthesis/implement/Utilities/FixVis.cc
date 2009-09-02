#include <synthesis/Utilities/FixVis.h>
#include <msvis/MSVis/MSUVWGenerator.h>
#include <msvis/MSVis/SubMS.h>
#include <measures/Measures/MeasTable.h>
#include <measures/Measures/UVWMachine.h>
#include <casa/Logging/LogIO.h>
#include <casa/Exceptions/Error.h>
#include <casa/Quanta/MVAngle.h>
#include <coordinates/Coordinates/Projection.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/ObsInfo.h>
#include <images/Images/PagedImage.h>           // Used to pass coords
#include <images/Images/ImageInfo.h>            // to FTMachine.
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSDopplerUtil.h>
#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <ms/MeasurementSets/MSSelectionTools.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisSet.h>
//#include <msvis/MSVis/VisSetUtil.h>
#include <casa/BasicSL/String.h>	// for parseColumnNames()

namespace casa {

FixVis::FixVis(MeasurementSet& ms, const String& dataColName) :
  FTMachine(),
  ms_p(ms),
  nsel_p(0),
  nAllFields_p(1),
  npix_p(32),
  cimageShape_p(4, npix_p, npix_p, 1, 1), // Can we get away with
  tileShape_p(4, npix_p, npix_p, 1, 1),   // (1, 1, 1, 1)?  Does it matter?
  tiledShape_p(cimageShape_p, tileShape_p),
  antennaSel_p(false),
  freqFrameValid_p(false)
{
  logSink() << LogOrigin("FixVis", "") << LogIO::NORMAL3;
  antennaId_p.resize();
  antennaSelStr_p.resize();
  distances_p.resize();
  dataColNames_p = SubMS::parseColumnNames(dataColName);
  nDataCols_p = dataColNames_p.nelements();

  // To use FTMachine, the image has to be set up with coordinates, even
  // though no image will be made.
	  
  //imageNchan_p = 1;        // for MFS.
  //imageStart_p = 0
  //imageStep_p  = 1
  //destroySkyEquation();    
  // npol_p = 1;

  // nchan we need to get rid of one of these variables 
  nchan_p = 1; // imageNchan_p;

  spectralwindowids_p.resize(ms_p.spectralWindow().nrow());
  indgen(spectralwindowids_p);

  lockCounter_p = 0;
}
  
// // Assignment (only copies reference to MS, need to reset selection etc)
// FixVis::FixVis& operator=(FixVis::FixVis& other)
// {
//   ms_p = other.ms_p;
//   DDIds_p = other.DDIds_p;
//   FieldIds_p = other.FieldIds_p;
//   nSpw_p = other.nSpw_p;
// }
  
// Destructor
FixVis::~FixVis()
{
}

// Interpret field indices (MSSelection)
Vector<Int> FixVis::getFieldIdx(const String& fields)
{
  MSSelection mssel;

  mssel.setFieldExpr(fields);
  return mssel.getFieldList(&ms_p);
}

// Set the required field Ids
uInt FixVis::setField(const String& field)
{
  Vector<Int> fldIndices(getFieldIdx(field));
  
  FieldIds_p = getFieldIdx(field);
  nsel_p = check_fields();
  logSink() << LogOrigin("FixVis", "setField")
	    << LogIO::NORMAL
	    << FieldIds_p << " -> " << nsel_p << " selected fields." 
	    << LogIO::POST;
  return nsel_p;
}
  
uInt FixVis::setFields(const Vector<Int>& fieldIds)
{
  FieldIds_p = fieldIds;
  nsel_p = check_fields();
  logSink() << LogOrigin("FixVis", "setFields")
	    << LogIO::NORMAL
	    << "Selected " << nsel_p << " fields: " << FieldIds_p
	    << LogIO::POST;
  return nsel_p;
}

void FixVis::setPhaseDirs(const Vector<MDirection>& phaseDirs,
                          const Vector<Int>& fieldIds)
{
  phaseDirs_p = phaseDirs;

  //TODO
  // Update the FIELD table with the new phase directions.
  // Do not change REFERENCE_DIR since it is supposed to be the original
  // PHASE_DIR.  In particular, if the POINTING table is invalid (i.e. VLA),
  // the REFERENCE_DIR seems like the best substitute for the pointing
  // direction.  Do not touch DELAY_DIR either.

  // Do a consistency check between fieldIds and FieldIds_p.
  logSink() << LogOrigin("FixVis", "setPhaseDirs");
  uInt n2change = phaseDirs.nelements();
  if(n2change != fieldIds.nelements() || n2change != nsel_p)
    logSink() << LogIO::EXCEPTION
              << "Mismatch between the number of new directions and the fields to change"
              << LogIO::POST;
  
  Int fldId;
  for(uInt i = 0; i < n2change; ++i){
    fldId = fieldIds[i];
    if(fldId < 0 || fldId >= static_cast<Int>(nAllFields_p) || 
       FieldIds_p[fldId] < 0)
      logSink() << LogIO::EXCEPTION
                << "A new phase tracking center was specified for an unselected field"
                << LogIO::POST;
  }
}

// const ROArrayMeasColumn<MDirection>& FixVis::getPhaseDirs() const
// {
//   ROMSFieldColumns msField(msc_p->field());

//   return msField.phaseDirMeasCol();
// }


void FixVis::convertFieldDirs(const MDirection::Types outType)
{
  logSink() << LogOrigin("FixVis", "convertFieldDirs");

  // Note that the each direction column in the FIELD table only allows one
  // reference frame for the entire column, but polynomials can be assigned on
  // a row-wise basis for objects moving in that frame.

  MSColumns msc(ms_p);
  MSFieldColumns& msfcs(msc.field());
  ArrayMeasColumn<MDirection> msPhaseDirCol(msfcs.phaseDirMeasCol());

  // There is no physical or known programming need to change the delay and
  // reference direction frames as well, but for aesthetic reasons we keep them
  // all in the same frame if they start in the same frame.
  ArrayMeasColumn<MDirection> msDelayDirCol(msfcs.delayDirMeasCol());
  ArrayMeasColumn<MDirection> msReferenceDirCol(msfcs.referenceDirMeasCol());
  Bool doAll3 = (msPhaseDirCol.getMeasRef() == msDelayDirCol.getMeasRef()
                 && msPhaseDirCol.getMeasRef() == msReferenceDirCol.getMeasRef());
  
  // Setup conversion machines.
  // Set the frame - choose the first antenna. For e.g. VLBI, we
  // will need to reset the frame per antenna
  mLocation_p = msc.antenna().positionMeas()(0);

  // Expect problems if a moving object is involved!
  mFrame_p = MeasFrame(msfcs.timeMeas()(0), mLocation_p);

  //MDirection::Ref startref(msPhaseDirCol.getMeasRef());
  // If the either of the start or destination frames refers to a finite
  // distance, then the conversion has to be done in two steps:
  // MDirection::Convert start2app(msPhaseDirCol(0), MDirection::APP);
  // 
  // Otherwise the conversion can be done directly.
  Bool haveMovingFrame = (msPhaseDirCol.getMeasRef().getType() > MDirection::N_Types ||
                          outType > MDirection::N_Types);

  const MDirection::Ref newFrame(haveMovingFrame ? MDirection::APP : outType,
                                 mFrame_p);

  convertFieldCols(msPhaseDirCol, newFrame, doAll3, msDelayDirCol, msReferenceDirCol,
                   msfcs.nrow());
  
  if(haveMovingFrame){
    // Since ArrayMeasCol most likely uses one frame for the whole column, do
    // the second half of the conversion with a second pass through the column
    // instead of on a row-by-row basis.
    logSink() << LogIO::WARN
              << "Switching to or from accelerating frames is not well tested."
              << LogIO::POST;

    // Using msPhaseDirCol(0)[0] to initialize converter will only work if
    // msPhaseDirCol's type has been set to APP.
    const MDirection::Ref newerFrame(outType, mFrame_p);

    convertFieldCols(msPhaseDirCol, newerFrame, doAll3, msDelayDirCol,
                     msReferenceDirCol, msfcs.nrow());
  }

  // Update the reference frame label.
  //msfcs.phaseDir()
}


void FixVis::convertFieldCols(ArrayMeasColumn<MDirection>& pdc,
                              const MDirection::Ref& newFrame,
                              const Bool doAll3,
                              ArrayMeasColumn<MDirection>& ddc,
                              ArrayMeasColumn<MDirection>& rdc,
                              uInt nrows)
{
  // Unfortunately ArrayMeasColumn::doConvert() is private, which squashes the
  // point of making a conversion machine here.
//   Vector<MDirection> dummyV;
//   dummyV.assign(pdc(0));
//   MDirection::Convert *converter = new MDirection::Convert(dummyV[0],
//                                                            newFrame);
//   if(!converter)
//     logSink() << "Cannot make direction conversion machine"
//               << LogIO::EXCEPTION;

  // Convert each phase tracking center.  This will make them numerically
  // correct in the new frame, but will the column will still be labelled with the
  // old frame?
  for(uInt i = 0; i < nrows; ++i){
    //pdc.put(i, pdc.doConvert(i, *converter));
    pdc.put(i, pdc.convert(i, newFrame));

    if(doAll3){
      //ddc.put(i, ddc.doConvert(i, *converter));
      ddc.put(i, ddc.convert(i, newFrame));
      //rdc.put(i, rdc.doConvert(i, *converter));
      rdc.put(i, rdc.convert(i, newFrame));
    }
  }
  //delete converter;
}


void FixVis::setDistances(const Vector<Double>& distances)
{
  logSink() << LogOrigin("FixVis", "setDistances");
  if(distances.nelements() != nsel_p)
    logSink() << LogIO::EXCEPTION
              << "Mismatch between the # of specified distances and selected fields."
              << LogIO::POST;
  distances_p = distances;
}

uInt FixVis::check_fields()
{
  // Make sure FieldIds_p.nelements() == # fields in *ms_p.
  uInt nFields = FieldIds_p.nelements();
  nAllFields_p = ms_p.field().nrow();
  
  if(nAllFields_p != nFields)
    return 0;

  // Go through FieldIds_p, make sure the nonnegative entries are in the right
  // spots, and count them.
  uInt nsel = 0;
  for(uInt i = 0; i < nFields; ++i){
    if(FieldIds_p[i] > -1){
      if(static_cast<uInt>(FieldIds_p[i]) != i)
        return 0;
      ++nsel;
    }
  }
  return nsel;
}

// Calculate the (u, v, w)s and store them in ms_p.
Bool FixVis::calc_uvw(const String& refcode)
{
  Bool retval = false;
  
  logSink() << LogOrigin("FixVis", "calc_uvw");
  
  // Make sure FieldIds_p has a Field ID for each selected field, and -1 for
  // everything else!
  if(nsel_p > 0 // && static_cast<uInt>(nsel_p) == phaseDirs_p.nelements()
     ){
    // This is just an enum!  Why can't Muvw just return it instead of
    // filling out a reference?
    Muvw::Types uvwtype;
    MBaseline::Types bltype;
    
    try{
      MBaseline::getType(bltype, refcode);
      Muvw::getType(uvwtype, refcode);
    }
    catch(AipsError x){
      logSink() << LogIO::SEVERE
		<< "refcode \"" << refcode << "\" is not valid for baselines."
		<< LogIO::POST;
    }
    
    try{
      MSUVWGenerator uvwgen(ms_p, bltype, uvwtype);  
      retval = uvwgen.make_uvws(FieldIds_p);

      // Update HISTORY table
      LogSink localLogSink = LogSink(LogMessage::NORMAL, False);	  
      localLogSink.clearLocally();
      LogIO os(LogOrigin("im", "calcuvw()", WHERE), localLogSink);
      
      os << "UVWs regenerated for field";
      if(FieldIds_p.nelements() > 1)
        os << "s";
      os << " " << FieldIds_p
        //<< ", proj=" << proj
         << LogIO::POST;
      ms_p.lock();
      MSHistoryHandler mhh(ms_p, "FixVis::calcuvw()");
      mhh.addMessage(os);
      ms_p.unlock();      
    }
    catch(AipsError x){
      logSink() << LogIO::SEVERE
		<< "Error " << x.getMesg() << " in MSUVWGenerator::uvwgen()."
		<< LogIO::POST;
    }
  }
  else{
    logSink() << LogIO::SEVERE
	      << "There is a problem with the selected field IDs and phase tracking centers."
	      << LogIO::POST;
  }
  return retval;
}

// Don't just calculate the (u, v, w)s, do everything and store them in ms_p.
Bool FixVis::fixvis(const String& refcode, const String& dataColName)
{
  logSink() << LogOrigin("FixVis", "fixvis");

  Bool retval = false;
  if(nsel_p > 0){
    if(phaseDirs_p.nelements() == static_cast<uInt>(nsel_p)){
      // First calculate new UVWs for the selected fields?
      // calc_uvw(refcode);

      //**** Adjust the phase tracking centers and distances. ****
      // Loop through each selected field.
      Int selectedField;
      for(uInt fldCounter = 0; fldCounter < nsel_p; ++fldCounter){
        selectedField = FieldIds_p[fldCounter];
        setImageField(selectedField);
        if(makeSelection(selectedField)){
          processSelected(fldCounter);

	  // Update FIELD (and/or optional tables SOURCE, OBSERVATION, but not
	  // POINTING?) to new PTC.

          retval = true;
        }
        else{
          logSink() << LogIO::SEVERE
                    << "Field " << selectedField
                    << " could not be selected for phase tracking center or"
                    << " distance adjustment."
                    << LogIO::POST;
        }
      }
    }
    else if(phaseDirs_p.nelements() > 0){
      logSink() << LogIO::SEVERE
		<< "There is a problem with the selected field IDs and phase"
                << " tracking centers.\n"
		<< "No adjustments of phase tracking centers or distances will"
                << "  be done."
		<< LogIO::POST;
    }
  }
  else{
    logSink() << LogIO::SEVERE << "No fields are selected." << LogIO::POST;
  }  
  return retval;
}

// void FixVis::destroySkyEquation() 
// {
//   // if(se_p) delete se_p; se_p = 0;
//   //redoSkyModel_p = True;
// }

Bool FixVis::setImageField(const Int fieldid,
                           const Bool dotrackDir //, const MDirection& trackDir
                           )
{
  logSink() << LogOrigin("FixVis", "setImageField()");
//   ostringstream clicom;
//   clicom << " phaseCenter='" << phaseCenter;
//   os << String(clicom) << LogIO::POST;  
  try{
    //this->lock();

    doTrackSource_p = dotrackDir;
    //trackDir_p      = trackDir;

    fieldid_p = fieldid;
    //facets_p = 1;

    //ROMSFieldColumns msfield(ms_p->field());
    //phaseCenter_p = msfield.phaseDirMeas(fieldid_p);

    //this->unlock();
    return True;
  }
  catch(AipsError x){
    this->unlock();
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg()
       << LogIO::EXCEPTION;
    return False;
  } 
  return True;
}

Bool FixVis::lock()
{
  Bool ok = true;

  if(lockCounter_p == 0)
    ok = ms_p.lock();
  ++lockCounter_p;

  return ok;
}

void FixVis::unlock()
{
  if(lockCounter_p == 1)
    ms_p.unlock();

  if(lockCounter_p > 0)
    --lockCounter_p;
}


Bool FixVis::makeSelection(const Int selectedField)
{
  logSink() << LogOrigin("FixVis", "makeSelection()");
    
  //VisSet/MSIter will check if SORTED_TABLE exists and resort if necessary.
  MSSelection thisSelection;
  if(selectedField >= 0 && nAllFields_p > 1){
    Vector<Int> wrapper;
    wrapper.resize(1);
    wrapper[0] = selectedField;
    thisSelection.setFieldExpr(MSSelection::indexExprStr(wrapper));
  }
  else if(antennaSel_p){
    if(antennaId_p.nelements() > 0)
      thisSelection.setAntennaExpr(MSSelection::indexExprStr(antennaId_p));
    if(antennaSelStr_p[0] != "")
      thisSelection.setAntennaExpr(MSSelection::nameExprStr(antennaSelStr_p));
  }
    
  TableExprNode exprNode = thisSelection.toTableExprNode(&ms_p);    
    
  // Now remake the selected ms
  if(!(exprNode.isNull())){
    mssel_p = MeasurementSet(ms_p(exprNode));
  }
  else if(selectedField < 0 || nsel_p == nAllFields_p){
    // Null take all the ms ...setdata() blank means that
    mssel_p = MeasurementSet(ms_p);
  }
  else{
    logSink() << LogIO::SEVERE
              << "Error selecting field " << selectedField << ".  "
              << "Are you trying to adjust antenna positions at the same time?"
              << LogIO::POST;
    return false;
  }

  //mssel_p.rename(ms_p.tableName()+"/SELECTED_TABLE", Table::Scratch);
  if(mssel_p.nrow() == 0)
    return false;

  if(mssel_p.nrow() < ms_p.nrow())
    logSink() << LogIO::NORMAL
              << mssel_p.nrow() << " rows selected out of " << ms_p.nrow() << "." 
              << LogIO::POST;

  return true;
}

CoordinateSystem FixVis::getCoords(uInt numInSel)
{
  logSink() << LogOrigin("FixVis", "getCoords()");

  CoordinateSystem coordInfo;

//   if(!mssel_p)
//     logSink() << LogIO::EXCEPTION
//               << "Could not set the phase tracking center for field "
//               << FieldIds_p[numInSel]
//               << LogIO::POST;

  Vector<Double> deltas(2);
  deltas(0) = -1.0e-5; // -mcellx_p.get("rad").getValue();
  deltas(1) =  1.0e-5; //  mcelly_p.get("rad").getValue();
  
  MSColumns msc(ms_p);
  MFrequency::Types obsFreqRef = MFrequency::DEFAULT;
  ROScalarColumn<Int> measFreqRef(ms_p.spectralWindow(),
				  MSSpectralWindow::columnName(MSSpectralWindow::MEAS_FREQ_REF));
  //using the first frame of reference; TO DO should do the right thing 
  //for different frames selected. 
  //Int eh = spectralwindowids_p(0);
  if(measFreqRef(0) >= 0)
     obsFreqRef = (MFrequency::Types)measFreqRef(0);

  MVDirection mvPhaseCenter(phaseDirs_p[numInSel].getAngle());
  // Normalize correctly
  MVAngle ra = mvPhaseCenter.get()(0);
  ra(0.0);
  MVAngle dec = mvPhaseCenter.get()(1);
  Vector<Double> refCoord(2);
  refCoord(0) = ra.get().getValue();    
  refCoord(1) = dec;    
  
  Vector<Double> refPixel(2); 
  refPixel(0) = Double(npix_p / 2);
  refPixel(1) = Double(npix_p / 2);
  
  //defining observatory...needed for position on earth
  String telescop = msc.observation().telescopeName()(0);

  // defining epoch as begining time from timerange in OBSERVATION subtable
  // Using first observation for now
  MEpoch obsEpoch = msc.observation().timeRangeMeas()(0)(IPosition(1,0));

  //Now finding the position of the telescope on Earth...needed for proper
  //frequency conversions

  MPosition obsPosition;
  if(!(MeasTable::Observatory(obsPosition, telescop))){
    logSink() << LogIO::WARN << "Did not get the position of " << telescop 
       << " from data repository" << LogIO::POST;
    logSink() << LogIO::WARN 
       << "Please contact CASA to add it to the repository."
       << LogIO::POST;
    logSink() << LogIO::WARN << "Frequency conversion will not work " << LogIO::POST;
    freqFrameValid_p = False;
  }
  else{
    mLocation_p = obsPosition;
    freqFrameValid_p = True;
  }
  // Now find the projection to use: could probably also use
  // max(abs(w))=0.0 as a criterion
  Projection projection(Projection::SIN);
  if(telescop == "ATCASCP") {
    logSink() << LogIO::NORMAL << "Using SIN image projection adjusted for SCP" 
              << LogIO::POST;
    Vector<Double> projectionParameters(2);
    projectionParameters(0) = 0.0;
    if(sin(dec) != 0.0){
      projectionParameters(1) = cos(dec) / sin(dec);
      projection = Projection(Projection::SIN, projectionParameters);
    }
    else{
      logSink() << LogIO::WARN
                << "Singular projection for ATCA: using plain SIN"
                << LogIO::POST;
      projection = Projection(Projection::SIN);
    }
  }
  else if(telescop == "WSRT"){
    logSink() << LogIO::NORMAL << "Using SIN image projection adjusted for NCP" 
              << LogIO::POST;
    Vector<Double> projectionParameters(2);
    projectionParameters(0) = 0.0;
    if(sin(dec) != 0.0){
      projectionParameters(1) = cos(dec) / sin(dec);
      projection = Projection(Projection::SIN, projectionParameters);
    }
    else{
      logSink() << LogIO::WARN << "Singular projection for WSRT: using plain SIN" 
                << LogIO::POST;
      projection = Projection(Projection::SIN);
    }
  }
  else{
    logSink() << LogIO::DEBUGGING << "Using SIN image projection" << LogIO::POST;
  }
  logSink() << LogIO::NORMAL;
  
  Matrix<Double> xform(2, 2);
  xform = 0.0;
  xform.diagonal() = 1.0;
  DirectionCoordinate
    myRaDec(MDirection::Types(phaseDirs_p[numInSel].getRefPtr()->getType()),
	    projection,
	    refCoord(0), refCoord(1),
	    deltas(0), deltas(1),
	    xform,
	    refPixel(0), refPixel(1));
  
  // Now set up spectral coordinate
  SpectralCoordinate* mySpectral = 0;
  Double refChan = 0.0;
  
  // Include all spectral windows a la spectral synthesis.
  uInt nspw = msc.spectralWindow().nrow();
  {
    Double fmin = C::dbl_max;
    Double fmax = -(C::dbl_max);
    Double fmean = 0.0;
    for(uInt spw = 0; spw < nspw; ++spw){
      Vector<Double> chanFreq = msc.spectralWindow().chanFreq()(spw); 
      Vector<Double> freqResolution = msc.spectralWindow().chanWidth()(spw); 
      
      if(spw == 0){
        fmin = min(chanFreq - abs(freqResolution));
        fmax = max(chanFreq + abs(freqResolution));
      }
      else{
        fmin = min(fmin, min(chanFreq - abs(freqResolution)));
        fmax = max(fmax, max(chanFreq + abs(freqResolution)));
      }
    }

    fmean = 0.5 * (fmax + fmin);
    Vector<Double> restFreqArray;
    Double restFreq = fmean;
    if(getRestFreq(restFreqArray, 0, FieldIds_p[numInSel]))
      restFreq = restFreqArray[0];

    //imageNchan_p = 1;        // for MFS.
    Double finc = fmax - fmin; 
    mySpectral = new SpectralCoordinate(obsFreqRef, fmean, finc,
      					refChan, restFreq);
    logSink() << "Frequency = "
       << MFrequency(Quantity(fmean, "Hz")).get("GHz").getValue()
       << " GHz, synthesized continuum bandwidth = "
       << MFrequency(Quantity(finc, "Hz")).get("GHz").getValue()
       << " GHz" << LogIO::POST;
  }

  // In FTMachine lsrk is used for channel matching with data channel hence we
  // make sure that we convert to lsrk when dealing with the channels.
  freqFrameValid_p = freqFrameValid_p && (obsFreqRef != MFrequency::REST);
  if(freqFrameValid_p)
      mySpectral->setReferenceConversion(MFrequency::LSRK, obsEpoch, 
					 obsPosition, phaseDirs_p[numInSel]);
 
  // Polarization
  //polRep_p = SkyModel::CIRCULAR;  // Assuming it doesn't matter.

  Vector<Int> whichStokes(1);
  whichStokes.resize(1);
  whichStokes(0) = Stokes::I;  
  StokesCoordinate myStokes(whichStokes);
  
  //Set Observatory info
  ObsInfo myobsinfo;
  myobsinfo.setTelescope(telescop);
  myobsinfo.setPointingCenter(mvPhaseCenter);
  myobsinfo.setObsDate(obsEpoch);
  myobsinfo.setObserver(msc.observation().observer()(0));
  this->setObsInfo(myobsinfo);

  //Adding everything to the coordsystem
  coordInfo.addCoordinate(myRaDec);
  coordInfo.addCoordinate(myStokes);
  coordInfo.addCoordinate(*mySpectral);
  coordInfo.setObsInfo(myobsinfo);

  if(mySpectral)
    delete mySpectral;

  return coordInfo;
}

Bool FixVis::getRestFreq(Vector<Double>& restFreq, const Int spw,
                           const Int fldID)
{
  MSDopplerUtil msdoppler(ms_p);  // MS Doppler tracking utility

  restFreq.resize();
  try{
    msdoppler.dopplerInfo(restFreq, spw, fldID);
  }
  catch(...){
    restFreq.resize();
  }
  return restFreq.nelements() > 0;
}

void FixVis::setObsInfo(ObsInfo& obsinfo)
{
  latestObsInfo_p = obsinfo;
}

void FixVis::processSelected(uInt numInSel)
{
  logSink() << LogOrigin("FixVis", "processSelected()");

  PagedImage<Complex> cImageImage(tiledShape_p,
                                  getCoords(numInSel),
                                  "should_not_have_been_made");
  //cImageImage.set(Complex(0.0));  // Should be unnecessary.

  // Should the cache be tiny since cImageImage is and won't be made?
  cImageImage.setMaximumCacheSize((HostInfo::memoryTotal() / 8) * 1024 / 256);
  cImageImage.table().markForDelete();

  // "Make" the image.
//   this->lock();
//   this->makeEmptyImage(coords, modelName, fieldid_p);
//   this->unlock();

  // Add the distance to the object: this is not nice. We should define the
  // coordinates properly.
  Record info(cImageImage.miscInfo());
  info.define("distance", numInSel < distances_p.nelements() ?
              distances_p[numInSel] : 0.0);
  cImageImage.setMiscInfo(info);
  
  VisSet vs(makeVisSet(mssel_p));
  AlwaysAssert(&vs, AipsError);
  ROVisIter& vi(vs.iter());	// Initialize the gradients
	  
  // Loop over all visibilities in the selected field.
  VisBuffer vb(vi);
  
  // Initialize put for this model.
  vi.origin();
  // Loop over the visibilities, putting VisBuffers.  Watch out!  vi gets
  // initialized and stepped, but vb is the used object (vb ~ *vi).
  for(vi.originChunks(); vi.moreChunks(); vi.nextChunk()){
    for(vi.origin(); vi.more(); ++vi){
      for(uInt datacol = 0; datacol < nDataCols_p; ++datacol){
        if(dataColNames_p[datacol] == MS::columnName(MS::MODEL_DATA))
          vb.visCube() = vb.modelVisCube();
        else if(dataColNames_p[datacol] == MS::columnName(MS::CORRECTED_DATA))
          vb.visCube() = vb.correctedVisCube();
			
        put(vb, -1);

        // Correct differential aberration?
      }
    }
  }
  vs.flush();

  // Update HISTORY table
  LogSink localLogSink = LogSink(LogMessage::NORMAL, False);	  
  localLogSink.clearLocally();
  LogIO os(LogOrigin("FixVis", "processSelected()", WHERE), localLogSink);
      
  os << "Processed field " << FieldIds_p[numInSel] << LogIO::POST;
  ms_p.lock();
  MSHistoryHandler mhh(ms_p, "FixVis::calcuvw()");
  mhh.addMessage(os);
  ms_p.unlock();
}
  
void FixVis::put(const VisBuffer& vb, Int row, Bool dopsf, FTMachine::Type type, 
                 const Matrix<Float>& imweight)
{
  //Check if ms has changed then cache new spw and chan selection
  if(vb.newMS())
    matchAllSpwChans(vb);
  
  //Here we redo the match or use previous match
  
  //Channel matching for the actual spectral window of buffer
  if(doConversion_p[vb.spectralWindow()]){
    matchChannel(vb.spectralWindow(), vb);
  }
  else{
    chanMap.resize();
    chanMap = multiChanMap_p[vb.spectralWindow()];
  }

  //No point in reading data if its not matching in frequency
  if(max(chanMap) == -1)
    return;

  // If row is -1 then we pass through all rows
  Int startRow, endRow, nRow;
  if(row == -1){
    nRow     = vb.nRow();
    startRow = 0;
    endRow   = nRow - 1;
  }
  else{
    nRow     = 1;
    startRow = row;
    endRow   = row;
  }


  Matrix<Double> uvw(3, vb.uvw().nelements());
  uvw=0.0;
  Vector<Double> dphase(vb.uvw().nelements());
  dphase=0.0;

  // //NEGATING to correct for an image inversion problem
//   for(Int i = startRow; i <= endRow; ++i){
// 	for(uInt idim = 0; idim < 2; ++idim)
// 	  uvw(idim, i) = -vb.uvw()(i)(idim);
// 	uvw(2, i) = vb.uvw()(i)(2);
//   }

  rotateUVW(uvw, dphase, vb);

  // Immediately returns if not needed.
  refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);
}

VisSet FixVis::makeVisSet(MeasurementSet& ms, Bool compress, Bool mosaicOrder)
{
  Block<Int> sort(0);
  sort.resize(4);
  if(mosaicOrder){
    sort[0] = MS::FIELD_ID;
    sort[1] = MS::ARRAY_ID;
  }
  else{ 					// use default sort order
    sort[0] = MS::ARRAY_ID;
    sort[1] = MS::FIELD_ID;
  }
  sort[2] = MS::DATA_DESC_ID;
  sort[3] = MS::TIME;

  Matrix<Int> noselection;
  Double timeInterval = 0;
  return VisSet(ms, sort, noselection, timeInterval, compress);
}

void FixVis::ok() {
  AlwaysAssert(image, AipsError);
}

void FixVis::init()
{
  logSink() << LogOrigin("FixVis", "init")  << LogIO::NORMAL;

  ok();

  //npol  = image->shape()(2);
  //nchan = image->shape()(3);
}

// Initialize FTMachine.
void FixVis::initializeToVis(ImageInterface<Complex>& iimage,
			     const VisBuffer& vb)
{
  image = &iimage;

  ok();
  init();

  // Initialize the maps for polarization and channel. These maps
  // translate visibility indices into image indices
  //RR initMaps(vb);
}

void FixVis::finalizeToVis()
{
}

// Initialize the FFT to the Sky. Here we have to setup and initialize the
// grid. 
void FixVis::initializeToSky(ImageInterface<Complex>& iimage,
			     Matrix<Float>& weight, const VisBuffer& vb)
{
  // image always points to the image
  image = &iimage;

  init();

  // Initialize the maps for polarization and channel. These maps
  // translate visibility indices into image indices
  //RR initMaps(vb);
}

}  // End of casa namespace.

