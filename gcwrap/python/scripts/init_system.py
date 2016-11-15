import os
import sys
import time
import argparse

try:
    from casac import casac
except ImportError, e:
    print "failed to load casa:\n", e
    sys.exit(1)

try:
    import matplotlib
except ImportError, e:
    print "failed to load matplotlib:\n", e
    print "sys.path =", "\n\t".join(sys.path)

from asap_init import *


homedir = os.getenv('HOME')
if homedir == None :
   print "Environment variable HOME is not set, please set it"
   sys.exit(1)

##
## the authoritative verison comes from casac.utils, with misc
## build details (wich may vary) come from casadef...
##
import casadef
utilstool = casac.utils
cu = casac.cu = utilstool( )

from procmgr import procmgr

casa = { 'build': {
             'time': casadef.build_time,
             'version': cu.version_info( ),
             'number': casadef.subversion_revision
         },
         'source': {
             'url': casadef.subversion_url,
             'revision': casadef.subversion_revision
         },
         'helpers': {
             'crashPoster' : 'CrashReportPoster',
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
             'recipes': None,
             'root': None,
             'python': None,
             'pipeline': None,
             'xml': None
         },
         'flags': { },
         'files': {
             'logfile': os.getcwd( ) + '/casa-'+time.strftime("%Y%m%d-%H%M%S", time.gmtime())+'.log'
         },
         'state' : {
             'init_version': 1,
             'startup': True,
             'unwritable': set( )
         },
         'procmgr': procmgr( ),
       }


## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
## set up casa root
## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
if os.environ.has_key('CASAPATH') :
    __casapath__ = os.environ['CASAPATH'].split(' ')[0]
    __casaarch__ = os.environ['CASAPATH'].split(' ')[1]
    if not os.path.exists(__casapath__ + "/data") :
        print "DEBUG: CASAPATH = %s" % (__casapath__)
        raise RuntimeError, "Unable to find the data repository directory in your CASAPATH. Please fix."
    else :
        casa['dirs']['root'] = __casapath__
        casa['dirs']['data'] = __casapath__ + "/data"
        if os.path.exists(__casapath__ + "/" + __casaarch__ + "/python/2.7/assignmentFilter.py"):
            casa['dirs']['python'] = __casapath__ + "/" + __casaarch__ + "/python/2.7"
        elif os.path.exists(__casapath__ + "/lib/python2.7/assignmentFilter.py"):
            casa['dirs']['python'] = __casapath__ + "/lib/python2.7"
        elif os.path.exists(__casapath__ + "/Resources/python/assignmentFilter.py"):
            casa['dirs']['python'] = __casapath__ + "/Resources/python"

        if casa['dirs']['python'] is not None:
            casa['dirs']['recipes'] = casa['dirs']['python'] + "/recipes"

        if os.path.exists(__casapath__ + "/" + __casaarch__ + "/xml"):
            casa['dirs']['xml'] = __casapath__ + "/" + __casaarch__ + "/xml"
        elif os.path.exists(__casapath__ + "/xml"):
            casa['dirs']['xml'] = __casapath__ + "/xml"
        else:
            raise RuntimeError, "Unable to find the XML constraints directory in your CASAPATH"
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
        if os.path.exists(__casapath__ + "/" + __casaarch__ + "python/2.7/assignmentFilter.py"):
            casa['dirs']['python'] = __casapath__ + "/" + __casaarch__ + "/python/2.7"
        elif os.path.exists(__casapath__ + "/lib/python2.7/assignmentFilter.py"):
            casa['dirs']['python'] = __casapath__ + "/lib/python2.7"
        elif os.path.exists(__casapath__ + "/Resources/python/assignmentFilter.py"):
            casa['dirs']['python'] = __casapath__ + "/Resources/python"

        if casa['dirs']['python'] is not None:
            casa['dirs']['recipes'] = casa['dirs']['python'] + "/recipes"

        if os.path.exists(__casapath__ + "/" + __casaarch__ + "/xml"):
            casa['dirs']['xml'] = __casapath__ + "/" + __casaarch__ + "/xml"
        elif os.path.exists(__casapath__ + "/xml"):
            casa['dirs']['xml'] = __casapath__ + "/xml"
        else:
            raise RuntimeError, "Unable to find the XML constraints directory in your CASAPATH"

##
## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
## try to set casapyinfo path...
## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
if os.path.exists( __casapath__ + "/bin/casapyinfo") :
    casa['helpers']['info'] = __casapath__ + "/bin/casa-config"

class iArgumentParser(argparse.ArgumentParser):
    '''iPython thinks it knows that a user would never want to
    exit in any way other than typing "exit" at the command line'''
    def exit(self, status=0, message=None):
        if message:
            self._print_message(message, sys.stderr)
        os._exit(status)

argparser = iArgumentParser(prog="casa",description='Start CASA (Common Astronomy Software Applications)')

argparser.add_argument( '--rcdir',dest='rcdir',default=casa['dirs']['rc'],
                        help='location for startup files' )
argparser.add_argument( '--logfile',dest='logfile',default=casa['files']['logfile'],
                        help='path to log file' )
argparser.add_argument( "--maclogger",dest='maclogger',action='store_const',const='console',
                        default=__casapath__+'/'+__casaarch__+'/apps/casalogger.app/Contents/MacOS/casalogger',
                        help='logger to use on Apple systems' )
argparser.add_argument( "--log2term",dest='log2term',action='store_const',const=True,default=False,
                        help='direct output to terminal' )
argparser.add_argument( "--nologger",dest='nologger',action='store_const',const=True,default=False,
                        help='do not start CASA logger' )
argparser.add_argument( "--nologfile",dest='nologfile',action='store_const',const=True,default=False,
                        help='do not create a log file' )
argparser.add_argument( "--nogui",dest='nogui',action='store_const',const=True,default=False,
                        help='avoid starting GUI tools' )
argparser.add_argument( '--colors', dest='prompt', default='NoColor',
                        help='prompt color', choices=['NoColor', 'Linux', 'LightBG'] )
argparser.add_argument( "--pipeline",dest='pipeline',action='store_const',const=True,default=False,
                        help='start CASA pipeline run' )
argparser.add_argument( "-c",dest='execute',default=[],nargs='+',
                        help='python eval string or python script to execute' )

casa['flags'] = argparser.parse_args( )
#### must keep args in sync with 'casa' state...
casa['files']['logfile'] = casa['flags'].logfile
casa['dirs']['rc'] = casa['flags'].rcdir

print casa['flags']
