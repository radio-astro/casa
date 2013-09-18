from taskinit import casalog, metool, qa, smtool, tbtool
from simutil import simutil
import pylab as pl
import os
import shutil
import tempfile
import matplotlib

def plotcomp(compdict, showplot=True, wantdict=False, symb=',',
             include0amp=False, include0bl=False, blunit='', bl0flux=0.0):

    """
    Given a dict including
    
    {'clist': component list,
     'objname': objname,
     'epoch': epoch,
     'shape': component shape dict, including direction.
     'freqs (GHz)': pl.array of frequencies,
     'antennalist': An array configuration file as used by simdata,
     'savedfig': False or, if specified, the filename to save the plot to,
     'standard': setjy fluxstandard type},

    and symb: One of matplotlib's codes for plot symbols: .:,o^v<>s+xDd234hH|_
          default: ',':  The smallest points I could find,

    make a plot of visibility amplitude vs. baseline length for clist at epoch.

    If antennalist is not found as is, it will look for antennalist in
    os.getenv('CASAPATH').split(' ')[0] + '/data/alma/simmos/'.
    
    showplot: Whether or not to show the plot on screen.

    If wantdict is True, it returns a dictionary with the amplitudes and
    baselines on success.  Otherwise, it returns True or False as its estimated
    success value.

    include0amp: Force the lower limit of the amplitude axis to 0.
    include0bl: Force the lower limit of the baseline length axis to 0.
    blunit: unit of the baseline length (='' used the unit in the data or klambda)
    bl0flux: Zero baseline flux
    """
    def failval():
        """
        Returns an appropriate failure value.
        Note that mydict.update(plotcomp(wantdict=True, ...)) would give a
        confusing error message if plotcomp returned False.
        """
        retval = False
        if wantdict:
            retval = {}
        return retval
    retval = failval()  # Default
    try:
        clist = compdict['clist']
        objname = compdict['objname']
        epoch = compdict['epoch']
        epstr = mepoch_to_str(epoch)
        antennalist = compdict['antennalist']

        # Read the configuration info.
        if not antennalist:
            print "compdict['antennalist'] must be set!"
            print "Try something in", os.getenv("CASAPATH").split(' ')[0] + "/data/alma/simmos/"
            return failval()
        # Try repodir if raw antennalist doesn't work.
        if not os.path.exists(antennalist):
            repodir = os.getenv("CASAPATH").split(' ')[0] + "/data/alma/simmos/"
            antennalist = repodir + antennalist

        su = simutil("")
        stnx, stny, stnz, diam, padnames, nant, telescopename = su.readantenna(antennalist)
        #print "telescopename:", telescopename

        # Check that the source is up.
        myme = metool()
        posobs = myme.observatory(telescopename)
        #print "posobs:", posobs
        myme.doframe(epoch)
        myme.doframe(posobs)
        azel = myme.measure(compdict['shape']['direction'], 'azel')
        azeldegs = tuple([qa.convert(azel[m], 'deg')['value'] for m in ('m0', 'm1')])
        casalog.post("(az, el): (%.2f, %.2f) degrees" % azeldegs)
        # riseset blabs to the logger, so introduce it now.
        casalog.post('Rise and set times of ' + objname + " from " + telescopename + ':')
        approx = ''
        if 'JPL' in compdict.get('standard', 'JPL'):
            # The object is in the Solar System or not known to be extragalactic.
            approx = "APPROXIMATE.  The times do not account for the apparent motion of "\
                     + objname + "."
            casalog.post("  (" + approx + ")")
        riset = myme.riseset(compdict['shape']['direction'])
        msg = ''
        if riset['rise'] == 'above':
            msg = objname + " is circumpolar"
        elif riset['rise'] == 'below':
            msg = objname + ' is not visible from ' + telescopename
        if msg:
            if approx:
                msg += ' around ' + mepoch_to_str(epoch)
            casalog.post(msg)
        else:
            for t in riset:
                riset[t]['str'] = mepoch_to_str(riset[t]['utc'])
            casalog.post(objname + " rises at %s and sets at %s." % (riset['rise']['str'],
                                                                     riset['set']['str']))
            tmeridian=(riset['rise']['utc']['m0']['value']+riset['set']['utc']['m0']['value'])/2.
            casalog.post(objname + ': meridian passage at ' + qa.time(str(tmeridian)+'d')[0])

        if approx:
            riset['NOTE'] = approx
        if not azel['m1']['value'] > 0.0:
            casalog.post(objname + " is not visible from " + telescopename + " at " + epstr,
                         'SEVERE')
            if wantdict:
                return riset
            else:
                return False

        # Start a temp MS.
        workingdir = os.path.abspath(os.path.dirname(clist.rstrip('/')))
        tempms = tempfile.mkdtemp(prefix=objname, dir=workingdir)

        mysm = smtool()
        mysm.open(tempms)

        su.setcfg(mysm, telescopename, stnx, stny, stnz, diam,
                  padnames, posobs)

        #print "cfg set"

        # Only 1 polarization is wanted for now.
        stokes, feeds = su.polsettings(telescopename, 'RR')
        
        casalog.post("stokes, feeds: %s, %s" % (stokes, feeds))
        fband = su.bandname(compdict['freqs (GHz)'][0])
        chaninc = 1.0
        nchan = len(compdict['freqs (GHz)'])
        if nchan > 1:
            chaninc = (compdict['freqs (GHz)'][-1] - compdict['freqs (GHz)'][0]) / (nchan - 1)
        mysm.setspwindow(spwname=fband,
                         freq=str(compdict['freqs (GHz)'][0]) + 'GHz', 
                         deltafreq=str(chaninc) + 'GHz', 
                         freqresolution='1Hz', 
                         nchannels=nchan, refcode="LSRK",
                         stokes=stokes)
        mysm.setfeed(mode=feeds, pol=[''])
        mysm.setlimits(shadowlimit=0.01, elevationlimit='10deg')
        mysm.setauto(0.0)
        mysm.setfield(sourcename=objname,
                      sourcedirection=compdict['shape']['direction'],
                      calcode="OBJ", distance='0m')
        mysm.settimes(integrationtime="1s", usehourangle=False,
                      referencetime=epoch)
        
        # this only creates blank uv entries
        mysm.observe(sourcename=objname, spwname=fband,
                   starttime="-0.5s", stoptime="0.5s", project=objname)

        mysm.setdata(fieldid=[0])
        mysm.setvp()
        casalog.post("done setting up simulation parameters")

        mysm.predict(complist=clist)        # do actual calculation of visibilities:

        mysm.close()
        casalog.post("Simulation finished.")

        mytb = tbtool()
        mytb.open(tempms)
        data = mytb.getcol('DATA')[0]       # Again, only 1 polarization for now. 
        data = abs(data)
        baselines = mytb.getcol('UVW')[:2,:]  # Drop w.
        datablunit = mytb.getcolkeywords('UVW')['QuantumUnits']
        mytb.close()
        #print "Got the data and baselines"
        shutil.rmtree(tempms)

        if datablunit[1] != datablunit[0]:
            casalog.post('The baseline units are mismatched!: %s' % datablunit,
                         'SEVERE')
            return failval()
        datablunit = datablunit[0]
        # uv dist unit in klambda or m
        if datablunit == 'm' and blunit=='klambda':
            kl = qa.constants('C')['value']/(compdict['freqs (GHz)'][0]*1e6)
            blunit = 'k$\lambda$'
        else:
            blunit = datablunit
            kl = 1.0
        pl.ioff()
        #baselines = pl.hypot(baselines[0]/kl, baselines[1]/kl)
        baselines = pl.hypot(baselines[0], baselines[1])

        #if not showplot:
        #    casalog.post('Sorry, not showing the plot is not yet implemented',
        #                 'WARN')

        if showplot: 
          pl.ion()
        pl.clf()
        pl.ioff() 
        nfreqs = len(compdict['freqs (GHz)'])
        for freqnum in xrange(nfreqs):
            freq = compdict['freqs (GHz)'][freqnum]
            casalog.post("Plotting " + str(freq) + " GHz.")
            pl.plot(baselines/kl, data[freqnum], symb, label="%.3g GHz" % freq)
            #pl.plot(baselines, data[freqnum], symb, label="%.3g GHz" % freq)
        pl.xlabel("Baseline length (" + blunit + ")")
        pl.ylabel("Visibility amplitude (Jy)")
        if include0amp:
            pl.ylim(ymin=0.0)
        if include0bl:
            pl.xlim(xmin=0.0)
        pl.suptitle(objname + " (predicted by %s)" % compdict['standard'], fontsize=14)
        #pl.suptitle(objname + " (predicted)", fontsize=14)

        # Unlike compdict['antennalist'], antennalist might have had repodir
        # prefixed to it.
        pl.title('at ' + epstr + ' for ' + os.path.basename(compdict['antennalist']), fontsize=10)
        titletxt='($%.0f^\circ$ az, $%.0f^\circ$ el)' % azeldegs
        # for comparison of old and new models - omit azeldegs as all in az~0
        if bl0flux > 0.0:
            if len(compdict['freqs (GHz)']) == 1:
                titletxt+='\n bl0 flux:%.3f Jy' % bl0flux
            else:
                titletxt+='\n bl0 flux:%.3f Jy @ %s GHz' % (bl0flux, compdict['freqs (GHz)'][0]) 
        pl.legend(loc='best', title=titletxt)
        #pl.legend(loc='best', title='($%.0f^\circ$ az, $%.0f^\circ$ el)' % azeldegs)
        y_formatter=matplotlib.ticker.ScalarFormatter(useOffset=False)
        pl.axes().yaxis.set_major_formatter(y_formatter) 
        if showplot:
          pl.ion()
          pl.draw()
        if compdict.get('savedfig'):
            pl.savefig(compdict.get('savedfig'))
            casalog.post("Saved plot to " + str(compdict.get('savedfig')))

        if wantdict:
            retval = {'amps': data,
                      'antennalist': antennalist,  # Absolute path, now.
                      'azel': azel,
                      'baselines': baselines,
                      'blunit': blunit,
                      'riseset': riset,
                      'savedfig': compdict.get('savedfig')}
        else:
            retval = True
    except Exception, instance:
        casalog.post(str(instance), 'SEVERE')
        if os.path.isdir(tempms):
            shutil.rmtree(tempms)
    return retval

def mepoch_to_str(mepoch, showdate=True, showtime=True, showmjd=True):
    """
    Given an epoch as a measure, return it as a nicely formatted string.
    """
    tdic = qa.splitdate(mepoch['m0'])
    fmt = ""
    if showdate:
        fmt += '%(year)d-%(month)02d-%(monthday)02d'
    if showtime:
        if fmt:
            fmt += '/'            
        fmt += '%(hour)02d:%(min)02d:%(sec)02d %(tz)s'
        tdic['tz'] = mepoch['refer']
    if showmjd:
        islast = False
        if fmt:
            fmt += ' ('
            islast = True
        fmt += 'MJD %(mjd).2f'
        if islast:
            fmt += ')'
    return fmt % tdic
