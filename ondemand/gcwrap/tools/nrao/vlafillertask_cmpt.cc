
/***
 * Framework independent implementation file for vlafillertask...
 *
 * Implement the vlafillertask component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <vlafillertask_cmpt.h>

#include <nrao/VLA/VLAFiller.h>
#include <nrao/VLA/VLAFilterSet.h>
#include <nrao/VLA/VLAObsModeFilter.h>
#include <nrao/VLA/VLAProjectFilter.h>
#include <nrao/VLA/VLAFrequencyFilter.h>
#include <nrao/VLA/VLACalibratorFilter.h>
#include <nrao/VLA/VLATimeFilter.h>
#include <nrao/VLA/VLASourceFilter.h>
#include <nrao/VLA/VLASubarrayFilter.h>
#include <nrao/VLA/VLADiskInput.h>
#include <nrao/VLA/VLAArchiveInput.h>
#include <nrao/VLA/VLATapeInput.h>
#include <nrao/VLA/VLAOnlineInput.h>
#include <casa/Containers/Block.h>
#include <casa/Exceptions/Error.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MFrequency.h>
#include <casa/OS/File.h>
#include <casa/OS/Path.h>
#include <casa/OS/SymLink.h>
#include <casa/Quanta/MVEpoch.h>
#include <casa/Quanta/MVFrequency.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>
#include <stdcasa/StdCasa/CasacSupport.h>


using namespace std;

namespace casac {

vlafillertask::vlafillertask()
{

}

vlafillertask::~vlafillertask()
{

}

void
vlafillertask::fill(const std::string& msname, 
		    const std::string& inputfile, 
		    const std::string& project, 
		    const std::string& start, 
		    const std::string& stop, 
		    const std::string& centerfreq, 
		    const std::string& bandwidth, 
		    const std::string& bandname, 
		    const std::string& source, 
		    const int subarray, 
		    const int qualifier, 
		    const std::string &calcode, 
		    const bool overwrite, 
		    const ::casac::variant& freqtol,
		    const bool applytsys,
		    const bool keepautocorr, 
		    const std::string& antnamescheme, 
		    const int useday,
		    const bool keepblanks,
		    const bool evlabands)
{
   VLALogicalRecord *in(0);
   Int logProgress = 0;
   MeasurementSet *out = new MeasurementSet;
   *out = VLAFiller::getMS(String(msname), overwrite);
   char defcalcode = calcode[0];

   try
   {
      std::string centerNu(centerfreq);
      std::string bandwidthNu(bandwidth);
      VLAFilterSet filters;

      // Fill default (useful) modes for now
      //  TBD: provide user control for this
      filters.addFilter(VLAObsModeFilter());

      filters.addFilter(VLASourceFilter(source, qualifier, keepblanks));
      if(!(project == "all"))
      {
         filters.addFilter(VLAProjectFilter(project));
      }
      if(bandname == "*"){
	      centerNu="";
	      bandwidthNu="";
      }
      if(!bandname.empty()){
	 if(bandname == "4" || bandname == "4m"){
              centerNu="75MHz";
	      bandwidthNu="48MHz";
	 }else if(bandname == "P" || bandname == "p" || bandname == "90cm"){
              centerNu="321.5MHz";
	      bandwidthNu="47MHz";
	 }else if(bandname == "L" || bandname == "l" || bandname == "20cm"){
	      if(!evlabands){
                 centerNu="1450MHz";
	         bandwidthNu="600MHz";
	      } else {
                 centerNu="1.5GHz";
	         bandwidthNu="1GHz";
	      }
	 }else if(bandname == "S" || bandname == "s" || bandname == "10cm"){
              centerNu="3GHz";
	      bandwidthNu="2GHz";
	 }else if(bandname == "C" || bandname == "c" || bandname == "6cm"){
	      if(!evlabands){
                 centerNu="4.65GHz";
	         bandwidthNu="900MHz";
	      } else {
                 centerNu="6GHz";
	         bandwidthNu="4GHz";
	      }
	 }else if(bandname == "X" || bandname == "x" || bandname == "3.6cm"){
	      if(!evlabands){
                 centerNu="8.2GHz";
	         bandwidthNu="2.8GHz";
	      } else {
                 centerNu="10GHz";
	         bandwidthNu="4GHz";
	      }
	 }else if(bandname == "U" || bandname == "u" || bandname == "2cm"){
	      if(!evlabands){
                 centerNu="14.9GHz";
	         bandwidthNu="2.8GHz";
	      } else {
                 centerNu="15GHz";
	         bandwidthNu="6GHz";
	      }
	 }else if(bandname == "K" || bandname == "k" || bandname == "1.2cm"){
	      if(!evlabands){
                 centerNu="23.3GHz";
	         bandwidthNu="5GHz";
	      } else {
                 centerNu="22.26GHz";
	         bandwidthNu="8GHz";
	      }
	 }else if(bandname == "Ka" || bandname == "ka" || bandname == "9mm"){
              centerNu="33.25GHz";
	      bandwidthNu="13.5GHz";
	 }else if(bandname == "Q" || bandname == "q" || bandname == "7mm"){
	      if(!evlabands){
                 centerNu="44.5GHz";
	         bandwidthNu="13GHz";
	      } else {
                 centerNu="45GHz";
	         bandwidthNu="10GHz";
	      }
	 }else{
	 }
      } else {
             centerNu = centerfreq;
	     bandwidthNu  = bandwidth;
      }

      if(!centerNu.empty())
      {
         istringstream iss(centerNu);
	 casa::Quantity freq;
         iss >> freq;
         istringstream issbw(bandwidthNu);
	 casa::Quantity qbw;
         issbw >> qbw;
         MVFrequency rf(freq);
         MVFrequency bw(qbw);
         filters.addFilter(VLAFrequencyFilter(rf,bw));
      }

      
      defcalcode = '#';
      filters.addFilter(VLACalibratorFilter(defcalcode));

      {
         VLATimeFilter tf;
         Quantum<Double> t;
         Bool timeFiltering = False;
	 string tstart(start);
	 if(start == "")
		 tstart="1970/1/31/00:00:00";

         if(MVTime::read(t, tstart))
         {
            tf.startTime(MVEpoch(t));
            timeFiltering = True;
         }
	 string tstop(stop);
	 if(stop == "")
		 tstop="2199/1/31/23:59:59";
         if(MVTime::read(t, tstop))
         {
            tf.stopTime(MVEpoch(t));
            timeFiltering = True;
         }
         if(timeFiltering){
            filters.addFilter(tf);
	 }
      }
      

      if(inputfile != "online"){
         const Path fileName (inputfile);
         // AlwaysAssert(fileName.isValid(), AipsError);
         File file(fileName);
         // AlwaysAssert(file.exists(), AipsError);
         if(file.isSymLink())
         {
            SymLink link(file);
            Path realFileName = link.followSymLink();
            AlwaysAssert(realFileName.isValid(), AipsError);
            file = File(realFileName);
            AlwaysAssert(file.exists(), AipsError);
            DebugAssert(file.isSymLink() == False, AipsError);
         }
         if(file.isRegular())
         {
            in = new VLALogicalRecord(new VLADiskInput(fileName));
            // Not dealing with tape!
            //
         }
         else if(file.isCharacterSpecial())
         {
            // in = VLALogicalRecord(new VLATapeInput(inputName, files));
         } else {
             throw(AipsError(String("vlatoms - cannot read from: ") +
             fileName.expandedName()));
         }
      } else {
           String online(inputfile);
           online.downcase();
           in = new VLALogicalRecord(new VLAOnlineInput(online, useday));
      }
      Double freqtol2(0.0);
      switch(freqtol.type()){
          case variant::STRING :     
		  {
	      istringstream iss;
	      casa::Quantity qfreqtol;
	      iss >> qfreqtol;
	      freqtol2 = qfreqtol.getValue();
		  }
	      break;
	  case variant::DOUBLE :
	  case variant::INT :
	      freqtol2 = freqtol.toDouble();
	      break;
	  default :
	      break;
      }
      VLAFiller *vla = new VLAFiller(*out, *in, freqtol2, keepautocorr, antnamescheme,applytsys);
      vla->setFilter(filters);
      if(inputfile == "online"){
	      String t1(project);
	      String t2(stop);
         vla->setStopParams(t1, t2);
      }
      vla->fill(logProgress);
      delete vla;
      delete out;
      delete in;
      vla = 0;
   }
   catch (AipsError x)
   {
      //cerr << x.getMesg() << endl;
      RETHROW(x);
      return;
   }

   return;
}

} // casac namespace

