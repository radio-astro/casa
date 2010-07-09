
/***
 * Framework independent implementation file for autoflag...
 *
 * Implement the autoflag component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <autoflag_cmpt.h>
#include <casa/Logging/LogIO.h>
#include <casa/Exceptions/Error.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <flagging/Flagging/RedFlagger.h>
#include <flagging/Flagging/RFCommon.h>
#include <casa/Containers/RecordInterface.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
#include <casa/sstream.h>
#include <unistd.h>
#include <xmlcasa/StdCasa/CasacSupport.h>


using namespace std;
using namespace casa;

namespace casac {

autoflag::autoflag()
{
  itsMS = 0;
  itsSelect = 0;
  itsLog = new casa::LogIO;
  itsFlagger = new RedFlagger();
  agents = 0;
  opts = 0;
  agentCount = 0;
  need2AddSelect = False;
}

autoflag::~autoflag()
{
	// Need to redo so everything can get deleted.
  delete itsLog;
  if(itsFlagger)
     delete itsFlagger;
  if(agents)
     delete agents;
  if(opts)
     delete opts;
  if(itsMS)
     delete itsMS;
}

bool
autoflag::initSelectRecord(){
 Bool rstat(False);
 try {
   if(!itsSelect){
       itsSelect = new Record();
       itsSelect->define("id", String("select"));
       need2AddSelect = true;
       rstat = True;
    }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported initSelecRecord: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::settimemed(const double thr, const int hw, const double rowthr, const int rowhw, const bool norow, const std::string& column, const std::string& expr, const bool fignore)
{
 Bool rstat(False);
 try {
    Bool debug(False);
    if(!itsFlagger)
	           throw(AipsError("autoflag::setTimeMed - No Measurement Set specified for flagging"));
        RecordDesc flagDesc;
	flagDesc.addField("id", TpString);
	flagDesc.addField("thr", TpFloat);
	flagDesc.addField("hw", TpInt);
	flagDesc.addField("rowthr", TpFloat);
	flagDesc.addField("rowhw", TpInt);
	flagDesc.addField("norow", TpBool);
	flagDesc.addField("column", TpString);
	flagDesc.addField("expr", TpString);
	flagDesc.addField("debug", TpBool);
	flagDesc.addField("fignore", TpBool);
	//
	Record *flagRec = new Record(flagDesc);
	flagRec->define("id", String("timemed"));
	flagRec->define("thr", Float(thr));
	flagRec->define("hw", hw);
	flagRec->define("rowthr", Float(rowthr));
	flagRec->define("rowhw", rowhw);
	flagRec->define("norow", norow);
	flagRec->define("column", column);
	flagRec->define("expr", expr);
	flagRec->define("debug", debug);
	flagRec->define("fignore", fignore);
	//
	addAgent(*flagRec);
	rstat = True;
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported settimemed: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::addAgent(casa::RecordInterface &newAgent)
{

 Bool rstat(False);
 try {
      if(!agents){
          agentCount = 0;
          agents= new Record;
          *itsLog << LogIO::WARN << "Creating agents record " << LogIO::POST;
      }
      ostringstream fieldName;
      fieldName << agentCount++;
      agents->defineRecord(*(new String(fieldName)), newAgent);
      rstat = True;
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported addAgent: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::setnewtimemed(const double thr, const std::string& column, const std::string& expr, const bool fignore)
{

 Bool rstat(False);
 try {
    if(!itsFlagger)
	           throw(AipsError("autoflag::setTimeMed - No Measurement Set specified for flagging"));
    RecordDesc flagDesc;
    flagDesc.addField("id", TpString);
    flagDesc.addField("thr", TpDouble);
    flagDesc.addField("column", TpString);
    flagDesc.addField("expr", TpString);
    flagDesc.addField("fignore", TpBool);
    //
    Record *flagRec = new Record(flagDesc);
    flagRec->define("id", String("newtimemed"));
    flagRec->define("thr", thr);
    flagRec->define("column", column);
    flagRec->define("expr", expr);
    flagRec->define("fignore", fignore);
    //
    addAgent(*flagRec);
    rstat = True;
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported setnewtimemed: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::setdata(const std::string& mode, const std::vector<int>& nchan, const std::vector<int>& start, const std::vector<int>& step, const Quantity& mstart, const Quantity& mstep, const std::vector<int>& spwid, const std::vector<int>& fieldid, const std::string& msselect, const bool async)
{

 Bool rstat(False);
 try {
       if(!itsFlagger)
              throw(AipsError("acsautoflag::setSelect - No Measurement Set specified for flagging"));
//  
       Vector<Int> tfieldid(0);
       Vector<Int> tspwid(0);
       if(!(spwid.size() == 1 && spwid[0] == -1))
          tspwid  = spwid;
       if(!(fieldid.size() == 1 && fieldid[0] == -1))
          tfieldid  = fieldid;
       casa::Quantity q1 = casaQuantity(mstart);
       casa::Quantity q2 = casaQuantity(mstep);
       itsFlagger->setdata(String(mode),
                        Vector<Int>(nchan),
                        Vector<Int>(start),
                        Vector<Int>(step),
                        MRadialVelocity(q1), MRadialVelocity(q2),
                        tspwid,
                        tfieldid,
                        String(msselect));
       rstat = True;
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported setdata: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::setfreqmed(const double thr, const int hw, const double rowthr, const int rowhw, const std::string& column, const std::string& expr, const bool fignore)
{

 Bool rstat(False);
 try {
    Bool debug(False);
    if(!itsFlagger)
       throw(AipsError("acsautoflag::setFreqMed - No Measurement Set specified for flagging"));
    RecordDesc flagDesc;
    flagDesc.addField("id", TpString);
    flagDesc.addField("thr", TpFloat);
    flagDesc.addField("hw", TpInt);
    flagDesc.addField("rowthr", TpFloat);
    flagDesc.addField("rowhw", TpInt);
    //flagDesc.addField("norow", TpBool);
    flagDesc.addField("column", TpString);
    flagDesc.addField("expr", TpString);
    flagDesc.addField("debug", TpBool);
    flagDesc.addField("fignore", TpBool);
    //
    Record flagRec(flagDesc);
    flagRec.define("id", "freqMed");
    flagRec.define("thr", thr);
    flagRec.define("hw", hw);
    flagRec.define("rowthr", rowthr);
    flagRec.define("rowhw", rowhw);
    //flagRec.define("norow", norow);
    flagRec.define("column", column);
    flagRec.define("expr", expr);
    flagRec.define("debug", debug);
    flagRec.define("fignore", fignore);
    //
    addAgent(flagRec);
    rstat = True;
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported setfreqmed: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::setsprej(const int ndeg, const double rowthr, const int rowhw, const bool norow, const int spwid, const std::vector<int>& chan, const ::casac::record& region, const std::string& column, const std::string& expr, const bool fignore)
{

	/*
	 * Note to self, region record must be defined appropriatel; 
	 * Maybe Record specRegion = toRecord(region);
	 */
 Bool rstat(False);
 try {
    if(!itsFlagger)
	      throw(AipsError("acsautoflag::setSPrej - No Measurement Set specified for flagging"));
      try {
	      /*
           RecordDesc regionDesc;
           regionDesc.addField(RF_SPWID, TpInt);
           regionDesc.addField(RF_FREQS, TpArrayDouble);
           regionDesc.addField(RF_CHANS, TpArrayInt);
           // Not quite done here need to handle an array of SpectralRegions
           Record allRegions;
           for (int i=0; i<spec->count; i++){
                Record regionRec(regionDesc);
                regionRec.define(RF_SPWID, spec->specWindowID);
           
                if(spec->useFreq){
                     regionRec.define(RF_FREQS, Array<double>(IPosition(2,spec->count), spec->Freq));
                }else{
                     regionRec.define(RF_CHANS, Array<int>(IPosition(2, spec->count), spec->Chan));
                }
                allRegions.defineRecord(String(i), regionRec);
           }
	   */
	   Bool debug(False);
	   Record *allRegions = toRecord(region); // Should comple OK now.
           RecordDesc flagDesc;
           flagDesc.addField("id", TpString);
           flagDesc.addField(RF_REGION, TpRecord);
           flagDesc.addField(RF_NDEG, TpInt);
           flagDesc.addField("rowthr", TpFloat);
           flagDesc.addField("rowhw", TpInt);
           flagDesc.addField("column", TpString);
           flagDesc.addField("expr", TpString);
           flagDesc.addField("debug", TpBool);
           flagDesc.addField("fignore", TpBool);
           //
           Record flagRec(flagDesc);
           flagRec.define("id","sprej");
           flagRec.defineRecord(RF_REGION, *allRegions);
           flagRec.define(RF_NDEG, ndeg);
           flagRec.define("rowthr", rowthr);
           flagRec.define("rowhw", rowhw);
           flagRec.define("column", column);
           flagRec.define("expr", expr);
           flagRec.define("debug", debug);
           flagRec.define("fignore", fignore);
           //
           addAgent(flagRec);
	   delete allRegions;
           //
	   rstat = True;
      } catch (AipsError x) {
           *itsLog << LogIO::SEVERE  << "ExceptionReportde: " << x.getMesg() << LogIO::POST;
	   RETHROW(x);
      }

 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported setsprej: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::setuvbin(const double thr, const int minpop, const std::vector<int>& nbins, const int plotchan, const bool econoplot, const std::string& column, const std::string& expr, const bool fignore)
{

 Bool rstat(False);
 try {
	 Bool debug(False);
       if(!itsFlagger)
              throw(AipsError("acsautoflag::setUVBin - No Measurement Set specified for flagging"));
       RecordDesc flagDesc;
       flagDesc.addField("id", TpString);
       flagDesc.addField("thr", TpFloat);
       flagDesc.addField("minpop", TpInt);
       flagDesc.addField("nbins", TpArrayInt);
       flagDesc.addField("plotchan", TpBool);
       flagDesc.addField("econoplot", TpBool);
       flagDesc.addField("column", TpString);
       flagDesc.addField("expr", TpString);
       flagDesc.addField("debug", TpBool);
       flagDesc.addField("fignore", TpBool);
//
       Record flagRec(flagDesc);
       flagRec.define("id", "uvbin");
       flagRec.define("thr", thr);
       flagRec.define("minpop", minpop);
       Vector<Int> dnbins(2);
       dnbins(0) = nbins[0];
       dnbins(1) = nbins[1];
       flagRec.define("nbins", dnbins);
       flagRec.define("plotchan", plotchan);
       flagRec.define("econoplot", econoplot);
       flagRec.define("column", column);
       flagRec.define("expr", expr);
       flagRec.define("debug", debug);
       flagRec.define("fignore", fignore);
       //
       addAgent(flagRec);
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported setuvbin: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::open(const std::string& msname)
{

 Bool rstat(False);
 try {
    if(!itsFlagger){
      itsFlagger = new RedFlagger();
     } else {
       if(itsMS){
	    delete itsMS;
	    itsFlagger->detach();
	    delete agents;
	    delete opts;
	    delete itsSelect;
	    itsSelect = 0;
       }
     }
     agents = new Record();
     agentCount =  0;
     opts = new Record();
     itsMS = new MeasurementSet(msname, Table::Update);
     itsFlagger->attach(*itsMS);
     need2AddSelect = false;
     rstat = True;
    
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported open: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::run(const ::casac::record& globparm, const int plotscr, const int plotdev, const std::string& devfile, const bool reset, const bool trial)
{

 Bool rstat(False);
 try {
      if(!itsFlagger)
            throw(AipsError("acsautoflag::run - No Measurement Set specified for flagging"));
      if(itsSelect){
            addAgent(*itsSelect);
      }
      itsFlagger->run(*agents, *opts, 0);
      rstat = True;
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported autoflag.run: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::summary()
{

 Bool rstat(False);
 try {
      if(!itsFlagger)
            throw(AipsError("acsautoflag::summary - No Measurement Set specified for flagging"));
      if(itsSelect){
            addAgent(*itsSelect);
            need2AddSelect = false;
      }
      itsFlagger->summary(*agents, *opts, 0);
      rstat = True;
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported autoflag.summary: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::help(const std::string& names)
{

 Bool rstat(False);
 try {
    // TODO : IMPLEMENT ME HERE !
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::reset(const std::vector<std::string>& methods)
{
    Bool rstat(False);
    if (agents != NULL) {
        try {
            /* for all the methods in in methods, remove them from the agents record */
            for(unsigned int i=0; i<methods.size();i++){
                if(agents->isDefined(methods[i])){
                    agents->removeField(RecordFieldId(String(methods[i])));
                    *itsLog << LogIO::NORMAL3 << "Field " << methods[i] << " removed from agents record." << LogIO::POST;
                } else {
                    *itsLog << LogIO::WARN << "Field " << methods[i] << " not found in agents record, not reset." << LogIO::POST;
                }
                if(itsSelect && itsSelect->isDefined(methods[i])){
                    itsSelect->removeField(RecordFieldId(String(methods[i])));
                    *itsLog << LogIO::NORMAL3 << "Field " << methods[i] << " removed from agents record." << LogIO::POST;
                } else {
                    *itsLog << LogIO::WARN << "Field " << methods[i] << " not found in agents record, not reset." << LogIO::POST;
                }
            }
        } catch (AipsError x) {
            *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
            RETHROW(x);
        }
    }
    return rstat;
}

bool
autoflag::resetall()
{

 Bool rstat(False);
 try {
    delete agents;
    delete itsSelect;
    itsSelect = 0;
    agents = new Record();
    *itsLog << LogIO::NORMAL3 << "Agents record cleared, reset all done." << LogIO::POST;
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::close()
{

 Bool rstat(False);
 try {
       if(!itsFlagger)
              throw(AipsError("acsautoflag::setSelect - No Measurement Set specified for flagging"));
        itsFlagger->detach();
	//delete itsFlagger;
        delete itsMS;
        delete  agents;
        delete  opts;
	delete itsSelect;
        itsMS = 0;
	agents = 0;
	opts = 0;
	itsSelect = 0;
	// itsFlagger = 0;
        rstat = True;
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::done()
{

 Bool rstat(False);
 try {
    return close();
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::setspwid(const std::vector<int>& spwid)
{
 Bool rstat(False);
 try {
      if(itsFlagger){
         initSelectRecord();
         RecordDesc flagDesc;
         flagDesc.addField(RF_SPWID, TpArrayInt);
         Record flagRec(flagDesc);
         flagRec.define(RF_SPWID, Vector<Int>(spwid));
         itsSelect->mergeField(flagRec, RF_SPWID, RecordInterface::OverwriteDuplicates);
         rstat = True;
      } else {
            *itsLog << LogIO::SEVERE << "autoflag::setspwid - No Measurement Set specified for flagging"
		    << LogIO::POST;
      }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported setspwid: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::setfield(const ::casac::variant& field)
{
 Bool rstat(False);
 try {
    // TODO : IMPLEMENT ME HERE !
      if(itsFlagger){
         initSelectRecord();
         RecordDesc flagDesc;
         rstat = True;
	 switch(field.type())
	 {
		 case variant::INT :
			 { flagDesc.addField(RF_FIELD, TpInt);
                         Record flagRec(flagDesc);
                         flagRec.define(RF_FIELD, field.getInt());
                         itsSelect->mergeField(flagRec, RF_FIELD, RecordInterface::OverwriteDuplicates); }
		         rstat = True;
			 break;
		 case variant::INTVEC :
			 { flagDesc.addField(RF_FIELD, TpArrayInt);
                         Record flagRec(flagDesc);
                         flagRec.define(RF_FIELD, Vector<Int>(field.getIntVec()));
                         itsSelect->mergeField(flagRec, RF_FIELD, RecordInterface::OverwriteDuplicates); }
		         rstat = True;
			 break;
		 case variant::STRING :
			 { flagDesc.addField(RF_FIELD, TpString);
                         Record flagRec(flagDesc);
                         flagRec.define(RF_FIELD, field.getString());
                         itsSelect->mergeField(flagRec, RF_FIELD, RecordInterface::OverwriteDuplicates); }
		         rstat = True;
			 break;
		 case variant::STRINGVEC :
			 { flagDesc.addField(RF_FIELD, TpArrayString);
                         Record flagRec(flagDesc);
                         flagRec.define(RF_FIELD, toVectorString(field.getStringVec()));
                         itsSelect->mergeField(flagRec, RF_FIELD, RecordInterface::OverwriteDuplicates); }
		         rstat = True;
			 break;
		 default :
		    rstat = False;
		    break;
	 }
      } else {
            *itsLog << LogIO::SEVERE << "autoflag::field - No Measurement Set specified for flagging"
		    << LogIO::POST;
      }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported field: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::setfreqs(const Quantity& freqs)
{
 Bool rstat(False);
 try {
	 // TODO
      if(itsFlagger){
         initSelectRecord();
         RecordDesc flagDesc;
         flagDesc.addField(RF_FREQS, TpQuantity);
         Record flagRec(flagDesc);
         // flagRec.define(RF_FREQS, casaQuantity(freqs));
         itsSelect->mergeField(flagRec, RF_FREQS, RecordInterface::OverwriteDuplicates);
         rstat = True;
      } else {
            *itsLog << LogIO::SEVERE << "autoflag::freqs - No Measurement Set specified for flagging"
		    << LogIO::POST;
      }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported freqs: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::setchans(const std::vector<int>& chan){
 Bool rstat(False);
 try {
      if(itsFlagger){
         initSelectRecord();
         RecordDesc flagDesc;
         flagDesc.addField(RF_CHANS, TpArrayInt);
         Record flagRec(flagDesc);
         flagRec.define(RF_CHANS, Vector<Int>(chan));
         itsSelect->mergeField(flagRec, RF_CHANS, RecordInterface::OverwriteDuplicates);
         rstat = True;
      } else {
            *itsLog << LogIO::SEVERE << "autoflag::setchans - No Measurement Set specified for flagging"
		    << LogIO::POST;
      }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported setchans: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}   

bool
autoflag::setcorr(const std::vector<std::string>& corr)
{
 Bool rstat(False);
 try {
      if(itsFlagger){
         initSelectRecord();
         RecordDesc flagDesc;
         flagDesc.addField(RF_CORR, TpArrayString);
         Record flagRec(flagDesc);
         flagRec.define(RF_CORR, toVectorString(corr));
         itsSelect->mergeField(flagRec, RF_CORR, RecordInterface::OverwriteDuplicates);
         rstat = True;
      } else {
            *itsLog << LogIO::SEVERE << "autoflag::setcorr - No Measurement Set specified for flagging"
		    << LogIO::POST;
      }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported setcorr: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::setantenna(const std::vector<std::string>& ant)
{
 Bool rstat(False);
 try {
      if(itsFlagger){
         initSelectRecord();
         RecordDesc flagDesc;
         flagDesc.addField(RF_ANT, TpArrayString);
         Record flagRec(flagDesc);
         flagRec.define(RF_ANT, toVectorString(ant));
         itsSelect->mergeField(flagRec, RF_ANT, RecordInterface::OverwriteDuplicates);
         rstat = True;
      } else {
            *itsLog << LogIO::SEVERE << "autoflag::setantenna - No Measurement Set specified for flagging"
		    << LogIO::POST;
      }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported setantenna: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::setantennaID(const std::vector<int>& antID)
{
 Bool rstat(False);
 try {
      if(itsFlagger){
         initSelectRecord();
         RecordDesc flagDesc;
         flagDesc.addField(RF_ANT, TpArrayInt);
         Record flagRec(flagDesc);
         flagRec.define(RF_ANT, Vector<Int>(antID));
         itsSelect->mergeField(flagRec, RF_ANT, RecordInterface::OverwriteDuplicates);
         rstat = True;
      } else {
            *itsLog << LogIO::SEVERE << "autoflag::setantennaID - No Measurement Set specified for flagging"
		    << LogIO::POST;
      }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported setantennaID: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}
bool
autoflag::setbaseline(const std::vector<int>& baseline)
{
 Bool rstat(False);
 try {
      if(itsFlagger){
         initSelectRecord();
         RecordDesc flagDesc;
         flagDesc.addField(RF_BASELINE, TpArrayInt);
         Record flagRec(flagDesc);
         flagRec.define(RF_BASELINE, Vector<Int>(baseline));
         itsSelect->mergeField(flagRec, RF_BASELINE, RecordInterface::OverwriteDuplicates);
         rstat = True;
      } else {
            *itsLog << LogIO::SEVERE << "autoflag::setbaseline - No Measurement Set specified for flagging"
		    << LogIO::POST;
      }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported setbaseline: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::settimerange(const std::vector<std::string>& timerange)
{
 Bool rstat(False);
 try {
      if(itsFlagger){
         initSelectRecord();
         RecordDesc flagDesc;
         flagDesc.addField(RF_TIMERANGE, TpArrayString);
         Record flagRec(flagDesc);
         flagRec.define(RF_TIMERANGE, toVectorString(timerange));
         itsSelect->mergeField(flagRec, RF_TIMERANGE, RecordInterface::OverwriteDuplicates);
         rstat = True;
      } else {
            *itsLog << LogIO::SEVERE << "autoflag::timerange - No Measurement Set specified for flagging"
		    << LogIO::POST;
      }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported timerange: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::setautocorrelation(const bool autocorr)
{
 Bool rstat(False);
 try {
      if(itsFlagger){
         initSelectRecord();
         RecordDesc flagDesc;
         flagDesc.addField(RF_AUTOCORR, TpBool);
         Record flagRec(flagDesc);
         flagRec.define(RF_AUTOCORR, autocorr);
         itsSelect->mergeField(flagRec, RF_AUTOCORR, RecordInterface::OverwriteDuplicates);
         // RecordFieldId dum(RF_AUTOCORR);
         //itsSelect->define(dum, ac);
         rstat = True;
      } else {
            *itsLog << LogIO::SEVERE << "autoflag::autocorrelation - No Measurement Set specified for flagging"
		    << LogIO::POST;
      }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported autocorrelation: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::settimeslot(const std::vector<std::string>& timeslot)
{
 Bool rstat(False);
 try {
      if(itsFlagger){
         initSelectRecord();
         RecordDesc flagDesc;
         flagDesc.addField(RF_CENTERTIME, TpArrayString);
         Record flagRec(flagDesc);
         flagRec.define(RF_CENTERTIME, toVectorString(timeslot));
         itsSelect->mergeField(flagRec, RF_CENTERTIME, RecordInterface::OverwriteDuplicates);
         rstat = True;
      } else {
            *itsLog << LogIO::SEVERE << "autoflag::settimeslot - No Measurement Set specified for flagging"
		    << LogIO::POST;
      }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported settimeslot: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::setdtime(const double dtime)
{
 Bool rstat(False);
 try {
      if(itsFlagger){
         initSelectRecord();
         RecordDesc flagDesc;
         flagDesc.addField(RF_TIMEDELTA, TpDouble);
         Record flagRec(flagDesc);
         flagRec.define(RF_TIMEDELTA, dtime);
         itsSelect->mergeField(flagRec, RF_TIMEDELTA, RecordInterface::OverwriteDuplicates);
         rstat = True;
      } else {
            *itsLog << LogIO::SEVERE << "autoflag::setdtime - No Measurement Set specified for flagging"
		    << LogIO::POST;
      }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported setdtime: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::setclip(const ::casac::record& clip)
{
 Bool rstat(False);
 try {
    //  Could need to be a vector of records
    //  Should check that the clip record is valid
	 //TODO
      if(itsFlagger){
         initSelectRecord();
	 /*
         RecordDesc clipDesc;
         clipDesc.addField(RF_EXPR, TpString);
         clipDesc.addField(RF_MIN, TpDouble);
         clipDesc.addField(RF_MAX, TpDouble);
	 */

	 Record *clipRec = toRecord(clip);

         Record flagRec;
         flagRec.defineRecord("clip", *clipRec);
         itsSelect->defineRecord(RF_CLIP, flagRec);
         rstat = True;
      } else {
            *itsLog << LogIO::SEVERE << "autoflag::autocorrelation - No Measurement Set specified for flagging"
		    << LogIO::POST;
      }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported autocorrelation: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::setflagrange(const ::casac::record& flagrange)
{
 Bool rstat(False);
 try {
    // Here again may need to be a vector of records
	 //TODO
      if(itsFlagger){
         initSelectRecord();
         RecordDesc clipDesc;
         clipDesc.addField(RF_EXPR, TpString);
         clipDesc.addField(RF_MIN, TpDouble);
         clipDesc.addField(RF_MAX, TpDouble);

         Record clipRec(clipDesc);
         //clipRec.define(RF_EXPR, clip.find("expr").toString());
	 //clipRec.define(RF_MIN, clip.find("minVal").toDouble());
	 //clipRec.define(RF_MAX, clip.find("maxVal").toDouble());

         Record flagRec;
         flagRec.defineRecord("flagrange", clipRec);
         itsSelect->defineRecord(RF_FLAGRANGE, flagRec);
         rstat = True;
      } else {
            *itsLog << LogIO::SEVERE << "autoflag::flagrange - No Measurement Set specified for flagging"
		    << LogIO::POST;
      }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported flagrange: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}
bool
autoflag::setquack(const std::vector<double>& quack)
{
 Bool rstat(False);
 try {
      if(itsFlagger){
         initSelectRecord();
         RecordDesc flagDesc;
         flagDesc.addField(RF_QUACK, TpArrayDouble);
         Record flagRec(flagDesc);
         flagRec.define(RF_QUACK, Vector<Double>(quack));
         itsSelect->mergeField(flagRec, RF_QUACK, RecordInterface::OverwriteDuplicates);
         rstat = True;
      } else {
            *itsLog << LogIO::SEVERE << "autoflag::setquack - No Measurement Set specified for flagging"
		    << LogIO::POST;
      }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported setquack: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::setunflag(const bool unflag)
{
 Bool rstat(False);
 try {
      if(itsFlagger){
         initSelectRecord();
         RecordDesc flagDesc;
         flagDesc.addField(RF_UNFLAG, TpBool);
         Record flagRec(flagDesc);
         flagRec.define(RF_UNFLAG, unflag);
         itsSelect->mergeField(flagRec, RF_UNFLAG, RecordInterface::OverwriteDuplicates);
         rstat = True;
      } else {
            *itsLog << LogIO::SEVERE << "autoflag::setunflag - No Measurement Set specified for flagging"
		    << LogIO::POST;
      }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported setunflag: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
autoflag::setnewselector()
{
 Bool rstat(False);
 try {
      if(itsFlagger){
	 if(itsSelect){
		 addAgent(*itsSelect);
		 delete itsSelect;
		 itsSelect = 0;
		 initSelectRecord();
	 }
         rstat = True;
      } else {
            *itsLog << LogIO::SEVERE << "autoflag::newselector - No Measurement Set specified for flagging"
		    << LogIO::POST;
      }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported setnewselector: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}


} // casac namespace

