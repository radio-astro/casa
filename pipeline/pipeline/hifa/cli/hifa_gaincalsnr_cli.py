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
from task_hifa_gaincalsnr import hifa_gaincalsnr
class hifa_gaincalsnr_cli_:
    __name__ = "hifa_gaincalsnr"
    rkey = None
    i_am_a_casapy_task = None
    # The existence of the i_am_a_casapy_task attribute allows help()
    # (and other) to treat casapy tasks as a special case.

    def __init__(self) :
       self.__bases__ = (hifa_gaincalsnr_cli_,)
       self.__doc__ = self.__call__.__doc__

       self.parameters={'vis':None, 'field':None, 'intent':None, 'spw':None, 'phasesnr':None, 'bwedgefrac':None, 'hm_nantennas':None, 'maxfracflagged':None, 'pipelinemode':None, 'dryrun':None, 'acceptresults':None, }


    def result(self, key=None):
	    #### and add any that have completed...
	    return None


    def __call__(self, vis=None, field=None, intent=None, spw=None, phasesnr=None, bwedgefrac=None, hm_nantennas=None, maxfracflagged=None, pipelinemode=None, dryrun=None, acceptresults=None, ):

        """Compute gaincal signal to noise ratios per spw

	Detailed Description:

Compute the gain calibration signal to noise ratios required to determine which spectral window mapping strategy to use.

	Arguments :
		vis:	List of input MeasurementSets
		   Default Value: 

		field:	Set of data selection field names
		   Default Value: 

		intent:	Set of data selection observing intents
		   Default Value: PHASE

		spw:	Set of data selection spectral window ids
		   Default Value: 

		phasesnr:	The signal to noise minimum
		   Default Value: 25.0

		bwedgefrac:	The fraction of the bandwidth edge that is flagged
		   Default Value: 0.03125

		hm_nantennas:	The antenna selection heuristic (unsupported) 
		   Default Value: all
		   Allowed Values:
				all
				unflagged

		maxfracflagged:	The maximum fraction of data flagged per antenna (unsupported)
		   Default Value: 0.90

		pipelinemode:	The pipeline operating mode
		   Default Value: automatic
		   Allowed Values:
				automatic
				interactive
				getinputs

		dryrun:	Run task (False) or display the command(True)
		   Default Value: False

		acceptresults:	Automatically accept results into the context
		   Default Value: True

	Returns: void

	Example :


Compute the per scan gaincal solution signal to noise ratio per science spw

---- pipeline parameter arguments which can be set in any pipeline mode

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
       determines the values of all context defined pipeline inputs
       automatically.  In interactive mode the user can set the pipeline
       context defined parameters manually.  In 'getinputs' mode the user
       can check the settings of all pipeline parameters without running
       the task.
       default: 'automatic'.

phasesnr -- The required gaincal solution signal to noise
    default: 25.0
    example: phasesnr = 20.0

bwedgefrac -- The fraction of the bandwidth edges that is flagged
    default: 0.03125
    example: bwedgefrac = 0.0


hm_nantennas -- The heuristics for determines the number of antennas to use
    in the signal to noise estimate. The options are 'all' and 'unflagged'.
    The 'unflagged' options is not currently supported.
    default: 'all'
    example: hm_nantennas='unflagged'

maxfracflagged -- The maximum fraction of an antenna that can be flagged
    before its is excluded from the signal to noise estimate.
    default: 0.90
    example: maxfracflagged=0.80

---- pipeline context defined parameter arguments which can be set only in
'interactive mode'

vis -- The list of input MeasurementSets. Defaults to the list of MeasurementSets
    specified in the pipeline context
    default: ''
    example: ['M82A.ms', 'M82B.ms'] 

field -- The list of field names of sources to be used for signal to noise
    estimation. Defaults to all fields with the standard intent.
    default: '' 
    example: '3C279'

intent -- A string the intent against which the the selected fields are matched.
    Defaults to 'PHASE'.
    default: 'PHASE' 
    example: intent='BANDPASS'

spw -- The list of spectral windows and channels for which gain solutions are
    computed. Defaults to all the science spectral windows for which there are
    both 'intent' and TARGET intents.
    default: '' 
    example: '13,15'

--- pipeline task execution modes
dryrun -- Run the commands (True) or generate the commands to be run but
   do not execute (False).
   default: False

acceptresults -- Add the results of the task to the pipeline context (True) or
   reject them (False).
   default: True

Output:

results -- If pipeline mode is 'getinputs' then None is returned. Otherwise
    the results object for the pipeline task is returned

Description

The gaincal solution signal to noise is determined as follows
follows.

o For each data set the list of source(s) to use for the per scan gaincal solution signal
  to noise estimation is compiled based on the values of the field, intent,
  and spw parameters. 

o Source fluxes are determined for each spw and source combination.
    o Fluxes in Jy are derived from the pipeline context.
    o Pipeline context fluxes are derived from the online flux calibrator catalog,
      the ASDM, or the user via the flux.csv file.
    o If no fluxes are available the task terminates.

o Atmospheric calibration and observations scans are determined for each spw
  and source combination. 
    o If intent is set to 'PHASE' are there are no atmospheric scans
      associated with the 'PHASE' calibrator, 'TARGET' atmospheric scans
      will be used in stead.
    o If atmospheric scans cannot be associated with any of the spw and
      source combinations the task terminates.

o Science spws are mapped to atmospheric spws for each science spw and
  source combinations.
    o If mappings cannot be determined for any of the spws the task
      terminates

o The median Tsys value for each atmospheric spw and source combination is
  determined from the SYSCAL table. Medians are computed first by channel,
  then by antenna, in order to reduce sensitivity to deviant values.

o The science spw parameters, exposure time(s), and integration time(s) are
  determined.

o The per scan sensitivity and signal to noise estimates are computed per science
  spectral window. Nominal Tsys and sensitivity values per receiver band provide by
  the ALMA project are used for this estimate.

o The QA score is based on how many signal to noise estimates greater then the
  requested signal to noise ration can be computed.




Examples

1. Estimate the per scan gaincal solution sensitivities and signal to noise
   ratios for all the science spectral windows.

hifa_gaincalsnr()


        """
	if not hasattr(self, "__globals__") or self.__globals__ == None :
           self.__globals__=stack_frame_find( )
	#casac = self.__globals__['casac']
	casalog = self.__globals__['casalog']
	casa = self.__globals__['casa']
	#casalog = casac.casac.logsink()
        self.__globals__['__last_task'] = 'hifa_gaincalsnr'
        self.__globals__['taskname'] = 'hifa_gaincalsnr'
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
            myparams['phasesnr'] = phasesnr = self.parameters['phasesnr']
            myparams['bwedgefrac'] = bwedgefrac = self.parameters['bwedgefrac']
            myparams['hm_nantennas'] = hm_nantennas = self.parameters['hm_nantennas']
            myparams['maxfracflagged'] = maxfracflagged = self.parameters['maxfracflagged']
            myparams['pipelinemode'] = pipelinemode = self.parameters['pipelinemode']
            myparams['dryrun'] = dryrun = self.parameters['dryrun']
            myparams['acceptresults'] = acceptresults = self.parameters['acceptresults']

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
        mytmp['phasesnr'] = phasesnr
        mytmp['bwedgefrac'] = bwedgefrac
        mytmp['hm_nantennas'] = hm_nantennas
        mytmp['maxfracflagged'] = maxfracflagged
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli/"
	trec = casac.casac.utils().torecord(pathname+'hifa_gaincalsnr.xml')

        casalog.origin('hifa_gaincalsnr')
	try :
          #if not trec.has_key('hifa_gaincalsnr') or not casac.casac.utils().verify(mytmp, trec['hifa_gaincalsnr']) :
	    #return False

          casac.casac.utils().verify(mytmp, trec['hifa_gaincalsnr'], True)
          scriptstr=['']
          saveinputs = self.__globals__['saveinputs']
          if type(self.__call__.func_defaults) is NoneType:
              saveinputs=''
          else:
              saveinputs('hifa_gaincalsnr', 'hifa_gaincalsnr.last', myparams, self.__globals__,scriptstr=scriptstr)
          tname = 'hifa_gaincalsnr'
          spaces = ' '*(18-len(tname))
          casalog.post('\n##########################################'+
                       '\n##### Begin Task: ' + tname + spaces + ' #####')
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('Begin Task: ' + tname)
          if type(self.__call__.func_defaults) is NoneType:
              casalog.post(scriptstr[0]+'\n', 'INFO')
          else :
              casalog.post(scriptstr[1][1:]+'\n', 'INFO')
          result = hifa_gaincalsnr(vis, field, intent, spw, phasesnr, bwedgefrac, hm_nantennas, maxfracflagged, pipelinemode, dryrun, acceptresults)
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('End Task: ' + tname)
          casalog.post('##### End Task: ' + tname + '  ' + spaces + ' #####'+
                       '\n##########################################')

	except Exception, instance:
          if(self.__globals__.has_key('__rethrow_casa_exceptions') and self.__globals__['__rethrow_casa_exceptions']) :
             raise
          else :
             #print '**** Error **** ',instance
	     tname = 'hifa_gaincalsnr'
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
#        paramgui.runTask('hifa_gaincalsnr', myf['_ip'])
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
        a['phasesnr']  = 25.0
        a['bwedgefrac']  = 0.03125
        a['hm_nantennas']  = 'all'
        a['pipelinemode']  = 'automatic'

        a['hm_nantennas'] = {
                    0:{'value':'all'}, 
                    1:odict([{'value':'unflagged'}, {'maxfracflagged':0.90}])}
        a['pipelinemode'] = {
                    0:{'value':'automatic'}, 
                    1:odict([{'value':'interactive'}, {'vis':[]}, {'field':''}, {'intent':'PHASE'}, {'spw':''}, {'dryrun':False}, {'acceptresults':True}]), 
                    2:odict([{'value':'getinputs'}, {'vis':[]}, {'field':''}, {'intent':'PHASE'}, {'spw':''}])}

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
    def description(self, key='hifa_gaincalsnr', subkey=None):
        desc={'hifa_gaincalsnr': 'Compute gaincal signal to noise ratios per spw',
               'vis': 'List of input MeasurementSets',
               'field': 'Set of data selection field names',
               'intent': 'Set of data selection observing intents',
               'spw': 'Set of data selection spectral window ids',
               'phasesnr': 'The signal to noise minimum',
               'bwedgefrac': 'The fraction of the bandwidth edge that is flagged',
               'hm_nantennas': 'The antenna selection heuristic (unsupported) ',
               'maxfracflagged': 'The maximum fraction of data flagged per antenna (unsupported)',
               'pipelinemode': 'The pipeline operating mode',
               'dryrun': 'Run task (False) or display the command(True)',
               'acceptresults': 'Automatically accept results into the context',

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
        a['intent']  = 'PHASE'
        a['spw']  = ''
        a['phasesnr']  = 25.0
        a['bwedgefrac']  = 0.03125
        a['hm_nantennas']  = 'all'
        a['maxfracflagged']  = 0.90
        a['pipelinemode']  = 'automatic'
        a['dryrun']  = False
        a['acceptresults']  = True

        #a = sys._getframe(len(inspect.stack())-1).f_globals

        if self.parameters['hm_nantennas']  == 'unflagged':
            a['maxfracflagged'] = 0.90

        if self.parameters['pipelinemode']  == 'interactive':
            a['vis'] = []
            a['field'] = ''
            a['intent'] = 'PHASE'
            a['spw'] = ''
            a['dryrun'] = False
            a['acceptresults'] = True

        if self.parameters['pipelinemode']  == 'getinputs':
            a['vis'] = []
            a['field'] = ''
            a['intent'] = 'PHASE'
            a['spw'] = ''

        if a.has_key(paramname) :
	      return a[paramname]
hifa_gaincalsnr_cli = hifa_gaincalsnr_cli_()
