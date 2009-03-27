from taskinit import *
from vishead_util import *
import re
import shutil

def fixvis(vis, outputvis, fldids=None):
    casalog.origin('fixvis')
    casalog.post("parameter vis:       " + str(vis), 'DEBUG1' )
    casalog.post("parameter fldids:    " + str(fldids), 'DEBUG1' )

    if outputvis != vis:
        try:
            #if os.path.isdir(outputvis):
            #    shutil.
            shutil.copytree(vis, outputvis)
            vis = outputvis
        except Exception, instance:
            casalog.post("*** Error %s copying %s to %s." % (instance,
                                                             vis, outputvis),
                         'SEVERE')
            return
    
    try:
        # Get field IDs before opening the ms so that tb doesn't interfere with
        # ms.
        allflds = getput_keyw('get', vis, ['FIELD', 'NAME'], '')[0]

        if not fldids:
            fldids = range(len(allflds))
        else:
            fldnams = []
            if isinstance(fldids, str):
                fldnams = re.split(r"(,|\s)+", fldids) # Is TaQL supportable?

                # I can't believe that even re.split() in Python is this stupid.
                fldnams = [f for f in fldnams if f != ' ']

            # Separate selected field names from selected field numbers.
            fldnums = []
            for i in xrange(len(fldnams)):
                try:
                    fldid = int(fldnams[i])
                    fldnums.append(fldid)
                    fldnams.pop(i)
                except:
                    pass

            fldids  = [-1 for i in range(len(allflds))]
            for i in xrange(len(allflds)):
                if (i in fldnums) or (allflds[i] in fldnams):
                    fldids[i] = i

        casalog.post("fldid vector: " + str(fldids), 'DEBUG1' )
        ms.open(vis, nomodify=False)
        ms.calcuvw(fldids)
        ms.close()        
    except Exception, instance:
        casalog.post('*** Error *** ' + str(instance), 'SEVERE')
    return
