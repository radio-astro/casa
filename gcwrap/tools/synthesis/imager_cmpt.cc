/***
 * Framework independent implementation file for imager...
 *
 * Implement the imager component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 
 * @author Wes Young
 * @version 
 ***/

#include <iostream>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Exceptions/Error.h>
#include <synthesis/MeasurementEquations/ImagerMultiMS.h>
#include <synthesis/Utilities/FixVis.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Utilities/Assert.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MeasureHolder.h>
#include <measures/Measures/MeasTable.h>
#include <casa/Quanta/QuantumHolder.h>
#include <ms/MeasurementSets.h>
#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <casa/Logging/LogIO.h>
#include <imager_cmpt.h>
#include <image_cmpt.h>
#include <casa/iostream.h>
//#include <casa/System/PGPlotterNull.h>
//#include <graphics/Graphics/PGPlotterLocal.h>

using namespace std;
using namespace casacore;
using namespace casa;

     
using namespace casacore;
namespace casac {

imager::imager()
{
 itsMS = 0;
 hasValidMS_p=false;
 itsImager = new ImagerMultiMS() ;
 itsLog = new LogIO();
 // OK this is probably not the way to set the plotter but it's OK for now I think.
 //PGPlotterInterface *worker(0);
 //try {
    //worker = new PGPlotterLocal("/NULL");
 //} catch (AipsError x) {
   //worker = new PGPlotterNull("/NULL");
 //}
 //itsPlotter = new PGPlotter(worker);
 //itsImager->setPGPlotter(*itsPlotter);
}

imager::~imager()
{
 if(itsMS)
   delete itsMS;
 itsMS = 0;
 if(itsImager)
   delete itsImager;
 itsImager = 0;
}

bool
imager::advise( const bool takeadvice, const double amplitudeloss, const ::casac::variant& fieldofview,
	       int& pixels, ::casac::record& cell, int& facets, string& phasecenter )
{
   Bool rstat(false);
   if(hasValidMS_p){
      try {
	      casacore::MDirection mphaseCenter;
      // mdFromString(mphaseCenter, phasecenter);
         casacore::Quantity qcell;
         casacore::Quantity qfieldofview(casaQuantity(fieldofview));
   
   // pixels and facets are expected to be returned !
         rstat = itsImager->advise(takeadvice, amplitudeloss, qfieldofview,
                                   qcell, pixels, facets, mphaseCenter);
         cell = *recordFromQuantity(qcell);
      //std::cerr << qcell << std::endl;
      //std::cerr << pixels << std::endl;
      //std::cerr << facets << std::endl;
      //std::cerr << mphaseCenter << std::endl;
         if(!MDirection2str(mphaseCenter, phasecenter)){ // <- try preferred format
           ostringstream oss;
           mphaseCenter.print(oss);     // Much less friendly format.
           phasecenter = oss.str();
         }
       }
      catch(AipsError x){
        //*itsLog << LogIO::SEVERE << "Exception Reported: "
         //       << x.getMesg() << LogIO::POST;
        RETHROW(x);
      }
   }
   else{
      *itsLog << LogIO::SEVERE
              << "No MeasurementSet has been assigned, please run open."
              << LogIO::POST;
   }
   return rstat;
}


::casac::record* imager::advisechansel(const double freqstart, const double freqend, const double freqstep, const std::string& freqframe, const std::string& msname, 
				       const int fieldid, const bool getfreqrange, const std::string& spwselection){
  casac::record* retval=0;
  if(hasValidMS_p || msname.length() > 0){
      try {
	casacore::Vector<Vector<Int> > spw;
	casacore::Vector<Vector<Int> > start;
	casacore::Vector<Vector<Int> > nchan;
	MFrequency::Types tp;
	if(!MFrequency::getType(tp, freqframe))
	  throw(AipsError("Invalid frequency frame"));
	Double fstart=freqstart;
	Double fend=freqend;
	if(itsImager->adviseChanSelex(fstart, fend, freqstep, tp, spw, start, nchan, msname, fieldid, getfreqrange, spwselection)){
	  Record outRec;
	  if(!getfreqrange){
	    for (uInt k =0; k < spw.nelements(); ++k){
	      Record subRec;
	      subRec.define("spw", spw[k]);
	      subRec.define("start", start[k]);
	      subRec.define("nchan", nchan[k]);
	      outRec.defineRecord(String("ms_")+String::toString(k), subRec);
	    }
	  }
	  else{
	    outRec.define("freqstart", fstart);
	    outRec.define("freqend", fend);

	  }
	  retval=fromRecord(outRec);
	}
       } catch  (AipsError x) {
          //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
       }
  } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open/selectvis or use msname" << LogIO::POST;
  }
    return retval;

  }

bool
imager::approximatepsf(const std::string& psf, const bool /*async*/)
{

   Bool rstat(false);
   if(hasValidMS_p){
      try {
         String apsf(psf);
         rstat = itsImager->approximatepsf(apsf);
       } catch  (AipsError x) {
          //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
       }
    } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
    }
    return rstat;
}

bool
imager::boxmask(const std::string& mask, const std::vector<int>& blc, const std::vector<int>& trc, const double value)
{

   Bool rstat(false);
   if(hasValidMS_p){
      try {
      //Vector <String> amodel(toVectorString(model));
      //Vector <String> apsf(toVectorString(psf));
          rstat = itsImager->boxmask(mask, blc, trc, value);
       } catch  (AipsError x) {
          //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
       }
    } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
    }
    return rstat;
}

bool imager::calcuvw(const std::vector<int>& fields, const std::string& refcode, 
                     const bool reuse)
{
  Bool rstat(false);
  try {
    *itsLog << LogOrigin("im", "calcuvw");
    if(!itsMS->isWritable()){
      *itsLog << LogIO::SEVERE
	      << "Please open im with a writable ms."
              << " (i.e. use im.open(vis, usescratch=true) whether or not"
              << "       you care about scratch columns)"
	      << LogIO::POST;
      return false;
    }

    FixVis visfixer(*itsMS);
    visfixer.setFields(fields);
    // String obs(observation);
    // if(obs != "")
    //   visfixer.setObsIDs(obs);
    rstat = visfixer.calc_uvw(String(refcode), reuse);

    // Update HISTORY table of modfied MS 
    ostringstream param;
    param << "fields = " << Vector<Int>(fields) << ", refcode = " << refcode
          << ", reuse = " << reuse;// << ", observation = " << obs; 
    String paramstr=param.str();
    if(!(Table::isReadable(itsMS->historyTableName()))){
      TableRecord &kws = itsMS->rwKeywordSet();
      SetupNewTable historySetup(itsMS->historyTableName(),
				 MSHistory::requiredTableDesc(),Table::New);
      kws.defineTable(MS::keywordName(MS::HISTORY), Table(historySetup));
    }
    MSHistoryHandler::addMessage(*itsMS, "UVW and visibilities changed with calcuvw", "im", paramstr, "im::calcuvw()");

  }
  catch (AipsError x) {
    //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
     //       << LogIO::POST;
    Table::relinquishAutoLocks();
    RETHROW(x);
  }
  Table::relinquishAutoLocks();
  return rstat;
}

  casac::record* imager::clean(const std::string& algorithm, const int niter, const double gain,
                   const ::casac::variant& threshold, const bool displayprogress,
                   const std::vector<std::string>& model,
                   const std::vector<bool>& keepfixed, const std::string& complist,
                   const std::vector<std::string>& mask,
                   const std::vector<std::string>& image,
                   const std::vector<std::string>& residual,
                   const std::vector<std::string>& psfnames,
                   const bool interactive, const int npercycle,
                   const std::string& masktemplate, const bool /*async*/)
{
  casac::record* rstat(0);
   if(hasValidMS_p)
     {
       try 
	 {
	   Vector<String> amodel(toVectorString(model));
	   Vector<Bool>   fixed(keepfixed);
	   Vector<String> amask(toVectorString(mask));
	   Vector<String> aimage(toVectorString(image));
	   Vector<String> aresidual(toVectorString(residual));
	   Vector<String> apsf(toVectorString(psfnames));
	   Record retval;
	   retval = itsImager->iClean(String(algorithm), niter, gain,
				     casaQuantity(threshold), Bool(displayprogress),
				     amodel, fixed, String(complist),
				     amask, aimage, aresidual, apsf,
				     Bool(interactive), npercycle,
				     String(masktemplate));
	   rstat = fromRecord(retval);
	 } 
       catch  (AipsError x) 
	 {
	   //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	   RETHROW(x);
	 }
     } 
   else 
     {
       *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open."
	       << LogIO::POST;
     }
   return rstat;
}

bool
imager::clipimage(const std::string& image, const ::casac::variant& threshold)
{

   Bool rstat(false);
   if(hasValidMS_p){
      try {
         rstat = itsImager->clipimage(image, casaQuantity(threshold));
       } catch  (AipsError x) {
          //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
       }
    } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
    }
    return rstat;
}

bool
imager::clipvis(const ::casac::variant& threshold)
{
   Bool rstat(false);
   if(hasValidMS_p){
      try {
         rstat = itsImager->clipvis(casaQuantity(threshold));
       } catch  (AipsError x) {
          //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
       }
    } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
    }
    return rstat;
}

bool
imager::close()
{
 bool rstat(false);

 *itsLog << LogOrigin("im", "close");
 try {
   if(itsMS && !(itsMS->isNull())){
     *itsLog << LogIO::NORMAL3;
     if(itsMS->isWritable())
       *itsLog << "Flushing data to disk and detaching from file.";
     else
       *itsLog << "Readonly measurement set: just detaching from file.";
     *itsLog << LogIO::POST;
   }

   delete itsMS;
   itsMS = 0;
   delete itsImager;
   hasValidMS_p=false;
   itsImager = new ImagerMultiMS();
   //itsImager->setPGPlotter(*itsPlotter);
    
   rstat = true;
 } catch  (AipsError x) {
    //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 Table::relinquishAutoLocks();
 return rstat;
}


bool
imager::done()
{
   return close();
}

::casac::record*  imager::drawmask(const std::string& image, const std::string& mask, const int niter,  const int npercycle, const std::string& threshold){

  casac::record* rstat=0;
  try{
    String elmask(mask);
    if(elmask==String("")){
      elmask=String(image)+String(".mask");
    }
    Int elniter=niter;
    Int elnpercycle=npercycle;
    String elthreshold=threshold;
    Int stat=itsImager->interactivemask(image, elmask, elniter, elnpercycle, elthreshold);
    Record elrec;
    elrec.define("stat", stat);
    elrec.define("niter", elniter);
    elrec.define("npercycle", elnpercycle);
    elrec.define("threshold", elthreshold);
    rstat=fromRecord(elrec);

  } catch  (AipsError x) {
    //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }

  return rstat;
}


bool
imager::exprmask(const std::string& /*mask*/, const double /*expr*/)
{

    // TODO : IMPLEMENT ME HERE !
   *itsLog << LogIO::WARN << "Sorry not implemented yet" << LogIO::POST;
   return false;
}

bool
imager::feather(const std::string& image, const std::string& highres, const std::string& lowres, const std::string& lowpsf, const double dishdiam, const bool showplot, const bool /*async*/)
{

  Bool rstat(false);
   try {
     rstat = itsImager->feather(String(image), String(highres), String(lowres), String(lowpsf), Float(dishdiam), showplot);
   } catch  (AipsError x) {
     //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
     RETHROW(x);
   }
   
   return rstat;
}

bool
imager::filter(const std::string& type, const ::casac::variant& bmaj, const ::casac::variant& bmin, const ::casac::variant& bpa, const bool /*async*/)
{
   Bool rstat(false);
   if(hasValidMS_p){
   try {
     casacore::Quantity posang(0.0, "deg");
     if((String(bpa.toString()) != casacore::String(""))
	&& (String(bpa.toString()) != casacore::String("[]")))
       posang=casaQuantity(bpa);
     rstat = itsImager->filter(type, casaQuantity(bmaj), 
			       casaQuantity(bmin), posang);
    } catch  (AipsError x) {
       //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       RETHROW(x);
    }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
    return rstat;
}

bool
imager::fitpsf( const std::string& psf, const bool /*async*/,
	       ::casac::record& bmaj, ::casac::record& bmin, ::casac::record& bpa)
{
  /*
    bmaj, bmin, and bpa are returned, so they must not be const!
  */
   Bool rstat(false);
   try {
	   ImageBeamSet elbeam;
	   rstat = itsImager->fitpsf(psf, elbeam);
	   IPosition ipos=elbeam.shape();
	   ipos=0;
	   GaussianBeam beam=elbeam(0,0);
	   bpa   = *recordFromQuantity(beam.getPA());
	   bmaj  = *recordFromQuantity(beam.getMajor());
	   bmin  = *recordFromQuantity(beam.getMinor());
   }
   catch(AipsError x){
     //*itsLog << LogIO::SEVERE << "Exception Reported: "
      //       << x.getMesg() << LogIO::POST;
     RETHROW(x);
   }
   return rstat;
}

bool imager::fixvis(const std::vector<int>& fields,
                    const std::vector<std::string>& phaseDirs,
                    const std::string& refcode,
                    const std::vector<double>& distances,
                    const std::string& dataCol)
{
  Bool rstat(false);
  try {
    *itsLog << LogOrigin("im", "fixvis");
    if(!itsMS->isWritable()){
      *itsLog << LogIO::SEVERE
	      << "Please open im with a writable ms."
              << " (i.e. use im.open(vis, usescratch=true) whether or not"
              << "       you care about scratch columns)"
	      << LogIO::POST;
      return false;
    }
    casa::FixVis visfixer(*itsMS, dataCol);
    casacore::Vector<casacore::Int> cFields(fields);
    //int nFields = cFields.nelements();
	
    visfixer.setFields(cFields);

    int nPhDirs = phaseDirs.size(); 
    casacore::Vector<casacore::MDirection> phaseCenters(nPhDirs);
    for(int i = 0; i < nPhDirs; ++i){
      if(!casaMDirection(phaseDirs[i], phaseCenters[i]))
        throw(AipsError("Could not interpret phaseDirs parameter"));
    }
    visfixer.setPhaseDirs(phaseCenters);
    visfixer.setDistances(Vector<Double>(distances));

    rstat = visfixer.fixvis(refcode);

    // Update HISTORY table of modfied MS 
    ostringstream param;
    Vector<String> phasedirsStr(nPhDirs);
    for(int i = 0;  i < nPhDirs; ++i){
      phasedirsStr[i] = phaseDirs[i];
    }
    param << "fields =" << Vector<Int>(fields) << ", phasedirs = " << phasedirsStr
	  << ", refcode = " << refcode << ", distances =" << Vector<Double>(distances) 
	  << ", datacol = " << dataCol;
    String paramstr=param.str(); 
    if(!(Table::isReadable(itsMS->historyTableName()))){
      TableRecord &kws = itsMS->rwKeywordSet();
      SetupNewTable historySetup(itsMS->historyTableName(),
				 MSHistory::requiredTableDesc(),Table::New);
      kws.defineTable(MS::keywordName(MS::HISTORY), Table(historySetup));
    }
    MSHistoryHandler::addMessage(*itsMS, "UVW and visibilities changed with fixvis",
                                 "im", paramstr, "im::fixvis()");

    rstat = true;
  }
  catch (AipsError x) {
    //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
     //       << LogIO::POST;
    Table::relinquishAutoLocks();
    RETHROW(x);
  }
  Table::relinquishAutoLocks();
  return rstat;
}

bool
imager::ft(const std::vector<std::string>& model, const std::string& complist, const bool incremental, const bool /*async*/)
{
   Bool rstat(false);
   if(hasValidMS_p){
   try {
      Vector <String> amodel(toVectorString(model));
      rstat = itsImager->ft(amodel, complist, incremental);
    } catch  (AipsError x) {
       //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       RETHROW(x);
    }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
    return rstat;
}

bool
imager::linearmosaic(const std::vector<std::string>& images, const std::string& mosaic, const std::string& fluxscale, const std::string& sensitivity, const std::vector<int>& fieldids, const bool usedefaultvp, const std::string& vptable, const bool /*async*/)
{

   Bool rstat(false);
   if(hasValidMS_p){
   try {
      Vector <String> aimages(toVectorString(images));
      String vp(vptable);
      if(!vp.empty())
	itsImager->setvp(true, false, vp, false, casacore::Quantity(360.0, "deg"), casacore::Quantity(180.0, "deg"));
      else if(usedefaultvp)
	itsImager->setvp(true, true, "", false, casacore::Quantity(360.0, "deg"), casacore::Quantity(180.0, "deg"));
      rstat = itsImager->linearmosaic(mosaic, fluxscale, sensitivity, aimages, fieldids);
    } catch  (AipsError x) {
       //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       RETHROW(x);
    }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
    return rstat;
}

bool imager::make(const std::string& image, const bool /*async*/)
{

   Bool rstat(false);
   if(hasValidMS_p){
   try {
      rstat = itsImager->make(image);
    } catch  (AipsError x) {
       //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       RETHROW(x);
    }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
    return rstat;
}

std::string imager::predictcomp(const std::string& objname,
                                const std::string& standard,
                                const ::casac::variant& time,
                                const std::vector<double>& freqs,
                                const std::string& pfx)
{
  std::string rstat("");
  try{
    *itsLog << LogOrigin("im", "predictcomp");
    *itsLog << LogIO::DEBUG1 << "starting imager::predictcomp" << LogIO::POST;
    MEpoch epoch;
    if(!casaMEpoch(time, epoch)){
      *itsLog << LogIO::SEVERE
	      << "Could not convert time to an epoch measure."
	      << LogIO::POST;
      return std::string( );
    }
    *itsLog << LogIO::DEBUG1 << "epoch made" << LogIO::POST;

    uInt nfreqs = freqs.size();
    Vector<MFrequency> freqv(nfreqs);

    for(uInt f = 0; f < nfreqs; ++f)
      freqv[f].set(MVFrequency(freqs[f]));
    *itsLog << LogIO::DEBUG1 << "freqs set" << LogIO::POST;
    rstat = itsImager->make_comp(String(objname), String(standard), epoch,
				 freqv, String(pfx));
  }
  catch(AipsError x){
    //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
     //       << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool imager::makeimage(const std::string& type, const std::string& image,
                       const std::string& compleximage, const bool verbose,
                       const bool /*async*/)
{
   Bool rstat(false);
   if(hasValidMS_p){
      try {
        rstat = itsImager->makeimage(String(type), String(image), String(compleximage),
                                     verbose);
      } catch  (AipsError x) {
         //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	 RETHROW(x);
      }
   } else {
      *itsLog << LogIO::SEVERE
              << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

bool imager::makemodelfromsd(const std::string& sdimage, const std::string& modelimage,
                             const std::string& sdpsf, const std::string& maskimage)
{
  try {
    String mask(maskimage);
    if(mask==String(""))
      mask=String(modelimage)+String(".sdmask");
    return itsImager->makemodelfromsd(String(sdimage), String(modelimage), 
                                      String(sdpsf), mask);
      
  } catch  (AipsError x) {
    //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return false;
}

bool imager::mask(const std::string& image, const std::string& mask,
                  const ::casac::variant& threshold, const bool /*async*/)
{
   Bool rstat(false);
   try {
     rstat = itsImager->mask(mask, image, casaQuantity(threshold));
   } catch  (AipsError x) {
     //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
     RETHROW(x);
   }
   return rstat;
}

bool
imager::mem(const std::string& algorithm, const int niter, const ::casac::variant& sigma, const ::casac::variant& targetflux, const bool constrainflux, const bool displayprogress, const std::vector<std::string>& model, const std::vector<bool>& keepfixed, const std::string& complist, const std::vector<std::string>& prior, const std::vector<std::string>& mask, const std::vector<std::string>& image, const std::vector<std::string>& residual, const bool /*async*/)
{

   Bool rstat(false);
   if(hasValidMS_p){
      try {
         Vector<String> amodel(toVectorString(model));
         Vector<Bool>   fixed(keepfixed);
         Vector<String> aprior(toVectorString(prior));
         Vector<String> amask(toVectorString(mask));
         Vector<String> aimage(toVectorString(image));
         Vector<String> aresidual(toVectorString(residual));
         rstat = itsImager->mem(String(algorithm), niter, casaQuantity(sigma), casaQuantity(targetflux),
			        constrainflux, displayprogress, amodel, fixed,
                                String(complist), aprior, amask,
                                aimage, aresidual);
      } catch  (AipsError x) {
          //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
      }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

bool
imager::nnls(const std::vector<std::string>& model, const std::vector<bool>& keepfixed, const std::string& complist, const int niter, const double tolerance, const std::vector<std::string>& fluxmask, const std::vector<std::string>& datamask, const std::vector<std::string>& image, const std::vector<std::string>& residual, const bool /*async*/)
{

   Bool rstat(false);
   if(hasValidMS_p){
      try {
         Vector<String> amodel(toVectorString(model));
         Vector<Bool>   fixed(keepfixed);
         Vector<String> adatamask(toVectorString(datamask));
         Vector<String> afluxmask(toVectorString(fluxmask));
         Vector<String> aimage(toVectorString(image));
         Vector<String> aresidual(toVectorString(residual));
         rstat = itsImager->nnls(String(), niter, tolerance, amodel, fixed, complist,
                                 afluxmask, adatamask, aresidual, aimage);
      } catch  (AipsError x) {
          //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
      }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

bool
imager::open(const std::string& thems, const bool compress, const bool useScratch)
{
    bool rstat(false);
    try {
      if(itsMS){
          delete itsMS;
	  itsMS=0;
	  hasValidMS_p=false;
      }
      if(itsImager){
	delete itsImager;
	itsImager=new Imager();
      }
      if(Table::isWritable(thems)){
	itsMS = new MeasurementSet(String(thems), TableLock(TableLock::AutoNoReadLocking), Table::Update);
	itsMS->setMemoryResidentSubtables(MrsEligibility::defaultEligible());

      }
      else{
	itsMS = new MeasurementSet(String(thems), TableLock(TableLock::AutoNoReadLocking), Table::Old);
	itsMS->setMemoryResidentSubtables(MrsEligibility::defaultEligible());
      }
      // itsImager = new Imager(*itsMS, compress);
      AlwaysAssert(itsMS, AipsError);
      rstat = itsImager->open(*itsMS, compress, useScratch);
      if(rstat) hasValidMS_p=true;
    } catch  (AipsError x) {

      //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
    }
    return rstat;
}

bool
imager::pb(const std::string& inimage, const std::string& outimage,
           const std::string& incomps, const std::string& outcomps,
           const std::string& operation, const ::casac::variant& pointingcenter,
           const ::casac::variant& parangle, const std::string& pborvp,
           const bool /*async*/)
{
  Bool rstat(false);
  if(hasValidMS_p){
    try{
      casacore::MDirection mpointingcenter(casacore::Quantity(0, "rad"), casacore::Quantity(0, "rad"), casacore::MDirection::J2000) ;
      if((String(pointingcenter.toString()) != casacore::String(""))
	 && (String(pointingcenter.toString()) != casacore::String("[]")))
	casaMDirection(pointingcenter, mpointingcenter);
      rstat = itsImager->pb(inimage, outimage, incomps, outcomps, operation,
                            mpointingcenter, casaQuantity(parangle), pborvp);
    }
    catch(AipsError x){
      //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
       //       << LogIO::POST;
      RETHROW(x);
    }
  }
  else{
    *itsLog << LogIO::SEVERE
            << "No MeasurementSet has been assigned, please run open."
            << LogIO::POST;
  }
  return rstat;
}

bool
imager::plotsummary()
{
   Bool rstat(false);
   if(hasValidMS_p){
      try {
        rstat = itsImager->plotsummary();
      } catch  (AipsError x) {
         //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	 RETHROW(x);
      }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

bool
imager::plotuv(const bool rotate)
{

   Bool rstat(false);
   if(hasValidMS_p){
      try {
        rstat = itsImager->plotuv(rotate);
      } catch  (AipsError x) {
         //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	 RETHROW(x);
      }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

bool
imager::plotvis(const std::string& type, const int increment)
{

   Bool rstat(false);
   if(hasValidMS_p){
      try {
        rstat = itsImager->plotvis(type, increment);
      } catch  (AipsError x) {
         //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	 RETHROW(x);
      }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

 bool
 imager::plotweights(const bool gridded, const int increment)
 {
    Bool rstat(false);
   if(hasValidMS_p){
       try {
         // Has a tendency to dump core.  Add to plotms, replace with
         // something else, or fix.
         rstat = itsImager->plotweights(gridded, increment);
      } catch  (AipsError x) {
         // *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
 	 RETHROW(x);
       }
    } else {
       *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
    }
    return rstat;
 }

 ::casac::variant*
 imager::getweightgrid(const std::string& type, const std::vector<std::string>& wgtimages)
 {
   ::casac::variant *rstat = 0;
   if(hasValidMS_p){
       try {
         
	 Cube<Float> weight;
	 Block<Matrix<Float> > blockOGrid;
	 if(String(type)==String("imaging")){
	   if(itsImager->getWeightGrid(blockOGrid, String(type))){
	     weight.resize(blockOGrid[0].shape()(0), blockOGrid[0].shape()(0), blockOGrid.nelements());
	     for (uInt k=0; k < blockOGrid.nelements(); ++k){
	       weight.xyPlane(k)= blockOGrid[k];
	     }
	     std::vector<int> s_shape;
	     weight.shape().asVector().tovector(s_shape);
	     std::vector<double> d_weight(weight.nelements());
	     {
	       Cube<Double> temp(weight.shape());
	       casacore::convertArray(temp, weight);
	       temp.tovector(d_weight);
	     }
	     rstat = new ::casac::variant(d_weight, s_shape);
	   }
	 }
	 else if(String(type)==String("ftweight")){
	   Vector <String> wgtim(toVectorString(wgtimages));
	   itsImager->getWeightGrid(blockOGrid, String(type), wgtim);
	   std::vector<double> d_weight(0);
	   std::vector<int> s_shape(0);
	   rstat = new ::casac::variant(d_weight, s_shape);
	 }

      } catch  (AipsError x) {
         // *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
 	 RETHROW(x);
       }
    } else {
       *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
    }
    return rstat;
 }

bool
imager::setweightgrid(const ::casac::variant& d_weight, const std::string& type){
Bool rstat(false);
   if(hasValidMS_p){
      try {
	Vector<Float> weight;
	Vector<Int> shape;
	if (d_weight.type() == ::casac::variant::DOUBLEVEC) {
	  shape = d_weight.arrayshape();
	  weight.resize(casacore::product(shape));
	  casacore::convertArray(weight, Vector<Double>(d_weight.getDoubleVec()));
	} else if (d_weight.type() == ::casac::variant::INTVEC) {
	  shape = d_weight.arrayshape();
	  weight.resize(casacore::product(shape));
	  casacore::convertArray(weight, Vector<Int>(d_weight.getIntVec()));
	  
	} else {
	  *itsLog << LogIO::SEVERE
		<< "pixels is not understood, try using an array "
		<< LogIO::POST;
	  return rstat;
	}
	Block<Matrix<Float> > blocko(shape(2));
	Cube<Float> elcube(weight.reform(IPosition(shape)));
	for (Int k=0; k< shape(2); ++k){
	  blocko[k]=elcube.xyPlane(k);
	}
	rstat = itsImager->setWeightGrid(blocko, type);	

      } catch  (AipsError x) {
	//*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
      }
   } else {
     *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open or selectvis" << LogIO::POST;
   }


   return rstat;

}



bool
imager::regionmask(const std::string& mask, const ::casac::record& region, 
		   const ::casac::variant& boxes,const ::casac::variant& circles,  const double value)
{
  Bool rstat(false);
   if(hasValidMS_p){
      try {
	
	casacore::Record *pRegion = toRecord(region);
	if(pRegion->nfields() < 2){
	  delete pRegion;
	  pRegion=0;
	}
	casacore::Matrix<casacore::Quantity> quantBoxes;
	Matrix<Float> theBoxes;
	if((boxes.type() == ::casac::variant::DOUBLEVEC) ||
	   (boxes.type() == ::casac::variant::INTVEC) ){
	  Vector<Double> boxVector;
	  if(boxes.type() == ::casac::variant::DOUBLEVEC)
	    boxVector=Vector<Double>(boxes.toDoubleVec());
	  else{
	    std::vector<int> intvector=boxes.toIntVec();
	    boxVector.resize(intvector.size());
	    convertArray(boxVector, Vector<Int>(intvector));
	   
	  }
	  Vector<Int> shape;
	  if(boxes.arrayshape().size() !=2){
	    shape=Vector<Int>(2);
	    shape(0)=boxes.arrayshape()[0]/4;
	    shape(1)=4;
	  }
	  else{
	    shape=Vector<Int>(boxes.arrayshape());
	  }
	  theBoxes.resize(IPosition(shape));
	  Vector<Double> localbox(boxVector);
	  casacore::convertArray(theBoxes,localbox.reform(IPosition(shape)));
	  quantBoxes.resize(theBoxes.shape());
	  for (Int k=0; k < shape[1]; ++k){
	    for(Int j=0; j < shape[0]; ++j){
	        quantBoxes(j,k)=casacore::Quantity(theBoxes(j,k), "pix");
	    } 	    
	  }
	}
	else if(boxes.type() == ::casac::variant::STRINGVEC){
	  Vector<Int> shape = boxes.arrayshape();
	  casacore::Vector<casacore::Quantity> theCorners;
	  toCasaVectorQuantity(boxes, theCorners);
	  if(casacore::product(shape) != Int(theCorners.nelements())){
	    throw(AipsError("Error in converting list of strings to Quantities"));
	  }
	  if(shape.nelements()==1){
	    // list is a single file so assuming every 4 elements is blc trc
	    Int nrows=shape[0]/4;
	    if(shape[0] != nrows*4)
	      throw(AipsError("list of blc trc has to have 4 elements each"));
	    quantBoxes.resize(nrows,shape(0));
	  }
	  else if(shape.nelements()==2){
	    quantBoxes.resize(shape);
	  }
	  else{
             throw(AipsError("Only dealing with 2D blc and trc's for now"));
	  }
	  Vector<casacore::Quantity> refQuantBoxes(quantBoxes.reform(IPosition(1, theCorners.nelements())));
	  refQuantBoxes=theCorners;
	  
	}
	
	Matrix<Float> theCircles;
	
	{
	  Vector<Double> circleVector;
	  if(circles.type() == ::casac::variant::DOUBLEVEC)
	    circleVector=Vector<Double>(circles.toDoubleVec());
	  else if(circles.type() == ::casac::variant::INTVEC){
	    std::vector<int> intvector=circles.toIntVec();
	    circleVector.resize(intvector.size());
	    convertArray(circleVector, Vector<Int>(intvector));
	    
	  }
	  Vector<Int> shape;
	  if(circles.arrayshape().size() !=2){
	    shape=Vector<Int>(2);
	    shape(0)=circles.arrayshape()[0]/3;
	    shape(1)=3;
	  }
	  else{
	    shape=Vector<Int>(circles.arrayshape());
	  }

	  theCircles.resize(IPosition(shape));
	  Vector<Double> localcircles(circleVector);
	  casacore::convertArray(theCircles,localcircles.reform(IPosition(shape)));
	}


	


	
	rstat=itsImager->regionmask(String(mask), pRegion, quantBoxes, 
				    theCircles,Float(value));
	if(pRegion != 0)
	  delete pRegion;

      } catch  (AipsError x) {
	//*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
      }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open or selectvis" << LogIO::POST;
   }


   return rstat;
}

bool
imager::regiontoimagemask(const std::string& mask, 
			  const ::casac::record& region, 
			  const ::casac::variant& boxes, 
			  const ::casac::variant& circles, const double value)
{
  Bool rstat(false);
  try {
    
    if(!casacore::Table::isWritable(mask))
      throw(AipsError(String("Image ")+String(mask)+String(" is not write accessible")));
    casacore::Record *pRegion = toRecord(region);
    if(pRegion->nfields() < 2){
      delete pRegion;
      pRegion=0;
    }
    casacore::Matrix<casacore::Quantity> quantBoxes;
    Matrix<Float> theBoxes;
    if((boxes.type() == ::casac::variant::DOUBLEVEC) ||
       (boxes.type() == ::casac::variant::INTVEC) ){
      Vector<Double> boxVector;
      if(boxes.type() == ::casac::variant::DOUBLEVEC)
	boxVector=Vector<Double>(boxes.toDoubleVec());
      else{
	std::vector<int> intvector=boxes.toIntVec();
	boxVector.resize(intvector.size());
	convertArray(boxVector, Vector<Int>(intvector));
	
      }
      Vector<Int> shape;
      if(boxes.arrayshape().size() !=2){
	shape=Vector<Int>(2);
	shape(0)=boxes.arrayshape()[0]/4;
	shape(1)=4;
      }
      else{
	shape=Vector<Int>(boxes.arrayshape());
      }
      theBoxes.resize(IPosition(shape));
      Vector<Double> localbox(boxVector);
      casacore::convertArray(theBoxes,localbox.reform(IPosition(shape)));
      quantBoxes.resize(theBoxes.shape());
      for (Int k=0; k < shape[1]; ++k){
	for(Int j=0; j < shape[0]; ++j){
	  quantBoxes(j,k)=casacore::Quantity(theBoxes(j,k), "pix");
	} 	    
      }
    }
    else if(boxes.type() == ::casac::variant::STRINGVEC){
      Vector<Int> shape = boxes.arrayshape();
      casacore::Vector<casacore::Quantity> theCorners;
      toCasaVectorQuantity(boxes, theCorners);
      if(casacore::product(shape) != Int(theCorners.nelements())){
	throw(AipsError("Error in converting list of strings to Quantities"));
      }
      if(shape.nelements()==1){
	    // list is a single file so assuming every 4 elements is blc trc
	Int nrows=shape[0]/4;
	if(shape[0] != nrows*4)
	  throw(AipsError("list of blc trc has to have 4 elements each"));
	quantBoxes.resize(nrows,shape(0));
      }
      else if(shape.nelements()==2){
	quantBoxes.resize(shape);
      }
      else{
	throw(AipsError("Only dealing with 2D blc and trc's for now"));
      }
      Vector<casacore::Quantity> refQuantBoxes(quantBoxes.reform(IPosition(1, theCorners.nelements())));
      refQuantBoxes=theCorners;
      
      
    }
    
    Matrix<Float> theCircles;
    
    {
      Vector<Double> circleVector;
      if(circles.type() == ::casac::variant::DOUBLEVEC)
	circleVector=Vector<Double>(circles.toDoubleVec());
      else if(circles.type() == ::casac::variant::INTVEC){
	std::vector<int> intvector=circles.toIntVec();
	circleVector.resize(intvector.size());
	convertArray(circleVector, Vector<Int>(intvector));
	
      }
      Vector<Int> shape;
      if(circles.arrayshape().size() !=2){
	shape=Vector<Int>(2);
	shape(0)=circles.arrayshape()[0]/3;
	shape(1)=3;
      }
      else{
	shape=Vector<Int>(circles.arrayshape());
      }

      theCircles.resize(IPosition(shape));
      Vector<Double> localcircles(circleVector);
      casacore::convertArray(theCircles,localcircles.reform(IPosition(shape)));
    }
    rstat=itsImager->regionToImageMask(String(mask), pRegion, quantBoxes, theCircles, Float(value));
    if(pRegion != 0)
      delete pRegion;
    
  } catch  (AipsError x) {
    //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  

  return rstat;
}



bool
imager::residual(const std::vector<std::string>& model, const std::string& complist, const std::vector<std::string>& image, const bool /*async*/)
{
   Bool rstat(false);
   if(hasValidMS_p){
      try {
         Vector <String> amodel(toVectorString(model));
         Vector <String> aimage(toVectorString(image));
         rstat = itsImager->residual(amodel, complist, aimage);
      } catch  (AipsError x) {
         //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	 RETHROW(x);
      }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

bool imager::updateresidual(const std::vector<std::string>& model, const std::string& complist, const std::vector<std::string>& image, const std::vector<std::string>& residual){

  Bool rstat(false);
  if(hasValidMS_p){
    try {
      Vector <String> amodel(toVectorString(model));
      Vector <String> aimage(toVectorString(image));
      Vector <String> aresidual(toVectorString(residual));
      rstat = itsImager->updateresidual(amodel, complist, aimage, aresidual);
    } catch  (AipsError x) {
      //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
    }
  } else {
    *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
  }
  return rstat;


}

bool
imager::restore(const std::vector<std::string>& model, const std::string& complist, const std::vector<std::string>& image, const std::vector<std::string>& residual, const bool /*async*/)
{
   Bool rstat(false);
   if(hasValidMS_p){
      try {
         Vector <String> amodel(toVectorString(model));
         Vector <String> aimage(toVectorString(image));
         Vector <String> aresidual(toVectorString(residual));
         rstat = itsImager->restore(amodel, complist, aimage, aresidual);
      } catch  (AipsError x) {
         //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	 RETHROW(x);
      }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

bool
imager::sensitivity(const bool /*async*/, ::casac::record& pointsource, double& relative,
                    double& sumweights,
		    ::casac::record& senrec)
{
   Bool rstat(false);
   if(hasValidMS_p){
      try{ 

	// Warn of pending deprecation
	*itsLog << LogIO::WARN
		<< "This function, as currently implemented, will be removed in CASA v.4.5."
		<< endl
		<< " Please consider using the 'apparentsens' function, which will"
		<< endl
		<< " generally be more accurate if your weights are properly calibrated"
		<< endl
		<< " (and doesn't require independent knowledge of Tsys/Aeff)."
		<< LogIO::POST;

        casacore::Quantity qpointsource;
	Vector<Vector<Double> > sumwtChan, sumwtsqChan, sumInverseVarianceChan;
	Vector<Vector<Int> >nData;
	Matrix<Int> mssChanSel;
	Double effBW, effTInt;
	Int nBaselines;
        rstat = itsImager->sensitivity(qpointsource, relative, sumweights,
				       effBW, effTInt, nBaselines,
				       mssChanSel,
				       nData,
				       sumwtChan, sumwtsqChan, sumInverseVarianceChan);
	Record retrec;
	Vector<Int> spwIDs(mssChanSel.shape()(0));

	for (uInt i=0; i<spwIDs.nelements(); i++)
	  spwIDs[i]=mssChanSel(i,0);
	retrec.define("spwid", spwIDs);
	retrec.define("effectivebandwidth", effBW);
	retrec.define("effectiveintegration", effTInt);
	retrec.define("nbaselines", nBaselines);
	retrec.define("sumwt", sumweights);
	// for (Int i=0;i<sumwtChan.nelements(); i++)
	//   {
	//     ostringstream str;
	//     str << i;
	//     retrec.define("sumwtchan "+ str,sumwtChan[i]);
	//     retrec.define("sumwtsqchan "+ str,sumwtsqChan[i]);
	//     retrec.define("ndata "+str,nData[i]);
	//   }
	senrec = *fromRecord(retrec);
        pointsource = *recordFromQuantity(qpointsource);
      }
      catch(AipsError x){
         //*itsLog << LogIO::SEVERE << "Exception Reported: "
          //       << x.getMesg() << LogIO::POST;
	 RETHROW(x);
      }
   }
   else{
      *itsLog << LogIO::SEVERE
              << "No MeasurementSet has been assigned, please run open."
              << LogIO::POST;
   }
   return rstat;
}

bool
imager::apparentsens(const bool /*async*/, double& pointsource, double& relative)
{
   Bool rstat(false);
   if(hasValidMS_p){
      try{ 
        rstat = itsImager->apparentSensitivity(pointsource, relative);
      }
      catch(AipsError x){
         //*itsLog << LogIO::SEVERE << "Exception Reported: "
          //       << x.getMesg() << LogIO::POST;
	 RETHROW(x);
      }
   }
   else{
      *itsLog << LogIO::SEVERE
              << "No MeasurementSet has been assigned, please run open."
              << LogIO::POST;
   }
   return rstat;
}

bool
imager::setbeam(const ::casac::variant& bmaj, const ::casac::variant& bmin,
                const ::casac::variant& bpa, const bool /*async*/)
{
   Bool rstat(false);
   if(hasValidMS_p){
	  try{
		  rstat = itsImager->setbeam(GaussianBeam(casaQuantity(bmaj), casaQuantity(bmin),
				  casaQuantity(bpa)));
      }
      catch(AipsError x){
         //*itsLog << LogIO::SEVERE << "Exception Reported: "
          //       << x.getMesg() << LogIO::POST;
	 RETHROW(x);
      }
   }
   else{
      *itsLog << LogIO::SEVERE
              << "No MeasurementSet has been assigned, please run open."
              << LogIO::POST;
   }
   return rstat;
}


bool
imager::selectvis(const std::string& vis, const std::vector<int>& nchan,
		  const std::vector<int>& start, const std::vector<int>& step, 
                  const ::casac::variant& spw, const ::casac::variant& field,
                  const ::casac::variant& baseline,
		  const ::casac::variant& time, const ::casac::variant& scan,
                  const std::string& intent, const ::casac::variant& observation,
                  const ::casac::variant& uvrange, const std::string& taql,
                  const bool useScratch, const bool datainmemory, const bool writeaccess)
{
    Bool rstat(false);
    //Damn the xml does not allow "readonly" to be a parameter
    //so used writeaccess
    if(itsMS || (vis != "")){
       try {
	 String mode="channel";
	 if(nchan.size()==1 && nchan[0] <0)
	   mode="none";
	 casacore::String fieldnames="";
	 casacore::Vector<Int> fieldIndex;
	 fieldnames=toCasaString(field);
	 //if(fieldnames.contains(String("-"), -1)){
	 if(fieldnames==String("-1")){
	     fieldnames="";
	     fieldIndex=Vector<Int>();
	 }
	 casacore::String antennanames="";
	 Vector<Int> antIndex;
	 antennanames=toCasaString(baseline);
	 //if(antennanames.contains(String("-"), -1)){
	 if(antennanames==String("-1")){
	   antennanames="";
	   antIndex=Vector<Int>();
	 }
	 casacore::String spwstring="";
	 casacore::Vector<Int> spwid;
	 spwstring=toCasaString(spw);
	 //if(spwstring.contains(String("-"), -1)){
	 if(spwstring==String("-1")){
	   spwstring="";
	   spwid=Vector<Int>();
	 }
	 casacore::String timerange="";
	 timerange=toCasaString(time);
	 casacore::String uvdist="";
	 uvdist=toCasaString(uvrange);
	 casacore::String scanrange="";
	 scanrange=toCasaString(scan);
	 casacore::String obsrange="";
	 obsrange=toCasaString(observation);
	 //Only load to memory if open is not used and loadinmemory
	 if((String(vis) != String("")) && datainmemory)
	   rstat=(static_cast<ImagerMultiMS *>(itsImager))->setDataToMemory(
									    vis, mode, Vector<Int>(nchan),
									    Vector<Int>(start),
									    Vector<Int>(step), 
									    Vector<Int>(spwid),
									    fieldIndex, 
									    String(taql), String(timerange),
									    fieldnames, antIndex, antennanames, 
									    spwstring, uvdist,
                                                                            scanrange, String(intent), obsrange);
	 else
	   rstat = itsImager->setDataPerMS(vis, mode, Vector<Int>(nchan), 
					   Vector<Int>(start),
					   Vector<Int>(step), Vector<Int>(spwid),
					   fieldIndex, 
					   String(taql), String(timerange),
					   fieldnames, antIndex, antennanames, 
					   spwstring, uvdist, scanrange, String(intent),
                                           obsrange, useScratch, !writeaccess);
	 hasValidMS_p=rstat;
       } catch  (AipsError x) {
          //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
       }
    } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open or assign an ms here via the vis parameter" << LogIO::POST;
    }
    return rstat;
}

bool
imager::defineimage(const int nx, const int ny, const ::casac::variant& cellx, 
		    const ::casac::variant& celly, const std::string& stokes, 
		    const ::casac::variant& phasecenter, 
		    const std::string& mode, const int nchan, 
		    const ::casac::variant& start, 
		    const ::casac::variant& step, 
		    const std::vector<int>& spwid, 
		    const ::casac::variant& restfreq, 
	            const std::string& outframe,
	            const std::string& veltype,
	            const int facets, 
		    const ::casac::variant& movingsource,
		    const ::casac::variant& distance)
{
  Bool rstat(false);
  if(hasValidMS_p){
    try {
      
      Int nX, nY;
      nX=nx;
      nY=ny;
      if(nY < 1)
	nY=nx;
      casacore::Quantity cellX=casaQuantity(cellx);

      if(cellX.getValue()==0.0)
	cellX=casacore::Quantity(1.0, "arcsec");

      casacore::Quantity cellY;
      if(toCasaString(celly) == String("")){
	cellY=cellX;
      }
      else{
	cellY=casaQuantity(celly);
      }
      casacore::MDirection  phaseCenter;
      Int fieldid=-1;
      //If phasecenter is a simple numeric value then its taken as a fieldid 
      //otherwise its converted to a MDirection
      if(phasecenter.type()==::casac::variant::DOUBLEVEC 
	 || phasecenter.type()==::casac::variant::DOUBLE
	 || phasecenter.type()==::casac::variant::INTVEC
	 || phasecenter.type()==::casac::variant::INT){
	fieldid=phasecenter.toInt();
	
      }
      else{
	if(toCasaString(phasecenter)==String("")){
	  fieldid=0;
	}
	else{

	  if(!casaMDirection(phasecenter, phaseCenter)){
	    throw(AipsError("Could not interprete phasecenter parameter"));
	  }
	}
      }
      casacore::String lamoda(mode);
      if(lamoda.contains("velo") || lamoda.contains("VELO")){
	 if (String(veltype).contains("optical") || String(veltype).contains("OPTICAL")) {
	   lamoda=String("OPTICALVELOCITY"); 
	 }
	 else if (String(veltype).contains("radio") || String(veltype).contains("RADIO") || String(veltype)==String("RADIO") || String(veltype)==String("radio") ){
	   lamoda="RADIOVELOCITY";
	 }
	 else if(String(veltype).contains("true") ||  String(veltype).contains("relativistic")){
	   lamoda="TRUEVELOCITY";
	 }
      }
      lamoda.upcase();
      casacore::MRadialVelocity mvel;
      casacore::MFrequency mfreq;
      casacore::Quantity qstart;
      casacore::Quantity qstep;
      casacore::Int startoo=-1;
      casacore::Int stepoo=0;

      //qstart=casac::image::casaQuantityFromVar(start, *itsLog);
      //qstep=casac::image::casaQuantityFromVar(step, *itsLog);
      if(lamoda.contains(String("VEL"))){
	if(!casaMRadialVelocity(start, mvel)){
	  *itsLog << LogIO::SEVERE << "Could not interpret velocity value " 
		  << LogIO::POST;	
	}
	qstep=casaQuantity(step);
      }
      else if(lamoda.contains(String("FREQ"))){
	if(!casaMFrequency(start, mfreq)){
	  *itsLog << LogIO::SEVERE << "Could not interpret frequency value " 
		  << LogIO::POST;	
	}
	
	qstep=casaQuantity(step);
      }
      else{
	startoo=start.toInt();
	stepoo=step.toInt();      
      }
      
      casacore::Quantity restFreq= casaQuantity(restfreq);
      casacore::Quantity cdistance=casaQuantity(distance);

      casacore::String baseframe("LSRK");
      if(itsMS && !(itsMS->isNull())){
	ROMSSpWindowColumns spwc(itsMS->spectralWindow());
	int spw0 = 0;
	if (spwid.size() > 0 && spwid[0] > 0 && spwid[0] < (int) spwc.nrow())
	  spw0 = spwid[0];
	baseframe = MFrequency::showType(spwc.measFreqRef()(spw0));
      }
      casacore::String cframe=toCasaString(outframe);
      if(cframe=="") {
        cframe=baseframe;
      } 
      casacore::MFrequency::Types mfframe;
      if(!casacore::MFrequency::getType(mfframe,cframe)) {
        *itsLog << LogIO::SEVERE << "Could not interprete frequency frame type "
                << LogIO::POST;
      }
      Bool domovingSource=false;
      casacore::MDirection movingDir;
      if(toCasaString(movingsource)!=String("")){
	if(!casaMDirection(movingsource, movingDir)){
	    throw(AipsError("Could not interprete phasecenter parameter"));
	}
	domovingSource=true;
      }

      rstat = itsImager->defineImage(nX, nY, cellX, cellY, stokes, phaseCenter, 
				     fieldid, lamoda, nchan, startoo, 
				     stepoo, mfreq, mvel,  qstep, 
				     Vector<Int>(spwid), facets, restFreq, mfframe,
				     cdistance, domovingSource, movingDir);
    } catch  (AipsError x) {
      //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
    }    
  } else {
    *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open or selectvis" << LogIO::POST;
  }
  return rstat;
}



//bool
casac::record*
imager::setjy(const ::casac::variant& field, const ::casac::variant& spw, 
	      const std::string& modimage,
	      const std::vector<double>& fluxdensity, const std::string& standard,
              //const bool scalebychan, const double spix,
              const bool scalebychan, const std::vector<double>& spix,
              const ::casac::variant& reffreq, 
              const std::vector<double>& polindex, const std::vector<double>& polangle,
              const double rotmeas,
              const std::string& time,
              const std::string& scan, const std::string& intent,
              const std::string& observation,
              const std::string& interpolation)
{
  //Bool rstat = false;
  Record retval;
  casac::record* rstat=0;
  *itsLog << LogOrigin("im", "setjy");

  if(hasValidMS_p){
    try{
      casacore::String fieldnames="";
      casacore::Vector<Int> fieldIndex;
      fieldnames=toCasaString(field);
      // if(fieldnames.contains(String("-"), -1)){
      if(fieldnames==String("-1")){
        fieldnames="";
        fieldIndex=Vector<Int>(1,-1);
      }
      casacore::String spwstring="";
      casacore::Vector<Int> spwid;
      spwstring=toCasaString(spw);
      //if(spwstring.contains(String("-"), -1)){
      if(spwstring==String("-1")){
        spwstring="";
        spwid=Vector<Int>(1,-1);
      }
      casacore::MFrequency mfreqref;
      // Unfortunately the default c'tor for mfreqref sets it to 0.
      // SpectralIndex divides by it, so we really want to get a nonzero value
      // in even if we do not think it will be used.  However, don't complain
      // about a bogus reffreq if it's clearly irrelevant.
      if(!casaMFrequency(reffreq, mfreqref) && fluxdensity[0] >= 0.0){
        *itsLog << LogIO::SEVERE
                << "Could not interpret the reference frequency"
                << LogIO::POST;
        return 0;
      }
      casacore::String timerange(time);
      casacore::String scanstr(scan);
      casacore::String obsstr(observation);
      casacore::String intentstr(intent);
      casacore::String interpstr(interpolation);

      //rstat = itsImager->setjy(fieldIndex, spwid, fieldnames, spwstring, 
      retval = itsImager->setjy(fieldIndex, spwid, fieldnames, spwstring, 
                               modimage, fluxdensity, standard, scalebychan,
                               spix, mfreqref, polindex, polangle, rotmeas, 
                               timerange, scanstr, intentstr, obsstr, interpstr);
    } 
    catch(AipsError x){
      //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
    }
  }
  else{
    *itsLog << LogIO::SEVERE
            << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
  }
  rstat=fromRecord(retval);
  return rstat;
}

// This was a temporary copy of setjy while flux calibration with Solar System
// objects was being tested.
bool
imager::ssoflux()
{
   *itsLog << LogIO::SEVERE
           << "ssoflux is no longer supported.  Use setjy."
           << LogIO::POST;

    return false;
}

bool
imager::setmfcontrol(const double cyclefactor, const double cyclespeedup, const double cyclemaxpsffraction, const int stoplargenegatives, const int stoppointmode, const double minpb, const std::string& scaletype, const double constpb, const std::vector<std::string>& fluxscale, const bool flatnoise)
{
   Bool rstat(false);
   if(hasValidMS_p){
      try {
         Vector <String> afluxscale(toVectorString(fluxscale));
         rstat = itsImager->setmfcontrol(cyclefactor, cyclespeedup, cyclemaxpsffraction, stoplargenegatives,
                                         stoppointmode, scaletype, minpb, constpb, afluxscale, flatnoise);
       } catch  (AipsError x) {
          //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
       }
    } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
    }
    return rstat;
}

bool
imager::setoptions(const std::string& ftmachine, const int cache, const int tile, const std::string& gridfunction, const ::casac::variant& location, const double padding, const std::string& freqinterp, const int wprojplanes, const std::string& epjtablename, const bool applypointingoffsets, const bool dopbgriddingcorrections, const std::string& cfcachedirname, const double rotpastep, const double pastep, const double pblimit, const int imagetilevol, const bool singprec, const int numthreads,const bool psterm, const bool aterm, const bool mterm, const bool wbawp, const bool conjbeams)
{

   Bool rstat(false);
   if(hasValidMS_p){
      try {
	      casacore::MPosition mlocation;
	  if ((String(location.toString()) != casacore::String("")) &&
	      (String(location.toString()) != casacore::String("[]")) ){
	    casaMPosition(location, mlocation);
	  }
          rstat = itsImager->setoptions(String(ftmachine), cache, tile, 
					String(gridfunction),
                                        mlocation, padding,
					wprojplanes, String(epjtablename), 
					applypointingoffsets, 
					dopbgriddingcorrections, 
					String(cfcachedirname), Float(rotpastep), Float(pastep),
					Float(pblimit), String(freqinterp), imagetilevol, singprec, numthreads,
					psterm, aterm, mterm, wbawp,conjbeams);
       } catch  (AipsError x) {
          //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
       }
    } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
    }
    return rstat;
}

bool
imager::setscales(const std::string& scalemethod, const int nscales, const std::vector<double>& uservector)
{


   Bool rstat(false);
   if(hasValidMS_p){


     try {
       Vector<Float> scalvec(uservector.size());
       convertArray(scalvec, Vector<Double> (uservector));
       rstat = itsImager->setscales(scalemethod, nscales, scalvec);
     } catch  (AipsError x) {
       //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       RETHROW(x);
     }
     
   } else {
     *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
   
}

bool 
imager::setsmallscalebias(const float inbias)
{
  Bool rstat(false);
  if(hasValidMS_p){

     try {
       rstat = itsImager->setSmallScaleBias(inbias);
     } catch  (AipsError x) {
       //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       RETHROW(x);
     }
     
   } else {
     *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}
  
bool
imager::settaylorterms(const int ntaylorterms, const double reffreq)
{
   Bool rstat(false);
   if(hasValidMS_p){

     try {
       rstat = itsImager->settaylorterms(ntaylorterms,reffreq);
     } catch  (AipsError x) {
       //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       RETHROW(x);
     }
     
   } else {
     *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

bool
imager::setsdoptions(const double scale, const double weight, const int convsupport, const std::string& pointingcolumntouse,
    const ::casac::variant &truncate, const ::casac::variant &gwidth, const ::casac::variant &jwidth,
    const double minweight, const bool clipminmax)
{

   Bool rstat(false);
   try {
     casacore::String pcolToUse(pointingcolumntouse);
     casacore::Quantity qTruncate = casaQuantity(truncate);
     casacore::Quantity qGWidth = casaQuantity(gwidth);
     casacore::Quantity qJWidth = casaQuantity(jwidth);
     *itsLog << LogIO::DEBUGGING
             << "qTruncate=" << qTruncate.getValue() << qTruncate.getUnit() << endl
             << "qGWidth=" << qGWidth.getValue() << qGWidth.getUnit() << endl
             << "qJWidth=" << qJWidth.getValue() << qJWidth.getUnit() << LogIO::POST;
     rstat = itsImager->setsdoptions(scale, weight, convsupport, pcolToUse,
                                     qTruncate, qGWidth, qJWidth,
                                     Float(minweight), Bool(clipminmax));
   } catch  (AipsError x) {
     //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
     RETHROW(x);
   }

   return rstat;
}

bool imager::setvp(const bool dovp, const bool usedefaultvp,
                   const std::string& vptable, const bool dosquint,
                   const ::casac::variant& parangleinc,
                   const ::casac::variant& skyposthreshold,
                   const std::string& telescope, const bool verbose)
{
   Bool rstat(false);
   try {
     casacore::Quantity skyposthr(180, "deg");
     casacore::Quantity parang(360, "deg");
     if ((String(parangleinc.toString()) != casacore::String("")) &&
	 (String(parangleinc.toString()) != casacore::String("[]")) )
       parang=casaQuantity(parangleinc);
     if ((String(skyposthreshold.toString()) != casacore::String("")) &&
	 (String(skyposthreshold.toString()) != casacore::String("[]")))
       skyposthr=casaQuantity(skyposthreshold);
     rstat = itsImager->setvp(dovp, usedefaultvp, vptable, dosquint, parang, 
			      skyposthr, telescope, verbose);
   } catch  (AipsError x) {
     //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
     RETHROW(x);
   }
   return rstat;
}

bool
imager::smooth(const std::vector<std::string>& model, const std::vector<std::string>& image, const bool usefit, const ::casac::variant& bmaj, const ::casac::variant& bmin, const ::casac::variant& bpa, const bool normalize, const bool /*async*/)
{
   Bool rstat(false);
   if(hasValidMS_p){
      try {
         casacore::Quantity qbmaj(casaQuantity(bmaj));
         casacore::Quantity qbmin(casaQuantity(bmin));
         casacore::Quantity qbpa(casaQuantity(bpa));
         Vector<String> amodel(toVectorString(model));
         Vector<String> aimage(toVectorString(image));
         ImageBeamSet beam(GaussianBeam (qbmaj, qbmin, qbpa));

         rstat = itsImager->smooth(amodel, aimage, usefit,
        		 beam, normalize);
       } catch  (AipsError x) {
          //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
       }
    } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
    }
    return rstat;
}

bool
imager::stop()
{

    // TODO : IMPLEMENT ME HERE !
   *itsLog << LogIO::WARN << "Sorry not implemented yet" << LogIO::POST;
   return false;
}

bool
imager::summary()
{

   Bool rstat(false);
   if(hasValidMS_p){
      try {
         rstat = itsImager->summary();
      } catch  (AipsError x) {
         //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	 RETHROW(x);
      }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

bool
imager::uvrange(const double uvmin, const double uvmax)
{

   Bool rstat(false);
   if(hasValidMS_p){
      try {
         rstat = itsImager->uvrange(uvmin, uvmax);
         if(!rstat)
            *itsLog << LogIO::WARN << "uvrange failed did you setdata first?" << LogIO::POST;
      } catch  (AipsError x) {
         //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	 RETHROW(x);
      }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

bool
imager::weight(const std::string& type, const std::string& rmode,
               const ::casac::variant& noise, const double robust,
               const ::casac::variant& fieldofview, const int npixels,
               const bool mosaic, const bool /*async*/)
{
  Bool rstat(false);
  if(hasValidMS_p){
    try{
      rstat = itsImager->weight(String(type), String(rmode), casaQuantity(noise),
                                robust, casaQuantity(fieldofview), npixels, mosaic);
    }
    catch(AipsError x){
      //*itsLog << LogIO::SEVERE << "Exception Reported: "
       //       << x.getMesg() << LogIO::POST;
      RETHROW(x);
    }
  }
  else{
      *itsLog << LogIO::SEVERE
              << "No MeasurementSet has been assigned, please run open."
              << LogIO::POST;
  }
  return rstat;
}

bool imager::mpFromString(casacore::MPosition &/*thePos*/, const casacore::String &/*in*/)
{
   bool rstat(false);
   return rstat;
}

bool imager::mdFromString(casacore::MDirection &theDir, const casacore::String &in)
{
   bool rstat(false);
   String tmpA, tmpB, tmpC;
   std::istringstream iss(in);
   iss >> tmpA >> tmpB >> tmpC;
   casacore::Quantity tmpQA;
   casacore::Quantity tmpQB;
   casacore::Quantity::read(tmpQA, tmpA);
   casacore::Quantity::read(tmpQB, tmpB);
   if(tmpC.length() > 0){
	   casacore::MDirection::Types theRF;
	   casacore::MDirection::getType(theRF, tmpC);
      theDir = casacore::MDirection (tmpQA, tmpQB, theRF);
      rstat = true;
   } else {
      theDir = casacore::MDirection (tmpQA, tmpQB);
      rstat = true;
   }
   return rstat;
}

bool imager::mrvFromString(casacore::MRadialVelocity &theRadialVelocity,
                           const casacore::String &in)
{
   String tmpA, tmpB, tmpC;
   bool rstat(false);
   std::istringstream iss(in);
   iss >> tmpA >> tmpB >> tmpC;
   casacore::Quantity tmpQ;
   casacore::Quantity::read(tmpQ, tmpA);
   if(tmpQ.getUnit().length() == 0){
      tmpQ.setUnit(Unit(tmpB));
      if(tmpC.length() > 0){
	      casacore::MRadialVelocity::Types theRF;
	      casacore::MRadialVelocity::getType(theRF, String(tmpC));
         theRadialVelocity = casacore::MRadialVelocity(tmpQ, theRF);
         rstat = true;
      } else {
         theRadialVelocity = casacore::MRadialVelocity(tmpQ);
         rstat = true;
      }
   }else{
      if(tmpB.length() > 0){
	      casacore::MRadialVelocity::Types theRF;
	      casacore::MRadialVelocity::getType(theRF, String(tmpB));
         theRadialVelocity = casacore::MRadialVelocity(tmpQ, theRF);
         rstat = true;
      } else {
         theRadialVelocity = casacore::MRadialVelocity(tmpQ);
         rstat = true;
      }
   }
   return rstat; 
}

casac::record *imager::mapextent(const std::string &ref, const std::string &movingsource,
        const std::string &pointingcolumntouse)
{
    Bool rstat = false;
    casac::record *returnValue;
    Record r;
    if(hasValidMS_p){
        Vector<Double> center(2, 0.0);
        Vector<Double> blc(2, 0.0);
        Vector<Double> trc(2, 0.0);
        Vector<Double> extent(2, 0.0);
        String directionRef(ref);
        String movingSource(movingsource);
        String columnName(pointingcolumntouse);
        rstat = itsImager->mapExtent(directionRef, movingSource, columnName,
                center, blc, trc, extent);
        r.define("status", rstat);
        r.define("center", center);
        r.define("blc", blc);
        r.define("trc", trc);
        r.define("extent", extent);
    }
    else{
        r.define("status", rstat);
        *itsLog << LogIO::SEVERE
                << "No MeasurementSet has been assigned, please run open."
                << LogIO::POST;
    }
    returnValue = fromRecord(r);
    return returnValue;
}

casac::record *imager::pointingsampling(const std::string &pattern, const std::string &ref, const std::string &movingsource,
				 const std::string &pointingcolumntouse, const std::string &antenna)
{
    casac::record *returnValue;
    Record r;
    if(hasValidMS_p){
      try {
	if (pattern != "raster") {
	  throw(AipsError("Only raster pattern is supported."));
	}
        String directionRef(ref);
        String movingSource(movingsource);
        String columnName(pointingcolumntouse);
        String antSel(antenna);
	Quantum<Vector<Double>> sampling;
	casacore::Quantity positionAngle(0.0, "rad");
	Bool rstat = itsImager->pointingSampling(directionRef, movingSource, columnName, antSel,
					    sampling, positionAngle);
	if (!rstat) {
	  throw(AipsError("Failed to get sampling interval."));
	}
        QuantumHolder qh(sampling);
        r.defineRecord("sampling", qh.toRecord());
        qh = QuantumHolder(positionAngle);
        r.defineRecord("angle", qh.toRecord());
      }
      catch(AipsError x){
	RETHROW(x);
      }
    }
    else{
        *itsLog << LogIO::SEVERE
                << "No MeasurementSet has been assigned, please run open."
                << LogIO::POST;
    }
    returnValue = fromRecord(r);
    return returnValue;
}

} // casac namespace
