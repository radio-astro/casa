from __future__ import absolute_import
import collections
import datetime
import itertools
import operator
import types
from bisect import bisect_left
import pylab
import math
import string

import pipeline.infrastructure.casatools as casatools
import numpy

import pipeline.infrastructure.api as api
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure.jobrequest import casa_tasks
import pipeline.infrastructure.logging as logging



# the logger for this module
LOG = logging.get_logger(__name__)

def buildscans(msfile, scd):
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
    ms = casatools.casac.ms()
    tb = casatools.casac.table()

    # Access the MS
    try:
        ms.open(msfile,nomodify=True)
    except:
        print "ERROR: failed to open ms tool on file "+msfile
        exit(1)

    ####print 'Getting scansummary from MS'
    # get the scan summary using ms.getscansummary method
    #mysc = ms.getscansummary()
    
    #Already open - uncomment to run getscansummary
    #scd = ms.getscansummary()

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
    ####print 'Found '+str(ndd)+' DataDescription IDs'
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
    ####print 'Extracted information for '+str(nspw)+' SpectralWindows'
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
    ####print 'Extracted information for '+str(npols)+' Polarization Setups'
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
    ####print 'Found '+str(nstates)+' StateIds'
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
    ####print 'Found total '+str(ntottimes)+' times'

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
    ####print 'Found '+str(nscans)+' scans min='+str(min(scanlist))+' max='+str(max(scanlist))

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
    ####print 'Size of scandict in memory is '+str(mysize)+' bytes'
    
    return scandict


class VLAUtilsInputs(basetask.StandardInputs):
    def __init__(self, context, vis=None):
        self._init_properties(vars())

class VLAUtils(basetask.StandardTaskTemplate):
    
    # link the accompanying inputs to this task 
    Inputs = VLAUtilsInputs

    def prepare(self):
        return True
    
    def analyse(self, results):
	    return results



    def find_EVLA_band(self, frequency, bandlimits=[0.0e6, 150.0e6, 700.0e6, 2.0e9, 4.0e9, 8.0e9, 12.0e9, 18.0e9, 26.5e9, 40.0e9, 56.0e9], BBAND='?4PLSCXUKAQ?'):
        """identify VLA band"""
        i = bisect_left(bandlimits, frequency)

        return BBAND[i]
        
    def convertspw2band(self):
        """match spw number to EVLA band identifier
        """
        
        spw2band = {}
        
        for spw in self.scandict['DataDescription']:
	    strelems =  list(self.scandict['DataDescription'][spw]['spwname'])
            bandname = strelems[5]
            if bandname in '4PLSCXUKAQ':
                spw2band[spw] = strelems[5]        
        return spw2band

        
    def spwsforfield(self,field):
        # get observed DDIDs for specified field from MAIN
        with casatools.TableReader(self.inputs.vis) as table:
            st = table.query('FIELD_ID=='+str(field))
            ddids=pylab.unique(st.getcol('DATA_DESC_ID'))
            st.close()
    
        # get SPW_IDs corresponding to those DDIDs
        with casatools.TableReader(self.inputs.vis+'/DATA_DESCRIPTION') as table:
            spws=table.getcol('SPECTRAL_WINDOW_ID')[ddids]
        
        # return as a list
        return list(spws)

    def identifyspw(self):
        """Indentify spw information"""
        
        casatools.ms.open(self.inputs.vis)
        self.scan_summary = casatools.ms.getscansummary()
        self.ms_summary = casatools.ms.summary()
        casatools.ms.close()
        
        self.scandict = buildscans(self.inputs.vis, self.scan_summary)
        
        #Create dictionary of band names from spw ids
        self.spw2band = self.convertspw2band()
        
        casatools.table.open(self.inputs.vis+'/SPECTRAL_WINDOW')
        channels = casatools.table.getcol('NUM_CHAN')
        
        self.channels = channels
        
        self.originalBBClist = casatools.table.getcol('BBC_NO')
        self.spw_bandwidths=casatools.table.getcol('TOTAL_BANDWIDTH')
        self.reference_frequencies = casatools.table.getcol('REF_FREQUENCY')
        casatools.table.close()
        
        center_frequencies = map(lambda rf, spwbw: rf + spwbw/2,self.reference_frequencies, self.spw_bandwidths)
        
        bands = self.spw2band.values()
        
        print bands
        
        if (bands == []):
            bands = map(self.find_EVLA_band,center_frequencies)

        print bands        
        
        unique_bands = list(numpy.unique(bands))
        
        numSpws = len(channels)
        
        self.numSpws = numSpws
        
        
        
        return True


    def initialgainspw(self):
        """Find information on spws for initial gain solutions,
        """
        
        tst_delay_spw=''
        all_spw=''

        for ispw in range(self.numSpws):
            endch1=int(self.channels[ispw]/3.0)
            endch2=int(2.0*self.channels[ispw]/3.0)+1
            if (ispw<max(range(self.numSpws))):
                tst_delay_spw=tst_delay_spw+str(ispw)+':'+str(endch1)+'~'+str(endch2)+','
                all_spw=all_spw+str(ispw)+','
            else:
                tst_delay_spw=tst_delay_spw+str(ispw)+':'+str(endch1)+'~'+str(endch2)
                all_spw=all_spw+str(ispw)
        
        tst_bpass_spw=tst_delay_spw
        
        self.tst_bpass_spw = tst_bpass_spw
        self.tst_delay_spw = tst_delay_spw
        self.all_spw = all_spw
        
        return True
            
    def getFields(self):
        """Identify number of fields, positions, and source IDs
        """
        
        casatools.table.open(self.inputs.vis+'/FIELD')
        self.numFields = casatools.table.nrows()
        self.field_positions = casatools.table.getcol('PHASE_DIR')
        self.field_ids=range(self.numFields)
        self.field_names=casatools.table.getcol('NAME')
        casatools.table.close()
        
        #Map field IDs to spws
        self.field_spws = []
        for ii in range(self.numFields):
            self.field_spws.append(self.spwsforfield(ii))
        
        return True
        
        

    def integrationTime(self):
        # Identify scan numbers, map scans to field ID, and run scan summary
        # (needed for figuring out integration time later)
        
        casatools.table.open(self.inputs.vis)
        self.scanNums = sorted(numpy.unique(casatools.table.getcol('SCAN_NUMBER')))
        self.field_scans = []
        for ii in range(0,self.numFields):
            subtable = casatools.table.query('FIELD_ID==%s'%ii)
            self.field_scans.append(list(numpy.unique(subtable.getcol('SCAN_NUMBER'))))
        casatools.table.close()
        
        
        ## field_scans is now a list of lists containing the scans for each field.
        ## so, to access all the scans for the fields, you'd:
        #
        #for ii in range(0,len(field_scans)):
        #   for jj in range(0,len(field_scans[ii]))
        #
        ## the jj'th scan of the ii'th field is in field_scans[ii][jj]
        
        # Identify intents
        
        casatools.table.open(self.inputs.vis+'/STATE')
        self.intents=casatools.table.getcol('OBS_MODE')
        casatools.table.close()
        
        
        """Figure out integration time used"""
        
        
        self.startdate=float(self.ms_summary['BeginTime'])
    
        integ_scan_list = []
        for scan in self.scan_summary:
            integ_scan_list.append(int(scan))
        self.sorted_scan_list = sorted(integ_scan_list)
        
        # find max and median integration times
        #
        integration_times = []
        for ii in self.sorted_scan_list:
            integration_times.append(self.scan_summary[str(ii)]['0']['IntegrationTime'])
            
        maximum_integration_time = max(integration_times)
        median_integration_time = numpy.median(integration_times)
        
        int_time = maximum_integration_time
        
        self.int_time = int_time
        
        #This will be needed later in the pipeline
        self.gain_solint1 = 'int'
        self.gain_solint2 = 'int'
        self.shortsol1 = 0.0
        self.shortsol2 = 0.0
        self.longsolint = 0.0
        self.short_solint = 0.0
        self.new_gain_solint1 = '1.0s'
        
        self.fluxscale_sources = []
        self.fluxscale_flux_densities = []
        self.fluxscale_spws = []
        
        self.flagspw1  = ''
        self.flagspw1b = ''
        self.flagspw2  = ''
        
        return int_time
        
    def quackingScans(self):
        """ Find scans for quacking
        """
        
        scan_list = [1]
        old_scan = self.scan_summary[str(self.sorted_scan_list[0])]['0']
        old_field = old_scan['FieldId']
        old_spws = old_scan['SpwIds']
        for ii in range(1,len(self.sorted_scan_list)):
            new_scan = self.scan_summary[str(self.sorted_scan_list[ii])]['0']
            new_field = new_scan['FieldId']
            new_spws = new_scan['SpwIds']
            if ((new_field != old_field) or (set(new_spws) != set(old_spws))):
                scan_list.append(self.sorted_scan_list[ii])
                old_field = new_field
                old_spws = new_spws
        self.quack_scan_string = ','.join(["%s" % ii for ii in scan_list])
        
        return True
            
    def basebandspws(self):
        """ For 1 GHz wide basebands, figure out which spws are associated
         with the edges of the baseband filters
         """
         
        self.sorted_frequencies = sorted(self.reference_frequencies)
        self.sorted_indices = []
        
        for ii in range (0,len(self.sorted_frequencies)):
            for jj in range (0,len(self.reference_frequencies)):
                if (self.sorted_frequencies[ii] == self.reference_frequencies[jj]):
                    self.sorted_indices.append(jj)
        
        self.spwList = []
        self.BBC_bandwidths = []
        ii = 0
        
        while (ii < len(self.sorted_frequencies)):
            self.upper_frequency = self.sorted_frequencies[ii] + self.spw_bandwidths[self.sorted_indices[ii]]
            self.BBC_bandwidth = self.spw_bandwidths[self.sorted_indices[ii]]
            self.thisSpwList = [self.sorted_indices[ii]]
            jj = ii + 1
            while (jj < len(self.sorted_frequencies)):
                self.lower_frequency = self.sorted_frequencies[jj]
                if ((math.fabs(self.lower_frequency - self.upper_frequency) < 1.0) and \
                    (self.originalBBClist[self.sorted_indices[ii]] == self.originalBBClist[self.sorted_indices[jj]])):
                    self.thisSpwList.append(self.sorted_indices[jj])
                    self.upper_frequency += self.spw_bandwidths[self.sorted_indices[jj]]
                    self.BBC_bandwidth += self.spw_bandwidths[self.sorted_indices[jj]]
                    jj += 1
                    ii += 1
                else:
                    jj = len(self.sorted_frequencies)
            self.spwList.append(self.thisSpwList)
            self.BBC_bandwidths.append(self.BBC_bandwidth)
            ii += 1        
        
        self.low_spws = []
        self.high_spws = []

        for ii in range(0,len(self.BBC_bandwidths)):
            if (self.BBC_bandwidths[ii] > 1.0e9):
                self.low_spws.append(self.spwList[ii][0])
                self.high_spws.append(self.spwList[ii][len(self.spwList[ii])-1])
        
        return True
            
            
            
##############################################################################
##############################################################################            

    def calibratorIntents(self):
        """
            # Identify scans and fields associated with different calibrator intents

            # NB: the scan intent definitions changed in the OPT on Feb 21,
            # 2012.  So test on date:
        """

        self.bandpass_state_IDs = []
        self.delay_state_IDs = []
        self.flux_state_IDs = []
        self.polarization_state_IDs = []
        self.phase_state_IDs = []
        self.amp_state_IDs = []
        self.calibrator_state_IDs = []
        self.pointing_state_IDs = []

        if self.startdate <= 55978.50:
            for state_ID in range(0,len(self.intents)):
                self.state_intents = self.intents[state_ID].rsplit(',')
                for intent in range(0,len(self.state_intents)):
                    self.scan_intent = self.state_intents[intent].rsplit('#')[0]
                    self.subscan_intent = self.state_intents[intent].rsplit('#')[1]
                    if (self.scan_intent == 'CALIBRATE_BANDPASS'):
                        self.bandpass_state_IDs.append(state_ID)
                        self.calibrator_state_IDs.append(state_ID)
                    elif (self.scan_intent == 'CALIBRATE_DELAY'):
                        self.delay_state_IDs.append(state_ID)
                        self.calibrator_state_IDs.append(state_ID)
                    elif (self.scan_intent == 'CALIBRATE_AMPLI'):
                        self.flux_state_IDs.append(state_ID)
                        self.calibrator_state_IDs.append(state_ID)
                    elif (self.scan_intent == 'CALIBRATE_POLARIZATION'):
                        self.polarization_state_IDs.append(state_ID)
                        self.calibrator_state_IDs.append(state_ID)
                    elif (self.scan_intent == 'CALIBRATE_PHASE'):
                        self.phase_state_IDs.append(state_ID)
                        self.calibrator_state_IDs.append(state_ID)
                    elif (self.scan_intent == 'CALIBRATE_POINTING'):
                        self.pointing_state_IDs.append(state_ID)
                        self.calibrator_state_IDs.append(state_ID)
        
            casatools.table.open(self.inputs.vis)
        
            if (len(self.flux_state_IDs) == 0):
                #QA2_msinfo='Fail'
                #logprint("ERROR: No flux density calibration scans found", logfileout='logs/msinfo.log')
                raise Exception("No flux density calibration scans found")
            else:
                self.flux_state_select_string = ('STATE_ID in [%s'%self.flux_state_IDs[0])
                for state_ID in range(1,len(self.flux_state_IDs)):
                    self.flux_state_select_string += (',%s')%self.flux_state_IDs[state_ID]
                self.flux_state_select_string += ']'
                subtable = casatools.table.query(self.flux_state_select_string)
                self.flux_scan_list = list(numpy.unique(subtable.getcol('SCAN_NUMBER')))
                self.flux_scan_select_string = ','.join(["%s" % ii for ii in self.flux_scan_list])
                #logprint ("Flux density calibrator(s) scans are "+flux_scan_select_string, logfileout='logs/msinfo.log')
                self.flux_field_list = list(numpy.unique(subtable.getcol('FIELD_ID')))
                self.flux_field_select_string = ','.join(["%s" % ii for ii in self.flux_field_list])
                #logprint ("Flux density calibrator(s) are fields "+flux_field_select_string, logfileout='logs/msinfo.log')
        
            if (len(self.bandpass_state_IDs) == 0):
                logprint ("No bandpass calibration scans defined, using flux density calibrator(s)")
                self.bandpass_scan_select_string=self.flux_scan_select_string
                #logprint ("Bandpass calibrator(s) scans are "+bandpass_scan_select_string, logfileout='logs/msinfo.log')
                self.bandpass_field_select_string=self.flux_field_select_string
                #logprint ("Bandpass calibrator(s) are fields "+bandpass_field_select_string, logfileout='logs/msinfo.log')
            else:
                self.bandpass_state_select_string = ('STATE_ID in [%s'%self.bandpass_state_IDs[0])
                for state_ID in range(1,len(self.bandpass_state_IDs)):
                    self.bandpass_state_select_string += (',%s')%self.bandpass_state_IDs[state_ID]
                self.bandpass_state_select_string += ']'
                subtable = casatools.table.query(self.bandpass_state_select_string)
                self.bandpass_scan_list = list(numpy.unique(subtable.getcol('SCAN_NUMBER')))
                self.bandpass_scan_select_string = ','.join(["%s" % ii for ii in self.bandpass_scan_list])
                #logprint ("Bandpass calibrator(s) scans are "+bandpass_scan_select_string, logfileout='logs/msinfo.log')
                self.bandpass_field_list = list(numpy.unique(subtable.getcol('FIELD_ID')))
                self.bandpass_field_select_string = ','.join(["%s" % ii for ii in self.bandpass_field_list])
                #logprint ("Bandpass calibrator(s) are fields "+bandpass_field_select_string, logfileout='logs/msinfo.log')
                if (len(self.bandpass_field_list) > 1):
                    #logprint ("WARNING: More than one field is defined as the bandpass calibrator.", logfileout='logs/msinfo.log')
                    #logprint ("WARNING: Models are required for all BP calibrators if multiple fields", logfileout='logs/msinfo.log')
                    #logprint ("WARNING: are to be used, not yet implemented; the pipeline will use", logfileout='logs/msinfo.log')
                    #logprint ("WARNING: only the first field.", logfileout='logs/msinfo.log')
                    self.bandpass_field_select_string = str(self.bandpass_field_list[0])
        
            if (len(self.delay_state_IDs) == 0):
                #logprint ("No delay calibration scans defined, using bandpass calibrator")
                self.delay_scan_select_string=self.bandpass_scan_select_string
                #logprint ("Delay calibrator(s) scans are "+delay_scan_select_string, logfileout='logs/msinfo.log')
                self.delay_field_select_string=self.bandpass_field_select_string
                #logprint ("Delay calibrator(s) are fields "+delay_field_select_string, logfileout='logs/msinfo.log')
            else:
                self.delay_state_select_string = ('STATE_ID in [%s'%self.delay_state_IDs[0])
                for state_ID in range(1,len(self.delay_state_IDs)):
                    self.delay_state_select_string += (',%s')%self.delay_state_IDs[state_ID]
                self.delay_state_select_string += ']'
                subtable = casatools.table.query(self.delay_state_select_string)
                self.delay_scan_list = list(numpy.unique(subtable.getcol('SCAN_NUMBER')))
                self.delay_scan_select_string = ','.join(["%s" % ii for ii in self.delay_scan_list])
                #logprint ("Delay calibrator(s) scans are "+delay_scan_select_string, logfileout='logs/msinfo.log')
                self.delay_field_list = list(numpy.unique(subtable.getcol('FIELD_ID')))
                self.delay_field_select_string = ','.join(["%s" % ii for ii in self.delay_field_list])
                #logprint ("Delay calibrator(s) are fields "+delay_field_select_string, logfileout='logs/msinfo.log')
                if (len(self.delay_field_list) > 1):
                    #logprint ("WARNING: More than one field is defined as the delay calibrator.", logfileout='logs/msinfo.log')
                    #logprint ("WARNING: Models are required for all delay calibrators if multiple fields", logfileout='logs/msinfo.log')
                    #logprint ("WARNING: are to be used, not yet implemented; the pipeline will use", logfileout='logs/msinfo.log')
                    #logprint ("WARNING: only the first field.", logfileout='logs/msinfo.log')
                    self.delay_field_select_string = str(self.delay_field_list[0])
        
            if (len(self.polarization_state_IDs) == 0):
                #logprint ("No polarization calibration scans defined, no polarization calibration possible", logfileout='logs/msinfo.log')
                self.polarization_scan_select_string=''
                self.polarization_field_select_string=''
            else:
                #logprint ("Warning: polarization calibration scans found, but polarization calibration not yet implemented", logfileout='logs/msinfo.log')
                self.polarization_state_select_string = ('STATE_ID in [%s'%self.polarization_state_IDs[0])
                for state_ID in range(1,len(self.polarization_state_IDs)):
                    self.polarization_state_select_string += (',%s')%self.polarization_state_IDs[state_ID]
                self.polarization_state_select_string += ']'
                subtable = casatools.table.query(self.polarization_state_select_string)
                self.polarization_scan_list = list(numpy.unique(subtable.getcol('SCAN_NUMBER')))
                self.polarization_scan_select_string = ','.join(["%s" % ii for ii in self.polarization_scan_list])
                #logprint ("Polarization calibrator(s) scans are "+polarization_scan_select_string, logfileout='logs/msinfo.log')
                self.polarization_field_list = list(numpy.unique(subtable.getcol('FIELD_ID')))
                self.polarization_field_select_string = ','.join(["%s" % ii for ii in self.polarization_field_list])
                #logprint ("Polarization calibrator(s) are fields "+polarization_field_select_string, logfileout='logs/msinfo.log'
        
            if (len(self.phase_state_IDs) == 0):
                #QA2_msinfo='Fail'
                #logprint("ERROR: No gain calibration scans found", logfileout='logs/msinfo.log')
                raise Exception("No gain calibration scans found")
            else:
                self.phase_state_select_string = ('STATE_ID in [%s'%self.phase_state_IDs[0])
                for state_ID in range(1,len(self.phase_state_IDs)):
                    self.phase_state_select_string += (',%s')%self.phase_state_IDs[state_ID]
                self.phase_state_select_string += ']'
                subtable = casatools.table.query(self.phase_state_select_string)
                self.phase_scan_list = list(numpy.unique(subtable.getcol('SCAN_NUMBER')))
                self.phase_scan_select_string = ','.join(["%s" % ii for ii in self.phase_scan_list])
                #logprint ("Phase calibrator(s) scans are "+phase_scan_select_string, logfileout='logs/msinfo.log')
                self.phase_field_list = list(numpy.unique(subtable.getcol('FIELD_ID')))
                self.phase_field_select_string = ','.join(["%s" % ii for ii in self.phase_field_list])
                #logprint ("Phase calibrator(s) are fields "+phase_field_select_string, logfileout='logs/msinfo.log')
        
        # Find all calibrator scans and fields
        
            self.calibrator_state_select_string = ('STATE_ID in [%s'%self.calibrator_state_IDs[0])
            for state_ID in range(1,len(self.calibrator_state_IDs)):
                self.calibrator_state_select_string += (',%s')%self.calibrator_state_IDs[state_ID]
        
            self.calibrator_state_select_string += ']' 
            subtable = casatools.table.query(self.calibrator_state_select_string)
            self.calibrator_scan_list = list(numpy.unique(subtable.getcol('SCAN_NUMBER')))
            self.calibrator_scan_select_string = ','.join(["%s" % ii for ii in self.calibrator_scan_list])
            self.calibrator_field_list = list(numpy.unique(subtable.getcol('FIELD_ID')))
            self.calibrator_field_select_string = ','.join(["%s" % ii for ii in self.calibrator_field_list])
        
            casatools.table.close()
            
        else:
            for state_ID in range(0,len(self.intents)):
                self.state_intents = self.intents[state_ID].rsplit(',')
                for intent in range(0,len(self.state_intents)):
                    self.scan_intent = self.state_intents[intent].rsplit('#')[0]
                    self.subscan_intent = self.state_intents[intent].rsplit('#')[1]
                    if (self.scan_intent == 'CALIBRATE_BANDPASS'):
                        self.bandpass_state_IDs.append(state_ID)
                        self.calibrator_state_IDs.append(state_ID)
                    elif (self.scan_intent == 'CALIBRATE_DELAY'):
                        self.delay_state_IDs.append(state_ID)
                        self.calibrator_state_IDs.append(state_ID)
                    elif (self.scan_intent == 'CALIBRATE_FLUX'):
                        self.flux_state_IDs.append(state_ID)
                        self.calibrator_state_IDs.append(state_ID)
                    elif (self.scan_intent == 'CALIBRATE_POLARIZATION'):
                        self.polarization_state_IDs.append(state_ID)
                        self.calibrator_state_IDs.append(state_ID)
                    elif (self.scan_intent == 'CALIBRATE_AMPLI'):
                        self.amp_state_IDs.append(state_ID)
                        self.calibrator_state_IDs.append(state_ID)
                    elif (self.scan_intent == 'CALIBRATE_PHASE'):
                        self.phase_state_IDs.append(state_ID)
                        self.calibrator_state_IDs.append(state_ID)
                    elif (self.scan_intent == 'CALIBRATE_POINTING'):
                        self.pointing_state_IDs.append(state_ID)
                        self.calibrator_state_IDs.append(state_ID)   
            
            casatools.table.open(self.inputs.vis)
            
            if (len(self.flux_state_IDs) == 0):
                #QA2_msinfo='Fail'
                #logprint("ERROR: No flux density calibration scans found", logfileout='logs/msinfo.log')
                raise Exception("No flux density calibration scans found")
            else:
                self.flux_state_select_string = ('STATE_ID in [%s'%self.flux_state_IDs[0])
                for state_ID in range(1,len(self.flux_state_IDs)):
                    self.flux_state_select_string += (',%s')%self.flux_state_IDs[state_ID]
                self.flux_state_select_string += ']'
                subtable = casatools.table.query(self.flux_state_select_string)
                self.flux_scan_list = list(numpy.unique(subtable.getcol('SCAN_NUMBER')))
                self.flux_scan_select_string = ','.join(["%s" % ii for ii in self.flux_scan_list])
                #logprint ("Flux density calibrator(s) scans are "+flux_scan_select_string, logfileout='logs/msinfo.log')
                self.flux_field_list = list(numpy.unique(subtable.getcol('FIELD_ID')))
                self.flux_field_select_string = ','.join(["%s" % ii for ii in self.flux_field_list])
                #logprint ("Flux density calibrator(s) are fields "+flux_field_select_string, logfileout='logs/msinfo.log')
 
 
            if (len(self.bandpass_state_IDs) == 0):
                #logprint ("No bandpass calibration scans defined, using flux density calibrator", logfileout='logs/msinfo.log')
                self.bandpass_scan_select_string=self.flux_scan_select_string
                #logprint ("Bandpass calibrator(s) scans are "+bandpass_scan_select_string, logfileout='logs/msinfo.log')
                self.bandpass_field_select_string=self.flux_field_select_string
                #logprint ("Bandpass calibrator(s) are fields "+bandpass_field_select_string, logfileout='logs/msinfo.log')
            else:
                self.bandpass_state_select_string = ('STATE_ID in [%s'%self.bandpass_state_IDs[0])
                for state_ID in range(1,len(self.bandpass_state_IDs)):
                    self.bandpass_state_select_string += (',%s')%self.bandpass_state_IDs[state_ID]
                self.bandpass_state_select_string += ']'
                subtable = casatools.table.query(self.bandpass_state_select_string)
                self.bandpass_scan_list = list(numpy.unique(subtable.getcol('SCAN_NUMBER')))
                self.bandpass_scan_select_string = ','.join(["%s" % ii for ii in self.bandpass_scan_list])
                #logprint ("Bandpass calibrator(s) scans are "+bandpass_scan_select_string, logfileout='logs/msinfo.log')
                self.bandpass_field_list = list(numpy.unique(subtable.getcol('FIELD_ID')))
                self.bandpass_field_select_string = ','.join(["%s" % ii for ii in self.bandpass_field_list])
                #logprint ("Bandpass calibrator(s) are fields "+bandpass_field_select_string, logfileout='logs/msinfo.log')
                if (len(self.bandpass_field_list) > 1):
                    #logprint ("WARNING: More than one field is defined as the bandpass calibrator.", logfileout='logs/msinfo.log')
                    #logprint ("WARNING: Models are required for all BP calibrators if multiple fields", logfileout='logs/msinfo.log')
                    #logprint ("WARNING: are to be used, not yet implemented; the pipeline will use", logfileout='logs/msinfo.log')
                    #logprint ("WARNING: only the first field.", logfileout='logs/msinfo.log')
                    self.bandpass_field_select_string = str(self.bandpass_field_list[0])
        
            if (len(self.delay_state_IDs) == 0):
                #logprint ("No delay calibration scans defined, using bandpass calibrator", logfileout='logs/msinfo.log')
                self.delay_scan_select_string=self.bandpass_scan_select_string
                #logprint ("Delay calibrator(s) scans are "+delay_scan_select_string, logfileout='logs/msinfo.log')
                self.delay_field_select_string=self.bandpass_field_select_string
                #logprint ("Delay calibrator(s) are fields "+delay_field_select_string, logfileout='logs/msinfo.log')
            else:
                self.delay_state_select_string = ('STATE_ID in [%s'%self.delay_state_IDs[0])
                for state_ID in range(1,len(self.delay_state_IDs)):
                    self.delay_state_select_string += (',%s')%self.delay_state_IDs[state_ID]
                self.delay_state_select_string += ']'
                subtable = casatools.table.query(self.delay_state_select_string)
                self.delay_scan_list = list(numpy.unique(subtable.getcol('SCAN_NUMBER')))
                self.delay_scan_select_string = ','.join(["%s" % ii for ii in self.delay_scan_list])
                #logprint ("Delay calibrator(s) scans are "+delay_scan_select_string, logfileout='logs/msinfo.log')
                self.delay_field_list = list(numpy.unique(subtable.getcol('FIELD_ID')))
                self.delay_field_select_string = ','.join(["%s" % ii for ii in self.delay_field_list])
                #logprint ("Delay calibrator(s) are fields "+delay_field_select_string, logfileout='logs/msinfo.log')
            
            if (len(self.polarization_state_IDs) == 0):
                #logprint ("No polarization calibration scans defined, no polarization calibration possible", logfileout='logs/msinfo.log')
                self.polarization_scan_select_string=''
                self.polarization_field_select_string=''
            else:
                #logprint ("Warning: polarization calibration scans found, but polarization calibration not yet implemented", logfileout='logs/msinfo.log')
                self.polarization_state_select_string = ('STATE_ID in [%s'%self.polarization_state_IDs[0])
                for state_ID in range(1,len(self.polarization_state_IDs)):
                    self.polarization_state_select_string += (',%s')%self.polarization_state_IDs[state_ID]
                self.polarization_state_select_string += ']'
                subtable = casatools.table.query(self.polarization_state_select_string)
                self.polarization_scan_list = list(numpy.unique(subtable.getcol('SCAN_NUMBER')))
                self.polarization_scan_select_string = ','.join(["%s" % ii for ii in self.polarization_scan_list])
                #logprint ("Polarization calibrator(s) scans are "+polarization_scan_select_string, logfileout='logs/msinfo.log')
                self.polarization_field_list = list(numpy.unique(subtable.getcol('FIELD_ID')))
                self.polarization_field_select_string = ','.join(["%s" % ii for ii in self.polarization_field_list])
                #logprint ("Polarization calibrator(s) are fields "+polarization_field_select_string, logfileout='logs/msinfo.log')
            
            if (len(self.phase_state_IDs) == 0):
                #QA2_msinfo='Fail'
                #logprint("ERROR: No gain calibration scans found", logfileout='logs/msinfo.log')
                raise Exception("No gain calibration scans found")
            else:
                self.phase_state_select_string = ('STATE_ID in [%s'%self.phase_state_IDs[0])
                for state_ID in range(1,len(self.phase_state_IDs)):
                    self.phase_state_select_string += (',%s')%self.phase_state_IDs[state_ID]
                self.phase_state_select_string += ']'
                subtable = casatools.table.query(self.phase_state_select_string)
                self.phase_scan_list = list(numpy.unique(subtable.getcol('SCAN_NUMBER')))
                self.phase_scan_select_string = ','.join(["%s" % ii for ii in self.phase_scan_list])
                #logprint ("Phase calibrator(s) scans are "+phase_scan_select_string, logfileout='logs/msinfo.log')
                self.phase_field_list = list(numpy.unique(subtable.getcol('FIELD_ID')))
                self.phase_field_select_string = ','.join(["%s" % ii for ii in self.phase_field_list])
                #logprint ("Phase calibrator(s) are fields "+phase_field_select_string, logfileout='logs/msinfo.log')
            
            if (len(self.amp_state_IDs) == 0):
                #logprint ("No amplitude calibration scans defined, will use phase calibrator", logfileout='logs/msinfo.log')
                self.amp_scan_select_string=self.phase_scan_select_string
                #logprint ("Amplitude calibrator(s) scans are "+amp_scan_select_string, logfileout='logs/msinfo.log')
                self.amp_field_select_string=self.phase_scan_select_string
                #logprint ("Amplitude calibrator(s) are fields "+amp_field_select_string, logfileout='logs/msinfo.log')
            else:
                self.amp_state_select_string = ('STATE_ID in [%s'%self.amp_state_IDs[0])
                for state_ID in range(1,len(self.amp_state_IDs)):
                    self.amp_state_select_string += (',%s')%self.amp_state_IDs[state_ID]
                self.amp_state_select_string += ']'
                subtable = casatools.table.query(self.amp_state_select_string)
                self.amp_scan_list = list(numpy.unique(subtable.getcol('SCAN_NUMBER')))
                self.amp_scan_select_string = ','.join(["%s" % ii for ii in self.amp_scan_list])
                #logprint ("Amplitude calibrator(s) scans are "+amp_scan_select_string, logfileout='logs/msinfo.log')
                self.amp_field_list = list(numpy.unique(subtable.getcol('FIELD_ID')))
                self.amp_field_select_string = ','.join(["%s" % ii for ii in self.amp_field_list])
                #logprint ("Amplitude calibrator(s) are fields "+amp_field_select_string, logfileout='logs/msinfo.log')
            
            # Find all calibrator scans and fields
            
            self.calibrator_state_select_string = ('STATE_ID in [%s'%self.calibrator_state_IDs[0])
            for state_ID in range(1,len(self.calibrator_state_IDs)):
                self.calibrator_state_select_string += (',%s')%self.calibrator_state_IDs[state_ID]
            
            self.calibrator_state_select_string += ']' 
            subtable = casatools.table.query(self.calibrator_state_select_string)
            self.calibrator_scan_list = list(numpy.unique(subtable.getcol('SCAN_NUMBER')))
            self.calibrator_scan_select_string = ','.join(["%s" % ii for ii in self.calibrator_scan_list])
            self.calibrator_field_list = list(numpy.unique(subtable.getcol('FIELD_ID')))
            self.calibrator_field_select_string = ','.join(["%s" % ii for ii in self.calibrator_field_list])
            
            casatools.table.close()
            
        #If there are any pointing state IDs, subtract 2 from the number of
        #science spws -- needed for QA scores
        if (len(self.pointing_state_IDs)>0):
            self.numSpws2 = self.numSpws - 2
        else:
            self.numSpws2 = self.numSpws
        
        if (self.delay_scan_select_string == self.bandpass_scan_select_string):
            self.testgainscans=self.bandpass_scan_select_string
        else:
            self.testgainscans=self.bandpass_scan_select_string+','+self.delay_scan_select_string

        
        
        
        self.checkflagfields=''
        if (self.bandpass_field_select_string == self.delay_field_select_string):
            self.checkflagfields = self.bandpass_field_select_string
        else:
            self.checkflagfields = (self.bandpass_field_select_string+','+ self.delay_field_select_string)
        
        
        
        
        
        return True
            
    def corrStrings(self):
        """
        Prep string listing of correlations from dictionary created by method buildscans
        For now, only use the parallel hands.  Cross hands will be implemented later.
        """
        
        self.corrstring_list = self.scandict['DataDescription'][0]['corrdesc']
        self.removal_list = ['RL', 'LR', 'XY', 'YX']
        self.corrstring_list = list(set(self.corrstring_list).difference(set(self.removal_list)))
        self.corrstring = string.join(self.corrstring_list,',')
        #logprint ("Correlations shown in plotms will be "+corrstring, logfileout='logs/msinfo.log')
    
        return True
    
    def getAntennas(self):
        """
        Get number of antennas, store in self.numAntenna
        """
        tbLoc = casatools.casac.table()
        tbLoc.open( '%s/ANTENNA' % self.inputs.vis)
        self.nameAntenna = tbLoc.getcol( 'NAME' )
        self.numAntenna = len(self.nameAntenna)
        tbLoc.close()
        
        self.minBL_for_cal=max(3,int(self.numAntenna/2.0))
        
        return True
    
    def find_3C84(self, positions):
        MAX_SEPARATION = 60*2.0e-5
        position_3C84 = casatools.measures.direction('j2000', '3h19m48.160', '41d30m42.106')
        fields_3C84 = []
    
        for ii in range(0,len(positions)):
            position = casatools.measures.direction('j2000', str(positions[ii][0])+'rad', str(positions[ii][1])+'rad')
            separation = casatools.measures.separation(position,position_3C84)['value'] * math.pi/180.0
            if (separation < MAX_SEPARATION):
                    fields_3C84.append(ii)
    
        return fields_3C84
        
        
    def determine3C84(self):
        """
        #Determine if 3C84 was used as a bandpass or delay calibrator
        """
        
        self.positions = []

        for ii in range(0,len(self.field_positions[0][0])):
            self.positions.append([self.field_positions[0][0][ii], self.field_positions[1][0][ii]])
            
        self.fields_3C84 = self.find_3C84(self.positions)
        
        self.cal3C84_d = False
        self.cal3C84_bp = False
        #uvrange3C84 = '0~1800klambda'
        self.uvrange3C84 = ''
        
        if self.fields_3C84 != []:
            for field_int in self.fields_3C84:
                if (str(field_int) in self.delay_field_select_string):
                    self.cal3C84_d = True
                    #logprint("WARNING: 3C84 was observed as a delay calibrator, uvrange limits may be used", logfileout='logs/msinfo.log')
                if (str(field_int) in self.bandpass_field_select_string):
                    self.cal3C84_bp = True
                    #logprint("WARNING: 3C84 was observed as a BP calibrator, uvrange limits may be used", logfileout='logs/msinfo.log')

        if ((self.cal3C84_d == True) or (self.cal3C84_bp == True)):
            self.cal3C84=True
        else:
            self.cal3C84=False

        return True
                
    
    def identifySubbands(self):
        """ Identify bands/basebands/spws
        """
        
        casatools.table.open(self.inputs.vis+'/SPECTRAL_WINDOW')
        self.spw_names = casatools.table.getcol('NAME')
        casatools.table.close()
        
        # If the dataset is too old to have the bandname in it, assume that
        # either there are 8 spws per baseband (and allow for one or two for
        # pointing), or that this is a dataset with one spw per baseband

        if (len(self.spw_names)>=8):
            self.critfrac=0.9/int(len(self.spw_names)/8.0)
        else:
            self.critfrac=0.9/float(len(self.spw_names))
        
        
        if '#' in self.spw_names[0]:
        #
        # i assume that if any of the spw_names have '#', they all do...
        #
            self.bands_basebands_subbands = []
            for spw_name in self.spw_names:
                self.receiver_name, self.baseband, self.subband = spw_name.split('#')
                self.receiver_band = (self.receiver_name.split('_'))[1]
                self.bands_basebands_subbands.append([self.receiver_band, self.baseband, int(self.subband)])
            self.spws_info = [[self.bands_basebands_subbands[0][0], self.bands_basebands_subbands[0][1], [], []]]
            self.bands = [self.bands_basebands_subbands[0][0]]
            for ii in range(len(self.bands_basebands_subbands)):
                self.band,self.baseband,self.subband = self.bands_basebands_subbands[ii]
                found = -1
                for jj in range(len(self.spws_info)):
                    self.oband,self.obaseband,self.osubband,self.ospw_list = self.spws_info[jj]
                    if self.band==self.oband and self.baseband==self.obaseband:
                        self.osubband.append(self.subband)
                        self.ospw_list.append(ii)
                        found = jj
                        break
                if found >= 0:
                    self.spws_info[found] = [self.oband,self.obaseband,self.osubband,self.ospw_list]
                else:
                    self.spws_info.append([self.band,self.baseband,[self.subband],[ii]])
                    self.bands.append(self.band)
            #logprint("Bands/basebands/spws are:", logfileout='logs/msinfo.log')
            for spw_info in self.spws_info:
                self.spw_info_string = spw_info[0] + '   ' + spw_info[1] + '   [' + ','.join(["%d" % ii for ii in spw_info[2]]) + ']   [' + ','.join(["%d" % ii for ii in spw_info[3]]) + ']'
                #logprint(spw_info_string, logfileout='logs/msinfo.log')
                
        # Critical fraction of flagged solutions in delay cal to avoid an
        # entire baseband being flagged on all antennas
            self.critfrac=0.9/float(len(self.spws_info))
        elif ':' in self.spw_names[0]:
            print("old spw names with :")
            #logprint("old spw names with :", logfileout='logs/msinfo.log')
        else:
            print("unknown spw names")
            #logprint("unknown spw names", logfileout='logs/msinfo.log')
        
        
        # Check for missing scans

        self.missingScans = 0
        self.missingScanStr = ''
        
        for i in range(max(self.scanNums)):
            if self.scanNums.count(i+1) == 1: pass
            else:
                #logprint ("WARNING: Scan "+str(i+1)+" is not present", logfileout='logs/msinfo.log')
                self.missingScans += 1
                self.missingScanStr = self.missingScanStr+str(i+1)+', '
        
        if (self.missingScans > 0):
            print("WARNING: There were "+str(self.missingScans)+" missing scans in this MS")
            #logprint ("WARNING: There were "+str(missingScans)+" missing scans in this MS", logfileout='logs/msinfo.log')
        else:
            print("No missing scans found.")
            #logprint ("No missing scans found.", logfileout='logs/msinfo.log')

        return True
