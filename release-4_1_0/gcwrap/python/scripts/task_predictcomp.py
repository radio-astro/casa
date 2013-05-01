from taskinit import casalog, cltool, imtool, metool, qa
from plotcomp import plotcomp
from predictcomp_helper import *
import pylab as pl
import os

def predictcomp(objname=None, standard=None, epoch=None,
                minfreq=None, maxfreq=None, nfreqs=None, prefix=None,
                antennalist=None, showplot=None, savefig=None, symb=None,
                include0amp=None, include0bl=None, blunit=None, bl0flux=None):
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
    prefix: The component list will be saved to
              prefix + 'spw0_<objname>_<minfreq><epoch>.cl'
            Default: ''
    antennalist: An array configuration file as used by simdata.
                 If given, a plot of S vs. |u| will be made.
                 Default: '' (None, just make clist.)
    showplot: Whether or not to show the plot on screen.
              Subparameter of antennalist.
              Default: Necessarily False if antennalist is not specified.
                       True otherwise.
    savefig: Filename for saving a plot of S vs. |u|.
             Subparameter of antennalist.
             Default: False (necessarily if antennalist is not specified)
             Examples: True (save to prefix + '.png')
                       'myplot.png' (save to myplot.png) 
    symb: One of matplotlib's codes for plot symbols: .:,o^v<>s+xDd234hH|_
          default: ',':  The smallest points I could find.
    include0amp: Force the lower limit of the amplitude axis to 0.
                 Default: False
    include0bl: Force the lower limit of the baseline length axis to 0.
    blunit: Unit of the baseline length 
    bl0flux: show zero baseline flux
    """
    retval = False
    try:
        casalog.origin('predictcomp')
        # some parameter minimally required
        if objname=='':
          raise Exception, "Error, objname is undefined"
        if minfreq=='':
          raise Exception, "Error, minfreq is undefined" 
        minfreqq = qa.quantity(minfreq)
        minfreqHz = qa.convert(minfreqq, 'Hz')['value']
        try:
            maxfreqq = qa.quantity(maxfreq)
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

        myme = metool()
        mepoch = myme.epoch('UTC', epoch)
        if not prefix:
            ## meanfreq = {'value': 0.5 * (minfreqHz + maxfreqHz),
            ##             'unit': frequnit}
            ## prefix = "%s%s_%.7g" % (objname, epoch.replace('/', '-'),
            ##                         minfreqq['value'])
            ## if minfreqHz != maxfreqHz:
            ##     prefix += "to" + maxfreq
            ## else:
            ##     prefix += minfreqq['unit']
            ## prefix += "_"
            prefix = ''

        # Get clist
        myim = imtool()
        if hasattr(myim, 'predictcomp'):
            casalog.post('local im instance created', 'DEBUG1')
        else:
            casalog.post('Error creating a local im instance.', 'SEVERE')
            return False
        #print "FREQS=",freqs
        if standard=='Butler-JPL-Horizons 2012':
            clist = predictSolarObjectCompList(objname, mepoch, freqs.tolist(), prefix)
        else:
            clist = myim.predictcomp(objname, standard, mepoch, freqs.tolist(), prefix)
        #print "created componentlist =",clist
        if os.path.isdir(clist):
            # The spw0 is useless here, but it is added by FluxStandard for the sake of setjy.
            casalog.post('The component list was saved to ' + clist)
            
            retval = {'clist': clist,
                      'objname': objname,
                      'standard': standard,
                      'epoch': mepoch,
                      'freqs (GHz)': 1.0e-9 * freqs,
                      'antennalist': antennalist}
            mycl = cltool()
            mycl.open(clist)
            comp = mycl.getcomponent(0)
            zeroblf=comp['flux']['value']
            if standard=='Butler-JPL-Horizons 2012':
              f0=comp['spectrum']['frequency']['m0']['value']
            else:
              f0=retval['freqs (GHz)'][0]
            casalog.post("Zero baseline flux %s @ %sGHz " % (zeroblf, f0),'INFO')
            mycl.close(False)               # False prevents the stupid warning.
            for k in ('shape', 'spectrum'):
                retval[k] = comp[k]
            if antennalist:
                retval['spectrum']['bl0flux']={}
                retval['spectrum']['bl0flux']['value']=zeroblf[0]
                retval['spectrum']['bl0flux']['unit']='Jy'
                retval['savedfig'] = savefig
                if not bl0flux:
                  zeroblf=[0.0]
                retval.update(plotcomp(retval, showplot, wantdict=True, symb=symb,
                                       include0amp=include0amp, include0bl=include0bl, blunit=blunit, bl0flux=zeroblf[0]))
            else:
                retval['savedfig'] = None
        else:
            casalog.post("There was an error in making the component list.",
                         'SEVERE')

    except Exception, instance:
        casalog.post(str(instance), 'SEVERE')
        raise Exception, instance
    return retval
