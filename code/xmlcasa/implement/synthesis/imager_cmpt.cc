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
#include <casa/Logging/LogIO.h>
#include <xmlcasa/synthesis/imager_cmpt.h>
#include <xmlcasa/images/image_cmpt.h>
#include <casa/System/PGPlotterNull.h>
#include <graphics/Graphics/PGPlotterLocal.h>

using namespace std;
using namespace casa;

     
namespace casac {

imager::imager()
{
 itsMS = 0;
 hasValidMS_p=false;
 itsImager = new ImagerMultiMS() ;
 itsLog = new LogIO();
 // OK this is probably not the way to set the plotter but it's OK for now I think.
 PGPlotterInterface *worker(0);
 try {
    worker = new PGPlotterLocal("/NULL");
 } catch (AipsError x) {
   worker = new PGPlotterNull("/NULL");
 }
 itsPlotter = new PGPlotter(worker);
 itsImager->setPGPlotter(*itsPlotter);
}

imager::~imager()
{
 if(itsMS)
   delete itsMS;
 itsMS = 0;
 if(itsImager)
   delete itsImager;
 itsImager = 0;
 if(itsPlotter)
   delete itsPlotter;
 itsPlotter = 0;
}

bool
imager::advise(int& pixels, ::casac::record& cell, int& facets,
               std::string& phasecenter, const bool takeadvice,
               const double amplitudeloss, const ::casac::variant& fieldofview)
{
   Bool rstat(False);
   if(hasValidMS_p){
      try {
         MDirection mphaseCenter;
      // mdFromString(mphaseCenter, phasecenter);
         casa::Quantity qcell;
         casa::Quantity qfieldofview(casaQuantity(fieldofview));
   
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
        *itsLog << LogIO::SEVERE << "Exception Reported: "
                << x.getMesg() << LogIO::POST;
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
imager::approximatepsf(const std::string& psf, const bool async)
{

   Bool rstat(False);
   if(hasValidMS_p){
      try {
         String apsf(psf);
         rstat = itsImager->approximatepsf(apsf);
       } catch  (AipsError x) {
          *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
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

   Bool rstat(False);
   if(hasValidMS_p){
      try {
      //Vector <String> amodel(toVectorString(model));
      //Vector <String> apsf(toVectorString(psf));
          rstat = itsImager->boxmask(mask, blc, trc, value);
       } catch  (AipsError x) {
          *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
       }
    } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
    }
    return rstat;
}

bool imager::calcuvw(const std::vector<int>& fields, const std::string& refcode)
{
  Bool rstat(false);
  try {
    *itsLog << LogOrigin("im", "calcuvw");
    if(!itsMS->isWritable()){
      *itsLog << LogIO::SEVERE
	      << "Please open im with a writable ms."
              << " (i.e. use im.open(vis, usescratch=True) whether or not"
              << "       you care about scratch columns)"
	      << LogIO::POST;
      return false;
    }

    *itsLog << LogIO::NORMAL2 << "calcuvw starting" << LogIO::POST;
      
    FixVis visfixer(*itsMS);
    *itsLog << LogIO::NORMAL2 << "FixVis created" << LogIO::POST;
    //visfixer.setField(m1toBlankCStr_(fields));
    visfixer.setFields(fields);
    rstat = visfixer.calc_uvw(String(refcode));
    *itsLog << LogIO::NORMAL2 << "calcuvw finished" << LogIO::POST;
  }
  catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
            << LogIO::POST;
    Table::relinquishAutoLocks();
    RETHROW(x);
  }
  Table::relinquishAutoLocks();
  return rstat;
}

bool
imager::clean(const std::string& algorithm, const int niter, const double gain, const ::casac::variant& threshold, const bool displayprogress, const std::vector<std::string>& model, const std::vector<bool>& keepfixed, const std::string& complist, const std::vector<std::string>& mask, const std::vector<std::string>& image, const std::vector<std::string>& residual, const std::vector<std::string>& psfnames, const bool interactive, const int npercycle, const std::string& masktemplate, const bool async)
{

   Bool rstat(False);
   if(hasValidMS_p){
      try {
         Vector <String> amodel(toVectorString(model));
         Vector<Bool>   fixed(keepfixed);
         Vector <String> amask(toVectorString(mask));
         Vector <String> aimage(toVectorString(image));
         Vector <String> aresidual(toVectorString(residual));
	 Vector <String> apsf(toVectorString(psfnames));
	 if( (apsf.nelements()==1) && apsf[0]==String(""))
	   apsf.resize();
	 if(!interactive){
	   rstat = itsImager->clean(String(algorithm), niter, gain, 
				    casaQuantity(threshold), displayprogress, 
				    amodel, fixed, String(complist), amask,  
				    aimage, aresidual, apsf);
	 }
	 else{
	   /*if(amodel.nelements() > 1){
	     throw(AipsError("Interactive clean in multifield mode is not supported yet"));
	   }
	   */
	   if((amask.nelements()==0) || (amask[0]==String(""))){
	     amask.resize(amodel.nelements());
	     for (uInt k=0; k < amask.nelements(); ++k){
		 amask[k]=amodel[k]+String(".mask");
	     }
	   }
	   Vector<Bool> nointerac(amodel.nelements());
	   nointerac.set(False);
	   if(fixed.nelements() != amodel.nelements()){
	     fixed.resize(amodel.nelements());
	     fixed.set(False);
	   }
	   Int nloop=0;
	   if(npercycle != 0)
	     nloop=niter/npercycle;
	   Int continter=0;
	   Int elniter=npercycle;
	   ostringstream oos;
	   casaQuantity(threshold).print(oos);
	   String thresh=String(oos);
	   if(String(masktemplate) != String("")){
	     continter=itsImager->interactivemask(masktemplate, amask[0], 
						  elniter, nloop, thresh);
	   }
	   else {
	     // do a zero component clean to get started
	     itsImager->clean(String(algorithm), 0, gain, 
			      casaQuantity(threshold), displayprogress,
			      amodel, fixed, String(complist), amask,  
			      aimage, aresidual, apsf);
	     for (uInt nIm=0; nIm < aresidual.nelements(); ++nIm){
	       continter=itsImager->interactivemask(aresidual[nIm], amask[nIm], 
						    elniter, nloop,thresh);
	       if(continter>=1)
		 nointerac(nIm)=True;
	       if(continter==2)
		 fixed(nIm)=True;

	     }
	     if(allEQ(nointerac, True)){
		   elniter=niter;
		   //make it do one more loop/clean but with all niter 
		   nloop=1;
		 }
	   }
	   for (Int k=0; k < nloop; ++k){
	     
	     casa::Quantity thrsh;
	     if(!casa::Quantity::read(thrsh, thresh)){
	       *itsLog << LogIO::WARN << "Error interpreting threshold" 
		       << LogIO::POST;
	       thrsh=casa::Quantity(0, "Jy");
	       thresh="0.0Jy";
	     }
	     if(anyEQ(fixed, False)){
	       // interactive on first model only for now
	       rstat = itsImager->clean(String(algorithm), elniter, gain, 
					thrsh, 
					displayprogress,
					amodel, fixed, String(complist), 
					amask,  
					aimage, aresidual);
	       //if clean converged... equivalent to stop
	       if(rstat){
		 continter=2;
		 fixed.set(True);
	       }
	       if(anyEQ(fixed, False) && anyEQ(nointerac,False)){
		 Int remainloop=nloop-k-1;
		 for (uInt nIm=0; nIm < aresidual.nelements(); ++nIm){
		   if(!nointerac(nIm)){
			continter=itsImager->interactivemask(aresidual[nIm], amask[nIm],
							     
							     elniter, remainloop, 
							     thresh);
			if(continter>=1)
			  nointerac(nIm)=True;
			if(continter==2)
			  fixed(nIm)=True;
		   }
		 }
		 k=nloop-remainloop-1;

		 if(allEQ(nointerac,True)){
		   elniter=niter-(k+1)*npercycle;
		   //make it do one more loop/clean but with remaining niter 
		   k=nloop-2;
		 }
	       } 
	     }
	   }
	 }
      } catch  (AipsError x) {
          *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
       }
    } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
    }
    return rstat;
}

bool
imager::clipimage(const std::string& image, const ::casac::variant& threshold)
{

   Bool rstat(False);
   if(hasValidMS_p){
      try {
         rstat = itsImager->clipimage(image, casaQuantity(threshold));
       } catch  (AipsError x) {
          *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
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
   Bool rstat(False);
   if(hasValidMS_p){
      try {
         rstat = itsImager->clipvis(casaQuantity(threshold));
       } catch  (AipsError x) {
          *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
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
 bool rstat(False);
 try {
   if(itsMS)delete itsMS;
       itsMS = 0;
    delete itsImager;
    hasValidMS_p=false;
    itsImager = new ImagerMultiMS();
    itsImager->setPGPlotter(*itsPlotter);
    
    rstat = True;
 } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
imager::correct(const bool doparallactic, const Quantity& timestep)
{
   Bool rstat(False);
   if(hasValidMS_p){
      try {
         casa::Quantity qtimestep(casaQuantity(timestep));
         rstat = itsImager->correct(doparallactic, qtimestep);
      } catch  (AipsError x) {
         *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	 RETHROW(x);
      }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

bool
imager::done()
{
   return close();
}

bool imager::drawmask(const std::string& image, const std::string& mask ){


  try{
    String elmask(mask);
    if(elmask==String("")){
      elmask=String(image)+String(".mask");
    }
    Int dummy=0;
    Int dummier=0;
    String dummiest="0.0Jy";
    itsImager->interactivemask(image, elmask,dummy, dummier, dummiest);
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }

  return True;
}


bool
imager::exprmask(const std::string& mask, const double expr)
{

    // TODO : IMPLEMENT ME HERE !
   *itsLog << LogIO::WARN << "Sorry not implemented yet" << LogIO::POST;
   return False;
}

bool
imager::feather(const std::string& image, const std::string& highres, const std::string& lowres, const std::string& lowpsf, const bool async)
{

  Bool rstat(False);
   try {
      rstat = itsImager->feather(String(image), String(highres), String(lowres), String(lowpsf));
   } catch  (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
     RETHROW(x);
   }
   
   return rstat;
}

bool
imager::filter(const std::string& type, const ::casac::variant& bmaj, const ::casac::variant& bmin, const ::casac::variant& bpa, const bool async)
{
   Bool rstat(False);
   if(hasValidMS_p){
   try {
     casa::Quantity posang(0.0, "deg");
     if((String(bpa.toString()) != casa::String("")) 
	&& (String(bpa.toString()) != casa::String("[]")))
       posang=casaQuantity(bpa);
     rstat = itsImager->filter(type, casaQuantity(bmaj), 
			       casaQuantity(bmin), posang);
    } catch  (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       RETHROW(x);
    }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
    return rstat;
}

bool
imager::fitpsf(::casac::record& bmaj, ::casac::record& bmin, ::casac::record& bpa,
               const std::string& psf, const bool async)
{
  /*
    bmaj, bmin, and bpa are returned, so they must not be const!
  */
   Bool rstat(False);
   try {
      casa::Quantity cbmaj, cbmin, cbpa;
      rstat = itsImager->fitpsf(psf, cbmaj, cbmin, cbpa);
      bpa   = *recordFromQuantity(cbpa);
      bmaj  = *recordFromQuantity(cbmaj);
      bmin  = *recordFromQuantity(cbmin);
   }
   catch(AipsError x){
     *itsLog << LogIO::SEVERE << "Exception Reported: "
             << x.getMesg() << LogIO::POST;
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
  Bool rstat(False);
  try {
    *itsLog << LogOrigin("im", "fixvis");
    if(!itsMS->isWritable()){
      *itsLog << LogIO::SEVERE
	      << "Please open im with a writable ms."
              << " (i.e. use im.open(vis, usescratch=True) whether or not"
              << "       you care about scratch columns)"
	      << LogIO::POST;
      return false;
    }
    *itsLog << LogIO::NORMAL2 << "fixvis starting" << LogIO::POST;
      
    casa::FixVis visfixer(*itsMS);
    casa::Vector<casa::Int> cFields(fields);
    int nFields = cFields.nelements();
	
    visfixer.setFields(cFields);

    casa::Vector<casa::MDirection> phaseCenters;
    phaseCenters.resize(nFields);
    for(int i = 0; i < nFields; ++i){
      if(!casaMDirection(phaseDirs[i], phaseCenters[i]))
        throw(AipsError("Could not interpret phaseDirs parameter"));
    }
    visfixer.setPhaseDirs(phaseCenters, cFields);

    visfixer.setDistances(Vector<Double>(distances));
    rstat = visfixer.fixvis(refcode, dataCol);
    *itsLog << LogIO::NORMAL2 << "fixvis finished" << LogIO::POST;  
  }
  catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
            << LogIO::POST;
    Table::relinquishAutoLocks();
    RETHROW(x);
  }
  Table::relinquishAutoLocks();
  return rstat;
}

bool
imager::ft(const std::vector<std::string>& model, const std::string& complist, const bool incremental, const bool async)
{
   Bool rstat(False);
   if(hasValidMS_p){
   try {
      Vector <String> amodel(toVectorString(model));
      rstat = itsImager->ft(amodel, complist, incremental);
    } catch  (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       RETHROW(x);
    }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
    return rstat;
}

bool
imager::linearmosaic(const std::vector<std::string>& images, const std::string& mosaic, const std::string& fluxscale, const std::string& sensitivity, const std::vector<int>& fieldids, const bool usedefaultvp, const std::string& vptable, const bool async)
{

   Bool rstat(False);
   if(hasValidMS_p){
   try {
      Vector <String> aimages(toVectorString(images));
      rstat = itsImager->linearmosaic(mosaic, fluxscale, sensitivity, aimages, fieldids);
    } catch  (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       RETHROW(x);
    }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
    return rstat;
}

bool
imager::make(const std::string& image, const bool async)
{

   Bool rstat(False);
   if(hasValidMS_p){
   try {
      rstat = itsImager->make(image);
    } catch  (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       RETHROW(x);
    }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
    return rstat;
}

bool
imager::makeimage(const std::string& type, const std::string& image, const std::string& compleximage, const bool async)
{
   Bool rstat(False);
   if(hasValidMS_p){
      try {
         rstat = itsImager->makeimage(String(type), String(image), String(compleximage));
      } catch  (AipsError x) {
         *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	 RETHROW(x);
      }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

bool
imager::makemodelfromsd(const std::string& sdimage, const std::string& modelimage, const std::string& sdpsf, const std::string& maskimage)
{
  

    try {
   
      String mask(maskimage);
      if(mask==String(""))
	mask=String(modelimage)+String(".sdmask");
      return itsImager->makemodelfromsd(String(sdimage), String(modelimage), 
					String(sdpsf), mask);
      
    } catch  (AipsError x) {
      *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
    }
  return False;
}

bool
imager::mask(const std::string& image, const std::string& mask, const ::casac::variant& threshold, const bool async)
{
   Bool rstat(False);
   if(hasValidMS_p){
      try {
         rstat = itsImager->mask(mask, image, casaQuantity(threshold));
       } catch  (AipsError x) {
          *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
       }
    } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
    }
    return rstat;
}

bool
imager::mem(const std::string& algorithm, const int niter, const ::casac::variant& sigma, const ::casac::variant& targetflux, const bool constrainflux, const bool displayprogress, const std::vector<std::string>& model, const std::vector<bool>& keepfixed, const std::string& complist, const std::vector<std::string>& prior, const std::vector<std::string>& mask, const std::vector<std::string>& image, const std::vector<std::string>& residual, const bool async)
{

   Bool rstat(False);
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
          *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
      }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

bool
imager::nnls(const std::vector<std::string>& model, const std::vector<bool>& keepfixed, const std::string& complist, const int niter, const double tolerance, const std::vector<std::string>& fluxmask, const std::vector<std::string>& datamask, const std::vector<std::string>& image, const std::vector<std::string>& residual, const bool async)
{

   Bool rstat(False);
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
          *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
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
    bool rstat(False);
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
      if(useScratch){
	itsMS = new MeasurementSet(String(thems), TableLock(TableLock::AutoLocking), Table::Update);
      }
      else{
	itsMS = new MeasurementSet(String(thems), TableLock(TableLock::AutoNoReadLocking), Table::Old);
      }
      // itsImager = new Imager(*itsMS, compress);
      AlwaysAssert(itsMS, AipsError);
      rstat = itsImager->open(*itsMS, compress, useScratch);
      if(rstat) hasValidMS_p=true;
    } catch  (AipsError x) {

      *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
    }
    return rstat;
}

bool
imager::pb(const std::string& inimage, const std::string& outimage,
           const std::string& incomps, const std::string& outcomps,
           const std::string& operation, const std::string& pointingcenter,
           const ::casac::variant& parangle, const std::string& pborvp,
           const bool async)
{
  Bool rstat(False);
  if(hasValidMS_p){
    try{
      MDirection mpointingcenter;
      mdFromString(mpointingcenter, pointingcenter);
      rstat = itsImager->pb(inimage, outimage, incomps, outcomps, operation,
                            mpointingcenter, casaQuantity(parangle), pborvp);
    }
    catch(AipsError x){
      *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
              << LogIO::POST;
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
imager::pixon(const std::string& algorithm, const Quantity& sigma, const std::string& model, const bool async)
{

   Bool rstat(False);
   if(hasValidMS_p){
      try {
	*itsLog << LogIO::WARN << "PIXON is no longer supported; will be removed  "  << LogIO::POST;
         rstat = itsImager->pixon(algorithm, casaQuantity(sigma), model);
      } catch  (AipsError x) {
         *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	 RETHROW(x);
      }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

bool
imager::plotsummary()
{
   Bool rstat(False);
   if(hasValidMS_p){
      try {
        rstat = itsImager->plotsummary();
      } catch  (AipsError x) {
         *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
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

   Bool rstat(False);
   if(hasValidMS_p){
      try {
        rstat = itsImager->plotuv(rotate);
      } catch  (AipsError x) {
         *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
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

   Bool rstat(False);
   if(hasValidMS_p){
      try {
        rstat = itsImager->plotvis(type, increment);
      } catch  (AipsError x) {
         *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
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

   Bool rstat(False);
   if(hasValidMS_p){
      try {
        rstat = itsImager->plotweights(gridded, increment);
      } catch  (AipsError x) {
         *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	 RETHROW(x);
      }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

bool
imager::regionmask(const std::string& mask, const ::casac::record& region, 
		   const ::casac::variant& boxes,const ::casac::variant& circles,  const double value)
{
  Bool rstat(False);
   if(hasValidMS_p){
      try {
	
	casa::Record *pRegion = toRecord(region);
	if(pRegion->nfields() < 2){
	  delete pRegion;
	  pRegion=0;
	}
	casa::Matrix<casa::Quantity> quantBoxes;
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
	  casa::convertArray(theBoxes,localbox.reform(IPosition(shape)));
	  quantBoxes.resize(theBoxes.shape());
	  for (Int k=0; k < shape[1]; ++k){
	    for(Int j=0; j < shape[0]; ++j){
	        quantBoxes(j,k)=casa::Quantity(theBoxes(j,k), "pix");	      
	    } 	    
	  }
	}
	else if(boxes.type() == ::casac::variant::STRINGVEC){
	  Vector<Int> shape = boxes.arrayshape();
	  casa::Vector<casa::Quantity> theCorners;
	  toCasaVectorQuantity(boxes, theCorners);
	  if(casa::product(shape) != Int(theCorners.nelements())){
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
	  Vector<casa::Quantity> refQuantBoxes(quantBoxes.reform(IPosition(1, theCorners.nelements())));
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
	  casa::convertArray(theCircles,localcircles.reform(IPosition(shape)));
	}


	


	
	rstat=itsImager->regionmask(String(mask), pRegion, quantBoxes, 
				    theCircles,Float(value));
	if(pRegion != 0)
	  delete pRegion;

      } catch  (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
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
  Bool rstat(False);
  try {
    
    if(!casa::Table::isWritable(mask))
      throw(AipsError(String("Image ")+String(mask)+String(" is not write accessible")));
    casa::Record *pRegion = toRecord(region);
    if(pRegion->nfields() < 2){
      delete pRegion;
      pRegion=0;
    }
    casa::Matrix<casa::Quantity> quantBoxes;
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
      casa::convertArray(theBoxes,localbox.reform(IPosition(shape)));
      quantBoxes.resize(theBoxes.shape());
      for (Int k=0; k < shape[1]; ++k){
	for(Int j=0; j < shape[0]; ++j){
	  quantBoxes(j,k)=casa::Quantity(theBoxes(j,k), "pix");	      
	} 	    
      }
    }
    else if(boxes.type() == ::casac::variant::STRINGVEC){
      Vector<Int> shape = boxes.arrayshape();
      casa::Vector<casa::Quantity> theCorners;
      toCasaVectorQuantity(boxes, theCorners);
      if(casa::product(shape) != Int(theCorners.nelements())){
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
      Vector<casa::Quantity> refQuantBoxes(quantBoxes.reform(IPosition(1, theCorners.nelements())));
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
      casa::convertArray(theCircles,localcircles.reform(IPosition(shape)));
    }
    rstat=itsImager->regionToImageMask(String(mask), pRegion, quantBoxes, theCircles, Float(value));
    if(pRegion != 0)
      delete pRegion;
    
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  

  return rstat;
}



bool
imager::residual(const std::vector<std::string>& model, const std::string& complist, const std::vector<std::string>& image, const bool async)
{
   Bool rstat(False);
   if(hasValidMS_p){
      try {
         Vector <String> amodel(toVectorString(model));
         Vector <String> aimage(toVectorString(image));
         rstat = itsImager->residual(amodel, complist, aimage);
      } catch  (AipsError x) {
         *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	 RETHROW(x);
      }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

bool
imager::restore(const std::vector<std::string>& model, const std::string& complist, const std::vector<std::string>& image, const std::vector<std::string>& residual, const bool async)
{
   Bool rstat(False);
   if(hasValidMS_p){
      try {
         Vector <String> amodel(toVectorString(model));
         Vector <String> aimage(toVectorString(image));
         Vector <String> aresidual(toVectorString(residual));
         rstat = itsImager->restore(amodel, complist, aimage, aresidual);
      } catch  (AipsError x) {
         *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	 RETHROW(x);
      }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

bool
imager::sensitivity(::casac::record& pointsource, double& relative,
                    double& sumweights, const bool async)
{
   Bool rstat(False);
   if(hasValidMS_p){
      try{ 
        casa::Quantity qpointsource;
        rstat = itsImager->sensitivity(qpointsource, relative, sumweights);
        pointsource = *recordFromQuantity(qpointsource);
      }
      catch(AipsError x){
         *itsLog << LogIO::SEVERE << "Exception Reported: "
                 << x.getMesg() << LogIO::POST;
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
                const ::casac::variant& bpa, const bool async)
{
   Bool rstat(False);
   if(hasValidMS_p){
      try{
        rstat = itsImager->setbeam(casaQuantity(bmaj), casaQuantity(bmin),
                                   casaQuantity(bpa));
      }
      catch(AipsError x){
         *itsLog << LogIO::SEVERE << "Exception Reported: "
                 << x.getMesg() << LogIO::POST;
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
imager::setdata(const std::string& mode, const std::vector<int>& nchan, const std::vector<int>& start, const std::vector<int>& step, const Quantity& mstart, const Quantity& mstep, const std::vector<int>& spwid, const std::vector<int>& fieldid, const std::string& msselect, const bool async)
{
    Bool rstat(False);
    if(hasValidMS_p){
       try {
          rstat = itsImager->setdata(mode, Vector<Int>(nchan), Vector<Int>(start),
                         Vector<Int>(step), casaQuantity(mstart), casaQuantity(mstep), Vector<Int>(spwid),
                         Vector<Int>(fieldid), String(msselect));
       } catch  (AipsError x) {
          *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
       }
    } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
    }
    return rstat;
}

bool
imager::selectvis(const std::string& vis, const std::vector<int>& nchan,
		  const std::vector<int>& start, const std::vector<int>& step, 
                  const ::casac::variant& spw, const ::casac::variant& field,
                  const ::casac::variant& baseline,
		  const ::casac::variant& time,const ::casac::variant& scan,
                  const ::casac::variant& uvrange, const std::string& taql,
                  const bool useScratch)
{
    Bool rstat(False);
    if(itsMS || (vis != "")){
       try {
	 String mode="channel";
	 if(nchan.size()==1 && nchan[0] <0)
	   mode="none";
	 casa::String fieldnames="";
	 casa::Vector<Int> fieldIndex;
	 fieldnames=toCasaString(field);
	 if(fieldnames.contains(String("-"), -1)){
	     fieldnames="";
	     fieldIndex=Vector<Int>();
	 }
	 casa::String antennanames="";
	 Vector<Int> antIndex;
	 antennanames=toCasaString(baseline);
	 if(antennanames.contains(String("-"), -1)){
	   antennanames="";
	   antIndex=Vector<Int>();
	 }
	 casa::String spwstring="";
	 casa::Vector<Int> spwid;
	 spwstring=toCasaString(spw);
	 if(spwstring.contains(String("-"), -1)){
	   spwstring="";
	   spwid=Vector<Int>();
	 }
	 casa::String timerange="";
	 timerange=toCasaString(time);
	 casa::String uvdist="";
	 uvdist=toCasaString(uvrange);
	 casa::String scanrange="";
	 scanrange=toCasaString(scan);
	 

	 rstat = itsImager->setDataPerMS(vis, mode, Vector<Int>(nchan), 
					 Vector<Int>(start),
					 Vector<Int>(step), Vector<Int>(spwid),
					 fieldIndex, 
					 String(taql), String(timerange),
					 fieldnames, antIndex, antennanames, 
                                         spwstring, uvdist, scanrange, useScratch);
	 hasValidMS_p=rstat;
       } catch  (AipsError x) {
          *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
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
		    const ::casac::variant& restfreq, const int facets, 
		    const ::casac::variant& movingsource,
		    const ::casac::variant& distance)
{
  Bool rstat(False);

  if(hasValidMS_p){
    try {
      
      Int nX, nY;
      nX=nx;
      nY=ny;
      if(nY < 1)
	nY=nx;
      casa::Quantity cellX=casaQuantity(cellx);

      if(cellX.getValue()==0.0)
	cellX=casa::Quantity(1.0, "arcsec");

      casa::Quantity cellY;
      if(toCasaString(celly) == String("")){
	cellY=cellX;
      }
      else{
	cellY=casaQuantity(celly);
      }
      casa::MDirection  phaseCenter;
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
      casa::String lamoda(mode);
      lamoda.upcase();
      casa::MRadialVelocity mvel;
      casa::MFrequency mfreq;
      casa::Quantity qstart;
      casa::Quantity qstep;
      casa::Int startoo=-1;
      casa::Int stepoo=0;

      //qstart=casac::image::casaQuantityFromVar(start, *itsLog);
      //qstep=casac::image::casaQuantityFromVar(step, *itsLog);
      if(lamoda.contains(String("VEL"))){
	if(!casaMRadialVelocity(start, mvel)){
	  *itsLog << LogIO::SEVERE << "Could not interprete velocity value " 
		  << LogIO::POST;	
	}
	qstep=casaQuantity(step);
      }
      else if(lamoda.contains(String("FREQ"))){
	if(!casaMFrequency(start, mfreq)){
	  *itsLog << LogIO::SEVERE << "Could not interprete frequency value " 
		  << LogIO::POST;	
	}
	
	qstep=casaQuantity(step);
      }
      else{
	startoo=start.toInt();
	stepoo=step.toInt();      
      }
      
      casa::Quantity restFreq= casaQuantity(restfreq);
      casa::Quantity cdistance=casaQuantity(distance);
      Bool domovingSource=False;
      casa::MDirection movingDir;
      if(toCasaString(movingsource)!=String("")){
	if(!casaMDirection(movingsource, movingDir)){
	    throw(AipsError("Could not interprete phasecenter parameter"));
	}
	domovingSource=True;
      }


      rstat = itsImager->defineImage(nX, nY, cellX, cellY, stokes, phaseCenter, 
				     fieldid, lamoda, nchan, startoo, 
				     stepoo, mfreq, mvel,  qstep, 
				     Vector<Int>(spwid), restFreq, facets,
				     cdistance, domovingSource, movingDir);
    } catch  (AipsError x) {
      *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
    }    
  } else {
    *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open or selectvis" << LogIO::POST;
  }
  return rstat;
}



bool
imager::setimage(const int nx, const int ny, const Quantity& cellx, const Quantity& celly, const std::string& stokes, const bool doshift, const std::string& phasecenter, const Quantity& shiftx, const Quantity& shifty, const std::string& mode, const int nchan, const int start, const int step, const std::string& mstart, const std::string& mstep, const std::vector<int>& spwid, const int fieldid, const int facets, const Quantity& distance)
{

   Bool rstat(False);
   if(hasValidMS_p){
      try {
         casa::Quantity qcellx(casaQuantity(cellx));
         casa::Quantity qcelly(casaQuantity(celly));
         String err;
         MDirection mphaseCenter;
         mdFromString(mphaseCenter, phasecenter);
         casa::Quantity qshiftx(casaQuantity(shiftx));
         casa::Quantity qshifty(casaQuantity(shifty));

//
         casa::MRadialVelocity mmStart;
         mrvFromString(mmStart, mstart);
         casa::MRadialVelocity mmStep;
         mrvFromString(mmStep, mstep);
//
         Vector<Int> mspwid = spwid;
         casa::Quantity qdistance(casaQuantity(distance));
         rstat = itsImager->setimage(nx, ny, qcellx, qcelly, stokes, doshift, mphaseCenter,
                                qshiftx, qshifty, String(mode), nchan, start, step,
                                mmStart, mmStep, mspwid, fieldid, facets, qdistance);
       } catch  (AipsError x) {
          *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
       }
    } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
    }
    return rstat;
}

bool
imager::setjy(const ::casac::variant& field, const ::casac::variant& spw, 
	      const std::string& modimage,
	      const std::vector<double>& fluxdensity, const std::string& standard)
{
   Bool rstat(False);
   if(hasValidMS_p){
      try {
	casa::String fieldnames="";
	casa::Vector<Int> fieldIndex;
	fieldnames=toCasaString(field);
	if(fieldnames.contains(String("-"), -1)){
	  fieldnames="";
	  fieldIndex=Vector<Int>(1,-1);
	}
	casa::String spwstring="";
	casa::Vector<Int> spwid;
	spwstring=toCasaString(spw);
	if(spwstring.contains(String("-"), -1)){
	  spwstring="";
	  spwid=Vector<Int>(1,-1);
	}

	rstat = itsImager->setjy(fieldIndex, spwid, fieldnames, spwstring, 
				 modimage,fluxdensity, standard);
       } catch  (AipsError x) {
          *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
       }
    } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
    }
    return rstat;
}

bool
imager::setmfcontrol(const double cyclefactor, const double cyclespeedup, const int stoplargenegatives, const int stoppointmode, const double minpb, const std::string& scaletype, const double constpb, const std::vector<std::string>& fluxscale)
{
   Bool rstat(False);
   if(hasValidMS_p){
      try {
         Vector <String> afluxscale(toVectorString(fluxscale));
         rstat = itsImager->setmfcontrol(cyclefactor, cyclespeedup, stoplargenegatives,
                                         stoppointmode, scaletype, minpb, constpb, afluxscale);
       } catch  (AipsError x) {
          *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
       }
    } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
    }
    return rstat;
}

bool
imager::setoptions(const std::string& ftmachine, const int cache, const int tile, const std::string& gridfunction, const ::casac::variant& location, const double padding, const std::string& freqinterp, const int wprojplanes, const std::string& epjtablename, const bool applypointingoffsets, const bool dopbgriddingcorrections, const std::string& cfcachedirname, const double pastep, const double pblimit)
{

   Bool rstat(False);
   if(hasValidMS_p){
      try {
          MPosition mlocation;
	  if ((String(location.toString()) != casa::String("")) && 
	      (String(location.toString()) != casa::String("[]")) ){
	    casaMPosition(location, mlocation);
	  }
          rstat = itsImager->setoptions(String(ftmachine), cache, tile, 
					String(gridfunction),
                                        mlocation, padding,
					wprojplanes, String(epjtablename), 
					applypointingoffsets, 
					dopbgriddingcorrections, 
					String(cfcachedirname), Float(pastep), 
					Float(pblimit), String(freqinterp));
       } catch  (AipsError x) {
          *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
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


   Bool rstat(False);
   if(hasValidMS_p){


     try {
       Vector<Float> scalvec(uservector.size());
       convertArray(scalvec, Vector<Double> (uservector));
       rstat = itsImager->setscales(scalemethod, nscales, scalvec);
     } catch  (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
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
  Bool rstat(False);
  if(hasValidMS_p){

     try {
       rstat = itsImager->setSmallScaleBias(inbias);
     } catch  (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
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
   Bool rstat(False);
   if(hasValidMS_p){

     try {
       rstat = itsImager->settaylorterms(ntaylorterms,reffreq);
     } catch  (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       RETHROW(x);
     }
     
   } else {
     *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

bool
imager::setsdoptions(const double scale, const double weight, const int convsupport, const std::string& pointingcolumntouse)
{

   Bool rstat(False);
   if(hasValidMS_p){
      try {
	casa::String pcolToUse(pointingcolumntouse);
	
	rstat = itsImager->setsdoptions(scale, weight, convsupport, pcolToUse);
      } catch  (AipsError x) {
          *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
      }
   } else {
      *itsLog << LogIO::SEVERE << "No MeasurementSet has been assigned, please run open." << LogIO::POST;
   }
   return rstat;
}

bool
imager::setvp(const bool dovp, const bool usedefaultvp, const std::string& vptable, const bool dosquint, const ::casac::variant& parangleinc, const ::casac::variant& skyposthreshold, const std::string& telescope)
{
   Bool rstat(False);
   try {
     casa::Quantity skyposthr(180, "deg");
     casa::Quantity parang(360, "deg");
     if ((String(parangleinc.toString()) != casa::String("")) && 
	 (String(parangleinc.toString()) != casa::String("[]")) )
       parang=casaQuantity(parangleinc);
     if ((String(skyposthreshold.toString()) != casa::String("")) && 
	 (String(skyposthreshold.toString()) != casa::String("[]")))
       skyposthr=casaQuantity(skyposthreshold);
     rstat = itsImager->setvp(dovp, usedefaultvp, vptable, dosquint, parang, 
			      skyposthr, telescope);
   } catch  (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
     RETHROW(x);
   }
   return rstat;
}

bool
imager::smooth(const std::vector<std::string>& model, const std::vector<std::string>& image, const bool usefit, const ::casac::variant& bmaj, const ::casac::variant& bmin, const ::casac::variant& bpa, const bool normalize, const bool async)
{
   Bool rstat(False);
   if(hasValidMS_p){
      try {
         casa::Quantity qbmaj(casaQuantity(bmaj));
         casa::Quantity qbmin(casaQuantity(bmin));
         casa::Quantity qbpa(casaQuantity(bpa));
         Vector<String> amodel(toVectorString(model));
         Vector<String> aimage(toVectorString(image));
         rstat = itsImager->smooth(amodel, aimage, usefit,
                         qbmaj, qbmin, qbpa, normalize);
       } catch  (AipsError x) {
          *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
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
   return False;
}

bool
imager::summary()
{

   Bool rstat(False);
   if(hasValidMS_p){
      try {
         rstat = itsImager->summary();
      } catch  (AipsError x) {
         *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
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

   Bool rstat(False);
   if(hasValidMS_p){
      try {
         rstat = itsImager->uvrange(uvmin, uvmax);
         if(!rstat)
            *itsLog << LogIO::WARN << "uvrange failed did you setdata first?" << LogIO::POST;
      } catch  (AipsError x) {
         *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
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
               const bool mosaic, const bool async)
{
  Bool rstat(False);
  if(hasValidMS_p){
    try{
      rstat = itsImager->weight(String(type), String(rmode), casaQuantity(noise),
                                robust, casaQuantity(fieldofview), npixels);
    }
    catch(AipsError x){
      *itsLog << LogIO::SEVERE << "Exception Reported: "
              << x.getMesg() << LogIO::POST;
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

bool imager::mpFromString(casa::MPosition &thePos, const casa::String &in)
{
   bool rstat(false);
   return rstat;
}

bool imager::mdFromString(casa::MDirection &theDir, const casa::String &in)
{
   bool rstat(false);
   String tmpA, tmpB, tmpC;
   std::istringstream iss(in);
   iss >> tmpA >> tmpB >> tmpC;
   casa::Quantity tmpQA;
   casa::Quantity tmpQB;
   casa::Quantity::read(tmpQA, tmpA);
   casa::Quantity::read(tmpQB, tmpB);
   if(tmpC.length() > 0){
      MDirection::Types theRF;
      MDirection::getType(theRF, tmpC);
      theDir = MDirection (tmpQA, tmpQB, theRF);
      rstat = true;
   } else {
      theDir = MDirection (tmpQA, tmpQB);
      rstat = true;
   }
   return rstat;
}

bool imager::mrvFromString(casa::MRadialVelocity &theRadialVelocity,
                           const casa::String &in)
{
   String tmpA, tmpB, tmpC;
   bool rstat(false);
   std::istringstream iss(in);
   iss >> tmpA >> tmpB >> tmpC;
   casa::Quantity tmpQ;
   casa::Quantity::read(tmpQ, tmpA);
   if(tmpQ.getUnit().length() == 0){
      tmpQ.setUnit(Unit(tmpB));
      if(tmpC.length() > 0){
         MRadialVelocity::Types theRF;
         MRadialVelocity::getType(theRF, String(tmpC));
         theRadialVelocity = MRadialVelocity(tmpQ, theRF);
         rstat = true;
      } else {
         theRadialVelocity = MRadialVelocity(tmpQ);
         rstat = true;
      }
   }else{
      if(tmpB.length() > 0){
         MRadialVelocity::Types theRF;
         MRadialVelocity::getType(theRF, String(tmpB));
         theRadialVelocity = MRadialVelocity(tmpQ, theRF);
         rstat = true;
      } else {
         theRadialVelocity = MRadialVelocity(tmpQ);
         rstat = true;
      }
   }
   return rstat; 
}

} // casac namespace
