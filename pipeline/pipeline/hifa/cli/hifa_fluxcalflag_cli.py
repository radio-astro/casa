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
from task_hifa_fluxcalflag import hifa_fluxcalflag
class hifa_fluxcalflag_cli_:
    __name__ = "hifa_fluxcalflag"
    rkey = None
    i_am_a_casapy_task = None
    # The existence of the i_am_a_casapy_task attribute allows help()
    # (and other) to treat casapy tasks as a special case.

    def __init__(self) :
       self.__bases__ = (hifa_fluxcalflag_cli_,)
       self.__doc__ = self.__call__.__doc__

       self.parameters={'vis':None, 'field':None, 'intent':None, 'spw':None, 'pipelinemode':None, 'threshold':None, 'appendlines':None, 'linesfile':None, 'applyflags':None, 'dryrun':None, 'acceptresults':None, }


    def result(self, key=None):
	    #### and add any that have completed...
	    return None


    def __call__(self, vis=None, field=None, intent=None, spw=None, pipelinemode=None, threshold=None, appendlines=None, linesfile=None, applyflags=None, dryrun=None, acceptresults=None, ):

        """Locate and flag line regions in solar system flux calibrators

	Detailed Description:

Locate and flag line regions in solar system flux calibrators

	Arguments :
		vis:	List of input MeasurementSets
		   Default Value: 

		field:	List of field names or ids
		   Default Value: 

		intent:	Observing intent of flux calibrators
		   Default Value: 

		spw:	List of spectral window ids
		   Default Value: 

		pipelinemode:	The pipeline operating mode
		   Default Value: automatic
		   Allowed Values:
				automatic
				interactive
				getinputs

		threshold:	Threshold for flagging the entire spw
		   Default Value: 0.75

		appendlines:	Append user defined line regions to the line dictionary
		   Default Value: False

		linesfile:	File containing user defined lines
		   Default Value: 

		applyflags:	Apply the computed flag commands
		   Default Value: True

		dryrun:	Run the task (False) or display the commands(True)
		   Default Value: False

		acceptresults:	Automatically accept results into the context
		   Default Value: True

	Returns: void

	Example :


Fills the model column with the model visibilities.

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context defined pipeline inputs automatically.
   In interactive mode the user can set the pipeline context defined parameters
   manually.  In 'getinputs' mode the user can check the settings of all
   pipeline parameters without running the task.
   default: 'automatic'.

---- pipeline parameter arguments which can be set in any pipeline mode

threshold -- If the fraction of an spw occupied by line regions is greater
   then threshold flag the entire spectral window.

---- pipeline context defined parameter arguments which can be set only in
'interactive mode'

vis -- The list of input MeasurementSets. Defaults to the list of MeasurementSets
    defined in the pipeline context.
    default: []
    example:

field -- The list of field names or field ids for which the models are
    to be set. Defaults to all fields with intent 'AMPLITUDE'.
    default: ''
    example: '3C279', '3C279, M82'

intent -- A string containing a comma delimited list of intents against
    which the the selected fields are matched.  Defaults to all data
    with amplitude intent.
    default: ''
    example: 'AMPLITUDE'

spw -- The list of spectral windows and channels for which bandpasses are
    computed. Defaults to all science spectral windows.
    default: ''
    example: '11,13,15,17'

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

Search the bultin solar system flux calibrater line catalog for overlaps with the
science spectral windows.  Generate a list of line overlap regions and flagging
commands.

Issues


Examples

1. Locate known lines in any solar system object flux calibrators. 

    hifa_fluxcalflag()


        """
	if not hasattr(self, "__globals__") or self.__globals__ == None :
           self.__globals__=stack_frame_find( )
	#casac = self.__globals__['casac']
	casalog = self.__globals__['casalog']
	casa = self.__globals__['casa']
	#casalog = casac.casac.logsink()
        self.__globals__['__last_task'] = 'hifa_fluxcalflag'
        self.__globals__['taskname'] = 'hifa_fluxcalflag'
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
            myparams['pipelinemode'] = pipelinemode = self.parameters['pipelinemode']
            myparams['threshold'] = threshold = self.parameters['threshold']
            myparams['appendlines'] = appendlines = self.parameters['appendlines']
            myparams['linesfile'] = linesfile = self.parameters['linesfile']
            myparams['applyflags'] = applyflags = self.parameters['applyflags']
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
        mytmp['pipelinemode'] = pipelinemode
        mytmp['threshold'] = threshold
        mytmp['appendlines'] = appendlines
        mytmp['linesfile'] = linesfile
        mytmp['applyflags'] = applyflags
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli/"
	trec = casac.casac.utils().torecord(pathname+'hifa_fluxcalflag.xml')

        casalog.origin('hifa_fluxcalflag')
	try :
          #if not trec.has_key('hifa_fluxcalflag') or not casac.casac.utils().verify(mytmp, trec['hifa_fluxcalflag']) :
	    #return False

          casac.casac.utils().verify(mytmp, trec['hifa_fluxcalflag'], True)
          scriptstr=['']
          saveinputs = self.__globals__['saveinputs']
          if type(self.__call__.func_defaults) is NoneType:
              saveinputs=''
          else:
              saveinputs('hifa_fluxcalflag', 'hifa_fluxcalflag.last', myparams, self.__globals__,scriptstr=scriptstr)
          tname = 'hifa_fluxcalflag'
          spaces = ' '*(18-len(tname))
          casalog.post('\n##########################################'+
                       '\n##### Begin Task: ' + tname + spaces + ' #####')
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('Begin Task: ' + tname)
          if type(self.__call__.func_defaults) is NoneType:
              casalog.post(scriptstr[0]+'\n', 'INFO')
          else :
              casalog.post(scriptstr[1][1:]+'\n', 'INFO')
          result = hifa_fluxcalflag(vis, field, intent, spw, pipelinemode, threshold, appendlines, linesfile, applyflags, dryrun, acceptresults)
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('End Task: ' + tname)
          casalog.post('##### End Task: ' + tname + '  ' + spaces + ' #####'+
                       '\n##########################################')

	except Exception, instance:
          if(self.__globals__.has_key('__rethrow_casa_exceptions') and self.__globals__['__rethrow_casa_exceptions']) :
             raise
          else :
             #print '**** Error **** ',instance
	     tname = 'hifa_fluxcalflag'
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
#        paramgui.runTask('hifa_fluxcalflag', myf['_ip'])
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
        a['threshold']  = 0.75

        a['pipelinemode'] = {
                    0:{'value':'automatic'}, 
                    1:odict([{'value':'interactive'}, {'vis':[]}, {'field':''}, {'intent':''}, {'spw':''}, {'appendlines':False}, {'linesfile':''}, {'applyflags':True}, {'dryrun':False}, {'acceptresults':True}]), 
                    2:odict([{'value':'getinputs'}, {'vis':[]}, {'field':''}, {'intent':''}, {'spw':''}, {'appendlines':False}, {'linesfile':''}, {'applyflags':True}])}

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
    def description(self, key='hifa_fluxcalflag', subkey=None):
        desc={'hifa_fluxcalflag': 'Locate and flag line regions in solar system flux calibrators',
               'vis': 'List of input MeasurementSets',
               'field': 'List of field names or ids',
               'intent': 'Observing intent of flux calibrators',
               'spw': 'List of spectral window ids',
               'pipelinemode': 'The pipeline operating mode',
               'threshold': 'Threshold for flagging the entire spw',
               'appendlines': 'Append user defined line regions to the line dictionary',
               'linesfile': 'File containing user defined lines',
               'applyflags': 'Apply the computed flag commands',
               'dryrun': 'Run the task (False) or display the commands(True)',
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
        a['intent']  = ''
        a['spw']  = ''
        a['pipelinemode']  = 'automatic'
        a['threshold']  = 0.75
        a['appendlines']  = False
        a['linesfile']  = ''
        a['applyflags']  = True
        a['dryrun']  = False
        a['acceptresults']  = True

        #a = sys._getframe(len(inspect.stack())-1).f_globals

        if self.parameters['pipelinemode']  == 'interactive':
            a['vis'] = []
            a['field'] = ''
            a['intent'] = ''
            a['spw'] = ''
            a['appendlines'] = False
            a['linesfile'] = ''
            a['applyflags'] = True
            a['dryrun'] = False
            a['acceptresults'] = True

        if self.parameters['pipelinemode']  == 'getinputs':
            a['vis'] = []
            a['field'] = ''
            a['intent'] = ''
            a['spw'] = ''
            a['appendlines'] = False
            a['linesfile'] = ''
            a['applyflags'] = True

        if a.has_key(paramname) :
	      return a[paramname]
hifa_fluxcalflag_cli = hifa_fluxcalflag_cli_()
