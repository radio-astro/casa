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
from task_hifa_exportdata import hifa_exportdata
class hifa_exportdata_cli_:
    __name__ = "hifa_exportdata"
    rkey = None
    i_am_a_casapy_task = None
    # The existence of the i_am_a_casapy_task attribute allows help()
    # (and other) to treat casapy tasks as a special case.

    def __init__(self) :
       self.__bases__ = (hifa_exportdata_cli_,)
       self.__doc__ = self.__call__.__doc__

       self.parameters={'vis':None, 'session':None, 'imaging_products_only':None, 'exportmses':None, 'pprfile':None, 'calintents':None, 'calimages':None, 'targetimages':None, 'products_dir':None, 'pipelinemode':None, 'dryrun':None, 'acceptresults':None, }


    def result(self, key=None):
	    #### and add any that have completed...
	    return None


    def __call__(self, vis=None, session=None, imaging_products_only=None, exportmses=None, pprfile=None, calintents=None, calimages=None, targetimages=None, products_dir=None, pipelinemode=None, dryrun=None, acceptresults=None, ):

        """Prepare interferometry data for export

	Detailed Description:


	Arguments :
		vis:	List of input visibility data
		   Default Value: 

		session:	List of sessions one per visibility file
		   Default Value: 

		imaging_products_only:	Export science target imaging products only
		   Default Value: False

		exportmses:	Export final MeasurementSets instead of flags and caltables
		   Default Value: False

		pprfile:	The pipeline processing request file to be exported
		   Default Value: 

		calintents:	The calibrater source target intents to be exported
		   Default Value: 

		calimages:	List of calibrator images to be exported
		   Default Value: 

		targetimages:	List of target images to be exported
		   Default Value: 

		products_dir:	The data products directory
		   Default Value: 

		pipelinemode:	The pipeline operating mode
		   Default Value: automatic
		   Allowed Values:
				automatic
				interactive
				getinputs

		dryrun:	Run the task (False) or display task command (True)
		   Default Value: False

		acceptresults:	Add the results into the pipeline context
		   Default Value: True

	Returns: void

	Example :

The hif_exportdata task exports the data defined in the pipeline context
and exports it to the data products directory, converting and or
packing it as necessary.

Keyword arguments:

---- pipeline parameter arguments which can be set in any pipeline mode

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context defined pipeline inputs automatically.
   In 'interactive' mode the user can set the pipeline context defined
   parameters manually.  In 'getinputs' mode the user can check the settings
   of all pipeline parameters without running the task.
   default: 'automatic'.

---- pipeline context defined parameter argument which can be set only in
'interactive mode'

vis -- List of visibility data files for which flagging and calibration
   information will be exported. Defaults to the list maintained in the
   pipeline context.
   default: []
   example: vis=['X227.ms', 'X228.ms']

session -- List of sessions one per visibility file. Currently defaults
   to a single virtual session containing all the visibility files in vis. 
   In future will default to set of observing sessions defined in the 
   context.
   default: []
   example: session=['session1', 'session2']

exportmses -- Export the final MeasurementSets instead of the final flags,
   calibration tables, and calibration instructions.
   default: False
   example: exportmses = True

pprfile -- Name of the pipeline processing request to be exported. Defaults
   to a file matching the template 'PPR_*.xml'.
   default: []
   example: pprfile=['PPR_GRB021004.xml']

calintents -- List of calibrator image types to be exported. Defaults to
   all standard calibrator intents 'BANDPASS', 'PHASE', 'FLUX'
   default: ''
   example: calintents='PHASE'

calimages -- List of calibrator images to be exported. Defaults to all
   calibrator images recorded in the pipeline context.
   default: []
   example: calimages=['3C454.3.bandpass', '3C279.phase']

targetimages -- List of science target images to be exported. Defaults to all
   science target images recorded in the pipeline context.
   default: []
   example: targetimages=['NGC3256.band3', 'NGC3256.band6']

products_dir -- Name of the data products subdirectory. Defaults to './'
   default: ''
   example: products_dir='../products'


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

The hif_exportdata task exports the data defined in the pipeline context
and exports it to the data products directory, converting and or packing
it as necessary.

The current version of the task exports the following products

o an XML file containing the pipeline processing request
o a tar file per ASDM / MS containing the final flags version
o a text file per ASDM / MS containing the final calibration apply list
o a FITS image for each selected calibrator source image
o a FITS image for each selected science target source image
o a tar file per session containing the caltables for that session
o a tar file containing the file web log
o a text file containing the final list of CASA commands


Issues

Support for merging the calibration state information into the pipeline
context / results structure and retrieving it still needs to be added.

Support for merging the clean results into the pipeline context / results
structure and retrieving it still needs to be added.

Support for creating the final pipeline results entity still needs to
be added.
    
Session information is not currently handled by the pipeline context.
By default all ASDMs are combined into one session.


Examples

1. Export the pipeline results for a single sessions to the data products
directory

    !mkdir ../products
    hif_exportdata (products_dir='../products')

2. Export the pipeline results to the data products directory specify that
only the gain calibrator images be saved.

    !mkdir ../products
    hif_exportdata (products_dir='../products', calintents='*PHASE*')


        """
	if not hasattr(self, "__globals__") or self.__globals__ == None :
           self.__globals__=stack_frame_find( )
	#casac = self.__globals__['casac']
	casalog = self.__globals__['casalog']
	casa = self.__globals__['casa']
	#casalog = casac.casac.logsink()
        self.__globals__['__last_task'] = 'hifa_exportdata'
        self.__globals__['taskname'] = 'hifa_exportdata'
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
            myparams['session'] = session = self.parameters['session']
            myparams['imaging_products_only'] = imaging_products_only = self.parameters['imaging_products_only']
            myparams['exportmses'] = exportmses = self.parameters['exportmses']
            myparams['pprfile'] = pprfile = self.parameters['pprfile']
            myparams['calintents'] = calintents = self.parameters['calintents']
            myparams['calimages'] = calimages = self.parameters['calimages']
            myparams['targetimages'] = targetimages = self.parameters['targetimages']
            myparams['products_dir'] = products_dir = self.parameters['products_dir']
            myparams['pipelinemode'] = pipelinemode = self.parameters['pipelinemode']
            myparams['dryrun'] = dryrun = self.parameters['dryrun']
            myparams['acceptresults'] = acceptresults = self.parameters['acceptresults']

        if type(vis)==str: vis=[vis]
        if type(session)==str: session=[session]
        if type(calimages)==str: calimages=[calimages]
        if type(targetimages)==str: targetimages=[targetimages]

	result = None

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['session'] = session
        mytmp['imaging_products_only'] = imaging_products_only
        mytmp['exportmses'] = exportmses
        mytmp['pprfile'] = pprfile
        mytmp['calintents'] = calintents
        mytmp['calimages'] = calimages
        mytmp['targetimages'] = targetimages
        mytmp['products_dir'] = products_dir
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli/"
	trec = casac.casac.utils().torecord(pathname+'hifa_exportdata.xml')

        casalog.origin('hifa_exportdata')
	try :
          #if not trec.has_key('hifa_exportdata') or not casac.casac.utils().verify(mytmp, trec['hifa_exportdata']) :
	    #return False

          casac.casac.utils().verify(mytmp, trec['hifa_exportdata'], True)
          scriptstr=['']
          saveinputs = self.__globals__['saveinputs']
          if type(self.__call__.func_defaults) is NoneType:
              saveinputs=''
          else:
              saveinputs('hifa_exportdata', 'hifa_exportdata.last', myparams, self.__globals__,scriptstr=scriptstr)
          tname = 'hifa_exportdata'
          spaces = ' '*(18-len(tname))
          casalog.post('\n##########################################'+
                       '\n##### Begin Task: ' + tname + spaces + ' #####')
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('Begin Task: ' + tname)
          if type(self.__call__.func_defaults) is NoneType:
              casalog.post(scriptstr[0]+'\n', 'INFO')
          else :
              casalog.post(scriptstr[1][1:]+'\n', 'INFO')
          result = hifa_exportdata(vis, session, imaging_products_only, exportmses, pprfile, calintents, calimages, targetimages, products_dir, pipelinemode, dryrun, acceptresults)
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('End Task: ' + tname)
          casalog.post('##### End Task: ' + tname + '  ' + spaces + ' #####'+
                       '\n##########################################')

	except Exception, instance:
          if(self.__globals__.has_key('__rethrow_casa_exceptions') and self.__globals__['__rethrow_casa_exceptions']) :
             raise
          else :
             #print '**** Error **** ',instance
	     tname = 'hifa_exportdata'
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
#        paramgui.runTask('hifa_exportdata', myf['_ip'])
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
        a['imaging_products_only']  = False
        a['exportmses']  = False
        a['pipelinemode']  = 'automatic'

        a['pipelinemode'] = {
                    0:{'value':'automatic'}, 
                    1:odict([{'value':'interactive'}, {'vis':[]}, {'session':[]}, {'pprfile':''}, {'calintents':''}, {'calimages':[]}, {'targetimages':[]}, {'products_dir':''}, {'dryrun':False}, {'acceptresults':True}]), 
                    2:odict([{'value':'getinputs'}, {'vis':[]}, {'session':[]}, {'pprfile':''}, {'calintents':''}, {'calimages':[]}, {'targetimages':[]}, {'products_dir':''}])}

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
    def description(self, key='hifa_exportdata', subkey=None):
        desc={'hifa_exportdata': 'Prepare interferometry data for export',
               'vis': 'List of input visibility data',
               'session': 'List of sessions one per visibility file',
               'imaging_products_only': 'Export science target imaging products only',
               'exportmses': 'Export final MeasurementSets instead of flags and caltables',
               'pprfile': 'The pipeline processing request file to be exported',
               'calintents': 'The calibrater source target intents to be exported',
               'calimages': 'List of calibrator images to be exported',
               'targetimages': 'List of target images to be exported',
               'products_dir': 'The data products directory',
               'pipelinemode': 'The pipeline operating mode',
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
        a['vis']  = ['']
        a['session']  = ['']
        a['imaging_products_only']  = False
        a['exportmses']  = False
        a['pprfile']  = ''
        a['calintents']  = ''
        a['calimages']  = ['']
        a['targetimages']  = ['']
        a['products_dir']  = ''
        a['pipelinemode']  = 'automatic'
        a['dryrun']  = False
        a['acceptresults']  = True

        #a = sys._getframe(len(inspect.stack())-1).f_globals

        if self.parameters['pipelinemode']  == 'interactive':
            a['vis'] = []
            a['session'] = []
            a['pprfile'] = ''
            a['calintents'] = ''
            a['calimages'] = []
            a['targetimages'] = []
            a['products_dir'] = ''
            a['dryrun'] = False
            a['acceptresults'] = True

        if self.parameters['pipelinemode']  == 'getinputs':
            a['vis'] = []
            a['session'] = []
            a['pprfile'] = ''
            a['calintents'] = ''
            a['calimages'] = []
            a['targetimages'] = []
            a['products_dir'] = ''

        if a.has_key(paramname) :
	      return a[paramname]
hifa_exportdata_cli = hifa_exportdata_cli_()
