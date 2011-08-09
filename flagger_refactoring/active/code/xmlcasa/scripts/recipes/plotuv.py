def plotuv(vis, colors=['r', 'y', 'g', 'b'], symb=',', ncycles=1,
           maxnpts=100000, spw='*', field='*', antenna='*'):
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
    ncolors = ncycles * len(colors)
    try:
        symbs = [c + symb for c in colors]
    except Exception, e:
        print "Exception %s forming the plot symbols out of %s and %s" % (e, colors, symb)
        return False
    nsymbs = len(symbs)

    title = vis
    try:
        # Convert '' to '*' for ms.msseltoindex.
        if not spw:
            spw = '*'
        if not field:
            field = '*'
        if not antenna:
            antenna = '*'
        if antenna != '*':
            print "Sorry, antenna selection is ignored for now."
        msselindices = ms.msseltoindex(vis, spw=spw, field=field)
    except Exception, e:
        print "Exception %s parsing the spw and field selection." % e
        return False
    
    try:
        tb.open(vis + '/SPECTRAL_WINDOW')
        chfs = tb.getvarcol('CHAN_FREQ')
        tb.close()
        nspw = len(chfs)
        if nspw > 1:
            # Bite the bullet now instead of while the main table is open.
            tb.open(vis + '/DATA_DESCRIPTION')
            dd_to_spw = tb.getcol('SPECTRAL_WINDOW_ID')
            tb.close()
        else:
            dd_to_spw = [0]
    except Exception, e:
        print "Exception %s getting the frequencies from %s" % (e, vis)
        return False

    def colorind(f):
        if freqspan > 0:
            return min(int(ncolors * (f - globminf) / freqspan),
                       ncolors - 1)
        else:
            return 0

    try:
        tb.open(vis)
        # I'm sure nested queries can be done, but I want to avoid temp tables
        # on disk.
        #st = tb.query('FIELD_ID==' + str(msselindices['field'][0]),
        #              columns='UVW, DATA_DESC_ID')
        basequery = 'FIELD_ID==' + str(msselindices['field'][0])
        if len(msselindices['field']) > 1:
            print "Sorry, plotuv can only show one field per call."
        if len(msselindices['field']) > 1 or field != '*':
            title += ', field ' + str(msselindices['field'][0])
        nbl = tb.nrows()
        #print "nbl:", nbl

        # Figure out how to divvy up the plotting among the frequencies.
        if nspw > 1:
            ddids = tb.getcol('DATA_DESC_ID')
            usedddids = {}
            for d in ddids:
                if dd_to_spw[d] in msselindices['spw']:
                    usedddids[d] = dd_to_spw[d]
            ddids = list(usedddids.keys())
            ddids.sort()
            usedspws = list(set(usedddids.values()))
            usedspws.sort()
        else:
            ddids = [0]
            usedspws = [0]
        maxddid = ddids[-1]
        minddid = ddids[0]
        minmax = {}
        globminf = -1
        globmaxf = -1
        for s in usedspws:
            r = 'r' + str(s + 1)
            minf = chfs[r][0, 0]
            maxf = chfs[r][-1, 0]
            if maxf < minf:
                minf, maxf = maxf, minf
            minmax[s] = (minf, maxf)
            #print "minmax[s]:", minmax[s]
            if minf < globminf or globminf < 0.0:
                globminf = minf
            if maxf > globmaxf:
                globmaxf = maxf
        freqspan = globmaxf - globminf
        if maxnpts > 0 and nbl * (1 + colorind(globmaxf) -
                                  colorind(globminf)) > maxnpts:
            casalog.post(
     "Only plotting %d out of %d (scaled) baselines to conserve memory" % (maxnpts,
                              nbl * (1 + colorind(globmaxf) - colorind(globminf))),
                         'WARN')

        pl.clf()
        for d in ddids:
            s = dd_to_spw[d]
            chfkey = 'r' + str(s + 1)
            maxci = colorind(minmax[s][1])
            minci = colorind(minmax[s][0])
            ncolsspanned = 1 + maxci - minci

            # Get the subset of UVW that will be plotted for this spw.
            st = tb.query(basequery + ' and DATA_DESC_ID==' + str(d), columns='UVW')
            uvw = 0.001 * st.getcol('UVW')
            snbl = st.nrows()
            st.close()
            if maxnpts > 0:
                ntoplot = (maxnpts * snbl) / (nbl * ncolsspanned)
            else:
                ntoplot = snbl
            if ntoplot < snbl:
                uvinds = [((snbl - 1) * uvi) / (ntoplot - 1) for uvi in xrange(ntoplot)]
                u = uvw[0, uvinds]
                v = uvw[1, uvinds]
            else:
                u = uvw[0, :]
                v = uvw[1, :]
            del uvw
        
            freqspread = minmax[s][1] - minmax[s][0]
            # It'd be easier to just divide the frequency range by ncolors, but those
            # frequencies might not land on real channels.
            nchans = chfs[chfkey].shape[0]
            if ncolsspanned > 1:
                cinds = [((nchans - 1) * c) / (ncolsspanned - 1) for c in
                         xrange(ncolsspanned)]
            else:
                cinds = [nchans / 2]
            wvlngths = 2.9978e8 / chfs[chfkey].flatten()[cinds]

            # All this fussing with permutations and sieves is to give all the
            # frequencies a chance at being seen, at least in the case of a
            # single spw.  This way a channel will only blot out the plot where
            # either it really does have a much higher density than the others
            # or all the channels overlap.
            perm = pl.array(range(ncolsspanned - 1, -1, -1))
            #print "****s:", s
            #print "perm:", perm
            #print "ntoplot:", ntoplot
            #print "ncolsspanned:", ncolsspanned
            #print "minci:", minci
            for si in perm:
                #print '(perm + si) % ncolsspanned =', (perm + si) % ncolsspanned
                for ci in (perm + si) % ncolsspanned:
                    symb = symbs[(ci + minci) % nsymbs]
                    wvlngth = wvlngths[ci]
                    pl.plot( u[si:ntoplot:ncolsspanned] / wvlngth,
                             v[si:ntoplot:ncolsspanned] / wvlngth, symb)
                    pl.plot(-u[si:ntoplot:ncolsspanned] / wvlngth,
                            -v[si:ntoplot:ncolsspanned] / wvlngth, symb)
        
        pl.axis('equal')
        pl.axis('scaled')
        pl.xlabel('u (k$\lambda$)')
        pl.ylabel('v (k$\lambda$)')
        pl.title(title)
        tb.close()
    except Exception, e:
        print "Exception %s getting the UVWs from %s" % (e, vis)
        return False
    return True
