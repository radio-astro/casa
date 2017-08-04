from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)

# CHECKING FLAGGING OF ALL CALIBRATORS
# use rflag mode of flagdata

class CheckflagInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None, checkflagmode=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())
    
    @property
    def checkflagmode(self):
        return self._checkflagmode

    @checkflagmode.setter
    def checkflagmode(self, value):
        if value is None:
            value = None
        self._checkflagmode = value
    

class CheckflagResults(basetask.Results):
    def __init__(self, jobs=None):

        if jobs is None:
            jobs = []

        super(CheckflagResults, self).__init__()

        self.jobs = jobs
        
    def __repr__(self):
        s = 'Checkflag (rflag mode) results:\n'
        for job in self.jobs:
            s += '%s performed. Statistics to follow?' % str(job)
        return s 


class Checkflag(basetask.StandardTaskTemplate):
    Inputs = CheckflagInputs
    
    def prepare(self):
        
        LOG.info("Checking RFI flagging of BP and Delay Calibrators")
        m = self.inputs.context.observing_run.measurement_sets[0]
        tint = m.get_vla_max_integration_time()
        pols = m.polarizations[0]
        timedevscale = 4.0
        freqdevscale = 4.0

        if (self.inputs.checkflagmode == 'bpd'):
            fieldselect = self.inputs.context.evla['msinfo'][m.name].checkflagfields
            scanselect = self.inputs.context.evla['msinfo'][m.name].testgainscans

        if (self.inputs.checkflagmode == 'allcals'):
            fieldselect = self.inputs.context.evla['msinfo'][m.name].calibrator_field_select_string
            scanselect = self.inputs.context.evla['msinfo'][m.name].calibrator_scan_select_string

            checkflagfields = self.inputs.context.evla['msinfo'][m.name].checkflagfields
            testgainscans = self.inputs.context.evla['msinfo'][m.name].testgainscans

            fieldselect = fieldselect.split(',')
            scanselect = scanselect.split(',')

            checkflagfields = checkflagfields.split(',')
            testgainscans = testgainscans.split(',')

            fieldselect = [fieldid for fieldid in fieldselect if fieldid not in checkflagfields]
            scanselect = [scan for scan in scanselect if scan not in testgainscans]

            fieldselect = ','.join(fieldselect)
            scanselect = ','.join(scanselect)

        if (self.inputs.checkflagmode == 'target'):
            fieldsobj = m.get_fields(intent='TARGET')
            fieldids = [field.id for field in fieldsobj]
            fieldselect = ','.join([str(fieldid) for fieldid in fieldids])
            scanselect = ''
            timedevscale = 7.0

        if (self.inputs.checkflagmode == 'bpd' or
            self.inputs.checkflagmode == 'allcals' or
            self.inputs.checkflagmode == 'target'):

            if ('RL' in pols.corr_type_string) and ('LR' in pols.corr_type_string):
                for correlation in ['ABS_RL', 'ABS_LR']:

                    method_args = {'mode': 'rflag',
                                   'field': fieldselect,
                                   'correlation': correlation,
                                   'scan': scanselect,
                                   'ntime': 'scan',
                                   'datacolumn': 'corrected',
                                   'flagbackup': True}

                    checkflag_result = self._do_checkflag(**method_args)

                extendflag_result = self._do_extendflag(field=fieldselect,
                                                        scan=scanselect)

            datacolumn = 'residual'
            corrlist = ['REAL_RR', 'REAL_LL']

            if self.inputs.checkflagmode in ('allcals','target'):
                datacolumn = 'corrected'
                corrlist = ['ABS_RR', 'ABS_LL']

            for correlation in corrlist:
                method_args = {'mode': 'rflag',
                               'field': fieldselect,
                               'correlation': correlation,
                               'scan': scanselect,
                               'ntime': 'scan',
                               'datacolumn': datacolumn,
                               'flagbackup': True,
                               'timedevscale' : timedevscale,
                               'freqdevscale' : freqdevscale}

                checkflag_result = self._do_checkflag(**method_args)

            extendflag_result = self._do_extendflag(field=fieldselect,
                                                        scan=scanselect)

            if ('RL' in pols.corr_type_string) and ('LR' in pols.corr_type_string):
                for correlation in ['ABS_LR', 'ABS_RL']:
                    method_args = {'mode': 'tfcrop',
                                   'field': fieldselect,
                                   'correlation': correlation,
                                   'scan':scanselect,
                                   'ntime': tint,
                                   'datacolumn': 'corrected',
                                   'flagbackup': True}

                    tfcrop_result = self._do_tfcropflag(**method_args)

                extendflag_result = self._do_extendflag(field=fieldselect,
                                                        scan=scanselect)

            for correlation in ['ABS_LL', 'ABS_RR']:
                method_args = {'mode': 'tfcrop',
                               'field': fieldselect,
                               'correlation': correlation,
                               'scan': scanselect,
                               'ntime': tint,
                               'datacolumn': 'corrected',
                               'flagbackup': True}

                tfcrop_result = self._do_tfcropflag(**method_args)

            extendflag_result = self._do_extendflag(field=fieldselect,
                                                        scan=scanselect)

            # Do a second time on targets
            if (self.inputs.checkflagmode == 'target'):
                for correlation in ['ABS_LL', 'ABS_RR']:
                    method_args = {'mode': 'tfcrop',
                                   'field': fieldselect,
                                   'correlation': correlation,
                                   'scan': scanselect,
                                   'ntime': tint,
                                   'datacolumn': 'corrected',
                                   'flagbackup': True}

                    tfcrop_result = self._do_tfcropflag(**method_args)

                extendflag_result = self._do_extendflag(field=fieldselect,
                                                        scan=scanselect)

            return extendflag_result

        # Values from pipeline context
        m = self.inputs.context.observing_run.measurement_sets[0]
        checkflagfields = self.inputs.context.evla['msinfo'][m.name].checkflagfields
        corrstring = m.get_vla_corrstring()
        testgainscans = self.inputs.context.evla['msinfo'][m.name].testgainscans
        
        method_args = {'mode'        : 'rflag',
                       'field'       : checkflagfields,
                       'correlation' : 'ABS_' + corrstring,
                       'scan'        : testgainscans,
                       'ntime'       : 'scan',
                       'datacolumn'  : 'corrected',
                       'flagbackup'  : False}
        
        if (self.inputs.checkflagmode == 'semi'):
            calibrator_field_select_string = self.inputs.context.evla['msinfo'][m.name].calibrator_field_select_string
            calibrator_scan_select_string = self.inputs.context.evla['msinfo'][m.name].calibrator_scan_select_string
        
            method_args = {'mode'        : 'rflag',
                           'field'       : calibrator_field_select_string,
                           'correlation' : 'ABS_' + corrstring,
                           'scan'        : calibrator_scan_select_string,
                           'ntime'       : 'scan',
                           'datacolumn'  : 'corrected',
                           'flagbackup'  : False}
        
        checkflag_result = self._do_checkflag(**method_args)
        
        return checkflag_result
    
    def _do_checkflag(self, mode='rflag', field=None, correlation=None, scan=None,
                      ntime='scan', datacolumn='corrected', flagbackup=False, timedevscale=4.0,
                      freqdevscale=4.0):
        
        task_args = {'vis'          : self.inputs.vis,
                     'mode'         : mode,
                     'field'        : field,
                     'correlation'  : correlation,
                     'scan'         : scan,
                     'ntime'        : ntime,
                     'combinescans' : False,
                     'datacolumn'   : datacolumn,
                     'winsize'      : 3,
                     'timedevscale' : timedevscale,
                     'freqdevscale' : freqdevscale,
                     'action'       : 'apply',
                     'display'      : '',
                     'extendflags'  : False,
                     'flagbackup'   : flagbackup,
                     'savepars'     : True}
                     
        job = casa_tasks.flagdata(**task_args)
            
        self._executor.execute(job)
                
        return CheckflagResults([job])

    def analyse(self, results):
        return results

    def _do_extendflag(self, mode='extend', field=None,  scan=None,
                      ntime='scan', extendpols=True, flagbackup=False):

        task_args = {'vis': self.inputs.vis,
                     'mode': mode,
                     'field': field,
                     'scan': scan,
                     'ntime': ntime,
                     'combinescans': False,
                     'extendpols': extendpols,
                     'growtime': 100.0,
                     'growfreq': 60.0,
                     'action': 'apply',
                     'display': '',
                     'extendflags': False,
                     'flagbackup': flagbackup,
                     'savepars': False}

        job = casa_tasks.flagdata(**task_args)

        self._executor.execute(job)

        return CheckflagResults([job])

    def _do_tfcropflag(self, mode='tfcrop', field=None, correlation=None, scan=None,
                      ntime=0.45, datacolumn='corrected', flagbackup=True):

        task_args = {'vis': self.inputs.vis,
                     'mode': mode,
                     'field': field,
                     'correlation': correlation,
                     'scan': scan,
                     'ntime': ntime,
                     'combinescans': False,
                     'datacolumn': datacolumn,
                     'freqcutoff': 3.0,
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