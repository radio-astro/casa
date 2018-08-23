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
from task_hifa_antpos import hifa_antpos
class hifa_antpos_cli_:
    __name__ = "hifa_antpos"
    rkey = None
    i_am_a_casapy_task = None
    # The existence of the i_am_a_casapy_task attribute allows help()
    # (and other) to treat casapy tasks as a special case.

    def __init__(self) :
       self.__bases__ = (hifa_antpos_cli_,)
       self.__doc__ = self.__call__.__doc__

       self.parameters={'vis':None, 'caltable':None, 'hm_antpos':None, 'antenna':None, 'offsets':None, 'antposfile':None, 'pipelinemode':None, 'dryrun':None, 'acceptresults':None, }


    def result(self, key=None):
	    #### and add any that have completed...
	    return None


    def __call__(self, vis=None, caltable=None, hm_antpos=None, antenna=None, offsets=None, antposfile=None, pipelinemode=None, dryrun=None, acceptresults=None, ):

        """Derive an antenna position calibration table

	Detailed Description:

Derive the antenna position calibration for list of MeasurementSets

	Arguments :
		vis:	List of input MeasurementSets
		   Default Value: 

		caltable:	List of output caltable(s)
		   Default Value: 

		hm_antpos:	The antenna position determination method
		   Default Value: file
		   Allowed Values:
				file
				manual
				online

		antenna:	List of antennas to be corrected
		   Default Value: 

		offsets:	List of position corrections one set per antenna
		   Default Value: 

		antposfile:	File containing antenna position corrections
		   Default Value: 

		pipelinemode:	The pipeline operation mode
		   Default Value: automatic
		   Allowed Values:
				automatic
				interactive
				getinputs

		dryrun:	Run the task (False) or list commands(True)
		   Default Value: False

		acceptresults:	Automatically accept results into context
		   Default Value: True

	Returns: void

	Example :


Keyword arguments:

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context dependent pipeline inputs automatically.
   In interactive mode the user can set the pipeline context defined parameters
   manually.  In 'getinputs' mode the user can check the settings of all
   pipeline parameters without running the task.
   default: 'automatic'.

---- pipeline parameter arguments which can be set in any pipeline mode

hm_antpos -- Heuristics method for retrieving the antenna position
    corrections. The options are 'online' (not yet implemented), 'manual',
    and 'file'.
    default: 'file'
    example: hm_antpos='manual'

antenna -- The list of antennas for which the positions are to be corrected
    if hm_antpos is 'manual'
    default: none
    example 'DV05,DV07'

offsets -- The list of antenna offsets for each antenna in 'antennas'. Each
    offset is a set of 3 floating point numbers separated by commas, specified
    in the ITRF frame.
    default: none
    example: [0.01, 0.02, 0.03, 0.03, 0.02, 0.01] 

antposfile -- The file(s) containing the antenna offsets. Used if hm_antpos
    is 'file'. The default file name is 'antennapos.csv'

---- pipeline context defined parameter arguments which can be set only in
'interactive mode'

vis -- List of input visibility files
    default: []
    example: ['ngc5921.ms']

caltable -- Name of output gain calibration tables
    default: []
    example: caltable=['ngc5921.gcal']

-- Pipeline task execution modes

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

The hifa_antpos task corrects the antenna positions recorded in the ASDMs using
updated antenna position calibration information determined after the
observation was taken.

Corrections can be input by hand, read from a file on disk, or in future
by querying an ALMA database service.

The antenna positions file is in 'csv' format containing 6 comma delimited
columns as shown below. The default name of this file is 'antennapos.csv'

List if sample antennapos.csv file

ms,antenna,xoffset,yoffset,zoffset,comment
uid___A002_X30a93d_X43e.ms,DV11,0.000,0.010,0.000,"No comment"
uid___A002_X30a93d_X43e.dup.ms,DV11,0.000,-0.010,0.000,"No comment"

The corrections are used to generate a calibration table which is recorded
in the pipeline context and applied to the raw visibility data, on the fly to
generate other calibration tables, or permanently to generate calibrated
visibilities for imaging.


Issues

The hm_antpos 'online' option will be  implemented when the observing system
provides an antenna position determination service.


Example

1. Correct the position of antenna 5 for all the visibility files in a single
pipeline run.  

    hifa_antpos (antenna='DV05', offsets=[0.01, 0.02, 0.03])

2. Correct the position of antennas for all the visibility files in a single
pipeline run using antenna positions files on disk. These files are assumed
to conform to a default naming scheme if 'antposfile' is unspecified by the
user.

    hifa_antpos (hm_antpos='myantposfile.csv')


        """
	if not hasattr(self, "__globals__") or self.__globals__ == None :
           self.__globals__=stack_frame_find( )
	#casac = self.__globals__['casac']
	casalog = self.__globals__['casalog']
	casa = self.__globals__['casa']
	#casalog = casac.casac.logsink()
        self.__globals__['__last_task'] = 'hifa_antpos'
        self.__globals__['taskname'] = 'hifa_antpos'
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
            myparams['hm_antpos'] = hm_antpos = self.parameters['hm_antpos']
            myparams['antenna'] = antenna = self.parameters['antenna']
            myparams['offsets'] = offsets = self.parameters['offsets']
            myparams['antposfile'] = antposfile = self.parameters['antposfile']
            myparams['pipelinemode'] = pipelinemode = self.parameters['pipelinemode']
            myparams['dryrun'] = dryrun = self.parameters['dryrun']
            myparams['acceptresults'] = acceptresults = self.parameters['acceptresults']

        if type(vis)==str: vis=[vis]
        if type(caltable)==str: caltable=[caltable]
        if type(offsets)==float: offsets=[offsets]

	result = None

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['caltable'] = caltable
        mytmp['hm_antpos'] = hm_antpos
        mytmp['antenna'] = antenna
        mytmp['offsets'] = offsets
        mytmp['antposfile'] = antposfile
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli/"
	trec = casac.casac.utils().torecord(pathname+'hifa_antpos.xml')

        casalog.origin('hifa_antpos')
	try :
          #if not trec.has_key('hifa_antpos') or not casac.casac.utils().verify(mytmp, trec['hifa_antpos']) :
	    #return False

          casac.casac.utils().verify(mytmp, trec['hifa_antpos'], True)
          scriptstr=['']
          saveinputs = self.__globals__['saveinputs']
          if type(self.__call__.func_defaults) is NoneType:
              saveinputs=''
          else:
              saveinputs('hifa_antpos', 'hifa_antpos.last', myparams, self.__globals__,scriptstr=scriptstr)
          tname = 'hifa_antpos'
          spaces = ' '*(18-len(tname))
          casalog.post('\n##########################################'+
                       '\n##### Begin Task: ' + tname + spaces + ' #####')
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('Begin Task: ' + tname)
          if type(self.__call__.func_defaults) is NoneType:
              casalog.post(scriptstr[0]+'\n', 'INFO')
          else :
              casalog.post(scriptstr[1][1:]+'\n', 'INFO')
          result = hifa_antpos(vis, caltable, hm_antpos, antenna, offsets, antposfile, pipelinemode, dryrun, acceptresults)
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('End Task: ' + tname)
          casalog.post('##### End Task: ' + tname + '  ' + spaces + ' #####'+
                       '\n##########################################')

	except Exception, instance:
          if(self.__globals__.has_key('__rethrow_casa_exceptions') and self.__globals__['__rethrow_casa_exceptions']) :
             raise
          else :
             #print '**** Error **** ',instance
	     tname = 'hifa_antpos'
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
#        paramgui.runTask('hifa_antpos', myf['_ip'])
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
        a['hm_antpos']  = 'file'
        a['pipelinemode']  = 'automatic'

        a['hm_antpos'] = {
                    0:odict([{'value':'file'}, {'antposfile':''}]), 
                    1:odict([{'value':'manual'}, {'antenna':''}, {'offsets':''}]), 
                    2:{'value':'online'}}
        a['pipelinemode'] = {
                    0:{'value':'automatic'}, 
                    1:odict([{'value':'interactive'}, {'vis':[]}, {'caltable':[]}, {'dryrun':False}, {'acceptresults':True}]), 
                    2:odict([{'value':'getinputs'}, {'vis':[]}, {'caltable':[]}])}

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
    def description(self, key='hifa_antpos', subkey=None):
        desc={'hifa_antpos': 'Derive an antenna position calibration table',
               'vis': 'List of input MeasurementSets',
               'caltable': 'List of output caltable(s)',
               'hm_antpos': 'The antenna position determination method',
               'antenna': 'List of antennas to be corrected',
               'offsets': 'List of position corrections one set per antenna',
               'antposfile': 'File containing antenna position corrections',
               'pipelinemode': 'The pipeline operation mode',
               'dryrun': 'Run the task (False) or list commands(True)',
               'acceptresults': 'Automatically accept results into context',

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
        a['hm_antpos']  = 'file'
        a['antenna']  = ''
        a['offsets']  = []
        a['antposfile']  = ''
        a['pipelinemode']  = 'automatic'
        a['dryrun']  = False
        a['acceptresults']  = True

        #a = sys._getframe(len(inspect.stack())-1).f_globals

        if self.parameters['hm_antpos']  == 'file':
            a['antposfile'] = ''

        if self.parameters['hm_antpos']  == 'manual':
            a['antenna'] = ''
            a['offsets'] = ''

        if self.parameters['pipelinemode']  == 'interactive':
            a['vis'] = []
            a['caltable'] = []
            a['dryrun'] = False
            a['acceptresults'] = True

        if self.parameters['pipelinemode']  == 'getinputs':
            a['vis'] = []
            a['caltable'] = []

        if a.has_key(paramname) :
	      return a[paramname]
hifa_antpos_cli = hifa_antpos_cli_()
