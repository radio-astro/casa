from __future__ import absolute_import

import types

import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.contfilehandler as contfilehandler

LOG = infrastructure.get_logger(__name__)

# CHECKING FLAGGING OF ALL CALIBRATORS
# use rflag mode of flagdata

def read_contfile(contfile):
    '''
    Read in the spws/frequencies into a dictionary
    '''

    #First check to see if the file exists

    contfile_handler = contfilehandler.ContFileHandler(contfile)
    contdict = contfile_handler.read()

    return contdict


class TargetflagInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None, intents=None, contfile=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

    @property
    def intents(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('intents')

        if self._intents is not None:
            return self._intents

        # return just the unwanted intents that are present in the MS
        # intents_to_flag = set(['POINTING','FOCUS','ATMOSPHERE','SIDEBAND','UNKNOWN', 'SYSTEM_CONFIGURATION'])
        return ''

    @intents.setter
    def intents(self, value):
        self._intents = value

    @property
    def contfile(self):

        if self._contfile is not None:
            return self._contfile

        return ''

    @contfile.setter
    def contfile(self, value):
        self._contfile = value


class TargetflagResults(basetask.Results):
    def __init__(self, jobs=[]):
        super(TargetflagResults, self).__init__()

        self.jobs=jobs
        
    def __repr__(self):
        s = 'Targetflag (rflag mode) results:\n'
        for job in self.jobs:
            s += '%s performed. Statistics to follow?' % str(job)
        return s 


class Targetflag(basetask.StandardTaskTemplate):
    Inputs = TargetflagInputs
    
    def prepare(self):
        
        # Default values
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        # corrstring = self.inputs.context.evla['msinfo'][m.name].corrstring
        corrstring = m.get_vla_corrstring()


        if ('CALIBRATE' in self.inputs.intents):
            LOG.info("TARGETFLAG INFO: DOING CONTINUUM CALIBRATE")
            method_args = {'field'       : '',
                           'correlation' : 'ABS_' + corrstring,
                           'scan'        : '',
                           'intent'      : '*CALIBRATE*',
                           'spw'         : ''}

            rflag_result = self._do_rflag(**method_args)

        if ('TARGET' in self.inputs.intents and self.inputs.contfile == ''):
            LOG.info("TARGETFLAG INFO:  DOING CONTINUUM TARGET")
            method_args = {'field'       : '',
                           'correlation' : 'ABS_' + corrstring,
                           'scan'        : '',
                           'intent'      : '*TARGET*',
                           'spw'         : ''}

            rflag_result = self._do_rflag(**method_args)

        if (self.inputs.intents == '' and self.inputs.contfile == ''):
            LOG.info("TARGETFLAG INFO:  DOING EVERYTHING")
            method_args = {'field'       : '',
                           'correlation' : 'ABS_' + corrstring,
                           'scan'        : '',
                           'intent'      : '',
                           'spw'         : ''}

            rflag_result = self._do_rflag(**method_args)

            return TargetflagResults([rflag_result])

        if ('TARGET' in self.inputs.intents and self.inputs.contfile):
            LOG.info("TARGETFLAG INFO:  SPECTRAL LINE - CONTINUUM MASKING")
            fielddict = self._cont_file_to_CASA()

            for field in fielddict.keys():
                method_args = {'field'       : field,
                               'correlation' : 'ABS_' + corrstring,
                               'scan'        : '',
                               'intent'      : '*TARGET*',
                               'spw'         : fielddict[field]}

                rflag_result = self._do_rflag(**method_args)


        return TargetflagResults([rflag_result])
    
    def analyse(self, results):
        return results
    
    def _do_rflag(self, field=None, correlation=None, scan=None, intent=None, spw=None):
        
        task_args = {'vis'          : self.inputs.vis,
                     'mode'         : 'rflag',
                     'field'        : field,
                     'correlation'  : correlation,
                     'scan'         : scan,
                     'intent'       : intent,
                     'spw'          : spw,
                     'ntime'        : 'scan',
                     'combinescans' : False,
                     'datacolumn'   : 'corrected',
                     'winsize'      : 3,
                     'timedevscale' : 4.0,
                     'freqdevscale' : 4.0,
                     'action'       : 'apply',
                     'display'      : '',
                     'extendflags'  : False,
                     'flagbackup'   : False,
                     'savepars'     : True}
                     
        job = casa_tasks.flagdata(**task_args)
            
        return self._executor.execute(job)



    def _cont_file_to_CASA(self):
        '''
        Take the dictionary created by _read_cont_file and put it into the format:
        spw = '0:1.380~1.385GHz;1.390~1.400GHz'
        '''

        contdict = read_contfile(self.inputs.contfile)
        if contdict == {}:
            LOG.error(self.inputs.contfile + " is empty, does not exist or cannot be read.")
            return {}

        fielddict = {}

        for field in contdict.keys():
            spwstring = ''
            for spw in contdict[field].keys():
                spwstring = spwstring + spw + ':'
                for freqtuple in contdict[field][spw]:
                    spwstring = spwstring + str(freqtuple[0]) + '~' + str(freqtuple[1]) + 'GHz;'
                spwstring = spwstring[:-1]
                spwstring = spwstring + ','
            spwstring = spwstring[:-1]
            fielddict[field] = spwstring

        return fielddict