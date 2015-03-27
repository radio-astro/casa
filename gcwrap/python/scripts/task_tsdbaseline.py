import numpy
import os
from taskinit import *
import sdutil
ms,sdms,tb = gentools(['ms','sdms','tb'])

def tsdbaseline(infile=None, datacolumn=None, antenna=None, field=None, spw=None, timerange=None, scan=None, pol=None, maskmode=None, thresh=None, avg_limit=None, edge=None, blmode=None, dosubtract=None, blformat=None, bloutput=None, bltable=None, blfunc=None, order=None, npiece=None, applyfft=None, fftmethod=None, fftthresh=None, addwn=None, rejwn=None, clipthresh=None, clipniter=None, blparam=None, verify=None, verbose=None, showprogress=None, minnrow=None, outfile=None, overwrite=None):

    casalog.origin('tsdbaseline')

    try:
        if ((os.path.exists(outfile)) and (not overwrite)):
            raise Exception(outfile+' exists.')
        if (maskmode!='list'):
            raise ValueError, "maskmode='%s' is not supported yet" % maskmode
        if (blfunc=='variable' and not os.path.exists(blparam)):
            raise ValueError, "input file '%s' does not exists" % blparam
        if blmode == 'fit':
            blout_exists = False
            if (isinstance(bloutput, str) and os.path.exists(bloutput)):
                blout_exists = True
            elif isinstance(bloutput, list):
                for blout in bloutput:
                    if os.path.exists(blout):
                        blout_exists = True
                        break
            if blout_exists:
                raise ValueError, "file(s) specified in bloutput exists."

        if (spw == ''): spw = '*'
        selection = ms.msseltoindex(vis=infile, spw=spw, field=field, 
                                    baseline=str(antenna), time=timerange, 
                                    scan=scan)#, polarization=pol)
        sdms.open(infile)
        sdms.set_selection(spw=sdutil.get_spwids(selection), field=field, 
                           antenna=str(antenna), timerange=timerange, 
                           scan=scan)#, polarization=pol)
        if blfunc in ['poly', 'chebyshev']:
            sdms.subtract_baseline(datacolumn=datacolumn,
                                   outfile=outfile,
                                   bltable=bloutput,
                                   dosubtract=dosubtract,
                                   spw=spw,
                                   pol=pol,
                                   order=order, 
                                   clip_threshold_sigma=clipthresh, 
                                   num_fitting_max=clipniter+1)
        elif blfunc == 'cspline':
            sdms.subtract_baseline_cspline(datacolumn=datacolumn,
                                           outfile=outfile,
                                           bltable=bloutput,
                                           dosubtract=dosubtract,
                                           spw=spw,
                                           pol=pol,
                                           npiece=npiece, 
                                           clip_threshold_sigma=clipthresh, 
                                           num_fitting_max=clipniter+1)
        elif blfunc == 'variable':
            sdms.subtract_baseline_variable(datacolumn=datacolumn,
                                            outfile=outfile,
                                            bltable=bloutput,
                                            dosubtract=dosubtract,
                                            spw=spw,
                                            pol=pol,
                                            blparam=blparam)
        else:
            raise ValueError, "Unsupported blfunc = %s" % blfunc

    except Exception, instance:
        raise Exception, instance
