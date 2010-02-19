
/***
 * Framework independent implementation file for simulator...
 *
 * Implement the simulator component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <xmlcasa/synthesis/simulator_cmpt.h>
#include <casa/Exceptions/Error.h>
#include <casa/Containers/Record.h>
#include <synthesis/MeasurementEquations/Simulator.h>
#include<casa/BasicSL/String.h>
#include<casa/Utilities/Assert.h>
#include<measures/Measures/MDirection.h>
#include<measures/Measures/MPosition.h>
#include<measures/Measures/MEpoch.h>
#include <measures/Measures/MeasureHolder.h>
#include<casa/Quanta/QuantumHolder.h>
#include<ms/MeasurementSets.h>
#include <casa/Logging/LogIO.h>

using namespace std;
using namespace casa;

namespace casac {

simulator::simulator()
{

  itsSim=0;
  itsLog = new LogIO();

}

simulator::~simulator()
{

  if(itsSim !=0)
    delete itsSim;
  
  delete itsLog;

}

bool
simulator::open(const std::string& ms)
{
  Bool rstat(False);
  try {

    // In case already open, close it!
    close();

    String lems(ms);
    itsSim=new Simulator(lems);
    if(itsSim !=0)
      rstat=True;


  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }

  return rstat;

}

bool
simulator::openfromms(const std::string& ms)
{

  Bool rstat(False);

  try {

    // In case already open, close it!
    close();

    MeasurementSet thems(ms,TableLock(TableLock::AutoLocking), Table::Update);
    itsSim=new Simulator(thems);
    if(itsSim !=0)
      rstat=True;


  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }

  return rstat;

}

bool
simulator::close()
{
  Bool rstat(False);
  
  try {
  
    if(itsSim !=0)
      delete itsSim;
    itsSim=0;
    rstat=True;


  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }

  return rstat;

  
}

bool
simulator::done()
{

Bool rstat(False);
  
  try {
  
    if(itsSim !=0)
      delete itsSim;
    itsSim=0;
    rstat=True;


  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }

  return rstat;


}

std::string
simulator::name()
{

  std::string nameString;
try {
  
    if(itsSim !=0)
      nameString=itsSim->name();


  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }

 return nameString;
    
}

bool
simulator::summary()
{

 Bool rstat(False);
  
  try {
  
    if(itsSim !=0)
      rstat=itsSim->summary();
   
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }

  return rstat;

}

std::string
simulator::type()
{

  return std::string("Simulator");
}

bool
simulator::settimes(const ::casac::variant& integrationtime, const bool usehourangle, const ::casac::variant& referencetime)
{
 Bool rstat(False);
  
  try {
  
    if(itsSim !=0){
      MEpoch lepoch;
      if (!casaMEpoch(referencetime, lepoch)){
	*itsLog << LogIO::SEVERE 
		<< "Could not convert referencetime to an Epoch Measures"
		<< LogIO::POST;
	
      }
      casa::Quantity qIntTime(casaQuantity(integrationtime));
      rstat=itsSim->settimes(qIntTime, usehourangle, lepoch);
    }


  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }

  return rstat;

 
}

bool
simulator::observe(const std::string& sourcename, const std::string& spwname, const ::casac::variant& starttime, const ::casac::variant& stoptime)
{
Bool rstat(False);
  
  try {
  
    if(itsSim !=0){
      casa::Quantity qstarttime(casaQuantity(starttime));
      casa::Quantity qstoptime(casaQuantity(stoptime));
      rstat=itsSim->observe(sourcename, spwname, qstarttime, qstoptime);
    }


  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }

  return rstat;

}


bool
//simulator::observemany(const std::vector<string>& sourcenames, const std::string& spwname, const ::casac::variant& starttimes, const ::casac::variant& stoptimes, const ::casac::variant& directions)
simulator::observemany(const std::vector<string>& sourcenames, const std::string& spwname, const std::vector<string>& starttimes, const std::vector<string>& stoptimes, const std::vector<string>& directions)
{
Bool rstat(False);
  
  try {
  
    if(itsSim !=0){
      casa::String sspwname(spwname);
      Vector<String> ssourcenames(toVectorString(sourcenames));
//      Vector<String> ssourcenames();
//     Vector<String> inFiles;
//     if (infiles.type() == ::casac::variant::BOOLVEC) {
//	inFiles.resize(0);      // unset
//     } else if (infiles.type() == ::casac::variant::STRING) {
//	sepCommaEmptyToVectorStrings(inFiles, infiles.toString());
//     } else if (infiles.type() == ::casac::variant::STRINGVEC) {
//	inFiles = toVectorString(infiles.toStringVec());
//     } else {
//	*itsLog << LogIO::WARN << "Unrecognized infiles datatype"
//		<< LogIO::POST;
//     }
      Vector<String> sstarttimes(toVectorString(starttimes));
      Vector<String> sstoptimes(toVectorString(stoptimes));
      Vector<String> sdirections(toVectorString(directions));

      uInt nptg=ssourcenames.nelements();
      casa::Vector<casa::Quantity> qstarttimes(nptg);
      casa::Vector<casa::Quantity> qstoptimes(nptg);
      casa::Vector<casa::MDirection> mdirections(nptg);
      MDirection mdir;
      for (uInt i=0; i<nptg; i++) {
	qstarttimes[i]=casaQuantity(sstarttimes[i]);
	qstoptimes[i]=casaQuantity(sstoptimes[i]);
	if (!casaMDirection(sdirections[i], mdir)){
	  *itsLog << LogIO::SEVERE 
		  << "Could not convert direction to a Direction Measure."
		  << LogIO::POST;
	}
	mdirections[i]=mdir;
      }
      rstat=itsSim->observemany(ssourcenames, sspwname, qstarttimes, qstoptimes, mdirections);
    }


  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }

  return rstat;

}

bool
simulator::setlimits(const double shadowlimit, const ::casac::variant& elevationlimit)
{

  Bool rstat(False);
  try {
  
    if(itsSim !=0){
      casa::Quantity qelevationlimit(casaQuantity(elevationlimit));
      rstat=itsSim->setlimits(shadowlimit, qelevationlimit);
    }

    
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  
  return rstat;

}

bool
simulator::setauto(const double autocorrwt)
{

  Bool rstat(False);
  try {
  
    if(itsSim !=0){
      rstat=itsSim->setauto(autocorrwt);
    }
    

  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }

  return rstat;
  
}


bool
simulator::setconfig(const std::string& telescopename, const std::vector<double>& x, const std::vector<double>& y, const std::vector<double>& z, const std::vector<double>& dishdiameter, const std::vector<double>& offset, const std::vector<std::string>& mount, const std::vector<std::string>& antname, const std::vector<std::string>& padname, const std::string& coordsystem, const ::casac::variant& referencelocation)
{
  Bool rstat(False);
  try {
  
    if(itsSim !=0){
      MPosition mpos;
      if(!casaMPosition(referencelocation, mpos)){
	*itsLog << LogIO::SEVERE 
		<< "Could not convert referencelocation to a Position Measures"
		<< LogIO::POST;

	
      }
      rstat=itsSim->setconfig(telescopename, x, y, z, dishdiameter, offset,  
			      toVectorString(mount), toVectorString(antname), 
			      toVectorString(padname), 
			      coordsystem, mpos );
    }

    
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  
  return rstat;
  
}

bool
simulator::setknownconfig(const std::string& arrayname)
{

  Bool rstat(False);
  try {
    
    if(itsSim !=0){
      //      rstat=itsSim->setknownconfig(arrayname);
      //NEED TO IMPLEMENT known config from glish
    }
    
    
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  
  return rstat;


}

bool
simulator::setfeed(const std::string& mode, const std::vector<double>& x, const std::vector<double>& y, const std::vector<std::string>& pol)
{

Bool rstat(False);
  try {
    
    if(itsSim !=0){
      
      rstat=itsSim->setfeed(mode,x,y,toVectorString(pol));
    }
    
    
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  
  return rstat;


}

bool
simulator::setfield(const std::string& sourcename, const ::casac::variant& sourcedirection, const std::string& calcode, const ::casac::variant& distance)
{

  Bool rstat(False);
  try {
    
    if(itsSim !=0){
    
      MDirection mdir;
      if (!casaMDirection(sourcedirection, mdir)){
	*itsLog << LogIO::SEVERE 
		<< "Could not convert source direction to a Direction Measure."
		<< LogIO::POST;
	
      }
      casa::Quantity qdist(casaQuantity(distance));
      rstat=itsSim->setfield(sourcename, mdir, calcode, qdist);
    }
    
    
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  
  return rstat;



}

bool
simulator::setmosaicfield(const std::string& sourcename, 
			  const std::string& calcode, 
			  const ::casac::variant& fieldcenter, const int xmosp,
			  const int ymosp, const ::casac::variant& mosspacing, 
			  const ::casac::variant& distance)
{


Bool rstat(False);
 try {
    
   if(itsSim !=0){
     
     MDirection mdir;
     if (!casaMDirection(fieldcenter, mdir)){
	*itsLog << LogIO::SEVERE 
		<< "Could not convert field center to a Direction Measures"
		<< LogIO::POST;
     }
     casa::Quantity qdist(casaQuantity(distance));
     casa::Quantity qmosspacing(casaQuantity(mosspacing));
     rstat=itsSim->setmosaicfield(sourcename, calcode, mdir, xmosp, ymosp, 
				  qmosspacing, qdist);
   }
   
   
 } catch  (AipsError x) {
   *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	   << LogIO::POST;
    RETHROW(x);
 }
  
 return rstat;


}

bool
simulator::setspwindow(const std::string& spwname, 
		       const ::casac::variant& freq,
		       const ::casac::variant& deltafreq, 
		       const ::casac::variant& freqresolution, 
		       const int nchannels, const std::string& stokes)
{

Bool rstat(False);
 try {
    
   if(itsSim !=0){
     
     casa::Quantity qfreq(casaQuantity(freq));
     casa::Quantity qdeltafreq(casaQuantity(deltafreq));
     casa::Quantity qfreqres(casaQuantity(freqresolution));
     rstat=itsSim->setspwindow(spwname, qfreq, qdeltafreq, qfreqres, nchannels,
			       stokes);
   }
   
   
 } catch  (AipsError x) {
   *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	   << LogIO::POST;
    RETHROW(x);
 }
  
 return rstat;

}

bool
simulator::setdata(const std::vector<int>& spwid, const std::vector<int>& fieldid, const std::string& msselect)
{

 Bool rstat(False);
 try {
   
   if(itsSim !=0){
     rstat=itsSim->setdata(spwid, fieldid, msselect);
   }
    
    
 } catch  (AipsError x) {
   *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	   << LogIO::POST;
    RETHROW(x);
 }
  
 return rstat;
 

}

bool
simulator::predict(const std::vector<std::string>& modelImage, const std::string& complist, const bool incremental)
{

  Bool rstat(False);
  try {
    
    if(itsSim !=0){
      rstat=itsSim->predict(toVectorString(modelImage), complist, incremental);
    }
    
    
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  
  return rstat;

}

bool
simulator::setoptions(const std::string& ftmachine, const int cache, 
		      const int tile, const std::string& gridfunction, 
		      const ::casac::variant& location, const double padding,
		      const int facets, const double maxdata, 
		      const int wprojplanes)
{

  Bool rstat(False);
  try {
    
    if(itsSim !=0){
      
      MPosition mpos;
      if (!casaMPosition(location, mpos)){
	*itsLog << LogIO::SEVERE 
		<< "Could not convert location to a Position Measures"
		<< LogIO::POST;
      }
      rstat=itsSim->setoptions(ftmachine, cache, tile, gridfunction, mpos, padding, facets, maxdata, wprojplanes);
    }
    
    
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  
  return rstat;

}

bool
simulator::setvp(const bool dovp, const bool usedefaultvp, 
		 const std::string& vptable, const bool dosquint, 
		 const ::casac::variant& parangleinc, 
		 const ::casac::variant& skyposthreshold, const double pblimit)
{

  Bool rstat(False);
  try {
    
    if(itsSim !=0){
      casa::Quantity skyposthr(180, "deg");
      casa::Quantity parang(360, "deg");
      if ((String(parangleinc.toString()) != casa::String("")) && 
	  (String(parangleinc.toString()) != casa::String("[]")) )
	parang=casaQuantity(parangleinc);
      if ((String(skyposthreshold.toString()) != casa::String("")) && 
	  (String(skyposthreshold.toString()) != casa::String("[]")))
	skyposthr=casaQuantity(skyposthreshold);
      rstat=itsSim->setvp(dovp, usedefaultvp, vptable, dosquint,parang, 
			  skyposthr, pblimit);
    }
    
    
 } catch  (AipsError x) {
   *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	   << LogIO::POST;
    RETHROW(x);
 }
  
 return rstat;
   



}

bool
simulator::corrupt()
{

  Bool rstat(False);
  try {
    
    if(itsSim !=0){
      rstat=itsSim->corrupt();
    }
    
    
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  
  return rstat;
}

bool
simulator::reset()
{
  Bool rstat(False);
  try {
    
    if(itsSim !=0){
      rstat=itsSim->resetviscal();
      rstat=itsSim->resetimcal();
    }
    
    
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  
  return rstat;

}

bool
simulator::setbandpass(const std::string& mode, const std::string& table, 
		       const ::casac::variant& interval, 
		       const std::vector<double>& amplitude)
{

  Bool rstat(False);
  try {
    
    if(itsSim !=0){
      
      casa::Quantity qinter(casaQuantity(interval));
      rstat=itsSim->setbandpass(mode, table, qinter, amplitude);
    }
    
    
 } catch  (AipsError x) {
   *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	   << LogIO::POST;
    RETHROW(x);
 }
  
 return rstat;
   

}

bool 
simulator::setapply(const std::string& table, 
		    const std::string& type, 
		    const double t, 		    
		    const ::casac::variant& field,
		    const std::string& interp,
		    const std::string& select,
		    const bool calwt,
		    const std::vector<int>& spwmap,
		    const double opacity)
{
  Bool rstat(False);
  try {
    
    LogIO os(LogOrigin("simulator", "setapply"));
    os << "Beginning setapply--------------------------" << LogIO::POST;

    // Forward to the Simulator object
    if (itsSim)
      rstat = itsSim->setapply(type,t,table,
			       "",toCasaString(field),
			       interp,calwt,spwmap,opacity);
    
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  
  return rstat;

}



// RI TODO make pwv and windspeed variants here and quantities in Simulator.cc

bool
simulator::settrop(const std::string& mode, const std::string& table, 
		   const double pwv, const double deltapwv, 
		   const double beta, const double windspeed)
{
  Bool rstat(False);
  try {
    
    if(itsSim !=0){  
      //      casa::Quantity qinter(casaQuantity(interval));
      rstat=itsSim->settrop(mode, table, pwv, deltapwv, beta, windspeed);
    }
    // RI TODO interpolation params have to get to SolvableVisCal::setApply.
    // RI TODO do we make the user call sm.setapply to deal with that, 
    // RI TODO or do we have it pass through here to VC::setApply? 
      
 } catch  (AipsError x) {
   *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	   << LogIO::POST;
    RETHROW(x);
 }
 return rstat;  
}






bool
simulator::setgain(const std::string& mode, const std::string& table, 
		   const ::casac::variant& interval, 
		   const std::vector<double>& amplitude)
{
  Bool rstat(False);
  try {
    
    if(itsSim !=0){
      
      casa::Quantity qint(casaQuantity(interval));
      rstat=itsSim->setgain(mode, table, qint, amplitude);
    }
    
    
 } catch  (AipsError x) {
   *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	   << LogIO::POST;
    RETHROW(x);
 }
  
 return rstat;
   
}




bool 
simulator::setpointingerror(const std::string& epjtablename, 
			    const bool applypointingoffsets, 
			    const bool dopbcorrection){

  Bool rstat(False);
  try {
    
    if(itsSim !=0){
      
      rstat=itsSim->setpointingerror(epjtablename, applypointingoffsets, dopbcorrection);
    }
    
    
 } catch  (AipsError x) {
   *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	   << LogIO::POST;
    RETHROW(x);
 }
  
 return rstat;
   

}



bool
simulator::setleakage(const std::string& mode, const std::string& table, 
		      //const ::casac::variant& interval, 
		      const std::vector<double>& amplitude,
		      const std::vector<double>& offset)
{

  Bool rstat(False);
  try {
    
    if(itsSim !=0){
      
      //casa::Quantity qinter(casaQuantity(interval));
      //rstat=itsSim->setleakage(mode, table, qinter, amplitude);
      rstat=itsSim->setleakage(mode, table, amplitude, offset);
    }
    
    
 } catch  (AipsError x) {
   *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	   << LogIO::POST;
    RETHROW(x);
 }
  
 return rstat;
   
   
}

bool
simulator::oldsetnoise(const std::string& mode, const std::string& table, 
		    const ::casac::variant& simplenoise, 
		    const double antefficiency, const double correfficiency, 
		    const double spillefficiency, const double tau, 
		    const double trx, const double tatmos, const double tcmb)
{
  Bool rstat(False);
  try {
    
    if(itsSim !=0){
      
      LogIO os(LogOrigin("simulator", "oldsetnoise"));
      os << LogIO::WARN << "Using deprecated ACoh Noise - this will dissapear in the future - please switch to sm.setnoise" << LogIO::POST;
      
      casa::Quantity qnoise(casaQuantity(simplenoise));
      rstat=itsSim->oldsetnoise(mode, table, qnoise, antefficiency, correfficiency, spillefficiency, tau, trx, tatmos, tcmb);
    }
    
    
 } catch  (AipsError x) {
   *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	   << LogIO::POST;
    RETHROW(x);
 }  
 return rstat;
}


bool
simulator::setnoise(const std::string& mode, 
		    const std::string& table, 
		    const ::casac::variant& simplenoise, 
		    // atm parameters
		    const ::casac::variant& pground,
		    const double relhum,
		    const ::casac::variant& altitude,
		    const ::casac::variant& waterheight,
		    const ::casac::variant& pwv,
		    // OR specify tau and tatmos 
		    const double tatmos, 
		    const double tau,
		    // antenna parameters
		    const double antefficiency, 
		    const double spillefficiency, 
		    const double correfficiency,	    
		    const double trx, 
		    const double tground, 
		    const double tcmb
		    ) {
  Bool rstat(False);
  try {
    if(itsSim !=0){      
      casa::Quantity qnoise(casaQuantity(simplenoise));
      casa::Quantity qpress(casaQuantity(pground));
      casa::Quantity qalt(casaQuantity(altitude));
      casa::Quantity qwaterht(casaQuantity(waterheight));
      casa::Quantity qpwv(casaQuantity(pwv));
#ifdef RI_DEBUG
      cout<<qnoise<<" "<<qpress<<" "<<qalt<<" "<<qwaterht<<" "<<qpwv<<endl;
#endif
      rstat=itsSim->setnoise(mode, table, qnoise, 
			     qpress,relhum,qalt,qwaterht,qpwv,
			     tatmos,tau,
			     antefficiency, spillefficiency, correfficiency, 
			     trx, tground, tcmb);
    }    
 } catch  (AipsError x) {
   *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	   << LogIO::POST;
    RETHROW(x);
 }  
 return rstat;
}



bool
simulator::setpa(const std::string& mode, const std::string& table, 
		 const ::casac::variant& interval)
{


  Bool rstat(False);
  try {
    
    if(itsSim !=0){
      
      casa::Quantity qinter(casaQuantity(interval));
      rstat=itsSim->setpa(mode, table, qinter);
    }
    
    
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  
  return rstat;
   

}

bool
simulator::setseed(const int seed)
{

  Bool rstat(False);
  try {
    
    if(itsSim !=0){
      
      rstat=itsSim->setseed(seed);
    }
    
    
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  
  return rstat;
   


}



} // casac namespace

