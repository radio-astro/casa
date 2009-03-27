/***
 * Framework independent implementation file for calibrater...
 *
 *   Implemention of the calibrater component
 *   for header generated from calibrater.xml.
 * 
 *
 * @author Raymond Rusk
 * @version $Id$
 ***/

#include <iostream>
#include <xmlcasa/synthesis/calibrater_cmpt.h>
#include <synthesis/MeasurementComponents/Calibrater.h>
#include <ms/MeasurementSets.h>
#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>

#include <casa/Containers/Record.h>
#include <casa/Containers/SimOrdMap.h>

#include <casa/Quanta/QC.h>
#include <casa/Utilities/Regex.h>
//#include <casa/BasicSL/Constants.h>
#include <casa/OS/File.h>
#include <casa/OS/SymLink.h>
#include <ms/MeasurementSets/MSRange.h>

#include <measures/Measures/MeasTable.h>
#include <iostream>

using namespace std;
using namespace casa;

namespace casac {

calibrater::calibrater() : 
  itsMS(0)
{
  // Default constructor
  //    itsApplyMap   SimpleOrderedMap   Cal. table apply assignments
  //    itsSolveMap   SimpleOrderedMap   Cal. table solve assignments
  itsLog = new casa::LogIO();
  itsCalibrater = new casa::Calibrater();
}

calibrater::~calibrater()
{
  if(itsMS) delete itsMS;
  delete itsCalibrater;
  delete itsLog;
}

bool calibrater::open(const std::string& filename, 
		 const bool compress)
{
  bool rstat(False);
  try {
    LogIO os (LogOrigin ("calibrater", "open"));
    os << "Opening MS: " 
       << filename
       << " for calibration."
       << LogIO::POST;
    if(itsMS)
      delete itsMS;
    itsMS = new MeasurementSet(String(filename),
			       TableLock(TableLock::AutoLocking),
			       Table::Update);
    AlwaysAssert(itsMS, AipsError);
    rstat = itsCalibrater->initialize(*itsMS, compress);

    // Open LogSink for MS History table logging
    logSink_p=LogSink(LogMessage::NORMAL1, False);
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}


bool 
calibrater::selectvis(const ::casac::variant& time,
		      const ::casac::variant& spw, 
		      const ::casac::variant& scan,
		      const ::casac::variant& field,
		      const ::casac::variant& baseline,
		      const ::casac::variant& uvrange,
		      const std::string& chanmode,
		      const int nchan,
		      const int start,
		      const int step,
		      const Quantity& mstart,
		      const Quantity& mstep,
		      const std::string& msselect) {

  Bool rstat(False);
  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return false;
  }
  try {
    // Set up history logging infrastructure
    logSink_p.clearLocally();
    LogIO os(LogOrigin("calibrater", "setdata"), logSink_p);
    os << "Beginning selectvis--(MSSelection version)-------" << LogIO::POST;
    
    MRadialVelocity mmStart = new MRadialVelocity(casaQuantity(mstart));
    MRadialVelocity mmStep = new MRadialVelocity(casaQuantity(mstep));
    
    // run reset because setdata is going to delete itsCI's VisSet,
    //  which existing VisJones objects rely upon
    reset(True,True);

  /*
    casac::variant s(std::string(""));
    casac::variant v("");
    casac::variant x(::casac::initialize_variant(""));
    std::cout << s.typeString() << ": " << s.toString() << std::endl;
    std::cout << v.typeString() << ": " << v.toString() << std::endl;
    std::cout << x.typeString() << ": " << x.toString() << std::endl;

    std::cout << time.typeString() << ": " << time.toString() << std::endl;
    std::cout << field.typeString() << ": " << field.toString() << std::endl;
  */


    // Selection pars as Strings:
    String timeS=toCasaString(time);
    //    cout << "timsS     = " << timeS << " " << time.typeString() << endl;
    String spwS=toCasaString(spw);
    //    cout << "spwS      = " << spwS << " " << spw.typeString() << endl;
    String scanS=toCasaString(scan);
    //    cout << "scanS     = " << scanS << " " << scan.typeString() << endl;
    String fieldS=toCasaString(field);
    //    cout << "fieldS    = " << fieldS << " " << field.typeString() << endl;
    String baselineS=toCasaString(baseline);
    //    cout << "baselineS = " << baselineS << " " << baseline.typeString() << endl;
    String uvrangeS=toCasaString(uvrange);
    //    cout << "uvrangeS  = " << uvrangeS << " " << uvrange.typeString() << endl;

    // Invoke selectvis on the calibrater object
    itsCalibrater->selectvis(timeS,
			     spwS,
			     scanS,
			     fieldS,
			     baselineS,
			     uvrangeS,
			     chanmode, 
			     Int(nchan), Int(start),Int(step), 
			     mmStart, mmStep, 
			     msselect);

    // Log parameters to HISTORY table
    ostringstream o;
    o <<  "chanmode=" << chanmode << " nchan="  << nchan  << " start=" << start
      << " step=" << step << " mStart='" << mmStart.getValue()
      << (mmStart.getUnit()).getName() << "' mStep='" << mmStep.getValue()
      << (mmStep.getUnit()).getName() << "' msSelect="
      << "'" << msselect << "'";
    os << o.str();
    itsCalibrater->writeHistory(os,True);
    rstat = True;
  } catch  (AipsError x) {

    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
calibrater::setmodel(const std::string& modelImage)
{
  try
    {
      itsCalibrater->setmodel(modelImage);
    }
  catch (AipsError x)
    {
      *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
    }
  return true;
}

bool 
calibrater::setapply(const std::string& type,
		     const double t,
		     const std::string& table,
		     const ::casac::variant& field,
		     const std::string& interp,
		     const std::string& select,
		     const bool calwt, 
		     const std::vector<int>& spwmap,
		     const double opacity) {

  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return false;
  }
  try {

    LogIO os(LogOrigin("calibrater", "setapply"));
    os << "Beginning setapply--(MSSelection version)-------" << LogIO::POST;

    // Forward to the Calibrater object (no spw sel yet)
    itsCalibrater->setapply(type,t,table,
			    "",toCasaString(field),
			    interp,calwt,spwmap,opacity);
    
  } catch(AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return true;
}


bool 
calibrater::setsolve(const std::string& type, 
		     const ::casac::variant& t,
		     const std::string& table, 
		     const bool append, 
		     const double preavg, 
		     const bool phaseonly, 
		     const std::string& apmode,
		     const ::casac::variant& refant,
		     const bool solnorm,
		     const float minsnr,
		     const std::string& combine,
		     const int fillgaps,
		     const std::string& cfcache,
		     const float painc)
{
  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return false;
  }
  try {

    LogIO os(LogOrigin("calibrater", "setsolve"));
    os << "Beginning setsolve--(MSSelection version)-------" << LogIO::POST;
    
    // Remind phaseonly users that they should use apmode:
    String mode=apmode;
    if (phaseonly) {
      os << LogIO::WARN
	 << "Please note that the phaseonly parameter will soon be eliminated."
	 << endl << "You should begin using apmode='P' for phase-only." 
	 << LogIO::POST;
      mode="P";
    }

    // Forward to Calibrater object
    itsCalibrater->setsolve(type,toCasaString(t),table,append,preavg,mode,
			    toCasaString(refant),solnorm,minsnr,combine,fillgaps,
			    cfcache,painc);
    
  } catch(AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return true;
}



bool 
calibrater::setsolvegainspline(const std::string& table, 
			       const bool append,
			       const std::string& mode,
			       const double splinetime,
			       const double preavg,
			       const int npointaver,
			       const double phasewrap,
			       const ::casac::variant& refant)
{

  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return false;
  }

  try {

    LogIO os(LogOrigin("calibrater", "setsolvegainspline"));
    os << "Beginning setsolvegainspline--(MSSelection version)-------" << LogIO::POST;
    
    // Forward to Calibrater
    itsCalibrater->setsolvegainspline(table,append,mode,splinetime,
				      preavg,npointaver,phasewrap,
				      toCasaString(refant));

  } catch(AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return true;
}



bool calibrater::setsolvebandpoly(const std::string& table,
				  const bool append, 
				  const ::casac::variant& t,
				  const std::string& combine,
				  const int degamp, 
				  const int degphase, 
				  const bool visnorm, 
				  const bool solnorm, 
				  const int maskcenter, 
				  const double maskedge, 
				  const ::casac::variant& refant)
{
  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return false;
  }

  try {

    LogIO os(LogOrigin("calibrater", "setsolvebandpoly"));
    os << "Beginning setsolvebandpoly--(MSSelection version)-------" << LogIO::POST;

    Vector<Int> deg(2);
    deg(0)=degamp;
    deg(1)=degphase;

    // Forward to Calibrater object
    itsCalibrater->setsolvebandpoly(table,append,
				    toCasaString(t),combine,
				    deg,visnorm,solnorm,
				    maskcenter,maskedge,
				    toCasaString(refant));
    
  } catch(AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return true;
}


bool
calibrater::state()
{
  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return false;
  }

  try {

    LogIO os(LogOrigin("calibrater", "state"));
    //    os << "Beginning state-----------------------------" << LogIO::POST;

    // Forward to Calibrater object
    itsCalibrater->state();

  } catch(AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return true;
}

bool
calibrater::reset(const bool apply, const bool solve)
{
  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return false;
  }

  try {

    LogIO os(LogOrigin("calibrater", "reset"));
    os << "Reseting solve/apply state" << LogIO::POST;

    // Forward to Calibrater object:
    itsCalibrater->reset(apply,solve);

    // Report (now empty) state:
    //    state();
    
  } catch(AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return true;
}

bool
calibrater::initcalset(const int calset)
{
  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return false;
  }

  try {
    
    // Set up history logging infrastructure
    logSink_p.clearLocally();
    LogIO os(LogOrigin("calibrater", "initcalset"), logSink_p);
    os << "Beginning initcalset------------------------" << LogIO::POST;
    
    // Re-initialize cal scratch columns
    itsCalibrater->initCalSet(calset);
    
  } catch(AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return true;
}

bool
calibrater::solve()
{
  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return false;
  }

// Solve for the specified calibration components
//
   Bool retval = True;

   try {

     logSink_p.clearLocally();
     LogIO os (LogOrigin ("calibrater", "solve"), logSink_p);
     os << "Beginning solve-----------------------------" << LogIO::POST;
     // Update HISTORY table
     itsCalibrater->writeHistory(os);

     // Forward to Calibrater object:
     retval=itsCalibrater->solve();

     os << "Finished solving." << LogIO::POST;

   } catch(AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
   }
   return retval;
}

bool
calibrater::correct()
{
  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return false;
  }

// Apply the calibration to update the CORRECTED_DATA column in the MS
//
   Bool retval = True;

   try {

     logSink_p.clearLocally();
     LogIO os (LogOrigin ("calibrater", "correct"), logSink_p);
     os << "Beginning correct---------------------------" << LogIO::POST;
     // Update HISTORY table
     itsCalibrater->writeHistory(os);

     // Apply the calibration solutions to the uv-data
     retval = itsCalibrater->correct();
     //     AlwaysAssert (retval, AipsError);

     os << "Finished correcting." << LogIO::POST;
   
   } catch(AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
     RETHROW(x);
   }
   return retval;
}

bool
calibrater::corrupt()
{
  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return false;
  }

// Apply the calibration to corrupt the MODEL_DATA column in the MS
//
   Bool retval = True;

   try {

     logSink_p.clearLocally();
     LogIO os (LogOrigin ("calibrater", "corrupt"), logSink_p);
     os << "Beginning corrupt---------------------------" << LogIO::POST;
     // Update HISTORY table
     itsCalibrater->writeHistory(os);

     // Apply the calibration solutions to the uv-data
     retval = itsCalibrater->corrupt();
     //     AlwaysAssert (retval, AipsError);

     os << "Finished corrupting." << LogIO::POST;
   
   } catch(AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
     RETHROW(x);
   }
   return retval;
}


void
uniqueStrV(std::vector<string> &ulist, const std::vector<string> &list) {
  // for translating Glish's list:=unique(list);
  ulist = std::vector<string>(); // make sure ulist is empty
  vector<string> slist(list);
  sort(slist.begin(),slist.end());
  vector<string>::iterator new_end = unique(slist.begin(),slist.end());
  vector<string>::iterator li;
  for (li=slist.begin(); li != new_end; ++li) ulist.push_back(*li);
}

void
uniqueIntV(std::vector<int> &ulist, const Vector<Int> &list) {
  // for translating Glish's list:=unique(list);
  ulist = std::vector<int>(); // make sure ulist is empty
  int m = list.size();
  vector<int> slist(m);
  for (int i = 0; i < m; ++i) slist.push_back(list[i]);
  sort(slist.begin(),slist.end());
  vector<int>::iterator new_end = unique(slist.begin(),slist.end());
  vector<int>::iterator li;
  for (li=slist.begin(); li != new_end; ++li) ulist.push_back(*li);
}

//----------------------------------------------------------------------------
//Fluxscale - bootstrap the flux density scale from std. amplitude calibrators
bool 
calibrater::fluxscale(std::vector<double> & fluxd,
		      const std::string& tablein,
		      const ::casac::variant& reference,
		      const std::string& tableout,
		      const ::casac::variant& transfer,
		      const bool append, 
		      const std::vector<int>& refspwmap)
{
  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return false;
  }

  try {

    // Write parameters to HISTORY log
    logSink_p.clearLocally();
    LogIO os(LogOrigin("calibrater", "fluxscale"), logSink_p);
    os << "Beginning fluxscale--(MSSelection version)-------" << LogIO::POST;
    
    // Forward to Calibrater object:
    Matrix<Double> fluxScaleFactor;
    itsCalibrater->fluxscale(tablein,tableout,
			     toCasaString(reference),
			     refspwmap,
			     toCasaString(transfer),
			     append,fluxScaleFactor);
    
    // Do something about output
    //  fluxd.reference(fd.reform(IPosition(1,fd.nelements())));
    fluxd = std::vector<double>();
    for (uInt j=0; j<fluxScaleFactor.ncolumn(); j++) {
      for (uInt i=0; i<fluxScaleFactor.nrow(); i++) {
	fluxd.push_back(fluxScaleFactor(i,j));
      }
    }
    
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return true;
}

bool
calibrater::accumulate(const std::string& tablein, 
		       const std::string& incrtable,
		       const std::string& tableout,
		       const ::casac::variant& field,
		       const ::casac::variant& calfield,
		       const std::string& interp, 
		       const double t,
		       const std::vector<int>& spwmap)
{

  // TBD: rationalize with Calibrater::accumulate (i.e., move most
  //      of this stuff into Calibrater)

  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return false;
  }

  try {

    logSink_p.clearLocally();
    LogIO os (LogOrigin ("calibrater", "accumulate"), logSink_p);
    os << "Beginning accumulate--(MSSelection version)-------" << LogIO::POST;
     
    // If no table specified, accumulate in place
    // if (tableout=="") tableout:=tablein;;
    String tableOut;
    if (tableout == "")
      tableOut = tablein;
    else
      tableOut = tableout;
    
    itsCalibrater->accumulate(tablein,incrtable,tableOut,
			      toCasaString(field),
			      toCasaString(calfield),
			      interp,t,spwmap);
    
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return true;
}


bool
calibrater::smooth(const std::string& tablein, 
		   const std::string& tableout, 
		   const ::casac::variant& field,
		   const std::string& smoothtype,
		   const double smoothtime)
{
  bool rstat(false);

  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return false;
  }

  try {

    logSink_p.clearLocally();
    LogIO os(LogOrigin("calibrater", "smooth"),logSink_p);
    os << "Beginning smooth--(MSSelection version)-------" << LogIO::POST;

    String tabo(tableout);

    rstat = itsCalibrater->smooth(tablein,tabo,
				  smoothtype,smoothtime,
				  toCasaString(field));

  } catch(AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool 
calibrater::listcal(const std::string& tablein, 
		    const ::casac::variant& field, 
		    const ::casac::variant& antenna,
		    const ::casac::variant& spw,
		    const std::string& listfile, 
		    const int pagerows)
{

  Bool rstat(False);
  try {

    rstat=itsCalibrater->listCal(tablein,
				 toCasaString(field),
				 toCasaString(antenna),
				 toCasaString(spw),
				 toCasaString(listfile),
				 pagerows);

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;

}



bool
calibrater::posangcal(const std::vector<double>& posangcor,
		      const std::string& tablein, 
		      const std::string& tableout)
{
  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return false;
  }
  try {

  // Set up history logging infrastructure
  logSink_p.clearLocally();
  LogIO os(LogOrigin("calibrater", "posangcal"), logSink_p);


  throw("posangcal temporarily disabled (2006/11/11)");

  os << "Finished position angle calibration." << LogIO::POST;
  itsCalibrater->writeHistory(os);

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return true;
};  // end of posangcal

bool
calibrater::linpolcor(const std::string& tablein, 
		      const std::string& tableout, 
		      const std::vector<std::string>& fields)
{
  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return false;
  }

  try {

  // LinPolCor - correct the antenna gains for linear polarization of the
  // calibrator. Handles multiple spectral windows and calibrators.
  // Use only for arrays with linear feeds and Alt-Az mounts (e.g., ATCA).

  // Set up history logging infrastructure
  logSink_p.clearLocally();
  LogIO os(LogOrigin("calibrater", "linpolcor"), logSink_p);

  {
    os << LogIO::SEVERE
       << "The linpolcor() function has been disabled while some calibrater infrastructural work is completed."
       << LogIO::POST;
    return false;
  }


  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return true;
}

bool 
calibrater::plotcal(const std::vector<int>& antennas,
                    const std::vector<int>& fields,
                    const std::vector<int>& spwids,
                    const std::string& plottype,
                    const std::string& tablename,
                    const int polarization,
                    const bool multiplot, 
		    const int nx, 
		    const int ny, 
		    const std::string& psfile)
{

  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return false;
  }

  // TODO : IMPLEMENT ME HERE !
  /*
       return self.calutil.plotcal (plottype, tablename, antennas, fields,
          polarization, spwids, multiplot, nx, ny, psfile);
  */
  // TODO : IMPLEMENT ME HERE !
  *itsLog << LogIO::WARN << "Sorry not implemented yet" << LogIO::POST;
  return false;
}

std::vector<double>
calibrater::modelfit(const std::vector<bool>& vary, 
		     const int niter, 
		     const std::string& modeltype, 
		     const std::vector<double>& par, 
		     const std::string& file)
{
  std::vector<double> rstat;
  try {

    logSink_p.clearLocally();
    LogIO os(LogOrigin("calibrater", "modelfit"), logSink_p);
    os << "Beginning modelfit--------------------------" << LogIO::POST;
    
    Vector<Double> thefit = itsCalibrater->modelfit(niter, 
						    modeltype, 
						    par, 
						    vary, 
						    file);
    thefit.tovector(rstat);
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}


bool
calibrater::close()
{
 bool rstat(False);
 try {
    if(itsMS) delete itsMS;
    itsMS = 0;
    delete itsCalibrater;
    //delete itsLog;
    itsCalibrater = new Calibrater();

    rstat = True;
 } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
};

bool
calibrater::done()
{

  // TBD:  is 'new Calibrater()' ok here?

 bool rstat(False);
 try {
    if(itsMS) delete itsMS;
    itsMS = 0;
    delete itsCalibrater;
    //delete itsLog;
    itsCalibrater = new Calibrater();
    rstat = True;
 } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
};


//----------------------------------------------------------------------------


// Private function to look up DATA_DESC_ID's for a given SPW_ID
void calibrater::ddid(std::vector<int>& dd, const int spwid)
{
  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return;
  }

  try {

  // Open the DATA_DESCRIPTION sub-table
  Table ddtab(itsMS->dataDescriptionTableName(), Table::Old);
  ScalarColumn<Int> spwidColumn(ddtab, "SPECTRAL_WINDOW_ID");
  ScalarColumn<Bool> flrowColumn(ddtab, "FLAG_ROW");
  uInt numrow = ddtab.nrow();

  // Iterate through non-flagged rows, to find SPW_ID matches
  dd = std::vector<int>(); // make sure dd is empty
  // *** Is there an off-by-one error here? ***
  for (uInt i=0; i < numrow; i++) {
    if (!flrowColumn(i) && (spwidColumn(i)==Int(spwid) ) ) {
      dd.push_back(i);
    }
  }

  } catch (AipsError x) {

    cout << "OUCH=============================" << endl;

    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
};

//----------------------------------------------------------------------------

// Private function to generate uv-range TAQL selection strings
void calibrater::uvtaql(std::string& uvsel, bool& noselect,
			const std::vector<double>& uvrange)
{ 
  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return;
  }

  try {

  // Set up history logging infrastructure
  logSink_p.clearLocally();
  LogIO os(LogOrigin("calibrater", "uvtaql()"), logSink_p);

  std::vector<double> uvlim(uvrange);
  if (uvrange.size() == 1) {
    uvlim.resize(2);
    uvlim[1]=uvlim[0];
    uvlim[0]=0;
  } else {
    sort(uvlim.begin(),uvlim.end());
  }

  *itsLog << LogIO::NORMAL1 
	  << "Limiting selection to UVRANGE = "
	  << uvlim[0] << "-" << uvlim[1]
	  << " kilolambda." 
	  << LogIO::POST;

  // Change from klambda to lambda
  for(uInt i=0; i < uvlim.size(); ++i) {
    uvlim[i] *= 1000.0;
  }
  if(uvlim.back() > 0) { // i.e., if max(uvlim) > 0
    // Extract the reference frequencies
    Table spwtab(itsMS->spectralWindowTableName(), Table::Old);
    ScalarColumn<Double> reffreq(spwtab, "REF_FREQUENCY");
    uvsel = "( ";
    uInt nfreq=reffreq.nrow();
    Double c = (QC::c).getValue();
    double ffact;
    vector<int> dd;

    for (uInt ispw=0; ispw < nfreq; ++ispw) {
      ffact = reffreq(ispw)/c;
      // Look up the DATA_DESC_ID's for this SPW_ID
      ddid(dd,ispw);
      int ndd = dd.size();
      for (int idd=0; idd < ndd; ++idd) {
	ostringstream o;
	o << "((DATA_DESC_ID==" << dd[idd]
	  << " ) && (SQRT(UVW[1]^2 + UVW[2]^2) > "
	  << uvlim[0]/ffact
	  << " && SQRT(UVW[1]^2 + UVW[2]^2) < "
	  << uvlim[1]/ffact
	  << "))";
	uvsel += o.str();
	if (!(ispw == nfreq-1 && idd == ndd-1)) uvsel += " || ";
      }
      noselect = false;
      os << "Applying a uv-range selection of " << uvlim[0]/1000.0
	 << " to " << uvlim[1]/1000.0 << " klambda";
      itsCalibrater->writeHistory(os,True);
    }
    uvsel += " )";

  } else {
    uvsel = "";
    noselect = true;
  }

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
}

//----------------------------------------------------------------------------

// Private function to pre-process input selection strings
// ??? Is this needed anymore ???
void calibrater::validstring(std::string& outputstring,
			     const std::string inputstring)
{
  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return;
  }

  try {

  // Guard against "" or " " (???)
  outputstring = std::string();   // ensure outputstring is empty
  String instr(inputstring);
  uInt len = instr.length();
  if (len == 0) {
    outputstring = " ";
  } else {
    // Strip spurious start and end quotes
    //  output = output ~ s/^'(.*)'$/$1/;
    //  output = output ~ s/^"(.*)"$/$1/;
    if (instr.matches(Regex("^'.*'$")) ||
	instr.matches(Regex("^\".*\"$"))) {
      for (uInt j = 1; j < len-1; ++j) {
	outputstring.push_back(instr[j]);
      }
    } else {
      outputstring=inputstring;
    }
  }

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
}

//----------------------------------------------------------------------------

// getfldidlist - private function to obtain the field_id for a given field name
bool
calibrater::getfldidlist(vector<int>& fieldids, vector<string>& fieldnames)
{
  if (! itsMS) {
    *itsLog << LogIO::SEVERE << "Must first open a MeasurementSet."
	    << endl << LogIO::POST;
    return false;
  }

  try {

  LogIO os(LogOrigin("calibrater", "getfldidlist()"));

  int nflds = fieldnames.size();          // nflds:=len(fieldnames);
  // empty output list, to start
  fieldids = vector<int>();               // fieldids:=[];
  // t:= table(spaste(self.msfile,'/FIELD'), ack=F);
  String tablename = itsMS->fieldTableName();
  Table t = Table(tablename, Table::Old);
  if (t.isNull()) {                       // if (!is_table(t)) fail;
    os << LogIO::SEVERE << "Table " << tablename
       << " can not be opened." << LogIO::POST;
    return false;
  }
  ScalarColumn<String> names(t, "NAME");  // names:= t.getcol('NAME');
  uInt n = names.nrow();                  // n:= len(names);
  std::vector<int> thisfieldid;
  // for each specified fieldname, find matching indices
  for (int ifld=0; ifld < nflds; ++ifld) { // for (ifld in 1:nflds)
    // thisfield:= fieldnames[ifld] ~ s/\+/\\+/;
    String thisField = fieldnames[ifld];
    // thisField.gsub("+","\\+");
    // thisfieldid:= seq(1:n)[names ~ eval(spaste('m/',thisfield,'/'))];
    thisfieldid=vector<int>();
    for (uInt i=0; i < n; i++) { // 1-based??
      if (names(i).matches(thisField,-thisField.length())) {
	thisfieldid.push_back(i);
      }
    }
    // nfldid:= len(thisfieldid);
    int nfldid = thisfieldid.size();
    // accumulate this field id(s), or abort
    if (nfldid > 0) {
      // fieldids:=[fieldids,thisfieldid];
      for (int i=0; i<nfldid; ++i) {
	fieldids.push_back(thisfieldid[i]);
      }
    } else {
      // return throw(paste('Field: ', thisfield, ' not found'));
      os << LogIO::SEVERE
       << "Field: " << thisField << " not found" << LogIO::POST;
      return false;
    }
  }
  //return fieldids;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return true;
}

//----------------------------------------------------------------------------

// Getid - private function to obtain the field_id for a given field name
int
getid(string& msname, string fieldname)
{
  LogIO os(LogOrigin("calibrater", "getid()"));

  // t:= table(spaste(msname,'/FIELD'), ack=F);
  String tablename = msname + "/FIELD";
  Table t = Table(tablename, Table::Old);
  if (t.isNull()) {  // if (!is_table(t)) fail;
    os << LogIO::SEVERE << "Table " << tablename
       << " can not be opened." << LogIO::POST;
    return -1;
  }
  // names:= t.getcol('NAME');
  ScalarColumn<String> names(t, "NAME");
  // n:= len(names);
  uInt n = names.nrow();
  // fieldname:= fieldname ~ s/\+/\\+/;
  String fieldName(fieldname);
  // fieldName.gsub("+","\\+");
  // fieldids:= seq(1:n)[names ~ eval(spaste('m/',fieldname,'/'))];
  std::vector<int> fieldids;
  for (uInt i=1; i <= n; i++) {
    if (names(i).matches(fieldName))
      fieldids.push_back(i);
  }
  // nfldid:= len(fieldids);
  int nfldid = fieldids.size();
  if (nfldid == 0) {
    // return throw(paste('Field: ', fieldname, ' not found'))
    os << LogIO::SEVERE
       << "Field: " << fieldname << "not found" << LogIO::POST;
    return -1;
  }
  if (nfldid > 1) {
    // return throw(paste('More than one field name matches ',fieldname,
    //                    ': ', names[fieldids]))
    os << LogIO::SEVERE
       << "More than one field name matches " << fieldname << ": ";
    for (int i=0; i< nfldid; i++)
      os << names(fieldids[i]) << " ";
    os << LogIO::POST;
    return -1;
  }

  // return fieldids[1];
  return fieldids[0];
}

//

//----------------------------------------------------------------------------

} // casac namespace
