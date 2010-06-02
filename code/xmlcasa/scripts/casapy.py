import os
import sys
import time
import signal

##
## tweak path... where necessary...
##
path_addition = [ ]
for p in sys.path :
    if p.startswith(sys.prefix):
        # According to
        # http://www.debian.org/doc/packaging-manuals/python-policy/ch-python.html
        # "Python modules not handled by python-central or python-support must
        # be installed in the system Python modules directory,
        # /usr/lib/pythonX.Y/dist-packages for python2.6 and later, and
        # /usr/lib/pythonX.Y/site-packages for python2.5 and earlier."
        #
        # I am not sure if this is general Python policy, or just Debian policy.
        # In any case, dist-packages is a more honestly named site-packages, since
        # nominally site stuff goes in /usr/local or /opt.
        #
        for wanted in ['lib-tk', 'dist-packages']:
            newpath = p + os.sep + wanted
            if os.path.isdir(newpath):
                path_addition.append(newpath)

sys.path = sys.path + path_addition

# i.e. /usr/lib/pymodules/python2.6, needed for matplotlib in Debian and its derivatives.
pymodules_dir = sys.prefix + '/lib/pymodules/python' + '.'.join(map(str, sys.version_info[:2]))

if os.path.isdir(pymodules_dir) and pymodules_dir not in sys.path:
    sys.path.append(pymodules_dir)

##
## watchdog... which is *not* in the casapy process group
##
if os.fork( ) == 0 :
    signal.signal(signal.SIGINT, signal.SIG_IGN)
    signal.signal(signal.SIGTERM, signal.SIG_IGN)
    signal.signal(signal.SIGHUP, signal.SIG_IGN)
    ## close standard input to avoid terminal interrupts
    sys.stdin.close( )
    sys.stdout.close( )
    sys.stderr.close( )
    os.close(0)
    os.close(1)
    os.close(2)
    ppid = os.getppid( )
    while True :
        try:
            os.kill(ppid,0)
        except:
            break
        time.sleep(3)
    os.killpg(ppid, signal.SIGTERM)
    time.sleep(6)
    os.killpg(ppid, signal.SIGKILL)
    exit(0)

##
## ensure that we're the process group leader
## of all processes that we fork...
##
os.setpgid(0,0)


##
## no one likes a bloated watchdog...
## ...do this after setting up the watchdog
##
try:
    import casac
except ImportError, e:
    print "failed to load casa:\n", e
    exit(1)

try:
    import matplotlib
except ImportError, e:
    print "failed to load matplotlib:\n", e
    print "sys.path =", "\n\t".join(sys.path)
    
from asap_init import *


homedir = os.getenv('HOME')
if homedir == None :
   print "Environment variable HOME is not set, please set it"
   exit(1)

casa = { 'build': {
             'time': '/CASASUBST/build_time',
             'version': '/CASASUBST/casa_version',
             'number': '/CASASUBST/casa_build'
         },
         'source': {
             'url': '/CASASUBST/subversion_url',
             'revision': '/CASASUBST/subversion_revision'
         },
         'helpers': {
             'logger': 'casalogger',
             'viewer': 'casaviewer',
             'dbus': None
         },
         'dirs': {
             'rc': homedir + '/.casa'
         },
         'flags': { },
         'files': { },
         'state' : { 'startup': True }
       }


## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
## setup dbus-daemon launch path...
## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
##     first try to find dbus launch script in likely system areas
## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
##
##   for exe in ['dbus-daemon', 'dbus-daemon-1']:
##
##  hosts which have dbus-daemon-1 but not dbus-daemon seem to have
##  a broken dbus-daemon-1...
##
for exe in ['dbus-daemon']:
    for dir in ['/bin', '/usr/bin', '/opt/local/bin', '/usr/lib/qt-4.3.4/dbus/bin', '/usr/lib64/qt-4.3.4/dbus/bin'] :
        dd = dir + os.sep + exe
        if os.path.exists(dd) and os.access(dd,os.X_OK) :
            casa['helpers']['dbus'] = dd
            break
    if casa['helpers']['dbus'] is not None:
        break

## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
##     next search through $PATH for dbus launch script
## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
if casa['helpers']['dbus'] is None:
    for exe in ['dbus-daemon']:
        for dir in os.getenv('PATH').split(':') :
            dd = dir + os.sep + exe
            if os.path.exists(dd) and os.access(dd,os.X_OK) :
                casa['helpers']['dbus'] = dd
                break
        if casa['helpers']['dbus'] is not None:
            break

print "CASA Version " + casa['build']['version'] + " (r" + casa['source']['revision'] + ")\n  Compiled on: " + casa['build']['time']

a = [] + sys.argv             ## get a copy from goofy python
a.reverse( )
while len(a) > 0:
    c = a.pop()
    ##
    ## we join multi-arg parameters here
    ##
    if c == '--logfile' or c == '-c' or c == '--rcdir':
        if len(a) == 0 :
            print "A file must be specified with " + c + "..."
            exit(1)
        else :
            casa['flags'][c] = a.pop( )
            if c == '--rcdir':
                casa['dirs']['rc'] = casa['flags'][c]

    elif c.find('=') > 0 :
        casa['flags'][c[0:c.find('=')]] = c[c.find('=')+1:]

    else :
        casa['flags'][c] = ''


if os.uname()[0]=='Darwin' :
    casa_path = os.environ['CASAPATH'].split()

    casa['helpers']['viewer'] = casa_path[0]+'/'+casa_path[1]+'/apps/casaviewer.app/Contents/MacOS/casaviewer'
    # In the distro of the app then the apps dir is not there and you find things in MacOS
    if not os.path.exists(casa['helpers']['viewer']) :
        casa['helpers']['viewer'] = casa_path[0]+'/MacOS/casaviewer'

    if casa['flags'].has_key('--maclogger') :
        casa['helpers']['logger'] = 'console'
    else:
        casa['helpers']['logger'] = casa_path[0]+'/'+casa_path[1]+'/apps/casalogger.app/Contents/MacOS/casalogger'

        # In the distro of the app then the apps dir is not there and you find things in MacOS
        if not os.path.exists(casa['helpers']['logger']) :
            casa['helpers']['logger'] = casa_path[0]+'/MacOS/casalogger'


## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
## ensure default initialization occurs before this point...
## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
if os.path.exists( casa['dirs']['rc'] + '/init.py' ) :
    try:
        execfile ( casa['dirs']['rc'] + '/init.py' )
    except:
        print 'Could not execute initialization file: ' + casa['dirs']['rc'] + '/init.py'
        exit(1)

## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
## on linux set up a dbus-daemon for casa because each
## x-server (e.g. Xvfb) gets its own dbus session...
## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
if os.uname()[0] == 'Linux' :
    if casa['helpers']['dbus'] is not None :

        argv_0_path = os.path.dirname(os.path.abspath(sys.argv[0]))
        dbus_path = os.path.dirname(os.path.abspath(casa['helpers']['dbus']))
        dbus_conf = None

        if argv_0_path == dbus_path :
            dbus_conf = dbus_path
            while dbus_conf != '/' and not os.path.basename(dbus_conf).startswith("lib") :
                dbus_conf = os.path.dirname(dbus_conf)
            if os.path.basename(dbus_conf).startswith("lib"):
                dbus_conf = os.path.dirname(dbus_conf) + "/etc/dbus/session.conf"
            else:
                dbus_conf = None

        (r,w) = os.pipe( )

        if os.fork( ) == 0 :
            os.close(r)
            signal.signal(signal.SIGINT, signal.SIG_IGN)
            signal.signal(signal.SIGHUP, signal.SIG_IGN)
            ## close standard input to avoid terminal interrupts
            sys.stdin.close( )
            os.close(0)
            args = [ 'casa-dbus-daemon' ]
            args = args + ['--print-address', str(w)]
            if dbus_conf is not None and os.path.exists(dbus_conf) :
                args = args + ['--config-file',dbus_conf]
            else:
                args = args + ['--session']
            os.execvp(casa['helpers']['dbus'],args)
            sys.exit(1)
        
        os.close(w)
        dbus_address = os.read(r,200)
        dbus_address = dbus_address.strip( )
        os.close(r)
        if len(dbus_address) > 0 :
            os.putenv('DBUS_SESSION_BUS_ADDRESS',dbus_address)
            os.environ['DBUS_SESSION_BUS_ADDRESS'] = dbus_address


ipythonenv  = casa['dirs']['rc'] + '/ipython'
ipythonpath = casa['dirs']['rc'] + '/ipython'
try :
   os.makedirs(ipythonpath, 0755)
except :
   pass
os.environ['IPYTHONDIR']=ipythonpath
os.environ['__CASARCDIR__']=casa['dirs']['rc']

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

logpid=[]

def casalogger(logfile=''):
    """
    Spawn a new casalogger using logfile as the filename.
    You should only call this if the casalogger dies or you close it
    and restart it again.

    Note: if you changed the name of the log file using casalog.setlogfile
    you will need to respawn casalogger with the new log filename. Eventually,
    we will figure out how to signal the casalogger with the new name but not
    for a while.
    """

    if logfile == '':
        if casa.has_key('files') and casa['files'].has_key('logfile') :
            logfile = casa['files']['logfile']
        else:
            casa['files']['logfile'] = os.getcwd( ) + '/casapy.log'
            logfile = 'casapy.log'

    pid=9999
    if (os.uname()[0]=='Darwin'):
	if casa['helpers']['logger'] == 'console':
           os.system("open -a console " + logfile)
        else:
           pid=os.spawnvp(os.P_NOWAIT,casa['helpers']['logger'],[casa['helpers']['logger'], logfile])

    elif (os.uname()[0]=='Linux'):
        pid=os.spawnlp(os.P_NOWAIT,casa['helpers']['logger'],casa['helpers']['logger'],logfile)
    else:
        print 'Unrecognized OS: No logger available'

    if (pid!=9999): logpid.append(pid)



showconsole = False

thelogfile = ''

showconsole = casa['flags'].has_key('--log2term')
if casa['flags'].has_key('--logfile') :
    thelogfile = casa['flags']['--logfile']
if casa['flags'].has_key('--nologfile') :
    thelogfile = 'null'

deploylogger = True
if casa['flags'].has_key('--nolog') :
    print "--nolog is deprecated, please use --nologger"
    deploylogger = False

if casa['flags'].has_key('--nologger') :
    deploylogger = False

if casa['flags'].has_key('--nogui') :
    deploylogger = False

if deploylogger and (thelogfile != 'null') :
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
    myf = sys._getframe(len(inspect.stack())-1).f_globals
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
        myf=sys._getframe(len(inspect.stack())-1).f_globals
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
        myf=sys._getframe(len(inspect.stack())-1).f_globals
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
	        pathname = os.environ.get('CASAPATH').split()[0]+'/share/xml'
                if not os.path.exists(pathname) :
                   pathname = os.environ.get('CASAPATH').split()[0]+'/Resources/xml'
                
	else :
	   pathname = os.environ.get('CASAPATH').split()[0]+'/share/xml'
           if not os.path.exists(pathname) :
              pathname = os.environ.get('CASAPATH').split()[0]+'/Resources/xml'
        xmlfile=pathname+'/'+myf['taskname']+'.xml'
        if(os.path.exists(xmlfile)) :
            cu.setconstraints('file://'+xmlfile);

    a=myf[myf['taskname']].defaults("paramkeys",myf)
    itsdef=myf[myf['taskname']].defaults
    itsparams=myf[myf['taskname']].parameters
    params=a
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
                if(hascheck):
                    noerror = obj.check_params(params[k],myf[params[k]],myf)
                # RI this doesn't work with numpy arrays anymore.  Noone seems
                # interested, so I'll be the red hen and try to fix it.
                myfparamsk=myf[params[k]]
                if(type(myf[params[k]])==pl.ndarray):
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
                                    itsparams.update({subkey[kk]:itsdef(params[k], None, itsparams[params[k]], subkey[kk])})
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
    myf=sys._getframe(len(inspect.stack())-1).f_globals
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

def tput(taskname=None, outfile=''):
	myf = sys._getframe(len(inspect.stack())-1).f_globals
	if taskname == None: taskname = myf['taskname']
	if type(taskname) != str:
		taskname=taskname.__name__
	myf['taskname'] = taskname
	outfile = myf['taskname']+'.last'
	saveinputs(taskname, outfile)

def saveinputs(taskname=None, outfile='', myparams=None, ipython_globals=None):
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
	    myf = sys._getframe(len(inspect.stack())-1).f_globals
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
        ##make sure unfolded parameters get their default values
        myf['update_params'](func=myf['taskname'], printtext=False, ipython_globals=myf)
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
		    myparams[k] = myf[taskname].parameters[k]
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
	myf = sys._getframe(len(inspect.stack())-1).f_globals
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
        paramgui.setGlobals(sys._getframe(len(inspect.stack())-1).f_globals)
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
ipython = not casa['flags'].has_key('--noipython')

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
#pathname=os.environ.get('CASAPATH').split()[0]
#uname=os.uname()
#unameminusa=str.lower(uname[0])
fullpath='/CASASUBST/python_library_directory//assignmentFilter.py'
casalog.origin('casa')

#
# Use something else than python's builtin help() for
# documenting casapy tasks
#
import pydoc

class casaDocHelper(pydoc.Helper):
    def help(self, request):
        if hasattr(request, 'i_am_a_casapy_task'):
            pydoc.pager('Help on ' + pydoc.text.bold(request.__name__) + ' task:\n\n' + request.__doc__)
        else:
            return pydoc.Helper.help(self, request)

pydoc.help = casaDocHelper(sys.stdin, sys.stdout)

##
## /CASASUBST/python_library_directory/  is substituted at build time
##
fullpath='/CASASUBST/python_library_directory/' + '/assignmentFilter.py'

if os.environ.has_key('__CASAPY_PYTHONDIR'):
    fullpath=os.environ['__CASAPY_PYTHONDIR'] + '/assignmentFilter.py'

if ipython:

    if os.path.exists('ipython.log') and not os.access('ipython.log', os.W_OK):
        print "Error: ipython.log is not writable"
        sys.exit(1) 

    if not os.path.exists('ipython.log'):
        try:
           f = open('ipython.log', 'w')
           f.close()
        except:
           print "Error: the directory is not writable"
           sys.exit(1) 
    
   
    if casa['flags'].has_key('-c') :
        print 'will execute script',casa['flags']['-c']
        if os.path.exists( casa['dirs']['rc']+'/ipython/ipy_user_conf.py' ) :
            ipshell = IPython.Shell.IPShell( argv=['-prompt_in1','CASA <\#>: ','-autocall','2','-colors','LightBG', '-nomessages', '-nobanner','-logfile','ipython.log','-ipythondir',casa['dirs']['rc']+'/ipython','-c','execfile("'+casa['flags']['-c']+'")'], user_ns=globals() )
        else:
            ipshell = IPython.Shell.IPShell( argv=['-prompt_in1','CASA <\#>: ','-autocall','2','-colors','LightBG', '-nomessages', '-nobanner','-logfile','ipython.log','-upgrade','-ipythondir',casa['dirs']['rc']+'/ipython','-c','execfile("'+casa['flags']['-c']+'")'], user_ns=globals() )
    else:
        if os.path.exists( casa['dirs']['rc']+'/ipython/ipy_user_conf.py' ) :
            ipshell = IPython.Shell.IPShell( argv=['-prompt_in1','CASA <\#>: ','-autocall','2','-colors','LightBG', '-nomessages', '-nobanner','-logfile','ipython.log','-ipythondir',casa['dirs']['rc']+'/ipython'], user_ns=globals() )
        else:
            ipshell = IPython.Shell.IPShell( argv=['-prompt_in1','CASA <\#>: ','-autocall','2','-colors','LightBG', '-nomessages', '-nobanner','-logfile','ipython.log','-upgrade','-ipythondir',casa['dirs']['rc']+'/ipython'], user_ns=globals() )
        ipshell.IP.runlines('execfile("'+fullpath+'")')

#ipshell = IPython.Shell.IPShell( argv=['-prompt_in1','CASA <\#>: ','-autocall','2','-colors','LightBG','-logfile','ipython.log','-ipythondir',casa['dirs']['rc']+'/ipython'], user_ns=globals() )

casalog.setlogfile(thelogfile)

try:
    casalog.post('---')
except:
    print "Error: the logfile is not writable"
    exit(1)

casalog.showconsole(showconsole)
casalog.version()

##
## warn when available memory is < 512M (clean throws and exception)
if cu.hostinfo( )['memory']['available'] < 524288:
    casalog.post( 'available memory less than 512MB (with casarc settings)\n...some things will not run correctly', 'SEVERE' )

casa['state']['startup'] = False

import shutil
if ipython:
    ipshell.mainloop( )
    if(os.uname()[0] == 'Darwin') and casa['flags'].has_key('--maclogger') :
           os.system("osascript -e 'tell application \"Console\" to quit'")
    for pid in logpid: 
        #print 'pid: ',pid
        os.kill(pid,9)

    for x in os.listdir('.'):
       if x.lower().startswith('casapy.scratch-'):
          if os.path.isdir(x):
             #shutil.rmtree(x, ignore_errors=True)
             os.system("rm -rf %s" % x)
             #print "Removed: ", x, "\n"

    ## leave killing off children to the watchdog...
    ## so everyone has a chance to die naturally...
    print "leaving casapy..."
