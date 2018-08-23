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
from task_hifa_wvrgcal import hifa_wvrgcal
class hifa_wvrgcal_cli_:
    __name__ = "hifa_wvrgcal"
    rkey = None
    i_am_a_casapy_task = None
    # The existence of the i_am_a_casapy_task attribute allows help()
    # (and other) to treat casapy tasks as a special case.

    def __init__(self) :
       self.__bases__ = (hifa_wvrgcal_cli_,)
       self.__doc__ = self.__call__.__doc__

       self.parameters={'vis':None, 'caltable':None, 'offsetstable':None, 'hm_toffset':None, 'toffset':None, 'segsource':None, 'sourceflag':None, 'hm_tie':None, 'tie':None, 'nsol':None, 'disperse':None, 'wvrflag':None, 'hm_smooth':None, 'smooth':None, 'scale':None, 'maxdistm':None, 'minnumants':None, 'mingoodfrac':None, 'refant':None, 'qa_intent':None, 'qa_bandpass_intent':None, 'qa_spw':None, 'accept_threshold':None, 'pipelinemode':None, 'dryrun':None, 'acceptresults':None, }


    def result(self, key=None):
	    #### and add any that have completed...
	    return None


    def __call__(self, vis=None, caltable=None, offsetstable=None, hm_toffset=None, toffset=None, segsource=None, sourceflag=None, hm_tie=None, tie=None, nsol=None, disperse=None, wvrflag=None, hm_smooth=None, smooth=None, scale=None, maxdistm=None, minnumants=None, mingoodfrac=None, refant=None, qa_intent=None, qa_bandpass_intent=None, qa_spw=None, accept_threshold=None, pipelinemode=None, dryrun=None, acceptresults=None, ):

        """

	Detailed Description:

Generate a gain table based on Water Vapour Radiometer data, and calculate 
a QA score based on its effect on the interferometric data.

	Arguments :
		vis:	List of input visibility files
		   Default Value: 

		caltable:	List of output gain calibration tables
		   Default Value: 

		offsetstable:	List of input temperature offsets table files
		   Default Value: 

		hm_toffset:	Toffset computation heuristic method
		   Default Value: automatic
		   Allowed Values:
				manual
				automatic

		toffset:	Time offset (sec) between IF and WVR data
		   Default Value: 0

		segsource:	Compute new coefficient calculation for each source
		   Default Value: True

		sourceflag:	Flag the WVR data for these source(s)
		   Default Value: 

		hm_tie:	Tie computation heuristics method
		   Default Value: automatic
		   Allowed Values:
				manual
				automatic

		tie:	Sources for which to use the same atmospheric phase correction coefficients
		   Default Value: 

		nsol:	Number of solutions for phase correction coefficients
		   Default Value: 1

		disperse:	Apply correction for dispersion
		   Default Value: False

		wvrflag:	Flag the WVR data for these antenna(s) replace with interpolated values
		   Default Value: 

		hm_smooth:	Smoothing computation heuristics method
		   Default Value: automatic
		   Allowed Values:
				manual
				automatic

		smooth:	Smooth WVR data on the given timescale before calculating the correction
		   Default Value: 

		scale:	Scale the entire phase correction by this factor
		   Default Value: 1.

		maxdistm:	Maximum distance (m) of an antenna used for interpolation for a flagged antenna.
		   Default Value: -1

		minnumants:	Minimum number of near antennas (up to 3) required for interpolation
		   Default Value: 2
		   Allowed Values:
				1
				2
				3

		mingoodfrac:	Minimum fraction of good data per antenna
		   Default Value: 0.8

		refant:	Ranked list of reference antennas
		   Default Value: 

		qa_intent:	Data intents to use in estimating the effectiveness of the wvr correction
		   Default Value: 

		qa_bandpass_intent:	Data intent to use for the bandpass calibration in the qa calculation
		   Default Value: 

		qa_spw:	Data SpW(s) to use in estimating the effectiveness of the wvr correction
		   Default Value: 

		accept_threshold:	Improvement ratio (phase-rms without wvr / phase-rms with wvr) above which wvrg calibration file will be accepted
		   Default Value: 1.0

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

	Returns: void

	Example :


Generate a gain table based on the Water Vapour Radiometer data in each vis
file. By applying the wvr calibration to the data specified by 'qa_intent' and
'qa_spw', calculate a QA score to indicate its effect on interferometric data;
a score > 1 implies that the phase noise is improved, a score < 1 implies
that it is made worse. If the score is less than 'accept_threshold' then the 
wvr gain table is not accepted into the context for subsequent use.
  
vis -- List of input visibility files
    default: none, in which case the vis files to be used will be read
             from the context. 
    example: vis=['ngc5921.ms']

caltable -- List of output gain calibration tables
    default: none, in which case the names of the caltables will be 
             generated automatically.
    example: caltable='ngc5921.wvr'

offsetstable -- List of input temperature offsets table files to subtract from
                WVR measurements before calculating phase corrections.
    default: none, in which case no offsets are applied.
    example: offsetstable=['ngc5921.cloud_offsets']

hm_toffset -- If 'manual', set the 'toffset' parameter to the user-specified value.
          If 'automatic', set the 'toffset' parameter according to the 
          date of the MeasurementSet; toffset=-1 if before 2013-01-21T00:00:00
          toffset=0 otherwise.
    default: 'automatic'

toffset -- Time offset (sec) between interferometric and WVR data
    default: 0

segsource -- If True calculate new atmospheric phase correction 
             coefficients for each source, subject to the constraints of 
             the 'tie' parameter. 'segsource' is forced to be True if 
             the 'tie' parameter is set to a non-empty value by the 
             user or by the automatic heuristic.
    default: True

hm_tie -- If 'manual', set the 'tie' parameter to the user-specified value.
          If 'automatic', set the 'tie' parameter to include with the
          target all calibrators that are within 15 degrees of it: 
          if no calibrators are that close then 'tie' is left empty.
    default: 'automatic'

tie -- Use the same atmospheric phase correction coefficients when 
       calculating the wvr correction for all sources in the 'tie'. If 'tie' 
       is not empty then 'segsource' is forced to be True. Ignored unless
       hm_tie='manual'.
    default: []
    example: ['3C273,NGC253', 'IC433,3C279']

sourceflag -- Flag the WVR data for these source(s) as bad and do not produce
    corrections for it. Requires segsource=True.
    default: []
    example: ['3C273']

nsol -- Number of solutions for phase correction coefficients during this
    observation, evenly distributed in time throughout the observation. It
    is used only if segsource=False because if segsource=True then the
    coefficients are recomputed whenever the telescope moves to a new source
    (within the limits imposed by 'tie').
    default: 1

disperse -- Apply correction for dispersion
    default: False

wvrflag -- Flag the WVR data for the listed antennas as bad and replace 
    their data with values interpolated from the 3 nearest antennas with
    unflagged data.
    default: []
    example: ['DV03','DA05','PM02']           

hm_smooth -- If 'manual' set the 'smooth' parameter to the user-specified value.
    If 'automatic', run the wvrgcal task with the range of 'smooth' parameters
    required to match the integration time of the wvr data to that of the
    interferometric data in each spectral window.

smooth -- Smooth WVR data on this timescale before calculating the correction.
    Ignored unless hm_smooth='manual'.
    default: ''

scale -- Scale the entire phase correction by this factor.
    default: 1

maxdistm -- Maximum distance in meters of an antenna used for interpolation
    from a flagged antenna.

    default: -1  (automatically set to 100m if >50% of antennas are 7m
        antennas without WVR and otherwise set to 500m)

    example: 550

minnumants -- Minimum number of nearby antennas (up to 3) used for
    interpolation from a flagged antenna.
    default: 2
    example: 3

mingoodfrac -- Minimum fraction of good data per  antenna
    default: 0.8

refant -- Ranked comma delimited list of reference antennas.
    default: ''
    example 'DV01,DV02'

qa_intent -- The list of data intents on which the wvr correction is to be 
    tried as a means of estimating its effectiveness.

    A QA 'view' will be calculated for each specified intent, in each spectral
    window in each vis file.

    Each QA 'view' will consist of a pair of 2-d images with dimensions
    ['ANTENNA', 'TIME'], one showing the data phase-noise before the 
    wvr application, the second showing the phase noise after (both 'before'
    and 'after' images have a bandpass calibration applied as well).

    An overall QA score is calculated for each vis file, by dividing the
    'before' images by the 'after' and taking the median of the result. An
    overall score of 1 would correspond to no change in the phase noise,
    a score > 1 implies an improvement.

    If the overall score for a vis file is less than the value in          
    'accept_threshold' then the wvr calibration file is not made available 
    for merging into the context for use in the subsequent reduction.

    If you do not want any QA calculations then set qa_intent=''.

    default: ''
    example: 'PHASE'

qa_bandpass_intent -- The data intent to use for the bandpass calibration
    in the qa calculation. The default is blank to allow the 
    underlying bandpass task to select a sensible intent if the dataset
    lacks BANDPASS data.

    default: ''

qa_spw -- The SpW(s) to use for the qa calculation, in the order
    that they should be tried. Input as a comma-separated list.
    The default is blank, in which case the task will try SpWs
    in order of decreasing median sky opacity.

    default: ''

accept_threshold -- The phase-rms improvement ratio
    (rms without wvr / rms with wvr) above which the wrvg file will be
    accepted into the context for subsequent application.

    default: 1.0


Example

1. Compute the WVR calibration for all the MeasurementSets.

    hifa_wvrgcal (hm_tie='automatic')


        """
	if not hasattr(self, "__globals__") or self.__globals__ == None :
           self.__globals__=stack_frame_find( )
	#casac = self.__globals__['casac']
	casalog = self.__globals__['casalog']
	casa = self.__globals__['casa']
	#casalog = casac.casac.logsink()
        self.__globals__['__last_task'] = 'hifa_wvrgcal'
        self.__globals__['taskname'] = 'hifa_wvrgcal'
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
            myparams['offsetstable'] = offsetstable = self.parameters['offsetstable']
            myparams['hm_toffset'] = hm_toffset = self.parameters['hm_toffset']
            myparams['toffset'] = toffset = self.parameters['toffset']
            myparams['segsource'] = segsource = self.parameters['segsource']
            myparams['sourceflag'] = sourceflag = self.parameters['sourceflag']
            myparams['hm_tie'] = hm_tie = self.parameters['hm_tie']
            myparams['tie'] = tie = self.parameters['tie']
            myparams['nsol'] = nsol = self.parameters['nsol']
            myparams['disperse'] = disperse = self.parameters['disperse']
            myparams['wvrflag'] = wvrflag = self.parameters['wvrflag']
            myparams['hm_smooth'] = hm_smooth = self.parameters['hm_smooth']
            myparams['smooth'] = smooth = self.parameters['smooth']
            myparams['scale'] = scale = self.parameters['scale']
            myparams['maxdistm'] = maxdistm = self.parameters['maxdistm']
            myparams['minnumants'] = minnumants = self.parameters['minnumants']
            myparams['mingoodfrac'] = mingoodfrac = self.parameters['mingoodfrac']
            myparams['refant'] = refant = self.parameters['refant']
            myparams['qa_intent'] = qa_intent = self.parameters['qa_intent']
            myparams['qa_bandpass_intent'] = qa_bandpass_intent = self.parameters['qa_bandpass_intent']
            myparams['qa_spw'] = qa_spw = self.parameters['qa_spw']
            myparams['accept_threshold'] = accept_threshold = self.parameters['accept_threshold']
            myparams['pipelinemode'] = pipelinemode = self.parameters['pipelinemode']
            myparams['dryrun'] = dryrun = self.parameters['dryrun']
            myparams['acceptresults'] = acceptresults = self.parameters['acceptresults']

        if type(vis)==str: vis=[vis]
        if type(caltable)==str: caltable=[caltable]
        if type(offsetstable)==str: offsetstable=[offsetstable]
        if type(sourceflag)==str: sourceflag=[sourceflag]
        if type(tie)==str: tie=[tie]
        if type(wvrflag)==str: wvrflag=[wvrflag]

	result = None

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['caltable'] = caltable
        mytmp['offsetstable'] = offsetstable
        mytmp['hm_toffset'] = hm_toffset
        mytmp['toffset'] = toffset
        mytmp['segsource'] = segsource
        mytmp['sourceflag'] = sourceflag
        mytmp['hm_tie'] = hm_tie
        mytmp['tie'] = tie
        mytmp['nsol'] = nsol
        mytmp['disperse'] = disperse
        mytmp['wvrflag'] = wvrflag
        mytmp['hm_smooth'] = hm_smooth
        mytmp['smooth'] = smooth
        mytmp['scale'] = scale
        mytmp['maxdistm'] = maxdistm
        mytmp['minnumants'] = minnumants
        mytmp['mingoodfrac'] = mingoodfrac
        mytmp['refant'] = refant
        mytmp['qa_intent'] = qa_intent
        mytmp['qa_bandpass_intent'] = qa_bandpass_intent
        mytmp['qa_spw'] = qa_spw
        mytmp['accept_threshold'] = accept_threshold
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli/"
	trec = casac.casac.utils().torecord(pathname+'hifa_wvrgcal.xml')

        casalog.origin('hifa_wvrgcal')
	try :
          #if not trec.has_key('hifa_wvrgcal') or not casac.casac.utils().verify(mytmp, trec['hifa_wvrgcal']) :
	    #return False

          casac.casac.utils().verify(mytmp, trec['hifa_wvrgcal'], True)
          scriptstr=['']
          saveinputs = self.__globals__['saveinputs']
          if type(self.__call__.func_defaults) is NoneType:
              saveinputs=''
          else:
              saveinputs('hifa_wvrgcal', 'hifa_wvrgcal.last', myparams, self.__globals__,scriptstr=scriptstr)
          tname = 'hifa_wvrgcal'
          spaces = ' '*(18-len(tname))
          casalog.post('\n##########################################'+
                       '\n##### Begin Task: ' + tname + spaces + ' #####')
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('Begin Task: ' + tname)
          if type(self.__call__.func_defaults) is NoneType:
              casalog.post(scriptstr[0]+'\n', 'INFO')
          else :
              casalog.post(scriptstr[1][1:]+'\n', 'INFO')
          result = hifa_wvrgcal(vis, caltable, offsetstable, hm_toffset, toffset, segsource, sourceflag, hm_tie, tie, nsol, disperse, wvrflag, hm_smooth, smooth, scale, maxdistm, minnumants, mingoodfrac, refant, qa_intent, qa_bandpass_intent, qa_spw, accept_threshold, pipelinemode, dryrun, acceptresults)
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('End Task: ' + tname)
          casalog.post('##### End Task: ' + tname + '  ' + spaces + ' #####'+
                       '\n##########################################')

	except Exception, instance:
          if(self.__globals__.has_key('__rethrow_casa_exceptions') and self.__globals__['__rethrow_casa_exceptions']) :
             raise
          else :
             #print '**** Error **** ',instance
	     tname = 'hifa_wvrgcal'
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
#        paramgui.runTask('hifa_wvrgcal', myf['_ip'])
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
        a['hm_toffset']  = 'automatic'
        a['segsource']  = True
        a['disperse']  = False
        a['hm_smooth']  = 'automatic'
        a['maxdistm']  = -1
        a['minnumants']  = 2
        a['mingoodfrac']  = 0.8
        a['qa_intent']  = ''
        a['qa_bandpass_intent']  = ''
        a['qa_spw']  = ''
        a['accept_threshold']  = 1.0
        a['pipelinemode']  = 'automatic'

        a['pipelinemode'] = {
                    0:{'value':'automatic'}, 
                    1:odict([{'value':'interactive'}, {'vis':[]}, {'caltable':[]}, {'offsetstable':[]}, {'wvrflag':[]}, {'scale':1.0}, {'refant':''}, {'dryrun':False}, {'acceptresults':True}]), 
                    2:odict([{'value':'getinputs'}, {'vis':[]}, {'caltable':[]}, {'offsetstable':[]}, {'wvrflag':[]}, {'scale':1.0}, {'refant':''}])}
        a['hm_smooth'] = {
                    0:{'value':'automatic'}, 
                    1:odict([{'value':'manual'}, {'smooth':''}])}
        a['hm_tie'] = {
                    0:{'value':'automatic'}, 
                    1:odict([{'value':'manual'}, {'sourceflag':[]}, {'tie':[]}, {'segsource':False}, {'nsol':1}])}
        a['hm_toffset'] = {
                    0:{'value':'automatic'}, 
                    1:odict([{'value':'manual'}, {'toffset':0}])}

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
    def description(self, key='hifa_wvrgcal', subkey=None):
        desc={'hifa_wvrgcal': '',
               'vis': 'List of input visibility files',
               'caltable': 'List of output gain calibration tables',
               'offsetstable': 'List of input temperature offsets table files',
               'hm_toffset': 'Toffset computation heuristic method',
               'toffset': 'Time offset (sec) between IF and WVR data',
               'segsource': 'Compute new coefficient calculation for each source',
               'sourceflag': 'Flag the WVR data for these source(s)',
               'hm_tie': 'Tie computation heuristics method',
               'tie': 'Sources for which to use the same atmospheric phase correction coefficients',
               'nsol': 'Number of solutions for phase correction coefficients',
               'disperse': 'Apply correction for dispersion',
               'wvrflag': 'Flag the WVR data for these antenna(s) replace with interpolated values',
               'hm_smooth': 'Smoothing computation heuristics method',
               'smooth': 'Smooth WVR data on the given timescale before calculating the correction',
               'scale': 'Scale the entire phase correction by this factor',
               'maxdistm': 'Maximum distance (m) of an antenna used for interpolation for a flagged antenna.',
               'minnumants': 'Minimum number of near antennas (up to 3) required for interpolation',
               'mingoodfrac': 'Minimum fraction of good data per antenna',
               'refant': 'Ranked list of reference antennas',
               'qa_intent': 'Data intents to use in estimating the effectiveness of the wvr correction',
               'qa_bandpass_intent': 'Data intent to use for the bandpass calibration in the qa calculation',
               'qa_spw': 'Data SpW(s) to use in estimating the effectiveness of the wvr correction',
               'accept_threshold': 'Improvement ratio (phase-rms without wvr / phase-rms with wvr) above which wvrg calibration file will be accepted',
               'pipelinemode': 'The pipeline operating mode',
               'dryrun': 'Run the task (False) or display the command(True)',
               'acceptresults': 'Add the results to the pipeline context',

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
        a['offsetstable']  = ['']
        a['hm_toffset']  = 'automatic'
        a['toffset']  = 0
        a['segsource']  = True
        a['sourceflag']  = ['']
        a['hm_tie']  = 'automatic'
        a['tie']  = ['']
        a['nsol']  = 1
        a['disperse']  = False
        a['wvrflag']  = ['']
        a['hm_smooth']  = 'automatic'
        a['smooth']  = ''
        a['scale']  = 1.
        a['maxdistm']  = -1
        a['minnumants']  = 2
        a['mingoodfrac']  = 0.8
        a['refant']  = ''
        a['qa_intent']  = ''
        a['qa_bandpass_intent']  = ''
        a['qa_spw']  = ''
        a['accept_threshold']  = 1.0
        a['pipelinemode']  = 'automatic'
        a['dryrun']  = False
        a['acceptresults']  = True

        #a = sys._getframe(len(inspect.stack())-1).f_globals

        if self.parameters['pipelinemode']  == 'interactive':
            a['vis'] = []
            a['caltable'] = []
            a['offsetstable'] = []
            a['wvrflag'] = []
            a['scale'] = 1.0
            a['refant'] = ''
            a['dryrun'] = False
            a['acceptresults'] = True

        if self.parameters['pipelinemode']  == 'getinputs':
            a['vis'] = []
            a['caltable'] = []
            a['offsetstable'] = []
            a['wvrflag'] = []
            a['scale'] = 1.0
            a['refant'] = ''

        if self.parameters['hm_smooth']  == 'manual':
            a['smooth'] = ''

        if self.parameters['hm_tie']  == 'manual':
            a['sourceflag'] = []
            a['tie'] = []
            a['segsource'] = False
            a['nsol'] = 1

        if self.parameters['hm_toffset']  == 'manual':
            a['toffset'] = 0

        if a.has_key(paramname) :
	      return a[paramname]
hifa_wvrgcal_cli = hifa_wvrgcal_cli_()
