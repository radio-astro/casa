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
from task_hsd_baseline import hsd_baseline
class hsd_baseline_cli_:
    __name__ = "hsd_baseline"
    rkey = None
    i_am_a_casapy_task = None
    # The existence of the i_am_a_casapy_task attribute allows help()
    # (and other) to treat casapy tasks as a special case.

    def __init__(self) :
       self.__bases__ = (hsd_baseline_cli_,)
       self.__doc__ = self.__call__.__doc__

       self.parameters={'fitfunc':None, 'fitorder':None, 'linewindow':None, 'linewindowmode':None, 'edge':None, 'broadline':None, 'clusteringalgorithm':None, 'deviationmask':None, 'pipelinemode':None, 'infiles':None, 'field':None, 'antenna':None, 'spw':None, 'pol':None, 'dryrun':None, 'acceptresults':None, 'parallel':None, }


    def result(self, key=None):
	    #### and add any that have completed...
	    return None


    def __call__(self, fitfunc=None, fitorder=None, linewindow=None, linewindowmode=None, edge=None, broadline=None, clusteringalgorithm=None, deviationmask=None, pipelinemode=None, infiles=None, field=None, antenna=None, spw=None, pol=None, dryrun=None, acceptresults=None, parallel=None, ):

        """Detect and validate spectral lines, subtract baseline by masking detected lines

	Detailed Description:


	Arguments :
		fitfunc:	Fitting function for baseline subtraction
		   Default Value: cspline
		   Allowed Values:
				cspline
				spline
				CSPLINE
				SPLINE

		fitorder:	Fitting order for baseline subtraction
		   Default Value: -1

		linewindow:	Pre-defined line window
		   Default Value: 

		linewindowmode:	Merge or replace given manual line window with line detection/validation result
		   Default Value: replace
		   Allowed Values:
				replace
				merge

		edge:	Edge channels to be dropped
		   Default Value: 

		broadline:	Try to detect broad component of the line
		   Default Value: True

		clusteringalgorithm:	Algorithm for line validation clustering algorithm
		   Default Value: kmean
		   Allowed Values:
				kmean
				hierarchy

		deviationmask:	Apply deviation mask in addition to detected line masks
		   Default Value: True

		pipelinemode:	The pipeline operating mode
		   Default Value: automatic
		   Allowed Values:
				automatic
				interactive
				getinputs

		infiles:	List of input files to be calibrated (default all)
		   Default Value: 

		field:	select data by field
		   Default Value: 

		antenna:	select data by antenna
		   Default Value: 

		spw:	select data by IF IDs (spectral windows), e.g. \'3,5,7\' (\'\'=all)
		   Default Value: 

		pol:	select data by polarizations, e.g. \'0~1\' (\'\'=all)
		   Default Value: 

		dryrun:	Run the task (False) or display task command (True)
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

The hsd_baseline task subtracts baseline from calibrated spectra.
By default, the task tries to find spectral line feature using
line detection and validation algorithms. Then, the task puts a
mask on detected lines and perform baseline subtraction. The user
is able to turn off automatic line masking by setting linewindow
parameter, which specifies pre-defined line window. 

Fitting order is automatically determined by default. It can be
disabled by specifying fitorder as non-negative value. In this case,
the value specified by fitorder will be used.
  
Keyword arguments:

---- pipeline parameter arguments which can be set in any pipeline mode
fitfunc -- fitting function for baseline subtraction. You can only choose
   cubic spline ('spline' or 'cspline')
   default: 'cspline'.

fitorder -- Fitting order for polynomial. For cubic spline, it is used
   to determine how much the spectrum is segmented into. Default (-1) is
   to determine the order automatically.
   default: -1 (auto determination)

linewindow -- Pre-defined line window. If this is set, specified line 
   windows are used as a line mask for baseline subtraction instead to 
   determine masks based on line detection and validation stage. Several  
   types of format are acceptable. One is channel-based window, 
   
      [min_chan, max_chan] 
      
   where min_chan and max_chan should be an integer. For multiple 
   windows, nested list is also acceptable, 
   
      [[min_chan0, max_chan0], [min_chan1, max_chan1], ...]
      
   Another way is frequency-based window, 
   
      [min_freq, max_freq]
      
   where min_freq and max_freq should be either a float or a string. 
   If float value is given, it is interpreted as a frequency in Hz. 
   String should be a quantity consisting of "value" and "unit", e.g., 
   '100GHz'. Multiple windows are also supported.
   
      [[min_freq0, max_freq0], [min_freq1, max_freq1], ...]
      
   Note that the specified frequencies are assumed to be the value in 
   LSRK frame. Note also that there is a limitation when multiple MSs are 
   processed. If native frequency frame of the data is not LSRK (e.g. TOPO), 
   frequencies need to be converted to that frame. As a result, corresponding 
   chnnnel range may vary between MSs. However, current implementation is 
   not able to handle such case. Frequencies are converted to desired frame 
   using representative MS (time, position, direction). 
      
   In the above cases, specified line windows are applied to all science 
   spws. In case when line windows vary with spw, line windows can be 
   specified by a dictionary whose key is spw id while value is line window.
   For example, the following dictionary gives different line windows 
   to spws 17 and 19. Other spws, if available, will have an empty line 
   window.
   
      {17: [[100, 200], [1200, 1400]], 19: ['112115MHz', '112116MHz']}
      
   Furthermore, linewindow accepts MS selection string. The following 
   string gives [[100,200],[1200,1400]] for spw 17 while [1000,1500] 
   for spw 21.
   
      "17:100~200;1200~1400,21:1000~1500"
      
   The string also accepts frequency with units. Note, however, that 
   frequency reference frame in this case is not fixed to LSRK. 
   Instead, the frame will be taken from the MS (typically TOPO for ALMA). 
   Thus, the following two frequency-based line windows result different 
   channel selections.
   
      {19: ['112115MHz', '112116MHz']} # frequency frame is LSRK
      "19:11215MHz~11216MHz" # frequency frame is taken from the data
                             # (TOPO for ALMA)
   
   default: [] (do line detection and validation)
   example: [100,200] (channel), [115e9, 115.1e9] (frequency in Hz)
            ['115GHz', '115.1GHz'], see above for more examples

linewindowmode -- Merge or replace given manual line window with line 
   detection/validation result. If 'replace' is given, line detection 
   and validation will not be performed. On the other hand, when 
   'merge' is specified, line detection/validation will be performed 
   and manually specified line windows are added to the result. 
   Note that this has no effect when linewindow for target spw is 
   empty. In that case, line detection/validation will be performed 
   regardless of the value of linewindowmode.
   default: 'replace'
   options: 'replace', 'merge'

edge -- number of edge channels to be dropped from baseline subtraction.
   the value must be a list with length of 2, whose values specifies
   left and right edge channels respectively.
   default: [] ([0,0])
   example: [10,10]

broadline -- Try to detect broad component of spectral line if True.
   default: True

clusteringalgorithm -- selection of the algorithm used in the clustering
   analysis to check the validity of detected line features. 'kmean'
   algorithm and hierarchical clustering algorithm 'hierarchy' are so
   far implemented.
   default: 'kmean'.
   
deviationmask -- Apply deviation mask in addition to masks determined by 
   the automatic line detection.
   default: True

pipelinemode -- The pipeline operating mode. In 'automatic' mode the 
   pipeline determines the values of all context defined pipeline inputs
   automatically.  In 'interactive' mode the user can set the pipeline
   context defined parameters manually.  In 'getinputs' mode the user
   can check the settings of all pipeline parameters without running
   the task.
   default: 'automatic'.

---- pipeline context defined parameter argument which can be set only in
'interactive mode'

infiles -- List of data files. These must be a name of Scantables that 
   are registered to context via hsd_importdata task.
   default: []
   example: vis=['X227.ms', 'X228.ms']
   
field -- Data selection by field.
   default: '' (all fields)
   example: '1' (select by FIELD_ID)
            'M100*' (select by field name)

antenna -- Data selection by antenna.
   default: '' (all antennas)
   example: '1' (select by ANTENNA_ID)
            'PM03' (select by antenna name)

spw -- Data selection by spw.
   default: '' (all spws)
   example: '3,4' (generate caltable for spw 3 and 4)
            ['0','2'] (spw 0 for first data, 2 for second)

pol -- Data selection by pol.
   default: '' (all polarizations)
   example: '0' (generate caltable for pol 0)
            ['0~1','0'] (pol 0 and 1 for first data, only 0 for second)

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
        self.__globals__['__last_task'] = 'hsd_baseline'
        self.__globals__['taskname'] = 'hsd_baseline'
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

            myparams['fitfunc'] = fitfunc = self.parameters['fitfunc']
            myparams['fitorder'] = fitorder = self.parameters['fitorder']
            myparams['linewindow'] = linewindow = self.parameters['linewindow']
            myparams['linewindowmode'] = linewindowmode = self.parameters['linewindowmode']
            myparams['edge'] = edge = self.parameters['edge']
            myparams['broadline'] = broadline = self.parameters['broadline']
            myparams['clusteringalgorithm'] = clusteringalgorithm = self.parameters['clusteringalgorithm']
            myparams['deviationmask'] = deviationmask = self.parameters['deviationmask']
            myparams['pipelinemode'] = pipelinemode = self.parameters['pipelinemode']
            myparams['infiles'] = infiles = self.parameters['infiles']
            myparams['field'] = field = self.parameters['field']
            myparams['antenna'] = antenna = self.parameters['antenna']
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

        mytmp['fitfunc'] = fitfunc
        mytmp['fitorder'] = fitorder
        mytmp['linewindow'] = linewindow
        mytmp['linewindowmode'] = linewindowmode
        mytmp['edge'] = edge
        mytmp['broadline'] = broadline
        mytmp['clusteringalgorithm'] = clusteringalgorithm
        mytmp['deviationmask'] = deviationmask
        mytmp['pipelinemode'] = pipelinemode
        mytmp['infiles'] = infiles
        mytmp['field'] = field
        mytmp['antenna'] = antenna
        mytmp['spw'] = spw
        mytmp['pol'] = pol
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
        mytmp['parallel'] = parallel
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli/"
	trec = casac.casac.utils().torecord(pathname+'hsd_baseline.xml')

        casalog.origin('hsd_baseline')
	try :
          #if not trec.has_key('hsd_baseline') or not casac.casac.utils().verify(mytmp, trec['hsd_baseline']) :
	    #return False

          casac.casac.utils().verify(mytmp, trec['hsd_baseline'], True)
          scriptstr=['']
          saveinputs = self.__globals__['saveinputs']
          if type(self.__call__.func_defaults) is NoneType:
              saveinputs=''
          else:
              saveinputs('hsd_baseline', 'hsd_baseline.last', myparams, self.__globals__,scriptstr=scriptstr)
          tname = 'hsd_baseline'
          spaces = ' '*(18-len(tname))
          casalog.post('\n##########################################'+
                       '\n##### Begin Task: ' + tname + spaces + ' #####')
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('Begin Task: ' + tname)
          if type(self.__call__.func_defaults) is NoneType:
              casalog.post(scriptstr[0]+'\n', 'INFO')
          else :
              casalog.post(scriptstr[1][1:]+'\n', 'INFO')
          result = hsd_baseline(fitfunc, fitorder, linewindow, linewindowmode, edge, broadline, clusteringalgorithm, deviationmask, pipelinemode, infiles, field, antenna, spw, pol, dryrun, acceptresults, parallel)
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('End Task: ' + tname)
          casalog.post('##### End Task: ' + tname + '  ' + spaces + ' #####'+
                       '\n##########################################')

	except Exception, instance:
          if(self.__globals__.has_key('__rethrow_casa_exceptions') and self.__globals__['__rethrow_casa_exceptions']) :
             raise
          else :
             #print '**** Error **** ',instance
	     tname = 'hsd_baseline'
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
#        paramgui.runTask('hsd_baseline', myf['_ip'])
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
        a['fitfunc']  = 'cspline'
        a['fitorder']  = -1
        a['linewindow']  = ''
        a['edge']  = []
        a['broadline']  = True
        a['clusteringalgorithm']  = 'kmean'
        a['deviationmask']  = True
        a['pipelinemode']  = 'automatic'

        a['linewindow'] = {
                    0:odict([{'notvalue':''}, {'linewindowmode':'replace'}])}
        a['pipelinemode'] = {
                    0:{'value':'automatic'}, 
                    1:odict([{'value':'interactive'}, {'infiles':[]}, {'field':''}, {'antenna':''}, {'spw':''}, {'pol':''}, {'parallel':'automatic'}, {'dryrun':False}, {'acceptresults':True}]), 
                    2:odict([{'value':'getinputs'}, {'infiles':[]}, {'field':''}, {'antenna':''}, {'spw':''}, {'pol':''}])}

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
    def description(self, key='hsd_baseline', subkey=None):
        desc={'hsd_baseline': 'Detect and validate spectral lines, subtract baseline by masking detected lines',
               'fitfunc': 'Fitting function for baseline subtraction',
               'fitorder': 'Fitting order for baseline subtraction',
               'linewindow': 'Pre-defined line window',
               'linewindowmode': 'Merge or replace given manual line window with line detection/validation result',
               'edge': 'Edge channels to be dropped',
               'broadline': 'Try to detect broad component of the line',
               'clusteringalgorithm': 'Algorithm for line validation clustering algorithm',
               'deviationmask': 'Apply deviation mask in addition to detected line masks',
               'pipelinemode': 'The pipeline operating mode',
               'infiles': 'List of input files to be calibrated (default all)',
               'field': 'select data by field',
               'antenna': 'select data by antenna',
               'spw': 'select data by IF IDs (spectral windows), e.g. \'3,5,7\' (\'\'=all)',
               'pol': 'select data by polarizations, e.g. \'0~1\' (\'\'=all)',
               'dryrun': 'Run the task (False) or display task command (True)',
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
        a['fitfunc']  = 'cspline'
        a['fitorder']  = -1
        a['linewindow']  = ''
        a['linewindowmode']  = 'replace'
        a['edge']  = []
        a['broadline']  = True
        a['clusteringalgorithm']  = 'kmean'
        a['deviationmask']  = True
        a['pipelinemode']  = 'automatic'
        a['infiles']  = ['']
        a['field']  = ''
        a['antenna']  = ''
        a['spw']  = ''
        a['pol']  = ''
        a['dryrun']  = False
        a['acceptresults']  = True
        a['parallel']  = 'automatic'

        #a = sys._getframe(len(inspect.stack())-1).f_globals

        if self.parameters['linewindow']  != '':
            a['linewindowmode'] = 'replace'

        if self.parameters['pipelinemode']  == 'interactive':
            a['infiles'] = []
            a['field'] = ''
            a['antenna'] = ''
            a['spw'] = ''
            a['pol'] = ''
            a['parallel'] = 'automatic'
            a['dryrun'] = False
            a['acceptresults'] = True

        if self.parameters['pipelinemode']  == 'getinputs':
            a['infiles'] = []
            a['field'] = ''
            a['antenna'] = ''
            a['spw'] = ''
            a['pol'] = ''

        if a.has_key(paramname) :
	      return a[paramname]
hsd_baseline_cli = hsd_baseline_cli_()
