######################################################################
#
# Copyright (C) 2013
# Associated Universities, Inc. Washington DC, USA,
#
# This library is free software; you can redistribute it and/or modify it
# under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; if not, write to the Free Software Foundation,
# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
#
# Correspondence concerning VLA Pipelines should be addressed as follows:
#    Please register and submit helpdesk tickets via: https://help.nrao.edu
#    Postal address:
#              National Radio Astronomy Observatory
#              VLA Pipeline Support Office
#              PO Box O
#              Socorro, NM,  USA
#
######################################################################
# Functions for EVLA pipeline script
# Runs on CASA 3.4.0
# 05/01/12 CJC
# 05/03/12 STM some modified functions
# 06/19/12 STM handle channel sol flags
# 09/12/12 STM bug fix, add medians to getCalFlaggedSoln
# Runs on CASA 4.0.0
# 11/13/12 STM new SWIG calling method
# 11/19/12 STM add getCalStatistics function
# 12/17/12 STM add phases to getCalStatistics
######################################################################
from taskinit import *

def getCalFlaggedSoln(calTable):
    """
    Version 2012-05-03 v1.0 STM to 3.4 from original 3.3 version, new dictionary
    Version 2012-05-03 v1.1 STM indexed by ant, spw also
    Version 2012-09-11 v1.1 STM correct doc of <polid> indexing, bug fix, median over ant
    Version 2012-09-12 v1.2 STM median over ant revised
    Version 2012-11-13 v2.0 STM casa 4.0 version with new call mechanism
    Version 2013-01-11 v2.1 STM use getvarcol
    
    This method will look at the specified calibration table and return the
    fraction of flagged solutions for each Antenna, SPW, Poln.  This assumes
    that the specified cal table will not have any channel dependent flagging.

    return structure is a dictionary with AntennaID and Spectral Window ID
    as the keys and returns a list of fractional flagging per polarization in
    the order specified in the Cal Table.

    STM 2012-05-03 revised dictionary structure:
    key: 'all' all solutions
             ['all']['total'] = <number>
             ['all']['flagged'] = <number>
             ['all']['fraction'] = <fraction>

         'antspw' indexed by antenna and spectral window id per poln
             ['antspw'][<antid>][<spwid>][<polid>]['total'] = <total number sols per poln>
             ['antspw'][<antid>][<spwid>][<polid>]['flagged'] = <flagged number sols per poln>
             ['antspw'][<antid>][<spwid>][<polid>]['fraction'] = <flagged fraction per poln>

         'ant' indexed by antenna summed over spectral window per poln
             ['ant'][<antid>][<polid>]['total'] = <total number sols per poln>
             ['ant'][<antid>][<polid>]['flagged'] = <flagged number sols per poln>
             ['ant'][<antid>][<polid>]['fraction'] = <flagged fraction per poln>

         'spw' indexed by spectral window summed over antenna per poln
             ['spw'][<spwid>][<polid>]['total'] = <total number sols per poln>
             ['spw'][<spwid>][<polid>]['flagged'] = <flagged number sols per poln>
             ['spw'][<spwid>][<polid>]['fraction'] = <flagged fraction per poln>

         'antmedian' median fractions over antenna summed over spw and polarization
             ['total'] = median <total number sols per ant>
             ['flagged'] = median <flagged number sols per ant>
             ['fraction'] = median <flagged fraction of sols per ant>
             ['number'] = number of antennas that went into the median

    Note that fractional numbers flagged per poln are computed as a fraction of channels
    (thus a full set of channels for a given ant/spw/poln count as 1.0)

    Example:

    !cp /home/sandrock2/smyers/casa/pipeline/lib_EVLApipeutils.py .
    from lib_EVLApipeutils import getCalFlaggedSoln
    result = getCalFlaggedSoln('calSN2010FZ.G0')
    result['all']
        Out: {'flagged': 1212, 'fraction': 0.16031746031746033, 'total': 7560}
    result['antspw'][21][7]
        Out: {0: {'flagged': 3.0, 'fraction': 0.29999999999999999, 'total': 10},
              1: {'flagged': 3.0, 'fraction': 0.29999999999999999, 'total': 10}}
    result['ant'][15]
        Out: {0: {'flagged': 60.0, 'fraction': 0.42857142857142855, 'total': 140},
              1: {'flagged': 60.0, 'fraction': 0.42857142857142855, 'total': 140}}
    result['spw'][3] 
        Out: {0: {'flagged': 39.0, 'fraction': 0.14444444444444443, 'total': 270},
              1: {'flagged': 39.0, 'fraction': 0.14444444444444443, 'total': 270}}

    Bresult = getCalFlaggedSoln('calSN2010FZ.B0')
    Bresult['all']
        Out: {'flagged': 69.171875, 'fraction': 0.091497189153439157, 'total': 756}
    Bresult['ant'][15]
        Out: {0: {'flagged': 6.03125, 'fraction': 0.43080357142857145, 'total': 14},
              1: {'flagged': 6.03125, 'fraction': 0.43080357142857145, 'total': 14}}
    Bresult['antmedian']
        Out: {'flagged': 0.0625, 'fraction': 0.002232142857142857, 'number': 27, 'total': 28.0}

    Another example, to make a list of spws in the caltable that have any
    unflagged solutions in them:

    G2result = getCalFlaggedSoln('calSN2010FZ.G2.2')
    goodspw = []
    for ispw in G2result['spw'].keys():
       tot = 0
       flagd = 0
       for ipol in G2result['spw'][ispw].keys():
          tot += G2result['spw'][ispw][ipol]['total']
          flagd += G2result['spw'][ispw][ipol]['flagged']
       if tot>0:
          fract = flagd/tot
          if fract<1.0:
             goodspw.append(ispw)

    goodspw
        Out: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
    
    """

    #from taskinit import tbtool
    #mytb = tbtool.create()
    mytb = casac.table()

    import pylab as pl
    
    mytb.open(calTable)
    antCol = mytb.getcol('ANTENNA1')
    spwCol = mytb.getcol('SPECTRAL_WINDOW_ID')
    fldCol = mytb.getcol('FIELD_ID')
    #flagCol = mytb.getcol('FLAG')
    flagVarCol = mytb.getvarcol('FLAG')
    mytb.close()

    # Initialize a list to hold the results
    # Get shape of FLAG
    #(np,nc,ni) = flagCol.shape
    rowlist = flagVarCol.keys()
    nrows = len(rowlist)

    # Create the output dictionary
    outDict = {}
    outDict['all'] = {}
    outDict['antspw'] = {}
    outDict['ant'] = {}
    outDict['spw'] = {}
    outDict['antmedian'] = {}

    # Ok now go through and for each row and possibly channel compile flags
    ntotal = 0
    nflagged = 0
    # Lists for median calc
    medDict = {}
    medDict['total'] = []
    medDict['flagged'] = []
    medDict['fraction'] = []
    
    for rrow in rowlist:
        rown = rrow.strip('r')
        idx = int(rown)-1
        antIdx = antCol[idx]
        spwIdx = spwCol[idx]
        #
        flagArr = flagVarCol[rrow]
        # Get the shape of this data row
        (np,nc,ni) = flagArr.shape
        # ni should be 1 for this
        iid = 0
        #
        # Set up dictionaries if needed
        if outDict['antspw'].has_key(antIdx):
            if not outDict['antspw'][antIdx].has_key(spwIdx):
                outDict['antspw'][antIdx][spwIdx] = {}
                for poln in range(np):
                    outDict['antspw'][antIdx][spwIdx][poln] = {}
                    outDict['antspw'][antIdx][spwIdx][poln]['total'] = 0
                    outDict['antspw'][antIdx][spwIdx][poln]['flagged'] = 0
        else:
            outDict['ant'][antIdx] = {}
            outDict['antspw'][antIdx] = {}
            outDict['antspw'][antIdx][spwIdx] = {}
            for poln in range(np):
                outDict['ant'][antIdx][poln] = {}
                outDict['ant'][antIdx][poln]['total'] = 0
                outDict['ant'][antIdx][poln]['flagged'] = 0.0
                outDict['antspw'][antIdx][spwIdx][poln] = {}
                outDict['antspw'][antIdx][spwIdx][poln]['total'] = 0
                outDict['antspw'][antIdx][spwIdx][poln]['flagged'] = 0.0
        if not outDict['spw'].has_key(spwIdx):
            outDict['spw'][spwIdx] = {}
            for poln in range(np):
                outDict['spw'][spwIdx][poln] = {}
                outDict['spw'][spwIdx][poln]['total'] = 0
                outDict['spw'][spwIdx][poln]['flagged'] = 0.0
        #
        # Sum up the in-row (per pol per chan) flags for this row
        nptotal = 0
        npflagged = 0
        for poln in range(np):
            ntotal += 1
            nptotal += 1
            ncflagged = 0
            for chan in range(nc):
                if flagArr[poln][chan][iid]:
                    ncflagged += 1
            npflagged = float(ncflagged)/float(nc)
            nflagged += float(ncflagged)/float(nc)
            #
            outDict['ant'][antIdx][poln]['total'] += 1
            outDict['spw'][spwIdx][poln]['total'] += 1
            outDict['antspw'][antIdx][spwIdx][poln]['total'] += 1
            #
            outDict['ant'][antIdx][poln]['flagged'] += npflagged
            outDict['spw'][spwIdx][poln]['flagged'] += npflagged
            outDict['antspw'][antIdx][spwIdx][poln]['flagged'] += npflagged
            #

    outDict['all']['total'] = ntotal
    outDict['all']['flagged'] = nflagged
    if ntotal>0:
        outDict['all']['fraction'] = float(nflagged)/float(ntotal)
    else:
        outDict['all']['fraction'] = 0.0

    # Go back and get fractions
    for antIdx in outDict['ant'].keys():
        nptotal = 0
        npflagged = 0
        for poln in outDict['ant'][antIdx].keys():
            nctotal = outDict['ant'][antIdx][poln]['total']
            ncflagged = outDict['ant'][antIdx][poln]['flagged']
            outDict['ant'][antIdx][poln]['fraction'] = float(ncflagged)/float(nctotal)
            #
            nptotal += nctotal
            npflagged += ncflagged
        medDict['total'].append(nptotal)
        medDict['flagged'].append(npflagged)
        medDict['fraction'].append(float(npflagged)/float(nptotal))
    #
    for spwIdx in outDict['spw'].keys():
        for poln in outDict['spw'][spwIdx].keys():
            nptotal = outDict['spw'][spwIdx][poln]['total']
            npflagged = outDict['spw'][spwIdx][poln]['flagged']
            outDict['spw'][spwIdx][poln]['fraction'] = float(npflagged)/float(nptotal)
    #
    for antIdx in outDict['antspw'].keys():
        for spwIdx in outDict['antspw'][antIdx].keys():
            for poln in outDict['antspw'][antIdx][spwIdx].keys():
                nptotal = outDict['antspw'][antIdx][spwIdx][poln]['total']
                npflagged = outDict['antspw'][antIdx][spwIdx][poln]['flagged']
                outDict['antspw'][antIdx][spwIdx][poln]['fraction'] = float(npflagged)/float(nptotal)
    #
    # do medians
    outDict['antmedian'] = {}
    for item in medDict.keys():
        alist = medDict[item]
        aarr = pl.array(alist)
        amed = pl.median(aarr)
        outDict['antmedian'][item] = amed
    outDict['antmedian']['number'] = len(medDict['fraction'])
    
    return outDict
    

def buildscans(msfile):
    """
   buildscans:  compile scan information for msfile
  
   Created S.T. Myers 2012-05-07  v1.0 
   Updated S.T. Myers 2012-05-14  v1.1 add corrtype
   Updated S.T. Myers 2012-06-27  v1.2 add corrdesc lookup
   Updated S.T. Myers 2012-11-13  v2.0 STM casa 4.0 new calls
             
   Usage:
          from lib_EVLApipeutils import buildscans
  
   Input:
  
           msfile   -   name of MS
  
   Output: scandict (return value)
  
   Examples:
  
   CASA <2>: from lib_EVLApipeutils import buildscans
   
   CASA <3>: msfile = 'TRSR0045_sb600507.55900.ms'
   
   CASA <4>: myscans = buildscans(msfile)
   Getting scansummary from MS
   Found 16 DataDescription IDs
   Found 4 StateIds
   Found 3422 times in DD=0
   Found 3422 times in DD=1
   Found 3422 times in DD=2
   Found 3422 times in DD=3
   Found 3422 times in DD=4
   Found 3422 times in DD=5
   Found 3422 times in DD=6
   Found 3422 times in DD=7
   Found 3422 times in DD=8
   Found 3422 times in DD=9
   Found 3422 times in DD=10
   Found 3422 times in DD=11
   Found 3422 times in DD=12
   Found 3422 times in DD=13
   Found 3422 times in DD=14
   Found 3422 times in DD=15
   Found total 54752 times
   Found 175 scans min=1 max=180
   Size of scandict in memory is 248 bytes
   
   CASA <5>: myscans['Scans'][1]['intents']
     Out[5]: 'CALIBRATE_AMPLI#UNSPECIFIED,UNSPECIFIED#UNSPECIFIED'
   
   CASA <6>: myscans['Scans'][1]['dd']
     Out[6]: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]
   
   CASA <7>: myscans['DataDescription'][0]
     Out[7]: 
   {'corrdesc': ['RR', 'RL', 'LR'', 'LL'],
    'corrtype': [5, 6, 7, 8],
    'ipol': 0,
    'nchan': 64,
    'npol': 4,
    'reffreq': 994000000.0,
    'spw': 0,
    'spwname': 'Subband:0'}

   CASA <8>: myscans['Scans'][1]['times'][0]
     Out[8]: 
   [4829843281.500001,
    4829843282.5,
    4829843283.5,
    4829843284.5,
    ...
    4829843336.5]

   The last of these returns the integration midpoints for scan 1 DD 0.

   Note that to get spw and pol info you use the DD indexes from ['dd']
   in the 'Scans' part to index into the 'DataDescription' info.

   You can also list the keys available in the Scans sub-dictionary:
   
   CASA <9>: myscans['Scans'][1].keys()
     Out[9]: 
   ['scan_mid',
    'intents',
    'field',
    'dd',
    'npol',
    'rra',
    'spw',
    'scan_int',
    'scan_start',
    'times',
    'scan_end',
    'rdec']

    """

    # dictionary with lookup for correlation strings
    # from http://casa.nrao.edu/docs/doxygen/html/classcasa_1_1Stokes.html
    cordesclist = ['Undefined',
                   'I',
                   'Q',
                   'U',
                   'V',
                   'RR',
                   'RL',
                   'LR',
                   'LL',
                   'XX',
                   'XY',
                   'YX',
                   'YY',
                   'RX',
                   'RY',
                   'LX',
                   'LY',
                   'XR',
                   'XL',
                   'YR',
                   'YL',
                   'PP',
                   'PQ',
                   'QP',
                   'QQ',
                   'RCircular',
                   'LCircular',
                   'Linear',
                   'Ptotal',
                   'Plinear',
                   'PFtotal',
                   'PFlinear',
                   'Pangle' ]
    #
    # Usage: find desc for an index, e.g. cordesclist[corrtype]
    #        find index for a desc, e.g. cordesclist.index(corrdesc)
    #
    
    #try:
    #    import casac
    #except ImportError, e:
    #    print "failed to load casa:\n", e
    #    exit(1)
    #mstool = casac.homefinder.find_home_by_name('msHome')
    #ms = casac.ms = mstool.create()
    #tbtool = casac.homefinder.find_home_by_name('tableHome')
    #tb = casac.tb = tbtool.create()
    ms = casac.ms()
    tb = casac.table()

    # Access the MS
    try:
        ms.open(msfile,nomodify=True)
    except:
        print "ERROR: failed to open ms tool on file "+msfile
        exit(1)

    print 'Getting scansummary from MS'
    # get the scan summary using ms.getscansummary method
    #mysc = ms.getscansummary()
    scd = ms.getscansummary()

    # #nscans = len( mysc['summary'].keys() )
    # nscans = len( scd.keys() )
    # print 'Found '+str(nscans)+' scans'

    #
    # Find number of data description IDs
    tb.open(msfile+"/DATA_DESCRIPTION")
    ddspwarr=tb.getcol("SPECTRAL_WINDOW_ID")
    ddpolarr=tb.getcol("POLARIZATION_ID")
    tb.close()
    ddspwlist = ddspwarr.tolist()
    ddpollist = ddpolarr.tolist()
    ndd = len(ddspwlist)
    print 'Found '+str(ndd)+' DataDescription IDs'
    #
    # The SPECTRAL_WINDOW table
    tb.open(msfile+"/SPECTRAL_WINDOW")
    nchanarr=tb.getcol("NUM_CHAN")
    spwnamearr=tb.getcol("NAME")
    reffreqarr=tb.getcol("REF_FREQUENCY")
    tb.close()
    nspw = len(nchanarr)
    spwlookup = {}
    for isp in range(nspw):
        spwlookup[isp] = {}
        spwlookup[isp]['nchan'] = nchanarr[isp]
        spwlookup[isp]['name'] = str( spwnamearr[isp] )
        spwlookup[isp]['reffreq'] = reffreqarr[isp]
    print 'Extracted information for '+str(nspw)+' SpectralWindows'
    #
    # Now the polarizations (number of correlations in each pol id
    tb.open(msfile+"/POLARIZATION")
    ncorarr=tb.getcol("NUM_CORR")
    # corr_type is in general variable shape, have to read row-by-row
    # or use getvarcol to return into dictionary, we will iterate manually
    npols = len(ncorarr)
    polindex = {}
    poldescr = {}
    for ip in range(npols):
        cort=tb.getcol("CORR_TYPE",startrow=ip,nrow=1)
        (nct,nr) = cort.shape
        cortypes = []
        cordescs = []
        for ict in range(nct):
            cct = cort[ict][0]
            cde = cordesclist[cct]
            cortypes.append(cct)
            cordescs.append(cde)
        polindex[ip] = cortypes
        poldescr[ip] = cordescs
    # cortype is an array of npol, e.g. 5,6,7,8 is for RR,RL,LR,LL respectively
    # for alma this would be 9,10,11,12 for XX,XY,YX,YY respectively
    # cordesc are the strings associated with the types (enum for casa)
    tb.close()
    print 'Extracted information for '+str(npols)+' Polarization Setups'
    #
    # Build the DD index
    #
    ddindex = {}
    ncorlist=ncorarr.tolist()
    for idd in range(ndd):
        ddindex[idd] = {}
        isp = ddspwlist[idd]
        ddindex[idd]['spw'] = isp
        ddindex[idd]['spwname'] = spwlookup[isp]['name']
        ddindex[idd]['nchan'] = spwlookup[isp]['nchan']
        ddindex[idd]['reffreq'] = spwlookup[isp]['reffreq']
        #
        ipol = ddpollist[idd]
        ddindex[idd]['ipol'] = ipol
        ddindex[idd]['npol'] = ncorlist[ipol]
        ddindex[idd]['corrtype'] = polindex[ipol]
        ddindex[idd]['corrdesc'] = poldescr[ipol]
    #
    # Now get raw scan intents from STATE table
    tb.open(msfile+"/STATE")
    intentarr=tb.getcol("OBS_MODE")
    subscanarr=tb.getcol("SUB_SCAN")
    tb.close()
    intentlist = intentarr.tolist()
    subscanlist = subscanarr.tolist()
    nstates = intentlist.__len__()
    print 'Found '+str(nstates)+' StateIds'
    #
    # Now get FIELD table directions
    tb.open(msfile+"/FIELD")
    fnamearr=tb.getcol("NAME")
    fpdirarr=tb.getcol("PHASE_DIR")
    tb.close()
    flist = fnamearr.tolist()
    nfields = len(flist)
    (nd1, nd2, npf) = fpdirarr.shape
    fielddict = {}
    for ifld in range(nfields):
        fielddict[ifld] = {}
        fielddict[ifld]['name'] = flist[ifld]
        # these are numpy.float64
        fielddict[ifld]['rra'] = fpdirarr[0,0,ifld]
        fielddict[ifld]['rdec'] = fpdirarr[1,0,ifld]
    #
    # Now compile list of visibility times and info
    #
    # Extract times from the Main Table
    # Build lookup for DDs by scan_number
    timdict = {}
    ddlookup = {}
    ddscantimes = {}
    ntottimes=0
    for idd in range(ndd):
        # Select this DD (after reset if needed)
        if idd>0: ms.selectinit(reset=True)
        ms.selectinit(idd)
        #recf = ms.getdata(["flag"])
        #(nx,nc,ni) = recf['flag'].shape
        # get the times
        rect = ms.getdata(["time","field_id","scan_number"],ifraxis=True)
        nt = rect['time'].shape[0]
        ntottimes+=nt
        print 'Found '+str(nt)+' times in DD='+str(idd)
        #
        timdict[idd] = {}
        timdict[idd]['time'] = rect['time']
        timdict[idd]['field_id'] = rect['field_id']
        timdict[idd]['scan_number'] = rect['scan_number']
        #
        for it in range(nt):
            isc = rect['scan_number'][it]
            tim = rect['time'][it]
            if ddlookup.has_key(isc):
                if ddlookup[isc].count(idd)<1:
                    ddlookup[isc].append(idd)
                if ddscantimes[isc].has_key(idd):
                    ddscantimes[isc][idd].append(tim)
                else:
                    ddscantimes[isc][idd] = [tim]
            else:
                ddlookup[isc] = [idd]
                ddscantimes[isc] = {}
                ddscantimes[isc][idd] = [tim]
        
    #
    print 'Found total '+str(ntottimes)+' times'

    ms.close()

    # compile a list of scan times 
    #
    #scd = mysc['summary']
    scanlist = []
    scanindex = {}
    scl = scd.keys()
    for sscan in scl:
        isc = int(sscan)
        scanlist.append(isc)
        scanindex[isc]=sscan

    scanlist.sort()

    nscans = len(scanlist)
    print 'Found '+str(nscans)+' scans min='+str(min(scanlist))+' max='+str(max(scanlist))

    scantimes = []
    scandict = {}

    # Put DataDescription lookup into dictionary
    scandict['DataDescription'] = ddindex
        
    # Put Scan information in dictionary
    scandict['Scans'] = {}
    for isc in scanlist:
        sscan = scanindex[isc]
        # sub-scans, differentiated by StateId
        subs = scd[sscan].keys()
        #
        scan_start = -1.
        scan_end = -1.
        for ss in subs:
            bt = scd[sscan][ss]['BeginTime']
            et = scd[sscan][ss]['EndTime']
            if scan_start>0.:
                if bt<scan_start:
                    scan_start=bt
            else:
                scan_start=bt
            if scan_end>0.:
                if et>scan_end:
                    scan_end=et
            else:
                scan_end=et
        scan_mid = 0.5*(scan_start + scan_end)

        scan_int = scd[sscan]['0']['IntegrationTime']

        scantimes.append(scan_mid)

        scandict['Scans'][isc] = {}
        scandict['Scans'][isc]['scan_start'] = scan_start
        scandict['Scans'][isc]['scan_end'] = scan_end
        scandict['Scans'][isc]['scan_mid'] = scan_mid
        scandict['Scans'][isc]['scan_int'] = scan_int

        ifld = scd[sscan]['0']['FieldId']
        scandict['Scans'][isc]['field'] = ifld
        scandict['Scans'][isc]['rra'] = fielddict[ifld]['rra']
        scandict['Scans'][isc]['rdec'] = fielddict[ifld]['rdec']

        spws = scd[sscan]['0']['SpwIds']
        scandict['Scans'][isc]['spw'] = spws.tolist()

        # state id of first sub-scan
        stateid = scd[sscan]['0']['StateId']
        # get intents from STATE table
        intents = intentlist[stateid]
        # this is a string with comma-separated intents
        scandict['Scans'][isc]['intents'] = intents

        # DDs for this scan
        ddlist = ddlookup[isc]
        scandict['Scans'][isc]['dd'] = ddlist
        
        # number of polarizations for this list of dds
        ddnpollist = []
        for idd in ddlist:
            npol = ddindex[idd]['npol']
            ddnpollist.append(npol)
        scandict['Scans'][isc]['npol'] = ddnpollist
        
        # visibility times per dd in this scan
        #
        scandict['Scans'][isc]['times'] = {}
        for idd in ddlist:
            scandict['Scans'][isc]['times'][idd] = ddscantimes[isc][idd]
        
    mysize = scandict.__sizeof__()
    print 'Size of scandict in memory is '+str(mysize)+' bytes'
    
    return scandict
# Done

def getBCalStatistics(calTable,innerbuff=0.1):
    """
    Version 2012-11-20 v1.0 STM casa 4.0 version
    Version 2012-12-17 v1.0 STM casa 4.1 version, phase, real, imag stats
    
    This method will look at the specified B calibration table and return the
    statistics of unflagged solutions for each Antenna, SPW, Poln.  This assumes
    that the specified cal table will not have any channel dependent flagging.

    return structure is a dictionary with AntennaID and Spectral Window ID
    as the keys and returns a list of statistics per polarization in
    the order specified in the Cal Table.

    Input:
  
         calTable  -  name of MS
         innerbuff -  fraction of spw bandwidth to ignore at each edge
                      (<0.5, default=0.1 use inner 80% of channels each spw)
  
    Output: OutDict (return value)
  
    STM 2012-11-19 dictionary structure:
    key: 'antspw' indexed by antenna and spectral window id per poln
             ['antspw'][<antid>][<spwid>][<polid>]

         'antband' indexed by antenna and rx/baseband summed over subbands and poln
             ['ant'][<antid>][<rxband>][<baseband>]
             Note: this requires that the spectral window NAME follow JVLA convention of
                   rxband#baseband#spw (e.g. 'EVLA_K#A0C0#48')

    For each of these there is a data structure for ['all'] and ['inner']
    that contains the fields: ['total'], ['number'], ['mean'], ['min'], ['max']

    Some useful indexing dictionaries are also output:
       ['antDict'][ant] the name of antenna with index ant
       ['spwDict'][spw] {'RX':rx, 'Baseband':bb, 'Subband':sb} correspoding to a given spw
       ['rxBasebandDict'][rx][bb] list of spws corresponding to a given rx and bb
       
    Example:

    !cp /home/sandrock2/smyers/casa/pipeline/pipeline4.1/lib_EVLApipeutils.py .
    from lib_EVLApipeutils import getBCalStatistics
    result = getBCalStatistics('testBPcal.b')
    
    This is a B Jones table, proceeding
    Found 1 Rx bands
    Rx band EVLA_K has basebands: ['A0C0', 'B0D0']
    Within all channels:
    Found 864 total solutions with 47824 good (unflagged)
    Within inner 0.8 of channels:
    Found 0 total solutions with 42619 good (unflagged)

        AntID      AntName      Rx-band      Baseband    min/max(all)  min/max(inner) ALERT?
            0         ea01       EVLA_K         A0C0        0.3541        0.3575 
            0         ea01       EVLA_K         B0D0        0.4585        0.4591 
            1         ea02       EVLA_K         A0C0        0.3239        0.3298 
            1         ea02       EVLA_K         B0D0        0.2130        0.2179 
            2         ea03       EVLA_K         A0C0        0.3097        0.3124 
            2         ea03       EVLA_K         B0D0        0.3247        0.3282 
            3         ea04       EVLA_K         A0C0        0.3029        0.3055 
            3         ea04       EVLA_K         B0D0        0.2543        0.2543 
            4         ea05       EVLA_K         A0C0        0.2657        0.2715 
            4         ea05       EVLA_K         B0D0        0.2336        0.2357 
            5         ea06       EVLA_K         A0C0        0.3474        0.3501 
            5         ea06       EVLA_K         B0D0        0.2371        0.2398 
            6         ea07       EVLA_K         A0C0        0.2294        0.2322 
            6         ea07       EVLA_K         B0D0        0.3684        0.3690 
            7         ea08       EVLA_K         A0C0        0.2756        0.2809 
            7         ea08       EVLA_K         B0D0        0.5269        0.5269 
            8         ea10       EVLA_K         A0C0        0.2617        0.2686 
            8         ea10       EVLA_K         B0D0        0.2362        0.2407 
            9         ea11       EVLA_K         A0C0        0.2594        0.2607 
            9         ea11       EVLA_K         B0D0        0.3015        0.3111 
           10         ea12       EVLA_K         A0C0        0.2418        0.2491 
           10         ea12       EVLA_K         B0D0        0.3839        0.3849 
           11         ea13       EVLA_K         A0C0        0.3017        0.3156 
           11         ea13       EVLA_K         B0D0        0.3299        0.3299 
           12         ea14       EVLA_K         A0C0        0.2385        0.2415 
           12         ea14       EVLA_K         B0D0        0.2081        0.2101 
           13         ea15       EVLA_K         A0C0        0.3118        0.3151 
           13         ea15       EVLA_K         B0D0        0.1749        0.1844 * 
           14         ea16       EVLA_K         A0C0        0.2050        0.2051 
           14         ea16       EVLA_K         B0D0        0.4386        0.4537 
           15         ea17       EVLA_K         A0C0        0.4084        0.4107 
           15         ea17       EVLA_K         B0D0        0.2151        0.2189 
           16         ea18       EVLA_K         A0C0        0.3124        0.3147 
           16         ea18       EVLA_K         B0D0        0.3712        0.3730 
           17         ea19       EVLA_K         A0C0        0.3084        0.3132 
           17         ea19       EVLA_K         B0D0        0.2453        0.2522 
           18         ea20       EVLA_K         A0C0        0.4618        0.4672 
           18         ea20       EVLA_K         B0D0        0.2772        0.2790 
           19         ea21       EVLA_K         A0C0        0.3971        0.4032 
           19         ea21       EVLA_K         B0D0        0.2812        0.2818 
           20         ea22       EVLA_K         A0C0        0.2495        0.2549 
           20         ea22       EVLA_K         B0D0        0.3777        0.3833 
           21         ea23       EVLA_K         A0C0        0.3213        0.3371 
           21         ea23       EVLA_K         B0D0        0.2702        0.2787 
           22         ea24       EVLA_K         A0C0        0.2470        0.2525 
           22         ea24       EVLA_K         B0D0        0.0127        0.0127 *** 
           23         ea25       EVLA_K         A0C0        0.3173        0.3205 
           23         ea25       EVLA_K         B0D0        0.0056        0.0066 *** 
           24         ea26       EVLA_K         A0C0        0.3505        0.3550 
           24         ea26       EVLA_K         B0D0        0.4028        0.4071 
           25         ea27       EVLA_K         A0C0        0.3165        0.3244 
           25         ea27       EVLA_K         B0D0        0.2644        0.2683 
           26         ea28       EVLA_K         A0C0        0.1695        0.1712 * 
           26         ea28       EVLA_K         B0D0        0.3213        0.3318 

    result['antband'][22]['EVLA_K']['B0D0']
       Out: 
         {'all': {'amp': {'max': 3.3507643208628997,
                          'mean': 1.2839660621605888,
                          'min': 0.042473547230042749,
                          'var': 0.42764307104292559},
                  'imag': {'max': array(1.712250828742981),
                           'mean': -0.0042696134968516824,
                           'min': array(-2.2086853981018066),
                           'var': 0.43859873853192027},
                  'number': 900,
                  'phase': {'max': 112.32456525225054,
                            'mean': -0.70066399447640126,
                            'min': -108.23612296337477,
                            'var': 1445.9499062237583},
                  'real': {'max': array(3.3105719089508057),
                           'mean': 1.0975328904785548,
                           'min': array(-0.13342639803886414),
                           'var': 0.433613996950728},
                  'total': 1024},
          'inner': {'amp': {'max': 3.3507643208628997,
                            'mean': 1.2913493102726084,
                            'min': 0.042473547230042749,
                            'var': 0.45731194085061877},
                    'imag': {'max': array(1.712250828742981),
                             'mean': -0.0019235184823728287,
                             'min': array(-2.2086853981018066),
                             'var': 0.43650578363669668},
                    'number': 802,
                    'phase': {'max': 112.32456525225054,
                              'mean': -0.74464737957566107,
                              'min': -108.23612296337477,
                              'var': 1496.4536912732597},
                    'real': {'max': array(3.3105719089508057),
                             'mean': 1.1054519462910002,
                             'min': array(-0.13342639803886414),
                             'var': 0.4665390728630604},
                    'total': 816}}

    result['rxBasebandDict']['EVLA_K']['B0D0']
       Out: [8, 9, 10, 11, 12, 13, 14, 15]


    result['antspw'][22][10]
       Out: 
         {0: {'all': {'amp': {'max': 3.3507643208628997,
                              'mean': 1.6887832659451747,
                              'min': 0.054888048286296932,
                              'var': 0.84305637195708472},
                      'imag': {'max': array(1.4643619060516357),
                               'mean': -0.13419617990315968,
                               'min': array(-2.2086853981018066),
                               'var': 0.97892254522390565},
                      'number': 59,
                      'phase': {'max': 112.32456525225054,
                                'mean': -1.0111711887628112,
                                'min': -102.4143890204484,
                                'var': 2885.5110793438112},
                      'real': {'max': array(3.3105719089508057),
                               'mean': 1.2845739619253926,
                               'min': array(-0.13342639803886414),
                               'var': 1.0099576839663802},
                      'total': 64},
              'inner': {'amp': {'max': 3.3507643208628997,
                                'mean': 1.1064667538670538,
                                'min': 0.054888048286296932,
                                'var': 0.94882587512210448},
                        'imag': {'max': array(1.4643619060516357),
                                 'mean': 0.056129313275038409,
                                 'min': array(-2.2086853981018066),
                                 'var': 0.55401604820332395},
                        'number': 201,
                        'phase': {'max': 112.32456525225054,
                                  'mean': 17.99353823044413,
                                  'min': -102.4143890204484,
                                  'var': 5065.0038289850836},
                        'real': {'max': array(3.3105719089508057),
                                 'mean': 0.75592079894228781,
                                 'min': array(-0.13342639803886414),
                                 'var': 0.89091406078797231},
                        'total': 51}},
         1: {'all': {'amp': {'max': 1.0603890232940267,
                              'mean': 0.9966154059201201,
                              'min': 0.92736792814263314,
                              'var': 0.032951023002670755},
                      'imag': {'max': array(0.28490364551544189),
                               'mean': 0.10552169008859259,
                               'min': array(-0.08266795426607132),
                               'var': 0.017548692059680165},
                      'number': 59,
                      'phase': {'max': 17.237656150196859,
                                'mean': 6.0660176622725466,
                                'min': -4.9577763029789832,
                                'var': 40.224751029787001},
                      'real': {'max': array(1.047441840171814),
                               'mean': 0.98505325640662234,
                               'min': array(0.90479201078414917),
                               'var': 0.032541082320460629},
                      'total': 64},
              'inner': {'amp': {'max': 1.0603890232940267,
                                'mean': 1.0327309832106102,
                                'min': 0.92837663017787742,
                                'var': 0.19543922221719845},
                        'imag': {'max': array(0.26414120197296143),
                                 'mean': 0.13256895535348001,
                                 'min': array(-0.08266795426607132),
                                 'var': 0.036957882304178208},
                        'number': 201,
                        'phase': {'max': 15.395755389451365,
                                  'mean': 7.3133434743802201,
                                  'min': -4.9577763029789832,
                                  'var': 18.607793373980815},
                        'real': {'max': array(1.047441840171814),
                                 'mean': 1.0216352563400155,
                                 'min': array(0.92740714550018311),
                                 'var': 0.19275019403126262},
                        'total': 51}}}

    NOTE: You can see the problem is in poln 0 of this spw from the amp range and phase.

    """
    # define range for "inner" channels
    if innerbuff>=0.0 and innerbuff<0.5:
        fcrange = [innerbuff,1.0-innerbuff]
    else:
        fcrange = [0.1,0.9]

    # Print extra information here?
    doprintall = False

    # Create the output dictionary
    outDict = {}

    #from taskinit import tbtool
    #mytb = tbtool.create()
    mytb = casac.table()

    import pylab as pl
    
    mytb.open(calTable)

    # Check that this is a B Jones table
    caltype = mytb.getkeyword('VisCal')
    if caltype=='B Jones':
        print 'This is a B Jones table, proceeding'
    else:
        print 'This is NOT a B Jones table, aborting'
        return outDict
    
    antCol = mytb.getcol('ANTENNA1')
    spwCol = mytb.getcol('SPECTRAL_WINDOW_ID')
    fldCol = mytb.getcol('FIELD_ID')

    # these columns are possibly variable in size
    #flagCol = mytb.getcol('FLAG')
    flagVarCol = mytb.getvarcol('FLAG')
    #dataCol = mytb.getcol('CPARAM')
    dataVarCol = mytb.getvarcol('CPARAM')
    mytb.close()

    # get names from ANTENNA table
    mytb.open(calTable+'/ANTENNA')
    antNameCol = mytb.getcol('NAME')
    mytb.close()
    nant = len(antNameCol)

    antDict = {}
    for iant in range(nant):
        antDict[iant] = antNameCol[iant]

    # get names from SPECTRAL_WINDOW table
    mytb.open(calTable+'/SPECTRAL_WINDOW')
    spwNameCol = mytb.getcol('NAME')
    mytb.close()
    nspw = len(spwNameCol)

    # get baseband list
    rxbands = []
    rxBasebandDict = {}
    spwDict = {}
    for ispw in range(nspw):
        try:
            (rx,bb,sb) = spwNameCol[ispw].split('#')
        except:
            rx = 'Unknown'
            bb = 'Unknown'
            sb = ispw
        spwDict[ispw] = {'RX':rx, 'Baseband':bb, 'Subband':sb}
        if rxbands.count(rx)<1:
            rxbands.append(rx)
        if rxBasebandDict.has_key(rx):
            if rxBasebandDict[rx].has_key(bb):
                rxBasebandDict[rx][bb].append(ispw)
            else:
                rxBasebandDict[rx][bb] = [ispw]
        else:
            rxBasebandDict[rx] = {}
            rxBasebandDict[rx][bb] = [ispw]
    
    print 'Found '+str(len(rxbands))+' Rx bands'
    for rx in rxBasebandDict.keys():
        bblist = rxBasebandDict[rx].keys()
        print 'Rx band '+str(rx)+' has basebands: '+str(bblist)

    # Initialize a list to hold the results
    # Get shape of FLAG
    #(np,nc,ni) = flagCol.shape
    # Get shape of CPARAM
    #(np,nc,ni) = dataCol.shape
    rowlist = dataVarCol.keys()
    nrows = len(rowlist)

    # Populate output dictionary structure
    outDict['antspw'] = {}
    outDict['antband'] = {}

    # Our fields will be: running 'n','mean','min','max' for 'amp'
    # Note - running mean(n+1) = ( n*mean(n) + x(n+1) )/(n+1) = mean(n) + (x(n+1)-mean(n))/(n+1)

    # Ok now go through for each row and possibly channel
    ntotal = 0
    ninner = 0
    nflagged = 0
    ngood = 0
    ninnergood = 0
    for rrow in rowlist:
        rown = rrow.strip('r')
        idx = int(rown)-1
        antIdx = antCol[idx]
        spwIdx = spwCol[idx]
        #
        dataArr = dataVarCol[rrow]
        flagArr = flagVarCol[rrow]
        # Get the shape of this data row
        (np,nc,ni) = dataArr.shape
        # ni should be 1 for this
        iid = 0

        # receiver and baseband and subband
        rx = spwDict[spwIdx]['RX']
        bb = spwDict[spwIdx]['Baseband']
        sb = spwDict[spwIdx]['Subband']
        
        # Set up dictionaries if needed
        parts = ['all','inner']
        quants = ['amp','phase','real','imag']
        vals = ['min','max','mean','var']
        if outDict['antspw'].has_key(antIdx):
            if not outDict['antspw'][antIdx].has_key(spwIdx):
                outDict['antspw'][antIdx][spwIdx] = {}
                for poln in range(np):
                    outDict['antspw'][antIdx][spwIdx][poln] = {}
                    for part in parts:
                        outDict['antspw'][antIdx][spwIdx][poln][part] = {}
                        outDict['antspw'][antIdx][spwIdx][poln][part]['total'] = 0
                        outDict['antspw'][antIdx][spwIdx][poln][part]['number'] = 0
                        for quan in quants:
                            outDict['antspw'][antIdx][spwIdx][poln][part][quan] = {}
                            for val in vals:
                                outDict['antspw'][antIdx][spwIdx][poln][part][quan][val] = 0.0
            if outDict['antband'][antIdx].has_key(rx):
                if not outDict['antband'][antIdx][rx].has_key(bb):
                    outDict['antband'][antIdx][rx][bb] = {}
                    for part in parts:
                        outDict['antband'][antIdx][rx][bb][part] = {}
                        outDict['antband'][antIdx][rx][bb][part]['total'] = 0
                        outDict['antband'][antIdx][rx][bb][part]['number'] = 0
                        for quan in quants:
                            outDict['antband'][antIdx][rx][bb][part][quan] = {}
                            for val in vals:
                                outDict['antband'][antIdx][rx][bb][part][quan][val] = 0.0
            else:
                outDict['antband'][antIdx][rx] = {}
                outDict['antband'][antIdx][rx][bb] = {}
                for part in parts:
                    outDict['antband'][antIdx][rx][bb][part] = {}
                    outDict['antband'][antIdx][rx][bb][part]['total'] = 0
                    outDict['antband'][antIdx][rx][bb][part]['number'] = 0
                    for quan in quants:
                        outDict['antband'][antIdx][rx][bb][part][quan] = {}
                        for val in vals:
                            outDict['antband'][antIdx][rx][bb][part][quan][val] = 0.0

        else:
            outDict['antspw'][antIdx] = {}
            outDict['antspw'][antIdx][spwIdx] = {}
            for poln in range(np):
                outDict['antspw'][antIdx][spwIdx][poln] = {}
                for part in parts:
                    outDict['antspw'][antIdx][spwIdx][poln][part] = {}
                    outDict['antspw'][antIdx][spwIdx][poln][part]['total'] = 0
                    outDict['antspw'][antIdx][spwIdx][poln][part]['number'] = 0
                    for quan in quants:
                        outDict['antspw'][antIdx][spwIdx][poln][part][quan] = {}
                        for val in vals:
                            outDict['antspw'][antIdx][spwIdx][poln][part][quan][val] = 0.0

            outDict['antband'][antIdx] = {}
            outDict['antband'][antIdx][rx] = {}
            outDict['antband'][antIdx][rx][bb] = {}
            for part in parts:
                outDict['antband'][antIdx][rx][bb][part] = {}
                outDict['antband'][antIdx][rx][bb][part]['total'] = 0
                outDict['antband'][antIdx][rx][bb][part]['number'] = 0
                for quan in quants:
                    outDict['antband'][antIdx][rx][bb][part][quan] = {}
                    for val in vals:
                        outDict['antband'][antIdx][rx][bb][part][quan][val] = 0.0

        #
        # Sum up the in-row (per pol per chan) flags for this row
        nptotal = 0
        npflagged = 0
        for poln in range(np):
            ntotal += 1
            nptotal += 1
            ncflagged = 0
            ncgood = 0
            ncinnergood = 0
            for chan in range(nc):
                outDict['antspw'][antIdx][spwIdx][poln]['all']['total'] += 1
                outDict['antband'][antIdx][rx][bb]['all']['total'] += 1
                #
                fc = 1
                if nc>0:
                    fc = float(chan)/float(nc)
                if fc>=fcrange[0] and fc<fcrange[1]:
                    outDict['antspw'][antIdx][spwIdx][poln]['inner']['total'] += 1
                    outDict['antband'][antIdx][rx][bb]['inner']['total'] += 1
                #
                if flagArr[poln][chan][iid]:
                    # a flagged data point
                    ncflagged += 1
                else:
                    cx = dataArr[poln][chan][iid]
                    # get quantities from complex data
                    ampx = pl.absolute(cx)
                    phasx = pl.angle(cx,deg=True)
                    realx = pl.real(cx)
                    imagx = pl.imag(cx)
                    #
                    # put in dictionary
                    cdict = {}
                    cdict['amp'] = ampx
                    cdict['phase'] = phasx
                    cdict['real'] = realx
                    cdict['imag'] = imagx
                    # an unflagged data point
                    ncgood += 1
                    # Data stats
                    # By antspw per poln
                    nx = outDict['antspw'][antIdx][spwIdx][poln]['all']['number']
                    if nx==0:
                        outDict['antspw'][antIdx][spwIdx][poln]['all']['number'] = 1
                        for quan in quants:
                            for val in vals:
                                outDict['antspw'][antIdx][spwIdx][poln]['all'][quan][val] = cdict[quan]
                    else:
                        for quan in quants:
                            vx = cdict[quan]
                            if vx>outDict['antspw'][antIdx][spwIdx][poln]['all'][quan]['max']:
                                outDict['antspw'][antIdx][spwIdx][poln]['all'][quan]['max'] = vx
                            if vx<outDict['antspw'][antIdx][spwIdx][poln]['all'][quan]['min']:
                                outDict['antspw'][antIdx][spwIdx][poln]['all'][quan]['min'] = vx
                            # Running mean(n+1) = mean(n) + (x(n+1)-mean(n))/(n+1)
                            meanx = outDict['antspw'][antIdx][spwIdx][poln]['all'][quan]['mean']
                            runx = meanx + (vx - meanx)/float(nx+1)
                            outDict['antspw'][antIdx][spwIdx][poln]['all'][quan]['mean'] = runx
                            # Running var(n+1) = {n*var(n)+[x(n+1)-mean(n)][x(n+1)-mean(n+1)]}/(n+1)
                            varx = outDict['antspw'][antIdx][spwIdx][poln]['all'][quan]['var']
                            qx = nx*varx + (vx-meanx)*(vx-runx)
                            outDict['antspw'][antIdx][spwIdx][poln]['all'][quan]['var'] = qx/float(nx+1)
                    outDict['antspw'][antIdx][spwIdx][poln]['all']['number'] += 1
                    # Now the rx-band stats
                    ny = outDict['antband'][antIdx][rx][bb]['all']['number']
                    if ny==0:
                        outDict['antband'][antIdx][rx][bb]['all']['number'] = 1
                        for quan in quants:
                            for val in vals:
                                outDict['antband'][antIdx][rx][bb]['all'][quan][val] = cdict[quan]
                    else:
                        for quan in quants:
                            vy = cdict[quan]
                            if vy>outDict['antband'][antIdx][rx][bb]['all'][quan]['max']:
                                outDict['antband'][antIdx][rx][bb]['all'][quan]['max'] = vy
                            if vy<outDict['antband'][antIdx][rx][bb]['all'][quan]['min']:
                                outDict['antband'][antIdx][rx][bb]['all'][quan]['min'] = vy
                            # Running mean(n+1) = mean(n) + (x(n+1)-mean(n))/(n+1)
                            meany = outDict['antband'][antIdx][rx][bb]['all'][quan]['mean']
                            runy = meany + (vy - meany)/float(ny+1)
                            outDict['antband'][antIdx][rx][bb]['all'][quan]['mean'] = runy
                            # Running var(n+1) = {n*var(n)+[x(n+1)-mean(n)][x(n+1)-mean(n+1)]}/(n+1)
                            vary = outDict['antband'][antIdx][rx][bb]['all'][quan]['var']
                            qy = ny*vary + (vy-meany)*(vy-runy)
                            outDict['antband'][antIdx][rx][bb]['all'][quan]['var'] = qy/float(ny+1)
                        outDict['antband'][antIdx][rx][bb]['all']['number'] += 1
                    #
                    if fc>=fcrange[0] and fc<fcrange[1]:
                        # this chan lies in the "inner" part of the spw
                        ncinnergood += 1
                        # Data stats
                        # By antspw per poln
                        nx = outDict['antspw'][antIdx][spwIdx][poln]['inner']['number']
                        if nx==0:
                            outDict['antspw'][antIdx][spwIdx][poln]['inner']['number'] = 1
                            for quan in quants:
                                for val in vals:
                                    outDict['antspw'][antIdx][spwIdx][poln]['inner'][quan][val] = cdict[quan]
                        else:
                            for quan in quants:
                                vx = cdict[quan]
                                if vx>outDict['antspw'][antIdx][spwIdx][poln]['inner'][quan]['max']:
                                    outDict['antspw'][antIdx][spwIdx][poln]['inner'][quan]['max'] = vx
                                if vx<outDict['antspw'][antIdx][spwIdx][poln]['inner'][quan]['min']:
                                    outDict['antspw'][antIdx][spwIdx][poln]['inner'][quan]['min'] = vx
                                # Running mean(n+1) = mean(n) + (x(n+1)-mean(n))/(n+1)
                                meanx = outDict['antspw'][antIdx][spwIdx][poln]['inner'][quan]['mean']
                                runx = meanx + (vx - meanx)/float(nx+1)
                                outDict['antspw'][antIdx][spwIdx][poln]['inner'][quan]['mean'] = runx
                                # Running var(n+1) = {n*var(n)+[x(n+1)-mean(n)][x(n+1)-mean(n+1)]}/(n+1)
                                varx = outDict['antspw'][antIdx][spwIdx][poln]['inner'][quan]['var']
                                qx = nx*varx + (vx-meanx)*(vx-runx)
                                outDict['antspw'][antIdx][spwIdx][poln]['inner'][quan]['var'] = qx/float(nx+1)
                                outDict['antspw'][antIdx][spwIdx][poln]['inner']['number'] += 1
                        # Now the rx-band stats
                        ny = outDict['antband'][antIdx][rx][bb]['inner']['number']
                        if ny==0:
                            outDict['antband'][antIdx][rx][bb]['inner']['number'] = 1
                            for quan in quants:
                                for val in vals:
                                    outDict['antband'][antIdx][rx][bb]['inner'][quan][val] = cdict[quan]
                        else:
                            for quan in quants:
                                vy = cdict[quan]
                                if vy>outDict['antband'][antIdx][rx][bb]['inner'][quan]['max']:
                                    outDict['antband'][antIdx][rx][bb]['inner'][quan]['max'] = vy
                                if vy<outDict['antband'][antIdx][rx][bb]['inner'][quan]['min']:
                                    outDict['antband'][antIdx][rx][bb]['inner'][quan]['min'] = vy
                                # Running mean(n+1) = mean(n) + (x(n+1)-mean(n))/(n+1)
                                meany = outDict['antband'][antIdx][rx][bb]['inner'][quan]['mean']
                                runy = meany + (vy - meany)/float(ny+1)
                                outDict['antband'][antIdx][rx][bb]['inner'][quan]['mean'] = runy
                                # Running var(n+1) = {n*var(n)+[x(n+1)-mean(n)][x(n+1)-mean(n+1)]}/(n+1)
                                vary = outDict['antband'][antIdx][rx][bb]['inner'][quan]['var']
                                qy = ny*vary + (vy-meany)*(vy-runy)
                                outDict['antband'][antIdx][rx][bb]['inner'][quan]['var'] = qy/float(ny+1)
                            outDict['antband'][antIdx][rx][bb]['inner']['number'] += 1
                        
                            
            npflagged = float(ncflagged)/float(nc)
            nflagged += float(ncflagged)/float(nc)
            ngood += ncgood
            ninnergood += ncinnergood

    # Assemble rest of dictionary
    outDict['antDict'] = antDict
    outDict['spwDict'] = spwDict
    outDict['rxBasebandDict'] = rxBasebandDict
        
    # Print summary
    print 'Within all channels:'
    print 'Found '+str(ntotal)+' total solutions with '+str(ngood)+' good (unflagged)'
    print 'Within inner '+str(fcrange[1]-fcrange[0])+' of channels:'
    print 'Found '+str(ninner)+' total solutions with '+str(ninnergood)+' good (unflagged)'
    print ''
    print '        AntID      AntName      Rx-band      Baseband    min/max(all)  min/max(inner) ALERT?'
    #
    # Print more?
    if doprintall:
        for ant in outDict['antband'].keys():
            antName = antDict[ant]
            for rx in outDict['antband'][ant].keys():
                for bb in outDict['antband'][ant][rx].keys():
                    xmin = outDict['antband'][ant][rx][bb]['all']['amp']['min']
                    xmax = outDict['antband'][ant][rx][bb]['all']['amp']['max']
                    ymin = outDict['antband'][ant][rx][bb]['inner']['amp']['min']
                    ymax = outDict['antband'][ant][rx][bb]['inner']['amp']['max']
                    if xmax!=0.0:
                        xrat = xmin/xmax
                    else:
                        xrat = -1
                    if ymax!=0.0:
                        yrat = ymin/ymax
                    else:
                        yrat = -1
                    #
                    if yrat<0.05:
                        print ' %12s %12s %12s %12s  %12.4f  %12.4f *** ' % (ant,antName,rx,bb,xrat,yrat)
                    elif yrat<0.1:
                        print ' %12s %12s %12s %12s  %12.4f  %12.4f ** ' % (ant,antName,rx,bb,xrat,yrat)
                    elif yrat<0.2:
                        print ' %12s %12s %12s %12s  %12.4f  %12.4f * ' % (ant,antName,rx,bb,xrat,yrat)
                    else:
                        print ' %12s %12s %12s %12s  %12.4f  %12.4f ' % (ant,antName,rx,bb,xrat,yrat)
    
    return outDict


