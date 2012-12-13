from matplotlib.widgets import Button
from taskinit import ms, tbtool, casalog
from update_spw import expand_tilde

#from taskutil import get_global_namespace
#my_globals = get_global_namespace()
#pl = my_globals['pl']
#del my_globals

import pylab as pl

def plotuv(vis=None, field=None, antenna=None, spw=None, observation=None, array=None,
           maxnpts=None, colors=None, symb=None, ncycles=None, figfile=None):
    """
    Plots the uv coverage of vis in klambda.  ncycles of colors will be
    allocated to representative wavelengths.

    colors: a list of matplotlib color codes.
    symb: One of matplotlib's codes for plot symbols: .:,o^v<>s+xDd234hH|_
          default: ',':  The smallest points I could find.
    maxnpts: Save memory and/or screen space by plotting a maximum of maxnpts
             (or all of them if maxnpts < 1).  There is a very sharp
             slowdown if the plotter starts swapping.
    spw: spw selection string (ignores channel specifications for now).
    field: field selection string (for now, only 1 field will be plotted).
    antenna: antenna selection string (currently ignored).
    """
    casalog.origin('plotuv')
    try:
        uvplotinfo = UVPlotInfo(vis, spw, field, antenna, observation, array,
                                ncycles, colors, symb, figfile, maxnpts)
    except Exception, e:
        casalog.post("Error plotting the UVWs of %s:" % vis, 'SEVERE')
        casalog.post("%s" % e, 'SEVERE')
        return False
    retval = True
    try:
        if len(uvplotinfo.selindices['field']) > 1:
            fldnav = NavField(uvplotinfo)
            #inprogress = fldnav.show()
            fldnav.next("dummy")
        else:
            retval = plotfield(uvplotinfo.selindices['field'][0], uvplotinfo)
    except Exception, e:
        casalog.post("Error plotting the UVWs of %s:" % vis, 'SEVERE')
        casalog.post("%s" % e, 'SEVERE')
        return False
    return retval

class UVPlotInfo:
    """Gathers and holds info for a uv plot or set of them."""
    def __init__(self, vis, spw, field, antenna, observation, array,
                 ncycles, colors, symb, figfile, maxnpts):
        self.ncolors = ncycles * len(colors)
        try:
            self.symbs = [c + symb for c in colors]
        except Exception, e:
            raise ValueError, "Exception %s forming the plot symbols out of %s and %s" % (e, colors, symb)
        self.nsymbs = len(self.symbs)
        self.maxnpts = maxnpts
        try:
            self.figfile = figfile
            if figfile:
                figfileparts = figfile.split('.')
                self.ext = '.' + figfileparts[-1]
                self.figfile = '.'.join(figfileparts[:-1]) + '_fld'
        except Exception, e:
            raise ValueError, "Exception %s parsing figfile" % e

        self.vis = vis
        self.title = vis
        self.tb = tbtool()

        # Convert '' to '*' for ms.msseltoindex.
        if not spw:
            spw = '*'
        if not field:
            field = '*'
        if not antenna:
            antenna = '*'
        self.selindices = ms.msseltoindex(vis, field=field, spw=spw,
                                          baseline=antenna,
                                          observation=str(observation))
        basequery = ""
        if observation:
            basequery += 'OBSERVATION_ID in %s' % self.selindices['obsids']
        if antenna != '*':
            if basequery:
                basequery += ' and '
            basequery += 'ANTENNA1 in [%s] and ANTENNA2 in [%s]' % (
                ','.join(map(str, self.selindices['antenna1'])),
                ','.join(map(str, self.selindices['antenna2'])))
        if array:
            if basequery:
                basequery += ' and '
            arrids = expand_tilde(array)
            basequery += 'ARRAY_ID in [%s]' % ','.join(map(str, arrids))
        self.basequery = basequery

        try:
            self.fldnames = {}
            self.listfield = False
            if field != '*' or len(self.selindices['field']) > 1:
                self.listfield = True
                self.tb.open(vis + '/FIELD')
                fldnamarr = self.tb.getcol('NAME')
                self.tb.close()
                for i in xrange(len(fldnamarr)):
                    self.fldnames[i] = fldnamarr[i]
            self.subtitle = ''
            if spw != '*' or antenna != '*' or observation:
                subtitles = []
                if spw != '*':
                    subtitles.append("spw='%s'" % spw)
                if antenna != '*':
                    subtitles.append("antenna='%s'" % antenna)
                if observation:
                    subtitles.append("observation='%s'" % observation)
                self.subtitle = '(' + ', '.join(subtitles) + ')'
        except Exception, e:
            raise ValueError, "Exception %s parsing the selection." % e

        try:
            self.tb.open(vis + '/SPECTRAL_WINDOW')
            self.chfs = self.tb.getvarcol('CHAN_FREQ')
            self.tb.close()
            self.nspw = len(self.chfs)
            if self.nspw > 1:
                # Bite the bullet now instead of while the main table is open.
                self.tb.open(vis + '/DATA_DESCRIPTION')
                self.dd_to_spw = self.tb.getcol('SPECTRAL_WINDOW_ID')
                self.tb.close()
            else:
                self.dd_to_spw = [0]
        except Exception, e:
            raise ValueError, "Exception %s getting the frequencies from %s" % (e, vis)

def plotfield(fld, uvplotinfo, debug=False):
    """Plot the selected baselines of fld."""
    fldquery = uvplotinfo.basequery
    fldtitle = uvplotinfo.title
    if uvplotinfo.listfield:
        if fldquery:
            fldquery += ' and '
        fldquery += 'FIELD_ID==' + str(fld)
        fldtitle += ', field %d (%s)' % (fld, uvplotinfo.fldnames[fld])
        casalog.post('Plotting field %d (%s)' % (fld, uvplotinfo.fldnames[fld]))

    # Figure out how to divvy up the plotting among the frequencies.
    # I'm sure nested queries can be done, but I want to avoid temp tables
    # on disk.
    casalog.post('fldquery: ' + fldquery, 'DEBUG1')
    uvplotinfo.tb.open(uvplotinfo.vis)
    ftab = uvplotinfo.tb.query(fldquery, columns='DATA_DESC_ID')
    nbl = ftab.nrows()
    casalog.post("nbl: %d" % nbl, 'DEBUG1')

    if uvplotinfo.nspw > 1:
        ddids = ftab.getcol('DATA_DESC_ID')
        usedddids = {}
        for d in ddids:
            if uvplotinfo.dd_to_spw[d] in uvplotinfo.selindices['spw']:
                usedddids[d] = uvplotinfo.dd_to_spw[d]
        ddids = list(usedddids.keys())
        ddids.sort()
        usedspws = list(set(usedddids.values()))
        usedspws.sort()
    elif uvplotinfo.nspw == 1:
        ddids = [0]
        usedspws = [0]
    else:
        ddids = []
        usedspws = []
    ftab.close()

    if not ddids:
        casalog.post('Nothing selected for field %d' % fld)
        return False

    maxddid = ddids[-1]
    minddid = ddids[0]
    minmax = {}
    globminf = -1
    globmaxf = -1
    for s in usedspws:
        r = 'r' + str(s + 1)
        minf = uvplotinfo.chfs[r][0, 0]
        maxf = uvplotinfo.chfs[r][-1, 0]
        if maxf < minf:
            minf, maxf = maxf, minf
        minmax[s] = (minf, maxf)
        #print "minmax[s]:", minmax[s]
        if minf < globminf or globminf < 0.0:
            globminf = minf
        if maxf > globmaxf:
            globmaxf = maxf
    freqspan = globmaxf - globminf

    def colorind(f):
        if freqspan > 0:
            return min(int(uvplotinfo.ncolors * (f - globminf) / freqspan),
                       uvplotinfo.ncolors - 1)
        else:
            return 0
    
    if uvplotinfo.maxnpts > 0 and nbl * (1 + colorind(globmaxf) -
                              colorind(globminf)) > uvplotinfo.maxnpts:
        casalog.post(
 "Only plotting %d out of %d (scaled) baselines to conserve memory" % (uvplotinfo.maxnpts,
                          nbl * (1 + colorind(globmaxf) - colorind(globminf))),
                     'WARN')

    if fldquery:
        fldquery += ' and '
    pl.ion()                 # Magic incantation to make the plot
    pl.clf()                 # window appear (and clear it).
    pl.ioff()
    for d in ddids:
        #print "minmax[%d] = %s" % (d, minmax[d])
        s = uvplotinfo.dd_to_spw[d]
        maxci = colorind(minmax[s][1])
        minci = colorind(minmax[s][0])
        ncolsspanned = 1 + maxci - minci

        # Get the subset of UVW that will be plotted for this spw.
        st = uvplotinfo.tb.query(fldquery + 'DATA_DESC_ID==' + str(d), columns='UVW')
        snbl = st.nrows()
        #print "snbl:", snbl
        if snbl > 0:
            uvw = 0.001 * st.getcol('UVW')
            st.close()
            if uvplotinfo.maxnpts > 0:
                ntoplot = (uvplotinfo.maxnpts * snbl) / (nbl * ncolsspanned)
            else:
                ntoplot = snbl
            if ntoplot < snbl:
                uvinds = [((snbl - 1) * uvi) / (ntoplot - 1) for uvi in xrange(ntoplot)]
                casalog.post("(max, min)(uvinds) = %g, %g" % (max(uvinds), min(uvinds)),
                             'DEBUG1')
                casalog.post("len(uvw[0]) = %d" % len(uvw[0]), 'DEBUG1')
                u = uvw[0, uvinds]
                v = uvw[1, uvinds]
            else:
                u = uvw[0, :]
                v = uvw[1, :]
            del uvw
            casalog.post('len(u) = %d' % len(u), 'DEBUG1')

            freqspread = minmax[s][1] - minmax[s][0]
            # It'd be easier to just divide the frequency range by ncolors, but those
            # frequencies might not land on real channels.
            chfkey = 'r' + str(s + 1)
            nchans = uvplotinfo.chfs[chfkey].shape[0]
            casalog.post("nchans: %d" % nchans, 'DEBUG1')
            if ncolsspanned > 1:
                cinds = [((nchans - 1) * c) / (ncolsspanned - 1) for c in
                         xrange(ncolsspanned)]
            else:
                cinds = [nchans / 2]
            wvlngths = 2.9978e8 / uvplotinfo.chfs[chfkey].flatten()[cinds]

            # All this fussing with permutations and sieves is to give all the
            # frequencies a chance at being seen, at least in the case of a
            # single spw.  This way a channel will only blot out the plot where
            # either it really does have a much higher density than the others
            # or all the channels overlap.
            perm = pl.array(range(ncolsspanned - 1, -1, -1))
            if debug:
                print "****s:", s
                print "perm:", perm
                print "ntoplot:", ntoplot
                print "ncolsspanned:", ncolsspanned
                print "minci:", minci
            for si in perm:
                if debug:
                    print '(perm + si) % ncolsspanned =', (perm + si) % ncolsspanned
                for ci in (perm + si) % ncolsspanned:
                    symb = uvplotinfo.symbs[(ci + minci) % uvplotinfo.nsymbs]
                    wvlngth = wvlngths[ci]
                    #casalog.post("spw %d, lambda: %g" % (s, wvlngth), 'DEBUG1')
                    casalog.post(
                        "d %d, spw %d, si %d, ntoplot %d, ncolsspanned %d, symb %s" %
                                 (d, s, si, ntoplot, ncolsspanned, symb), 'DEBUG1')
                    pl.plot( u[si:ntoplot:ncolsspanned] / wvlngth,
                             v[si:ntoplot:ncolsspanned] / wvlngth, symb)
                    pl.plot(-u[si:ntoplot:ncolsspanned] / wvlngth,
                            -v[si:ntoplot:ncolsspanned] / wvlngth, symb)
                    casalog.post('plotted baselines both ways', 'DEBUG1')
        else:
            st.close()
    uvplotinfo.tb.close()
    casalog.post('Scaling axes', 'DEBUG1')
    pl.axis('equal')
    pl.axis('scaled')
    pl.xlabel('u (k$\lambda$)')
    pl.ylabel('v (k$\lambda$)')
    if uvplotinfo.subtitle:
        pl.suptitle(fldtitle, fontsize=14)
        pl.title(uvplotinfo.subtitle, fontsize=10)
    else:
        pl.title(fldtitle)                
    if uvplotinfo.figfile:
        pl.savefig(uvplotinfo.figfile + str(fld) + uvplotinfo.ext)
    pl.draw()
    pl.ion()
    return True

class NavField:
    def __init__(self, pltinfo):
        self.fld = -1
        self.nflds = len(pltinfo.selindices['field'])
        self.pltinfo = pltinfo
        prevwidth = 0.13
        nextwidth = 0.12
        butheight = 0.05
        butleft = 0.7
        butbot = 0.025
        butgap = 0.5 * butbot
        self.nextloc = [butleft + prevwidth + butgap,
                        butbot, nextwidth, butheight]
        self.prevloc = [butleft, butbot, prevwidth, butheight]
        self.inactivecolor = '#99aaff'
        self.activecolor = '#aaffcc'        

    def _draw_buts(self):
        if self.fld < self.nflds - 1:
            axnext = pl.axes(self.nextloc)
            self.bnext = Button(axnext, 'Next fld >',
                                color=self.inactivecolor,
                                hovercolor=self.activecolor)
            self.bnext.on_clicked(self.next)
        if self.fld > 0:
            axprev = pl.axes(self.prevloc)
            self.bprev = Button(axprev, '< Prev fld',
                                color=self.inactivecolor,
                                hovercolor=self.activecolor)
            self.bprev.on_clicked(self.prev)
        #pl.show()

    def _do_plot(self):
        didplot = plotfield(self.pltinfo.selindices['field'][self.fld],
                            self.pltinfo)
        if didplot:
            self._draw_buts()
        return didplot
    
    def next(self, event):
        didplot = False
        startfld = self.fld
        while self.fld < self.nflds - 1 and not didplot:
            self.fld += 1
            didplot = self._do_plot()
        if not didplot:
            print "You are on the last field with any selected baselines."
            self.fld = startfld
            #plotfield(self.pltinfo.selindices['field'][self.fld],
            #          self.pltinfo, self.mytb)

    def prev(self, event):
        didplot = False
        startfld = self.fld
        while self.fld > 0 and not didplot:
            self.fld -= 1
            didplot = self._do_plot()
        if not didplot:
            print "You are on the first field with any selected baselines."
            self.fld = startfld
            #plotfield(self.pltinfo.selindices['field'][self.fld],
            #          self.pltinfo, self.mytb)
