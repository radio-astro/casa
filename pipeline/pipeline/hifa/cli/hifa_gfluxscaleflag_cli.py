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
from task_hifa_gfluxscaleflag import hifa_gfluxscaleflag
class hifa_gfluxscaleflag_cli_:
    __name__ = "hifa_gfluxscaleflag"
    rkey = None
    i_am_a_casapy_task = None
    # The existence of the i_am_a_casapy_task attribute allows help()
    # (and other) to treat casapy tasks as a special case.

    def __init__(self) :
       self.__bases__ = (hifa_gfluxscaleflag_cli_,)
       self.__doc__ = self.__call__.__doc__

       self.parameters={'vis':None, 'phaseupsolint':None, 'solint':None, 'minsnr':None, 'refant':None, 'antnegsig':None, 'antpossig':None, 'tmantint':None, 'tmint':None, 'tmbl':None, 'antblnegsig':None, 'antblpossig':None, 'relaxed_factor':None, 'niter':None, 'pipelinemode':None, 'dryrun':None, 'acceptresults':None, }


    def result(self, key=None):
	    #### and add any that have completed...
	    return None


    def __call__(self, vis=None, phaseupsolint=None, solint=None, minsnr=None, refant=None, antnegsig=None, antpossig=None, tmantint=None, tmint=None, tmbl=None, antblnegsig=None, antblpossig=None, relaxed_factor=None, niter=None, pipelinemode=None, dryrun=None, acceptresults=None, ):

        """Derive the flux density scale with flagging

	Detailed Description:

    This task computes the flagging heuristics on the phase calibrator and flux
    calibrator by calling hif_correctedampflag which looks for outlier
    visibility points by statistically examining the scalar difference of
    corrected amplitudes minus model amplitudes, and flags those outliers. The
    philosophy is that only outlier data points that have remained outliers
    after calibration will be flagged. The heuristic works equally well on
    resolved calibrators and point sources because it is not performing a
    vector difference, and thus is not sensitive to nulls in the flux density
    vs. uvdistance domain. Note that the phase of the data is not assessed.

    In further detail, the workflow is as follows: an a priori calibration is
    applied using pre-existing caltables in the calibration state, a
    preliminary phase and amplitude gaincal solution is solved and applied, the
    flagging heuristics are run, and any outliers are flagged. Plots are
    generated at two points in this workflow: after preliminary phase and
    amplitude calibration but before flagging heuristics are run, and after
    flagging heuristics have been run and applied. If no points were flagged,
    the "after" plots are not generated or displayed. The score for this stage
    is the standard data flagging score, which depends on the fraction of data
    flagged.

	Arguments :
		vis:	List of input MeasurementSets
		   Default Value: 

		phaseupsolint:	Phaseup correction solution interval
		   Default Value: int

		solint:	Solution intervals
		   Default Value: inf

		minsnr:	Reject solutions below this SNR
		   Default Value: 2.0

		refant:	Reference antenna names
		   Default Value: 

		antnegsig:	Lower sigma threshold for identifying outliers as a result of bad antennas within individual timestamps
		   Default Value: 4.0

		antpossig:	Upper sigma threshold for identifying outliers as a result of bad antennas within individual timestamps
		   Default Value: 4.6

		tmantint:	Threshold for maximum fraction of timestamps that are allowed to contain outliers
		   Default Value: 0.063

		tmint:	Initial threshold for maximum fraction of "outlier timestamps" over "total timestamps" that a baseline may be a part of
		   Default Value: 0.085

		tmbl:	Initial threshold for maximum fraction of "bad baselines" over "all baselines" that an antenna may be a part of
		   Default Value: 0.175

		antblnegsig:	Lower sigma threshold for identifying outliers as a result of "bad baselines" and/or "bad antennas" within baselines (across all timestamps)
		   Default Value: 3.4

		antblpossig:	Upper sigma threshold for identifying outliers as a result of "bad baselines" and/or "bad antennas" within baselines (across all timestamps)
		   Default Value: 3.2

		relaxed_factor:	Relaxed value to set the threshold scaling factor to under certain conditions (see task description)
		   Default Value: 2.0

		niter:	Maximum number of times to iterate on evaluation of flagging heuristics.
		   Default Value: 2

		pipelinemode:	The pipeline operations mode
		   Default Value: automatic
		   Allowed Values:
				automatic
				interactive
				getinputs

		dryrun:	Run the task (False) or list commands(True)
		   Default Value: False

		acceptresults:	Automatically apply results to context
		   Default Value: True


	Example :


Keyword arguments:

--- pipeline parameter arguments which can be set in any pipeline mode

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context defined pipeline inputs automatically.
   In interactive mode the user can set the pipeline context defined parameters
   manually.  In 'getinputs' mode the user can check the settings of all
   pipeline parameters without running the task.
   default: 'automatic'.

---- pipeline context defined parameter arguments which can be set only in
'interactive mode'

vis -- The list of input MeasurementSets. Defaults to the list of MeasurementSets
    specified in the pipeline context.
    default: ''
    example: ['M51.ms']

phaseupsolint -- The phase correction solution interval in CASA syntax.
    default: 'int'
    example: phaseupsolint='300s'

solint --  Time and channel solution intervals in CASA syntax.
    default: 'inf'
    example: solint='inf,10ch', solint='inf'

minsnr -- Solutions below this SNR are rejected.
    default: 2.0

refant -- Reference antenna names. Defaults to the value(s) stored in the
    pipeline context. If undefined in the pipeline context defaults to
    the CASA reference antenna naming scheme.
    default: ''
    example: refant='DV01', refant='DV06,DV07'

antnegsig -- Lower sigma threshold for identifying outliers as a result of bad
    antennas within individual timestamps.
    default: 4.0

antpossig -- Upper sigma threshold for identifying outliers as a result of bad
    antennas within individual timestamps.
    default: 4.6

tmantint -- Threshold for maximum fraction of timestamps that are allowed to
    contain outliers.
    default: 0.063

tmint -- Initial threshold for maximum fraction of "outlier timestamps" over
    "total timestamps" that a baseline may be a part of.
    default: 0.085

tmbl -- Initial threshold for maximum fraction of "bad baselines" over "all
    baselines" that an antenna may be a part of.
    default: 0.175

antblnegsig -- Lower sigma threshold for identifying outliers as a result of
    "bad baselines" and/or "bad antennas" within baselines, across all
    timestamps.
    default: 3.4

antblpossig -- Upper sigma threshold for identifying outliers as a result of
    "bad baselines" and/or "bad antennas" within baselines, across all
    timestamps.
    default: 3.2

relaxed_factor -- Relaxed value to set the threshold scaling factor to under
    certain conditions (see task description).
    default: 2.0

niter -- Maximum number of times to iterate on evaluation of flagging
    heuristics. If an iteration results in no new flags, then subsequent
    iterations are skipped.
    default: 2

--- pipeline task execution modes
dryrun -- Run the commands (True) or generate the commands to be run but
   do not execute (False).
   default: False

acceptresults -- Add the results of the task to the pipeline context (True) or
   reject them (False).
   default: True


Output:

results -- If pipeline mode is 'getinputs' then None is returned. Otherwise
    the results object for the pipeline task is returned.


Examples:

1. run with recommended settings to create flux scale calibration with flagging
    using recommended thresholds:

    hifa_gfluxscaleflag()


        """
	if not hasattr(self, "__globals__") or self.__globals__ == None :
           self.__globals__=stack_frame_find( )
	#casac = self.__globals__['casac']
	casalog = self.__globals__['casalog']
	casa = self.__globals__['casa']
	#casalog = casac.casac.logsink()
        self.__globals__['__last_task'] = 'hifa_gfluxscaleflag'
        self.__globals__['taskname'] = 'hifa_gfluxscaleflag'
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
            myparams['phaseupsolint'] = phaseupsolint = self.parameters['phaseupsolint']
            myparams['solint'] = solint = self.parameters['solint']
            myparams['minsnr'] = minsnr = self.parameters['minsnr']
            myparams['refant'] = refant = self.parameters['refant']
            myparams['antnegsig'] = antnegsig = self.parameters['antnegsig']
            myparams['antpossig'] = antpossig = self.parameters['antpossig']
            myparams['tmantint'] = tmantint = self.parameters['tmantint']
            myparams['tmint'] = tmint = self.parameters['tmint']
            myparams['tmbl'] = tmbl = self.parameters['tmbl']
            myparams['antblnegsig'] = antblnegsig = self.parameters['antblnegsig']
            myparams['antblpossig'] = antblpossig = self.parameters['antblpossig']
            myparams['relaxed_factor'] = relaxed_factor = self.parameters['relaxed_factor']
            myparams['niter'] = niter = self.parameters['niter']
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
        mytmp['phaseupsolint'] = phaseupsolint
        mytmp['solint'] = solint
        mytmp['minsnr'] = minsnr
        mytmp['refant'] = refant
        mytmp['antnegsig'] = antnegsig
        mytmp['antpossig'] = antpossig
        mytmp['tmantint'] = tmantint
        mytmp['tmint'] = tmint
        mytmp['tmbl'] = tmbl
        mytmp['antblnegsig'] = antblnegsig
        mytmp['antblpossig'] = antblpossig
        mytmp['relaxed_factor'] = relaxed_factor
        mytmp['niter'] = niter
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli/"
	trec = casac.casac.utils().torecord(pathname+'hifa_gfluxscaleflag.xml')

        casalog.origin('hifa_gfluxscaleflag')
	try :
          #if not trec.has_key('hifa_gfluxscaleflag') or not casac.casac.utils().verify(mytmp, trec['hifa_gfluxscaleflag']) :
	    #return False

          casac.casac.utils().verify(mytmp, trec['hifa_gfluxscaleflag'], True)
          scriptstr=['']
          saveinputs = self.__globals__['saveinputs']
          if type(self.__call__.func_defaults) is NoneType:
              saveinputs=''
          else:
              saveinputs('hifa_gfluxscaleflag', 'hifa_gfluxscaleflag.last', myparams, self.__globals__,scriptstr=scriptstr)
          tname = 'hifa_gfluxscaleflag'
          spaces = ' '*(18-len(tname))
          casalog.post('\n##########################################'+
                       '\n##### Begin Task: ' + tname + spaces + ' #####')
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('Begin Task: ' + tname)
          if type(self.__call__.func_defaults) is NoneType:
              casalog.post(scriptstr[0]+'\n', 'INFO')
          else :
              casalog.post(scriptstr[1][1:]+'\n', 'INFO')
          result = hifa_gfluxscaleflag(vis, phaseupsolint, solint, minsnr, refant, antnegsig, antpossig, tmantint, tmint, tmbl, antblnegsig, antblpossig, relaxed_factor, niter, pipelinemode, dryrun, acceptresults)
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('End Task: ' + tname)
          casalog.post('##### End Task: ' + tname + '  ' + spaces + ' #####'+
                       '\n##########################################')

	except Exception, instance:
          if(self.__globals__.has_key('__rethrow_casa_exceptions') and self.__globals__['__rethrow_casa_exceptions']) :
             raise
          else :
             #print '**** Error **** ',instance
	     tname = 'hifa_gfluxscaleflag'
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
#        paramgui.runTask('hifa_gfluxscaleflag', myf['_ip'])
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
        a['pipelinemode']  = 'automatic'

        a['pipelinemode'] = {
                    0:{'value':'automatic'}, 
                    1:odict([{'value':'interactive'}, {'vis':[]}, {'phaseupsolint':'int'}, {'solint':'inf'}, {'minsnr':2.0}, {'refant':''}, {'antnegsig':4.0}, {'antpossig':4.6}, {'tmantint':0.063}, {'tmint':0.085}, {'tmbl':0.175}, {'antblnegsig':3.4}, {'antblpossig':3.2}, {'relaxed_factor':2.0}, {'niter':2}, {'dryrun':False}, {'acceptresults':True}]), 
                    2:odict([{'value':'getinputs'}, {'vis':[]}])}

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
    def description(self, key='hifa_gfluxscaleflag', subkey=None):
        desc={'hifa_gfluxscaleflag': 'Derive the flux density scale with flagging',
               'vis': 'List of input MeasurementSets',
               'phaseupsolint': 'Phaseup correction solution interval',
               'solint': 'Solution intervals',
               'minsnr': 'Reject solutions below this SNR',
               'refant': 'Reference antenna names',
               'antnegsig': 'Lower sigma threshold for identifying outliers as a result of bad antennas within individual timestamps',
               'antpossig': 'Upper sigma threshold for identifying outliers as a result of bad antennas within individual timestamps',
               'tmantint': 'Threshold for maximum fraction of timestamps that are allowed to contain outliers',
               'tmint': 'Initial threshold for maximum fraction of "outlier timestamps" over "total timestamps" that a baseline may be a part of',
               'tmbl': 'Initial threshold for maximum fraction of "bad baselines" over "all baselines" that an antenna may be a part of',
               'antblnegsig': 'Lower sigma threshold for identifying outliers as a result of "bad baselines" and/or "bad antennas" within baselines (across all timestamps)',
               'antblpossig': 'Upper sigma threshold for identifying outliers as a result of "bad baselines" and/or "bad antennas" within baselines (across all timestamps)',
               'relaxed_factor': 'Relaxed value to set the threshold scaling factor to under certain conditions (see task description)',
               'niter': 'Maximum number of times to iterate on evaluation of flagging heuristics.',
               'pipelinemode': 'The pipeline operations mode',
               'dryrun': 'Run the task (False) or list commands(True)',
               'acceptresults': 'Automatically apply results to context',

              }

#
# Set subfields defaults if needed
#

        if(desc.has_key(key)) :
           return desc[key]

    def itsdefault(self, paramname) :
        a = {}
        a['vis']  = ['']
        a['phaseupsolint']  = 'int'
        a['solint']  = 'inf'
        a['minsnr']  = 2.0
        a['refant']  = ''
        a['antnegsig']  = 4.0
        a['antpossig']  = 4.6
        a['tmantint']  = 0.063
        a['tmint']  = 0.085
        a['tmbl']  = 0.175
        a['antblnegsig']  = 3.4
        a['antblpossig']  = 3.2
        a['relaxed_factor']  = 2.0
        a['niter']  = 2
        a['pipelinemode']  = 'automatic'
        a['dryrun']  = False
        a['acceptresults']  = True

        #a = sys._getframe(len(inspect.stack())-1).f_globals

        if self.parameters['pipelinemode']  == 'interactive':
            a['vis'] = []
            a['phaseupsolint'] = 'int'
            a['solint'] = 'inf'
            a['minsnr'] = 2.0
            a['refant'] = ''
            a['antnegsig'] = 4.0
            a['antpossig'] = 4.6
            a['tmantint'] = 0.063
            a['tmint'] = 0.085
            a['tmbl'] = 0.175
            a['antblnegsig'] = 3.4
            a['antblpossig'] = 3.2
            a['relaxed_factor'] = 2.0
            a['niter'] = 2
            a['dryrun'] = False
            a['acceptresults'] = True

        if self.parameters['pipelinemode']  == 'getinputs':
            a['vis'] = []

        if a.has_key(paramname) :
	      return a[paramname]
hifa_gfluxscaleflag_cli = hifa_gfluxscaleflag_cli_()
