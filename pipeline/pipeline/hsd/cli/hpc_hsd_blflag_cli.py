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
from task_hpc_hsd_blflag import hpc_hsd_blflag
class hpc_hsd_blflag_cli_:
    __name__ = "hpc_hsd_blflag"
    rkey = None
    i_am_a_casapy_task = None
    # The existence of the i_am_a_casapy_task attribute allows help()
    # (and other) to treat casapy tasks as a special case.

    def __init__(self) :
       self.__bases__ = (hpc_hsd_blflag_cli_,)
       self.__doc__ = self.__call__.__doc__

       self.parameters={'iteration':None, 'edge':None, 'flag_tsys':None, 'tsys_thresh':None, 'flag_weath':None, 'weath_thresh':None, 'flag_prfre':None, 'prfre_thresh':None, 'flag_pofre':None, 'pofre_thresh':None, 'flag_prfr':None, 'prfr_thresh':None, 'flag_pofr':None, 'pofr_thresh':None, 'flag_prfrm':None, 'prfrm_thresh':None, 'prfrm_nmean':None, 'flag_pofrm':None, 'pofrm_thresh':None, 'pofrm_nmean':None, 'flag_user':None, 'user_thresh':None, 'plotflag':None, 'pipelinemode':None, 'infiles':None, 'antenna':None, 'field':None, 'spw':None, 'pol':None, 'dryrun':None, 'acceptresults':None, 'parallel':None, }


    def result(self, key=None):
	    #### and add any that have completed...
	    return None


    def __call__(self, iteration=None, edge=None, flag_tsys=None, tsys_thresh=None, flag_weath=None, weath_thresh=None, flag_prfre=None, prfre_thresh=None, flag_pofre=None, pofre_thresh=None, flag_prfr=None, prfr_thresh=None, flag_pofr=None, pofr_thresh=None, flag_prfrm=None, prfrm_thresh=None, prfrm_nmean=None, flag_pofrm=None, pofrm_thresh=None, pofrm_nmean=None, flag_user=None, user_thresh=None, plotflag=None, pipelinemode=None, infiles=None, antenna=None, field=None, spw=None, pol=None, dryrun=None, acceptresults=None, parallel=None, ):

        """Flag spectra based on predefined criteria of single-dish pipeline

	Detailed Description: 


	Arguments :
		iteration:	Number of iteration to perform sigma clipping to calculate threshold
		   Default Value: 5

		edge:	Number of edge channels to be excluded from statistic calculation to flag data
		   Default Value: 0,0

		flag_tsys:	Flag data by Tsys value
		   Default Value: True

		tsys_thresh:	Threshold for Tsys flag
		   Default Value: 3.0

		flag_weath:	Flag data by weather (not implemented yet)
		   Default Value: False

		weath_thresh:	Threshold for weather flag
		   Default Value: 3.0

		flag_prfre:	Flag data by EXPECTED RMS of pre-fit spectra
		   Default Value: True

		prfre_thresh:	Threshold for EXPECTED RMS of pre-fit spectra flag
		   Default Value: 3.0

		flag_pofre:	Flag data by EXPECTED RMS of post-fit spectra
		   Default Value: True

		pofre_thresh:	Threshold for EXPECTED RMS of post-fit spectra flag
		   Default Value: 1.3333

		flag_prfr:	Flag data by RMS of pre-fit spectra
		   Default Value: True

		prfr_thresh:	Threshold for RMS of pre-fit flag
		   Default Value: 4.5

		flag_pofr:	Flag data by RMS of post-fit spectra
		   Default Value: True

		pofr_thresh:	Threshold for RMS of post-fit spectra flag
		   Default Value: 4.0

		flag_prfrm:	Flag data by running mean of pre-fit spectra
		   Default Value: True

		prfrm_thresh:	Threshold for running mean of pre-fit spectra flag
		   Default Value: 5.5

		prfrm_nmean:	Number of channel for running mean of pre-fit spectra flag
		   Default Value: 5

		flag_pofrm:	Flag data by running mean of post-fit spectra
		   Default Value: True

		pofrm_thresh:	Threshold for running mean of post-fit spectra flag
		   Default Value: 5.0

		pofrm_nmean:	Number of channel for running mean of post-fit spectra flag
		   Default Value: 5

		flag_user:	Flag data by user flag (not implemented yet)
		   Default Value: False

		user_thresh:	Threshold for user flag
		   Default Value: 5.0

		plotflag:	Create plots for flagging
		   Default Value: True

		pipelinemode:	The pipeline operating  mode
		   Default Value: automatic
		   Allowed Values:
				automatic
				interactive
				getinputs

		infiles:	List of input files to be flagged (\'\'=all)
		   Default Value: 

		antenna:	select data by antenna names or ids, e.g. \'PM03,PM04\' (\'\'=all)
		   Default Value: 

		field:	select data by field names or ids, e.g. \'M100,Sgr*\' (\'\'=all)
		   Default Value: 

		spw:	select data by spectral windows, e.g. \'3,5,7\' (\'\'=all)
		   Default Value: 

		pol:	select data by polarizations, e.g. \'XX,YY\' (\'\'=all)
		   Default Value: 

		dryrun:	Run the task (False) or display the task command (True)
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


Keyword arguments:

---- pipeline parameter arguments which can be set in any pipeline mode
iteration -- Number of iterations to perform sigma clipping to calculate
       threshold value of flagging.
       default: 5

edge -- Number of channels to be dropped from the edge.
       The value must be a list of integer with length of one or two.
                       If list length is one, same number will be applied both side of 
       the band.
       default: [0,0]
       example: [10,20], [10]

flag_tsys -- Activate (True) or deactivate (False) Tsys flag.
       default: True

tsys_thresh -- Threshold value for Tsys flag.
       default: 3.0

flag_weath -- Activate (True) or deactivate (False) weather flag. 
       Since weather flagging is not implemented yet. Setting True 
       has no effect at the moment.
       default: False

weath_thresh -- Threshold value for weather flag.
       default: 3.0

flag_prfre -- Activate (True) or deactivate (False) flag by expected rms 
       of pre-fit spectra.
       default: True

                prfre_thresh -- Threshold value for flag by expected rms of pre-fit 
       spectra. 
       default: 3.0 

flag_pofre -- Activate (True) or deactivate (False) flag by expected rms 
       of post-fit spectra.
       default: True

pofre_thresh -- Threshold value for flag by expected rms of post-fit 
       spectra. 
       default: 1.3333

flag_prfr -- Activate (True) or deactivate (False) flag by rms of pre-fit 
       spectra.
       default: True

prfr_thresh -- Threshold value for flag by rms of pre-fit spectra. 
       default: 4.5

flag_pofr -- Activate (True) or deactivate (False) flag by rms of post-fit 
       spectra.
       default: True

                pofr_thresh -- Threshold value for flag by rms of post-fit spectra. 
       default: 4.0

flag_prfrm -- Activate (True) or deactivate (False) flag by running mean 
       of pre-fit spectra.
       default: True

prfrm_thresh -- Threshold value for flag by running mean of pre-fit 
       spectra. 
       default: 5.5

prfrm_nmean -- Number of channels for running mean of pre-fit spectra.
       default: 5

flag_pofrm -- Activate (True) or deactivate (False) flag by running mean 
       of post-fit spectra.
       default: True

pofrm_thresh -- Threshold value for flag by running mean of post-fit 
       spectra. 
       default: 5.0

pofrm_nmean -- Number of channels for running mean of post-fit spectra.
                       default: 5

flag_user -- Activate (True) or deactivate (False) user-defined flag. 
       Since user flagging is not implemented yet. Setting True 
       has no effect at the moment.
       default: False

user_thresh -- Threshold value for flag by user-defined rule.
       default: 3.0

plotflag -- True to plot result of data flagging.
       default: True

pipelinemode -- The pipeline operating mode. In 'automatic' mode the 
       pipeline determines the values of all context defined pipeline 
       inputs automatically.  In interactive mode the user can set the 
       pipeline context defined parameters manually. In 'getinputs' mode 
       the user can check the settings of all pipeline parameters without 
       running the task.
       default: 'automatic'.

---- pipeline context defined parameter argument which can be set only in
'interactive mode' or 'getinputs' modes 
                
infiles -- ASDM or MS files to be processed. This parameter behaves as 
       data selection parameter. The name specified by infiles must be 
       registered to context before you run hpc_hsd_blflag.
       default: [] (process all data in context)

antenna -- Data selection by antenna names or ids.
   default: '' (all antennas)
   example: 'PM03,PM04'
   
field -- Data selection by field names or ids.
   default: '' (all fields)
   example: '*Sgr*,M100'
   
spw -- Data selection by spw IDs.
   default: '' (all spws)
   example: '3,4' (spw 3 and 4)

pol -- Data selection by pol.
   default: '' (all polarizations)
   example: 'XX,YY' (correlation XX and YY)

--- pipeline task execution modes
dryrun -- Run the commands (True) or generate the commands to be run but
                       do not execute (False).
       default: True

acceptresults -- Add the results of the task to the pipeline context 
       (True) or reject them (False).
       default: True


Output:

results -- If pipeline mode is 'getinputs' then None is returned. Otherwise
       the results object for the pipeline task is returned.


Description

The hpc_hsd_blflag
6. Flagging 
Data are flagged based on several flagging rules. Available rules are: 
expected rms, calculated rms, and running mean of both pre-fit and 
post-fit spectra. Tsys flagging is also available. 

In addition, the heuristics script creates many plots for each stage. 
Those plots are included in the weblog.


Issues


Examples
                                

        """
	if not hasattr(self, "__globals__") or self.__globals__ == None :
           self.__globals__=stack_frame_find( )
	#casac = self.__globals__['casac']
	casalog = self.__globals__['casalog']
	casa = self.__globals__['casa']
	#casalog = casac.casac.logsink()
        self.__globals__['__last_task'] = 'hpc_hsd_blflag'
        self.__globals__['taskname'] = 'hpc_hsd_blflag'
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

            myparams['iteration'] = iteration = self.parameters['iteration']
            myparams['edge'] = edge = self.parameters['edge']
            myparams['flag_tsys'] = flag_tsys = self.parameters['flag_tsys']
            myparams['tsys_thresh'] = tsys_thresh = self.parameters['tsys_thresh']
            myparams['flag_weath'] = flag_weath = self.parameters['flag_weath']
            myparams['weath_thresh'] = weath_thresh = self.parameters['weath_thresh']
            myparams['flag_prfre'] = flag_prfre = self.parameters['flag_prfre']
            myparams['prfre_thresh'] = prfre_thresh = self.parameters['prfre_thresh']
            myparams['flag_pofre'] = flag_pofre = self.parameters['flag_pofre']
            myparams['pofre_thresh'] = pofre_thresh = self.parameters['pofre_thresh']
            myparams['flag_prfr'] = flag_prfr = self.parameters['flag_prfr']
            myparams['prfr_thresh'] = prfr_thresh = self.parameters['prfr_thresh']
            myparams['flag_pofr'] = flag_pofr = self.parameters['flag_pofr']
            myparams['pofr_thresh'] = pofr_thresh = self.parameters['pofr_thresh']
            myparams['flag_prfrm'] = flag_prfrm = self.parameters['flag_prfrm']
            myparams['prfrm_thresh'] = prfrm_thresh = self.parameters['prfrm_thresh']
            myparams['prfrm_nmean'] = prfrm_nmean = self.parameters['prfrm_nmean']
            myparams['flag_pofrm'] = flag_pofrm = self.parameters['flag_pofrm']
            myparams['pofrm_thresh'] = pofrm_thresh = self.parameters['pofrm_thresh']
            myparams['pofrm_nmean'] = pofrm_nmean = self.parameters['pofrm_nmean']
            myparams['flag_user'] = flag_user = self.parameters['flag_user']
            myparams['user_thresh'] = user_thresh = self.parameters['user_thresh']
            myparams['plotflag'] = plotflag = self.parameters['plotflag']
            myparams['pipelinemode'] = pipelinemode = self.parameters['pipelinemode']
            myparams['infiles'] = infiles = self.parameters['infiles']
            myparams['antenna'] = antenna = self.parameters['antenna']
            myparams['field'] = field = self.parameters['field']
            myparams['spw'] = spw = self.parameters['spw']
            myparams['pol'] = pol = self.parameters['pol']
            myparams['dryrun'] = dryrun = self.parameters['dryrun']
            myparams['acceptresults'] = acceptresults = self.parameters['acceptresults']
            myparams['parallel'] = parallel = self.parameters['parallel']

        if type(edge)==int: edge=[edge]
        if type(infiles)==str: infiles=[infiles]

	result = None

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['iteration'] = iteration
        mytmp['edge'] = edge
        mytmp['flag_tsys'] = flag_tsys
        mytmp['tsys_thresh'] = tsys_thresh
        mytmp['flag_weath'] = flag_weath
        mytmp['weath_thresh'] = weath_thresh
        mytmp['flag_prfre'] = flag_prfre
        mytmp['prfre_thresh'] = prfre_thresh
        mytmp['flag_pofre'] = flag_pofre
        mytmp['pofre_thresh'] = pofre_thresh
        mytmp['flag_prfr'] = flag_prfr
        mytmp['prfr_thresh'] = prfr_thresh
        mytmp['flag_pofr'] = flag_pofr
        mytmp['pofr_thresh'] = pofr_thresh
        mytmp['flag_prfrm'] = flag_prfrm
        mytmp['prfrm_thresh'] = prfrm_thresh
        mytmp['prfrm_nmean'] = prfrm_nmean
        mytmp['flag_pofrm'] = flag_pofrm
        mytmp['pofrm_thresh'] = pofrm_thresh
        mytmp['pofrm_nmean'] = pofrm_nmean
        mytmp['flag_user'] = flag_user
        mytmp['user_thresh'] = user_thresh
        mytmp['plotflag'] = plotflag
        mytmp['pipelinemode'] = pipelinemode
        mytmp['infiles'] = infiles
        mytmp['antenna'] = antenna
        mytmp['field'] = field
        mytmp['spw'] = spw
        mytmp['pol'] = pol
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
        mytmp['parallel'] = parallel
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli/"
	trec = casac.casac.utils().torecord(pathname+'hpc_hsd_blflag.xml')

        casalog.origin('hpc_hsd_blflag')
	try :
          #if not trec.has_key('hpc_hsd_blflag') or not casac.casac.utils().verify(mytmp, trec['hpc_hsd_blflag']) :
	    #return False

          casac.casac.utils().verify(mytmp, trec['hpc_hsd_blflag'], True)
          scriptstr=['']
          saveinputs = self.__globals__['saveinputs']
          if type(self.__call__.func_defaults) is NoneType:
              saveinputs=''
          else:
              saveinputs('hpc_hsd_blflag', 'hpc_hsd_blflag.last', myparams, self.__globals__,scriptstr=scriptstr)
          tname = 'hpc_hsd_blflag'
          spaces = ' '*(18-len(tname))
          casalog.post('\n##########################################'+
                       '\n##### Begin Task: ' + tname + spaces + ' #####')
          if type(self.__call__.func_defaults) is NoneType:
              casalog.post(scriptstr[0]+'\n', 'INFO')
          else :
              casalog.post(scriptstr[1][1:]+'\n', 'INFO')
          result = hpc_hsd_blflag(iteration, edge, flag_tsys, tsys_thresh, flag_weath, weath_thresh, flag_prfre, prfre_thresh, flag_pofre, pofre_thresh, flag_prfr, prfr_thresh, flag_pofr, pofr_thresh, flag_prfrm, prfrm_thresh, prfrm_nmean, flag_pofrm, pofrm_thresh, pofrm_nmean, flag_user, user_thresh, plotflag, pipelinemode, infiles, antenna, field, spw, pol, dryrun, acceptresults, parallel)
          casalog.post('##### End Task: ' + tname + '  ' + spaces + ' #####'+
                       '\n##########################################')

	except Exception, instance:
          if(self.__globals__.has_key('__rethrow_casa_exceptions') and self.__globals__['__rethrow_casa_exceptions']) :
             raise
          else :
             #print '**** Error **** ',instance
	     tname = 'hpc_hsd_blflag'
             casalog.post('An error occurred running task '+tname+'.', 'ERROR')
             pass

        gc.collect()
        return result
#
#
#
    def paramgui(self, useGlobals=True, ipython_globals=None):
        """
        Opens a parameter GUI for this task.  If useGlobals is true, then any relevant global parameter settings are used.
        """
        import paramgui
	if not hasattr(self, "__globals__") or self.__globals__ == None :
           self.__globals__=stack_frame_find( )

        if useGlobals:
	    if ipython_globals == None:
                myf=self.__globals__
            else:
                myf=ipython_globals

            paramgui.setGlobals(myf)
        else:
            paramgui.setGlobals({})

        paramgui.runTask('hpc_hsd_blflag', myf['_ip'])
        paramgui.setGlobals({})

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
        a['iteration']  = 5
        a['edge']  = [0,0]
        a['flag_tsys']  = True
        a['flag_weath']  = False
        a['flag_prfre']  = True
        a['flag_pofre']  = True
        a['flag_prfr']  = True
        a['flag_pofr']  = True
        a['flag_prfrm']  = True
        a['flag_pofrm']  = True
        a['flag_user']  = False
        a['plotflag']  = True
        a['pipelinemode']  = 'automatic'

        a['flag_tsys'] = {
                    0:odict([{'value':True}, {'tsys_thresh':3.0}]), 
                    1:{'value':False}}
        a['flag_weath'] = {
                    0:{'value':False}, 
                    1:odict([{'value':True}, {'weath_thresh':3.0}])}
        a['flag_prfre'] = {
                    0:odict([{'value':True}, {'prfre_thresh':3.0}]), 
                    1:{'value':False}}
        a['flag_pofre'] = {
                    0:odict([{'value':True}, {'pofre_thresh':1.3333}]), 
                    1:{'value':False}}
        a['flag_prfr'] = {
                    0:odict([{'value':True}, {'prfr_thresh':4.5}]), 
                    1:{'value':False}}
        a['flag_pofr'] = {
                    0:odict([{'value':True}, {'pofr_thresh':4.0}]), 
                    1:{'value':False}}
        a['flag_prfrm'] = {
                    0:odict([{'value':True}, {'prfrm_thresh':5.5}, {'prfrm_nmean':5}]), 
                    1:{'value':False}}
        a['flag_pofrm'] = {
                    0:odict([{'value':True}, {'pofrm_thresh':5.0}, {'pofrm_nmean':5}]), 
                    1:{'value':False}}
        a['flag_user'] = {
                    0:{'value':False}, 
                    1:odict([{'value':True}, {'user_thresh':5.0}])}
        a['pipelinemode'] = {
                    0:{'value':'automatic'}, 
                    1:odict([{'value':'interactive'}, {'infiles':[]}, {'antenna':''}, {'field':''}, {'spw':''}, {'pol':''}, {'dryrun':False}, {'acceptresults':True}, {'parallel':'automatic'}]), 
                    2:odict([{'value':'getinputs'}, {'infiles':[]}, {'antenna':''}, {'field':''}, {'spw':''}, {'pol':''}])}

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
    def description(self, key='hpc_hsd_blflag', subkey=None):
        desc={'hpc_hsd_blflag': 'Flag spectra based on predefined criteria of single-dish pipeline',
               'iteration': 'Number of iteration to perform sigma clipping to calculate threshold',
               'edge': 'Number of edge channels to be excluded from statistic calculation to flag data',
               'flag_tsys': 'Flag data by Tsys value',
               'tsys_thresh': 'Threshold for Tsys flag',
               'flag_weath': 'Flag data by weather (not implemented yet)',
               'weath_thresh': 'Threshold for weather flag',
               'flag_prfre': 'Flag data by EXPECTED RMS of pre-fit spectra',
               'prfre_thresh': 'Threshold for EXPECTED RMS of pre-fit spectra flag',
               'flag_pofre': 'Flag data by EXPECTED RMS of post-fit spectra',
               'pofre_thresh': 'Threshold for EXPECTED RMS of post-fit spectra flag',
               'flag_prfr': 'Flag data by RMS of pre-fit spectra',
               'prfr_thresh': 'Threshold for RMS of pre-fit flag',
               'flag_pofr': 'Flag data by RMS of post-fit spectra',
               'pofr_thresh': 'Threshold for RMS of post-fit spectra flag',
               'flag_prfrm': 'Flag data by running mean of pre-fit spectra',
               'prfrm_thresh': 'Threshold for running mean of pre-fit spectra flag',
               'prfrm_nmean': 'Number of channel for running mean of pre-fit spectra flag',
               'flag_pofrm': 'Flag data by running mean of post-fit spectra',
               'pofrm_thresh': 'Threshold for running mean of post-fit spectra flag',
               'pofrm_nmean': 'Number of channel for running mean of post-fit spectra flag',
               'flag_user': 'Flag data by user flag (not implemented yet)',
               'user_thresh': 'Threshold for user flag',
               'plotflag': 'Create plots for flagging',
               'pipelinemode': 'The pipeline operating  mode',
               'infiles': 'List of input files to be flagged (\'\'=all)',
               'antenna': 'select data by antenna names or ids, e.g. \'PM03,PM04\' (\'\'=all)',
               'field': 'select data by field names or ids, e.g. \'M100,Sgr*\' (\'\'=all)',
               'spw': 'select data by spectral windows, e.g. \'3,5,7\' (\'\'=all)',
               'pol': 'select data by polarizations, e.g. \'XX,YY\' (\'\'=all)',
               'dryrun': 'Run the task (False) or display the task command (True)',
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
        a['iteration']  = 5
        a['edge']  = [0,0]
        a['flag_tsys']  = True
        a['tsys_thresh']  = 3.0
        a['flag_weath']  = False
        a['weath_thresh']  = 3.0
        a['flag_prfre']  = True
        a['prfre_thresh']  = 3.0
        a['flag_pofre']  = True
        a['pofre_thresh']  = 1.3333
        a['flag_prfr']  = True
        a['prfr_thresh']  = 4.5
        a['flag_pofr']  = True
        a['pofr_thresh']  = 4.0
        a['flag_prfrm']  = True
        a['prfrm_thresh']  = 5.5
        a['prfrm_nmean']  = 5
        a['flag_pofrm']  = True
        a['pofrm_thresh']  = 5.0
        a['pofrm_nmean']  = 5
        a['flag_user']  = False
        a['user_thresh']  = 5.0
        a['plotflag']  = True
        a['pipelinemode']  = 'automatic'
        a['infiles']  = ['']
        a['antenna']  = ''
        a['field']  = ''
        a['spw']  = ''
        a['pol']  = ''
        a['dryrun']  = False
        a['acceptresults']  = True
        a['parallel']  = 'automatic'

        #a = sys._getframe(len(inspect.stack())-1).f_globals

        if self.parameters['flag_tsys']  == True:
            a['tsys_thresh'] = 3.0

        if self.parameters['flag_weath']  == True:
            a['weath_thresh'] = 3.0

        if self.parameters['flag_prfre']  == True:
            a['prfre_thresh'] = 3.0

        if self.parameters['flag_pofre']  == True:
            a['pofre_thresh'] = 1.3333

        if self.parameters['flag_prfr']  == True:
            a['prfr_thresh'] = 4.5

        if self.parameters['flag_pofr']  == True:
            a['pofr_thresh'] = 4.0

        if self.parameters['flag_prfrm']  == True:
            a['prfrm_thresh'] = 5.5
            a['prfrm_nmean'] = 5

        if self.parameters['flag_pofrm']  == True:
            a['pofrm_thresh'] = 5.0
            a['pofrm_nmean'] = 5

        if self.parameters['flag_user']  == True:
            a['user_thresh'] = 5.0

        if self.parameters['pipelinemode']  == 'interactive':
            a['infiles'] = []
            a['antenna'] = ''
            a['field'] = ''
            a['spw'] = ''
            a['pol'] = ''
            a['dryrun'] = False
            a['acceptresults'] = True
            a['parallel'] = 'automatic'

        if self.parameters['pipelinemode']  == 'getinputs':
            a['infiles'] = []
            a['antenna'] = ''
            a['field'] = ''
            a['spw'] = ''
            a['pol'] = ''

        if a.has_key(paramname) :
	      return a[paramname]
hpc_hsd_blflag_cli = hpc_hsd_blflag_cli_()
