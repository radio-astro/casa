from taskinit import casalog


def ssdreduce(vis,
             field, spw,
             selectdata, timerange, antenna, scan, pol, observaiton, msselect,
             gaintable, interp, spwmap,
             maskmode, thresh, avg_limit, edge, blmask,
             blfunc, order, npiece, applyfft, fftmethod, fftthresh, addwn,
             rejwn, clipthresh, clipniter,
             bloutput, blformat,
             clipminmax, 
             kernel, kwidth, usefft, interpflag,
             statmask, stoutput, stformat):
             
    print locals()
