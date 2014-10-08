import sys
import os
import string
import inspect
import pdb
from odict import odict

from parameter_check import *
from tasksinfo import *

def tget(task=None, savefile=''):
	""" Get last input values from file on disk for a specified task: 

	task -- Name of task
		default: <unset>; example: task='bandpass'
		<Options: type tasklist() for the complete list>
	savefile -- Output file for the task inputs
		default: task.last then task.saved
		example: savefile=task.orion

	"""
        try:
		a=inspect.stack()
		stacklevel=0
		for k in range(len(a)):
			if (string.find(a[k][1], 'ipython console') > 0):
				stacklevel=k
				break
		myf=sys._getframe(stacklevel).f_globals
		if task==None :
			if myf.has_key('task') :
				task=myf['task']
			else : 
				if myf.has_key('taskname') : task=myf['taskname']
		myf['taskname']=task
		myf['task']=task
                if type(task)!=str:
                        task=task.__name__
		       	myf['task']=task
                        myf['taskname']=task
                parameter_checktype(['task','savefile'],[task,savefile],[str,str])
                parameter_checkmenu('task',task,tasksum.keys())

                ###Check if task exists by checking if task_defaults is defined
		if ( not myf.has_key(task) and 
		     str(type(myf[task])) != "<type 'instance'>" and 
		     not hasattr(myf[task],"defaults") ):
		          raise TypeError, "task %s is not defined " %task

                if task==None: task=myf['task']
                myf['task']=task
                #f=zip(myf[taskname].func_code.co_varnames,myf[taskname].func_defaults)
                #for j in range(len(f)):
                #        k=f[j][0]
		#	print 'k is ',k
		#	stst = myf[k]
		#	if ( type(stst) == str ):
		#		sex = k+"='"+stst+"'"
		#	else:
		#		sex = k+"="+str(stst)
		#	exec(sex)
		if savefile=='':
			savefile=task+'.last'
			try:
				taskparameterfile=open(savefile,'r')
			except:
				savefile=task+'.saved'
				try:
					taskparameterfile=open(savefile,'r')
				except:
					print "Sorry - no task.last or .saved"
					return
		
			taskparameterfile.close()
		execfile(savefile)
		# Put the task parameters back into the global namespace
                f=zip(myf[task].__call__.func_code.co_varnames[1:],myf[task].__call__.func_defaults)
                missing_ks = []
                for j in f:
                        k = j[0]
			if k != 'self':
                                try:
                                        myf[k] = eval(k)
                                except NameError:
                                        missing_ks.append(k)
                if missing_ks:
                        print "Did not find a saved value for",
                        if len(missing_ks) > 1:
                                print ', '.join(missing_ks[:-1]),
                                print 'or', missing_ks[-1]
                        else:
                                print missing_ks[0]
                        print "The set of task parameters has probably changed"
                        print "since", savefile, "was written."
		print "Restored parameters from file "+savefile
        except TypeError, e:
                print "tget --error: ", e

def tget_defaults(param=None):
       a=inspect.stack()
       stacklevel=0
       for k in range(len(a)):
         if (string.find(a[k][1], 'ipython console') > 0):
               stacklevel=k
       myf=sys._getframe(stacklevel).f_globals
       a=odict()
       a['task']=''
       a['savefile']=''
       if(param == None):
               myf['__set_default_parameters'](a)
       elif(param == 'paramkeys'):
               return a.keys()
       else:
               if(a.has_key(param)):
                       return a[param]

def tget_description(key='listobs',subkey=None):
   desc={'listobs': 'List data set summary in the logger:\n',
       'task': 'Name of task whose parameters will be set',
       'savefile': 'Name of parameter file (e.g., task.last)',
       }
   if(desc.has_key(key)):
           return desc[key]
   return ''

def tget_check_params(param=None, value=None):
       if(param=='task'):
               if (type(value)!=str):
                       return False
               else:
                      return True 
       elif(param=='savefile'):
              if ((type(value)==str)):
		      return True
              else:
                     return False
       else:
               return True

