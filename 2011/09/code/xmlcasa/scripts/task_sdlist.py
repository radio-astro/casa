import os
from taskinit import *

import asap as sd

def sdlist(infile, antenna, scanaverage, outfile, overwrite):

        casalog.origin('sdlist')

        try:
            if infile=='':
                    raise Exception, 'infile is undefined'
            filename = os.path.expandvars(infile)
            filename = os.path.expanduser(filename)
            if not os.path.exists(filename):
                s = "File '%s' not found." % (filename)
                raise Exception, s
            if not overwrite and not outfile=='':
                outfilename = os.path.expandvars(outfile)
                outfilename = os.path.expanduser(outfilename)
                if os.path.exists(outfilename):
                    s = "Output file '%s' exist." % (outfilename)
                    raise Exception, s


            #load the data with or without averaging
            s=sd.scantable(infile,average=scanaverage,antenna=antenna)

            s._summary(outfile)
#             sum = s._summary()
            
#             #if ( outfile == '' ):
#                     #sum = s.summary()
#             #else:
#                     #sum = s.summary(outfile)
#             if ( outfile != '' ):
#                     f = open( outfile, 'w' )
#                     f.write( sum )
#                     f.close()

#             #if ( sd.rcParams['verbose'] == 'False'):
#                     # print the summary to the screen manually
#                     #print sum
#             casalog.post( sum )
#             casalog.post( '--------------------------------------------------------------------------------' )

            # Clean up scantable
            del s

            # DONE
        except Exception, instance:
                #print '***Error***',instance
                casalog.post( str(instance), priority = 'ERROR' )
                return

