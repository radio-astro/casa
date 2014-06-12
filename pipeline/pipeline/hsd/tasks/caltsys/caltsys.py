from __future__ import absolute_import

import os
import re
import numpy

from asap._asap import srctype 

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.sdfilenamer as filenamer
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
from .. import common

LOG = infrastructure.get_logger(__name__)

class SDCalTsysInputs(common.SingleDishInputs):
    """
    Inputs for single dish calibraton
    """
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None,
                 infiles=None, outfile=None, calmode=None, field=None,
                 spw=None, scan=None, pol=None):
        self._init_properties(vars())
        self._to_list(['infiles'])
        for key in ['spw', 'scan', 'pol']:
            val = getattr(self, key)
            if val is None or (val[0] == '[' and val[-1] == ']'):
                self._to_list([key])

    def to_casa_args(self):
        args = super(SDCalTsysInputs,self).to_casa_args()

        index = self.context.observing_run.st_names.index(os.path.basename(args['infile']))
        LOG.debug('index=%s'%(index))

        # iflist for tsys calibration
        # inputs.spw is mapped to tsysspw
        # spw is taken from observing_run
        tsysspw = self._to_casa_arg(args['spw'], index)
        if len(tsysspw) == 0:
            ifset = set()
            st = self.context.observing_run.get_scantable(os.path.basename(args['infile']))
            if st.tsys_transfer is not False:
                for from_to in st.tsys_transfer_list:
                    ifset.add(from_to[0])
            args['tsysspw'] = common.list_to_selection(list(ifset))
        else:
            args['tsysspw'] = tsysspw

        # check if any channels are flagged or not
        # flagged_channels {spwid: 'ch0~1,...'}
        tsysspw_list = common.selection_to_list(args['tsysspw'])
        flagged_channels = _flagged_channels(args['infile'], tsysspw_list)
        LOG.debug('flagged_channels=%s'%(flagged_channels))

        args['tsysavg'] = len(flagged_channels) > 0

        if args['tsysavg'] == True:
            args['tsysspw'] = _spw_with_channelrange(tsysspw_list, flagged_channels)
            LOG.debug('Performing spectral averaging of Tsys with channel range %s'%(args['tsysspw']))

        # filter out WVR
        spw = self.context.observing_run.get_spw_for_caltsys(args['infile'])
        args['spw'] = ','.join(map(str,spw))

        # scan
        args['scan'] = self._to_casa_arg(args['scan'], index)
            
        # pol
        args['pol'] = self._to_casa_arg(args['pol'], index)

        # take calmode
        if args['calmode'] is None or args['calmode'].lower() == 'auto':
            args['calmode'] = 'tsys'
        
        # always overwrite existing data
        args['overwrite'] = True

        # output file
        if args['outfile'] is None or len(args['outfile']) == 0:
            namer = filenamer.TsysCalibrationTable()
            st = self.context.observing_run.get_scantable(args['infile'])
            asdm = common.asdm_name(st)
            namer.asdm(asdm)
            namer.antenna_name(st.antenna.name)
            args['outfile'] = namer.get_filename()
            
        return args

def _flagged_channels(infile, spwid_list):
    flagged_channels = {}
    atmcal_intents = map(int,[getattr(srctype, s) for s in dir(srctype) if re.search('cal$', s)])
    with casatools.TableReader(infile) as tb:
        for spwid in spwid_list:
            tsel = tb.query('IFNO==%s && SRCTYPE IN %s'%(spwid,atmcal_intents))
            flags = tsel.getcol('FLAGTRA')
            tsel.close()
            if len(flags) == 0:
                continue

            net_flag = flags.sum(axis=1)
            nchan = len(net_flag)
            valid_channels = numpy.where(net_flag == 0)[0]
            if len(valid_channels) == 0:
                LOG.critical('All data are flagged for Tsys spw: %s'%(spwid))
            elif len(valid_channels) < nchan:
                flagged_channels[spwid] = (common.list_to_selection(valid_channels)).replace(',',';')

    return flagged_channels

def _spw_with_channelrange(spwid_list, flagged_channels):
    def g(spwid_list, flagged_channels):
        for spwid in spwid_list:
            if flagged_channels.has_key(spwid):
                yield '%s:%s'%(spwid,flagged_channels[spwid])
            else:
                yield '%s'%(spwid)
    return ','.join(g(spwid_list, flagged_channels))

class SDCalTsysResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDCalTsysResults,self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDCalTsysResults,self).merge_with_context(context)
        calapp = self.outcome
        if calapp is not None:
            context.callibrary.add(calapp.calto, calapp.calfrom)

    def _outcome_name(self):
        # usually, outcome is a name of the file
        return self.outcome.__str__()
    
class SDCalTsys(common.SingleDishTaskTemplate):
    Inputs = SDCalTsysInputs

    def prepare(self):
        # inputs
        inputs = self.inputs

        # if infiles is a list, call prepare for each element
        if isinstance(inputs.infiles, list):
            result = basetask.ResultsList()
            infiles = inputs.infiles[:]
            for infile in infiles:
                inputs.infiles = infile
                result.append(self.prepare())
            # do I need to restore self.inputs.infiles?
            inputs.infiles = infiles[:]

            LOG.todo('logrecords for SDCalTsysResults must be handled properly')
            # only add logrecords to first result
            logrecords = common._collect_logrecords(LOG)
            if len(result) > 0:
                result[0].logrecords = logrecords
                for r in result[1:]:
                    r.logrecords = []

            return result

        # In the following, inputs.infiles should be a string,
        # not a list of string
        args = inputs.to_casa_args()

        #if len(args['tsysiflist']) == 0:
        if len(args['tsysspw']) == 0:
            # Return empty Results object if calmode='none'
            LOG.info('Tsys transfer is not needed for scantable %s'%(args['infile'])) 
            result = SDCalTsysResults(task=self.__class__,
                                      success=True,
                                      outcome=None)
        else:                
            # input file
            args['infile'] = os.path.join(inputs.output_dir, args['infile'])

            # output file
            args['outfile'] = os.path.join(inputs.output_dir, args['outfile'])

            # print calmode
            LOG.info('calibration type is \'%s\' (type=%s)'%(args['calmode'],type(args['calmode'])))

            # create job
            job = casa_tasks.sdcal2(**args)

            # execute job
            self._executor.execute(job)

            basename = os.path.basename(args['infile'].rstrip('/'))
            scantable = inputs.context.observing_run.get_scantable(basename)
            spwmap = {}
            tsysspw_list = common.selection_to_list(args['tsysspw'])
            spw_list = common.selection_to_list(args['spw'])
            for from_to in scantable.tsys_transfer_list:
                if from_to[0] in tsysspw_list and from_to[1] in spw_list:
                    if spwmap.has_key(from_to[0]):
                        spwmap[from_to[0]].append(from_to[1])
                    else:
                        spwmap[from_to[0]] = [from_to[1]]

            # create CalTo object
            # CalTo object is created using associating MS name
            spw = args['spw']
            calto = callibrary.CalTo(vis=scantable.ms_name,
                                     spw=spw,
                                     antenna=scantable.antenna.name,
                                     intent='TARGET,REFERENCE')

            # create SDCalFrom object
            # need to set spwmap here
            calfrom = callibrary.SDCalFrom(gaintable=args['outfile'],
                                           interp='',
                                           spwmap=spwmap,
                                           caltype='tsys')

            # create SDCalApplication object
            calapp = callibrary.SDCalApplication(calto, calfrom)

            # create result object
            result = SDCalTsysResults(task=self.__class__,
                                      success=True,
                                      outcome=calapp)
        result.task = self.__class__
        
        if inputs.context.subtask_counter is 0: 
            result.stage_number = inputs.context.task_counter - 1
        else:
            result.stage_number = inputs.context.task_counter               

        return result

    def analyse(self, result):
        return result

