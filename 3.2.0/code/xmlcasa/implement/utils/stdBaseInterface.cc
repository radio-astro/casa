#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <xmlcasa/utils/stdBaseInterface.h>
#include <xmlcasa/record.h>
#include <iostream>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/NullLogSink.h>
#include <sys/stat.h>

#include <casa/BasicSL/String.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/Unit.h>


//extern "C" char **rl_completion_matches __P((const char *, rl_compentry_func_t *));

using namespace std;

namespace casac {

extern "C" char **casactl_completion __P((const char *, int , int));
extern "C" char *command_generator __P((const char *, int));

// These are the bits to make readline work...


/* Generator function for command completion.  STATE lets us
 *    know whether to start from scratch; without any state
 *       (i.e. STATE == 0), then we start at the top of the list. */

	/*
extern "C" char *command_generator (const char *text, int state)
{
   static unsigned list_index, len;
   char *rstat = NULL;
   const char *name;
   bool amatch(false);
	      // If this is a new word to complete, initialize now.  This
	      //      includes saving the length of TEXT for efficiency, and
	      //           initializing the index variable to 0.
   if (!state)
   {
      list_index = 0;
      len = strlen (text);
   }

// Return the next name which partially matches from the
//     command list.

   vector<string> commands = stdBaseInterface::getcommands();
   while (!amatch && list_index < commands.size()) {
      name = commands[list_index].c_str();
      list_index++;
      if (strncmp (name, text, len) == 0){
         rstat = strdup(name);
	 amatch = true;
      }
   }
		  // If no names matched, then return NULL.
   return rstat;
}
*/

/* Attempt to complete on the contents of TEXT.  START and END
 * bound the region of rl_line_buffer that contains the word to
 * complete.  TEXT is the word to complete.  We can use the entire
 * contents of rl_line_buffer in case we want to do some simple
 * parsing.  Returnthe array of matches, or NULL if there aren't any. */

/*
extern "C" char **casactl_completion (const char *text, int start, int end)
{
   char **matches;
   matches = (char **)NULL;
	      // If this word is at the start of the line, then it is a command
	      //      to complete.  Otherwise it is the name of a file in the current
	      //           directory.
   if (start == 0)
      matches = rl_completion_matches(text, command_generator);
   return (matches);
}
*/
//
// Need to sync up the commands list with Joe's python inputs
//
vector<string> stdBaseInterface::commands(0);

stdBaseInterface::stdBaseInterface(record &descParams)
{
	init();
//#include <xmlcasa/utils/standalones.h>
	//
	// Now loop over the the descParams.params record and
	// add those to the commands list
	//
}
stdBaseInterface::~stdBaseInterface(){}

int stdBaseInterface::inputs(record &params)
{
   int rstat(0);
   while(!done){
      char *line = readline("casactl: ");
      if(line){
	 if(!strcmp(line, "quit") || !strcmp(line, "exit")){ 
		 done=true;
		 cerr << "Hey quit" << endl;
		 rstat = -1;
	 } else if(!strcmp(line, "go") || !strcmp(line, "run")){
		 done = true;
		 cerr << "Hey go" << endl;
		 rstat = 1;
	 } else {
		 parse_it(params, line);
		 add_history(line);
	 }
         delete  line;
      }      
   }
   return rstat;
}

void stdBaseInterface::parse_it(record &params, const char *line){

   if(!strcmp(line, "help")){
      done = false;
      cerr << "Hey help" << endl;
   } else if(!strcmp(line, "clear")){
      done = false;
      cerr << "Hey clear" << endl;
   } else if(!strcmp(line, "reset")){
      done = false;
      cerr << "Hey reset" << endl;
   } else if(!strcmp(line, "show") || !strcmp(line, "list") || !strcmp(line, "inputs")){
      done = false;
      cerr << "Hey show" << endl;
   } else if(!strcmp(line, "set")){
      done = false;
      cerr << "Hey set" << endl;
   } else {
      cerr << "Unrecognized! " << line  << endl;
   }
}

bool stdBaseInterface::go(const record &params)
{
   bool rstat(true);
   return rstat;
}

void stdBaseInterface::init(){
   done = false;
   rl_readline_name = "casactl";
   rl_attempted_completion_function = casactl_completion;
   commands.push_back("go");
   commands.push_back("run");
   commands.push_back("quit");
   commands.push_back("exit");
   commands.push_back("show");
   commands.push_back("list");
   commands.push_back("inputs");
   commands.push_back("set");
   commands.push_back("reset");
   commands.push_back("unset");
   commands.push_back("clear");

}

void dumpRecord(ostream &oss, const record &theRec, string spaces){
   spaces += "   ";
   for(::casac::rec_map::const_iterator rec_it = theRec.begin(); rec_it != theRec.end(); rec_it++){
      switch((*rec_it).second.type()){
         case ::casac::variant::RECORD :
            {
            const record &tmpRecord = (*rec_it).second.getRecord();
	    oss << spaces << (*rec_it).first << "   :record:" << endl;
            dumpRecord(oss, tmpRecord, spaces);
            }
            break;
         case ::casac::variant::BOOL :
            {oss << spaces << (*rec_it).first << " :bool: " << (*rec_it).second.toBool() << endl;}
            break;
         case ::casac::variant::INT :
            {oss << spaces << (*rec_it).first << " :int: " << (*rec_it).second.getInt() << endl;}
            break;
         case ::casac::variant::DOUBLE :
            {oss << spaces << (*rec_it).first << " :double: " << (*rec_it).second.getDouble() << endl;}
            break;
         case ::casac::variant::COMPLEX :
            {oss << spaces << (*rec_it).first << " :complex: " << (*rec_it).second.getComplex() << endl;}
            break;
         case ::casac::variant::STRING :
            {oss << spaces << (*rec_it).first << " :string: " << (*rec_it).second.getString() << endl;}
            break;
         case ::casac::variant::BOOLVEC :
            {
            vector<int> shapeVec((*rec_it).second.arrayshape());
            vector<bool>boolVec((*rec_it).second.getBoolVec());
            unsigned int i(0);
	    oss << spaces <<  (*rec_it).first << " :bool vector: [";
            for(vector<bool>::iterator iter = boolVec.begin();
               iter != boolVec.end(); iter++)
               oss << boolVec[i++] << " ";
	    oss << "]" << endl;
            }
            break;
         case ::casac::variant::INTVEC :
            {
            vector<int> shapeVec((*rec_it).second.arrayshape());
            vector<int>intVec((*rec_it).second.getIntVec());
            unsigned int i(0);
	    oss << spaces <<  (*rec_it).first << " :int vector: [";
            for(vector<int>::iterator iter = intVec.begin();
               iter != intVec.end(); iter++)
               oss << intVec[i++] << " ";
	    oss << "]" << endl;
            }
            break;
         case ::casac::variant::DOUBLEVEC :
            {
            vector<int> shapeVec((*rec_it).second.arrayshape());
            vector<double>doubleVec((*rec_it).second.getDoubleVec());
            unsigned int i(0);
	    oss << spaces <<  (*rec_it).first << " :double vector: [";
            for(vector<double>::iterator iter = doubleVec.begin();
               iter != doubleVec.end(); iter++)
               oss << doubleVec[i++] << " ";
	    oss << "]" << endl;
            }
            break;
         case ::casac::variant::COMPLEXVEC :
            {
            vector<int> shapeVec((*rec_it).second.arrayshape());
            vector<complex<double> >theVec((*rec_it).second.getComplexVec());
            unsigned int i(0);
	    oss << spaces <<  (*rec_it).first << " :complex vector: [";
            for(vector<complex<double> >::iterator iter = theVec.begin();
               iter != theVec.end(); iter++)
               oss << theVec[i++] << " ";
	    oss << "]" << endl;
            }
            break;
         case ::casac::variant::STRINGVEC :
            {
            vector<int> shapeVec((*rec_it).second.arrayshape());
            vector<string>theVec((*rec_it).second.getStringVec());
            unsigned int i(0);
	    oss << spaces <<  (*rec_it).first << " :string vector: [";
            for(vector<string>::iterator iter = theVec.begin();
               iter != theVec.end(); iter++)
               oss << theVec[i++] << " ";
	    oss << "]" << endl;
            }
            break;
         default :
            std::cerr << "Unknown type: " << (*rec_it).second.type() << std::endl;
            break;
      }
   }
}
bool stdBaseInterface::verifyOne(record &inputs, record &constraints, casa::LogIO &itsLog, bool silent){
   bool rstat(true);
   rec_map::iterator data = inputs.begin();
   record& params = constraints["parameters"].asRecord();
   if(params.count((*data).first)){
	   //dumpRecord(cerr, params[data->first].asRecord());
	   //cerr << endl << data->first;
	   //dumpRecord(cerr, params[data->first].asRecord());
	   //cerr << endl << data->first;
	   //dumpRecord(cerr, params[data->first].asRecord());
           if(params[data->first].asRecord().count("units")){
		   //cerr << endl << data->first;
		   //dumpRecord(cerr, constraints[data->first].asRecord());
	      rstat = stdBaseInterface::checkQuanta(data->first, data->second,
			                            params[data->first].asRecord(), itsLog, silent);
           } else {
	      rstat = stdBaseInterface::checkme(data->first, data->second,
			                        params[data->first].asRecord(), itsLog, silent);
	   }
   } else {
	   // Not constrainted
   }
   return rstat;
}

bool stdBaseInterface::checkQuanta(const string &param, variant &user, record &constraintsRec, casa::LogIO &itsLog, bool silent){
	bool rstat(true);
	/*
	cerr << "***********" << endl;
	itsLog << casa::LogIO::NORMAL << "Checking Quanta" << casa::LogIO::POST;
	cerr << "User type is: " << user.type() << endl;
	cerr << "***********" << endl;
	*/
	variant  &dflt = constraintsRec["units"];
	switch(user.type()){
		case variant::STRING :
			{
			casa::Quantity uqa;
			if(uqa.read(uqa, user.asString())){
			   if(!silent && !uqa.isConform(casa::Unit(dflt.asString()))){
			      itsLog << casa::LogIO::NORMAL << "checkQuanta failed! Units don't conform for "
				     << param << " needs to be " << dflt.asString() << "." << casa::LogIO::POST;
			      rstat = false;
		           }
			} else {
				if(!silent)
			      itsLog << casa::LogIO::NORMAL << param << " checkQuanta failed! Unable to form quantum from "
				     << user.asString() << "." << casa::LogIO::POST;
			      rstat = false;
			}
			} 
			break;
		case variant::STRINGVEC :
			{
			casa::Quantity uqa;
			vector<string> userVec = user.asStringVec();
			for(unsigned int i=0;i<userVec.size();i++){
		            if(uqa.read(uqa, userVec[i])){
			       if(!silent && !uqa.isConform(casa::Unit(dflt.asString()))){
			          itsLog << casa::LogIO::NORMAL << "checkQuanta failed! Units don't conform for "
				         << param << " needs to be " << dflt.asString() << "." << casa::LogIO::POST;
			          rstat = false;
		               }
			    } else {
			      if(!silent)
			      itsLog << casa::LogIO::NORMAL << param << " checkQuanta failed! Unable to form quantum from "
				     << userVec[i] << "." << casa::LogIO::POST;
			      rstat = false;
			    }
			}
			}
			break;
		case variant::INT :
			      if(!silent)
			itsLog << casa::LogIO::NORMAL << "checkQuanta assuming " << param << " units are "
			       << dflt.asString() << "." << casa::LogIO::POST;
			break;
		case variant::INTVEC :
			      if(!silent)
			itsLog << casa::LogIO::NORMAL << "checkQuanta assuming " << param << " units are "
			       << dflt.asString() << "." << casa::LogIO::POST;
			break;
		case variant::DOUBLE :
			      if(!silent)
			itsLog << casa::LogIO::NORMAL << "checkQuanta assuming " << param << "  units are "
			       << dflt.asString() << "." << casa::LogIO::POST;
			break;
		case variant::DOUBLEVEC :
			      if(!silent)
			itsLog << casa::LogIO::NORMAL << "checkQuanta assuming " << param << " units are "
			       << dflt.asString() << "." << casa::LogIO::POST;
			break;
		case variant::RECORD :
			// as long as the record has a value and unit fields things are fine.
			{
			record &qRec = user.asRecord();
                        if((qRec.find("value") == qRec.end()) || (qRec.find("unit") == qRec.end())){
			   rstat = false;
			      if(!silent)
			   itsLog << casa::LogIO::NORMAL << param << " checkQuanta failed! Record does not contain value and units fields"
				   << casa::LogIO::POST;
			   //dumpRecord(cerr, qRec);
			}
			}
			break;
		default :
			      if(!silent)
			itsLog << casa::LogIO::NORMAL << param << " checkQuanta failed! Conversion from type "
				<< user.type() << casa::LogIO::POST;
			cerr << param << " checkQuanta failed! Conversion from type " << user.type() << endl;
			//rstat = false;
			break;
	}
	return rstat;
}

variant *stdBaseInterface::expandEnum(variant &allowed, const variant &value, casa::LogIO &itsLog, bool silent){
   variant *rstat(0);
   bool ambiguous = false;
   int count = 0;
   if(allowed.type() == variant::STRINGVEC){
      if(value.type() == variant::STRING && value.getString().length() > 0){
         for(unsigned int i=0;i<allowed.asStringVec().size();i++){
            // cerr << i << " " << allowed.asStringVec()[i] << endl;
            if(!allowed.asStringVec()[i].compare(0, value.getString().length(), value.getString())){
	       rstat = new variant(allowed.asStringVec()[i]);
	       if(allowed.asStringVec()[i] == value.getString()){
		       count = 0;
		       break;
	       }
	       count++;
	    }
	 }
      }else if(value.type() == variant::STRINGVEC){
         rstat = new variant(value);
      }
      if(count > 1)ambiguous = true;
   } else if(allowed.type() == variant::STRING){
      if(value.type() == variant::STRING){
	 if(!allowed.asString().compare(0, value.getString().length(), value.getString())){
	    rstat = new variant(allowed.asString());
         }
      }
   }
   if(ambiguous && !silent){
      rstat = new variant(value);
      itsLog << casa::LogIO::NORMAL << value.getString() << " is ambiguous, please supply more characters!"
				<< casa::LogIO::POST;
      cerr << "******* " << value.getString() << " is ambiguous, please supply more characters! *******" << endl;
   }
   if(!rstat)
      rstat = new variant(value);
   return rstat;
}

bool stdBaseInterface::checkme(const string &param, variant &user, record &constraintsRec, casa::LogIO &itsLog, bool silent){
	bool rstat(true);
	//
	//Ah but if we're an any type then we don't want to check against the default value type
	//
	bool checkit(true);
	variant  &dflt = constraintsRec["value"];
	if(constraintsRec["type"].asString() == "any" || constraintsRec["type"].asString()  == "variant"){
	   if(constraintsRec.count("limittypes") &&  constraintsRec["type"].asString().size()){
	      string theAllowedTypes = constraintsRec["limittypes"].asString();
	      string myType = user.typeString();
	      if(theAllowedTypes.find(myType) != string::npos){
		      rstat = true;
	      } else {
	         if(myType == "int" && theAllowedTypes.find("double") != string::npos){
			rstat = true;
	          } else if(myType == "intvec" && theAllowedTypes.find("doublevec") != string::npos){
			rstat = true;
		  } else {
		      rstat = false;
		  }
	       }
	   }
	   checkit = false;
	   // 
	   if(!user.size())
		   rstat = true;
	}
	if(user.type() != dflt.type() && user.size() && dflt.size() && checkit){
		 switch(dflt.type()){
		        case variant::BOOLVEC :
			    if(user.type() != variant::BOOL)
		               rstat = false;
		            break;
		        case variant::INT:
			    if(user.type() != variant::INT)
		               rstat = false;
		            break;
		        case variant::DOUBLE:
			    if(!(user.type() == variant::DOUBLE || user.type() == variant::INT))
		               rstat = false;
		            break;
		        case variant::COMPLEX:
			    if(user.type() != variant::COMPLEX)
		               rstat = false;
		            break;
		        case variant::INTVEC :
			    if(!(user.type() == variant::INT || user.type() == variant::INTVEC))
		               rstat = false;
		            break;
		        case variant::DOUBLEVEC :
			    if(!(user.type() == variant::DOUBLE || user.type() == variant::DOUBLEVEC
				 || user.type() == variant::INT || user.type() == variant::INTVEC))
		               rstat = false;
		            break;
		        case variant::COMPLEXVEC :
			    if(!(user.type() == variant::COMPLEX || user.type() == variant::COMPLEXVEC))
		               rstat = false;
		            break;
		        case variant::STRINGVEC :
			    if(!(user.type() == variant::STRING || user.type() == variant::STRINGVEC))
		               rstat = false;
		            break;
		        case variant::RECORD:
			    break;
		        default :
		            rstat = false;
		 }
	   }
	   if(!silent && !rstat && (user.type() != dflt.type())){
	         itsLog << casa::LogIO::NORMAL << param << " has wrong type, expected " << dflt.typeString()
	              << " got " << user.typeString() << casa::LogIO::POST;
	         cerr << param << " has wrong type, expected " << dflt.typeString()
	              << " got " << user.typeString() << endl;
		
		 cerr << "The Constraints Record for "<< param << " has the following information:" << endl;
		 dumpRecord(cerr, constraintsRec);
		 if(user.type() == variant::RECORD){
		         cerr << "The User Record" << endl;
			 dumpRecord(cerr, user.asRecord());
		 }
	   }
// 
// Now let's make sure the value is allowable
//
	   if(rstat && constraintsRec.count("allowed")){
	      variant &dflt = constraintsRec["allowed"];
	      switch(user.type()){
		  case variant::BOOL:
		  case variant::BOOLVEC :
			  // Hey it can only be true or false
		     break;
		  case variant::INT:
		     {
		      unsigned int i=0;
		      vector<int> &theEnums = dflt.asIntVec();
		      while(i<theEnums.size()){
			if(user.asInt() == theEnums[i])
			   break;
			 i++;
		       }
		       if(i == theEnums.size())
			       rstat = false;
		     }
		     break;
		  case variant::INTVEC :
		     {
		      vector<int> &theEnums = dflt.asIntVec();
		      vector<int> &userVals = user.asIntVec();
		      unsigned int j(0);
		      while(j<userVals.size()){
		         unsigned int i=0;
		         while(i<theEnums.size()){
			   if(userVals[j] == theEnums[i])
			      break;
			    i++;
		          }
		          if(i == theEnums.size())
			       rstat = false;
			  j++;
		       }
		     }
		     break;
		  case variant::DOUBLE:
		     {
		      unsigned int i=0;
		      vector<double> &theEnums = dflt.asDoubleVec();
		      while(i<theEnums.size()){
			if(user.asDouble() == theEnums[i])
			   break;
			 i++;
		       }
		       if(i == theEnums.size())
			       rstat = false;
		     }
		     break;
		  case variant::DOUBLEVEC :
			  // Hey it can only be true or false
		     {
		      vector<int> &theEnums = dflt.asIntVec();
		      vector<int> &userVals = user.asIntVec();
		      unsigned int j(0);
		      while(j<userVals.size()){
		         unsigned int i=0;
		         while(i<theEnums.size()){
			   if(userVals[j] == theEnums[i])
			      break;
			    i++;
		          }
		          if(i == theEnums.size())
			       rstat = false;
			  j++;
		       }
		     }
		     break;
		  case variant::COMPLEX:
		     {
		      unsigned int i=0;
		      vector<complex<double> > &theEnums = dflt.asComplexVec();
		      while(i<theEnums.size()){
			if(user.asComplex() == theEnums[i])
			   break;
			 i++;
		       }
		       if(i == theEnums.size())
			       rstat = false;
		     }
		     break;
		  case variant::COMPLEXVEC :
			  // Hey it can only be true or false
		     {
		      vector<complex<double> > &theEnums = dflt.asComplexVec();
		      vector<complex<double> > &userVals = user.asComplexVec();
		      unsigned int j(0);
		      while(j<userVals.size()){
		         unsigned int i=0;
		         while(i<theEnums.size()){
			   if(userVals[j] == theEnums[i])
			      break;
			    i++;
		          }
		          if(i == theEnums.size())
			       rstat = false;
			  j++;
		       }
		     }
		     break;
		  case variant::STRING:
		     {
		      unsigned int i=0;
		      bool ignorecase(false);
		      if(constraintsRec.count("ignorecase")){
		         casa::String ignoreit(constraintsRec["ignorecase"].asString());
		         ignoreit.downcase();
		         if(ignoreit == "true"){
                            ignorecase=true;
		         }
		      }
		      vector<string> &theEnums = dflt.asStringVec();
		      while(i<theEnums.size()){
			// std::cerr << i << "*"<< theEnums[i] << "*"<< user.asString() << "*"<< std::endl;
			if(ignorecase){
			   casa::String theval(user.asString());
			   theval.downcase();
			   if(user.asString().length()){
			      if(!theEnums[i].compare(0, user.asString().length(), theval)){
			         user.asString() = theEnums[i];
			         break;
			      }
			   } else {
			      if(user.asString() == theEnums[i])
				 break;
			   }
			}else {
			   if(user.asString().length()){
			      if(!theEnums[i].compare(0, user.asString().length(), user.asString())){
			         user.asString() = theEnums[i];
			         break;
			      }
			   } else if(user.asString() == theEnums[i]) {
				 break;
			   }
			}
			 i++;
		       }
		       if(i == theEnums.size())
			       rstat = false;
		     }
		     break;
		  case variant::STRINGVEC :
		     {
		      vector<string> &theEnums = dflt.asStringVec();
		      vector<string> &userVals = user.asStringVec();
		      unsigned int j(0);
		      while(j<userVals.size()){
		         unsigned int i=0;
		         while(i<theEnums.size()){
		            if(userVals[j].length()){
			    if(!theEnums[i].compare(0, userVals[j].length(), userVals[j]))
			       break;
			    } else if(userVals[j] == theEnums[i]){
			       break;
			    }
			    i++;
		          }
		          if(i == theEnums.size())
			       rstat = false;
			  j++;
		       }
		     }
		     break;
		  default :
		     break;
	      }
	      if(!silent && !rstat){
		   itsLog << casa::LogIO::NORMAL << "Enum checking failed for " << param << casa::LogIO::POST;
	      }
	   }
	   //cerr << param << " has a range record: " << constraintsRec.count("range") << endl;
	   if(rstat && constraintsRec.count("range")){
	      variant &dflt2 = constraintsRec["range"];
	      //cerr << "range type is " << dflt2.type() << endl;
	      //dumpRecord(cerr, dflt2.asRecord());
	      // Not quite there yet as it requires theMins.size() == theMaxs.size()
	      //
	      switch(user.type()){
		  case variant::INT:
		     {
		      unsigned int i=0;
		      vector<int> &theMins = dflt2.asRecord()["min"].asRecord()["value"].asIntVec();
		      vector<int> &theMaxs = dflt2.asRecord()["max"].asRecord()["value"].asIntVec();
		      if(!theMins.size()){
		         while(i<theMaxs.size()){
			    if(user.asInt() <= theMaxs[i]){
				    break;
			    }
			    i++;
			 }
		      } else if(!theMaxs.size()){
		         while(i<theMins.size()){
			    if(theMins[i] <= user.asInt()){
				    break;
			    }
			    i++;
			 }
		      } else {
		         while(i<theMins.size()){
			    if(theMins[i] <= user.asInt() && user.asInt() <= theMaxs[i]){
				    break;
			    }
			    i++;
			 }
		      }
		      if(theMins.size() && i == theMins.size())
			       rstat = false;
		      if(theMaxs.size() && i == theMaxs.size())
			       rstat = false;

		     }
		     break;
		  case variant::INTVEC :
		     {
		      vector<int> &theMins = dflt2.asRecord()["min"].asRecord()["value"].asIntVec();
		      vector<int> &theMaxs = dflt2.asRecord()["max"].asRecord()["value"].asIntVec();
		      vector<int> &userVals = user.asIntVec();
		      unsigned int j(0);
		      //cerr << "Range checking: mins: ";
		      //dumpRecord(cerr, dflt2.asRecord()["min"].asRecord());
		      //cerr << "Range checking: maxs: ";
		      //dumpRecord(cerr, dflt2.asRecord()["max"].asRecord());
		      while(j<userVals.size()){
		         unsigned int i=0;
			 if(!theMins.size()){
		            while(i<theMaxs.size()){
				  if(userVals[j] <= theMaxs[i])
				     break;
			       i++;
			    }
			 }else if(!theMaxs.size()){
		            while(i<theMins.size()){
				  if(userVals[j] >= theMins[i])
				     break;
			       i++;
			    }
			 } else {
		            while(i<theMins.size()){
				    if(theMins[i] <= userVals[j] && userVals[j] <= theMaxs[i])
					    break;
			       i++;
			    }
			 }
		         j++;
			 //cerr << theMins.size() << " " << i << endl;
			 //cerr << theMaxs.size() << " " << i << endl;
		         if(theMins.size() && i == theMins.size())
			       rstat = false;
		         if(theMaxs.size() && i == theMaxs.size())
			       rstat = false;
		      }
		     }
		     break;
		  case variant::DOUBLE:
		     {
		      unsigned int i=0;
		      vector<double> &theMins = dflt2.asRecord()["min"].asRecord()["value"].asDoubleVec();
		      vector<double> &theMaxs = dflt2.asRecord()["max"].asRecord()["value"].asDoubleVec();
		      if(!theMins.size()){
		         while(i<theMaxs.size()){
			    if(user.asDouble() <= theMaxs[i]){
				    break;
			    }
			    i++;
			 }
		      } else if(!theMaxs.size()){
		         while(i<theMins.size()){
			    if(theMins[i] <= user.asDouble()){
				    break;
			    }
			    i++;
			 }
		      } else {
		         while(i<theMins.size()){
			    if(theMins[i] <= user.asDouble() && user.asDouble() <= theMaxs[i]){
				    break;
			    }
			    i++;
			 }
		      }
		      if(theMins.size() && i == theMins.size())
			       rstat = false;
		      if(theMaxs.size() && i == theMaxs.size())
			       rstat = false;
		     }
		     break;
		  case variant::DOUBLEVEC :
			  // Hey it can only be true or false
		     {
		      vector<double> &theMins = dflt2.asRecord()["min"].asRecord()["value"].asDoubleVec();
		      vector<double> &theMaxs = dflt2.asRecord()["max"].asRecord()["value"].asDoubleVec();
		      vector<double> &userVals = user.asDoubleVec();
		      unsigned int j(0);
		      while(j<userVals.size()){
		         unsigned int i=0;
			 if(!theMins.size()){
		            while(i<theMaxs.size()){
				  if(userVals[j] <= theMaxs[i])
				     break;
			    }
			    i++;
			 }else if(!theMaxs.size()){
		            while(i<theMins.size()){
				  if(userVals[j] >= theMins[i])
				     break;
			    }
			    i++;
			 } else {
		            while(i<theMins.size()){
				    if(theMins[i] <= userVals[j] && userVals[j] <= theMaxs[i])
					    break;
			    }
			    i++;
			 }
		         j++;
		         if(theMins.size() && i == theMins.size())
			       rstat = false;
		         if(theMaxs.size() && i == theMaxs.size())
			       rstat = false;
		      }
		     }
		     break;
		     // 
		     // No range checking for these
		     // 
		  case variant::BOOL:
		  case variant::BOOLVEC :
		  case variant::COMPLEX:
		  case variant::COMPLEXVEC :
		  case variant::STRING:
		  case variant::STRINGVEC :
		     break;
		  default :
		     break;
	      }
	      if(!silent && !rstat){
		   itsLog << casa::LogIO::NORMAL << "Range checking failed for " << param << casa::LogIO::POST;
	      }
	   }
	   //
	   // File checking, if the attribute allowblank has been set then just pass it through
	   // otherwise the file must exist to verify
	   //
	   if(constraintsRec.count("mustexist")){
              bool blanksok(false);
	      if(constraintsRec.count("allowblank")){
		      blanksok = constraintsRec["allowblank"].asBool();
	      }
	      if(constraintsRec["mustexist"].asBool()){
	         struct stat mystat;
		 if(user.type() == variant::STRINGVEC){
			 vector<string> files = user.asStringVec();
			 for(unsigned int i = 0; i<files.size(); i++){
			     if(blanksok && files[i] == "")
				continue;
			     else if(stat(files[i].c_str(), &mystat)){
		                rstat = false;
				break;
			     }
			 }
		 } else {
		    if(blanksok && user.asString() == "")
			    rstat = true;
		    else if(stat(user.asString().c_str(), &mystat))
		       rstat = false;
                 }
	         if(!silent && !rstat){
		      itsLog << casa::LogIO::NORMAL <<  "File existance checking failed for "
			      << param << " " << user.asString() << casa::LogIO::POST;
		 }
	      }
	   }
	return rstat;
}

bool stdBaseInterface::verify(record &inputs, record &constraints, casa::LogIO &itsLog)
{
   bool rstat(true);
//
//Loop over the constraints
//   check that the inputs record has the same type as the constraints
//   if there allowed choices make sure the inputs has an acceptable value
//   if there are ranges make sure the value is in rang
//   if an inputs field is missing, check to make sure the constraints has
//   a default
//
   /*
   dumpRecord(cerr, inputs);
   cerr << "*******" <<endl;
   dumpRecord(cerr, constraints["parameters"].asRecord());
   */
   record& params = constraints["parameters"].asRecord();
   for(rec_map::iterator iter = params.begin(); iter != params.end(); iter++) {
     // First check to make sure there is a variable available.
     if(!inputs.count((*iter).first)) {
       if(!((*iter).second.asRecord().count("value"))) {
	 itsLog << casa::LogIO::NORMAL << "Missing " << (*iter).first << " and no default is available!" << casa::LogIO::POST;
	 rstat = false;
       }
     } else {
       variant &user = inputs[(*iter).first];
       record &dflt = (*iter).second.asRecord();
       if(dflt.count("units"))
	 rstat = stdBaseInterface::checkQuanta((*iter).first, user, dflt, itsLog, false);
       else
	 rstat = stdBaseInterface::checkme((*iter).first, user, dflt, itsLog, false);
     }
     if(!rstat) {
       itsLog << casa::LogIO::WARN << "Argument " << (*iter).first << " failed to verify." << casa::LogIO::POST;
       break;
     }
   }
   return rstat;

   /*
   for(rec_map::iterator iter = constraints.begin();
       iter != constraints.end(); iter++){
	   // First check to make sure there is a variable available.
       if(!inputs.count((*iter).first)){
	   if(!(iter->first == "shortdescription" || iter->first == "description"
	        || iter->first == "example" || iter->first == "returns" || iter->first == "constraints"
		|| iter->first == "")){
		   // std::cerr << "*** " << iter->first << " ***" << std::endl;
	      if(!((*iter).second.asRecord().count("value"))){
	         itsLog << casa::LogIO::NORMAL << "Missing " << (*iter).first << " and no default is available!" << casa::LogIO::POST;
	         rstat = false;
	      }
	   }
       } else {
	   variant &user = inputs[(*iter).first];
	   record &dflt = (*iter).second.asRecord();
	   if(dflt.count("units"))
	      rstat = stdBaseInterface::checkQuanta((*iter).first, user, dflt, itsLog, false);
	   else
              rstat = stdBaseInterface::checkme((*iter).first, user, dflt, itsLog, false);
       }
       if(!rstat){
	  itsLog << casa::LogIO::WARN << "Argument " << (*iter).first << " failed to verify." << casa::LogIO::POST;
	  break;
       }
       //cerr << iter->first << " " << rstat << endl;
    }
   return rstat;
   */
}

}
