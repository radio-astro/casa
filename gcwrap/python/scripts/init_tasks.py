from math import *
from tasks import *
from parameter_dictionary import *
from task_help import *
import numpy as np
from casa_stack_manip import stack_frame_find

from parameter_check import *

##allow globals for taskby default
casaglobals=True

####################
def go(taskname=None):
    """ Execute taskname: """
    myf=stack_frame_find( )
    if taskname==None: taskname=myf['taskname']
    oldtaskname=taskname
    if(myf.has_key('taskname')):
        oldtaskname=myf['taskname']
    #myf['taskname']=taskname
    if type(taskname)!=str:
        taskname=taskname.__name__
        myf['taskname']=taskname
    try:
        parameter_checktype(['taskname'],[taskname],str)
    except TypeError, e:
        print "go -- TypeError: ",e
        return
    fulltaskname=taskname+'()'
    print 'Executing: ',fulltaskname
    exec(fulltaskname)
    myf['taskname']=oldtaskname


# from RR
def announce_async_task(taskname):
	"""Use the logger to herald the beginning of an asynchronous task."""
	casalog.origin(taskname)
	casalog.post('')
	casalog.post('###############################################')
	casalog.post('###  Begin Task: ' + taskname + ' ' * (27 - len(taskname)) + '###')
	casalog.post("")
	casalog.post("Use: ")
	casalog.post("      tm.retrieve(return_value) # to retrieve the status")
	casalog.post("")
	
def write_task_obit(taskname):
	"""Eulogize the task in the logger."""
	casalog.post('###  End Task: ' + taskname + ' ' * (29 - len(taskname)) + '###')
	casalog.post('###############################################')
	casalog.post('')


def inp(taskname=None, page=False):
    """
    Function to browse input parameters of a given task
    taskname: name of task of interest
    page: use paging if True, useful if list of parameters is longer than terminal height
    """
    try:
        ####paging contributed by user Ramiro Hernandez
        if(page):
            #########################
            class TemporaryRedirect(object):
                def __init__(self, stdout=None, stderr=None):
                    self._stdout = stdout or sys.stdout
                    self._stderr = stderr or sys.stderr
                def __enter__(self):
                    self.old_stdout, self.old_stderr = sys.stdout, sys.stderr
                    self.old_stdout.flush(); self.old_stderr.flush()
                    sys.stdout, sys.stderr = self._stdout, self._stderr
                def __exit__(self, exc_type, exc_value, traceback):
                    self._stdout.flush(); self._stderr.flush()
                    sys.stdout = self.old_stdout
                    sys.stderr = self.old_stderr
            #######################end class
            tempfile="__temp_input.casa"
            temporal = open(tempfile, 'w')    

            with TemporaryRedirect(stdout=temporal):
                inp(taskname, False)
            temporal.close()
            os.system('more '+tempfile)
            os.system('rm '+tempfile)
            return
        ####
        myf=stack_frame_find( )
        if((taskname==None) and (not myf.has_key('taskname'))):
            print 'No task name defined for inputs display'
            return
        if taskname==None: taskname=myf['taskname']
        myf['taskname']=taskname
        if type(taskname)!=str:
            taskname=taskname.__name__
            myf['taskname']=taskname

        try:
            parameter_checktype(['taskname'],taskname,str)
        except TypeError, e:
            print "inp -- TypeError: ", e
            return
        except ValueError, e:
            print "inp -- OptionError: ", e
            return

        ###Check if task exists by checking if task_defaults is defined
        if ( not myf.has_key(taskname) and
             str(type(myf[taskname])) != "<type 'instance'>" and
             not hasattr(myf[taskname],"defaults") ):
            raise TypeError, "task %s is not defined " %taskname
        if(myf.has_key('__last_taskname')):
            myf['__last_taskname']=taskname
        else:
            myf.update({'__last_taskname':taskname})

        print '# ',myf['taskname']+' :: '+(eval(myf['taskname']+'.description()'))
        update_params(myf['taskname'], myf)
    except TypeError, e:
        print "inp --error: ", e
    except Exception, e:
        print "---",e

def update_params(func, printtext=True, ipython_globals=None):
    from odict import odict

    if ipython_globals == None:
        myf=stack_frame_find( )
    else:
        myf=ipython_globals

    ### set task to the one being called
    myf['taskname']=func
    obj=myf[func]

    if ( str(type(obj)) == "<type 'instance'>" and
         hasattr(obj,"check_params") ):
        hascheck = True
    else:
        hascheck = False

    noerror=True
    ###check if task has defined a task_check_params function

    if (hascheck):
	has_othertasks = myf.has_key('task_location')
	if(has_othertasks) :
	   has_task = myf['task_location'].has_key(myf['taskname'])
	   if (has_task) :
		pathname=myf['task_location'][myf['taskname']]
	   else :
	        pathname = os.environ.get('CASAPATH').split()[0]+'/'+os.environ.get('CASAPATH').split()[1]+'/xml'
                if not os.path.exists(pathname) :
                    pathname = os.environ.get('CASAPATH').split()[0]+'/xml'
                    if not os.path.exists(pathname) :
                        pathname = os.environ.get('CASAPATH').split()[0]+'/Resources/xml'
                
	else :
	   pathname = os.environ.get('CASAPATH').split()[0]+'/'+os.environ.get('CASAPATH').split()[1]+'/xml'
           if not os.path.exists(pathname) :
               pathname = os.environ.get('CASAPATH').split()[0]+'/xml'
               if not os.path.exists(pathname) :
                   pathname = os.environ.get('CASAPATH').split()[0]+'/Resources/xml'
                   if not os.path.exists(pathname) :
                       sys.exit("ERROR: casapy.py update_params() can not locate xml file for task %s" % (taskname))
                       
        xmlfile=pathname+'/'+myf['taskname']+'.xml'
        if(os.path.exists(xmlfile)) :
            cu.setconstraints('file://'+xmlfile);
        else:
            #
            # SRankin: quick fix for CAS-5381 - needs review.
            # The task is not a CASA internal task.  Extract the path from task_location.
            # This may not be robust.  I have not tracked down all the code that could update task_location.
            task_path=task_location[taskname]
            xmlfile=task_path+'/'+myf['taskname']+'.xml'
            if(os.path.exists(xmlfile)) :
                cu.setconstraints('file://'+xmlfile);

    a=myf[myf['taskname']].defaults("paramkeys",myf)
    itsdef=myf[myf['taskname']].defaults
    itsparams=myf[myf['taskname']].parameters
    params=a
    #print 'itsparams:', itsparams
    for k in range(len(params)):
        paramval = obj.defaults(params[k], myf)

        notdict=True
        ###if a dictionary with key 0, 1 etc then need to peel-open
        ###parameters
        if(type(paramval)==dict):
            if(paramval.has_key(0)):
                notdict=False
        if(myf.has_key(params[k])):
            itsparams.update({params[k]:myf[params[k]]})
        else:
            itsparams.update({params[k]:obj.itsdefault(params[k])})
        if (notdict ):
            if(not myf.has_key(params[k])):
                myf.update({params[k]:paramval})
                itsparams.update({params[k]:paramval})
            if(printtext):
                #print 'params:', params[k], '; myf[params]:', myf[params[k]]
                if(hascheck):
                    noerror = obj.check_params(params[k],myf[params[k]],myf)
                # RI this doesn't work with numpy arrays anymore.  Noone seems
                # interested, so I'll be the red hen and try to fix it.
                
                #print 'params:', params[k], '; noerror:', noerror, '; myf[params]:', myf[params[k]]
                myfparamsk=myf[params[k]]
                if(type(myf[params[k]])==np.ndarray):
                    myfparamsk=myfparamsk.tolist()
                #if(myf[params[k]]==paramval):
                if(myfparamsk==paramval):
                    print_params_col(params[k],myf[params[k]],obj.description(params[k]), 'ndpdef', 'black',noerror)
                else:
                    print_params_col(params[k],myf[params[k]],obj.description(params[k]), 'ndpnondef', 'black', noerror)
		itsparams[params[k]] = myf[params[k]]
        else:
            subdict=odict(paramval)
            ##printtext is False....called most probably to set
            ##undefined params..no harm in doing it anyways
            if(not printtext):
                ##locate which dictionary is user selected
                userdict={}
                subkeyupdated={}
                for somekey in paramval:
                    somedict=dict(paramval[somekey])
                    subkeyupdated.update(dict.fromkeys(somedict, False))
                    if(somedict.has_key('value') and myf.has_key(params[k])):
                        if(somedict['value']==myf[params[k]]):
                            userdict=somedict
                    elif(somedict.has_key('notvalue') and myf.has_key(params[k])):
                        if(somedict['notvalue']!=myf[params[k]]):
                            userdict=somedict
                ###The behaviour is to use the task.defaults
                ### for all non set parameters and parameters that
                ### have no meaning for this selection
                for j in range(len(subdict)):
                    subkey=subdict[j].keys()
                   
                    for kk in range(len(subkey)):
                        
                        if( (subkey[kk] != 'value') & (subkey[kk] != 'notvalue') ):
                            #if user selecteddict
                            #does not have the key
                            ##put default
                            if(userdict.has_key(subkey[kk])):
                                if(myf.has_key(subkey[kk])):
                                    itsparams.update({subkey[kk]:myf[subkey[kk]]})
                                else:
                                    itsparams.update({subkey[kk]:userdict[subkey[kk]]})
                                subkeyupdated[subkey[kk]]=True
                            elif((not subkeyupdated[subkey[kk]])):
                                itsparams.update({subkey[kk]:itsdef(params[k], None, itsparams[params[k]], subkey[kk])})
                                subkeyupdated[subkey[kk]]=True
            ### need to do default when user has not set val
            if(not myf.has_key(params[k])):
                if(paramval[0].has_key('notvalue')):
                    itsparams.update({params[k]:paramval[0]['notvalue']})
                    myf.update({params[k]:paramval[0]['notvalue']})
                else:
                    itsparams.update({params[k]:paramval[0]['value']})
                    myf.update({params[k]:paramval[0]['value']})
            userval=myf[params[k]]
            choice=0
            notchoice=-1
            valuekey='value'
            for j in range(len(subdict)):
                if(subdict[j].has_key('notvalue')):
                    valuekey='notvalue'
                    if(subdict[j]['notvalue'] != userval):
                        notchoice=j;
                        break
                else:
                    if(subdict[j]['value']==userval):
                        choice=j
                        notchoice=j
                        break
            subkey=subdict[choice].keys()
            if(hascheck):
                noerror=obj.check_params(params[k],userval,myf)
            if(printtext):
                if(myf[params[k]]==paramval[0][valuekey]):
                    print_params_col(params[k],myf[params[k]],obj.description(params[k]),'dpdef','black', noerror)
                else:
                    print_params_col(params[k],myf[params[k]],obj.description(params[k]),'dpnondef','black', noerror)
		itsparams[params[k]] = myf[params[k]]
            for j in range(len(subkey)):
                if((subkey[j] != valuekey) & (notchoice > -1)):
                    ###put default if not there
                    if(not myf.has_key(subkey[j])):
                        myf.update({subkey[j]:subdict[choice][subkey[j]]})
                    paramval=subdict[choice][subkey[j]]
                    if (j==(len(subkey)-1)):
                        # last subparameter - need to add an extra line to allow cut/pasting
                        comment='last'
                    else:
                        comment='blue'
                    if(hascheck):
                        noerror = obj.check_params(subkey[j],myf[subkey[j]],myf)
                    if(printtext):
                        if(myf[subkey[j]]==paramval):
                            print_params_col(subkey[j],myf[subkey[j]],obj.description(subkey[j],userval),'spdef',comment, noerror)
                        else:
                            print_params_col(subkey[j],myf[subkey[j]],obj.description(subkey[j],userval),'spnondef',comment, noerror)
		        itsparams[params[k]] = myf[params[k]]                    
    #
    # Verify the complete record, with errors being reported to the user
    #
    #cu.verify(itsparams, cu.torecord('file://'+xmlfile)[myf['taskname']]);

####function to print inputs with coloring
####colorparam 'blue'=> non-default, colorcomment 'green'=> can have sub params
#### 'blue' => is a sub-parameter 
# blue = \x1B[94m
# bold = \x1B[1m
# red  = \x1B[91m
# cyan = \x1B[96m
# green= \x1B[92m
# normal   = \x1B[0m
# underline= \x1B[04m
# reverse = \x1B[7m
# highlight with black = \x1B[40s

def print_params_col(param=None, value=None, comment='', colorparam=None,
                     colorcomment=None, noerrorval=True):
    try:
        from TerminalController import TerminalController
        term = TerminalController()
        cols = term.COLS
        del term
    except:
        cols = 80
    #
    #print 'colorparam is: ', colorparam
    #
    if type(value) == str:
        printval = "'" + value + "'"
    else:
        printval = value

    if colorparam == 'ndpnondef':
        firstcol = '\x1B[0m'
        valcol   = '\x1B[94m'
    elif colorparam == 'dpdef':
        firstcol = '\x1B[1m' + '\x1B[47m'
        valcol   = '\x1B[1m' + '\x1B[0m'
    elif colorparam == 'dpnondef':
        firstcol = '\x1B[1m' + '\x1B[47m'
        valcol   = '\x1B[1m' + '\x1B[94m'
    elif colorparam == 'spdef':
        firstcol = '\x1B[32m'
        valcol   = '\x1B[0m'
    elif colorparam == 'spnondef':
        firstcol = '\x1B[32m'
        valcol   = '\x1B[94m'
    else:
        firstcol = '\x1B[0m'
        valcol   = '\x1B[0m'

    if not noerrorval:
        valcol = '\x1B[1m' + '\x1B[91m'

    if colorcomment == 'green':
        secondcol = '\x1B[102m'
    elif colorcomment == 'blue':
        #secondcol='\x1B[104m'
        secondcol = '\x1B[0m'
    else:
        secondcol = '\x1B[0m'

    # RR: I think colorcomment should really be called submenu.
    #     Since these are left justified, I've absorbed the right space into
    #     the %s's, in order to handle as long a parameter name as possible.
    #     (The uvfilterb* params were busting out of %-10s.)
    if colorcomment in ('last', 'blue'):
        parampart = firstcol + '     %-14s ='
    else:
        parampart = firstcol + '%-19s ='
    parampart %= param

    valpart = valcol + ' %10s \x1B[0m' % printval + secondcol
    # Well behaved (short) parameters and values tally up to 33 characters
    # so far.  Pad them up to 40, assuming the param is short enough.
    pad = 7
    paramlen = len(str(param))
    if colorcomment in ('last', 'blue') and paramlen > 14:
        pad -= paramlen - 14
    elif paramlen > 19:
        pad -= paramlen - 19
    valuelen = len(str(printval))
    if valuelen > 10:
        pad -= valuelen - 10
    if pad > 0:
        valpart += ' ' * pad

    try:
        from textwrap import fill
        if pad < 0:
            firstskip = 40 - pad
            firstfiller = ' ' * firstskip + '#  '
            afterfiller = ' ' * 40 + '#   '
        else:
            firstskip = 40
            firstfiller = ' ' * 40 + '#  '
            afterfiller = firstfiller + ' '
        commentpart = fill(comment, cols, initial_indent=firstfiller,
                           subsequent_indent=afterfiller)[firstskip:]
    except:
        if comment:
            commentpart = '#  ' + comment
        else:
            commentpart = ''
    commentpart += '\x1B[0m'          # RR: I think this might be redundant.
    if colorcomment == 'last':        #     (Is colorcomment ever green?)
        commentpart += "\n"

    print parampart + valpart + commentpart

def __set_default_parameters(b):
    myf=stack_frame_find( )
    a=b
    elkey=a.keys()
    for k in range(len(a)):
        if (type(a[elkey[k]]) != dict):
            myf[elkey[k]]=a[elkey[k]]
        elif (type(a[elkey[k]]) == dict and len(a[elkey[k]])==0):
            myf[elkey[k]]=a[elkey[k]]
        else:
            subdict=a[elkey[k]]
            ##clear out variables of other options if they exist
            for j in range(1,len(subdict)):
                subkey=subdict[j].keys()
                for kk in range(len(subkey)):
                    if((subkey[kk] != 'value') & (subkey[kk] != 'notvalue') ):
                        if(myf.has_key(subkey[kk])):
                            del myf[subkey[kk]]
            ###
            if(subdict[0].has_key('notvalue')):
                myf[elkey[k]]=subdict[0]['notvalue']
            else:
                myf[elkey[k]]=subdict[0]['value']
            subkey=subdict[0].keys()
            for j in range(0, len(subkey)):
                if((subkey[j] != 'value') & (subkey[j] != 'notvalue')):
                    myf[subkey[j]]=subdict[0][subkey[j]]

def backupoldfile(thefile=''):
    import copy
    import shutil
    import filecmp
    if(thefile=='' or (not os.path.exists(thefile))):
        return 
    outpathdir = os.path.realpath(os.path.dirname(thefile))
    outpathfile = outpathdir + os.path.sep + os.path.basename(thefile)
    k=0
    backupfile=outpathfile+'.'+str(k)
    prevfile='--------'
    while (os.path.exists(backupfile)):
        k=k+1
        prevfile=copy.copy(backupfile)
        if(os.path.exists(prevfile)  and filecmp.cmp(outpathfile, prevfile)):
        ##avoid making multiple copies of the same file
            return
        backupfile=outpathfile+'.'+str(k)
    shutil.copy2(outpathfile, backupfile)

def tput(taskname=None, outfile=''):
	myf=stack_frame_find( )
	if taskname == None: taskname = myf['taskname']
	if type(taskname) != str:
		taskname=taskname.__name__
	myf['taskname'] = taskname
	outfile = myf['taskname']+'.last'
	saveinputs(taskname, outfile)

def saveinputs(taskname=None, outfile='', myparams=None, ipython_globals=None, scriptstr=['']):
    #parameter_printvalues(arg_names,arg_values,arg_types)
    """ Save current input values to file on disk for a specified task:

    taskname -- Name of task
        default: <unset>; example: taskname='bandpass'
        <Options: type tasklist() for the complete list>
    outfile -- Output file for the task inputs
        default: taskname.saved; example: outfile=taskname.orion

    """

    try:
        if ipython_globals == None:
            myf=stack_frame_find( )
        else:
            myf=ipython_globals

        if taskname==None: taskname=myf['taskname']
        myf['taskname']=taskname
        if type(taskname)!=str:
            taskname=taskname.__name__
            myf['taskname']=taskname

        parameter_checktype(['taskname','outfile'],[taskname,outfile],[str,str])

        ###Check if task exists by checking if task_defaults is defined
        obj = False
        if ( not myf.has_key(taskname) and
             str(type(myf[taskname])) != "<type 'instance'>" and
             not hasattr(myf[taskname],"defaults") ):
            raise TypeError, "task %s is not defined " %taskname
        else:
            obj = myf[taskname]

        if taskname==None: taskname=myf['taskname']
        myf['taskname']=taskname
        if outfile=='': outfile=taskname+'.saved'
        if(myf.has_key('__multibackup') and myf['__multibackup']):
            backupoldfile(outfile)
        
        ##make sure unfolded parameters get their default values
        myf['update_params'](func=myf['taskname'], printtext=False, ipython_globals=myf)
        ###
        do_save_inputs = False
        outpathdir = os.path.realpath(os.path.dirname(outfile))
        outpathfile = outpathdir + os.path.sep + os.path.basename(outfile)
        if outpathfile not in casa['state']['unwritable'] and outpathdir not in casa['state']['unwritable']:
            try:
                taskparameterfile=open(outfile,'w')
                print >>taskparameterfile, '%-15s    = "%s"'%('taskname', taskname)
                do_save_inputs = True
            except:
                print "********************************************************************************"
                print "Warning: no write permission for %s, cannot save task" % outfile
                if os.path.isfile(outfile):
                    print "         inputs in %s..." % outpathfile
                    casa['state']['unwritable'].add(outpathfile)
                elif not os.path.isdir(outfile):
                    print "         inputs in dir %s..." % outpathdir
                    casa['state']['unwritable'].add(outpathdir)
                else:
                    print "         inputs because given file (%s) is a dir..." % outpathfile
                print "********************************************************************************"
        f=zip(myf[taskname].__call__.func_code.co_varnames[1:],myf[taskname].__call__.func_defaults)
        scriptstring='#'+str(taskname)+'('
	if myparams == None :
		myparams = {}
        l=0
        for j in range(len(f)):
            k=f[j][0]
	    if not myparams.has_key(k) and k != 'self' :
		    myparams[k] = myf[taskname].parameters[k]
            if(k != 'self' and type(myparams[k])==str):
                if ( myparams[k].count( '"' ) < 1 ):
                    # if the string doesn't contain double quotes then
                    # use double quotes around it in the parameter file.
                    if do_save_inputs:
                        print >>taskparameterfile, '%-15s    =  "%s"'%(k, myparams[k])
                    scriptstring=scriptstring+k+'="'+myparams[k]+'",'
                else:
                    # use single quotes.
                    if do_save_inputs:
                        print >>taskparameterfile, "%-15s    =  '%s'"%(k, myparams[k])
                    scriptstring=scriptstring+k+"='"+myparams[k]+"',"
            else :
                if ( j != 0 or k != "self" or
                     str(type(myf[taskname])) != "<type 'instance'>" ) :
                    if do_save_inputs:
                        print >>taskparameterfile, '%-15s    =  %s'%(k, myparams[k])
                    scriptstring=scriptstring+k+'='+str(myparams[k])+','

            ###Now delete varianle from global user space because
            ###the following applies: "It would be nice if one
            ### could tell the system to NOT recall
            ### previous non-default settings sometimes."
            if(not myf['casaglobals'] and myf.has_key(k)):
                del myf[k]
            l=l+1
            if l%5==0:
                scriptstring=scriptstring+'\n        '
        scriptstring=scriptstring.rstrip()
        scriptstring=scriptstring.rstrip('\n')
        scriptstring=scriptstring.rstrip(',')
        scriptstring=scriptstring+')'        
        scriptstr.append(scriptstring)
        scriptstring=scriptstring.replace('        ', '')
        scriptstring=scriptstring.replace('\n', '')
        if do_save_inputs:
            print >>taskparameterfile,scriptstring
            taskparameterfile.close()
    except TypeError, e:
        print "saveinputs --error: ", e
def default(taskname=None):
    """ reset given task to its default values :

    taskname -- Name of task


    """

    try:
        myf=stack_frame_find( )
        if taskname==None: taskname=myf['taskname']
        myf['taskname']=taskname
        if type(taskname)!=str:
            taskname=taskname.__name__
            myf['taskname']=taskname

        ###Check if task exists by checking if task_defaults is defined
        if ( not myf.has_key(taskname) and
             str(type(myf[taskname])) != "<type 'instance'>" and
             not hasattr(myf[taskname],"defaults") ):
            raise TypeError, "task %s is not defined " %taskname
        eval(myf['taskname']+'.defaults()')

        casalog.origin('default')
        taskstring=str(taskname).split()[0]
        casalog.post(' #######  Setting values to default for task: '+taskstring+'  #######')


    except TypeError, e:
        print "default --error: ", e

def taskparamgui(useGlobals=True):
    """
        Show a parameter-setting GUI for all available tasks.
    """
    import paramgui

    if useGlobals:
        paramgui.setGlobals(stack_frame_find( ))
    else:
        paramgui.setGlobals({})

    paramgui.runAll(_ip)
    paramgui.setGlobals({})

####################
