from __future__ import absolute_import

import numpy as np

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure as infrastructure

from pipeline.infrastructure import casa_tasks

LOG = infrastructure.get_logger(__name__)


def removeRows(caltable, spwids):
    tb = casatools.casac.table()
    tb.open(caltable, nomodify=False)
    for spwid in spwids:
        subtb = tb.query('SPECTRAL_WINDOW_ID == '+str(spwid))
        flaggedrows = subtb.rownumbers()
        if len(flaggedrows) > 0: tb.removerows(flaggedrows)
        subtb.close()
    tb.flush()
    tb.close()

    tb.open(caltable + '/SPECTRAL_WINDOW',nomodify=False)
    for spwid in spwids:
        temparray = tb.getcol('FLAG_ROW')
        temparray[spwid] = True
        tb.putcol('FLAG_ROW',temparray)
    tb.close()

def computeChanFlag(vis, caltable, context):

    m = context.observing_run.get_ms(vis)
    channels = m.get_vla_numchan()

    with casatools.TableReader(caltable) as table:
        spwVarCol  = table.getvarcol('SPECTRAL_WINDOW_ID')
        dataVarCol = table.getvarcol('CPARAM')
        flagVarCol = table.getvarcol('FLAG')
        rowlist = dataVarCol.keys()
        rowlist.sort()
        nrows = len(rowlist)

        spwids = []

        largechunk = False

        for rrow in rowlist:
            dataArr = dataVarCol[rrow]
            flagArr = flagVarCol[rrow]
            spwArr = spwVarCol[rrow]

            ispw = spwArr[0]
            fivepctch = int(0.05*channels[ispw])
            startch1 = 0
            startch2 = fivepctch - 1
            endch1 = channels[ispw] - fivepctch
            endch2 = channels[ispw] - 1
            if (fivepctch < 3):
                startch2=2
                endch1=channels[ispw]-3

            # Find flagged ranges in both polarizations
            rangeA = utils.flagged_intervals(flagArr[0,:].flatten())
            rangeB = utils.flagged_intervals(flagArr[1,:].flatten())

            # print rangeA, rangeB

            # Determine contiguous lengths, but ignoring edge flagging
            for row in rangeA[1:-1]:
                length = row[-1]-row[0]
                spwids.append(spwArr[0])
                if length > len(flagArr[0])/32.0:
                    largechunk=True

            for row in rangeB[1:-1]:
                length = row[-1]-row[0]
                spwids.append(spwArr[0])
                if length > len(flagArr[0])/32.0:
                    largechunk=True

            #print rrow.rjust(4), 'Pol A:', str(np.sum(flagArr[0])).rjust(4),' Pol B:', str(np.sum(flagArr[1])).rjust(4),                                 ' / ',str(len(flagArr[0])), ' chan flagged ', '(', 100.0*float(np.sum(flagArr[0]))/len(flagArr[0]),'%,  ',                                 100.0*float(np.sum(flagArr[1]))/len(flagArr[1]), '%)'

    spwids = np.unique(spwids)
    spwids = list(spwids)

    return (largechunk, spwids)


def do_bandpass(vis, caltable, context=None, RefAntOutput=None, spw=None, ktypecaltable=None, bpdgain_touse=None, solint=None, append=None):
    """Run CASA task bandpass"""

    m = context.observing_run.get_ms(vis)
    bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
    bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
    minBL_for_cal = max(3,int(len(m.antennas)/2.0))

    BPGainTables = list(context.callibrary.active.get_caltable())
    BPGainTables.append(ktypecaltable)
    BPGainTables.append(bpdgain_touse)

    bandpass_task_args = {'vis'         :vis,
                          'caltable'    :caltable,
                          'field'       :bandpass_field_select_string,
                          'spw'         :spw,
                          'intent'      :'',
                          'selectdata'  :True,
                          'uvrange'     :'',
                          'scan'        :bandpass_scan_select_string,
                          'solint'      :solint,
                          'combine'     :'scan',
                          'refant'      :RefAntOutput.lower(),
                          'minblperant' :minBL_for_cal,
                          'minsnr'      :5.0,
                          'solnorm'     :False,
                          'bandtype'    :'B',
                          'fillgaps'    :0,
                          'smodel'      :[],
                          'append'      :append,
                          'docallib'    :False,
                          'gaintable'   :BPGainTables,
                          'gainfield'   :[''],
                          'interp'      :[''],
                          'spwmap'      :[],
                          'parang'      :False}

    job = casa_tasks.bandpass(**bandpass_task_args)

    return job


def weakbp(vis, caltable, context=None, RefAntOutput=None, ktypecaltable=None, bpdgain_touse=None, solint=None, append=None):

    m = context.observing_run.get_ms(vis)
    channels = m.get_vla_numchan()  #Number of channels before averaging

    bpjob = do_bandpass(vis, caltable, context=context, spw='', RefAntOutput=RefAntOutput,
                        ktypecaltable=ktypecaltable, bpdgain_touse=bpdgain_touse, solint='inf', append=False)
    bpjob.execute()
    (largechunk, spwids) = computeChanFlag(vis, caltable, context)
    #print largechunk, spwids
    if (largechunk==False and spwids==[]):
        #All solutions found - proceed as normal with the pipeline
        interp = ''
        return interp

    LOG.warning("Solutions for all channels not obtained.  Using weak bandpass calibrator heuristics.")
    cpa = 2  # Channel pre-averaging
    while(largechunk == True):

        LOG.info("Removing rows in table " + caltable + " for spwids="+','.join([str(i) for i in spwids]))
        removeRows(caltable, spwids)
        solint = 'inf,'+str(cpa)+'ch'
        LOG.warning("Largest contiguous set of channels with no BP solution is greater than maximum allowable 1/32 fractional bandwidth for spwid="+','.join([str(i) for i in spwids])+"."+ "  Using solint=" + solint)
        LOG.info('Weak bandpass heuristics.  Using solint='+solint)
        bpjob = do_bandpass(vis, caltable, context=context, RefAntOutput=RefAntOutput, spw=','.join([str(i) for i in spwids]),
                            ktypecaltable=ktypecaltable, bpdgain_touse=bpdgain_touse, solint=solint, append=True)
        bpjob.execute()
        (largechunk, spwids) = computeChanFlag(vis, caltable, context)
        for spw in spwids:
            preavgnchan = channels[spw]/float(cpa)
            LOG.debug("CPA: "+str(cpa)+"   NCHAN: "+str(preavgnchan)+"    NCHAN/32: "+str(preavgnchan/32.0))
            if (cpa >= preavgnchan/32.0):
                LOG.warn("Pre-averaging has exceeded Nchan/32 for spwid="+str(spw))
                largechunk = False  #This will break the while loop and move onto applycal
        cpa = cpa * 2

    LOG.warning("Gaps in bandpass solutions will be linearly interpolated over in applycal.")
    interp = 'nearest'
    return interp