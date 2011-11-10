from taskinit import casalog, metool, qa, smtool, tbtool
from simutil import simutil
import pylab as pl
import os
import shutil
import tempfile

def plotcomp(compdict, showplot=True, wantdict=False, symb=','):
    """
    Given a dict including
    
    {'clist': component list,
     'objname': objname,
     'epoch': epoch,
     'shape': component shape dict, including direction.
     'freqs (GHz)': pl.array of frequencies,
     'antennalist': An array configuration file as used by simdata,
     'savedfig': False or, if specified, the filename to save the plot to.},

    and symb: One of matplotlib's codes for plot symbols: .:,o^v<>s+xDd234hH|_
          default: ',':  The smallest points I could find,

    make a plot of visibility amplitude vs. baseline length for clist at epoch.

    If antennalist is not found as is, it will look for antennalist in
    os.getenv('CASAPATH').split(' ')[0] + '/data/alma/simmos/'.
    
    showplot: Whether or not to show the plot on screen.

    If wantdict is True, it returns a dictionary with the amplitudes and
    baselines on success.  Otherwise, it returns True or False as its estimated
    success value.
    """
    retval = False
    try:
        clist = compdict['clist']
        objname = compdict['objname']
        epoch = compdict['epoch']
        antennalist = compdict['antennalist']

        # Read the configuration info.
        if not antennalist:
            print "compdict['antennalist'] must be set!"
            print "Try something in", os.getenv("CASAPATH").split(' ')[0] + "/data/alma/simmos/"
            return False
        # Try repodir if raw antennalist doesn't work.
        if not os.path.exists(antennalist):
            repodir = os.getenv("CASAPATH").split(' ')[0] + "/data/alma/simmos/"
            antennalist = repodir + antennalist

        su = simutil("")
        stnx, stny, stnz, diam, padnames, nant, telescopename = su.readantenna(antennalist)
        #print "telescopename:", telescopename

        # Start a temp MS.
        workingdir = os.path.abspath(os.path.dirname(clist.rstrip('/')))
        tempms = tempfile.mkdtemp(prefix=objname, dir=workingdir)

        mysm = smtool.create()
        myme = metool.create()
        mysm.open(tempms)
        posobs = myme.observatory(telescopename)
        #print "posobs:", posobs

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

        mytb = tbtool.create()
        mytb.open(tempms)
        data = mytb.getcol('DATA')[0]       # Again, only 1 polarization for now. 
        data = abs(data)
        baselines = mytb.getcol('UVW')[:2,:]  # Drop w.
        blunit = mytb.getcolkeywords('UVW')['QuantumUnits']
        mytb.close()
        #print "Got the data and baselines"
        shutil.rmtree(tempms)

        if blunit[1] != blunit[0]:
            casalog.post('The baseline units are mismatched!: %s' % blunit,
                         'SEVERE')
            return False
        blunit = blunit[0]
        baselines = pl.hypot(baselines[0], baselines[1])

        if not showplot:
            casalog.post('Sorry, not showing the plot is not yet implemented',
                         'WARN')

        pl.ion()
        pl.clf()
        pl.ioff()
        nfreqs = len(compdict['freqs (GHz)'])
        for freqnum in xrange(nfreqs):
            freq = compdict['freqs (GHz)'][freqnum]
            casalog.post("Plotting " + str(freq) + " GHz.")
            pl.plot(baselines, data[freqnum], symb, label="%.3g GHz" % freq)
        pl.xlabel("Baseline length (" + blunit + ")")
        pl.ylabel("Visibility amplitude (Jy)")
        epstr = mepoch_to_str(epoch)
        pl.suptitle(objname + " (predicted)", fontsize=14)

        # Unlike compdict['antennalist'], antennalist might have had repodir
        # prefixed to it.
        pl.title('at ' + epstr + ' for ' + compdict['antennalist'], fontsize=10)
        
        pl.legend(loc='best')
        pl.ion()
        pl.draw()
        if compdict.get('savedfig'):
            pl.savefig(compdict.get('savedfig'))
            casalog.post("Saved plot to " + str(compdict.get('savedfig')))

        if wantdict:
            retval = {'amps': data,
                      'antennalist': antennalist,  # Absolute path, now.
                      'baselines': baselines,
                      'blunit': blunit,
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
