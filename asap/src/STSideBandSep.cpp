// C++ Interface: STSideBandSep
//
// Description:
//    A class to invoke sideband separation of Scantable
//
// Author: Kana Sugimoto <kana.sugi@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//

// STL
#include <ctype.h>

// cascore
#include <casa/OS/File.h>
#include <casa/Logging/LogIO.h>
#include <casa/Quanta/QuantumHolder.h>

#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MCFrequency.h>

#include <tables/Tables/TableRow.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableVector.h>

// asap
#include "STGrid.h"
#include "STMath.h"
#include "MathUtils.h"
#include "STSideBandSep.h"

using namespace std ;
using namespace casa ;
using namespace asap ;

// #ifndef KS_DEBUG
// #define KS_DEBUG
// #endif

namespace asap {

// constructors
STSideBandSep::STSideBandSep(const vector<string> &names)
{
  LogIO os(LogOrigin("STSideBandSep","STSideBandSep()", WHERE));
  os << "Setting scantable names to process." << LogIO::POST ;
  // Set file names
  ntable_ = names.size();
  infileList_.resize(ntable_);
  for (unsigned int i = 0; i < ntable_; i++){
    if (!checkFile(names[i], "d"))
      throw( AipsError("File does not exist") );
    infileList_[i] = names[i];
  }
  intabList_.resize(0);

  init();

  {// Summary
    os << ntable_ << " files are set: [";
    for (unsigned int i = 0; i < ntable_; i++) {
      os << " '" << infileList_[i] << "' ";
      if (i != ntable_-1) os << ",";
    }
    os << "] " << LogIO::POST;
  }
};

STSideBandSep::STSideBandSep(const vector<ScantableWrapper> &tables)
{
  LogIO os(LogOrigin("STSideBandSep","STSideBandSep()", WHERE));
  os << "Setting list of scantables to process." << LogIO::POST ;
  // Set file names
  ntable_ = tables.size();
  intabList_.resize(ntable_);
  for (unsigned int i = 0; i < ntable_; i++){
    intabList_[i] = tables[i].getCP();
  }
  infileList_.resize(0);

  init();
  tp_ = intabList_[0]->table().tableType();

  os << ntable_ << " tables are set." << LogIO::POST;
};

STSideBandSep::~STSideBandSep()
{
};

void STSideBandSep::init()
{
  // frequency setup
  sigIfno_= -1;
  ftol_ = -1;
  solFrame_ = MFrequency::N_Types;
  // shifts
  initshift();
  // direction tolerance
  xtol_ = ytol_ = 9.69627e-6; // 2arcsec
  // solution parameters
  otherside_ = false;
  doboth_ = false;
  rejlimit_ = 0.2;
  // LO1 values
  lo1Freq_ = -1;
  loTime_ = -1;
  loDir_ = "";
  // Default LO frame is TOPO
  loFrame_ = MFrequency::TOPO;
  // scantable storage
  tp_ = Table::Memory;
};

void STSideBandSep::initshift()
{
  // shifts
  nshift_ = 0;
  nchan_ = 0;
  sigShift_.resize(0);
  imgShift_.resize(0);
  tableList_.resize(0);
};

void STSideBandSep::setFrequency(const int ifno,
				 const string freqtol,
				 const string frame)
{
  LogIO os(LogOrigin("STSideBandSep","setFrequency()", WHERE));

  initshift();

  // IFNO
  sigIfno_ = ifno;

  // Frequency tolerance
  Quantum<Double> qftol;
  readQuantity(qftol, String(freqtol));
  if (!qftol.getUnit().empty()){
    // make sure the quantity is frequency
    if (qftol.getFullUnit().getValue() != Unit("Hz").getValue())
      throw( AipsError("Invalid quantity for frequency tolerance.") );
    qftol.convert("Hz");
  }
  ftol_ = qftol;

  // Frequency Frame
  if (!frame.empty()){
    MFrequency::Types mft;
    if (!MFrequency::getType(mft, frame))
      throw( AipsError("Invalid frame type.") );
    solFrame_ = mft;
  } else {
    solFrame_ = MFrequency::N_Types;
  }

  {// Summary
    const String sframe = ( (solFrame_ == MFrequency::N_Types) ?
			    "table frame" :
			    MFrequency::showType(solFrame_) );
    os << "Frequency setup to search IF group: "
       << "IFNO of table[0] = " << sigIfno_ 
       << " , Freq tolerance = " << ftol_.getValue() << " [ "
       << (ftol_.getUnit().empty() ? "channel" : ftol_.getUnit() )
       << " ] (in " << sframe <<")" << LogIO::POST;
  }
};


void STSideBandSep::setDirTolerance(const vector<string> dirtol)
{
  LogIO os(LogOrigin("STSideBandSep","setDirTolerance()", WHERE));
  Quantum<Double> qcell;
  if ( (dirtol.size() == 1) && !dirtol[0].empty() ) {
    readQuantity(qcell, String(dirtol[0]));
    if (qcell.getFullUnit().getValue() == Unit("rad").getValue())
      xtol_ = ytol_ = qcell.getValue("rad");
    else
      throw( AipsError("Invalid unit for direction tolerance.") );
  }
  else if (dirtol.size() > 1) {
    if ( dirtol[0].empty() && dirtol[1].empty() )
      throw( AipsError("Direction tolerance is empty.") );
    if ( !dirtol[0].empty() ) {
      readQuantity(qcell, String(dirtol[0]));
      if (qcell.getFullUnit().getValue() == Unit("rad").getValue())
	xtol_ = qcell.getValue("rad");
      else
	throw( AipsError("Invalid unit for direction tolerance.") );
    }
    if ( !dirtol[1].empty() ) {
      readQuantity(qcell, String(dirtol[1]));
      if (qcell.getFullUnit().getValue() == Unit("rad").getValue())
	ytol_ = qcell.getValue("rad");
      else
	throw( AipsError("Invalid unit for direction tolerance.") );
    }
    else {
      ytol_ = xtol_;
    }
  }
  else throw( AipsError("Invalid direction tolerance.") );

  os << "Direction tolerance: ( "
     << xtol_ << " , " << ytol_ << " ) [rad]" << LogIO::POST;
};

void STSideBandSep::setShift(const vector<double> &shift)
{
  LogIO os(LogOrigin("STSideBandSep","setShift()", WHERE));
  imgShift_.resize(shift.size());
  for (unsigned int i = 0; i < shift.size(); i++)
    imgShift_[i] = shift[i];

  if (imgShift_.size() == 0) {
    os << "Channel shifts are cleared." << LogIO::POST;
  } else {
    os << "Channel shifts of image sideband are set: ( ";
    for (unsigned int i = 0; i < imgShift_.size(); i++) {
      os << imgShift_[i];
      if (i != imgShift_.size()-1) os << " , ";
    }
    os << " ) [channels]" << LogIO::POST;
  }
};

void STSideBandSep::setThreshold(const double limit)
{
  LogIO os(LogOrigin("STSideBandSep","setThreshold()", WHERE));
  if (limit < 0)
    throw( AipsError("Rejection limit should be a positive number.") );

  rejlimit_ = limit;
  os << "Rejection limit is set to " << rejlimit_ << LogIO::POST;
};

void STSideBandSep::separate(string outname)
{
  LogIO os(LogOrigin("STSideBandSep","separate()", WHERE));
  if (outname.empty())
    outname = "sbseparated.asap";

  // Set up a goup of IFNOs in the list of scantables within
  // the frequency tolerance and make them a list.
  nshift_ = setupShift();
  if (nshift_ < 2)
    throw( AipsError("At least 2 IFs are necessary for convolution.") );
  // Grid scantable and generate output tables
  ScantableWrapper gridst = gridTable();
  sigTab_p = gridst.getCP();
  if (doboth_)
    imgTab_p = gridst.copy().getCP();
  vector<unsigned int> remRowIds;
  remRowIds.resize(0);
  Matrix<float> specMat(nchan_, nshift_);
  Matrix<bool> flagMat(nchan_, nshift_);
  vector<float> sigSpec(nchan_), imgSpec(nchan_);
  Vector<bool> flagVec(nchan_);
  vector<uInt> tabIdvec;

  //Generate FFTServer
  fftsf.resize(IPosition(1, nchan_), FFTEnums::REALTOCOMPLEX);
  fftsi.resize(IPosition(1, nchan_), FFTEnums::COMPLEXTOREAL);

  /// Loop over sigTab_p and separate sideband
  for (int irow = 0; irow < sigTab_p->nrow(); irow++){
    tabIdvec.resize(0);
    const int polId = sigTab_p->getPol(irow);
    const int beamId = sigTab_p->getBeam(irow);
    const vector<double> dir = sigTab_p->getDirectionVector(irow);
    // Get a set of spectra to solve
    if (!getSpectraToSolve(polId, beamId, dir[0], dir[1],
			   specMat, flagMat, tabIdvec)){
      remRowIds.push_back(irow);
#ifdef KS_DEBUG
      cout << "no matching row found. skipping row = " << irow << endl;
#endif
      continue;
    }
    // Solve signal sideband
    sigSpec = solve(specMat, tabIdvec, true);
    sigTab_p->setSpectrum(sigSpec, irow);
    if (sigTab_p->isAllChannelsFlagged(irow)){
      // unflag the spectrum since there should be some valid data
      sigTab_p->flagRow(vector<uInt>(irow), true);
      // need to unflag whole channels anyway
      sigTab_p->flag(irow, vector<bool>(), true);
    }
    // apply channel flag
    flagVec = collapseFlag(flagMat, tabIdvec, true);
    //boolVec = !boolVec; // flag
    vector<bool> tmpflag;
    flagVec.tovector(tmpflag);
    sigTab_p->flag(irow, tmpflag, false);

    // Solve image sideband
    if (doboth_) {
      imgSpec = solve(specMat, tabIdvec, false);
      imgTab_p->setSpectrum(imgSpec, irow);
      if (imgTab_p->isAllChannelsFlagged(irow)){
	// unflag the spectrum since there should be some valid data
	imgTab_p->flagRow(vector<uInt>(irow), true);
	// need to unflag whole channels anyway
	imgTab_p->flag(irow, vector<bool>(), true);
      }
      // apply channel flag
      flagVec = collapseFlag(flagMat, tabIdvec, false);
      //boolVec = !boolVec; // flag
      flagVec.tovector(tmpflag);
      imgTab_p->flag(irow, tmpflag, false);
    }
  } // end of row loop

  // Remove or flag rows without relevant data from gridded tables
  if (remRowIds.size() > 0) {
    const size_t nrem = remRowIds.size();
    if ( sigTab_p->table().canRemoveRow() ) {
      sigTab_p->table().removeRow(remRowIds);
      os << "Removing " << nrem << " rows from the signal band table"
	 << LogIO::POST;
    } else {
      sigTab_p->flagRow(remRowIds, false);
      os << "Cannot remove rows from the signal band table. Flagging "
	 << nrem << " rows" << LogIO::POST;
    }

    if (doboth_) {
      if ( imgTab_p->table().canRemoveRow() ) {
	imgTab_p->table().removeRow(remRowIds);
	os << "Removing " << nrem << " rows from the image band table"
	   << LogIO::POST;
      } else {
	imgTab_p->flagRow(remRowIds, false);
	os << "Cannot remove rows from the image band table. Flagging "
	   << nrem << " rows" << LogIO::POST;
      }
    }
  }

  // Finally, save tables on disk
  if (outname.size() ==0)
    outname = "sbseparated.asap";
  const string sigName = outname + ".signalband";
  os << "Saving SIGNAL sideband table: " << sigName << LogIO::POST;
  sigTab_p->makePersistent(sigName);
  if (doboth_) {
    solveImageFrequency();
    const string imgName = outname + ".imageband";
    os << "Saving IMAGE sideband table: " << sigName << LogIO::POST;
    imgTab_p->makePersistent(imgName);
  }

};

unsigned int STSideBandSep::setupShift()
{
  LogIO os(LogOrigin("STSideBandSep","setupShift()", WHERE));
  if (infileList_.size() == 0 && intabList_.size() == 0)
    throw( AipsError("No scantable has been set. Set a list of scantables first.") );

  const bool byname = (intabList_.size() == 0);
  // Make sure sigIfno_ exists in the first table.
  CountedPtr<Scantable> stab;
  vector<string> coordsav;
  vector<string> coordtmp(3);
  os << "Checking IFNO in the first table." << LogIO::POST;
  if (byname) {
    if (!checkFile(infileList_[0], "d"))
      os << LogIO::SEVERE << "Could not find scantable '" << infileList_[0]
	 << "'" << LogIO::EXCEPTION;
    stab = CountedPtr<Scantable>(new Scantable(infileList_[0]));
  } else {
    stab = intabList_[0];
  }
  if (sigIfno_ < 0) {
    sigIfno_ = (int) stab->getIF(0);
    os << "IFNO to process has not been set. Using the first IF = "
       << sigIfno_ << LogIO::POST;
  }

  unsigned int basench;
  double basech0, baseinc, ftolval, inctolval;
  coordsav = stab->getCoordInfo();
  const string stfframe = coordsav[1];
  coordtmp[0] = "Hz";
  coordtmp[1] = ( (solFrame_ == MFrequency::N_Types) ?
		  stfframe :
		  MFrequency::showType(solFrame_) );
  coordtmp[2] = coordsav[2];
  stab->setCoordInfo(coordtmp);
  if (!getFreqInfo(stab, (unsigned int) sigIfno_, basech0, baseinc, basench)) {
    os << LogIO::SEVERE << "No data with IFNO=" << sigIfno_
       << " found in the first table." << LogIO::EXCEPTION;
  }
  else {
    os << "Found IFNO = " << sigIfno_
       << " in the first table." << LogIO::POST;
  }
  if (ftol_.getUnit().empty()) {
    // tolerance in unit of channels
    ftolval = ftol_.getValue() * baseinc;
  }
  else {
    ftolval = ftol_.getValue("Hz");
  }
  inctolval = abs(baseinc/(double) basench);
  const string poltype0 = stab->getPolType();

  // Initialize shift values
  initshift();

//   const bool setImg = ( doboth_ && (imgShift_.size() == 0) );
  const bool setImg = (imgShift_.size() == 0);
  // Select IFs
  for (unsigned int itab = 0; itab < ntable_; itab++ ){
    os << "Table " << itab << LogIO::POST;
    if (itab > 0) {
      if (byname) {
	if (!checkFile(infileList_[itab], "d"))
	  os << LogIO::SEVERE << "Could not find scantable '"
	     << infileList_[itab] << "'" << LogIO::EXCEPTION;
	stab = CountedPtr<Scantable>(new Scantable(infileList_[itab]));
      } else {
	stab = intabList_[itab];
      }
      //POLTYPE should be the same.
      if (stab->getPolType() != poltype0 ) {
	os << LogIO::WARN << "POLTYPE differs from the first table."
	   << " Skipping the table" << LogIO::POST;
	continue;
      }
      // Multi beam data may not handled properly
      if (stab->nbeam() > 1)
	os <<  LogIO::WARN << "Table contains multiple beams. "
	   << "It may not be handled properly."  << LogIO::POST;

      coordsav = stab->getCoordInfo();
      coordtmp[2] = coordsav[2];
      stab->setCoordInfo(coordtmp);
    }
    bool selected = false;
    vector<uint> ifnos = stab->getIFNos();
    vector<uint>::iterator iter;
    const STSelector& basesel = stab->getSelection();
    for (iter = ifnos.begin(); iter != ifnos.end(); iter++){
      unsigned int nch;
      double freq0, incr;
      if ( getFreqInfo(stab, *iter, freq0, incr, nch) ){
	if ( (nch == basench) && (abs(freq0-basech0) < ftolval)
	     && (abs(incr-baseinc) < inctolval) ){
	  //Found
	  STSelector sel(basesel);
	  sel.setIFs(vector<int>(1,(int) *iter));
	  stab->setSelection(sel);
	  CountedPtr<Scantable> seltab = ( new Scantable(*stab, false) );
	  stab->setSelection(basesel);
	  seltab->setCoordInfo(coordsav);
	  const double chShift = (freq0 - basech0) / baseinc;
	  tableList_.push_back(seltab);
	  sigShift_.push_back(-chShift);
	  if (setImg)
	    imgShift_.push_back(chShift);

	  selected = true;
	  os << "- IF" << *iter << " selected: sideband shift = "
	     << chShift << " channels" << LogIO::POST;
	}
      }
    } // ifno loop
    stab->setCoordInfo(coordsav);
    if (!selected)
      os << LogIO::WARN << "No data selected in Table "
	 << itab << LogIO::POST;
  } // table loop
  nchan_ = basench;

  os << "Total number of IFs selected = " << tableList_.size()
     << LogIO::POST;
  if ( setImg && (sigShift_.size() != imgShift_.size()) ){
      os << LogIO::SEVERE
	 << "User defined channel shift of image sideband has "
	 << imgShift_.size()
	 << " elements, while selected IFNOs are " << sigShift_.size()
	 << "\nThe frequency tolerance (freqtol) may be too small."
	 << LogIO::EXCEPTION;
  }

  return tableList_.size();
};

bool STSideBandSep::getFreqInfo(const CountedPtr<Scantable> &stab,
				const unsigned int &ifno,
				double &freq0, double &incr,
				unsigned int &nchan)
{
    vector<uint> ifnos = stab->getIFNos();
    bool found = false;
    vector<uint>::iterator iter;
    for (iter = ifnos.begin(); iter != ifnos.end(); iter++){
      if (*iter == ifno) {
	found = true;
	break;
      }
    }
    if (!found)
      return false;

    const STSelector& basesel = stab->getSelection();
    STSelector sel(basesel);
    sel.setIFs(vector<int>(1,(int) ifno));
    stab->setSelection(sel);
    vector<double> freqs;
    freqs = stab->getAbcissa(0);
    freq0 = freqs[0];
    incr = freqs[1] - freqs[0];
    nchan = freqs.size();
    stab->setSelection(basesel);
    return true;
};

ScantableWrapper STSideBandSep::gridTable()
{
  LogIO os(LogOrigin("STSideBandSep","gridTable()", WHERE));
  if (tableList_.size() == 0)
    throw( AipsError("Internal error. No scantable has been set to grid.") );
  Double xmin, xmax, ymin, ymax;
  mapExtent(tableList_, xmin, xmax, ymin, ymax);
  const Double centx = 0.5 * (xmin + xmax);
  const Double centy = 0.5 * (ymin + ymax);
  const int nx = max(1, (int) ceil( (xmax - xmin) * cos(centy) /xtol_ ) );
  const int ny = max(1, (int) ceil( (ymax - ymin) / ytol_ ) );

  string scellx, scelly;
  {
    ostringstream oss;
    oss << xtol_ << "rad" ;
    scellx = oss.str();
  }
  {
    ostringstream oss;
    oss << ytol_ << "rad" ;
    scelly = oss.str();
  }

  ScantableWrapper stab0;
  if (intabList_.size() > 0)
    stab0 = ScantableWrapper(intabList_[0]);
  else
    stab0 = ScantableWrapper(infileList_[0]);

  string scenter;
  {
    ostringstream oss;
    oss << stab0.getCP()->getDirectionRefString() << " "
	<< centx << "rad" << " " << centy << "rad";
    scenter = oss.str();
  } 
  
  STGrid2 gridder = STGrid2(stab0);
  gridder.setIF(sigIfno_);
  gridder.defineImage(nx, ny, scellx, scelly, scenter);
  //  gridder.setFunc("box", 1); // convsupport=1 fails
  gridder.setFunc("box");
  gridder.setWeight("uniform");
#ifdef KS_DEBUG
  cout << "Grid parameter summary: " << endl;
  cout << "- IF = " << sigIfno_ << endl;
  cout << "- center = " << scenter << "\n"
       << "- npix = (" << nx << ", " << ny << ")\n"
       << "- cell = (" << scellx << ", " << scelly << ")" << endl;
#endif
  gridder.grid();
  const int itp = (tp_ == Table::Memory ? 0 : 1);
  ScantableWrapper gtab = gridder.getResultAsScantable(itp);
  // WORKAROUND : Shift TIME for proper pointing resolution in future imaging.
  shiftTimeInGriddedST(gtab.getCP());
  return gtab;
};

void STSideBandSep::mapExtent(vector< CountedPtr<Scantable> > &tablist,
			      Double &xmin, Double &xmax, 
			      Double &ymin, Double &ymax)
{
  ROArrayColumn<Double> dirCol_;
  dirCol_.attach( tablist[0]->table(), "DIRECTION" );
  Matrix<Double> direction = dirCol_.getColumn();
  Vector<Double> ra( direction.row(0) );
  mathutil::rotateRA(ra);
  minMax( xmin, xmax, ra );
  minMax( ymin, ymax, direction.row(1) );
  Double amin, amax, bmin, bmax;
  const uInt ntab = tablist.size();
  for ( uInt i = 1 ; i < ntab ; i++ ) {
    dirCol_.attach( tablist[i]->table(), "DIRECTION" );
    direction.assign( dirCol_.getColumn() );
    ra.assign( direction.row(0) );
    mathutil::rotateRA(ra);
    minMax( amin, amax, ra );
    minMax( bmin, bmax, direction.row(1) );
    xmin = min(xmin, amin);
    xmax = max(xmax, amax);
    ymin = min(ymin, bmin);
    ymax = max(ymax, bmax);
  }
};

// STGrid sets the identical time for all rows in scantable
// which is reasonable thing to do in position based averaging.
// However, this prevents CASA from finding proper pointing
// per spectra once the gridded scantable is converted to
// measurement set (MS). It is because MS does not
// have ability to store per spectra pointing information.
// MS stores pointing information in a subtable, POINTING,
// with corresponding TIME when an antenna pointed the direction.
// The pointing direction corresponding to a spectra is resolved
// in MS by interpolating DIRECTION in POINTING subtable in TIME
// the spectra is observed. If there are multiple match,
// the first match is adopted. Therefore, gridded table (whose TIME
// is set to a single value) is misunderstood in MS that all data
// come from a single pointing.
// The function workarounds this defect by artificially shifting
// TIME by INTERVAL in each row.
void STSideBandSep::shiftTimeInGriddedST(const CountedPtr<Scantable> &stab)
{
  LogIO os(LogOrigin("STSideBandSep", "shiftTimeInGriddedST()", WHERE));
  // Gridded table usually has an IF and a BEAM.
  {
    std::vector<uint> bmnos = stab->getBeamNos();
    if (bmnos.size() > 1)
      throw( AipsError("Multiple BEAMNOs found in the scantable. This may not a gridded table") );
    std::vector<uint> ifnos = stab->getIFNos();
    if (ifnos.size() > 1)
      throw( AipsError("Multiple IFNOs found in the scantable. This may not a gridded table") );
  }
  // Rows in gridded table usually sorted by DIRECTION
  const Table& tab = stab->table();
  ScalarColumn<Double> mjdCol( tab, "TIME");
  ROScalarColumn<Double> intCol( tab, "INTERVAL");
  ROArrayColumn<Double> dirCol( tab, "DIRECTION");
  Matrix<Double> direction = dirCol.getColumn();
  Vector<Double> ra( direction.row(0) );
  Vector<Double> dec( direction.row(1) );
  Double prevTime, prevInt, prevRA(ra[0]), prevDec(dec[0]);
  mjdCol.get(0, prevTime);
  intCol.get(0, prevInt);
  Double currInt, currRA, currDec;
  Double dx(xtol_*0.95), dy(ytol_*0.95);
  Double secToDay(1./24./3600.);
  for (int irow = 0; irow < stab->nrow(); ++irow){
    currRA = ra[irow];
    currDec = dec[irow];
    if ((prevRA+dx-currRA)*(currRA-prevRA+dx)>=0 &&
	(prevDec+dy-currDec)*(currDec-prevDec+dy)>=0) {
      // the same time stamp as the previous row
      mjdCol.put(irow, prevTime);
      // remember the longest interval
      intCol.get(irow, currInt);
      if (currInt > prevInt) prevInt = currInt;
    } else {
      // a new direction. need to set new time stamp.
      prevTime += prevInt*secToDay;
      mjdCol.put(irow, prevTime);
      // new interval and direction
      intCol.get(irow, prevInt);
      prevRA = currRA;
      prevDec = currDec;
    }
  }
};

// bool STSideBandSep::getSpectraToSolve(const int polId, const int beamId,
// 				      const double dirX, const double dirY,
// 				      Matrix<float> &specMat, vector<uInt> &tabIdvec)
bool STSideBandSep::getSpectraToSolve(const int polId, const int beamId,
				      const double dirX, const double dirY,
				      Matrix<float> &specMat,
				      Matrix<bool> &flagMat,
				      vector<uInt> &tabIdvec)
{
  LogIO os(LogOrigin("STSideBandSep","getSpectraToSolve()", WHERE));

  tabIdvec.resize(0);
  specMat.resize(nchan_, nshift_);
  Vector<float> spec;
  Vector<bool> boolVec;
  uInt nspec = 0;
  STMath stm(false); // insitu has no effect for average.
  for (uInt itab = 0 ; itab < nshift_ ; itab++) {
    CountedPtr<Scantable> currtab_p = tableList_[itab];
    // Selection by POLNO and BEAMNO
    const STSelector& basesel = currtab_p->getSelection();
    STSelector sel(basesel);
    sel.setPolarizations(vector<int>(1, polId));
    sel.setBeams(vector<int>(1, beamId));
    try {
      currtab_p->setSelection(sel);
    } catch (...) {
#ifdef KS_DEBUG
      cout << "Table " << itab << " - No spectrum found. skipping the table."
	   << endl;
#endif
      continue;
    }
    // Selection by direction;
    vector<int> selrow(0);
    vector<double> currDir(2, 0.);
    const int nselrow = currtab_p->nrow();
    for (int irow = 0 ; irow < nselrow ; irow++) {
      currDir = currtab_p->getDirectionVector(irow);
      if ( (abs(currDir[0]-dirX) > xtol_) ||
	   (abs(currDir[1]-dirY) > ytol_) )
	continue;
      // within direction tolerance
      selrow.push_back(irow);
    } // end of row loop

    if (selrow.size() < 1) {
      currtab_p->setSelection(basesel);

#ifdef KS_DEBUG
      cout << "Table " << itab << " - No spectrum found. skipping the table."
	   << endl;
#endif

      continue;
    }

    // At least a spectrum is selected in this table
    CountedPtr<Scantable> seltab_p = ( new Scantable(*currtab_p, false) );
    currtab_p->setSelection(basesel);
    STSelector sel2(seltab_p->getSelection());
    sel2.setRows(selrow);
    seltab_p->setSelection(sel2);
    CountedPtr<Scantable> avetab_p;
    if (seltab_p->nrow() > 1) {
      // STMath::average also merges FLAGTRA and FLAGROW
      avetab_p = stm.average(vector< CountedPtr<Scantable> >(1, seltab_p),
			     vector<bool>(), "TINTSYS", "NONE");
#ifdef KS_DEBUG
      cout << "Table " << itab << " - more than a spectrum is selected. averaging rows..."
	   << endl;
#endif
      if (avetab_p->nrow() > 1)
	throw( AipsError("Averaged table has more than a row. Somethigs went wrong.") );
    } else {
      avetab_p = seltab_p;
    }
    // Check FLAGTRA and FLAGROW if there's any valid channel in the spectrum
    if (avetab_p->getFlagRow(0) || avetab_p->isAllChannelsFlagged(0)) {
#ifdef KS_DEBUG
      cout << "Table " << itab << " - All data are flagged. skipping the table."
	   << endl;
#endif
      continue;
    }
    // Interpolate flagged channels of the spectrum.
    Vector<Float> tmpSpec = avetab_p->getSpectrum(0);
    // Mask is true if the data is valid (!flag)
    vector<bool> mask = avetab_p->getMask(0);
    mathutil::doZeroOrderInterpolation(tmpSpec, mask);
    spec.reference(specMat.column(nspec));
    spec = tmpSpec;
    boolVec.reference(flagMat.column(nspec));
    boolVec = mask; // cast std::vector to casa::Vector
    boolVec = !boolVec;
    tabIdvec.push_back((uInt) itab);
    nspec++;
    //Liberate from reference
    spec.unique();
    boolVec.unique();
  } // end of table loop
  // Check the number of selected spectra and resize matrix.
  if (nspec != nshift_){
    //shiftSpecmat.resize(nchan_, nspec, true);
    specMat.resize(nchan_, nspec, true);
    flagMat.resize(nchan_, nspec, true);
#ifdef KS_DEBUG
      cout << "Could not find corresponding rows in some tables."
	   << endl;
      cout << "Number of spectra selected = " << nspec << endl;
#endif
  }
  if (nspec < 2) {
#ifdef KS_DEBUG
      cout << "At least 2 spectra are necessary for convolution"
	   << endl;
#endif
      return false;
  }
  return true;
};


Vector<bool> STSideBandSep::collapseFlag(const Matrix<bool> &flagMat,
					 const vector<uInt> &tabIdvec,
					 const bool signal)
{
  LogIO os(LogOrigin("STSideBandSep","collapseFlag()", WHERE));
  if (tabIdvec.size() == 0)
    throw(AipsError("Internal error. Table index is not defined."));
  if (flagMat.ncolumn() != tabIdvec.size())
    throw(AipsError("Internal error. The row number of input matrix is not conformant."));
  if (flagMat.nrow() != nchan_)
    throw(AipsError("Internal error. The channel size of input matrix is not conformant."));
  
  const size_t nspec = tabIdvec.size();
  vector<double> *thisShift;
  if (signal == otherside_) {
    // (solve signal && solveother = T) OR (solve image && solveother = F)
    thisShift = &imgShift_;
  } else {
    // (solve signal && solveother = F) OR (solve image && solveother = T)
    thisShift =  &sigShift_;
 }

  Vector<bool> outflag(nchan_, false);
  double tempshift;
  Vector<bool> shiftvec(nchan_, false);
  Vector<bool> accflag(nchan_, false);
  uInt shiftId;
  for (uInt i = 0 ; i < nspec; ++i) {
    shiftId = tabIdvec[i];
    tempshift = - thisShift->at(shiftId);
    shiftFlag(flagMat.column(i), tempshift, shiftvec);
    // Now accumulate Flag
    for (uInt j = 0 ; j < nchan_ ; ++j)
      accflag[j] |= shiftvec[j];
  }
  outflag = accflag;
  // Shift back Flag
  //cout << "Shifting FLAG back to " << thisShift->at(0) << " channels" << endl;
  //shiftFlag(accflag, thisShift->at(0), outflag);

  return outflag;
}


vector<float> STSideBandSep::solve(const Matrix<float> &specmat,
				   const vector<uInt> &tabIdvec,
				   const bool signal)
{
  LogIO os(LogOrigin("STSideBandSep","solve()", WHERE));
  if (tabIdvec.size() == 0)
    throw(AipsError("Internal error. Table index is not defined."));
  if (specmat.ncolumn() != tabIdvec.size())
    throw(AipsError("Internal error. The row number of input matrix is not conformant."));
  if (specmat.nrow() != nchan_)
    throw(AipsError("Internal error. The channel size of input matrix is not conformant."));
  

#ifdef KS_DEBUG
  cout << "Solving " << (signal ? "SIGNAL" : "IMAGE") << " sideband."
     << endl;
#endif

  const size_t nspec = tabIdvec.size();
  vector<double> *thisShift, *otherShift;
  if (signal == otherside_) {
    // (solve signal && solveother = T) OR (solve image && solveother = F)
    thisShift = &imgShift_;
    otherShift = &sigShift_;
#ifdef KS_DEBUG
    cout << "Image sideband will be deconvolved." << endl;
#endif
  } else {
    // (solve signal && solveother = F) OR (solve image && solveother = T)
    thisShift =  &sigShift_;
    otherShift = &imgShift_;
#ifdef KS_DEBUG
    cout << "Signal sideband will be deconvolved." << endl;
#endif
 }

  vector<double> spshift(nspec);
  Matrix<float> shiftSpecmat(nchan_, nspec, 0.);
  double tempshift;
  Vector<float> shiftspvec;
  uInt shiftId;
  for (uInt i = 0 ; i < nspec; i++) {
    shiftId = tabIdvec[i];
    spshift[i] = otherShift->at(shiftId) - thisShift->at(shiftId);
    tempshift = - thisShift->at(shiftId);
    shiftspvec.reference(shiftSpecmat.column(i));
    shiftSpectrum(specmat.column(i), tempshift, shiftspvec);
  }

  Matrix<float> convmat(nchan_, nspec*(nspec-1)/2, 0.);
  vector<float> thisvec(nchan_, 0.);

  float minval, maxval;
  minMax(minval, maxval, shiftSpecmat);
#ifdef KS_DEBUG
  cout << "Max/Min of input Matrix = (max: " << maxval << ", min: " << minval << ")" << endl;
#endif

#ifdef KS_DEBUG
  cout << "starting deconvolution" << endl;
#endif
  deconvolve(shiftSpecmat, spshift, rejlimit_, convmat);
#ifdef KS_DEBUG
  cout << "finished deconvolution" << endl;
#endif

  minMax(minval, maxval, convmat);
#ifdef KS_DEBUG
  cout << "Max/Min of output Matrix = (max: " << maxval << ", min: " << minval << ")" << endl;
#endif

  aggregateMat(convmat, thisvec);

  if (!otherside_) return thisvec;

  // subtract from the other side band.
  vector<float> othervec(nchan_, 0.);
  subtractFromOther(shiftSpecmat, thisvec, spshift, othervec);
  return othervec;
};


void STSideBandSep::shiftSpectrum(const Vector<float> &invec,
				  double shift,
				  Vector<float> &outvec)
{
  LogIO os(LogOrigin("STSideBandSep","shiftSpectrum()", WHERE));
  if (invec.size() != nchan_)
    throw(AipsError("Internal error. The length of input vector differs from nchan_"));
  if (outvec.size() != nchan_)
    throw(AipsError("Internal error. The length of output vector differs from nchan_"));

#ifdef KS_DEBUG
  cout << "Start shifting spectrum for " << shift << " channels" << endl;
#endif

  // tweak shift to be in 0 ~ nchan_-1
  if ( fabs(shift) > nchan_ ) shift = fmod(shift, nchan_);
  if (shift < 0.) shift += nchan_;
  double rweight = fmod(shift, 1.);
  if (rweight < 0.) rweight += 1.;
  double lweight = 1. - rweight;
  uInt lchan, rchan;

  outvec = 0;
  for (uInt i = 0 ; i < nchan_ ; i++) {
    lchan = uInt( floor( fmod( (i + shift), nchan_ ) ) );
    if (lchan < 0.) lchan += nchan_;
    rchan = ( (lchan + 1) % nchan_ );
    outvec(lchan) += invec(i) * lweight;
    outvec(rchan) += invec(i) * rweight;
#ifdef KS_DEBUG
    if (i == 2350 || i== 2930) {
      cout << "Channel= " << i << " of input vector: " << endl;
      cout << "L channel = " << lchan << endl;
      cout << "R channel = " << rchan << endl;
      cout << "L weight = " << lweight << endl;
      cout << "R weight = " << rweight << endl;
    }
#endif
  }
};


void STSideBandSep::shiftFlag(const Vector<bool> &invec,
				  double shift,
				  Vector<bool> &outvec)
{
  LogIO os(LogOrigin("STSideBandSep","shiftFlag()", WHERE));
  if (invec.size() != nchan_)
    throw(AipsError("Internal error. The length of input vector differs from nchan_"));
  if (outvec.size() != nchan_)
    throw(AipsError("Internal error. The length of output vector differs from nchan_"));

#ifdef KS_DEBUG
  cout << "Start shifting flag for " << shift << "channels" << endl;
#endif

  // shift is almost integer think it as int.
  // tolerance should be in 0 - 1
  double tolerance = 0.01;
  // tweak shift to be in 0 ~ nchan_-1
  if ( fabs(shift) > nchan_ ) shift = fmod(shift, nchan_);
  if (shift < 0.) shift += nchan_;
  double rweight = fmod(shift, 1.);
  bool ruse(true), luse(true);
  if (rweight < 0.) rweight += 1.;
  if (rweight < tolerance){
    // the shift is almost lchan
    ruse = false;
    luse = true;
  }
  if (rweight > 1-tolerance){
    // the shift is almost rchan
    ruse = true;
    luse = false;
  }
  uInt lchan, rchan;

  outvec = false;
  for (uInt i = 0 ; i < nchan_ ; i++) {
    lchan = uInt( floor( fmod( (i + shift), nchan_ ) ) );
    if (lchan < 0.) lchan += nchan_;
    rchan = ( (lchan + 1) % nchan_ );
    outvec(lchan) |= (invec(i) && luse);
    outvec(rchan) |= (invec(i) && ruse);
#ifdef KS_DEBUG
    if (i == 2350 || i == 2930) {
      cout << "Channel= " << i << " of input vector: " << endl;
      cout << "L channel = " << lchan << endl;
      cout << "R channel = " << rchan << endl;
      cout << "L channel will be " << (luse ? "used" : "ignored") << endl;
      cout << "R channel will be " << (ruse ? "used" : "ignored") << endl;
    }
#endif
  }
};


void STSideBandSep::deconvolve(Matrix<float> &specmat,
			       const vector<double> shiftvec,
			       const double threshold,
			       Matrix<float> &outmat)
{
  LogIO os(LogOrigin("STSideBandSep","deconvolve()", WHERE));
  if (specmat.nrow() != nchan_)
    throw(AipsError("Internal error. The length of input matrix differs from nchan_"));
  if (specmat.ncolumn() != shiftvec.size())
    throw(AipsError("Internal error. The number of input shifts and spectrum  differs."));

#ifdef KS_DEBUG
  float minval, maxval;
#endif
#ifdef KS_DEBUG
  minMax(minval, maxval, specmat);
  cout << "Max/Min of input Matrix = (max: " << maxval << ", min: " << minval << ")" << endl;
#endif

  uInt ninsp = shiftvec.size();
  outmat.resize(nchan_, ninsp*(ninsp-1)/2, 0.);
  Matrix<Complex> fftspmat(nchan_/2+1, ninsp, 0.);
  Vector<float> rvecref(nchan_, 0.);
  Vector<Complex> cvecref(nchan_/2+1, 0.);
  uInt icol = 0;
  unsigned int nreject = 0;

#ifdef KS_DEBUG
  cout << "Starting initial FFT. The number of input spectra = " << ninsp << endl;
  cout << "out matrix has ncolumn = " << outmat.ncolumn() << endl;
#endif

  for (uInt isp = 0 ; isp < ninsp ; isp++) {
    rvecref.reference( specmat.column(isp) );
    cvecref.reference( fftspmat.column(isp) );

#ifdef KS_DEBUG
    minMax(minval, maxval, rvecref);
    cout << "Max/Min of inv FFTed Matrix = (max: " << maxval << ", min: " << minval << ")" << endl;
#endif

    fftsf.fft0(cvecref, rvecref, true);

#ifdef KS_DEBUG
    double maxr=cvecref[0].real(), minr=cvecref[0].real(),
      maxi=cvecref[0].imag(), mini=cvecref[0].imag();
    for (uInt i = 1; i<cvecref.size();i++){
      maxr = max(maxr, cvecref[i].real());
      maxi = max(maxi, cvecref[i].imag());
      minr = min(minr, cvecref[i].real());
      mini = min(mini, cvecref[i].imag());
    }
    cout << "Max/Min of inv FFTed Matrix (size=" << cvecref.size() << ") = (max: " << maxr << " + " << maxi << "j , min: " << minr << " + " << mini << "j)" << endl;
#endif
  }

  //Liberate from reference
  rvecref.unique();

  Vector<Complex> cspec(nchan_/2+1, 0.);
  const double PI = 6.0 * asin(0.5);
  const double nchani = 1. / (float) nchan_;
  const Complex trans(0., 1.);
#ifdef KS_DEBUG
  cout << "starting actual deconvolution" << endl;
#endif
  for (uInt j = 0 ; j < ninsp ; j++) {
    for (uInt k = j+1 ; k < ninsp ; k++) {
      const double dx = (shiftvec[k] - shiftvec[j]) * 2. * PI * nchani;

#ifdef KS_DEBUG
      cout << "icol = " << icol << endl;
#endif

      for (uInt ichan = 0 ; ichan < cspec.size() ; ichan++){
	cspec[ichan] = ( fftspmat(ichan, j) + fftspmat(ichan, k) )*0.5;
	double phase = dx*ichan;
	if ( fabs( sin(phase) ) > threshold){
	  cspec[ichan] += ( fftspmat(ichan, j) - fftspmat(ichan, k) ) * 0.5
	    * trans * sin(phase) / ( 1. - cos(phase) );
	} else {
	  nreject++;
	}
      } // end of channel loop

#ifdef KS_DEBUG
      cout << "done calculation of cspec" << endl;
#endif

      Vector<Float> rspec;
      rspec.reference( outmat.column(icol) );

#ifdef KS_DEBUG
      cout << "Starting inverse FFT. icol = " << icol << endl;
      //cout << "- size of complex vector = " << cspec.size() << endl;
      double maxr=cspec[0].real(), minr=cspec[0].real(),
	maxi=cspec[0].imag(), mini=cspec[0].imag();
      for (uInt i = 1; i<cspec.size();i++){
	maxr = max(maxr, cspec[i].real());
	maxi = max(maxi, cspec[i].imag());
	minr = min(minr, cspec[i].real());
	mini = min(mini, cspec[i].imag());
      }
      cout << "Max/Min of conv vector (size=" << cspec.size() << ") = (max: " << maxr << " + " << maxi << "j , min: " << minr << " + " << mini << "j)" << endl;
#endif

      fftsi.fft0(rspec, cspec, false);

#ifdef KS_DEBUG
      //cout << "- size of inversed real vector = " << rspec.size() << endl;
      minMax(minval, maxval, rspec);
      cout << "Max/Min of inv FFTed Vector (size=" << rspec.size() << ") = (max: " << maxval << ", min: " << minval << ")" << endl;
      //cout << "Done inverse FFT. icol = " << icol << endl;
#endif

      icol++;
    }
  }

#ifdef KS_DEBUG
  minMax(minval, maxval, outmat);
  cout << "Max/Min of inv FFTed Matrix = (max: " << maxval << ", min: " << minval << ")" << endl;
#endif

  os << "Threshold = " << threshold << ", Rejected channels = " << nreject << endl;
};


void STSideBandSep::aggregateMat(Matrix<float> &inmat,
				 vector<float> &outvec)
{
  LogIO os(LogOrigin("STSideBandSep","aggregateMat()", WHERE));
  if (inmat.nrow() != nchan_)
    throw(AipsError("Internal error. The row numbers of input matrix differs from nchan_"));
//   if (outvec.size() != nchan_)
//     throw(AipsError("Internal error. The size of output vector should be equal to nchan_"));

  os << "Averaging " << inmat.ncolumn() << " spectra in the input matrix."
     << LogIO::POST;

  const uInt nspec = inmat.ncolumn();
  const double scale = 1./( (double) nspec );
  // initialize values with 0
  outvec.assign(nchan_, 0);
  for (uInt isp = 0 ; isp < nspec ; isp++) {
    for (uInt ich = 0 ; ich < nchan_ ; ich++) {
      outvec[ich] += inmat(ich, isp);
    }
  }

  vector<float>::iterator iter;
  for (iter = outvec.begin(); iter != outvec.end(); iter++){
    *iter *= scale;
  }
};

void STSideBandSep::subtractFromOther(const Matrix<float> &shiftmat,
				      const vector<float> &invec,
				      const vector<double> &shift,
				      vector<float> &outvec)
{
  LogIO os(LogOrigin("STSideBandSep","subtractFromOther()", WHERE));
  if (shiftmat.nrow() != nchan_)
    throw(AipsError("Internal error. The row numbers of input matrix differs from nchan_"));
  if (invec.size() != nchan_)
    throw(AipsError("Internal error. The length of input vector should be nchan_"));
  if (shift.size() != shiftmat.ncolumn())
    throw(AipsError("Internal error. The column numbers of input matrix != the number of elements in shift"));

  const uInt nspec = shiftmat.ncolumn();
  Vector<float> subsp(nchan_, 0.), shiftsub;
  Matrix<float> submat(nchan_, nspec, 0.);
  vector<float>::iterator iter;
  for (uInt isp = 0 ; isp < nspec ; isp++) {
    for (uInt ich = 0; ich < nchan_ ; ich++) {
      subsp(ich) = shiftmat(ich, isp) - invec[ich];
    }
    shiftsub.reference(submat.column(isp));
    shiftSpectrum(subsp, shift[isp], shiftsub);
  }

  aggregateMat(submat, outvec);
};


void STSideBandSep::setLO1(const string lo1, const string frame,
			   const double reftime, const string refdir)
{
  Quantum<Double> qfreq;
  readQuantity(qfreq, String(lo1));
  lo1Freq_ = qfreq.getValue("Hz");
  MFrequency::getType(loFrame_, frame);
  loTime_ = reftime;
  loDir_ = refdir;

#ifdef KS_DEBUG
  cout << "STSideBandSep::setLO1" << endl;
  if (lo1Freq_ > 0.)
    cout << "lo1 = " << lo1Freq_ << " [Hz] (" << frame << ")" << endl;
  if (loTime_ > 0.)
    cout << "ref time = " << loTime_ << " [day]" << endl;
  if (!loDir_.empty())
    cout << "ref direction = " << loDir_ << " [day]" << endl;
#endif
};

void STSideBandSep::setLO1Root(string name)
{
   LogIO os(LogOrigin("STSideBandSep","setLO1Root()", WHERE));
   os << "Searching for '" << name << "'..." << LogIO::POST;
  // Check for existance of the file
  if (!checkFile(name)) {
     throw(AipsError("File does not exist"));
  }
  if (name[(name.size()-1)] == '/')
    name = name.substr(0,(name.size()-2));

  if (checkFile(name+"/Receiver.xml", "file") &&
      checkFile(name+"/SpectralWindow.xml", "file")){
    os << "Found '" << name << "/Receiver.xml' ... got an ASDM name." << LogIO::POST;
    asdmName_ = name;
  } else if (checkFile(name+"/ASDM_RECEIVER") &&
	     checkFile(name+"/ASDM_SPECTRALWINDOW")){
    os << "Found '" << name << "/ASDM_RECEIVER' ... got a Table name." << LogIO::POST;
    asisName_ = name;
  } else {
    throw(AipsError("Invalid file name. Set an MS or ASDM name."));
  }

#ifdef KS_DEBUG
  cout << "STSideBandSep::setLO1Root" << endl;
  if (!asdmName_.empty())
    cout << "asdm name = " << asdmName_ << endl;
  if (!asisName_.empty())
    cout << "MS name = " << asisName_ << endl;
#endif
};


void STSideBandSep::solveImageFrequency()
{
  LogIO os(LogOrigin("STSideBandSep","solveImageFreqency()", WHERE));
  os << "Start calculating frequencies of image side band" << LogIO::POST;

  if (imgTab_p.null())
    throw AipsError("STSideBandSep::solveImageFreqency - an image side band scantable should be set first");

  // Convert frequency REFVAL to the value in frame of LO.
  // The code assumes that imgTab_p has only an IF and only a FREQ_ID
  // is associated to an IFNO
  // TODO: More complete Procedure would be
  // 1. Get freq IDs associated to sigIfno_
  // 2. Get freq information of the freq IDs
  // 3. For each freqIDs, get freq infromation in TOPO and an LO1 
  //    frequency and calculate image band frequencies.
  STFrequencies freqTab_ = imgTab_p->frequencies();
  // get the base frame of table
  const MFrequency::Types tabframe = freqTab_.getFrame(true);
  TableVector<uInt> freqIdVec( imgTab_p->table(), "FREQ_ID" );
  // assuming single freqID per IFNO
  uInt freqid = freqIdVec(0);
  int nChan = imgTab_p->nchan(imgTab_p->getIF(0));
  double refpix, refval, increment ;
  freqTab_.getEntry(refpix, refval, increment, freqid);
  //MFrequency sigrefval = MFrequency(MVFrequency(refval),tabframe);
  // get freq infromation of sigIfno_ in loFrame_
  const MPosition mp = imgTab_p->getAntennaPosition();
  MEpoch me;
  MDirection md;
  if (loTime_ < 0.)
    me = imgTab_p->getEpoch(-1);
  else
    me = MEpoch(MVEpoch(loTime_));
  if (loDir_.empty()) {
    ArrayColumn<Double> srcdirCol_;
    srcdirCol_.attach(imgTab_p->table(), "SRCDIRECTION");
    // Assuming J2000 and SRCDIRECTION in unit of rad
    Quantum<Double> srcra = Quantum<Double>(srcdirCol_(0)(IPosition(1,0)), "rad");
    Quantum<Double> srcdec = Quantum<Double>(srcdirCol_(0)(IPosition(1,1)), "rad");
    md = MDirection(srcra, srcdec, MDirection::J2000);
    //imgTab_p->getDirection(0);
  } else {
    // parse direction string
    string::size_type pos0 = loDir_.find(" ");
    
    if (pos0 == string::npos) {
      throw AipsError("bad string format in LO1 direction");
    }
    string::size_type pos1 = loDir_.find(" ", pos0+1);
    String sepoch, sra, sdec;
    if (pos1 != string::npos) {
      sepoch = loDir_.substr(0, pos0);
      sra = loDir_.substr(pos0+1, pos1-pos0);
      sdec = loDir_.substr(pos1+1);
    }
    MDirection::Types epoch;
    MDirection::getType(epoch, sepoch);
    QuantumHolder qh ;
    String err ;
    qh.fromString( err, sra);
    Quantum<Double> ra = qh.asQuantumDouble() ;
    qh.fromString( err, sdec ) ;
    Quantum<Double> dec = qh.asQuantumDouble() ;
    //md = MDirection(ra.getValue("rad"), dec.getValue("rad"),epoch);
    md = MDirection(ra, dec, epoch);
  }
  MeasFrame mframe( me, mp, md );
  MFrequency::Convert tobframe(loFrame_, MFrequency::Ref(tabframe, mframe));
  MFrequency::Convert toloframe(tabframe, MFrequency::Ref(loFrame_, mframe));
  // Convert refval to loFrame_
  double sigrefval;
  if (tabframe == loFrame_)
    sigrefval = refval;
  else
    sigrefval = toloframe(Quantum<Double>(refval, "Hz")).get("Hz").getValue();

  // Check for the availability of LO1
  if (lo1Freq_ > 0.) {
    os << "Using user defined LO1 frequency." << LogIO::POST;
  } else if (!asisName_.empty()) {
      // MS name is set.
    os << "Using user defined MS (asis): " << asisName_ << LogIO::POST;
    if (!getLo1FromAsisTab(asisName_, sigrefval, refpix, increment, nChan)) {
      throw AipsError("Failed to get LO1 frequency from MS");
    }
  } else if (!asdmName_.empty()) {
      // ASDM name is set.
    os << "Using user defined ASDM: " << asdmName_ << LogIO::POST;
    if (!getLo1FromAsdm(asdmName_, sigrefval, refpix, increment, nChan)) {
      throw AipsError("Failed to get LO1 frequency from ASDM");
    }
  } else {
    // Try getting ASDM name from scantable header
    os << "Try getting information from scantable header" << LogIO::POST;
    if (!getLo1FromScanTab(tableList_[0], sigrefval, refpix, increment, nChan)) {
      //throw AipsError("Failed to get LO1 frequency from asis table");
      os << LogIO::WARN << "Failed to get LO1 frequency using information in scantable." << LogIO::POST;
      os << LogIO::WARN << "Could not fill frequency information of IMAGE sideband properly." << LogIO::POST;
      os << LogIO::WARN << "Storing values of SIGNAL sideband in FREQUENCIES table" << LogIO::POST;
      return;
    }
  }

  // LO1 should now be ready.
  if (lo1Freq_ < 0.)
    throw(AipsError("Got negative LO1 Frequency"));

  // Print summary
  {
    // LO1
    Vector<Double> dirvec = md.getAngle(Unit(String("rad"))).getValue();
    os << "[LO1 settings]" << LogIO::POST;
    os << "- Frequency: " << lo1Freq_ << " [Hz] ("
       << MFrequency::showType(loFrame_) << ")" << LogIO::POST;
    os << "- Reference time: " << me.get(Unit(String("d"))).getValue()
       << " [day]" << LogIO::POST;
    os << "- Reference direction: [" << dirvec[0] << ", " << dirvec[1]
       << "] (" << md.getRefString() << ") " << LogIO::POST;

    // signal sideband
    os << "[Signal side band]" << LogIO::POST;
    os << "- IFNO: " << imgTab_p->getIF(0) << " (FREQ_ID = " << freqid << ")"
       << LogIO::POST;
    os << "- Reference value: " << refval << " [Hz] ("
       << MFrequency::showType(tabframe) << ") = "
       << sigrefval << " [Hz] (" <<  MFrequency::showType(loFrame_)
       << ")" << LogIO::POST;
    os << "- Reference pixel: " << refpix  << LogIO::POST;
    os << "- Increment: " << increment << " [Hz]" << LogIO::POST;
  }

  // Calculate image band incr and refval in loFrame_
  Double imgincr = -increment;
  Double imgrefval = 2 * lo1Freq_ - sigrefval;
  Double imgrefval_tab = imgrefval;
  // Convert imgrefval back to table base frame
  if (tabframe != loFrame_)
    imgrefval = tobframe(Quantum<Double>(imgrefval, "Hz")).get("Hz").getValue();
  // Set new frequencies table
  uInt fIDnew = freqTab_.addEntry(refpix, imgrefval, imgincr);
  // Update FREQ_ID in table.
  freqIdVec = fIDnew;

  // Print summary (Image sideband)
  {
    os << "[Image side band]" << LogIO::POST;
    os << "- IFNO: " << imgTab_p->getIF(0) << " (FREQ_ID = " << freqIdVec(0)
       << ")" << LogIO::POST;
    os << "- Reference value: " << imgrefval << " [Hz] ("
       << MFrequency::showType(tabframe) << ") = "
       << imgrefval_tab << " [Hz] (" <<  MFrequency::showType(loFrame_)
       << ")" << LogIO::POST;
    os << "- Reference pixel: " << refpix  << LogIO::POST;
    os << "- Increment: " << imgincr << " [Hz]" << LogIO::POST;
  }
};

Bool STSideBandSep::checkFile(const string name, string type)
{
  File file(name);
  if (!file.exists()){
    return false;
  } else if (type.empty()) {
    return true;
  } else {
    // Check for file type
    switch (tolower(type[0])) {
    case 'f':
      return file.isRegular(True);
    case 'd':
      return file.isDirectory(True);
    case 's':
      return file.isSymLink();
    default:
      throw AipsError("Invalid file type. Available types are 'file', 'directory', and 'symlink'.");
    }
  }
};

bool STSideBandSep::getLo1FromAsdm(const string asdmname,
				   const double /*refval*/,
				   const double /*refpix*/,
				   const double /*increment*/,
				   const int /*nChan*/)
{
  // Check for relevant tables.
  string spwname = asdmname + "/SpectralWindow.xml";
  string recname = asdmname + "/Receiver.xml";
  if (!checkFile(spwname) || !checkFile(recname)) {
    throw(AipsError("Could not find subtables in ASDM"));
  }

  return false;

};


bool STSideBandSep::getLo1FromScanTab(CountedPtr< Scantable > &scantab, 
				      const double refval,
				      const double refpix,
				      const double increment,
				      const int nChan)
{
  LogIO os(LogOrigin("STSideBandSep","getLo1FromScanTab()", WHERE));
  // Check for relevant tables.
  const TableRecord &rec = scantab->table().keywordSet() ;
  String spwname, recname;
  if (rec.isDefined("ASDM_SPECTRALWINDOW") && rec.isDefined("ASDM_RECEIVER")){
    spwname = rec.asString("ASDM_SPECTRALWINDOW");
    recname = rec.asString("ASDM_RECEIVER");
  }
  else {
    // keywords are not there
    os << LogIO::WARN
       << "Could not find necessary table names in scantable header."
       << LogIO::POST;
    return false;
  }
  if (!checkFile(spwname,"directory") || !checkFile(recname,"directory")) {
    throw(AipsError("Could not find relevant subtables in MS"));
  }
  // Get root MS name
  string msname;
  const String recsuff = "/ASDM_RECEIVER";
  String::size_type pos;
  pos = recname.size()-recsuff.size();
  if (recname.substr(pos) == recsuff)
    msname = recname.substr(0, pos);
  else
    throw(AipsError("Internal error in parsing table name from a scantable keyword."));

  if (!checkFile(msname))
    throw(AipsError("Internal error in parsing MS name from a scantable keyword."));

  return getLo1FromAsisTab(msname, refval, refpix, increment, nChan);

};

bool STSideBandSep::getLo1FromAsisTab(const string msname,
				      const double refval,
				      const double refpix,
				      const double increment,
				      const int nChan)
{
  LogIO os(LogOrigin("STSideBandSep","getLo1FromAsisTab()", WHERE));
  os << "Searching an LO1 frequency in '" << msname << "'" << LogIO::POST;
  // Check for relevant tables.
  const string spwname = msname + "/ASDM_SPECTRALWINDOW";
  const string recname = msname + "/ASDM_RECEIVER";
  if (!checkFile(spwname,"directory") || !checkFile(recname,"directory")) {
    throw(AipsError("Could not find relevant tables in MS"));
  }

  Table spwtab_ = Table(spwname);
  String asdmSpw;
  ROTableRow spwrow(spwtab_);
  const Double rtol = 0.01;
  for (uInt idx = 0; idx < spwtab_.nrow(); idx++){
    const TableRecord& rec = spwrow.get(idx);
    // Compare nchan
    if (rec.asInt("numChan") != (Int) nChan)
      continue;
    // Compare increment
    Double asdminc;
    Array<Double> incarr = rec.asArrayDouble("chanWidthArray");
    if (incarr.size() > 0)
      asdminc = incarr(IPosition(1, (uInt) refpix));
    else
      asdminc = rec.asDouble("chanWidth");
    if (abs(asdminc - abs(increment)) > rtol * abs(increment))
      continue;
    // Compare refval
    Double asdmrefv;
    Array<Double> refvarr = rec.asArrayDouble("chanFreqArray");
    if (refvarr.size() > 0){
      const uInt iref = (uInt) refpix;
      const Double ratio = refpix - (Double) iref;
      asdmrefv = refvarr(IPosition(1, iref))*(1.-ratio)
	+ refvarr(IPosition(1,iref+1))*ratio;
    }
    else {
      const Double ch0 = rec.asDouble("chanFreqStart");
      const Double chstep = rec.asDouble("chanFreqStep");
      asdmrefv = ch0 + chstep * refpix;
    }
    if (abs(asdmrefv - refval) < 0.5*abs(asdminc)){
      asdmSpw = rec.asString("spectralWindowId");
      break;
    }
  }

  if (asdmSpw.empty()){
    os << LogIO::WARN << "Could not find relevant SPW ID in " << spwname << LogIO::POST;
    return false;
  }
  else {
    os << asdmSpw << " in " << spwname
       << " matches the freqeuncies of signal side band." << LogIO::POST;
  }

  Table rectab_ = Table(recname);
  ROTableRow recrow(rectab_);
  for (uInt idx = 0; idx < rectab_.nrow(); idx++){
    const TableRecord& rec = recrow.get(idx);
    if (rec.asString("spectralWindowId") == asdmSpw){
      const Array<Double> loarr = rec.asArrayDouble("freqLO");
      lo1Freq_ = loarr(IPosition(1,0));
      os << "Found LO1 Frequency in " << recname << ": "
	 << lo1Freq_ << " [Hz]" << LogIO::POST;
      return true;
    }
  }
  os << LogIO::WARN << "Could not find " << asdmSpw << " in " << recname
     << LogIO::POST;
  return false;
};

} //namespace asap
