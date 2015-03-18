def commongrid(vis=[], spws=[], widths=[],
               outframe='LSRK', mode='channel', restfreqs=[], veltype='radio'):

    # DP (ESO), March 2015

    """
    commongrid: Determine the frequency grid with the maximum bandwidth
                which the SPWs of the same ID in the given MSs have in common
                and generate cvel code to produce MSs with identical grids
                with the given channel widths (and if velocity mode is used
                using the given rest frequencies).

                The generated cvel code will always used mode frequency.

                When successful, the return value will be a dictionary
                containing the necessary start, width, and nchan values,
                otherwise False.

    Parameters (similar to cvel and clean):

       vis - list of MSs

       spws - list of SPW IDs for which cvel code is to be generated

       widths - the new channels widths to be achieved in each of the SPWs;
                giving a single value assumes that value is to be used for all SPWs

       outframe - as in cvel

       mode - as in cvel

       restfreqs - (only needed for mode "velocity") the restfreq to be used for each SPW

       veltype - as in cvel

    """

    if len(vis)<2:
        print "ERROR: less than two input MSs"
        return False

    if type(widths)==int or type(widths)==str:
        widthlist = []
        for i in range(0,len(spws)):
            widthlist.append(widths)
        widths = widthlist
        print "Expanding widths parameter to ", widths

    if len(spws)!=len(widths):
        print "ERROR: the lists in parameters spws and widths must have the same length"
        return False

    if mode=='velocity' and len(spws)!=len(restfreqs):
        print "ERROR: the lists in parameters spws and restfreqs must have the same length"
        return False

    print 'Using outframe = ', outframe
    
    mymst = mstool()
    grids = []

    try:
        for myms in vis:
            mymst.open(myms)
            grids.append([])
            for myspw in spws:
                if mode=='velocity':
                    grids[len(grids)-1].append(mymst.cvelfreqs(spwids=[myspw],
                                                               mode=mode,
                                                               width=widths[myspw],
                                                               outframe=outframe,
                                                               veltype=veltype,
                                                               restfreq=restfreqs[myspw])
                                               )

                else:
                    grids[len(grids)-1].append(mymst.cvelfreqs(spwids=[myspw],
                                                               mode=mode,
                                                               width=widths[myspw],
                                                               outframe=outframe)
                                               )
        mymst.close()
    except:
        print "ERROR when determining grids."
        return False


    minfreqs = []
    gridlens = []
    for i in range(0,len(spws)):
        minfreqs.append([])
        gridlens.append([])
        for j in range(0,len(vis)):
            if(len(grids[j][i])<2):
                print "ERROR: cannot handle single channel grids"
                return False
            minfreqs[len(minfreqs)-1].append(min(grids[j][i]))
            gridlens[len(gridlens)-1].append(len(grids[j][i]))

    startfreqs = []
    nchans = []
    widths = []
    
    for i in range(0,len(spws)):
        chanwidth = fabs(grids[0][i][0]-grids[0][i][1])
        
        widths.append(chanwidth)

        startfreqs.append(max(minfreqs[i]))

        thenchan = int(min(gridlens[i])- 2*ceil((max(minfreqs[i]) - min(minfreqs[i]))/chanwidth))

        nchans.append( thenchan )

    print 'mymss = ',; print vis
    print 'myspws = ',; print spws
    print 'startfreqs = ',; print startfreqs
    print 'widths = ',; print widths
    print 'nchans = ',; print nchans
    if (mode=='velocity'):
        print 'restfreqs = ',; print restfreqs

    print 'for myms in mymss:'
    print '    for myspw in myspws:'
    thecommand = 'cvel(vis=myms, mode=\'frequency\', spw=str(myspw), outframe=\''+outframe+\
                  '\',\n        '+\
                  'start=str(startfreqs[myspw])+\'Hz\', width=str(widths[myspw])+'+\
                  '\'Hz\', nchan=nchans[myspw],\n        '+\
                  'outputvis=\'cvel_\'+myms+\'_spw\'+str(myspw))'
    print '      '+thecommand

    rval = {}
    rval['outframe'] = outframe
    rval['spw'] = spws
    rval['startfreqs_hz'] = startfreqs
    rval['widths_hz'] = widths
    rval['nchans'] = nchans

    return rval
    
