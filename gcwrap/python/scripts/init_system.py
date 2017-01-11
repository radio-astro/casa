import os
import sys
import time
import argparse
from IPython.terminal.prompts import Prompts, Token

try:
    from casac import casac
except ImportError, e:
    print "failed to load casa:\n", e
    sys.exit(1)

try:
    import matplotlib
    matplotlib.use('TkAgg')
except ImportError, e:
    print "failed to load matplotlib:\n", e
    print "sys.path =", "\n\t".join(sys.path)

from asap_init import *
from casa_system import casa

class _Prompt(Prompts):
     def in_prompt_tokens(self, cli=None):
         return [(Token.Prompt, 'CASA <'),
                 (Token.PromptNum, str(self.shell.execution_count)),
                 (Token.Prompt, '>: ')]

_ip = get_ipython()
_ip.prompts = _Prompt(_ip)

##
## toplevel frame marker
##
_casa_top_frame_ = True

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

## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
## try to set casapyinfo path...
## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
if os.path.exists( __casapath__ + "/bin/casa-config") :
    casa['helpers']['info'] = __casapath__ + "/bin/casa-config"

## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
##     first try to find executables using casapyinfo...
##            (since system area versions may be incompatible)...
##     next try likely system areas...
## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
##
##   note:  hosts which have dbus-daemon-1 but not dbus-daemon seem to have a broken dbus-daemon-1...
##
for info in [ (['dbus-daemon'],'dbus'),
              (['CrashReportPoster'],'crashPoster'),
              (['ipcontroller','ipcontroller-2.6'], 'ipcontroller'),
              (['ipengine','ipengine-2.6'], 'ipengine') ]:
    exelist = info[0]
    entry = info[1]
    for exe in exelist:
        if casa['helpers']['info']:
            casa['helpers'][entry] = (lambda fd: fd.readline().strip('\n'))(os.popen(casa['helpers']['info'] + " --exec 'which " + exe + "'"))
        if casa['helpers'][entry] and os.path.exists(casa['helpers'][entry]):
            break
        else:
            casa['helpers'][entry] = None

        ### first look in known locations relative to top (of binary distros) or known casa developer areas
        for srchdir in [ __casapath__ + '/MacOS', __casapath__ + '/lib/casa/bin', '/usr/lib64/casa/01/bin', '/opt/casa/01/bin' ] :
            dd = srchdir + os.sep + exe
            if os.path.exists(dd) and os.access(dd,os.X_OK) :
                casa['helpers'][entry] = dd
                break
        if casa['helpers'][entry] is not None:
            break

    ## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
    ##     next search through $PATH for executables
    ## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
    if casa['helpers'][entry] is None:
        for exe in exelist:
            for srchdir in os.getenv('PATH').split(':') :
                dd = srchdir + os.sep + exe
                if os.path.exists(dd) and os.access(dd,os.X_OK) :
                    casa['helpers'][entry] = dd
                    break
            if casa['helpers'][entry] is not None:
                break

## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
## try to set pipeline path...
## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
if os.path.exists(casa['dirs']['root']+"/pipeline"):
    casa['dirs']['pipeline'] = casa['dirs']['root']+"/pipeline"

# initialize/finalize Sakura library
if hasattr(casac,'sakura'):
    #casalog.post('Managing Sakura lifecycle', priority='DEBUG')
    casac.sakura().initialize_sakura()
    import atexit
    atexit.register(lambda: __import__('casac').casac.sakura().cleanup_sakura())

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

#### pipeline requires the Agg backend; any use of
#### matplotlib before 'init_pipeline.py' is loaded
#### would affect the ability to set the backend...
if casa['flags'].pipeline:
    matplotlib.use('Agg')
