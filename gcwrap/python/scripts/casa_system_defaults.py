import os
import sys
import time

_homedir = os.getenv('HOME')
if _homedir == None :
   print "Environment variable HOME is not set, please set it"
   sys.exit(1)

__curtime = time.strftime("%Y%m%d-%H%M%S", time.gmtime())
###
### global casa state
###
casa = { 'helpers': {
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
             'logfile': os.getcwd( ) + '/casa-'+__curtime+'.log',
             'iplogfile': os.getcwd( ) + '/ipython-'+__curtime+'.log'
         },
         'state' : {
             'init_version': 1,
             'startup': True,
             'unwritable': set( )
         },
       }

