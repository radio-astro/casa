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
from task_hifa_tsysflag import hifa_tsysflag
class hifa_tsysflag_cli_:
    __name__ = "hifa_tsysflag"
    rkey = None
    i_am_a_casapy_task = None
    # The existence of the i_am_a_casapy_task attribute allows help()
    # (and other) to treat casapy tasks as a special case.

    def __init__(self) :
       self.__bases__ = (hifa_tsysflag_cli_,)
       self.__doc__ = self.__call__.__doc__

       self.parameters={'vis':None, 'caltable':None, 'flag_nmedian':None, 'fnm_limit':None, 'fnm_byfield':None, 'flag_derivative':None, 'fd_max_limit':None, 'flag_edgechans':None, 'fe_edge_limit':None, 'flag_fieldshape':None, 'ff_refintent':None, 'ff_max_limit':None, 'flag_birdies':None, 'fb_sharps_limit':None, 'flag_toomany':None, 'tmf1_limit':None, 'tmef1_limit':None, 'metric_order':None, 'normalize_tsys':None, 'filetemplate':None, 'pipelinemode':None, 'dryrun':None, 'acceptresults':None, }


    def result(self, key=None):
	    #### and add any that have completed...
	    return None


    def __call__(self, vis=None, caltable=None, flag_nmedian=None, fnm_limit=None, fnm_byfield=None, flag_derivative=None, fd_max_limit=None, flag_edgechans=None, fe_edge_limit=None, flag_fieldshape=None, ff_refintent=None, ff_max_limit=None, flag_birdies=None, fb_sharps_limit=None, flag_toomany=None, tmf1_limit=None, tmef1_limit=None, metric_order=None, normalize_tsys=None, filetemplate=None, pipelinemode=None, dryrun=None, acceptresults=None, ):

        """Flag deviant system temperature measurements

	Detailed Description:

Flag deviant system temperatures for ALMA interferometry measurements. This is done by running a
sequence of flagging subtasks, each looking for a different type of possible error.

	Arguments :
		vis:	List of input MeasurementSets (Not used) 
		   Default Value: 

		caltable:	List of input caltables
		   Default Value: 

		flag_nmedian:	True to flag Tsys spectra with high median value
		   Default Value: True

		fnm_limit:	Flag spectra with median greater than fnm_limit * median over all spectra
		   Default Value: 2.0

		fnm_byfield:	Evaluate the nmedian metric separately for each field.
		   Default Value: False

		flag_derivative:	True to flag Tsys spectra with high median derivative
		   Default Value: True

		fd_max_limit:	Flag spectra with median derivative higher than fd_max_limit * median of this measure over all spectra
		   Default Value: 5.0

		flag_edgechans:	True to flag edges of Tsys spectra
		   Default Value: True

		fe_edge_limit:	Flag channels whose channel to channel difference greater than fe_edge_limit * median across spectrum
		   Default Value: 3.0

		flag_fieldshape:	True to flag Tsys spectra with a radically different shape to those of the ff_refintent
		   Default Value: True

		ff_refintent:	Data intent providing the reference shape for \'flag_fieldshape\'
		   Default Value: BANDPASS

		ff_max_limit:	Flag Tsys spectra with \'fieldshape\' metric greater than ff_max_limit
		   Default Value: 5.0

		flag_birdies:	True to flag channels covering sharp spectral features
		   Default Value: True

		fb_sharps_limit:	Flag channels bracketing a channel to channel difference greater than fb_sharps_limit
		   Default Value: 0.05

		flag_toomany:	True to flag Tsys spectra for which a proportion of timestamps or proportion of antennas that are entirely flagged exceeds their respective thresholds.
		   Default Value: True

		tmf1_limit:	Flag all Tsys spectra within a timestamp for an antenna if proportion flagged already exceeds tmf1_limit
		   Default Value: 0.666

		tmef1_limit:	Flag all Tsys spectra for all antennas in a spw, if proportion of antennas that are already entirely flagged in all timestamps exceeds tmef1_limit
		   Default Value: 0.666

		metric_order:	Order in which to evaluate the flagging metric(s); inactive metrics are skipped.
		   Default Value: nmedian,derivative,edgechans,fieldshape,birdies,toomany

		normalize_tsys:	Normalize Tsys prior to computing the flagging metric(s)
		   Default Value: False

		filetemplate:	File containing manual Tsys flags to apply.
		   Default Value: 

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


Flag deviant system temperatures for ALMA interferometry measurements.

Flag all deviant system temperature measurements in the system temperature
calibration table by running a sequence of flagging tests, each designed
to look for a different type of error.

If a file with manual Tsys flags is provided with the 'filetemplate'
parameter, then these flags are applied prior to the evaluation of the
flagging heuristics listed below.

The tests are:

1. Flag Tsys spectra with high median values

2. Flag Tsys spectra with high median derivatives. This is meant to spot 
spectra that are 'ringing'.

3. Flag the edge channels of the Tsys spectra in each SpW.

4. Flag Tsys spectra whose shape is different from that associated with
the BANDPASS intent.

5. Flag 'birdies'.

6. Flag the Tsys spectra of all antennas in a timestamp and spw if 
proportion of antennas already flagged in this timestamp and spw exceeds 
a threshold, and flag Tsys spectra for all antennas and all timestamps 
in a spw, if proportion of antennas that are already entirely flagged 
in all timestamps exceeds a threshold.


Keyword arguments:

--- Pipeline parameter arguments which can be set in any pipeline mode

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
    determines the values of all context defined pipeline inputs automatically.
    In interactive mode the user can set the pipeline context defined 
    parameters manually.  In 'getinputs' mode the user can check the settings 
    of all pipeline parameters without running the task.
    default: 'automatic'.

flag_nmedian -- True to flag Tsys spectra with high median value.
    default: True

fnm_limit -- Flag spectra with median value higher than fnm_limit * median 
    of this measure over all spectra.
    default: 2.0

fnm_byfield -- Evaluate the nmedian metric separately for each field.
    default: False
 
flag_derivative -- True to flag Tsys spectra with high median derivative.
    default: True

fd_max_limit -- Flag spectra with median derivative higher than 
    fd_max_limit * median of this measure over all spectra.
    default: 5.0

flag_edgechans -- True to flag edges of Tsys spectra.
    default: True

fe_edge_limit -- Flag channels whose channel to channel difference > 
    fe_edge_limit * median across spectrum.
    default: 3.0

flag_fieldshape -- True to flag Tsys spectra with a radically different 
    shape to those of the ff_refintent.
    default: True

ff_refintent -- Data intent that provides the reference shape for 
    'flag_fieldshape'.
    default: BANDPASS

ff_max_limit -- Flag Tsys spectra with 'fieldshape' metric values >
    ff_max_limit.
    default: 5.0

flag_birdies -- True to flag channels covering sharp spectral features.
    default: True

fb_sharps_limit -- Flag channels bracketing a channel to channel
    difference > fb_sharps_limit.
    default: 0.05

flag_toomany -- True to flag Tsys spectra for which a proportion of 
    antennas for given timestamp and/or proportion of antennas that are 
    entirely flagged in all timestamps exceeds their respective thresholds.
    default: True

tmf1_limit -- Flag Tsys spectra for all antennas in a timestamp and spw if 
    proportion of antennas already flagged in this timestamp and spw exceeds 
    tmf1_limit.
    default: 0.666

tmef1_limit -- Flag Tsys spectra for all antennas and all timestamps
    in a spw, if proportion of antennas that are already entirely flagged 
    in all timestamps exceeds tmef1_limit.
    default: 0.666

metric_order -- Order in which to evaluate the flagging metrics that are 
    enables. Disabled metrics are skipped.
    default: 'nmedian,derivative,edgechans,fieldshape,birdies,toomany'

normalize_tsys -- True to create a normalized Tsys table that is used to 
    evaluate the Tsys flagging metrics. All newly found flags are also applied
    to the original Tsys caltable that continues to be used for subsequent 
    calibration.
    default: False

filetemplate -- The name of a text file that contains the manual Tsys flagging
    template. If the template flags file is undefined, a name of the form
    'msname.flagtsystemplate.txt' is assumed.
    default: ''

--- Pipeline context defined parameter arguments which can be set only in
'interactive mode'

caltable -- List of input Tsys calibration tables
    default: [] - Use the table currently stored in the pipeline context. 
    example: caltable=['X132.ms.tsys.s2.tbl']

--- Pipeline task execution modes

dryrun -- Run the commands (True) or generate the commands to be run but
    do not execute (False).
    default: True

acceptresults -- Add the results of the task to the pipeline context (True) or
    reject them (False).
    default: True



Output:

results -- If pipeline mode is 'getinputs' then None is returned. Otherwise
    the results object for the pipeline task is returned.



Examples:

1. Flag Tsys measurements using currently recommended tests:

   hifa_tsysflag()

2. Flag Tsys measurements using all recommended tests apart from that
   using the 'fieldshape' metric.
 
   hifa_tsysflag(flag_fieldshape=False)

        """
	if not hasattr(self, "__globals__") or self.__globals__ == None :
           self.__globals__=stack_frame_find( )
	#casac = self.__globals__['casac']
	casalog = self.__globals__['casalog']
	casa = self.__globals__['casa']
	#casalog = casac.casac.logsink()
        self.__globals__['__last_task'] = 'hifa_tsysflag'
        self.__globals__['taskname'] = 'hifa_tsysflag'
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
            myparams['flag_nmedian'] = flag_nmedian = self.parameters['flag_nmedian']
            myparams['fnm_limit'] = fnm_limit = self.parameters['fnm_limit']
            myparams['fnm_byfield'] = fnm_byfield = self.parameters['fnm_byfield']
            myparams['flag_derivative'] = flag_derivative = self.parameters['flag_derivative']
            myparams['fd_max_limit'] = fd_max_limit = self.parameters['fd_max_limit']
            myparams['flag_edgechans'] = flag_edgechans = self.parameters['flag_edgechans']
            myparams['fe_edge_limit'] = fe_edge_limit = self.parameters['fe_edge_limit']
            myparams['flag_fieldshape'] = flag_fieldshape = self.parameters['flag_fieldshape']
            myparams['ff_refintent'] = ff_refintent = self.parameters['ff_refintent']
            myparams['ff_max_limit'] = ff_max_limit = self.parameters['ff_max_limit']
            myparams['flag_birdies'] = flag_birdies = self.parameters['flag_birdies']
            myparams['fb_sharps_limit'] = fb_sharps_limit = self.parameters['fb_sharps_limit']
            myparams['flag_toomany'] = flag_toomany = self.parameters['flag_toomany']
            myparams['tmf1_limit'] = tmf1_limit = self.parameters['tmf1_limit']
            myparams['tmef1_limit'] = tmef1_limit = self.parameters['tmef1_limit']
            myparams['metric_order'] = metric_order = self.parameters['metric_order']
            myparams['normalize_tsys'] = normalize_tsys = self.parameters['normalize_tsys']
            myparams['filetemplate'] = filetemplate = self.parameters['filetemplate']
            myparams['pipelinemode'] = pipelinemode = self.parameters['pipelinemode']
            myparams['dryrun'] = dryrun = self.parameters['dryrun']
            myparams['acceptresults'] = acceptresults = self.parameters['acceptresults']

        if type(vis)==str: vis=[vis]
        if type(caltable)==str: caltable=[caltable]
        if type(filetemplate)==str: filetemplate=[filetemplate]

	result = None

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['caltable'] = caltable
        mytmp['flag_nmedian'] = flag_nmedian
        mytmp['fnm_limit'] = fnm_limit
        mytmp['fnm_byfield'] = fnm_byfield
        mytmp['flag_derivative'] = flag_derivative
        mytmp['fd_max_limit'] = fd_max_limit
        mytmp['flag_edgechans'] = flag_edgechans
        mytmp['fe_edge_limit'] = fe_edge_limit
        mytmp['flag_fieldshape'] = flag_fieldshape
        mytmp['ff_refintent'] = ff_refintent
        mytmp['ff_max_limit'] = ff_max_limit
        mytmp['flag_birdies'] = flag_birdies
        mytmp['fb_sharps_limit'] = fb_sharps_limit
        mytmp['flag_toomany'] = flag_toomany
        mytmp['tmf1_limit'] = tmf1_limit
        mytmp['tmef1_limit'] = tmef1_limit
        mytmp['metric_order'] = metric_order
        mytmp['normalize_tsys'] = normalize_tsys
        mytmp['filetemplate'] = filetemplate
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli/"
	trec = casac.casac.utils().torecord(pathname+'hifa_tsysflag.xml')

        casalog.origin('hifa_tsysflag')
	try :
          #if not trec.has_key('hifa_tsysflag') or not casac.casac.utils().verify(mytmp, trec['hifa_tsysflag']) :
	    #return False

          casac.casac.utils().verify(mytmp, trec['hifa_tsysflag'], True)
          scriptstr=['']
          saveinputs = self.__globals__['saveinputs']
          if type(self.__call__.func_defaults) is NoneType:
              saveinputs=''
          else:
              saveinputs('hifa_tsysflag', 'hifa_tsysflag.last', myparams, self.__globals__,scriptstr=scriptstr)
          tname = 'hifa_tsysflag'
          spaces = ' '*(18-len(tname))
          casalog.post('\n##########################################'+
                       '\n##### Begin Task: ' + tname + spaces + ' #####')
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('Begin Task: ' + tname)
          if type(self.__call__.func_defaults) is NoneType:
              casalog.post(scriptstr[0]+'\n', 'INFO')
          else :
              casalog.post(scriptstr[1][1:]+'\n', 'INFO')
          result = hifa_tsysflag(vis, caltable, flag_nmedian, fnm_limit, fnm_byfield, flag_derivative, fd_max_limit, flag_edgechans, fe_edge_limit, flag_fieldshape, ff_refintent, ff_max_limit, flag_birdies, fb_sharps_limit, flag_toomany, tmf1_limit, tmef1_limit, metric_order, normalize_tsys, filetemplate, pipelinemode, dryrun, acceptresults)
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('End Task: ' + tname)
          casalog.post('##### End Task: ' + tname + '  ' + spaces + ' #####'+
                       '\n##########################################')

	except Exception, instance:
          if(self.__globals__.has_key('__rethrow_casa_exceptions') and self.__globals__['__rethrow_casa_exceptions']) :
             raise
          else :
             #print '**** Error **** ',instance
	     tname = 'hifa_tsysflag'
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
#        paramgui.runTask('hifa_tsysflag', myf['_ip'])
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
        a['flag_nmedian']  = True
        a['flag_derivative']  = True
        a['flag_edgechans']  = True
        a['flag_fieldshape']  = True
        a['flag_birdies']  = True
        a['flag_toomany']  = True
        a['metric_order']  = 'nmedian,derivative,edgechans,fieldshape,birdies,toomany'
        a['normalize_tsys']  = False
        a['filetemplate']  = ['']
        a['pipelinemode']  = 'automatic'

        a['pipelinemode'] = {
                    0:{'value':'automatic'}, 
                    1:odict([{'value':'interactive'}, {'caltable':[]}, {'dryrun':False}, {'acceptresults':True}]), 
                    2:odict([{'value':'getinputs'}, {'caltable':[]}])}
        a['flag_nmedian'] = {
                    0:odict([{'value':True}, {'fnm_limit':2.0}, {'fnm_byfield':False}]), 
                    1:{'value':False}}
        a['flag_derivative'] = {
                    0:odict([{'value':True}, {'fd_max_limit':5.0}]), 
                    1:{'value':False}}
        a['flag_edgechans'] = {
                    0:odict([{'value':True}, {'fe_edge_limit':3.0}]), 
                    1:{'value':False}}
        a['flag_fieldshape'] = {
                    0:odict([{'value':True}, {'ff_refintent':'BANDPASS'}, {'ff_max_limit':5.0}]), 
                    1:{'value':False}}
        a['flag_birdies'] = {
                    0:odict([{'value':True}, {'fb_sharps_limit':0.05}]), 
                    1:{'value':False}}
        a['flag_toomany'] = {
                    0:odict([{'value':True}, {'tmf1_limit':0.666}, {'tmef1_limit':0.666}]), 
                    1:{'value':False}}

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
    def description(self, key='hifa_tsysflag', subkey=None):
        desc={'hifa_tsysflag': 'Flag deviant system temperature measurements',
               'vis': 'List of input MeasurementSets (Not used) ',
               'caltable': 'List of input caltables',
               'flag_nmedian': 'True to flag Tsys spectra with high median value',
               'fnm_limit': 'Flag spectra with median greater than fnm_limit * median over all spectra',
               'fnm_byfield': 'Evaluate the nmedian metric separately for each field.',
               'flag_derivative': 'True to flag Tsys spectra with high median derivative',
               'fd_max_limit': 'Flag spectra with median derivative higher than fd_max_limit * median of this measure over all spectra',
               'flag_edgechans': 'True to flag edges of Tsys spectra',
               'fe_edge_limit': 'Flag channels whose channel to channel difference greater than fe_edge_limit * median across spectrum',
               'flag_fieldshape': 'True to flag Tsys spectra with a radically different shape to those of the ff_refintent',
               'ff_refintent': 'Data intent providing the reference shape for \'flag_fieldshape\'',
               'ff_max_limit': 'Flag Tsys spectra with \'fieldshape\' metric greater than ff_max_limit',
               'flag_birdies': 'True to flag channels covering sharp spectral features',
               'fb_sharps_limit': 'Flag channels bracketing a channel to channel difference greater than fb_sharps_limit',
               'flag_toomany': 'True to flag Tsys spectra for which a proportion of timestamps or proportion of antennas that are entirely flagged exceeds their respective thresholds.',
               'tmf1_limit': 'Flag all Tsys spectra within a timestamp for an antenna if proportion flagged already exceeds tmf1_limit',
               'tmef1_limit': 'Flag all Tsys spectra for all antennas in a spw, if proportion of antennas that are already entirely flagged in all timestamps exceeds tmef1_limit',
               'metric_order': 'Order in which to evaluate the flagging metric(s); inactive metrics are skipped.',
               'normalize_tsys': 'Normalize Tsys prior to computing the flagging metric(s)',
               'filetemplate': 'File containing manual Tsys flags to apply.',
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
        a['caltable']  = ['']
        a['flag_nmedian']  = True
        a['fnm_limit']  = 2.0
        a['fnm_byfield']  = False
        a['flag_derivative']  = True
        a['fd_max_limit']  = 5.0
        a['flag_edgechans']  = True
        a['fe_edge_limit']  = 3.0
        a['flag_fieldshape']  = True
        a['ff_refintent']  = 'BANDPASS'
        a['ff_max_limit']  = 5.0
        a['flag_birdies']  = True
        a['fb_sharps_limit']  = 0.05
        a['flag_toomany']  = True
        a['tmf1_limit']  = 0.666
        a['tmef1_limit']  = 0.666
        a['metric_order']  = 'nmedian,derivative,edgechans,fieldshape,birdies,toomany'
        a['normalize_tsys']  = False
        a['filetemplate']  = ['']
        a['pipelinemode']  = 'automatic'
        a['dryrun']  = False
        a['acceptresults']  = True

        #a = sys._getframe(len(inspect.stack())-1).f_globals

        if self.parameters['pipelinemode']  == 'interactive':
            a['caltable'] = []
            a['dryrun'] = False
            a['acceptresults'] = True

        if self.parameters['pipelinemode']  == 'getinputs':
            a['caltable'] = []

        if self.parameters['flag_nmedian']  == True:
            a['fnm_limit'] = 2.0
            a['fnm_byfield'] = False

        if self.parameters['flag_derivative']  == True:
            a['fd_max_limit'] = 5.0

        if self.parameters['flag_edgechans']  == True:
            a['fe_edge_limit'] = 3.0

        if self.parameters['flag_fieldshape']  == True:
            a['ff_refintent'] = 'BANDPASS'
            a['ff_max_limit'] = 5.0

        if self.parameters['flag_birdies']  == True:
            a['fb_sharps_limit'] = 0.05

        if self.parameters['flag_toomany']  == True:
            a['tmf1_limit'] = 0.666
            a['tmef1_limit'] = 0.666

        if a.has_key(paramname) :
	      return a[paramname]
hifa_tsysflag_cli = hifa_tsysflag_cli_()
