#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <xmlcasa/utils/BaseInterface.h>
#include <casa/Containers/Record.h>


//extern "C" char **rl_completion_matches __P((const char *, rl_compentry_func_t *));

namespace casa {

//extern "C" char **casactl_completion __P((const char *, int , int));
//extern "C" char *command_generator __P((const char *, int));

// These are the bits to make readline work...


/* Generator function for command completion.  STATE lets us
 *    know whether to start from scratch; without any state
 *       (i.e. STATE == 0), then we start at the top of the list. */

extern "C" char *command_generator (const char *text, int state)
{
   static unsigned list_index, len;
   char *rstat = NULL;
   const char *name;
   bool amatch(false);
	      /* If this is a new word to complete, initialize now.  This
	       *      includes saving the length of TEXT for efficiency, and
	       *           initializing the index variable to 0. */
   if (!state)
   {
      list_index = 0;
      len = strlen (text);
   }

/* Return the next name which partially matches from the
*      command list. */

   vector<string> commands = BaseInterface::getcommands();
   while (!amatch && list_index < commands.size()) {
      name = commands[list_index].c_str();
      list_index++;
      if (strncmp (name, text, len) == 0){
         rstat = strdup(name);
	 amatch = True;
      }
   }
		  /* If no names matched, then return NULL. */
   return rstat;
}

/* Attempt to complete on the contents of TEXT.  START and END
 * bound the region of rl_line_buffer that contains the word to
 * complete.  TEXT is the word to complete.  We can use the entire
 * contents of rl_line_buffer in case we want to do some simple
 * parsing.  Returnthe array of matches, or NULL if there aren't any. */

extern "C" char **casactl_completion (const char *text, int start, int end)
{
   char **matches;
   matches = (char **)NULL;
	      /* If this word is at the start of the line, then it is a command
	       *      to complete.  Otherwise it is the name of a file in the current
	       *           directory. */
   if (start == 0)
      matches = rl_completion_matches(text, command_generator);
   return (matches);
}

//
// Need to sync up the commands list with Joe's python inputs
//
vector<string> BaseInterface::commands(0);

BaseInterface::BaseInterface(Record &descParams)
{
	init();
//#include <xmlcasa/utils/standalones.h>
	//
	// Now loop over the the descParams.params record and
	// add those to the commands list
	//
}
BaseInterface::~BaseInterface(){}

Int BaseInterface::inputs(Record &params)
{
   Int rstat(0);
   while(!done){
      char *line = readline("casactl: ");
      if(line){
	 if(!strcmp(line, "quit") || !strcmp(line, "exit")){ 
		 done=True;
		 std::cerr << "Hey quit" << endl;
		 rstat = -1;
	 } else if(!strcmp(line, "go") || !strcmp(line, "run")){
		 done = True;
		 std::cerr << "Hey go" << endl;
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

void BaseInterface::parse_it(Record &params, const char *line){

   if(!strcmp(line, "help")){
      done = False;
      std::cerr << "Hey help" << endl;
   } else if(!strcmp(line, "clear")){
      done = False;
      std::cerr << "Hey clear" << endl;
   } else if(!strcmp(line, "reset")){
      done = False;
      std::cerr << "Hey reset" << endl;
   } else if(!strcmp(line, "show") || !strcmp(line, "list") || !strcmp(line, "inputs")){
      done = False;
      std::cerr << "Hey show" << endl;
   } else if(!strcmp(line, "set")){
      done = False;
      std::cerr << "Hey set" << endl;
   } else {
      std::cerr << "Unrecognized! " << line  << endl;
   }
}

Bool BaseInterface::go(const Record &params)
{
   Bool rstat(True);
   return rstat;
}

void BaseInterface::init(){
   done = False;
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


}
