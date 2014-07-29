/***
 * Framework independent implementation file for imager...
 *
 * Implement the imager component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 
 ***/

#include <iostream>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Utilities/Assert.h>
#include <ms/MeasurementSets.h>
#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <casa/Logging/LogIO.h>

#include <synthesis/ImagerObjects/SynthesisImager.h>
#include <synthesis/ImagerObjects/SynthesisUtilMethods.h>

#include <synthesisimager_cmpt.h>

using namespace std;
using namespace casa;

     
namespace casac {

synthesisimager::synthesisimager() 
{
  itsImager = new SynthesisImager();
}

synthesisimager::~synthesisimager()
{
  done();
}


/*
bool
synthesisimager::selectdata(const std::string& msname, 
			    const std::string& spw,
			    const std::string& freqbeg, // ? why here ?
			    const std::string& freqend, // ? why here ?
			    const std::string& freqframe, // ? why here ?
			    const std::string& field, 
			    const std::string& antenna,
			    const std::string& timestr,
			    const std::string& scan,
			    const std::string& obs,
			    const std::string& state,
			    const std::string& uvdist,
			    const std::string& taql,
			    const bool usescratch,
			    const bool readonly,
			    const bool incrmodel)
*/
bool 
synthesisimager::selectdata(const casac::record& selpars)
{
  Bool rstat(False);
  
  try 
    {

      if( ! itsImager ) itsImager = new SynthesisImager();

      casa::Record recpars = *toRecord( selpars );
      SynthesisParamsSelect pars;
      pars.fromRecord( recpars );

      itsImager->selectData( pars );

      //      itsImager->selectData( pars.msname, pars.spw, 
      //		     pars.freqbeg, pars.freqend, pars.freqframe,
      //		     pars.field, pars.antenna, pars.timestr, pars.scan, 
      //		     pars.obs, pars.state, pars.uvdist, pars.taql,
      //		     pars.usescratch, pars.readonly, pars.incrmodel );

    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}


bool synthesisimager::defineimage(const casac::record& impars, const casac::record& gridpars)
{
  Bool rstat(False);

  try 
    {
    
    if( ! itsImager ) itsImager = new SynthesisImager();
    
    casa::Record irecpars = *toRecord( impars );
    SynthesisParamsImage ipars;
    ipars.fromRecord( irecpars );

    casa::Record grecpars = *toRecord( gridpars );
    SynthesisParamsGrid gpars;
    gpars.fromRecord( grecpars );

    itsImager->defineImage( ipars, gpars );
 
    /*
    itsImager->defineImage( ipars.imageName, ipars.imsize[0], ipars.imsize[1], 
			    ipars.cellsize[0], ipars.cellsize[1], ipars.stokes, ipars.phaseCenter,
			    ipars.nchan, ipars.freqStart, ipars.freqStep, ipars.restFreq, ipars.facets, 
			    gpars.ftmachine, ipars.nTaylorTerms, ipars.refFreq, 
			    ipars.projection, gpars.distance, ipars.freqFrame, 
			    gpars.trackSource, gpars.trackDir, ipars.overwrite,
			    gpars.padding, gpars.useAutoCorr, gpars.useDoublePrec, gpars.wprojplanes, 
			    gpars.convFunc, ipars.startModel, gpars.aTermOn,
			    gpars.psTermOn, gpars.mTermOn, gpars.wbAWP, gpars.cfCache,
			    gpars.doPointing,gpars.doPBCorr,gpars.conjBeams,
			    gpars.computePAStep,gpars.rotatePAStep);
    */

  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

/*
bool
synthesisimager::defineimage(const std::string& imagename,
			     const int nx, 
			     const int ny,			
			     const ::casac::variant& cellx, 
			     const ::casac::variant& celly,
			     const std::string& stokes,
			     const ::casac::variant& phasecenter,
			     const int nchan,
			     const ::casac::variant& freqstart,
			     const ::casac::variant& freqstep,
			     const ::casac::variant& restfreq,
			     const int facets,
			     const std::string& ftmachine,
			     const int ntaylorterms,
			     const ::casac::variant& reffreq,
			     const std::string& projection,
			     const ::casac::variant& distance,
			     const std::string& freqframe, 
			     const bool tracksource,
			     const ::casac::variant& trackdir,
			     const bool overwrite,
			     const float padding,
			     const bool useautocorr,
			     const bool usedoubleprec,
			     const int wprojplanes,
			     const std::string& convfunc,
			     const std::string& startmodel,

			     const bool aterm,//    = True,
			     const bool psterm,//   = True,
			     const bool mterm,//    = False,
			     const bool wbawp,//      = True,
			     const std::string& cfcache,//  = "",
			     const bool dopointing,// = False,
			     const bool dopbcorr,//   = True,
			     const bool conjbeams,//  = True,
			     const float computepastep,         //=360.0
			     const float rotatepastep          //=5.0
			     )
{
  Bool rstat(False);

  try 
    {

      if( ! itsImager ) itsImager = new SynthesisImager();   // any more here ?

      // Check nx, ny
      Int nX, nY;
      nX=nx;
      nY=ny;
      if(nY < 1)
	nY=nx;

      // Convert cellx, celly
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

      // Convert phasecenter ( If it is an integer, it's a field, id, connect to last MS.... )
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
	    throw(AipsError("cmpt : Could not interprete phasecenter parameter"));
	  }
	}
      }


      // Convert projection.
      casa::String projectionStr = toCasaString( projection );
      casa::Projection imageprojection = Projection::type( projectionStr );

      // Convert distance
      casa::Quantity cdistance = casaQuantity( distance );

      // Convert trackDir
      casa::MDirection  trackDir;
      if( toCasaString(trackdir) != casa::String("")){
	  if(!casaMDirection(trackdir, trackDir)){
	    throw(AipsError("cmpt : Could not interprete trackdir parameter"));
	  }
	}

      //----------------------------------------------------------------------------------------------------------------
      //------------------------------- Frequency Frame Setup : Start ----------------------------------
      //----------------------------------------------------------------------------------------------------------------
      //------ Convert all inputs into LSRK start, step and reference -------------------------
      //----------------------------------------------------------------------------------------------------------------

      // Convert freqstart, freqstep, restfreq - whatever units.
      casa::Quantity freqStart, freqStep, refFreq;
      freqStart = casaQuantity(freqstart);
      freqStep = casaQuantity(freqstep);
      refFreq = casaQuantity(reffreq);

      // Convert rest-freq
      casa::Vector<casa::Quantity> restFreq;
      toCasaVectorQuantity( restfreq, restFreq );

      // Convert freqframe
      casa::MFrequency::Types freqframetype;
      if( !casa::MFrequency::getType(freqframetype, freqframe) )
	throw(AipsError("cmpt : Invalid Frequency Frame " + freqframe));

      //----------------------------------------------------------------------------------------------------------------
      //------------------------------- Frequency Frame Setup : End ------------------------------------
      //----------------------------------------------------------------------------------------------------------------


      itsImager->defineImage( imagename, nX, nY, cellX, cellY, stokes, phaseCenter,
			      nchan, freqStart, freqStep, restFreq, facets, ftmachine, 
			      ntaylorterms, refFreq, 
			      imageprojection, cdistance, freqframetype, tracksource, trackDir, overwrite,
			      padding, useautocorr, usedoubleprec, wprojplanes, convfunc, startmodel, aterm,
			      psterm, mterm,wbawp, cfcache,dopointing,dopbcorr,conjbeams,computepastep,rotatepastep);
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}

*/

bool synthesisimager::setweighting(const std::string& type,
				   const std::string& rmode,
				   const ::casac::variant& noise,
				   const double robust,
				   const ::casac::variant& fieldofview,
				   const int npixels,
				   const bool multifield,
				   const std::vector<std::string>& uvtaper
				   /*				   const std::string& filtertype,
				   const ::casac::variant& filterbmaj,
				   const ::casac::variant& filterbmin,
				   const ::casac::variant& filterbpa */
				   )
{
  Bool rstat(False);

  try 
    {

      if( ! itsImager ) itsImager = new SynthesisImager(); // More here ? Check that defineImage has been called

      casa::Quantity cnoise = casaQuantity( noise );
      casa::Quantity cfov = casaQuantity( fieldofview );

      Vector<String> uvtaperpars( toVectorString(uvtaper) );

      casa::Quantity bmaj(0.0,"deg"), bmin(0.0,"deg"), bpa(0.0,"deg");
      String filtertype("");
      if(uvtaperpars.nelements()>0) bmaj = casaQuantity( uvtaperpars[0] );
      if(uvtaperpars.nelements()>1) bmin = casaQuantity( uvtaperpars[1] );
      if(uvtaperpars.nelements()>2) bpa = casaQuantity( uvtaperpars[2] );
      if(uvtaperpars.nelements()==1) bmin = bmaj;

      if(uvtaperpars.nelements()>0 && uvtaperpars[0].length()>0) filtertype=String("gaussian");

      itsImager->weight( type, rmode, cnoise, robust, cfov, npixels, multifield, filtertype, bmaj, bmin, bpa  );

    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}

  bool synthesisimager::makepsf()
  {
    Bool rstat(False);
    
    try {
      
      if( ! itsImager ) itsImager = new SynthesisImager();
      
      itsImager->makePSF();
      
    } catch  (AipsError x) {
      RETHROW(x);
    }
    return rstat;
  }

  bool synthesisimager::predictmodel()
  {
    Bool rstat(False);
    
    try {
      
      if( ! itsImager ) itsImager = new SynthesisImager();
      
      itsImager->predictModel();
      
    } catch  (AipsError x) {
      RETHROW(x);
    }
    return rstat;
  }

  bool synthesisimager::predictcalmodel()
  {
    Bool rstat(False);
    
    try {
      
      if( ! itsImager ) itsImager = new SynthesisImager();
      
      itsImager->predictCalModel();
      
    } catch  (AipsError x) {
      RETHROW(x);
    }
    return rstat;
  }


bool synthesisimager::executemajorcycle(const casac::record& controls)
{
  Bool rstat(False);

  try {

      if( ! itsImager ) itsImager = new SynthesisImager();

    casa::Record recpars = *toRecord( controls );
    itsImager->executeMajorCycle( recpars, False/*usevb2*/ );

  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

synthesisimstore* synthesisimager::getimstore(const int id)
{
  synthesisimstore *rstat;
  try {

      if( ! itsImager ) itsImager = new SynthesisImager();

      rstat = new synthesisimstore( &*(itsImager->imageStore( id )) );

  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}


bool
synthesisimager::done()
{
  Bool rstat(False);

  try 
    {
      if (itsImager)
	{
	  delete itsImager;
	  itsImager=NULL;
	}
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}


  /* PRIVATE HELPER FUNCTIONS */

casa::String checkStr(std::string instr)
{
  casa::String cstr = toCasaString( instr );
  if( cstr == String("-1") )
    {
      cstr = "";
    }
  return cstr;
}


/*
  bool synthesisimager::setupdeconvolution(const casac::record& decpars)
{
  Bool rstat(False);

  try 
    {
      casa::Record rec = *toRecord( decpars );
      itsDeconvolver->setupDeconvolution( rec );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}
*/
 /*
casac::record* synthesisimager::getiterationdetails()
{
  casac::record* rstat(0);

  try 
    {
      rstat=fromRecord(itsIterBot->getIterationDetails());
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}
 */

  /*
casac::record* synthesisimager::getiterationsummary()
{
  casac::record* rstat(0);

  try 
    {
      rstat=fromRecord(itsIterBot->getIterationSummary());
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}
  */

   /*
casac::record* synthesisimager::setupiteration(const casac::record& iterpars)
{

  try 
    {
      casa::Record recpars = *toRecord( iterpars );
      itsIterBot->setupIteration( recpars );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return getiterationdetails();
}
   */

/*
bool synthesisimager::cleanComplete()
{
  Bool rstat(False);

  try 
    {
      rstat = itsIterBot->cleanComplete( );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}
*/

  /*
   casac::record* synthesisimager::endloops()
  {
  try 
    {
      itsImager->endLoops();
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return getiterationdetails();
  }
  */

 /*
bool synthesisimager::runmajorcycle()
{
  Bool rstat(False);

  try 
    {
      // This is a convenience function for tool-level usage, for the non-parallel case.
      // Duplicates the code from getmajorcyclecontrols(), executemajorcycle(), endmajorcycle().
      casa::Record recpars; // = itsImager->getMajorCycleControls();
      itsImager->executeMajorCycle( recpars );
      itsIterBot->endMajorCycle();
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}
 */
  /*
casac::record* synthesisimager::getmajorcyclecontrols()
{
  casac::record* rstat(0);
  try {
    rstat=fromRecord(itsImager->getMajorCycleControls());
  } catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}  
  */

/*
bool synthesisimager::endmajorcycle()
{
  Bool rstat(False);
  
  try 
    {
      itsIterBot->endMajorCycle();
     } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}
*/

 /*
bool synthesisimager::runminorcycle()
{
  Bool rstat(False);
  
  try 
    {
      // This is a convenience function for tool-level usage, for the non-parallel case.
      // Duplicates the code from getsubiterbot(), executeminorcycle(), endminorcycle().
      casa::Record iterbotrec = itsIterBot->getSubIterBot();
      iterbotrec = itsDeconvolver->executeMinorCycle(iterbotrec);
      itsImager->endMinorCycle(iterbotrec);
     } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}
 */

  /*
casac::record* synthesisimager::getsubiterbot()
{
  casac::record* rstat(0);
  try {
    rstat=fromRecord(itsImager->getSubIterBot());
  } catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}  
  */

   /*
casac::record* synthesisimager::executeminorcycle(const casac::record& iterbot)
{
  casac::record* rstat(False);
  try {
    casa::Record recpars = *toRecord( iterbot );
    rstat = fromRecord(itsImager->executeMinorCycle( recpars ));
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}
   */
    /*
bool synthesisimager::endminorcycle(const casac::record& iterbot)
{
  Bool rstat(False);
  
  try 
    {
      casa::Record recpars = *toRecord( iterbot );
      itsImager->endMinorCycle(recpars);
     } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}
    */


} // casac namespace
