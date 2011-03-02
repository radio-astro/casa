import os
from taskinit import *

import asap as sd

def sdlist(sdfile, antenna, scanaverage, listfile, overwrite):

        casalog.origin('sdlist')

        try:
            if sdfile=='':
                    raise Exception, 'sdfile is undefined'
            filename = os.path.expandvars(sdfile)
            filename = os.path.expanduser(filename)
            if not os.path.exists(filename):
                s = "File '%s' not found." % (filename)
                raise Exception, s
            if not overwrite and not listfile=='':
                listfilename = os.path.expandvars(listfile)
                listfilename = os.path.expanduser(listfilename)
                if os.path.exists(listfilename):
                    s = "Output file '%s' exist." % (listfilename)
                    raise Exception, s


            #load the data with or without averaging
            s=sd.scantable(sdfile,average=scanaverage,antenna=antenna)

            sum = s._summary()
            
            #if ( listfile == '' ):
                    #sum = s.summary()
            #else:
                    #sum = s.summary(listfile)
            if ( listfile != '' ):
                    f = open( listfile, 'w' )
                    f.write( sum )
                    f.close()

            #if ( sd.rcParams['verbose'] == 'False'):
                    # print the summary to the screen manually
                    #print sum
            casalog.post( sum )
            casalog.post( '--------------------------------------------------------------------------------' )

            # Clean up scantable
            del s

            # DONE
        except Exception, instance:
                #print '***Error***',instance
                casalog.post( str(instance), priority = 'ERROR' )
                return

