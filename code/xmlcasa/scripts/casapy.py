#

try:
    import casac
except ImportError, e:
    print "failed to load casa:\n", e
    exit(1)

import os
import sys
import matplotlib

homedir = os.getenv('HOME')
if homedir == None :
   print "Environment variable HOME is not set, please set it"
   exit(1)

ipythonenv = homedir + '/.casa/ipython'
ipythonpath = homedir + '/.casa/ipython/security'
try :
   os.makedirs(ipythonpath, 0755)
except :
   pass
os.environ['IPYTHONDIR']=ipythonpath

#import string

#
# Check if the display environment is set if not
# switch the backend to Agg only if it's TkAgg
#
if not os.environ.has_key('DISPLAY') and matplotlib.get_backend() == "TkAgg" :
   matplotlib.use('Agg')

#
# We put in all the task declarations here...
#
from taskinit import *

casapy_flags=sys.argv
logpid=[]

if (os.uname()[0]=='Darwin'):
    casa_path = os.environ['CASAPATH'].split()
    logger_path = casa_path[0]+'/'+casa_path[1]+'/apps/casalogger.app/Contents/MacOS/casalogger'
#           from Carbon.CoreFoundation import kCFURLPOSIXPathStyle
#           kLSUnknownCreator = '\x00\x00\x00\x00'
#           fsRef, cfURL = LSFindApplicationForInfo(kLSUnknownCreator, None, "casalogger.app")
#           logger_path = os.path.join(fsRef.as_pathname(), 'Contents', 'MacOS', 'casalogger')

def casalogger(logfile='casapy.log'):
    pid=9999
    if (os.uname()[0]=='Darwin'):
        pid=os.spawnvp(os.P_NOWAIT,logger_path,[logger_path, logfile])
    elif (os.uname()[0]=='Linux'):
        pid=os.spawnlp(os.P_NOWAIT,'casalogger','casalogger',logfile)
    else:
        print 'Unrecognized OS: No logger available'

    if (pid!=9999): logpid.append(pid)

showconsole = False

thelogfile = 'casapy.log'

try:
    showconsole = casapy_flags.index('--log2term')
    showconsole = True
except ValueError,e:
    thelogfile = 'casapy.log'

try:
    mylogfile = casapy_flags.index('--nologfile')
    thelogfile = 'null'
except ValueError,e :
    pass

try:
    mylogfile = casapy_flags.index('--logfile')
    thelogfile = casapy_flags[mylogfile+1]
except ValueError,e:
    if thelogfile != 'null' :
       thelogfile = 'casapy.log'
except IndexError,e:
    if thelogfile != 'null' :
       thelogfile = 'casapy.log'

deploylogger1 = False
deploylogger2 = False
try:
    nolog=casapy_flags.index('--nolog')
    deploylogger1 = False
    print "--nolog is deprecated, please use --nologger"
except ValueError,e:
    deploylogger1 = True

try:
    nolog=casapy_flags.index('--nologger')
    deploylogger2 = False
except ValueError,e:
    deploylogger2 = True

if deploylogger1 & deploylogger2 & (thelogfile != 'null') :
	casalogger( thelogfile)

###################
#setup file catalog
###################

vwrpid=9999
####################
# Task Interface


from parameter_check import *
####################
def go(taskname=None):
    """ Execute taskname: """
    a=inspect.stack()
    stacklevel=0
    for k in range(len(a)):
        if (string.find(a[k][1], 'ipython console') > 0):
            stacklevel=k
            break
    myf=sys._getframe(stacklevel).f_globals
    if taskname==None: taskname=myf['taskname']
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

def selectfield(vis,minstring):
    """Derive the fieldid from  minimum matched string(s): """

    tb.open(vis+'/FIELD')
    fields=list(tb.getcol('NAME'))#get fieldname list
    tb.close()          #close table
    indexlist=list()    #initialize list
    stringlist=list()

    fldlist=minstring.split()#split string into elements
    print 'fldlist is ',fldlist
    for fld in fldlist:     #loop over fields
        _iter=fields.__iter__() #create iterator for fieldnames
        while 1:
            try:
                x=_iter.next() # has first value of field name
            except StopIteration:
                break
            #
            if (x.find(fld)!=-1): 
                indexlist.append(fields.index(x))
                stringlist.append(x)

    print 'Selected fields are: ',stringlist
    return indexlist

def asap_init():
    """ Initialize ASAP....: """
    a=inspect.stack()
    stacklevel=0
    for k in range(len(a)):
        if (string.find(a[k][1], 'ipython console') > 0):
            stacklevel=k
            break
    myf=sys._getframe(stacklevel).f_globals
    casapath=os.environ['CASAPATH']
    print '*** Loading ATNF ASAP Package...'
    import asap as sd
    print '*** ... ASAP (%s rev#%s) import complete ***' % (sd.__version__,sd.__revision__)
    os.environ['CASAPATH']=casapath
    from sdaverage_cli import sdaverage_cli as sdaverage
    from sdsmooth_cli import sdsmooth_cli as sdsmooth
    from sdbaseline_cli import sdbaseline_cli as sdbaseline
    from sdcal_cli import sdcal_cli as sdcal
    from sdcoadd_cli import sdcoadd_cli as sdcoadd
    from sdsave_cli import sdsave_cli as sdsave
    from sdscale_cli import sdscale_cli as sdscale
    from sdfit_cli import sdfit_cli as sdfit
    from sdplot_cli import sdplot_cli as sdplot
    from sdstat_cli import sdstat_cli as sdstat
    from sdlist_cli import sdlist_cli as sdlist
    from sdflag_cli import sdflag_cli as sdflag
    from sdtpimaging_cli import sdtpimaging_cli as sdtpimaging
    from sdmath_cli import sdmath_cli as sdmath
    myf['sd']=sd
    myf['sdaverage']=sdaverage
    myf['sdsmooth']=sdsmooth
    myf['sdbaseline']=sdbaseline
    myf['sdcal']=sdcal
    myf['sdcoadd']=sdcoadd
    myf['sdsave']=sdsave
    myf['sdscale']=sdscale
    myf['sdfit']=sdfit
    myf['sdplot']=sdplot
    myf['sdstat']=sdstat
    myf['sdlist']=sdlist
    myf['sdflag']=sdflag
    myf['sdtpimaging']=sdtpimaging
    myf['sdmath']=sdmath


def selectantenna(vis,minstring):
    """Derive the antennaid from matched string(s): """

    tb.open(vis+'/ANTENNA')
    ants=list(tb.getcol('NAME'))#get fieldname list
    tb.close()          #close table
    indexlist=list()    #initialize list
    stringlist=list()

    antlist=minstring.split()#split string into elements
    for ant in antlist:     #loop over fields
        try:
            ind=ants.index(ant)
            indexlist.append(ind)
            stringlist.append(ant)
        except ValueError:
            pass

    print 'Selected reference antenna: ',stringlist
    print 'indexlist: ',indexlist
    return indexlist[0]

def readboxfile(boxfile):
    """ Read a file containing clean boxes (compliant with AIPS BOXFILE)

    Format is:
    #FIELDID BLC-X BLC-Y TRC-X TRC-Y
    0       110   110   150   150 
    or
    0       hh:mm:ss.s dd.mm.ss.s hh:mm:ss.s dd.mm.ss.s

    Note all lines beginning with '#' are ignored.

    """
    union=[]
    f=open(boxfile)
    while 1:
        try: 
            line=f.readline()
            if (line.find('#')!=0): 
                splitline=line.split('\n')
                splitline2=splitline[0].split()
                if (len(splitline2[1])<6): 
                    boxlist=[int(splitline2[1]),int(splitline2[2]),
                    int(splitline2[3]),int(splitline2[4])]
                else:
                    boxlist=[splitline2[1],splitline2[2],splitline2[3],
                    splitline2[4]]
    
                union.append(boxlist)
    
        except:
            break

    f.close()
    print 'union is: ',union
    return union

def inp(taskname=None):
    try:
        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
            if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel=k
        myf=sys._getframe(stacklevel).f_globals
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
        update_params(myf['taskname'])
    except TypeError, e:
        print "inp --error: ", e
    except Exception, e:
        print "---",e

def update_params(func, printtext=True):
    from odict import odict
    a=inspect.stack()
    stacklevel=0
    for k in range(len(a)):
        if (string.find(a[k][1], 'ipython console') > 0):
            stacklevel=k
    myf=sys._getframe(stacklevel).f_globals
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
	        pathname = os.environ.get('CASAPATH').split()[0]+'/share/xml'
	else :
	   pathname = os.environ.get('CASAPATH').split()[0]+'/share/xml'
        xmlfile=pathname+'/'+myf['taskname']+'.xml'
        if(os.path.exists(xmlfile)) :
            cu.setconstraints('file://'+xmlfile);

    a=myf[myf['taskname']].defaults("paramkeys")

    params=a
    itsparams = {}
    for k in range(len(params)):
        paramval = obj.defaults(params[k])

        notdict=True
        ###if a dictionary with key 0, 1 etc then need to peel-open
        ###parameters
        if(type(paramval)==dict):
            if(paramval.has_key(0)):
                notdict=False
        
        if (notdict ):
            if(not myf.has_key(params[k])):
                myf.update({params[k]:paramval})
            if(printtext):
                if(hascheck):
                    noerror = obj.check_params(params[k],myf[params[k]])
                if(myf[params[k]]==paramval):
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
                ###The behaviour is to set to the first default
                ### all non set parameters and parameters that
                ### have no meaning for this selection
                for j in range(len(subdict)):
                    subkey=subdict[j].keys()
                    for kk in range(len(subkey)):
                        
                        if( (subkey[kk] != 'value') & (subkey[kk] != 'notvalue') ):
                            #if user selecteddict
                            #does not have the key
                            ##put default
                            if((not userdict.has_key(subkey[kk])) and (not subkeyupdated[subkey[kk]])):
                                myf.update({subkey[kk]:subdict[j][subkey[kk]]})
                                subkeyupdated[subkey[kk]]=True
                                
                    ###put default if not there
                            if(not myf.has_key(subkey[kk])):
                                myf.update({subkey[kk]:subdict[j][subkey[kk]]})
                        
            ### need to do default when user has not set val
            if(not myf.has_key(params[k])):
                if(paramval[0].has_key('notvalue')):
                    myf.update({params[k]:paramval[0]['notvalue']})
                else:
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
                noerror=obj.check_params(params[k],userval)
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
                        noerror = obj.check_params(subkey[j],myf[subkey[j]])
                    if(printtext):
                        if(myf[subkey[j]]==paramval):
                            print_params_col(subkey[j],myf[subkey[j]],obj.description(subkey[j],userval),'spdef',comment, noerror)
                        else:
                            print_params_col(subkey[j],myf[subkey[j]],obj.description(subkey[j],userval),'spnondef',comment, noerror)
		        itsparams[params[k]] = myf[params[k]]
    #print itsparams                    
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
    a=inspect.stack()
    stacklevel=0
    for k in range(len(a)):
        if (string.find(a[k][1], 'ipython console') > 0):
            stacklevel=k
    myf=sys._getframe(stacklevel).f_globals
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

def saveinputs(taskname=None, outfile='', myparams=None):
    #parameter_printvalues(arg_names,arg_values,arg_types)
    """ Save current input values to file on disk for a specified task:

    taskname -- Name of task
        default: <unset>; example: taskname='bandpass'
        <Options: type tasklist() for the complete list>
    outfile -- Output file for the task inputs
        default: taskname.saved; example: outfile=taskname.orion

    """

    try:
        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
            if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel=k
        myf=sys._getframe(stacklevel).f_globals
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
        ##make sure unfolded parameters get their default values
        myf['update_params'](func=myf['taskname'], printtext=False)
        ###
        taskparameterfile=open(outfile,'w')
        print >>taskparameterfile, '%-15s    = "%s"'%('taskname', taskname)
        f=zip(myf[taskname].__call__.func_code.co_varnames,myf[taskname].__call__.func_defaults)
        scriptstring='#'+str(taskname)+'('
	if myparams == None :
		myparams = {}
        for j in range(len(f)):
            k=f[j][0]
	    if not myparams.has_key(k) and k != 'self' :
		    myparams[k] = myf[k]
            if(k != 'self' and type(myparams[k])==str):
                if ( myparams[k].count( '"' ) < 1 ):
                    # if the string doesn't contain double quotes then
                    # use double quotes around it in the parameter file.
                    print >>taskparameterfile, '%-15s    =  "%s"'%(k, myparams[k])
                    scriptstring=scriptstring+k+'="'+myparams[k]+'",'
                else:
                    # use single quotes.
                    print >>taskparameterfile, "%-15s    =  '%s'"%(k, myparams[k])
                    scriptstring=scriptstring+k+"='"+myparams[k]+"',"
            else :
                if ( j != 0 or k != "self" or
                     str(type(myf[taskname])) != "<type 'instance'>" ) :
                    print >>taskparameterfile, '%-15s    =  %s'%(k, myparams[k])
                    scriptstring=scriptstring+k+'='+str(myparams[k])+','

            ###Now delete varianle from global user space because
            ###the following applies: "It would be nice if one
            ### could tell the system to NOT recall
            ### previous non-default settings sometimes."
            if(not myf['casaglobals']):
                del myf[k]
        scriptstring=scriptstring.rstrip(',')
        scriptstring=scriptstring+')'        
        print >>taskparameterfile,scriptstring
        taskparameterfile.close()
    except TypeError, e:
        print "saveinputs --error: ", e

def default(taskname=None):
    """ reset given task to its default values :

    taskname -- Name of task


    """

    try:
        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
            if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel=k
                break
        myf=sys._getframe(stacklevel).f_globals
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
        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
            if (string.find(a[k][1], 'ipython console') > 0) or (string.find(a[k][1], '<string>') >= 0):
                stacklevel=k
                break
        paramgui.setGlobals(sys._getframe(stacklevel).f_globals)
    else:
        paramgui.setGlobals({})

    paramgui.runAll(_ip)
    paramgui.setGlobals({})

####################

def exit():
    __IPYTHON__.exit_now=True
    #print 'Use CNTRL-D to exit'
    #return

import pylab as pl

#
# 
import platform
##
## CAS-951: matplotlib unresponsive on some 64bit systems
##

if (platform.architecture()[0]=='64bit'):
    pl.ioff( )
    pl.clf( )
    pl.ion( )
##
##

# Provide flexibility for boolean representation in the CASA shell
true  = True
T     = True
false = False
F     = False

# Case where casapy is run non-interactively
try:
    noipython=casapy_flags.index('--noipython')
    ipython = False
except ValueError,e:
    ipython = True

# setup available tasks
#
from math import *
from tasks import *
from parameter_dictionary import *
from task_help import *
#
home=os.environ['HOME']
if ipython:
    startup()

# assignment protection
#
pathname=os.environ.get('CASAPATH').split()[0]
uname=os.uname()
unameminusa=str.lower(uname[0])
fullpath=pathname+'/'+unameminusa+'/python/2.5/assignmentFilter.py'
casalog.origin('casa')
##
## /CASASUBST/python_library_directory/  is substitued at build time
##
fullpath='/CASASUBST/python_library_directory/' + '/assignmentFilter.py'

if os.environ.has_key('__CASAPY_PYTHONDIR'):
    fullpath=os.environ['__CASAPY_PYTHONDIR'] + '/assignmentFilter.py'

if ipython:
    try:
        script=casapy_flags.index('-c')
        scriptindex=script+1
        print 'will execute script',casapy_flags[scriptindex]
        if os.path.exists( home+'/.casa/ipython/ipy_user_conf.py' ) :
            ipshell = IPython.Shell.IPShell( argv=['-prompt_in1','CASA <\#>: ','-autocall','2','-colors','LightBG','-logfile','ipython.log','-ipythondir',home+'/.casa/ipython','-c','execfile("'+casapy_flags[scriptindex]+'")'], user_ns=globals() )
        else:
            ipshell = IPython.Shell.IPShell( argv=['-prompt_in1','CASA <\#>: ','-autocall','2','-colors','LightBG','-logfile','ipython.log','-upgrade','-ipythondir',home+'/.casa/ipython','-c','execfile("'+casapy_flags[scriptindex]+'")'], user_ns=globals() )
    except ValueError,e:
        if os.path.exists( home+'/.casa/ipython/ipy_user_conf.py' ) :
            ipshell = IPython.Shell.IPShell( argv=['-prompt_in1','CASA <\#>: ','-autocall','2','-colors','LightBG','-logfile','ipython.log','-ipythondir',home+'/.casa/ipython'], user_ns=globals() )
        else:
            ipshell = IPython.Shell.IPShell( argv=['-prompt_in1','CASA <\#>: ','-autocall','2','-colors','LightBG','-logfile','ipython.log','-upgrade','-ipythondir',home+'/.casa/ipython'], user_ns=globals() )
        ipshell.IP.runlines('execfile("'+fullpath+'")')

#ipshell = IPython.Shell.IPShell( argv=['-prompt_in1','CASA <\#>: ','-autocall','2','-colors','LightBG','-logfile','ipython.log','-ipythondir',home+'/.casa/ipython'], user_ns=globals() )
casalog.setlogfile(thelogfile)
casalog.showconsole(showconsole)
casalog.version()

import shutil
if ipython:
    ipshell.mainloop( )
    for pid in logpid: 
        #print 'pid: ',pid
        os.kill(pid,9)

    for x in os.listdir('.'):
       if x.lower().startswith('casapy.scratch-'):
          if os.path.isdir(x):
             #shutil.rmtree(x, ignore_errors=True)
             os.system("rm -rf %s" % x)
             #print "Removed: ", x, "\n"

    if vwrpid!=9999: os.kill(vwrpid,9)
    print "leaving casapy..."
