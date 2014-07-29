import os
from taskinit import *


def listobs(
    vis, selectdata, spw, field, antenna, uvrange,
    timerange, correlation, scan, intent, feed,
    array, observation, verbose, listfile, listunfl, cachesize
):
    
    """List data set summary in the logger:

       Lists following properties of a measurement set:
       scan list, field list, spectral window list with
       correlators, antenna locations, ms table information.

       Keyword arguments:
       vis -- Name of input visibility file
               default: none. example: vis='ngc5921.ms'
       selectdata -- select data from the MS
       verbose -- level of detail
             verbose=True: (default); scan and antenna lists
             verbose=False: less information
       listfile -- save the output in a file
             default: none. Example: listfile="mylist.txt"

       """

    casalog.origin('listobs')

       # Python script
       # parameter_printvalues(arg_names,arg_values,arg_types)
    try:
        if (type(vis) == str) & os.path.exists(vis):
            ms.open(thems=vis)
        else:
            raise Exception, \
                'Visibility data set not found - please verify the name'
                
        sel = {}
        if (selectdata):
            sel['spw'] = spw
            sel['time'] = timerange
            sel['field'] = field
            sel['baseline'] = antenna
            sel['scan'] = scan
            sel['scanintent'] = intent
            sel['polarization'] = correlation
            sel['uvdist'] = uvrange
            sel['observation'] = str(observation)
            sel['array'] = array
            sel['feed'] = feed

        # Select the data. Only-parse is set to false.
        ms.msselect(sel, False)
            
        ms.summary(verbose=verbose, listfile=listfile, listunfl=listunfl, cachesize=cachesize)

        ms.close()
    except Exception, instance:
        ms.close()
        print '*** Error ***', instance


