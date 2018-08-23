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
from task_hpc_hif_refant import hpc_hif_refant
class hpc_hif_refant_cli_:
    __name__ = "hpc_hif_refant"
    rkey = None
    i_am_a_casapy_task = None
    # The existence of the i_am_a_casapy_task attribute allows help()
    # (and other) to treat casapy tasks as a special case.

    def __init__(self) :
       self.__bases__ = (hpc_hif_refant_cli_,)
       self.__doc__ = self.__call__.__doc__

       self.parameters={'vis':None, 'field':None, 'intent':None, 'spw':None, 'hm_refant':None, 'refant':None, 'refantignore':None, 'geometry':None, 'flagging':None, 'pipelinemode':None, 'dryrun':None, 'acceptresults':None, 'parallel':None, }


    def result(self, key=None):
	    #### and add any that have completed...
	    return None


    def __call__(self, vis=None, field=None, intent=None, spw=None, hm_refant=None, refant=None, refantignore=None, geometry=None, flagging=None, pipelinemode=None, dryrun=None, acceptresults=None, parallel=None, ):

        """Select the best reference antennas
	Arguments :
		vis:	List of input MeasurementSets
		   Default Value: 

		field:	List of field names or ids
		   Default Value: 

		intent:	List of data selection intents
		   Default Value: AMPLITUDE,BANDPASS,PHASE

		spw:	List of spectral windows ids
		   Default Value: 

		hm_refant:	The referance antenna heuristics mode
		   Default Value: automatic
		   Allowed Values:
				automatic
				manual

		refant:	List of reference antennas
		   Default Value: 

		refantignore:	String list of antennas to ignore
		   Default Value: 

		geometry:	Score by proximity to center of the array
		   Default Value: True

		flagging:	Score by percentage of good data
		   Default Value: True

		pipelinemode:	The pipeline operating mode
		   Default Value: automatic
		   Allowed Values:
				automatic
				interactive
				getinputs

		dryrun:	Run the task (False) or display the command (True)
		   Default Value: False

		acceptresults:	Add the results into the pipeline context
		   Default Value: True

		parallel:	Execute using CASA HPC functionality, if available.
		   Default Value: automatic
		   Allowed Values:
				automatic
				true
				false

	Returns: void

	Example :

The hif_session_refant task selects a list of reference antennas and outputs them
in priority order. The priority order is determined by a weighted combination
of scores derived by the antenna selection heuristics.


Keyword arguments:

---- pipeline parameter arguments which can be set in any pipeline mode


pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
determines the values of all context defined pipeline inputs automatically.
In interactive mode the user can set the pipeline context defined parameters
manually. In 'getinputs' mode the user can check the settings of all
pipeline parameters without running the task.
default: 'automatic'.

hm_refant -- The heuristics method or mode for selection the reference
antenna. The options are 'manual' and 'automatic. In manual
mode a user supplied referenence antenna refant is supplied.
In 'automatic' mode the antennas are selected automatically.
default: 'automatic'

refant -- The user supplied reference antenna for 'manual' mode. If
no antenna list is supplied an empty list is returned.
default: ''
example: 'DV05'

geometry -- Score antenna by proximity to the center of the array. This
option is quick as only the ANTENNA table must be read.
default: True

flagging -- Score antennas by percentage of unflagged data. This option
requires computing flagging statistics.
default: True


---- pipeline context defined parameter arguments which can be set only in
'interactive mode'

vis -- The list of input MeasurementSets. Defaults to the list of
MeasurementSets in the pipeline context.
default: ''
example: ['M31.ms']

field -- The comma delimited list of field names or field ids for which flagging
scores are computed if hm_refant='automatic' and flagging = True
default: '' (Default to fields with the specified intents)
example: '3C279', '3C279,M82'

intent -- A string containing a comma delimited list of intents against
which the the selected fields are matched. Defaults to all supported
intents.
default: 'AMPLI,BANDPASS,PHASE'
example: 'BANDPASS'

spw -- A string containing the comma delimited list of spectral window ids for
which flagging scores are computed if hm_refant='automatic' and flagging = True.
default: '' (all spws observed with the specified intents)
example: '11,13,15,17'

refantignore -- string list to be ignored as reference antennas.
default: ''
Example: refantignore='ea02,ea03'


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

Description

The hif_refant task selects a list of reference antennas and stores them
in the pipeline context in priority order.

The priority order is determined by a weighted combination of scores derived
by the antenna selection heuristics. In manual mode the reference antennas
can be set by hand.


Issues

Examples

1. Compute the references antennas to be used for bandpass and gain calibration.

hpc_hif_refant()
        
        """
	if not hasattr(self, "__globals__") or self.__globals__ == None :
           self.__globals__=stack_frame_find( )
	#casac = self.__globals__['casac']
	casalog = self.__globals__['casalog']
	casa = self.__globals__['casa']
	#casalog = casac.casac.logsink()
        self.__globals__['__last_task'] = 'hpc_hif_refant'
        self.__globals__['taskname'] = 'hpc_hif_refant'
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

            myparams['vis'] = vis = self.parameters['vis']
            myparams['field'] = field = self.parameters['field']
            myparams['intent'] = intent = self.parameters['intent']
            myparams['spw'] = spw = self.parameters['spw']
            myparams['hm_refant'] = hm_refant = self.parameters['hm_refant']
            myparams['refant'] = refant = self.parameters['refant']
            myparams['refantignore'] = refantignore = self.parameters['refantignore']
            myparams['geometry'] = geometry = self.parameters['geometry']
            myparams['flagging'] = flagging = self.parameters['flagging']
            myparams['pipelinemode'] = pipelinemode = self.parameters['pipelinemode']
            myparams['dryrun'] = dryrun = self.parameters['dryrun']
            myparams['acceptresults'] = acceptresults = self.parameters['acceptresults']
            myparams['parallel'] = parallel = self.parameters['parallel']

        if type(vis)==str: vis=[vis]

	result = None

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['field'] = field
        mytmp['intent'] = intent
        mytmp['spw'] = spw
        mytmp['hm_refant'] = hm_refant
        mytmp['refant'] = refant
        mytmp['refantignore'] = refantignore
        mytmp['geometry'] = geometry
        mytmp['flagging'] = flagging
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
        mytmp['parallel'] = parallel
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli/"
	trec = casac.casac.utils().torecord(pathname+'hpc_hif_refant.xml')

        casalog.origin('hpc_hif_refant')
	try :
          #if not trec.has_key('hpc_hif_refant') or not casac.casac.utils().verify(mytmp, trec['hpc_hif_refant']) :
	    #return False

          casac.casac.utils().verify(mytmp, trec['hpc_hif_refant'], True)
          scriptstr=['']
          saveinputs = self.__globals__['saveinputs']
          if type(self.__call__.func_defaults) is NoneType:
              saveinputs=''
          else:
              saveinputs('hpc_hif_refant', 'hpc_hif_refant.last', myparams, self.__globals__,scriptstr=scriptstr)
          tname = 'hpc_hif_refant'
          spaces = ' '*(18-len(tname))
          casalog.post('\n##########################################'+
                       '\n##### Begin Task: ' + tname + spaces + ' #####')
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('Begin Task: ' + tname)
          if type(self.__call__.func_defaults) is NoneType:
              casalog.post(scriptstr[0]+'\n', 'INFO')
          else :
              casalog.post(scriptstr[1][1:]+'\n', 'INFO')
          result = hpc_hif_refant(vis, field, intent, spw, hm_refant, refant, refantignore, geometry, flagging, pipelinemode, dryrun, acceptresults, parallel)
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('End Task: ' + tname)
          casalog.post('##### End Task: ' + tname + '  ' + spaces + ' #####'+
                       '\n##########################################')

	except Exception, instance:
          if(self.__globals__.has_key('__rethrow_casa_exceptions') and self.__globals__['__rethrow_casa_exceptions']) :
             raise
          else :
             #print '**** Error **** ',instance
	     tname = 'hpc_hif_refant'
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
#        paramgui.runTask('hpc_hif_refant', myf['_ip'])
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
        a['hm_refant']  = 'automatic'
        a['pipelinemode']  = 'automatic'

        a['pipelinemode'] = {
                    0:{'value':'automatic'}, 
                    1:odict([{'value':'interactive'}, {'vis':[]}, {'field':''}, {'intents':'AMPLITUDE,BANDPASS,PHASE'}, {'spw':''}, {'dryrun':False}, {'acceptresults':True}, {'parallel':'automatic'}]), 
                    2:odict([{'value':'getinputs'}, {'vis':[]}, {'field':''}, {'intents':'AMPLITUDE,BANDPASS,PHASE'}, {'spw':''}])}
        a['hm_refant'] = {
                    0:odict([{'value':'automatic'}, {'geometry':True}, {'flagging':True}]), 
                    1:odict([{'value':'manual'}, {'refant':''}])}

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
    def description(self, key='hpc_hif_refant', subkey=None):
        desc={'hpc_hif_refant': 'Select the best reference antennas',
               'vis': 'List of input MeasurementSets',
               'field': 'List of field names or ids',
               'intent': 'List of data selection intents',
               'spw': 'List of spectral windows ids',
               'hm_refant': 'The referance antenna heuristics mode',
               'refant': 'List of reference antennas',
               'refantignore': 'String list of antennas to ignore',
               'geometry': 'Score by proximity to center of the array',
               'flagging': 'Score by percentage of good data',
               'pipelinemode': 'The pipeline operating mode',
               'dryrun': 'Run the task (False) or display the command (True)',
               'acceptresults': 'Add the results into the pipeline context',
               'parallel': 'Execute using CASA HPC functionality, if available.',

              }

#
# Set subfields defaults if needed
#

        if(desc.has_key(key)) :
           return desc[key]

    def itsdefault(self, paramname) :
        a = {}
        a['vis']  = ['']
        a['field']  = ''
        a['intent']  = 'AMPLITUDE,BANDPASS,PHASE'
        a['spw']  = ''
        a['hm_refant']  = 'automatic'
        a['refant']  = ''
        a['refantignore']  = ''
        a['geometry']  = True
        a['flagging']  = True
        a['pipelinemode']  = 'automatic'
        a['dryrun']  = False
        a['acceptresults']  = True
        a['parallel']  = 'automatic'

        #a = sys._getframe(len(inspect.stack())-1).f_globals

        if self.parameters['pipelinemode']  == 'interactive':
            a['vis'] = []
            a['field'] = ''
            a['intents'] = 'AMPLITUDE,BANDPASS,PHASE'
            a['spw'] = ''
            a['dryrun'] = False
            a['acceptresults'] = True
            a['parallel'] = 'automatic'

        if self.parameters['pipelinemode']  == 'getinputs':
            a['vis'] = []
            a['field'] = ''
            a['intents'] = 'AMPLITUDE,BANDPASS,PHASE'
            a['spw'] = ''

        if self.parameters['hm_refant']  == 'automatic':
            a['geometry'] = True
            a['flagging'] = True

        if self.parameters['hm_refant']  == 'manual':
            a['refant'] = ''

        if a.has_key(paramname) :
	      return a[paramname]
hpc_hif_refant_cli = hpc_hif_refant_cli_()
