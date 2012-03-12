from taskinit import *

def peel(vis, ptcs=None, remove=None, calmode=None):
    casalog.origin('peel')
    for arg in ('vis', 'ptcs', 'remove', 'calmode'):
        casalog.post("parameter %7s: %s" % (arg, eval(arg)), 'DEBUG1')

    try:
        tb.open(vis)
        
        # What's here is a modification of James Miller-Jones' script.
        nvis=tb.nrows()

        start = 0
        incr = 10000
        while start < nvis:
            print start, 'of', nvis
            if(nvis - start < incr):
                incr = nvis - start   
            model = numpy.array(tb.getcol('MODEL_DATA', startrow=start,
                                          nrow=incr))
            corr = numpy.array(tb.getcol('CORRECTED_DATA', startrow=start,
                                         nrow=incr))
            for k in range(incr):
                corr[:, :, k] = corr[:, :, k] - model[:, :, k]
            tb.putcol('CORRECTED_DATA', corr.tolist(), startrow=start,
                      nrow=incr)
            start += incr
        tb.close()

        tb.open(vis + 'selfcal_all_ap.tab', nomodify=False)
        start = 0
        incr = 10000
        while start < nvis:
            print start, 'of', nvis
            if(nvis - start < incr):
                incr = nvis - start
            g = tb.getcol('GAIN', startrow=start, nrow=incr)
            mask = abs(g) > 0.0
            g[mask] = 1.0 / g[mask]
            tb.putcol('GAIN', g, startrow=start, nrow=incr)
            start += incr

        start = 0
        incr = 10000
        while start < nvis:
            print start, 'of', nvis
            if(nvis - start < incr):
                incr = nvis - start   

            data = numpy.array(tb.getcol('DATA', startrow=start, nrow=incr))
            corr = numpy.array(tb.getcol('CORRECTED_DATA', startrow=start,
                                         nrow=incr))
            for k in range(incr):
                corr[:, :, k] = data[:, :, k]

            tb.putcol('CORRECTED_DATA', corr.tolist(), startrow=start,
                      nrow=incr)

        # End of James' script.
        tb.close()
    except Exception, instance:
        casalog.post('*** Error *** ' + str(instance), 'SEVERE')
    return
