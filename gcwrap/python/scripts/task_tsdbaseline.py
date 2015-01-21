import numpy
import os
from taskinit import *
import sdutil
ms,sdms,tb = gentools(['ms','sdms','tb'])

def tsdbaseline(infile=None, datacolumn=None, antenna=None, field=None, spw=None, timerange=None, scan=None, pol=None, maskmode=None, thresh=None, avg_limit=None, edge=None, blmode=None, dosubtract=None, blparam=None, blformat=None, bloutput=None, bltable=None, blfunc=None, order=None, npiece=None, applyfft=None, fftmethod=None, fftthresh=None, addwn=None, rejwn=None, clipthresh=None, clipniter=None, verify=None, verbose=None, showprogress=None, minnrow=None, outfile=None, overwrite=None):

    casalog.origin('tsdbaseline')

    try:
        if ((os.path.exists(outfile)) and (not overwrite)):
            raise Exception(outfile+' exists.')
        if (blfunc.lower().strip() != 'poly'):
            raise Exception(blfunc+' is not available.')

        if (spw == ''): spw = '*'
        selection = ms.msseltoindex(vis=infile, spw=spw, field=field, 
                                    baseline='%s&&&'%(antenna), time=timerange, 
                                    scan=scan, polarization=pol)

        sdms.open(infile)
        sdms.set_selection(spw=sdutil.get_spwids(selection), field=field, 
                           antenna='%s&&&'%(antenna), timerange=timerange, 
                           scan=scan, polarization=pol)
        sdms.subtract_baseline_new(datacolumn=datacolumn,
                                   outfile=outfile,
                                   spwch=sdutil.get_spwchs(selection, infile), 
                                   order=order, 
                                   clip_threshold_sigma=clipthresh, 
                                   num_fitting_max=clipniter+1)
        """
        for spwid in spw_list:
            nchan = nchanmap[spwid]
            mask = [False]*nchan
            for channel in selection['channel']:
                if channel[0] != spwid: continue
                maskidx_start = channel[1]
                maskidx_end   = channel[2]
                for idx in xrange(maskidx_start, maskidx_end+1): 
                    mask[idx] = True
            try:
                print 'a'
                #sdms.open(infile)
                #sdms.set_selection(spw=str(spwid), field=field, 
                #                   baseline='%s&&&'%(antenna), time=timerange, 
                #                   scan=scan, polarization=pol)
                #sdms.subtract_baseline(mask=mask, order=order, 
                #                       clip_threshold_sigma=clipthresh, 
                #                       num_fitting_max=clipniter+1)
            finally:
                sdms.close()
        """

        # file output (will be implemeted later)

    except Exception, instance:
        print '*** Exception ***', instance
        raise Exception, instance
