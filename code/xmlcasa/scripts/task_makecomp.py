def makecomp(clist=None, objname=None, standard=None, epoch=None,
             minfreq=None, maxfreq=None, nfreqs=None, uvrange=None,
             doplot=None, savefig=None):
    """
    Writes a component list named clist to disk and returns a dict of
    {'clist': clist,
     'objname': objname,
     'angdiam': angular diameter in radians (if used in clist),
     'standard': standard,
     'epoch': epoch,
     'freqs': pl.array of frequencies, in GHz,
     'uvrange': pl.array of baseline lengths, in m,
     'amps':  pl.array of predicted visibility amplitudes, in Jy,
     'savedfig': False or, if made, the filename of a plot.}
    or False on error.

    clist: directory name of the component list.
           Defaults to "%s%s_%s_%s.cl" % (objname, mjd(epoch),
                                          0.5 * (minfreq + maxfreq),
                                          uvrange)
    objname: An object supported by standard.
    standard: A standard for calculating flux densities, as in setjy.
              Default: 'Butler-JPL-Horizons 2010'
    epoch: The epoch to use for the calculations.   Irrelevant for
           extrasolar standards.
    minfreq: The minimum frequency to use.
             Example: '342.0GHz'
    maxfreq: The maximum frequency to use.
             Default: minfreq
             Example: '346.0GHz'
             Example: '', anything <= 0, or None: use minfreq.
    nfreqs:  The number of frequencies to use.
             Default: 1 if minfreq == maxfreq,
                      2 otherwise.
    uvrange: The range of baseline lengths to calculate for.
             Default: '' (None, just make clist.)
    doplot: Whether or not to show a plot of S vs. |u| on screen.
            Subparameter of uvrange.
            Default: Necessarily False if uvrange is not specified.
                     True otherwise.
    savefig: Filename for saving a plot of S vs. |u|.
             Subparameter of uvrange.
             Default: False (necessarily if uvrange is not specified)
             Examples: True (save to clist with a .png extension instead of .cl)
                       'myplot.png' (save to myplot.png) 
    """
    retval = False
    try:
        minfreqq = qa.quantity(minfreq)
        minfreqHz = qa.convert(minfreqq, 'Hz')['value']
        try:
            maxfreqq = qa.quantity(minfreq)
        except Exception, instance:
            maxfreqq = minfreqq
        frequnit = maxfreqq['unit']
        maxfreqHz = qa.convert(maxfreqq, 'Hz')['value']
        if maxfreqHz <= 0.0:
            maxfreqq = minfreqq
            maxfreqHz = minfreqHz
        if minfreqHz != maxfreqHz:
            if nfreqs < 2:
                nfreqs = 2
        else:
            nfreqs = 1
        freqs = pl.linspace(minfreqHz, maxfreqHz, nfreqs)
        
        if not clist:
            meanfreq = {'value': 0.5 * (minfreqHz + maxfreqHz),
                        'unit': frequnit}
            clist = "%s%s_%.7g%s_%s.cl" % (objname, mjd(epoch),
                                           qa.convert(meanfreq, frequnit)['value'],
                                           frequnit, uvrange)
        if savefig == True:
            savefig = clist.split('.')[:-1] + '.png'

        # Get clist

        if os.path.isdir(clist):
            retval = {'clist': clist,
                      'objname': objname,
                      'standard': standard,
                      'epoch': epoch,
                      'freqs': 1.0e-9 * freqs,
                      'uvrange': pl.array of baseline lengths, in m,
                      'amps':  pl.array of predicted visibility amplitudes, in Jy,
                      'savedfig': False or, if made, the filename of a plot.}
