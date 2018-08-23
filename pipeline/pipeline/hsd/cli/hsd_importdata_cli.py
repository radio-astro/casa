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
from task_hsd_importdata import hsd_importdata
class hsd_importdata_cli_:
    __name__ = "hsd_importdata"
    rkey = None
    i_am_a_casapy_task = None
    # The existence of the i_am_a_casapy_task attribute allows help()
    # (and other) to treat casapy tasks as a special case.

    def __init__(self) :
       self.__bases__ = (hsd_importdata_cli_,)
       self.__doc__ = self.__call__.__doc__

       self.parameters={'vis':None, 'session':None, 'pipelinemode':None, 'asis':None, 'process_caldevice':None, 'overwrite':None, 'nocopy':None, 'bdfflags':None, 'lazy':None, 'with_pointing_correction':None, 'createmms':None, 'dryrun':None, 'acceptresults':None, }


    def result(self, key=None):
	    #### and add any that have completed...
	    return None


    def __call__(self, vis=None, session=None, pipelinemode=None, asis=None, process_caldevice=None, overwrite=None, nocopy=None, bdfflags=None, lazy=None, with_pointing_correction=None, createmms=None, dryrun=None, acceptresults=None, ):

        """Imports data into the single dish pipeline

	Detailed Description:


	Arguments :
		vis:	List of input visibility data
		   Default Value: 

		session:	List of visibility data sessions
		   Default Value: 

		pipelinemode:	The pipeline operating mode
		   Default Value: automatic
		   Allowed Values:
				automatic
				interactive
				getinputs

		asis:	ASDM to convert as is
		   Default Value: SBSummary ExecBlock Antenna Station Receiver Source CalAtmosphere CalWVR

		process_caldevice:	Import the caldevice table from the ASDM
		   Default Value: False

		overwrite:	Overwrite existing files on import
		   Default Value: False

		nocopy:	Disable copying of MS to working directory
		   Default Value: False

		bdfflags:	Apply BDF flags on import
		   Default Value: True

		lazy:	Use the lazy import option
		   Default Value: False

		with_pointing_correction:	Apply pointing correction to DIRECTION
		   Default Value: True

		createmms:	Create a MMS
		   Default Value: false
		   Allowed Values:
				automatic
				true
				false

		dryrun:	Run the task (False) or display task command (True)
		   Default Value: False

		acceptresults:	Add the results into the pipeline context
		   Default Value: True

	Returns: void

	Example :

The hsd_importdata task loads the specified visibility data into the pipeline
context unpacking and / or converting it as necessary.

Keyword arguments:

---- pipeline parameter arguments which can be set in any pipeline mode

vis -- List of visibility data files. These may be ASDMs, tar files of ASDMs,
   MSs, or tar files of MSs, If ASDM files are specified, they will be
   converted  to MS format.
   default: []
   example: vis=['X227.ms', 'asdms.tar.gz']

session -- List of sessions to which the visibility files belong. Defaults
   to a single session containing all the visibility files, otherwise
   a session must be assigned to each vis file.
   default: []
   example: session=['Session_1', 'Sessions_2']

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context defined pipeline inputs
   automatically.  In 'interactive' mode the user can set the pipeline
   context defined parameters manually.  In 'getinputs' mode the user
   can check the settings of all pipeline parameters without running
   the task.
   default: 'automatic'.

---- pipeline context defined parameter argument which can be set only in
'interactive mode'

asis -- ASDM tables to convert as is
   default: 'Antenna Station Receiver CalAtmosphere'
   example: 'Receiver', ''

process_caldevice -- Ingest the ASDM caldevice table
   default: False
   example: True

overwrite -- Overwrite existing MSs on output.
   default: False

bdfflags -- Apply BDF flags on line
   default: True
   
lazy -- Use the lazy filter import
   default: False

with_pointing_correction -- add (ASDM::Pointing::encoder - ASDM::Pointing::pointingDirection) 
   to the value to be written in MS::Pointing::direction
   default: True

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

1. Load an ASDM list in the ../rawdata subdirectory into the context.

   hsd_importdata (vis=['../rawdata/uid___A002_X30a93d_X43e',
                '../rawdata/uid_A002_x30a93d_X44e'])

2. Load an MS in the current directory into the context.

   hsd_importdata (vis=[uid___A002_X30a93d_X43e.ms])

3. Load a tarred ASDM in ../rawdata into the context.

   hsd_importdata (vis=['../rawdata/uid___A002_X30a93d_X43e.tar.gz'])

4. Check the hsd_importdata inputs, then import the data

   myvislist = ['uid___A002_X30a93d_X43e.ms', 'uid_A002_x30a93d_X44e.ms']
   hsd_importdata(vis=myvislist, pipelinemode='getinputs')
   hsd_importdata(vis=myvislist)

5. Load an ASDM but check the results before accepting them into the context.

   results = hsd_importdata (vis=['uid___A002_X30a93d_X43e.ms'],
       acceptresults=False)
   results.accept()

6. Run in  dryrun mode before running for real
   results = hsd_importdata (vis=['uid___A002_X30a93d_X43e.ms'], dryrun=True)
   results = hsd_importdata (vis=['uid___A002_X30a93d_X43e.ms'])


        """
	if not hasattr(self, "__globals__") or self.__globals__ == None :
           self.__globals__=stack_frame_find( )
	#casac = self.__globals__['casac']
	casalog = self.__globals__['casalog']
	casa = self.__globals__['casa']
	#casalog = casac.casac.logsink()
        self.__globals__['__last_task'] = 'hsd_importdata'
        self.__globals__['taskname'] = 'hsd_importdata'
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
            myparams['pipelinemode'] = pipelinemode = self.parameters['pipelinemode']
            myparams['asis'] = asis = self.parameters['asis']
            myparams['process_caldevice'] = process_caldevice = self.parameters['process_caldevice']
            myparams['overwrite'] = overwrite = self.parameters['overwrite']
            myparams['nocopy'] = nocopy = self.parameters['nocopy']
            myparams['bdfflags'] = bdfflags = self.parameters['bdfflags']
            myparams['lazy'] = lazy = self.parameters['lazy']
            myparams['with_pointing_correction'] = with_pointing_correction = self.parameters['with_pointing_correction']
            myparams['createmms'] = createmms = self.parameters['createmms']
            myparams['dryrun'] = dryrun = self.parameters['dryrun']
            myparams['acceptresults'] = acceptresults = self.parameters['acceptresults']

        if type(vis)==str: vis=[vis]
        if type(session)==str: session=[session]

	result = None

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['session'] = session
        mytmp['pipelinemode'] = pipelinemode
        mytmp['asis'] = asis
        mytmp['process_caldevice'] = process_caldevice
        mytmp['overwrite'] = overwrite
        mytmp['nocopy'] = nocopy
        mytmp['bdfflags'] = bdfflags
        mytmp['lazy'] = lazy
        mytmp['with_pointing_correction'] = with_pointing_correction
        mytmp['createmms'] = createmms
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli/"
	trec = casac.casac.utils().torecord(pathname+'hsd_importdata.xml')

        casalog.origin('hsd_importdata')
	try :
          #if not trec.has_key('hsd_importdata') or not casac.casac.utils().verify(mytmp, trec['hsd_importdata']) :
	    #return False

          casac.casac.utils().verify(mytmp, trec['hsd_importdata'], True)
          scriptstr=['']
          saveinputs = self.__globals__['saveinputs']
          if type(self.__call__.func_defaults) is NoneType:
              saveinputs=''
          else:
              saveinputs('hsd_importdata', 'hsd_importdata.last', myparams, self.__globals__,scriptstr=scriptstr)
          tname = 'hsd_importdata'
          spaces = ' '*(18-len(tname))
          casalog.post('\n##########################################'+
                       '\n##### Begin Task: ' + tname + spaces + ' #####')
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('Begin Task: ' + tname)
          if type(self.__call__.func_defaults) is NoneType:
              casalog.post(scriptstr[0]+'\n', 'INFO')
          else :
              casalog.post(scriptstr[1][1:]+'\n', 'INFO')
          result = hsd_importdata(vis, session, pipelinemode, asis, process_caldevice, overwrite, nocopy, bdfflags, lazy, with_pointing_correction, createmms, dryrun, acceptresults)
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('End Task: ' + tname)
          casalog.post('##### End Task: ' + tname + '  ' + spaces + ' #####'+
                       '\n##########################################')

	except Exception, instance:
          if(self.__globals__.has_key('__rethrow_casa_exceptions') and self.__globals__['__rethrow_casa_exceptions']) :
             raise
          else :
             #print '**** Error **** ',instance
	     tname = 'hsd_importdata'
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
#        paramgui.runTask('hsd_importdata', myf['_ip'])
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
        a['vis']  = ['']
        a['session']  = ['']
        a['pipelinemode']  = 'automatic'
        a['lazy']  = False
        a['with_pointing_correction']  = True
        a['createmms']  = 'false'

        a['pipelinemode'] = {
                    0:{'value':'automatic'}, 
                    1:odict([{'value':'interactive'}, {'asis':'SBSummary ExecBlock Antenna Station Receiver Source CalAtmosphere CalWVR'}, {'process_caldevice':False}, {'overwrite':False}, {'nocopy':False}, {'bdfflags':True}, {'dryrun':False}, {'acceptresults':True}]), 
                    2:odict([{'value':'getinputs'}, {'asis':'SBSummary ExecBlock Antenna Station Receiver Source CalAtmosphere CalWVR'}, {'process_caldevice':False}, {'overwrite':False}, {'nocopy':False}, {'bdfflags':True}])}

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
    def description(self, key='hsd_importdata', subkey=None):
        desc={'hsd_importdata': 'Imports data into the single dish pipeline',
               'vis': 'List of input visibility data',
               'session': 'List of visibility data sessions',
               'pipelinemode': 'The pipeline operating mode',
               'asis': 'ASDM to convert as is',
               'process_caldevice': 'Import the caldevice table from the ASDM',
               'overwrite': 'Overwrite existing files on import',
               'nocopy': 'Disable copying of MS to working directory',
               'bdfflags': 'Apply BDF flags on import',
               'lazy': 'Use the lazy import option',
               'with_pointing_correction': 'Apply pointing correction to DIRECTION',
               'createmms': 'Create a MMS',
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
        a['pipelinemode']  = 'automatic'
        a['asis']  = 'SBSummary ExecBlock Antenna Station Receiver Source CalAtmosphere CalWVR'
        a['process_caldevice']  = False
        a['overwrite']  = False
        a['nocopy']  = False
        a['bdfflags']  = True
        a['lazy']  = False
        a['with_pointing_correction']  = True
        a['createmms']  = 'false'
        a['dryrun']  = False
        a['acceptresults']  = True

        #a = sys._getframe(len(inspect.stack())-1).f_globals

        if self.parameters['pipelinemode']  == 'interactive':
            a['asis'] = 'SBSummary ExecBlock Antenna Station Receiver Source CalAtmosphere CalWVR'
            a['process_caldevice'] = False
            a['overwrite'] = False
            a['nocopy'] = False
            a['bdfflags'] = True
            a['dryrun'] = False
            a['acceptresults'] = True

        if self.parameters['pipelinemode']  == 'getinputs':
            a['asis'] = 'SBSummary ExecBlock Antenna Station Receiver Source CalAtmosphere CalWVR'
            a['process_caldevice'] = False
            a['overwrite'] = False
            a['nocopy'] = False
            a['bdfflags'] = True

        if a.has_key(paramname) :
	      return a[paramname]
hsd_importdata_cli = hsd_importdata_cli_()
