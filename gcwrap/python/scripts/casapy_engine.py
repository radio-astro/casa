import os
if os.environ.has_key('LD_PRELOAD'):
    del os.environ['LD_PRELOAD']
import sys
import time
import signal


homedir = os.getenv('HOME')
if homedir == None :
   print "Environment variable HOME is not set, please set it"
   sys.exit(1)
home=os.environ['HOME']


try:
   import IPython
except ImportError, e:
   print 'Failed to load IPython: ', e
   exit(1)
   
   
try:
    import matplotlib
except ImportError, e:
    print "failed to load matplotlib:\n", e
    print "sys.path =", "\n\t".join(sys.path)


try:
    import casac 
except ImportError, e:
    print "failed to load casa:\n", e
    sys.exit(1)
    
    
from asap_init import *


import casadef
casa = { 'build': {
             'time': casadef.build_time,
             'version': casadef.casa_version,
             'number': casadef.subversion_revision
         },
         'source': {
             'url': casadef.subversion_url,
             'revision': casadef.subversion_revision
         },
         'helpers': {
             'logger': 'casalogger',
             'viewer': 'casaviewer',
             'info': None,
             'dbus': None,
             'ipcontroller': None,
             'ipengine': None
         },
         'dirs': {
             'rc': homedir + '/.casa',
             'data': None,
             'recipes': casadef.python_library_directory+'/recipes',
             'root': None
         },
         'flags': { },
         'files': { 
             'logfile': os.getcwd( ) + '/casapy-'+time.strftime("%Y%m%d-%H%M%S", time.gmtime())+'.log'
         },
         'state' : { 'startup': True }
       }


# Set up casa root
if os.environ.has_key('CASAPATH') :
    __casapath__ = os.environ['CASAPATH'].split(' ')[0]
    if not os.path.exists(__casapath__ + "/data") :
        raise RuntimeError, "Unable to find the data repository directory in your CASAPATH. Please fix."
    else :
        casa['dirs']['root'] = __casapath__
        casa['dirs']['data'] = __casapath__ + "/data"
else :
    __casapath__ = casac.__file__
    while __casapath__ and __casapath__ != "/" :
        if os.path.exists( __casapath__ + "/data") :
            break
        __casapath__ = os.path.dirname(__casapath__)
    if not os.path.exists(__casapath__ + "/data") :
        raise RuntimeError, "casa path could not be determined"
    else :
        casa['dirs']['root'] = __casapath__
        casa['dirs']['data'] = __casapath__ + "/data"


# Setup helper paths
if os.path.exists( __casapath__ + "/bin/casapyinfo") :
    casa['helpers']['info'] = __casapath__ + "/bin/casapyinfo"
    

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
            casa['helpers']['logger'] = casa_path[0]+'/Resources/Logger.app/Contents/MacOS/casalogger'
            

## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
## ensure default initialization occurs before this point...
##
##      prelude.py  =>  setup/modification of casa settings
##      init.py     =>  user setup (with task access)
##
## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
if os.path.exists( casa['dirs']['rc'] + '/prelude.py' ) :
    try:
        execfile ( casa['dirs']['rc'] + '/prelude.py' )
    except:
        print str(sys.exc_info()[0]) + ": " + str(sys.exc_info()[1])
        print 'Could not execute initialization file: ' + casa['dirs']['rc'] + '/prelude.py'
        sys.exit(1)


# Check IPYTHONDIR is defined by user and make it if not there
ipythonenv  = casa['dirs']['rc'] + '/ipython'
ipythonpath = casa['dirs']['rc'] + '/ipython'
try :
   os.makedirs(ipythonpath, 0755)
except :
   pass

if(not os.environ.has_key('IPYTHONDIR')):
    os.environ['IPYTHONDIR']=ipythonpath
if(not os.path.exists(os.environ['IPYTHONDIR'])):
    os.makedirs(os.environ['IPYTHONDIR'], 0755)


os.environ['__CASARCDIR__']=casa['dirs']['rc']


# Special case if the back-end is set to MacOSX reset it
# to TkAgg as our TablePlot stuff is specific for TkAgg
if matplotlib.get_backend() == "MacOSX" :
   matplotlib.use('TkAgg')


# Check if the display environment is set if not
# switch the backend to Agg only if it's TkAgg
if not os.environ.has_key('DISPLAY') and matplotlib.get_backend() == "TkAgg" :
   matplotlib.use('Agg')


showconsole = False
deploylogger = False
thelogfile = 'null'


# Task Interface
from parameter_check import *


# CAS-951: matplotlib unresponsive on some 64bit systems
import platform
import pylab as pl
if (platform.architecture()[0]=='64bit'):
    if os.environ.has_key('DISPLAY') and os.environ['DISPLAY']!="" and not casa['flags'].has_key('--nogui'):
        pl.ioff( )
        pl.clf( )
        pl.ion( )


# Provide flexibility for boolean representation in the CASA shell
true  = True
T     = True
false = False
F     = False

# Import tasks but don't load task manager and dbus
os.environ['CASA_ENGINE']="YES"
from tasks import *

# Setup available tasks
from math import *
from parameter_dictionary import *
from task_help import *


## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
##
##      prelude.py  =>  setup/modification of casa settings (above)
##      init.py     =>  user setup (with task access)
##
## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
if os.path.exists( casa['dirs']['rc'] + '/init.py' ) :
    try:
        execfile ( casa['dirs']['rc'] + '/init.py' )
    except:
        print str(sys.exc_info()[0]) + ": " + str(sys.exc_info()[1])
        print 'Could not execute initialization file: ' + casa['dirs']['rc'] + '/init.py'
        sys.exit(1)


casa['state']['startup'] = False
