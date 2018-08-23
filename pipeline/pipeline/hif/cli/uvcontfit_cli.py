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
from task_uvcontfit import uvcontfit
class uvcontfit_cli_:
    __name__ = "uvcontfit"
    rkey = None
    i_am_a_casapy_task = None
    # The existence of the i_am_a_casapy_task attribute allows help()
    # (and other) to treat casapy tasks as a special case.

    def __init__(self) :
       self.__bases__ = (uvcontfit_cli_,)
       self.__doc__ = self.__call__.__doc__

       self.parameters={'vis':None, 'caltable':None, 'field':None, 'intent':None, 'spw':None, 'combine':None, 'solint':None, 'fitorder':None, 'append':None, }


    def result(self, key=None):
	    #### and add any that have completed...
	    return None


    def __call__(self, vis=None, caltable=None, field=None, intent=None, spw=None, combine=None, solint=None, fitorder=None, append=None, ):

        """Fit the continuum in the UV plane

	Detailed Description:

Fit the continuum in the UV plane using polynomials.

	Arguments :
		vis:	The name of the input visibility file
		   Default Value: 

		caltable:	Name of output mueller matrix calibration table
		   Default Value: 

		field:	Select field(s) using id(s) or name(s)
		   Default Value: 

		intent:	Select intents
		   Default Value: 

		spw:	Spectral window / channels for fitting the continuum
		   Default Value: 

		combine:	Data axes to combine for the continuum estimation (none, spw and/or scan)
		   Default Value: 

		solint:	Time scale for the continuum fit
		   Default Value: int

		fitorder:	Polynomial order for the continuum fits
		   Default Value: 0

		append:	Append to a pre-existing table
		   Default Value: False


	Example :


This task estimates the continuum emission by fitting polynomials to
the real and imaginary parts of the spectral windows and channels
selected by spw and exclude spw. This fit represents a model of
the continuum in all channels. Fit orders less than 2 are strongly
recommended.
	

        """
	if not hasattr(self, "__globals__") or self.__globals__ == None :
           self.__globals__=stack_frame_find( )
	#casac = self.__globals__['casac']
	casalog = self.__globals__['casalog']
	casa = self.__globals__['casa']
	#casalog = casac.casac.logsink()
        self.__globals__['__last_task'] = 'uvcontfit'
        self.__globals__['taskname'] = 'uvcontfit'
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
            myparams['combine'] = combine = self.parameters['combine']
            myparams['solint'] = solint = self.parameters['solint']
            myparams['fitorder'] = fitorder = self.parameters['fitorder']
            myparams['append'] = append = self.parameters['append']


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
        mytmp['combine'] = combine
        mytmp['solint'] = solint
        mytmp['fitorder'] = fitorder
        mytmp['append'] = append
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli/"
	trec = casac.casac.utils().torecord(pathname+'uvcontfit.xml')

        casalog.origin('uvcontfit')
	try :
          #if not trec.has_key('uvcontfit') or not casac.casac.utils().verify(mytmp, trec['uvcontfit']) :
	    #return False

          casac.casac.utils().verify(mytmp, trec['uvcontfit'], True)
          scriptstr=['']
          saveinputs = self.__globals__['saveinputs']
          if type(self.__call__.func_defaults) is NoneType:
              saveinputs=''
          else:
              saveinputs('uvcontfit', 'uvcontfit.last', myparams, self.__globals__,scriptstr=scriptstr)
          tname = 'uvcontfit'
          spaces = ' '*(18-len(tname))
          casalog.post('\n##########################################'+
                       '\n##### Begin Task: ' + tname + spaces + ' #####')
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('Begin Task: ' + tname)
          if type(self.__call__.func_defaults) is NoneType:
              casalog.post(scriptstr[0]+'\n', 'INFO')
          else :
              casalog.post(scriptstr[1][1:]+'\n', 'INFO')
          result = uvcontfit(vis, caltable, field, intent, spw, combine, solint, fitorder, append)
          if (casa['state']['telemetry-enabled']):
              casalog.poststat('End Task: ' + tname)
          casalog.post('##### End Task: ' + tname + '  ' + spaces + ' #####'+
                       '\n##########################################')

	except Exception, instance:
          if(self.__globals__.has_key('__rethrow_casa_exceptions') and self.__globals__['__rethrow_casa_exceptions']) :
             raise
          else :
             #print '**** Error **** ',instance
	     tname = 'uvcontfit'
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
#        paramgui.runTask('uvcontfit', myf['_ip'])
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
        a['vis']  = ''
        a['caltable']  = ''
        a['field']  = ''
        a['intent']  = ''
        a['spw']  = ''
        a['combine']  = ''
        a['solint']  = 'int'
        a['fitorder']  = 0
        a['append']  = False


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
    def description(self, key='uvcontfit', subkey=None):
        desc={'uvcontfit': 'Fit the continuum in the UV plane',
               'vis': 'The name of the input visibility file',
               'caltable': 'Name of output mueller matrix calibration table',
               'field': 'Select field(s) using id(s) or name(s)',
               'intent': 'Select intents',
               'spw': 'Spectral window / channels for fitting the continuum',
               'combine': 'Data axes to combine for the continuum estimation (none, spw and/or scan)',
               'solint': 'Time scale for the continuum fit',
               'fitorder': 'Polynomial order for the continuum fits',
               'append': 'Append to a pre-existing table',

              }

        if(desc.has_key(key)) :
           return desc[key]

    def itsdefault(self, paramname) :
        a = {}
        a['vis']  = ''
        a['caltable']  = ''
        a['field']  = ''
        a['intent']  = ''
        a['spw']  = ''
        a['combine']  = ''
        a['solint']  = 'int'
        a['fitorder']  = 0
        a['append']  = False

        #a = sys._getframe(len(inspect.stack())-1).f_globals

        if a.has_key(paramname) :
	      return a[paramname]
uvcontfit_cli = uvcontfit_cli_()
