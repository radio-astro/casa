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
#include "STSideBandSep.h"

using namespace std ;
using namespace casa ;
using namespace asap ;

#ifndef KS_DEBUG
#define KS_DEBUG
#endif

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

  os << ntable_ << " tables are set." << LogIO::POST;
};

STSideBandSep::~STSideBandSep()
{
#ifdef KS_DEBUG
  cout << "Destructor ~STSideBandSep()" << endl;
#endif
};

void STSideBandSep::init()
{
  // frequency setup
  sigIfno_= 0;
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

void STSideBandSep::setFrequency(const unsigned int ifno,
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
    os << " ) [channel]" << LogIO::POST;
  }
};

void STSideBandSep::setThreshold(const double limit)
{
  LogIO os(LogOrigin("STSideBandSep","setThreshold()", WHERE));
  if (limit < 0)
    throw( AipsError("Rejection limit should be positive number.") );

  rejlimit_ = limit;
  os << "Rejection limit is set to " << rejlimit_;
};

// Temporal function to set scantable of image sideband
void STSideBandSep::setImageTable(const ScantableWrapper &s)
{
#ifdef KS_DEBUG
  cout << "STSideBandSep::setImageTable" << endl;
  cout << "got image table nrow = " << s.nrow() << endl;
#endif
  imgTab_p = s.getCP();
  AlwaysAssert(!imgTab_p.null(),AipsError);

};

//
void STSideBandSep::setLO1(const double lo1, const string frame,
			   const double reftime, const string refdir)
{
  lo1Freq_ = lo1;
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

///// TEMPORAL FUNCTION!!! /////
void STSideBandSep::setScanTb0(const ScantableWrapper &s){
  st0_ = s.getCP();
};
////////////////////////////////

void STSideBandSep::solveImageFreqency()
{
#ifdef KS_DEBUG
  cout << "STSideBandSep::solveImageFrequency" << endl;
#endif
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
    if (!getLo1FromScanTab(st0_, sigrefval, refpix, increment, nChan)) {
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

  // Print summary (LO1)
  Vector<Double> dirvec = md.getAngle(Unit(String("rad"))).getValue();
  os << "[LO1 settings]" << LogIO::POST;
  os << "- Frequency: " << lo1Freq_ << " [Hz] ("
     << MFrequency::showType(loFrame_) << ")" << LogIO::POST;
  os << "- Reference time: " << me.get(Unit(String("d"))).getValue()
     << " [day]" << LogIO::POST;
  os << "- Reference direction: [" << dirvec[0] << ", " << dirvec[1]
     << "] (" << md.getRefString() << ") " << LogIO::POST;

  //Print summary (signal)
  os << "[Signal side band]" << LogIO::POST;
  os << "- IFNO: " << imgTab_p->getIF(0) << " (FREQ_ID = " << freqid << ")"
     << LogIO::POST;
  os << "- Reference value: " << refval << " [Hz] ("
     << MFrequency::showType(tabframe) << ") = "
     << sigrefval << " [Hz] (" <<  MFrequency::showType(loFrame_)
     << ")" << LogIO::POST;
  os << "- Reference pixel: " << refpix  << LogIO::POST;
  os << "- Increment: " << increment << " [Hz]" << LogIO::POST;

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

  // Print summary (Image side band)
  os << "[Image side band]" << LogIO::POST;
  os << "- IFNO: " << imgTab_p->getIF(0) << " (FREQ_ID = " << freqIdVec(0)
     << ")" << LogIO::POST;
  os << "- Reference value: " << imgrefval << " [Hz] ("
     << MFrequency::showType(tabframe) << ") = "
     << imgrefval_tab << " [Hz] (" <<  MFrequency::showType(loFrame_)
     << ")" << LogIO::POST;
  os << "- Reference pixel: " << refpix  << LogIO::POST;
  os << "- Increment: " << imgincr << " [Hz]" << LogIO::POST;
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
				   const double refval,
				   const double refpix,
				   const double increment,
				   const int nChan)
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

// String STSideBandSep::()
// {

// };

} //namespace asap
