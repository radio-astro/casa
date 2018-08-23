#
# This file was generated using xslt from its XML file
#
# Copyright 2014, Associated Universities Inc., Washington DC
#
import sys
import os
#from casac import *
import casac
import string
import time
import inspect
import gc
import numpy
from casa_stack_manip import stack_frame_find
from odict import odict
from types import *
from task_hsd_imaging import hsd_imaging
class hsd_imaging_cli_:
    __name__ = "hsd_imaging"
    rkey = None
    i_am_a_casapy_task = None
    # The existence of the i_am_a_casapy_task attribute allows help()
    # (and other) to treat casapy tasks as a special case.

    def __init__(self) :
       self.__bases__ = (hsd_imaging_cli_,)
       self.__doc__ = self.__call__.__doc__

       self.parameters={'mode':None, 'restfreq':None, 'pipelinemode':None, 'infiles':None, 'field':None, 'spw':None, 'dryrun':None, 'acceptresults':None, }


    def result(self, key=None):
	    #### and add any that have completed...
	    return None


    def __call__(self, mode=None, restfreq=None, pipelinemode=None, infiles=None, field=None, spw=None, dryrun=None, acceptresults=None, ):

        """Generate single dish images

	Detailed Description:


	Arguments :
		mode:	Imaging mode [\'line\'|\'ampcal\']
		   Default Value: line
		   Allowed Values:
				line
				ampcal

		restfreq:	Rest frequency
		   Default Value: 

		pipelinemode:	The pipeline operating mode
		   Default Value: automatic
		   Allowed Values:
				automatic
				interactive
				getinputs

		infiles:	List of input files to be calibrated (default all)
		   Default Value: 

		field:	Field to be imaged, e.g., \'M100,Sgr*\' (default all)
		   Default Value: 

		spw:	select data by spectral window IDs, e.g. \'3,5,7\' (default all)
		   Default Value: 

		dryrun:	Run the task (False) or display task command (True)
		   Default Value: False

		acceptresults:	Add the results into the pipeline context
		   Default Value: True

	Returns: void

	Example :

The hsd_imaging task generates single dish images per antenna as well as 
combined image over whole antennas for each field and spectral window. 
Image configuration (grid size, number of pixels, etc.) is automatically 
determined based on meta data such as antenna diameter, map extent, etc.

Note that generated images are always in LSRK frame.
  
Keyword arguments:

mode -- imaging mode controls imaging parameters in the task.
   Accepts either 'line' (spectral line imaging) or 'ampcal' (image settings
   for amplitude calibrator)
   default: 'line'
   options: 'line', 'ampcal',

---- pipeline parameter arguments which can be set in any pipeline mode

pipelinemode -- The pipeline operating mode. In 'automatic' mode the 
   pipeline determines the values of all context defined pipeline inputs
   automatically.  In 'interactive' mode the user can set the pipeline
   context defined parameters manually.  In 'getinputs' mode the user
   can check the settings of all pipeline parameters without running
   the task.
   default: 'automatic'.

---- pipeline context defined parameter argument which can be set only in
'interactive mode'

infiles -- List of data files. These must be a name of MeasurementSets that 
   are registered to context via hsd_importdata task.
   default: []
   example: vis=['uid___A002_X85c183_X36f.ms', 'uid___A002_X85c183_X60b.ms']

field -- Data selection by field names or ids.
   default: '' (all fields)
   example: '*Sgr*,M100'
   
spw -- Data selection by spw IDs.
   default: '' (all spws)
   example: '3,4' (generate images for spw 3 and 4)

--- pipeline task execution modes

dryrun -- Run the commands (True) or generate the commands to be run but
   do not execute (False).
   default: True

acceptresults -- Add the results of the task to the pipeline context (True) or
   reject them (False).
   default: True

Output:

results -- If pipeline mode is 'getinputs' then None is returned. Otherwise
   the results object for the pipeline task is returned.


Examples



        """
	if not hasattr(self, "__globals__") or self.__globals__ == None :
           self.__globals__=stack_frame_find( )
	#casac = self.__globals__['casac']
	casalog = self.__globals__['casalog']
	casa = self.__globals__['casa']
	#casalog = casac.casac.logsink()
        self.__globals__['__last_task'] = 'hsd_imaging'
        self.__globals__['taskname'] = 'hsd_imaging'
        ###
        self.__globals__['update_params'](func=self.__globals__['taskname'],printtext=False,ipython_globals=self.__globals__)
        ###
        ###
        #Handle globals or user over-ride of arguments
        #
        if type(self.__call__.func_defaults) is NoneType:
            function_signature_defaults={}
	else:
	    function_signature_defaults=dict(zip(self.__call__.func_code.co_varnames[1:],self.__call__.func_defaults))
	useLocalDefaults = False

        for item in function_signature_defaults.iteritems():
                key,val = item
                keyVal = eval(key)
                if (keyVal == None):
                        #user hasn't set it - use global/default
                        pass
                else:
                        #user has set it - use over-ride
			if (key != 'self') :
			   useLocalDefaults = True

	myparams = {}
	if useLocalDefaults :
	   for item in function_signature_defaults.iteritems():
	       key,val = item
	       keyVal = eval(key)
	       exec('myparams[key] = keyVal')
	       self.parameters[key] = keyVal
	       if (keyVal == None):
	           exec('myparams[key] = '+ key + ' = self.itsdefault(key)')
		   keyVal = eval(key)
		   if(type(keyVal) == dict) :
                      if len(keyVal) > 0 :
		         exec('myparams[key] = ' + key + ' = keyVal[len(keyVal)-1][\'value\']')
		      else :
		         exec('myparams[key] = ' + key + ' = {}')

        else :
            print ''

            myparams['mode'] = mode = self.parameters['mode']
            myparams['restfreq'] = restfreq = self.parameters['restfreq']
            myparams['pipelinemode'] = pipelinemode = self.parameters['pipelinemode']
            myparams['infiles'] = infiles = self.parameters['infiles']
            myparams['field'] = field = self.parameters['field']
            myparams['spw'] = spw = self.parameters['spw']
            myparams['dryrun'] = dryrun = self.parameters['dryrun']
            myparams['acceptresults'] = acceptresults = self.parameters['acceptresults']

        if type(restfreq)==str: restfreq=[restfreq]
        if type(infiles)==str: infiles=[infiles]

	result = None

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['mode'] = mode
        mytmp['restfreq'] = restfreq
        mytmp['pipelinemode'] = pipelinemode
        mytmp['infiles'] = infiles
        mytmp['field'] = field
        mytmp['spw'] = spw
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli/"
	trec = casac.casac.utils().torecord(pathname+'hsd_imaging.xml')

        casalog.origin('hsd_imaging')
	try :
          #if not trec.has_key('hsd_imaging') or not casac.casac.utils().verify(mytmp, trec['hsd_imaging']) :
	    #return False

          casac.casac.utils().verify(mytmp, trec['hsd_imaging'], True)
          scriptstr=['']
          saveinputs = self.__globals__['saveinputs']
          if type(self.__call__.func_defaults) is NoneType:
              saveinputs=''
          else:
              saveinputs('hsd_imaging', 'hsd_imaging.last', myparams, self.__globals__,scriptstr=scriptstr)
          tname = 'hsd_imaging'
          spaces = ' '*(18-len(tname))
          casalog.post('\n##########################################'+
                       '\n##### Begin Task: ' + tname + spaces + ' #####')
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('Begin Task: ' + tname)
          if type(self.__call__.func_defaults) is NoneType:
              casalog.post(scriptstr[0]+'\n', 'INFO')
          else :
              casalog.post(scriptstr[1][1:]+'\n', 'INFO')
          result = hsd_imaging(mode, restfreq, pipelinemode, infiles, field, spw, dryrun, acceptresults)
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('End Task: ' + tname)
          casalog.post('##### End Task: ' + tname + '  ' + spaces + ' #####'+
                       '\n##########################################')

	except Exception, instance:
          if(self.__globals__.has_key('__rethrow_casa_exceptions') and self.__globals__['__rethrow_casa_exceptions']) :
             raise
          else :
             #print '**** Error **** ',instance
	     tname = 'hsd_imaging'
             casalog.post('An error occurred running task '+tname+'.', 'ERROR')
             pass
	casalog.origin('')

        gc.collect()
        return result
#
#
#
#    def paramgui(self, useGlobals=True, ipython_globals=None):
#        """
#        Opens a parameter GUI for this task.  If useGlobals is true, then any relevant global parameter settings are used.
#        """
#        import paramgui
#	if not hasattr(self, "__globals__") or self.__globals__ == None :
#           self.__globals__=stack_frame_find( )
#
#        if useGlobals:
#	    if ipython_globals == None:
#                myf=self.__globals__
#            else:
#                myf=ipython_globals
#
#            paramgui.setGlobals(myf)
#        else:
#            paramgui.setGlobals({})
#
#        paramgui.runTask('hsd_imaging', myf['_ip'])
#        paramgui.setGlobals({})
#
#
#
#
    def defaults(self, param=None, ipython_globals=None, paramvalue=None, subparam=None):
	if not hasattr(self, "__globals__") or self.__globals__ == None :
           self.__globals__=stack_frame_find( )
        if ipython_globals == None:
            myf=self.__globals__
        else:
            myf=ipython_globals

        a = odict()
        a['mode']  = 'line'
        a['restfreq']  = ['']
        a['pipelinemode']  = 'automatic'

        a['pipelinemode'] = {
                    0:{'value':'automatic'}, 
                    1:odict([{'value':'interactive'}, {'infiles':[]}, {'field':''}, {'spw':''}, {'dryrun':False}, {'acceptresults':True}]), 
                    2:odict([{'value':'getinputs'}, {'infiles':[]}, {'field':''}, {'spw':''}])}

### This function sets the default values but also will return the list of
### parameters or the default value of a given parameter
        if(param == None):
                myf['__set_default_parameters'](a)
        elif(param == 'paramkeys'):
                return a.keys()
        else:
            if(paramvalue==None and subparam==None):
               if(a.has_key(param)):
                  return a[param]
               else:
                  return self.itsdefault(param)
            else:
               retval=a[param]
               if(type(a[param])==dict):
                  for k in range(len(a[param])):
                     valornotval='value'
                     if(a[param][k].has_key('notvalue')):
                        valornotval='notvalue'
                     if((a[param][k][valornotval])==paramvalue):
                        retval=a[param][k].copy()
                        retval.pop(valornotval)
                        if(subparam != None):
                           if(retval.has_key(subparam)):
                              retval=retval[subparam]
                           else:
                              retval=self.itsdefault(subparam)
		     else:
                        retval=self.itsdefault(subparam)
               return retval


#
#
    def check_params(self, param=None, value=None, ipython_globals=None):
      if ipython_globals == None:
          myf=self.__globals__
      else:
          myf=ipython_globals
#      print 'param:', param, 'value:', value
      try :
         if str(type(value)) != "<type 'instance'>" :
            value0 = value
            value = myf['cu'].expandparam(param, value)
            matchtype = False
            if(type(value) == numpy.ndarray):
               if(type(value) == type(value0)):
                  myf[param] = value.tolist()
               else:
                  #print 'value:', value, 'value0:', value0
                  #print 'type(value):', type(value), 'type(value0):', type(value0)
                  myf[param] = value0
                  if type(value0) != list :
                     matchtype = True
            else :
               myf[param] = value
            value = myf['cu'].verifyparam({param:value})
            if matchtype:
               value = False
      except Exception, instance:
         #ignore the exception and just return it unchecked
         myf[param] = value
      return value
#
#
    def description(self, key='hsd_imaging', subkey=None):
        desc={'hsd_imaging': 'Generate single dish images',
               'mode': 'Imaging mode [\'line\'|\'ampcal\']',
               'restfreq': 'Rest frequency',
               'pipelinemode': 'The pipeline operating mode',
               'infiles': 'List of input files to be calibrated (default all)',
               'field': 'Field to be imaged, e.g., \'M100,Sgr*\' (default all)',
               'spw': 'select data by spectral window IDs, e.g. \'3,5,7\' (default all)',
               'dryrun': 'Run the task (False) or display task command (True)',
               'acceptresults': 'Add the results into the pipeline context',

              }

#
# Set subfields defaults if needed
#

        if(desc.has_key(key)) :
           return desc[key]

    def itsdefault(self, paramname) :
        a = {}
        a['mode']  = 'line'
        a['restfreq']  = ['']
        a['pipelinemode']  = 'automatic'
        a['infiles']  = ['']
        a['field']  = ''
        a['spw']  = ''
        a['dryrun']  = False
        a['acceptresults']  = True

        #a = sys._getframe(len(inspect.stack())-1).f_globals

        if self.parameters['pipelinemode']  == 'interactive':
            a['infiles'] = []
            a['field'] = ''
            a['spw'] = ''
            a['dryrun'] = False
            a['acceptresults'] = True

        if self.parameters['pipelinemode']  == 'getinputs':
            a['infiles'] = []
            a['field'] = ''
            a['spw'] = ''

        if a.has_key(paramname) :
	      return a[paramname]
hsd_imaging_cli = hsd_imaging_cli_()
