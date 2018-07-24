from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
from pipeline.infrastructure import casa_tasks
from pipeline.infrastructure import task_registry
import collections
import copy
import numpy as np

LOG = infrastructure.get_logger(__name__)

# CHECKING FLAGGING OF ALL CALIBRATORS
# use rflag mode of flagdata



class CheckflagInputs(vdp.StandardInputs):
    checkflagmode = vdp.VisDependentProperty(default='')

    def __init__(self, context, vis=None, checkflagmode=None):
        super(CheckflagInputs, self).__init__()
        self.context = context
        self.vis = vis
        self.checkflagmode = checkflagmode
    

class CheckflagResults(basetask.Results):
    def __init__(self, jobs=None, summarydict=None):

        if jobs is None:
            jobs = []
        if summarydict is None:
            summarydict = {}

        super(CheckflagResults, self).__init__()

        self.jobs = jobs
        self.summarydict = summarydict
        
    def __repr__(self):
        s = 'Checkflag (rflag mode) results:\n'
        for job in self.jobs:
            s += '%s performed. Statistics to follow?' % str(job)
        return s 


@task_registry.set_equivalent_casa_task('hifv_checkflag')
class Checkflag(basetask.StandardTaskTemplate):
    Inputs = CheckflagInputs
    
    def prepare(self):
        
        LOG.info("Checking RFI flagging of BP and Delay Calibrators")
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        self.tint = m.get_vla_max_integration_time()
        pols = m.polarizations[0]
        timedevscale = 4.0
        freqdevscale = 4.0


        summaries = []  # QA statistics summaries for before and after targetflag

        # get the before flag total statistics
        job = casa_tasks.flagdata(vis=self.inputs.vis, mode='summary')
        summarydict = self._executor.execute(job)
        summaries.append(summarydict)

        # Set up threshold multiplier values for calibrators and targets separately.
        # Xpol are used for cross-hands, Ppol are used for parallel hands. As
        # noted above, I'm still refining these values; I suppose they could be
        # input parameters for the task, if needed:

        self.rflagThreshMultiplierCalsXpol = 4.0
        self.rflagThreshMultiplierCalsPpol = 4.0
        self.rflagThreshMultiplierTargetXpol = 4.0
        self.rflagThreshMultiplierTargetPpol = 7.0
        self.tfcropThreshMultiplierCals = 3.0
        self.tfcropThreshMultiplierTarget = 3.0

        # tfcrop is run per integration, set ntime here (this should be derived
        # by the pipeline):
        # tint = 0.45

        if self.inputs.checkflagmode == 'bpd-vlass':
            extendflag_result = self.do_bpdvlass()
            return extendflag_result

        if self.inputs.checkflagmode == 'allcals-vlass':
            extendflag_result = self.do_allcalsvlass()
            return extendflag_result

        if self.inputs.checkflagmode == 'target-vlass':
            fieldsobj = m.get_fields(intent='TARGET')
            fieldids = [field.id for field in fieldsobj]
            fieldselect = ','.join([str(fieldid) for fieldid in fieldids])

            if not fieldselect:
                LOG.warning("No scans with intent=TARGET are present.  CASA task flagdata not executed.")
                return CheckflagResults()
            else:
                extendflag_result = self.do_targetvlass()
                return extendflag_result

        if self.inputs.checkflagmode == 'bpd':
            fieldselect = self.inputs.context.evla['msinfo'][m.name].checkflagfields
            scanselect = self.inputs.context.evla['msinfo'][m.name].testgainscans

        if self.inputs.checkflagmode == 'allcals':
            fieldselect = self.inputs.context.evla['msinfo'][m.name].calibrator_field_select_string.split(',')
            scanselect = self.inputs.context.evla['msinfo'][m.name].calibrator_scan_select_string.split(',')

            checkflagfields = self.inputs.context.evla['msinfo'][m.name].checkflagfields.split(',')
            testgainscans = self.inputs.context.evla['msinfo'][m.name].testgainscans.split(',')

            fieldselect = ','.join([fieldid for fieldid in fieldselect if fieldid not in checkflagfields])
            scanselect = ','.join([scan for scan in scanselect if scan not in testgainscans])

        if self.inputs.checkflagmode == 'target':
            fieldsobj = m.get_fields(intent='TARGET')
            fieldids = [field.id for field in fieldsobj]
            fieldselect = ','.join([str(fieldid) for fieldid in fieldids])
            scanselect = ''
            timedevscale = 7.0
            freqdevscale = 7.0

            if not fieldselect:
                LOG.warning("No scans with intent=TARGET are present.  CASA task flagdata not executed.")
                return CheckflagResults()

        if self.inputs.checkflagmode in ('bpd', 'allcals', 'target'):
            flagbackup = True

            if ('RL' in pols.corr_type_string) and ('LR' in pols.corr_type_string):
                for correlation in ['ABS_RL', 'ABS_LR']:

                    method_args = {'mode': 'rflag',
                                   'field': fieldselect,
                                   'correlation': correlation,
                                   'scan': scanselect,
                                   'ntime': 'scan',
                                   'datacolumn': 'corrected',
                                   'flagbackup': flagbackup}

                    checkflag_result, jobresult = self._do_checkflag(**method_args)

                    flagbackup = False

                extendflag_result = self._do_extendflag(field=fieldselect, scan=scanselect)

            datacolumn = 'residual'
            corrlist = ['REAL_RR', 'REAL_LL']

            if self.inputs.checkflagmode in ('allcals', 'target'):
                datacolumn = 'corrected'
                corrlist = ['ABS_RR', 'ABS_LL']

            for correlation in corrlist:
                method_args = {'mode': 'rflag',
                               'field': fieldselect,
                               'correlation': correlation,
                               'scan': scanselect,
                               'ntime': 'scan',
                               'datacolumn': datacolumn,
                               'flagbackup': False,
                               'timedevscale': timedevscale,
                               'freqdevscale': freqdevscale}

                checkflag_result, jobresult = self._do_checkflag(**method_args)

            extendflag_result = self._do_extendflag(field=fieldselect, scan=scanselect)

            if ('RL' in pols.corr_type_string) and ('LR' in pols.corr_type_string):
                for correlation in ['ABS_LR', 'ABS_RL']:
                    method_args = {'mode': 'tfcrop',
                                   'field': fieldselect,
                                   'correlation': correlation,
                                   'scan': scanselect,
                                   'ntime': self.tint,
                                   'datacolumn': 'corrected',
                                   'flagbackup': False}

                    tfcrop_result = self._do_tfcropflag(**method_args)

                extendflag_result = self._do_extendflag(field=fieldselect, scan=scanselect)

            for correlation in ['ABS_LL', 'ABS_RR']:
                method_args = {'mode': 'tfcrop',
                               'field': fieldselect,
                               'correlation': correlation,
                               'scan': scanselect,
                               'ntime': self.tint,
                               'datacolumn': 'corrected',
                               'flagbackup': False}

                tfcrop_result = self._do_tfcropflag(**method_args)

            extendflag_result = self._do_extendflag(field=fieldselect, scan=scanselect)

            # Do a second time on targets
            if self.inputs.checkflagmode == 'target':
                for correlation in ['ABS_LL', 'ABS_RR']:
                    method_args = {'mode': 'tfcrop',
                                   'field': fieldselect,
                                   'correlation': correlation,
                                   'scan': scanselect,
                                   'ntime': self.tint,
                                   'datacolumn': 'corrected',
                                   'flagbackup': False}

                    tfcrop_result = self._do_tfcropflag(**method_args)

                extendflag_result = self._do_extendflag(field=fieldselect, scan=scanselect)

            return extendflag_result

        # Values from pipeline context
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        checkflagfields = self.inputs.context.evla['msinfo'][m.name].checkflagfields
        corrstring = m.get_vla_corrstring()
        testgainscans = self.inputs.context.evla['msinfo'][m.name].testgainscans
        
        method_args = {'mode': 'rflag',
                       'field': checkflagfields,
                       'correlation': 'ABS_' + corrstring,
                       'scan': testgainscans,
                       'ntime': 'scan',
                       'datacolumn': 'corrected',
                       'flagbackup': False}
        
        if self.inputs.checkflagmode == 'semi':
            calibrator_field_select_string = self.inputs.context.evla['msinfo'][m.name].calibrator_field_select_string
            calibrator_scan_select_string = self.inputs.context.evla['msinfo'][m.name].calibrator_scan_select_string
        
            method_args = {'mode': 'rflag',
                           'field': calibrator_field_select_string,
                           'correlation': 'ABS_' + corrstring,
                           'scan': calibrator_scan_select_string,
                           'ntime': 'scan',
                           'datacolumn': 'corrected',
                           'flagbackup': False}
        
        self._do_checkflag(**method_args)

        # get the after flag total statistics
        job = casa_tasks.flagdata(vis=self.inputs.vis, mode='summary')
        summarydict = self._executor.execute(job)
        summaries.append(summarydict)

        return CheckflagResults([job], summarydict=summaries)

    def _do_checkflag(self, mode='rflag', field=None, correlation=None, scan=None, intent='',
                      ntime='scan', datacolumn='corrected', flagbackup=False, timedevscale=4.0,
                      freqdevscale=4.0, action='apply', timedev='', freqdev=''):
        
        task_args = {'vis': self.inputs.vis,
                     'mode': mode,
                     'field': field,
                     'correlation': correlation,
                     'scan': scan,
                     'intent': intent,
                     'ntime': ntime,
                     'combinescans': False,
                     'datacolumn': datacolumn,
                     'winsize': 3,
                     'timedevscale': timedevscale,
                     'freqdevscale': freqdevscale,
                     'timedev': timedev,
                     'freqdev': freqdev,
                     'action': action,
                     'display': '',
                     'extendflags': False,
                     'flagbackup': flagbackup,
                     'savepars': True}
                     
        job = casa_tasks.flagdata(**task_args)
            
        jobresult = self._executor.execute(job)
                
        return CheckflagResults([job]), jobresult

    def analyse(self, results):
        return results

    def _do_extendflag(self, mode='extend', field=None,  scan=None, intent='',
                       ntime='scan', extendpols=True, flagbackup=False,
                       growtime=100.0, growfreq=60.0, growaround=False,
                       flagneartime=False, flagnearfreq=False):

        task_args = {'vis': self.inputs.vis,
                     'mode': mode,
                     'field': field,
                     'scan': scan,
                     'intent': intent,
                     'ntime': ntime,
                     'combinescans': False,
                     'extendpols': extendpols,
                     'growtime': growtime,
                     'growfreq': growfreq,
                     'growaround': growaround,
                     'flagneartime': flagneartime,
                     'flagnearfreq': flagnearfreq,
                     'action': 'apply',
                     'display': '',
                     'extendflags': False,
                     'flagbackup': flagbackup,
                     'savepars': False}

        job = casa_tasks.flagdata(**task_args)

        self._executor.execute(job)

        job = casa_tasks.flagdata(vis=self.inputs.vis, mode='summary')
        summarydict = self._executor.execute(job)

        return CheckflagResults([job], summarydict=summarydict)

    def _do_tfcropflag(self, mode='tfcrop', field=None, correlation=None, scan=None, intent='',
                       ntime=0.45, datacolumn='corrected', flagbackup=True,
                       freqcutoff=3.0, timecutoff=4.0):

        task_args = {'vis': self.inputs.vis,
                     'mode': mode,
                     'field': field,
                     'correlation': correlation,
                     'scan': scan,
                     'intent': intent,
                     'ntime': ntime,
                     'combinescans': False,
                     'datacolumn': datacolumn,
                     'freqcutoff': freqcutoff,
                     'timecutoff': timecutoff,
                     'freqfit': 'line',
                     'flagdimension': 'freq',
                     'action': 'apply',
                     'display': '',
                     'extendflags': False,
                     'flagbackup': flagbackup,
                     'savepars': True}

        job = casa_tasks.flagdata(**task_args)

        self._executor.execute(job)

        return CheckflagResults([job])

    def vla_basebands(self, science_windows_only=True):

        vlabasebands = []
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)

        banddict = collections.defaultdict(lambda: collections.defaultdict(list))

        for spw in m.get_spectral_windows(science_windows_only=science_windows_only):
            try:
                band = spw.name.split('#')[0].split('_')[1]
                baseband = spw.name.split('#')[1]
                banddict[band][baseband].append({str(spw.id): (spw.min_frequency, spw.max_frequency)})
            except Exception as ex:
                LOG.warn("Exception: Baseband name cannot be parsed. {!s}".format(str(ex)))

        for band in banddict.keys():
            basebands = banddict[band].keys()
            for baseband in basebands:
                spws = []
                for spwitem in banddict[band][baseband]:
                    spws.append(spwitem.keys()[0])
                vlabasebands.append(','.join(spws))

        return vlabasebands

    def thresholds(self, inputThresholds):
        # the following command maps spws 2~9 to one baseband, and 10~17 to
        # the other; the pipeline should replace this with internally-derived
        # values

        vlabasebands = self.vla_basebands()
        bbspws = [map(int, i.split(',')) for i in vlabasebands]  # Need to change for Python 3?

        # bbspws = [[2, 3, 4, 5, 6, 7, 8, 9], [10, 11, 12, 13, 14, 15, 16, 17]]

        outputThresholds = copy.deepcopy(inputThresholds)

        for ftdev in ['freqdev', 'timedev']:
            fields = inputThresholds['report0'][ftdev][:, 0]
            spws = inputThresholds['report0'][ftdev][:, 1]
            threshes = inputThresholds['report0'][ftdev][:, 2]

            ufields = np.unique(fields)
            for ifield in ufields:
                fldmask = np.where(fields == ifield)
                if len(fldmask[0]) == 0:
                    continue  # no data matching field
                # filter spws and threshes whose fields==ifield
                field_spws = spws[fldmask]
                field_threshes = threshes[fldmask]

                for ibbspws in bbspws:
                    spwmask = np.where(np.array([ispw in ibbspws for ispw in field_spws]) == True)
                    if len(spwmask[0]) == 0:
                        continue  # no data matching ibbspws
                    # filter threshes whose fields==ifield and spws in ibbspws
                    spw_field_threshes = field_threshes[spwmask]
                    medthresh = np.median(spw_field_threshes)
                    medmask = np.where(spw_field_threshes > medthresh)
                    outmask = fldmask[0][spwmask[0][medmask]]
                    outputThresholds['report0'][ftdev][:, 2][outmask] = medthresh
        return outputThresholds

    def do_bpdvlass(self):

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        fieldselect = self.inputs.context.evla['msinfo'][m.name].checkflagfields
        scanselect = self.inputs.context.evla['msinfo'][m.name].testgainscans

        for correlation, scale, datacolumn in [('ABS_RL', self.rflagThreshMultiplierCalsXpol, 'corrected'),
                                               ('ABS_LR', self.rflagThreshMultiplierCalsXpol, 'corrected'),
                                               ('REAL_RR', self.rflagThreshMultiplierCalsPpol, 'residual'),
                                               ('REAL_LL', self.rflagThreshMultiplierCalsPpol, 'residual')]:

            method_args = {'mode': 'rflag',
                           'field': fieldselect,
                           'correlation': correlation,
                           'scan': scanselect,
                           'ntime': 'scan',
                           'timedevscale': scale,
                           'freqdevscale': scale,
                           'datacolumn': datacolumn,
                           'flagbackup': False,
                           'action': 'calculate'}

            checkflagresult, rflagthresholds = self._do_checkflag(**method_args)

            rflagthresholdsnew = self.thresholds(rflagthresholds)

            method_args['timedev'] = rflagthresholdsnew
            method_args['freqdev'] = rflagthresholdsnew
            method_args['action'] = 'apply'

            checkflag_result, rflagthresholds = self._do_checkflag(**method_args)

            extendflag_result = self._do_extendflag(field=fieldselect, scan=scanselect,
                                                    growtime=100.0, growfreq=100.0)

        for correlation in ['ABS_LR', 'ABS_RL', 'ABS_LL', 'ABS_RR']:
            method_args = {'mode': 'tfcrop',
                           'field': fieldselect,
                           'correlation': correlation,
                           'scan': scanselect,
                           'timecutoff': self.tfcropThreshMultiplierCals,
                           'freqcutoff': self.tfcropThreshMultiplierCals,
                           'ntime': self.tint,
                           'datacolumn': 'corrected',
                           'flagbackup': False}

            tfcrop_result = self._do_tfcropflag(**method_args)

            extendflag_result = self._do_extendflag(field=fieldselect, scan=scanselect,
                                                    growtime=100.0, growfreq=100.0)

        # Grow flags
        extendflag_result = self._do_extendflag(field=fieldselect, scan=scanselect,
                                                growtime=100.0, growfreq=100.0,
                                                growaround=True, flagneartime=True, flagnearfreq=True)

        return extendflag_result

    def do_allcalsvlass(self):

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)

        fieldselect = self.inputs.context.evla['msinfo'][m.name].calibrator_field_select_string.split(',')
        scanselect = self.inputs.context.evla['msinfo'][m.name].calibrator_scan_select_string.split(',')

        checkflagfields = self.inputs.context.evla['msinfo'][m.name].checkflagfields.split(',')
        testgainscans = self.inputs.context.evla['msinfo'][m.name].testgainscans.split(',')

        fieldselect = ','.join([fieldid for fieldid in fieldselect if fieldid not in checkflagfields])
        scanselect = ','.join([scan for scan in scanselect if scan not in testgainscans])

        for correlation, scale, datacolumn in [('ABS_RL', self.rflagThreshMultiplierCalsXpol, 'corrected'),
                                               ('ABS_LR', self.rflagThreshMultiplierCalsXpol, 'corrected'),
                                               ('ABS_RR', self.rflagThreshMultiplierCalsXpol, 'corrected'),
                                               ('ABS_LL', self.rflagThreshMultiplierCalsXpol, 'corrected')]:

            method_args = {'mode': 'rflag',
                           'field': fieldselect,
                           'correlation': correlation,
                           'scan': scanselect,
                           'ntime': 'scan',
                           'timedevscale': scale,
                           'freqdevscale': scale,
                           'datacolumn': datacolumn,
                           'flagbackup': False,
                           'action': 'calculate'}

            checkflagresult, rflagthresholds = self._do_checkflag(**method_args)

            rflagthresholdsnew = self.thresholds(rflagthresholds)

            method_args['timedev'] = rflagthresholdsnew
            method_args['freqdev'] = rflagthresholdsnew
            method_args['action'] = 'apply'

            checkflagresult, rflagthresholds = self._do_checkflag(**method_args)

            extendflag_result = self._do_extendflag(field=fieldselect, scan=scanselect,
                                                    growtime=100.0, growfreq=100.0)

        for correlation in ['ABS_LR', 'ABS_RL', 'ABS_LL', 'ABS_RR']:
            method_args = {'mode': 'tfcrop',
                           'field': fieldselect,
                           'correlation': correlation,
                           'scan': scanselect,
                           'timecutoff': self.tfcropThreshMultiplierCals,
                           'freqcutoff': self.tfcropThreshMultiplierCals,
                           'ntime': self.tint,
                           'datacolumn': 'corrected',
                           'flagbackup': False}

            tfcrop_result = self._do_tfcropflag(**method_args)

            extendflag_result = self._do_extendflag(field=fieldselect, scan=scanselect,
                                                    growtime=100.0, growfreq=100.0)

        # Grow flags
        extendflag_result = self._do_extendflag(field=fieldselect, scan=scanselect,
                                                growtime=100.0, growfreq=100.0,
                                                growaround=True, flagneartime=True, flagnearfreq=True)

        return extendflag_result

    def do_targetvlass(self):

        for correlation, scale, datacolumn in [('ABS_RL', self.rflagThreshMultiplierTargetXpol, 'corrected'),
                                               ('ABS_LR', self.rflagThreshMultiplierTargetXpol, 'corrected'),
                                               ('ABS_RR', self.rflagThreshMultiplierTargetPpol, 'corrected'),
                                               ('ABS_LL', self.rflagThreshMultiplierTargetPpol, 'corrected')]:

            method_args = {'mode': 'rflag',
                           'field': '',
                           'correlation': correlation,
                           'scan': '',
                           'intent': '*TARGET*',
                           'ntime': 'scan',
                           'timedevscale': scale,
                           'freqdevscale': scale,
                           'datacolumn': datacolumn,
                           'flagbackup': False,
                           'action': 'calculate'}

            checkflagresult, rflagthresholds = self._do_checkflag(**method_args)

            rflagthresholdsnew = self.thresholds(rflagthresholds)

            method_args['timedev'] = rflagthresholdsnew
            method_args['freqdev'] = rflagthresholdsnew
            method_args['action'] = 'apply'

            checkflagresult, rflagthresholds = self._do_checkflag(**method_args)

            extendflag_result = self._do_extendflag(field='', scan='', intent='*TARGET*',
                                                    growtime=100.0, growfreq=100.0)

        for correlation in ['ABS_LR', 'ABS_RL', 'ABS_LL', 'ABS_RR']:
            method_args = {'mode': 'tfcrop',
                           'field': '',
                           'correlation': correlation,
                           'scan': '',
                           'intent': '*TARGET*',
                           'timecutoff': self.tfcropThreshMultiplierTarget,
                           'freqcutoff': self.tfcropThreshMultiplierTarget,
                           'ntime': self.tint,
                           'datacolumn': 'corrected',
                           'flagbackup': False}

            tfcrop_result = self._do_tfcropflag(**method_args)

            extendflag_result = self._do_extendflag(field='', scan='', intent='*TARGET*',
                                                    growtime=100.0, growfreq=100.0)

        # Grow flags
        extendflag_result = self._do_extendflag(field='', scan='', intent='*TARGET*',
                                                growtime=100.0, growfreq=100.0,
                                                growaround=True, flagneartime=True, flagnearfreq=True)

        return extendflag_result


