#!/usr/bin/env python
#
# tsysNormalize: function for normalizing a Tsys cal table to a common elevation
# (the elevation of the first Tsys scan). 
# Based on Neil Phillips tsystransfer.py (originally posted in CAS-4636)
# Todd Hunter (July 12, 2016)
#
# Modified for Pipeline, Vincent Geers, 20 July 2016.

import numpy as np
import pylab as pb

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)


def TsysAfterPowerChange(v_orig, v_new, tsys_orig=200.0, t_atm=270.0):
    # The maths to derive a new Tsys
    x = (v_new / v_orig) * (tsys_orig / (tsys_orig + t_atm))
    return t_atm * x / (1.0 - x)


def getPower(vis, scan, spw, duration, fromEnd=False, skipStartSecs=1.0, 
             skipEndSecs=1.0, datacolname='data', verbose=True):
    """
    Return a per-antenna list of total power values for the two polarizations of the specified scan and spw.
    duration: number of samples to use starting from the start of the scan
    I think the idea here is that the sky subscan is always the first subscan of a Tsys scan.  If this ever
    changes to one of the loads, then the result will be less than optimal. It would likely result in very
    small changes from the original tsys table (i.e. it will not get normalized).   - Todd
    """

    # Read in data from MS
    with casatools.MSReader(vis) as ms:
        ms.selectinit(datadescid=spw)
        ms.selecttaql('SCAN_NUMBER==%d AND DATA_DESC_ID==%d AND ANTENNA1==ANTENNA2'%(scan, spw))
        d = ms.getdata([datacolname,'axis_info'], ifraxis=True)

    powers = d[datacolname]
    ants = d['axis_info']['ifr_axis']['ifr_name']
    pols = list(d['axis_info']['corr_axis'])
    idxPol0 = pols.index("XX")
    idxPol1 = pols.index("YY")
    if verbose:
        LOG.info("Pol 0,1 indexes: %d, %d" % (idxPol0, idxPol1))
    ts = d['axis_info']['time_axis']['MJDseconds']
    t0 = ts[0]
    tf = ts[-1]
    ts -= t0
    if verbose:
        LOG.info("times:", ts)
        LOG.info("pols:", pols)
    # choose the time samples we want
    sampStart = -99
    sampEnd = -99
    for i in range(len(ts)):
        if ts[i] > skipStartSecs:
            sampStart = i
            break
    if sampStart >= len(ts):
        sampStart = len(ts) - 1
    for i in range(len(ts)-1, sampStart, -1):
        if tf - ts[i] > skipEndSecs:
            sampEnd = i
            break
    if sampEnd <= sampStart:
        sampEnd = sampStart + 1
    if not fromEnd:
        # take duration samples from start
        for i in range(sampStart+1, sampEnd, 1):
            if ts[i] - ts[sampStart] > duration:
                sampEnd = i
                break
    else:
        # instead from end
        for i in range(sampEnd-1, sampStart, -1):
            if ts[sampEnd] - ts[i] > duration:
                sampStart = i
                break
    if verbose:
        LOG.info("chosen sample range: %d to %d" % (sampStart, sampEnd))
    # indexing is pol, baseline(=0), ant, sample
    if verbose:
        LOG.info("number of antennas to produce powers for:", len(ants))
    result = []
    for ant in range(len(ants)):
        powersPol0 = powers[idxPol0][0][ant][sampStart:sampEnd]
        powersPol1 = powers[idxPol1][0][ant][sampStart:sampEnd]
        #print "Ant %d powers pol 0: %s, pol 1: %s" % (ant, str(powersPol0), str(powersPol1))
        medianP0 = np.median(powersPol0)
        medianP1 = np.median(powersPol1)
        result.append([medianP0,medianP1])
        #print "Ant %2d (%s) median powers for pols 0,1: %12.6f, %12.6f (nSamples = %d, %d)" % (ant, ants[ant], medianP0, medianP1, len(powersPol0), len(powersPol1))
    return result

def scienceSpwForTsysSpw(mymsmd, tsysSpw):
    """
    Automatically pick one science spw for the specified Tsys spw.  
    Ideally it would be the one with the widest bandwidth, but it will likely make very little
    difference, so just take the first one if there are more than one.
    """
    # FIXME: Use domain objects from context instead of msmd.
    baseband = mymsmd.baseband(tsysSpw)
    spws = np.intersect1d(mymsmd.almaspws(tdm=True,fdm=True),mymsmd.spwsforbaseband(baseband))
    if 'OBSERVE_TARGET#ON_SOURCE' in mymsmd.intents():
        spws = np.intersect1d(mymsmd.spwsforintent('OBSERVE_TARGET#ON_SOURCE'), spws)
    elif 'CALIBRATE_FLUX#ON_SOURCE' in mymsmd.intents():
        spws = np.intersect1d(mymsmd.spwsforintent('CALIBRATE_FLUX#ON_SOURCE'), spws)
    elif 'CALIBRATE_AMPLI#ON_SOURCE' in mymsmd.intents():
        spws = np.intersect1d(mymsmd.spwsforintent('CALIBRATE_AMPLI#ON_SOURCE'), spws)
        
    if (len(spws) > 1):
        LOG.info("Multiple science spws for this tsys spw (%d).  Picking the first one (%d)" % (tsysSpw, spws[0]))
        return(spws[0])
    elif (len(spws) < 1):
        LOG.info("No science spws for this tsys spw (%d)" % (tsysSpw))
    else:
        return(spws[0])
    return -1


def tsysNormalize(vis, tsysTable, newTsysTable, scaleSpws=[], verbose=False):
    """
    Generate Tsys entries for one field from other fields, using autocorr
    (linear!) or SQLD data to determine the change in Tsys.
    Inputs:
     vis          the MS
     tsysTable:  the tsys caltable (default = <vis>.tsys)
     newTsysTable:  the new tsys caltable to create (default = <tsysTable>_normalized)
    """

    # intents likely to imply different attenuations or tuning to science-like
    # scans that we are applying Tsys to.
    badIntents = ['CALIBRATE_POINTING', 'CALIBRATE_FOCUS', 'CALIBRATE_SIDEBAND_RATIO', 'CALIBRATE_ATMOSPHERE']

    # Determine column name for data. This presently differs between
    # interferometry and single-dish, as they use different values 
    # for the parameter ocorr_mode at the importasdm stage.
    # Note: using lower-case for column name, as ms.getdata will return
    # a dictionary that has lower-case keys, regardless of whether 
    # the column was upper-case.
    # TODO: need a common solution for identifying data column.
    with casatools.TableReader(vis) as tb:
        # SD will contain "FLOAT_DATA"
        if 'FLOAT_DATA' in tb.colnames():
            datacolname = 'float_data'
        # IF will contain "DATA"
        else:
            datacolname = 'data'

    with casatools.MSMDReader(vis) as msmd:

        with casatools.TableReader(tsysTable, nomodify=False) as table:
        
            # For convenience squish the useful columns into unique lists
            tsysSpws     = pb.unique(table.getcol("SPECTRAL_WINDOW_ID"))
            tsysScans    = pb.unique(table.getcol("SCAN_NUMBER"))
            tsysTimes    = pb.unique(table.getcol("TIME"))
            tsysFields   = pb.unique(table.getcol("FIELD_ID"))
            tsysAntennas = pb.unique(table.getcol("ANTENNA1"))
        
            if len(scaleSpws) < len(tsysSpws):
                scaleSpws = []
                for tsysSpw in tsysSpws:
                    scaleSpws.append(scienceSpwForTsysSpw(msmd, tsysSpw))
                LOG.info("Identified autocorrelation spws to use: {0}".format(scaleSpws))
            
            LOG.info("Tsys Spws ({0}): {1}".format(len(tsysSpws), tsysSpws))
            LOG.info("Tsys Scans ({0}): {1}".format(len(tsysScans), tsysScans))
            LOG.info("Tsys Times ({0}): {1}".format(len(tsysTimes), tsysTimes))
            LOG.info("Tsys Fields ({0}): {1}".format(len(tsysFields), tsysFields))
            LOG.info("Tsys Antennas ({0}): {1}".format(len(tsysAntennas), tsysAntennas))
        
            # Gather the power levels to use in the normalization process
            refPowers = {}
            refScans = {}
            for f in tsysFields:
                scanFieldsTab = table.query('FIELD_ID==%d'%f)
                fieldTsysScans = pb.unique(scanFieldsTab.getcol("SCAN_NUMBER"))
                scanFieldsTab.close()
                fieldAllScans = msmd.scansforfield(f)
                fieldNonTsysScans = [x for x in fieldAllScans if x not in fieldTsysScans]
                if (len(fieldNonTsysScans) < 1):
                    # Then there is no non-tsys scan for this field, e.g. which can happen in a mosaic where the Tsys scan has a different field ID,
                    # but in this case the field name will have other scans with different field IDs, so revert to using field names.  Using field
                    # names might work from the outset, but I have not tried it.
                    fieldName = msmd.namesforfields(f)[0]
                    fieldAllScans = msmd.scansforfield(fieldName)
                    fieldNonTsysScans = [x for x in fieldAllScans if x not in fieldTsysScans]
                    if (len(fieldNonTsysScans) < 1):
                        LOG.info("This field (id={0}, name={1}) appears to have "
                                 "no non-Tsys-like-scans, and thus cannot be "
                                 "normalized.".format(f,fieldName))
                        return -1
                scienceLikeScans = []
                for s in fieldNonTsysScans:
                    intents = msmd.intentsforscan(s)
                    good = True
                    for i in intents:
                        for b in badIntents:
                            if i.startswith(b):
                                good = False
                                break
                    if good: scienceLikeScans.append(s)
                powerRefScans = []
                for s in fieldTsysScans:
                    minDist = 9999999
                    refScan = -1
                    for r in scienceLikeScans:
                        dist = abs(r - s)
                        if dist < minDist:
                            minDist = dist
                            refScan = r
                    powerRefScans.append(refScan)
                LOG.info("Field {0} Tsys scans: {1}, All scans: {2}, Non-Tsys "
                         "scans: {3}, Non-Tsys science-like scans: {4}".format(
                         f, fieldTsysScans, fieldAllScans, fieldNonTsysScans, 
                         scienceLikeScans))
                for i in range(len(fieldTsysScans)):
                    LOG.info("        Tsys scan {0} power reference scan: {1}".format(
                             fieldTsysScans[i], powerRefScans[i]))
                    refScans[fieldTsysScans[i]] = powerRefScans[i]
                if verbose:
                    LOG.info("populating powers corresponding to each Tsys scan...")
                for i in range(len(fieldTsysScans)):
                    refPowers[fieldTsysScans[i]] = []
                    for spw in scaleSpws:
                        if verbose:
                            LOG.info("calling getPower(vis, {0}, {1}, 10.0, {2})".format(
                              powerRefScans[i], spw, str(powerRefScans[i] < fieldTsysScans[i])))
                        p = getPower(vis, powerRefScans[i], spw, 10.0, 
                          powerRefScans[i] < fieldTsysScans[i], 
                          datacolname=datacolname, verbose=verbose)
                        refPowers[fieldTsysScans[i]].append(p)
                    if verbose:
                        LOG.info("powers to use for Tsys scan {0}: {1}".format(fieldTsysScans[i], refPowers[fieldTsysScans[i]]))
            if verbose: LOG.info(refPowers)
        
            # Create copy of the original Tsys caltable.
            copy = table.copy(newTsysTable)
            copy.close()    

    with casatools.TableReader(newTsysTable, nomodify=False) as table:
        startRefPower = refPowers[tsysScans[0]]
        for i in range(1,len(tsysScans)):
            # need to adjust each successive Tsys
            for ispw in range(len(tsysSpws)):
                spw = tsysSpws[ispw]
                for ant in range(len(tsysAntennas)):
                    tsysSubTab1 = table.query("SCAN_NUMBER==%d AND SPECTRAL_WINDOW_ID==%d AND ANTENNA1==%d"%(tsysScans[i],tsysSpws[ispw],ant))
                    tsys1 = tsysSubTab1.getcell('FPARAM', 0)
                    newTsys = tsysSubTab1.getcell('FPARAM', 0)
                    for pol in range(len(tsys1)):
                        for chan in range(len(tsys1[pol])):
                            a = TsysAfterPowerChange(refPowers[tsysScans[i]][ispw][ant][pol], startRefPower[ispw][ant][pol], tsys1[pol][chan])
                            newTsys[pol][chan] = a
                        #LOG.info("Scan %2d spw %2d pol %d mean %.1f --> %.1f" % (tsysScans[i], spw, pol, np.mean(tsys1[pol]), np.mean(newTsys[pol])))
                        LOG.info("Scan {0:2d} spw {1:2d} pol {2} mean {3:.1f} --> {4:.1f}".format(
                                 tsysScans[i], spw, pol, np.mean(tsys1[pol]), np.mean(newTsys[pol])))
                    tsysSubTab1.putcell('FPARAM', 0, newTsys)
                    tsysSubTab1.close()
