
/***
 * Framework independent implementation file for calplot...
 *
 * Implement the calplot component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <casa/Exceptions/Error.h>
#include <calplot_cmpt.h>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <tools/synthesis/CalTables/PlotCal.h>

#include <casa/Logging/LogIO.h>

#include <casa/namespace.h>

using namespace std;

namespace casac {

calplot::calplot()
{
   try {
      itsLog = new LogIO();
      itsPlotCal=NULL;
   } catch (AipsError x) {
      *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
   }
}

calplot::~calplot()
{

  this->done();

   try {
      if(itsLog)
         delete itsLog;
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
   }
}

bool
calplot::close()
{
   bool rstat(False);
   try {
     if( itsPlotCal != NULL )
       itsPlotCal->close();
     rstat = True;
   } catch (AipsError x)
   {
      *itsLog <<  LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
   }
   return rstat;
}


bool
calplot::done()
{

   bool rstat(False);
   try {
     if( itsPlotCal != NULL ){delete itsPlotCal; itsPlotCal = NULL;}
     rstat = True;
   } catch (AipsError x)
   {
      *itsLog <<  LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
   }
   return rstat;
}

bool
calplot::next()
{

   bool rstat(False);
   try {
     if(itsPlotCal==NULL){
       *itsLog <<  LogIO::WARN << "No calibration table defined: please use open"
	       << LogIO::POST; 
       return False;
     }
     rstat = itsPlotCal->iterPlotNext();
   } catch (AipsError x)
   {
      *itsLog <<  LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
   }
   return rstat;
}

bool
calplot::open(const std::string& caltable)
{
   bool rstat(False);
   try {

	if ( !caltable.compare("") ){
           *itsLog << LogIO::SEVERE << "No calibration table name has been given" << LogIO::POST;
            return rstat;
        }
	close();
	if(itsPlotCal == NULL)
	  itsPlotCal=new PlotCal();
        rstat = itsPlotCal->open(String(caltable));

   } catch (AipsError x)
   {
      *itsLog <<  LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
   }
   return rstat;
}

bool
calplot::plot(const std::string& xaxis, 
	      const std::string& yaxis)
{

   bool rstat(False);
   try {
     if(itsPlotCal==NULL){
       *itsLog <<  LogIO::WARN << "No calibration table used: please use open"
	       << LogIO::POST; 
       return False;
     }
     rstat = itsPlotCal->plot(String(xaxis),String(yaxis));
   } catch (AipsError x)
   {
      *itsLog <<  LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      //Catching an exception here leaves itsPlotCal in a very funny state
      //  that often causes segv on deletion
      //  better to leak it than crash
      //  (but this sometimes merely postpones the inevitable!) 
      // delete itsPlotCal; 
      //itsPlotCal=NULL;
      RETHROW(x);
   }
   return rstat;
}

bool
calplot::savefig( const std::string& filename,
                const int dpi,
		const std::string& orientation,
		const std::string& papertype,
		const std::string& facecolor,
		const std::string& edgecolor )
{
    bool rstat( casa::False );
    if(itsPlotCal==NULL){
	*itsLog <<  LogIO::WARN << "No calibration table used: please use open"
		<< LogIO::POST; 
	return False;
    }

    try {
	rstat = itsPlotCal->saveFigure( String( filename ), Int( dpi ),
		   String( orientation ), String( papertype ), 
		   String( facecolor ), String( edgecolor ) );
    } catch ( casa::AipsError x ) {
	*itsLog <<  LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
      rstat = casa::False;
    }

    return rstat;
}

bool
calplot::plotoptions(const int subplot,
		     const bool overplot,
		     const std::string& iteration,
		     const std::vector<double>& plotrange,
		     const bool showflags,
		     const std::string& plotsymbol,
		     const std::string& plotcolor,
		     const double markersize,
		     const double fontsize)
{
   bool rstat(False);
   try {
     if(itsPlotCal==NULL){
       *itsLog <<  LogIO::WARN << "No calibration table used: please use open"
	       << LogIO::POST; 
       return False;
     }

     rstat = itsPlotCal->setPlotParam(subplot,overplot,iteration,plotrange,showflags,
				      plotsymbol,plotcolor,markersize,fontsize);
   } catch (AipsError x)
   {
      *itsLog <<  LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
   }
   return rstat;
}

bool calplot::markflags(const int panel, const std::vector<double>& region){
  bool rstat(False);

  try {

    cout << "Does nothing." << endl;

   } catch (AipsError x)
   {
      *itsLog <<  LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
   }
   return rstat;

}

bool calplot::flagdata(){
  bool rstat(False);
  try {
    if(itsPlotCal==NULL){
      *itsLog <<  LogIO::WARN << "No calibration table used: please use open"
	      << LogIO::POST; 
      return False;
    }
    rstat = itsPlotCal->flagData(FLAG);
   } catch (AipsError x)
   {
      *itsLog <<  LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
   }
   return rstat;

}


bool calplot::locatedata(){
  bool rstat(False);
  try {
    if(itsPlotCal==NULL){
      *itsLog <<  LogIO::WARN << "No calibration table used: please use open"
	      << LogIO::POST; 
      return False;
    }
    rstat = itsPlotCal->locateData();
   } catch (AipsError x)
   {
      *itsLog <<  LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
   }
   return rstat;

}

bool 
calplot::selectcal(const ::casac::variant& antenna,
		   const ::casac::variant& field,
		   const ::casac::variant& spw,
		   const ::casac::variant& time,
		   const std::string& poln) 
{

  Bool rstat(False);
  try {
    //    std::cout << antenna.typeString() << ": " << antenna.toString() << std::endl;
    //    std::cout << field.typeString() << ": " << field.toString() << std::endl;
    //    std::cout << spw.typeString() << ": " << spw.toString() << std::endl;

    itsPlotCal->selectCal(getVariantAsString(antenna),
			  getVariantAsString(field),
			  getVariantAsString(spw),
			  getVariantAsString(time),
			  String(poln));
			  
    rstat=True;
    
  } catch (AipsError x) {
    *itsLog <<  LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;

}

bool
calplot::stopiter( const bool rmplotter )
{

   bool rstat(False);
   try {
     if(itsPlotCal==NULL) return False;
     rstat = itsPlotCal->iterPlotStop( rmplotter );
   } catch (AipsError x)
   {
      *itsLog <<  LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
   }
   return rstat;
}

bool
calplot::clearplot(const int subplot)
{

   bool rstat(False);
   try {
     if(itsPlotCal==NULL) return False;
     rstat = itsPlotCal->clearPlot(subplot);
   } catch (AipsError x)
   {
      *itsLog <<  LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
   }
   return rstat;
}

// Private function to convert variants to Strings
casa::String 
calplot::getVariantAsString(const ::casac::variant& var) {

  String out=String(var.toString());

  // [] protection
  if (out.length()>1 && out.firstchar()=='[' && out.lastchar()==']')
    out = out.substr(1,out.length()-2);

  return out;
}


} // casac namespace

