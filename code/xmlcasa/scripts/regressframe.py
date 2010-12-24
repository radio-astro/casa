#pipeline['null'] ={'fill':{}, 'flag':{}, 'calibrate':{}, 'image':{}, 'analysis':{}}
#pipeline['fill']['name'] = ''
#pipeline['fill']['args'] = []
#pipeline['fill']['verify'] = ''
#pipeline['fill']['data'] = ''

#regression(pipeline)
import time
import os
from tasks import *

class regressionframe :
   steps = ['fill', 'flag', 'calibrate', 'image', 'analysis']
   title = "Regression test"
   datarepos = ''
   workdir = ''
   fill = {}
   flag = {}
   calibrate = {}
   image = {}
   analysis = {}

   def run(self, workdir='testing', verbose=True) :
      try :
         startdir = os.getcwd()
         os.mkdir(self.workdir)
         os.chdir(self.workdir)
         startTime = time.time()
         print self.title + ":" + workdir
         for mystep in self.steps :
	    print "Processing step: " +mystep
	    beenrun = {}
	    for task in eval('self.'+mystep+"['tasks']") :
		    beenrun[task] = 0
	    for task in eval('self.'+mystep+"['tasks']") :
               #print "   "+ eval('self.'+mystep+"['"+task+"']")
               mycommand = "self."+mystep+"['"+task+"']"
	       the_args = eval(mycommand)
	       arg_list = ''
	       thecommand=task+'('
	       if(the_args.has_key('args')) :
		       argscount = len(the_args['args'].keys())
		       argcount = 0
		       comma = ', '
		       for key in the_args['args'] :
		               argcount = argcount+1
			       if(argcount == argscount):
			          comma = ''
			       quoteit = ''
			       if type(the_args['args'][key]) == str :
				       quoteit = "'"
			       arg_list=arg_list+key+'='+quoteit+str(the_args['args'][key])+quoteit+comma
		       thecommand=thecommand+arg_list
	       elif(len(the_args.keys())>1) :
		       count = beenrun[task]
		       argscount = len(the_args[count]['args'].keys())
		       argcount = 0
		       comma = ', '
		       for key in the_args[count]['args'] :
		               argcount = argcount+1
			       if(argcount == argscount):
			          comma = ''
			       quoteit = ''
			       if type(the_args[count]['args'][key]) == str :
				       quoteit = "'"
			       arg_list=arg_list+key+'='+quoteit+str(the_args[count]['args'][key])+quoteit+comma
		       thecommand=thecommand+arg_list
		       beenrun[task] = beenrun[task]+1
		       
	       thecommand=thecommand+')'
	       beginTime = time.time()
	       if verbose :
	          print thecommand
	       eval(thecommand)
	       endTime = time.time()
	       print "   " + task+': Time to complete was '+str(endTime-beginTime)
	    if (eval("self."+mystep+".has_key('verify')")) :
	        verified = eval('self.'+mystep+"['verify']()")
	        if not verified :
		   print mystep + " has failed verification."
                   os.chdir(startdir)
		   return False
	        else :
	           print '   '+mystep+' verified'
         endTime = time.time()
         print 'Total time was: '+str(endTime-startTime)
         os.chdir(startdir)
         return True
      except Exception, instance:
	 print "***Oops exception*** ",instance
         os.chdir(startdir)
	 return False
