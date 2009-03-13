import os
from taskinit import *
from tasksinfo import *

def find(matchstring=None):
       """ Find string in tasks, task names, parameter names:

	Lists the following:
		1) All of the task names that have the string
		2) All of the tasks whose  contents (e.g., documentation,
		parameters, etc) have the string
		3) All of the parameter names that have the string

       Keyword arguments:
       matchstring -- String to match in the documentation
               default: ''; example: matchstring='vis'

       """

       casalog.origin('find')

       #Python script
       #parameter_printvalues(arg_names,arg_values,arg_types)
       if ((type(matchstring)==str) & (matchstring!='')):
		pathname=os.environ.get('CASAPATH').split()[0]
		arch=os.environ.get('CASAPATH').split()[1]
		test=pathname.find('lib')
		if (test != -1):
			filepath=pathname+'/lib/python2.5/'
		else:
			filepath=pathname+'/'+arch+'/python/2.5/'

		# taskfiles = ['accum.py','simdata.py','applycal.py','bandpass.py','blcal.py','browsetable.py','clean.py','clearcal.py','clearplot.py','clearstat.py','concat.py','deconvolve.py','exportfits.py','exportuvfits.py','feather.py','filecatalog.py','flagautocorr.py','flagdata.py','flagmanager.py','fluxscale.py','fringecal.py','ft.py','gaincal.py','imhead.py','immoments.py','importasdm.py','importfits.py','importuvfits.py','importvla.py','invert.py','listcal.py','listhistory.py','listobs.py','makemask.py','mosaic.py','plotants.py','plotcal.py','plotxy.py','regridimage.py','setjy.py','smoothcal.py','split.py','uvcontsub.py','uvmodelfit.py','viewer.py']

		taskfiles = []
		for key in tasksum.keys() :
		    taskfiles.append(key+'.py')
		mydir = _ip.magic("sc -l = pwd")[0]
		os.chdir(filepath)

		print 'Found in:'
		print '-----------'
		print ''

		print 'Task Names:'
		for i in taskfiles:
			found=i.find(matchstring)
			if (found>=0):
				print '\t ',i.rstrip('.py')

                print 'Parameter Names:'

                paramstring='grep "def " parameter_dictionary.py | grep '+matchstring
                parameternames=_ip.magic("sc -l ="+paramstring)
                for i in parameternames:
                        temp1=i.lstrip('\tdef ')
                        temp2=temp1.rstrip('():')
                        print '\t ',temp2
		
		print 'Tasks (documentation, parameters)'

		for i in taskfiles:
		#for i in ['accum.py','applycal.py','bandpass.py','blcal.py']:
			mystring='grep -l '+matchstring+' '+i
			grepfile = _ip.magic("sc -l ="+mystring)[0]
			task = grepfile.rstrip('.py')
			if (task!=''): print '\t ',task

		os.chdir(mydir)

       else:
                raise Exception, 'Bad input string - try again'

