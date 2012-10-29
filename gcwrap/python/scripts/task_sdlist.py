import os
from taskinit import *

import asap as sd
import sdutil

def sdlist(infile, antenna, scanaverage, outfile, overwrite):

        casalog.origin('sdlist')

        try:
            sdutil.assert_infile_exists(infile)
            sdutil.assert_outfile_canoverwrite_or_nonexistent(outfile,'ASAP',overwrite)

            #load the data with or without averaging
            s=sd.scantable(infile,average=scanaverage,antenna=antenna)

            s._summary(outfile)

            # Clean up scantable
            del s

            # DONE
        except Exception, instance:
                sdutil.process_exception(instance)
                raise Exception, instance
                return

