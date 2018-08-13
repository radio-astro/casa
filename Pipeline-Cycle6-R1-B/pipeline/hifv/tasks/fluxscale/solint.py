from __future__ import absolute_import

import os
import numpy as np

import pipeline.hif.heuristics.findrefant as findrefant
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.vdp as vdp
from pipeline.hifv.heuristics import getCalFlaggedSoln
from pipeline.infrastructure import casa_tasks
from pipeline.infrastructure import task_registry

LOG = infrastructure.get_logger(__name__)


class SolintInputs(vdp.StandardInputs):
    limit_short_solint = vdp.VisDependentProperty(default='')
    refantignore = vdp.VisDependentProperty(default='')

    def __init__(self, context, vis=None, limit_short_solint=None, refantignore=None):
        super(SolintInputs, self).__init__()
        self.context = context
        self.vis = vis
        self.limit_short_solint = limit_short_solint
        self.refantignore = refantignore
        self.gain_solint1 = 'int'
        self.gain_solint2 = 'int'


class SolintResults(basetask.Results):
    def __init__(self, final=None, pool=None, preceding=None, longsolint=None, gain_solint2=None,
                 shortsol2=None, short_solint=None, new_gain_solint1=None, vis=None,
                 bpdgain_touse=None):

        if final is None:
            final = []
        if pool is None:
            pool = []
        if preceding is None:
            preceding = []

        super(SolintResults, self).__init__()

        self.vis = vis
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()
        self.longsolint = longsolint
        self.gain_solint2 = gain_solint2
        
        self.shortsol2 = shortsol2
        self.short_solint = short_solint
        self.new_gain_solint1 = new_gain_solint1
        self.bpdgain_touse = bpdgain_touse
        
    def merge_with_context(self, context):    
        m = context.observing_run.get_ms(self.vis)
        context.evla['msinfo'][m.name].gain_solint2 = self.gain_solint2
        context.evla['msinfo'][m.name].longsolint = self.longsolint
        
        context.evla['msinfo'][m.name].shortsol2 = self.shortsol2
        context.evla['msinfo'][m.name].short_solint = self.short_solint
        context.evla['msinfo'][m.name].new_gain_solint1 = self.new_gain_solint1


@task_registry.set_equivalent_casa_task('hifv_solint')
class Solint(basetask.StandardTaskTemplate):
    Inputs = SolintInputs
    
    def prepare(self):
    
        # Solint section
        calMs = 'calibrators.ms'
        split_result = self._do_split(calMs)
        (longsolint, gain_solint2) = self._do_determine_solint(calMs)

        try:
            stage_number = self.inputs.context.results[-1].read()[0].stage_number + 1
        except Exception as e:
            stage_number = self.inputs.context.results[-1].read().stage_number + 1

        tableprefix = os.path.basename(self.inputs.vis) + '.' + 'hifv_solint.s'
        
        # Testgains section
        context = self.inputs.context
        tablebase = tableprefix + str(stage_number) + '_1.' + 'testgaincal'
        table_suffix = ['.tbl','3.tbl','10.tbl', 'scan.tbl', 'limit.tbl']
        soltimes = [1.0,3.0,10.0] 
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        soltimes = [m.get_vla_max_integration_time() * x for x in soltimes]

        solints = ['int', str(soltimes[1])+'s', str(soltimes[2])+'s']
        soltime = soltimes[0]
        solint = solints[0]
        shortsol1 = context.evla['msinfo'][m.name].shortsol1
        combtime = 'scan'

        refantfield = context.evla['msinfo'][m.name].calibrator_field_select_string
        refantobj = findrefant.RefAntHeuristics(vis=calMs, field=refantfield,
                                                geometry=True, flagging=True, intent='',
                                                spw='', refantignore=self.inputs.refantignore)
        
        RefAntOutput = refantobj.calculate()

        refAnt = ','.join(RefAntOutput)

        bpdgain_touse = tablebase + table_suffix[0]
        testgains_result = self._do_gtype_testgains(calMs, bpdgain_touse, solint=solint,
                                                    context=context, combtime=combtime, refAnt=refAnt)

        flaggedSolnResult1 = getCalFlaggedSoln(bpdgain_touse)
        LOG.info("For solint = " + solint + " fraction of flagged solutions = " +
                 str(flaggedSolnResult1['all']['fraction']))
        LOG.info("Median fraction of flagged solutions per antenna = " +
                 str(flaggedSolnResult1['antmedian']['fraction']))

        if flaggedSolnResult1['all']['total'] > 0:
            fracFlaggedSolns1=flaggedSolnResult1['antmedian']['fraction']
        else:
            fracFlaggedSolns1=1.0

        shortsol2=soltime

        if fracFlaggedSolns1 > 0.05:
            soltime = soltimes[1]
            solint = solints[1]

            testgains_result = self._do_gtype_testgains(calMs, tablebase + table_suffix[1], solint=solint,
                                                        context=context, combtime=combtime, refAnt=refAnt)
            flaggedSolnResult3 = getCalFlaggedSoln(tablebase + table_suffix[0])
            
            LOG.info("For solint = "+solint+" fraction of flagged solutions = " +
                     str(flaggedSolnResult3['all']['fraction']))
            LOG.info("Median fraction of flagged solutions per antenna = " +
                     str(flaggedSolnResult3['antmedian']['fraction']))

            if flaggedSolnResult3['all']['total'] > 0:
                fracFlaggedSolns3=flaggedSolnResult3['antmedian']['fraction']
            else:
                fracFlaggedSolns3=1.0

            if fracFlaggedSolns3 < fracFlaggedSolns1:
                shortsol2 = soltime
                bpdgain_touse = tablebase + table_suffix[1]
            
                if fracFlaggedSolns3 > 0.05:
                    soltime = soltimes[2]
                    solint = solints[2]

                    testgains_result = self._do_gtype_testgains(calMs, tablebase + table_suffix[2], solint=solint,
                                                                context=context, combtime=combtime, refAnt=refAnt)
                    flaggedSolnResult10 = getCalFlaggedSoln(tablebase + table_suffix[2])
                    LOG.info("For solint = "+solint+" fraction of flagged solutions = " +
                             str(flaggedSolnResult3['all']['fraction']))
                    LOG.info("Median fraction of flagged solutions per antenna = " +
                             str(flaggedSolnResult3['antmedian']['fraction']))

                    if flaggedSolnResult10['all']['total'] > 0:
                        fracFlaggedSolns10 = flaggedSolnResult10['antmedian']['fraction']
                    else:
                        fracFlaggedSolns10 = 1.0

                    if fracFlaggedSolns10 < fracFlaggedSolns3:
                        shortsol2=soltime
                        bpdgain_touse = tablebase + table_suffix[2]

                        if fracFlaggedSolns10 > 0.05:
                            solint='inf'
                            combtime=''
                            testgains_result = self._do_gtype_testgains(calMs, tablebase + table_suffix[3],
                                                                        solint=solint, context=context,
                                                                        combtime=combtime, refAnt=refAnt)
                            flaggedSolnResultScan = getCalFlaggedSoln(tablebase + table_suffix[3])
                            LOG.info("For solint = "+solint+" fraction of flagged solutions = " +
                                     str(flaggedSolnResult3['all']['fraction']))
                            LOG.info("Median fraction of flagged solutions per antenna = " +
                                     str(flaggedSolnResult3['antmedian']['fraction']))
                            
                            if flaggedSolnResultScan['all']['total'] > 0:
                                fracFlaggedSolnsScan = flaggedSolnResultScan['antmedian']['fraction']
                            else:
                                fracFlaggedSolnsScan = 1.0
                                
                            if fracFlaggedSolnsScan < fracFlaggedSolns10:
                                shortsol2=context.evla['msinfo'][m.name].longsolint
                                bpdgain_touse = tablebase + table_suffix[3]
                                
                                if fracFlaggedSolnsScan > 0.05:
                                    LOG.warn("Warning, large fraction of flagged solutions.  " +
                                             "There might be something wrong with your data")

        LOG.info("ShortSol1: " + str(shortsol1))
        LOG.info("ShortSol2: " + str(shortsol2))
        
        short_solint = max(shortsol1,shortsol2)
        LOG.info("Short_solint determined from heuristics: " + str(short_solint))
        new_gain_solint1 = str(short_solint)+'s'

        if self.inputs.limit_short_solint:
            LOG.warn("Short Solint limited by user keyword input to " + str(self.inputs.limit_short_solint))
            limit_short_solint = self.inputs.limit_short_solint

            short_solint_str = "{:.12f}".format(short_solint)

            if limit_short_solint == 'int':
                limit_short_solint = '0'
                combtime = 'scan'
                short_solint = float(limit_short_solint)
                new_gain_solint1 = str(short_solint) + 's'
            elif limit_short_solint == 'inf':
                combtime = ''
                short_solint = limit_short_solint
                new_gain_solint1 = short_solint
                LOG.warn("   Note that since 'inf' was specified then combine='' for gaincal.")
            elif float(limit_short_solint) <= short_solint_str:
                short_solint = float(limit_short_solint)
                new_gain_solint1 = str(short_solint) + 's'
                combtime = 'scan'

            testgains_result = self._do_gtype_testgains(calMs, tablebase + table_suffix[4], solint=new_gain_solint1,
                                                        context=context, combtime=combtime, refAnt=refAnt)
            bpdgain_touse = tablebase + table_suffix[4]

        LOG.info("Using short solint = " + str(new_gain_solint1))

        if abs(longsolint - short_solint) <= soltime:
            LOG.warn('Short solint = long solint +/- integration time of {}s'.format(soltime))

        return SolintResults(longsolint=longsolint, gain_solint2=gain_solint2, shortsol2=shortsol2,
                             short_solint=short_solint, new_gain_solint1=new_gain_solint1, vis=self.inputs.vis,
                             bpdgain_touse=bpdgain_touse)
    
    def analyse(self, results):
        return results
    
    def _do_split(self, calMs):
        
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        calibrator_scan_select_string = self.inputs.context.evla['msinfo'][m.name].calibrator_scan_select_string
    
        LOG.info("Splitting out calibrators into " + calMs)
    
        task_args = {'vis': m.name,
                     'outputvis': calMs,
                     'datacolumn': 'corrected',
                     'keepmms': True,
                     'field': '',
                     'spw': '',
                     # 'width'        : int(max(channels)),
                     'width': 1,
                     'antenna': '',
                     'timebin': '0s',
                     'timerange': '',
                     'scan': calibrator_scan_select_string,
                     'intent': '',
                     'array': '',
                     'uvrange': '',
                     'correlation': '',
                     'observation': '',
                     'keepflags': False}
        
        job = casa_tasks.split(**task_args)
            
        return self._executor.execute(job)

    def _do_determine_solint(self, calMs):
        
        durations = []
        old_spws = []
        old_field = ''
        
        with casatools.MSReader(calMs) as ms:
            scan_summary = ms.getscansummary()    
            
            m = self.inputs.context.observing_run.get_ms(self.inputs.vis)

            phase_scan_list = self.inputs.context.evla['msinfo'][m.name].phase_scan_select_string.split(',')
            phase_scan_list = [int(i) for i in phase_scan_list]
            
            for kk in range(len(phase_scan_list)):
                ii = phase_scan_list[kk]
                
                try:
                    # Collect beginning and ending times
                    # Take max of end times and min of beginning times
                    endtimes = [scan_summary[str(ii)][scankey]['EndTime'] for scankey in scan_summary[str(ii)].keys()]
                    begintimes = [scan_summary[str(ii)][scankey]['BeginTime'] for scankey in scan_summary[str(ii)].keys()]

                    end_time = max(endtimes)
                    begin_time = min(begintimes)

                    new_spws = scan_summary[str(ii)]['0']['SpwIds']
                    new_field = scan_summary[str(ii)]['0']['FieldId']
                    
                    if ((kk > 0) and (phase_scan_list[kk-1] == ii-1) and
                            (set(new_spws) == set(old_spws)) and (new_field == old_field)):
                        # if contiguous scans, just increase the time on the previous one
                        LOG.info("End time, old begin time {} {}".format(end_time, old_begin_time))
                        durations[-1] = 86400*(end_time - old_begin_time)
                    else:
                        LOG.info("End time, begin time {} {}".format(end_time, begin_time))
                        durations.append(86400*(end_time - begin_time))
                        old_begin_time = begin_time
                        LOG.info("append durations, old, begin:".format(durations, old_begin_time, begin_time))
                    LOG.info("Scan "+str(ii)+" has "+str(durations[-1])+"s on source")
                    old_spws = new_spws
                    old_field = new_field
                
                except KeyError:
                    LOG.warn("WARNING: scan "+str(ii)+" is completely flagged and missing from " + calMs)

        longsolint = (np.max(durations)) * 1.01
        gain_solint2 = str(longsolint) + 's'
                   
        return longsolint, gain_solint2

    def _do_gtype_testgains(self, calMs, caltable, solint='int', context=None, combtime='scan', refAnt=None):

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)

        calibrator_scan_select_string=context.evla['msinfo'][m.name].calibrator_scan_select_string
        minBL_for_cal = m.vla_minbaselineforcal()

        task_args = {'vis': calMs,
                     'caltable': caltable,
                     'field': '',
                     'spw': '',
                     'intent': '',
                     'selectdata': True,
                     'scan': calibrator_scan_select_string,
                     'solint': solint,
                     'combine': combtime,
                     'preavg': -1.0,
                     'refant': refAnt.lower(),
                     'minblperant': minBL_for_cal,
                     'minsnr': 5.0,
                     'solnorm': False,
                     'gaintype': 'G',
                     'smodel': [],
                     'calmode': 'ap',
                     'append': False,
                     'gaintable': [''],
                     'gainfield': [''],
                     'interp': [''],
                     'spwmap': [],
                     'parang': True}

        job = casa_tasks.gaincal(**task_args)
            
        return self._executor.execute(job)
