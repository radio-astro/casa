import os
import time
import casadef
from procmgr import procmgr

import __casac__
_cu = __casac__.utils.utils()

_homedir = os.getenv('HOME')
if _homedir == None :
   print "Environment variable HOME is not set, please set it"
   sys.exit(1)

###
### global casa state
###
casa = { 'build': {
             'time': casadef.build_time,
             'version': _cu.version_info( ),
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
             'rc': _homedir + '/.casa',
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
