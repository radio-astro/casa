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
from task_hifa_session_bandpass import hifa_session_bandpass
class hifa_session_bandpass_cli_:
    __name__ = "hifa_session_bandpass"
    rkey = None
    i_am_a_casapy_task = None
    # The existence of the i_am_a_casapy_task attribute allows help()
    # (and other) to treat casapy tasks as a special case.

    def __init__(self) :
       self.__bases__ = (hifa_session_bandpass_cli_,)
       self.__doc__ = self.__call__.__doc__

       self.parameters={'vis':None, 'caltable':None, 'field':None, 'intent':None, 'spw':None, 'antenna':None, 'hm_phaseup':None, 'phaseupsolint':None, 'phaseupbw':None, 'phaseupsnr':None, 'phaseupnsols':None, 'hm_bandpass':None, 'solint':None, 'maxchannels':None, 'evenbpints':None, 'bpsnr':None, 'bpnsols':None, 'hm_bandtype':None, 'combine':None, 'refant':None, 'solnorm':None, 'minblperant':None, 'minsnr':None, 'degamp':None, 'degphase':None, 'pipelinemode':None, 'dryrun':None, 'acceptresults':None, 'parallel':None, }


    def result(self, key=None):
	    #### and add any that have completed...
	    return None


    def __call__(self, vis=None, caltable=None, field=None, intent=None, spw=None, antenna=None, hm_phaseup=None, phaseupsolint=None, phaseupbw=None, phaseupsnr=None, phaseupnsols=None, hm_bandpass=None, solint=None, maxchannels=None, evenbpints=None, bpsnr=None, bpnsols=None, hm_bandtype=None, combine=None, refant=None, solnorm=None, minblperant=None, minsnr=None, degamp=None, degphase=None, pipelinemode=None, dryrun=None, acceptresults=None, parallel=None, ):

        """Compute bandpass calibration solutions

	Detailed Description:

            Compute amplitude and phase as a function of frequency for each spectral
            window in each MeasurementSet.
        
	Arguments :
		vis:	List of input measurment sets
		   Default Value: 

		caltable:	List of output caltables
		   Default Value: 

		field:	Set of data selection field names or ids
		   Default Value: 

		intent:	Set of data selection intents
		   Default Value: 

		spw:	Set of data selection spectral window/channels
		   Default Value: 

		antenna:	Set of data selection antenna IDs
		   Default Value: 

		hm_phaseup:	Phaseup before computing the bandpass
		   Default Value: snr
		   Allowed Values:
				snr
				manual
				

		phaseupsolint:	Phaseup correction solution interval
		   Default Value: int

		phaseupbw:	Bandwidth to use for phaseup
		   Default Value: 

		phaseupsnr:	SNR for phaseup solution
		   Default Value: 20.0

		phaseupnsols:	Minimum number of phaseup gain solutions
		   Default Value: 2

		hm_bandpass:	Bandpass solution heuristics
		   Default Value: snr
		   Allowed Values:
				snr
				smoothed
				fixed

		solint:	Solution intervals
		   Default Value: inf

		maxchannels:	The smoothing factor in channels
		   Default Value: 240

		evenbpints:	Force frequency solint to even bandpass intervals
		   Default Value: True

		bpsnr:	SNR for bandpass solution
		   Default Value: 50.0

		bpnsols:	Minimum number of bandpass solutions
		   Default Value: 8

		hm_bandtype:	Bandpass solution type
		   Default Value: channel
		   Allowed Values:
				channel
				polynomial

		combine:	Data axes which to combine for solve (scan, spw, and/or field)
		   Default Value: scan

		refant:	Reference antenna names
		   Default Value: 

		solnorm:	Normalise the bandpass solution
		   Default Value: True

		minblperant:	Minimum baselines per antenna required for solve
		   Default Value: 4

		minsnr:	Reject solutions below this SNR
		   Default Value: 3.0

		degamp:	Degree for polynomial amplitude solution
		   Default Value: 

		degphase:	Degree for polynomial phase solution
		   Default Value: 

		pipelinemode:	The pipeline operating mode
		   Default Value: automatic
		   Allowed Values:
				automatic
				interactive
				getinputs

		dryrun:	Run the task (False) or display the command(True)
		   Default Value: False

		acceptresults:	Add the results to the pipeline context
		   Default Value: True

		parallel:	Execute using CASA HPC functionality, if available.
		   Default Value: automatic
		   Allowed Values:
				automatic
				true
				false

	Returns: void

	Example :

Compute  amplitude and phase as a function of frequency for each spectral
window in each MeasurementSet.

Previous calibration can be applied on the fly.

Keyword arguments:

--- pipeline parameter arguments which can be set in any pipeline mode

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context defined pipeline inputs automatically.
   In interactive mode the user can set the pipeline context defined parameters
   manually.  In 'getinputs' mode the user can check the settings of all
   pipeline parameters without running the task.
   default: 'automatic'.

hm_phaseup -- The pre-bandpass solution phaseup gain heuristics. The options
   are 'snr' (compute solution required to achieve the specified SNR),
   'manual' (use manual solution parameters), and '' (none).
   default: 'snr'
   example: hm_phaseup='manual'

phaseupsolint -- The phase correction solution interval in CASA syntax.
    Used when hm_phaseup='manual' or as a default if the hm_phasup='snr'
    heuristic computation fails.
    default: 'int'
    example: phaseupsolint='300s'

phaseupbw -- Bandwidth to be used for phaseup. Defaults to 500MHz.
    Used when hm_phaseup='manual'.
    default: ''
    example: '' default to entire bandpass, '500MHz' use centreal 500MHz

phaseupsnr -- The required SNR for the phaseup solution. Used only if
    hm_phaseup='snr'
    default: 20.0
    example: phaseupsnr=10.0

phaseupnsols -- The minimum number of phaseup gian solutions. Used only if
    hm_phaseup='snr'.
    default: 2
    example: phaseupnsols=4

hm_bandpass -- The bandpass solution heuristics. The options are 'snr'
    (compute the solution required to achieve the specified SNR),
    'smoothed' (simple smoothing heuristics), and 'fixed' (use
    the user defined parameters for all spws).

solint --  Time and channel solution intervals in CASA syntax.
    default: 'inf' Used for hm_bandpass='fixed', and as a default
    for the 'snr' and 'smoothed' options.
    default: 'inf,7.8125MHz'
    example: solint='inf,10ch', solint='inf'

maxchannels --  The bandpass solution smoothing factor in channels. The
    solution interval is bandwidth / 240. Set to 0 for no smoothing.
    Used if hm_bandpass='smoothed".
    default: 240
    example: 0

evenbpints -- Force the per spe frequency solint to be evenly divisible
    into the spw bandpass if hm_bandpass='snr'
    default: True
    example: evenbpints=False

bpsnr -- The required SNR for the bandpass solution. Used only if
    hm_bandpass='snr'
    default: 50.0
    example: bpsnr=20.0

bpnsols -- The minimum number of bandpass solutions. Used only if
    hm_bandpass='snr'.
    default: 8

hm_bandtype -- The type of bandpass. The options are 'channel' and
    'polynomial' for CASA bandpass types = 'B' and 'BPOLY' respectively.

combine -- Data axes to combine for solving. Axes are '', 'scan','spw','field'
    or any comma-separated combination.
    default; 'scan'
    example: combine='scan,field'

minblperant -- Minimum number of baselines required per antenna for each solve
    Antennas with fewer baselines are excluded from solutions. Used for
    hm_bandtype='channel' only.
    default: 4

minsnr -- Solutions below this SNR are rejected. Used for hm_bandtype=
    'channel' only
    default: 3.0


---- pipeline context defined parameter arguments which can be set only in
'interactive mode'

vis -- The list of input MeasurementSets. Defaults to the list of measurment
    sets specified in the pipeline context.
    default: ''
    example: ['M51.ms']

caltable -- The list of output calibration tables. Defaults to the standard
    pipeline naming convention.
    default: ''
    example: ['M51.bcal']

field -- The list of field names or field ids for which bandpasses are
    computed. Defaults to all fields.
    default: ''
    example: '3C279', '3C279, M82'

intent -- A string containing a comma delimited list of intents against
    which the the selected fields are matched.  Defaults to all data
    with bandpass intent.
    default: ''
    example: '*PHASE*'

spw -- The list of spectral windows and channels for which bandpasses are
    computed. Defaults to all science spectral windows.
    default: ''
    example: '11,13,15,17'

refant -- Reference antenna names. Defaults to the value(s) stored in the
    pipeline context. If undefined in the pipeline context defaults to
    the CASA reference antenna naming scheme.
    default: ''
    example: refant='DV01', refant='DV06,DV07'

solnorm -- Normalise the bandpass solutions
    default: False

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

Description

hif_bandpass computes a bandpass solution for every specified science
spectral window. By default a 'phaseup' pre-calibration is performed
and applied on the fly to to the data, before the bandpass is computed.

The hif_refant task may be used to precompute a prioritized list of
reference antennas.

Issues

There is currently some discussion about whether or not to do an 'ampup'
operations at the same time as the 'phaseup'. This is not required for the
bandpass computation but the amplitude information may provide a useful quality
assessment measure.

The specified minsnr parameter is currently applied to the bandpass
solution computation but not the 'phaseup' computation. Some noisy
solutions in the phaseup may not be properly rejected.

Examples

1. Compute a channel bandpass for all visibility files in the pipeline
context using the CASA reference antenna determination scheme.

    hif_bandpass()

2. Same as the above but precompute a prioritized reference antenna list

    hif_refant()
    hif_bandpass()
        
        """
	if not hasattr(self, "__globals__") or self.__globals__ == None :
           self.__globals__=stack_frame_find( )
	#casac = self.__globals__['casac']
	casalog = self.__globals__['casalog']
	casa = self.__globals__['casa']
	#casalog = casac.casac.logsink()
        self.__globals__['__last_task'] = 'hifa_session_bandpass'
        self.__globals__['taskname'] = 'hifa_session_bandpass'
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
            myparams['caltable'] = caltable = self.parameters['caltable']
            myparams['field'] = field = self.parameters['field']
            myparams['intent'] = intent = self.parameters['intent']
            myparams['spw'] = spw = self.parameters['spw']
            myparams['antenna'] = antenna = self.parameters['antenna']
            myparams['hm_phaseup'] = hm_phaseup = self.parameters['hm_phaseup']
            myparams['phaseupsolint'] = phaseupsolint = self.parameters['phaseupsolint']
            myparams['phaseupbw'] = phaseupbw = self.parameters['phaseupbw']
            myparams['phaseupsnr'] = phaseupsnr = self.parameters['phaseupsnr']
            myparams['phaseupnsols'] = phaseupnsols = self.parameters['phaseupnsols']
            myparams['hm_bandpass'] = hm_bandpass = self.parameters['hm_bandpass']
            myparams['solint'] = solint = self.parameters['solint']
            myparams['maxchannels'] = maxchannels = self.parameters['maxchannels']
            myparams['evenbpints'] = evenbpints = self.parameters['evenbpints']
            myparams['bpsnr'] = bpsnr = self.parameters['bpsnr']
            myparams['bpnsols'] = bpnsols = self.parameters['bpnsols']
            myparams['hm_bandtype'] = hm_bandtype = self.parameters['hm_bandtype']
            myparams['combine'] = combine = self.parameters['combine']
            myparams['refant'] = refant = self.parameters['refant']
            myparams['solnorm'] = solnorm = self.parameters['solnorm']
            myparams['minblperant'] = minblperant = self.parameters['minblperant']
            myparams['minsnr'] = minsnr = self.parameters['minsnr']
            myparams['degamp'] = degamp = self.parameters['degamp']
            myparams['degphase'] = degphase = self.parameters['degphase']
            myparams['pipelinemode'] = pipelinemode = self.parameters['pipelinemode']
            myparams['dryrun'] = dryrun = self.parameters['dryrun']
            myparams['acceptresults'] = acceptresults = self.parameters['acceptresults']
            myparams['parallel'] = parallel = self.parameters['parallel']

        if type(vis)==str: vis=[vis]
        if type(caltable)==str: caltable=[caltable]

	result = None

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['caltable'] = caltable
        mytmp['field'] = field
        mytmp['intent'] = intent
        mytmp['spw'] = spw
        mytmp['antenna'] = antenna
        mytmp['hm_phaseup'] = hm_phaseup
        mytmp['phaseupsolint'] = phaseupsolint
        mytmp['phaseupbw'] = phaseupbw
        mytmp['phaseupsnr'] = phaseupsnr
        mytmp['phaseupnsols'] = phaseupnsols
        mytmp['hm_bandpass'] = hm_bandpass
        mytmp['solint'] = solint
        mytmp['maxchannels'] = maxchannels
        mytmp['evenbpints'] = evenbpints
        mytmp['bpsnr'] = bpsnr
        mytmp['bpnsols'] = bpnsols
        mytmp['hm_bandtype'] = hm_bandtype
        mytmp['combine'] = combine
        mytmp['refant'] = refant
        mytmp['solnorm'] = solnorm
        mytmp['minblperant'] = minblperant
        mytmp['minsnr'] = minsnr
        mytmp['degamp'] = degamp
        mytmp['degphase'] = degphase
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
        mytmp['parallel'] = parallel
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli/"
	trec = casac.casac.utils().torecord(pathname+'hifa_session_bandpass.xml')

        casalog.origin('hifa_session_bandpass')
	try :
          #if not trec.has_key('hifa_session_bandpass') or not casac.casac.utils().verify(mytmp, trec['hifa_session_bandpass']) :
	    #return False

          casac.casac.utils().verify(mytmp, trec['hifa_session_bandpass'], True)
          scriptstr=['']
          saveinputs = self.__globals__['saveinputs']
          if type(self.__call__.func_defaults) is NoneType:
              saveinputs=''
          else:
              saveinputs('hifa_session_bandpass', 'hifa_session_bandpass.last', myparams, self.__globals__,scriptstr=scriptstr)
          tname = 'hifa_session_bandpass'
          spaces = ' '*(18-len(tname))
          casalog.post('\n##########################################'+
                       '\n##### Begin Task: ' + tname + spaces + ' #####')
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('Begin Task: ' + tname)
          if type(self.__call__.func_defaults) is NoneType:
              casalog.post(scriptstr[0]+'\n', 'INFO')
          else :
              casalog.post(scriptstr[1][1:]+'\n', 'INFO')
          result = hifa_session_bandpass(vis, caltable, field, intent, spw, antenna, hm_phaseup, phaseupsolint, phaseupbw, phaseupsnr, phaseupnsols, hm_bandpass, solint, maxchannels, evenbpints, bpsnr, bpnsols, hm_bandtype, combine, refant, solnorm, minblperant, minsnr, degamp, degphase, pipelinemode, dryrun, acceptresults, parallel)
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('End Task: ' + tname)
          casalog.post('##### End Task: ' + tname + '  ' + spaces + ' #####'+
                       '\n##########################################')

	except Exception, instance:
          if(self.__globals__.has_key('__rethrow_casa_exceptions') and self.__globals__['__rethrow_casa_exceptions']) :
             raise
          else :
             #print '**** Error **** ',instance
	     tname = 'hifa_session_bandpass'
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
#        paramgui.runTask('hifa_session_bandpass', myf['_ip'])
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
        a['hm_phaseup']  = 'snr'
        a['hm_bandpass']  = 'snr'
        a['hm_bandtype']  = 'channel'
        a['combine']  = 'scan'
        a['solnorm']  = True
        a['pipelinemode']  = 'automatic'

        a['hm_phaseup'] = {
                    0:odict([{'value':'snr'}, {'phaseupsolint':'int'}, {'phaseupsnr':20.0}, {'phaseupnsols':2}]), 
                    1:odict([{'value':'manual'}, {'phaseupsolint':'int'}, {'phaseupbw':''}]), 
                    2:{'value':''}}
        a['hm_bandpass'] = {
                    0:odict([{'value':'snr'}, {'solint':'inf'}, {'evenbpints':True}, {'bpsnr':50.0}, {'bpnsols':8}]), 
                    1:odict([{'value':'smoothed'}, {'solint':'inf'}, {'maxchannels':240}]), 
                    2:odict([{'value':'fixed'}, {'solint':'inf,7.8125MHz'}])}
        a['hm_bandtype'] = {
                    0:odict([{'value':'channel'}, {'minblperant':4}, {'minsnr':3.0}]), 
                    1:odict([{'value':'polynomial'}, {'degamp':''}, {'degphase':''}])}
        a['pipelinemode'] = {
                    0:{'value':'automatic'}, 
                    1:odict([{'value':'interactive'}, {'vis':[]}, {'caltable':[]}, {'field':''}, {'intent':''}, {'spw':''}, {'antenna':''}, {'refant':''}, {'solnorm':True}, {'dryrun':False}, {'acceptresults':True}, {'parallel':'automatic'}]), 
                    2:odict([{'value':'getinputs'}, {'vis':[]}, {'caltable':[]}, {'field':''}, {'intent':''}, {'spw':''}, {'antenna':''}, {'refant':''}, {'solnorm':True}])}

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
    def description(self, key='hifa_session_bandpass', subkey=None):
        desc={'hifa_session_bandpass': 'Compute bandpass calibration solutions',
               'vis': 'List of input measurment sets',
               'caltable': 'List of output caltables',
               'field': 'Set of data selection field names or ids',
               'intent': 'Set of data selection intents',
               'spw': 'Set of data selection spectral window/channels',
               'antenna': 'Set of data selection antenna IDs',
               'hm_phaseup': 'Phaseup before computing the bandpass',
               'phaseupsolint': 'Phaseup correction solution interval',
               'phaseupbw': 'Bandwidth to use for phaseup',
               'phaseupsnr': 'SNR for phaseup solution',
               'phaseupnsols': 'Minimum number of phaseup gain solutions',
               'hm_bandpass': 'Bandpass solution heuristics',
               'solint': 'Solution intervals',
               'maxchannels': 'The smoothing factor in channels',
               'evenbpints': 'Force frequency solint to even bandpass intervals',
               'bpsnr': 'SNR for bandpass solution',
               'bpnsols': 'Minimum number of bandpass solutions',
               'hm_bandtype': 'Bandpass solution type',
               'combine': 'Data axes which to combine for solve (scan, spw, and/or field)',
               'refant': 'Reference antenna names',
               'solnorm': 'Normalise the bandpass solution',
               'minblperant': 'Minimum baselines per antenna required for solve',
               'minsnr': 'Reject solutions below this SNR',
               'degamp': 'Degree for polynomial amplitude solution',
               'degphase': 'Degree for polynomial phase solution',
               'pipelinemode': 'The pipeline operating mode',
               'dryrun': 'Run the task (False) or display the command(True)',
               'acceptresults': 'Add the results to the pipeline context',
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
        a['caltable']  = ['']
        a['field']  = ''
        a['intent']  = ''
        a['spw']  = ''
        a['antenna']  = ''
        a['hm_phaseup']  = 'snr'
        a['phaseupsolint']  = 'int'
        a['phaseupbw']  = ''
        a['phaseupsnr']  = 20.0
        a['phaseupnsols']  = 2
        a['hm_bandpass']  = 'snr'
        a['solint']  = 'inf'
        a['maxchannels']  = 240
        a['evenbpints']  = True
        a['bpsnr']  = 50.0
        a['bpnsols']  = 8
        a['hm_bandtype']  = 'channel'
        a['combine']  = 'scan'
        a['refant']  = ''
        a['solnorm']  = True
        a['minblperant']  = 4
        a['minsnr']  = 3.0
        a['degamp']  = ''
        a['degphase']  = ''
        a['pipelinemode']  = 'automatic'
        a['dryrun']  = False
        a['acceptresults']  = True
        a['parallel']  = 'automatic'

        #a = sys._getframe(len(inspect.stack())-1).f_globals

        if self.parameters['hm_phaseup']  == 'snr':
            a['phaseupsolint'] = 'int'
            a['phaseupsnr'] = 20.0
            a['phaseupnsols'] = 2

        if self.parameters['hm_phaseup']  == 'manual':
            a['phaseupsolint'] = 'int'
            a['phaseupbw'] = ''

        if self.parameters['hm_bandpass']  == 'snr':
            a['solint'] = 'inf'
            a['evenbpints'] = True
            a['bpsnr'] = 50.0
            a['bpnsols'] = 8

        if self.parameters['hm_bandpass']  == 'smoothed':
            a['solint'] = 'inf'
            a['maxchannels'] = 240

        if self.parameters['hm_bandpass']  == 'fixed':
            a['solint'] = 'inf,7.8125MHz'

        if self.parameters['hm_bandtype']  == 'channel':
            a['minblperant'] = 4
            a['minsnr'] = 3.0

        if self.parameters['hm_bandtype']  == 'polynomial':
            a['degamp'] = ''
            a['degphase'] = ''

        if self.parameters['pipelinemode']  == 'interactive':
            a['vis'] = []
            a['caltable'] = []
            a['field'] = ''
            a['intent'] = ''
            a['spw'] = ''
            a['antenna'] = ''
            a['refant'] = ''
            a['solnorm'] = True
            a['dryrun'] = False
            a['acceptresults'] = True
            a['parallel'] = 'automatic'

        if self.parameters['pipelinemode']  == 'getinputs':
            a['vis'] = []
            a['caltable'] = []
            a['field'] = ''
            a['intent'] = ''
            a['spw'] = ''
            a['antenna'] = ''
            a['refant'] = ''
            a['solnorm'] = True

        if a.has_key(paramname) :
	      return a[paramname]
hifa_session_bandpass_cli = hifa_session_bandpass_cli_()
