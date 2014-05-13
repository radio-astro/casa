from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
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
                 infiles=None, outfile=None, calmode=None, iflist=None,
                 scanlist=None, pollist=None):
        self._init_properties(vars())
        self._to_list(['infiles', 'iflist', 'pollist', 'scanlist'])

    def to_casa_args(self):
        args = super(SDCalTsysInputs,self).to_casa_args()

        # iflist for tsys calibration
        # inputs.iflist is mapped to tsysiflist
        # iflist is taken from observing_run
        if len(args['iflist']) == 0:
            ifset = set()
            st = self.context.observing_run.get_scantable(os.path.basename(self.infiles))
            if st.tsys_transfer is not False:
                for from_to in st.tsys_transfer_list:
                    ifset.add(from_to[0])
            args['tsysiflist'] = list(ifset)
        else:
            args['tsysiflist'] = args['iflist'][:]

        # filter out WVR
        args['iflist'] = self.context.observing_run.get_spw_for_caltsys(args['infile'])

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
            for r in result:
                r.logrecords = []

            return result

        # In the following, inputs.infiles should be a string,
        # not a list of string
        args = inputs.to_casa_args()

        if len(args['tsysiflist']) == 0:
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
            for from_to in scantable.tsys_transfer_list:
                if spwmap.has_key(from_to[0]):
                    spwmap[from_to[0]].append(from_to[1])
                else:
                    spwmap[from_to[0]] = [from_to[1]]

            # create CalTo object
            # CalTo object is created using associating MS name
            spw = callibrary.SDCalApplication.iflist_to_spw(args['iflist'])
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

