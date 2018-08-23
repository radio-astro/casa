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
from task_hifv_vlasetjy import hifv_vlasetjy
class hifv_vlasetjy_cli_:
    __name__ = "hifv_vlasetjy"
    rkey = None
    i_am_a_casapy_task = None
    # The existence of the i_am_a_casapy_task attribute allows help()
    # (and other) to treat casapy tasks as a special case.

    def __init__(self) :
       self.__bases__ = (hifv_vlasetjy_cli_,)
       self.__doc__ = self.__call__.__doc__

       self.parameters={'vis':None, 'field':None, 'intent':None, 'spw':None, 'model':None, 'reffile':None, 'fluxdensity':None, 'spix':None, 'reffreq':None, 'scalebychan':None, 'standard':None, 'pipelinemode':None, 'dryrun':None, 'acceptresults':None, }


    def result(self, key=None):
	    #### and add any that have completed...
	    return None


    def __call__(self, vis=None, field=None, intent=None, spw=None, model=None, reffile=None, fluxdensity=None, spix=None, reffreq=None, scalebychan=None, standard=None, pipelinemode=None, dryrun=None, acceptresults=None, ):

        """Does an initial setjy run on the vis

	Detailed Description:


	Arguments :
		vis:	List of input visibility data
		   Default Value: 

		field:	List of field names or ids
		   Default Value: 

		intent:	Observing intent of flux calibrators
		   Default Value: 

		spw:	List of spectral window ids
		   Default Value: 

		model:	File location for field model
		   Default Value: 

		reffile:	Path to file with fluxes for non-solar system calibrators
		   Default Value: 

		fluxdensity:	Specified flux density [I,Q,U,V]; -1 will lookup values
		   Default Value: -1

		spix:	Spectral index of fluxdensity
		   Default Value: 0.0

		reffreq:	Reference frequency for spix
		   Default Value: 1GHz

		scalebychan:	Scale the flux density on a per channel basis or else on a per spw basis
		   Default Value: True

		standard:	Flux density standard
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

The hifv_vlasetjy task does an initial run of setjy on the vis

Keyword arguments:

---- pipeline parameter arguments which can be set in any pipeline mode

vis -- List of visibility data files. These may be ASDMs, tar files of ASDMs,
   MSs, or tar files of MSs, If ASDM files are specified, they will be
   converted  to MS format.
   default: []
   example: vis=['X227.ms', 'asdms.tar.gz']




pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context defined pipeline inputs
   automatically.  In 'interactive' mode the user can set the pipeline
   context defined parameters manually.  In 'getinputs' mode the user
   can check the settings of all pipeline parameters without running
   the task.
   default: 'automatic'.

---- pipeline context defined parameter argument which can be set only in
'interactive mode'


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

1. Initial run of setjy:

   hifv_vlasetjy()



        """
	if not hasattr(self, "__globals__") or self.__globals__ == None :
           self.__globals__=stack_frame_find( )
	#casac = self.__globals__['casac']
	casalog = self.__globals__['casalog']
	casa = self.__globals__['casa']
	#casalog = casac.casac.logsink()
        self.__globals__['__last_task'] = 'hifv_vlasetjy'
        self.__globals__['taskname'] = 'hifv_vlasetjy'
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
            myparams['model'] = model = self.parameters['model']
            myparams['reffile'] = reffile = self.parameters['reffile']
            myparams['fluxdensity'] = fluxdensity = self.parameters['fluxdensity']
            myparams['spix'] = spix = self.parameters['spix']
            myparams['reffreq'] = reffreq = self.parameters['reffreq']
            myparams['scalebychan'] = scalebychan = self.parameters['scalebychan']
            myparams['standard'] = standard = self.parameters['standard']
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
        mytmp['field'] = field
        mytmp['intent'] = intent
        mytmp['spw'] = spw
        mytmp['model'] = model
        mytmp['reffile'] = reffile
        mytmp['fluxdensity'] = fluxdensity
        mytmp['spix'] = spix
        mytmp['reffreq'] = reffreq
        mytmp['scalebychan'] = scalebychan
        mytmp['standard'] = standard
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli/"
	trec = casac.casac.utils().torecord(pathname+'hifv_vlasetjy.xml')

        casalog.origin('hifv_vlasetjy')
	try :
          #if not trec.has_key('hifv_vlasetjy') or not casac.casac.utils().verify(mytmp, trec['hifv_vlasetjy']) :
	    #return False

          casac.casac.utils().verify(mytmp, trec['hifv_vlasetjy'], True)
          scriptstr=['']
          saveinputs = self.__globals__['saveinputs']
          if type(self.__call__.func_defaults) is NoneType:
              saveinputs=''
          else:
              saveinputs('hifv_vlasetjy', 'hifv_vlasetjy.last', myparams, self.__globals__,scriptstr=scriptstr)
          tname = 'hifv_vlasetjy'
          spaces = ' '*(18-len(tname))
          casalog.post('\n##########################################'+
                       '\n##### Begin Task: ' + tname + spaces + ' #####')
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('Begin Task: ' + tname)
          if type(self.__call__.func_defaults) is NoneType:
              casalog.post(scriptstr[0]+'\n', 'INFO')
          else :
              casalog.post(scriptstr[1][1:]+'\n', 'INFO')
          result = hifv_vlasetjy(vis, field, intent, spw, model, reffile, fluxdensity, spix, reffreq, scalebychan, standard, pipelinemode, dryrun, acceptresults)
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('End Task: ' + tname)
          casalog.post('##### End Task: ' + tname + '  ' + spaces + ' #####'+
                       '\n##########################################')

	except Exception, instance:
          if(self.__globals__.has_key('__rethrow_casa_exceptions') and self.__globals__['__rethrow_casa_exceptions']) :
             raise
          else :
             #print '**** Error **** ',instance
	     tname = 'hifv_vlasetjy'
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
#        paramgui.runTask('hifv_vlasetjy', myf['_ip'])
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
        a['fluxdensity']  = -1
        a['scalebychan']  = True
        a['standard']  = ''
        a['pipelinemode']  = 'automatic'

        a['pipelinemode'] = {
                    0:{'value':'automatic'}, 
                    1:odict([{'value':'interactive'}, {'vis':[]}, {'field':''}, {'intent':''}, {'spw':''}, {'reffile':''}, {'model':''}, {'dryrun':False}, {'acceptresults':True}]), 
                    2:odict([{'value':'getinputs'}, {'vis':[]}, {'field':''}, {'intent':''}, {'spw':''}, {'reffile':''}, {'model':''}])}
        a['fluxdensity'] = {
                    0:odict([{'notvalue':-1}, {'spix':0}, {'reffreq':'1GHz'}])}

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
    def description(self, key='hifv_vlasetjy', subkey=None):
        desc={'hifv_vlasetjy': 'Does an initial setjy run on the vis',
               'vis': 'List of input visibility data',
               'field': 'List of field names or ids',
               'intent': 'Observing intent of flux calibrators',
               'spw': 'List of spectral window ids',
               'model': 'File location for field model',
               'reffile': 'Path to file with fluxes for non-solar system calibrators',
               'fluxdensity': 'Specified flux density [I,Q,U,V]; -1 will lookup values',
               'spix': 'Spectral index of fluxdensity',
               'reffreq': 'Reference frequency for spix',
               'scalebychan': 'Scale the flux density on a per channel basis or else on a per spw basis',
               'standard': 'Flux density standard',
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
        a['field']  = ''
        a['intent']  = ''
        a['spw']  = ''
        a['model']  = ''
        a['reffile']  = ''
        a['fluxdensity']  = -1
        a['spix']  = 0.0
        a['reffreq']  = '1GHz'
        a['scalebychan']  = True
        a['standard']  = ''
        a['pipelinemode']  = 'automatic'
        a['dryrun']  = False
        a['acceptresults']  = True

        #a = sys._getframe(len(inspect.stack())-1).f_globals

        if self.parameters['pipelinemode']  == 'interactive':
            a['vis'] = []
            a['field'] = ''
            a['intent'] = ''
            a['spw'] = ''
            a['reffile'] = ''
            a['model'] = ''
            a['dryrun'] = False
            a['acceptresults'] = True

        if self.parameters['pipelinemode']  == 'getinputs':
            a['vis'] = []
            a['field'] = ''
            a['intent'] = ''
            a['spw'] = ''
            a['reffile'] = ''
            a['model'] = ''

        if self.parameters['fluxdensity']  != -1:
            a['spix'] = 0
            a['reffreq'] = '1GHz'

        if a.has_key(paramname) :
	      return a[paramname]
hifv_vlasetjy_cli = hifv_vlasetjy_cli_()
