from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
from .. import common

LOG = infrastructure.get_logger(__name__)

import os
import asap as sd

class SDApplyCalInputs(common.SingleDishInputs):
    """
    Inputs for single dish calibraton
    """
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None,
                 infiles=None, outfile=None, field=None, iflist=None,
                 scanlist=None, pollist=None):
        self._init_properties(vars())
        self._to_list(['infiles', 'iflist', 'pollist'])

    @property
    def calto(self):
        if isinstance(self.infiles, list):
            calto_list = []
            for infile in self.infiles:
                basename = os.path.basename(infile.rstrip('/'))
                st = self.context.observing_run.get_scantable(basename)
                calto_list.append(callibrary.CalTo(vis=st.ms_name,
                                                   antenna=st.antenna.name))
            return calto_list
        basename = os.path.basename(self.infiles.rstrip('/'))
        st = self.context.observing_run.get_scantable(basename)
        return callibrary.CalTo(vis=st.ms_name,
                                antenna=st.antenna.name)

    @property
    def calstate(self):
        if isinstance(self.infiles, list):
            return [self.context.callibrary.get_calstate(calto)
                    for calto in self.calto]
        return self.context.callibrary.get_calstate(self.calto)
    

    def to_casa_args(self):
        args = super(SDApplyCalInputs,self).to_casa_args()

        # take iflist from observing_run (shouldbe ScantableList object)
        if len(args['iflist']) == 0:
            # filter out WVR
            args['iflist'] = self.context.observing_run.get_spw_without_wvr(args['infile'])

        # calmode is always 'apply'
        args['calmode'] = 'apply'
        
        # always overwrite existing data
        args['overwrite'] = True

        # output file
        if args['outfile'] is None or len(args['outfile']) == 0:
            suffix = '_applied'
            args['outfile'] = args['infile'].rstrip('/') + suffix

##         # iflist for tsys calibration
##         if args['ifmap'] is None or len(args['ifmap']) == 0:
##             if isinstance(args['infile'], list): 
##                 ifmap_list = []
##                 for infile in args['infile']:
##                     basename = os.path.basename(infile.rstrip('/'))
##                     st = self.context.observing_run.get_scantable(basename)
##                     tsys_transfer_list = st.tsys_transfer_list
##                     ifmap_list.append(self._generate_ifmap(tsys_transfer_list))
##                 args['ifmap'] = ifmap_list
##             else:
##                 basename = os.path.basename(args['infile'])
##                 st = self.context.observing_run.get_scantable(basename)
##                 tsys_transfer_list = st.tsys_transfer_list
##                 args['ifmap'] = self._generate_ifmap(tsys_transfer_list)
            
        return args

    def _generate_ifmap(self, transfer_list):
        d = {}
        for from_to in transfer_list:
            if d.has_key(from_to[0]):
                d[from_to[0]].append(from_to[1])
            else:
                d[from_to[0]] = from_to[1]
        return d


class SDApplyCalResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDApplyCalResults,self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDApplyCalResults,self).merge_with_context(context)
        calapp = self.outcome

        # merge with datatable, and export it
        filename = os.path.join(context.output_dir, calapp.infile)
        for _calfrom in calapp.calfrom:
            if _calfrom.caltype == 'tsys':
                tsystable = _calfrom.gaintable
                spwmap = _calfrom.spwmap
                datatable = context.observing_run.datatable_instance
                datatable._update_tsys(context, filename, tsystable, spwmap)

                # here, full export is necessary
                datatable.exportdata(minimal=False)
                break
                
        # applied caltables are marked as applied
        if calapp is not None:
            LOG.trace('Marking %s as applied' % calapp.as_applycal())
            context.callibrary.mark_as_applied(calapp.calto, calapp.calfrom)

        
    def _outcome_name(self):
        # usually, outcome is a name of the file
        return self.outcome.__str__()
    
class SDApplyCal(common.SingleDishTaskTemplate):
    Inputs = SDApplyCalInputs

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

            LOG.todo('logrecords for SDApplyCalResults must be handled properly')
            for r in result:
                r.logrecords = []

            return result

        # In the following, inputs.infiles should be a string,
        # not a list of string
        args = inputs.to_casa_args()

        # calstate
        calstate = inputs.calstate
        to_froms = calstate.merged(hide_empty=True)
        if len(to_froms) == 0:
            # Return empty Results object if calmode='none'
            LOG.info('No caltables exist for scantable %s'%(args['infile'])) 
            result = SDApplyCalResults(task=self.__class__,
                                       success=True,
                                       outcome=None)
        else:
            (calto, calfroms) = to_froms.items()[0]

            # antenna_name
            basename = os.path.basename(args['infile'].rstrip('/'))
            scantable = inputs.context.observing_run.get_scantable(basename)
            antenna_name = scantable.antenna.name

            # input file
            args['infile'] = os.path.join(inputs.output_dir, args['infile'])

            # output file
            #args['outfile'] = os.path.join(inputs.output_dir, args['outfile'])
            args['outfile'] = ''

            # print calmode
            LOG.info('calibration type is \'%s\' (type=%s)'%(args['calmode'],type(args['calmode'])))

            # take arguments from SDCalApplication object
            calapp = callibrary.SDCalApplication(calto, calfroms)
            args['ifmap'] = calapp.ifmap
            args['iflist'] = calapp.iflist
            args['scanlist'] = calapp.scanlist
            args['pollist'] = calapp.pollist
            #args['field'] = calapp.field.strip('"')
            args['applytable'] = [calfrom.gaintable for calfrom in calfroms]

            # create job
            job = casa_tasks.sdcal2(**args)

            # execute job
            self._executor.execute(job)

            basename = os.path.basename(args['infile'].rstrip('/'))
            scantable = inputs.context.observing_run.get_scantable(basename)

            # create result object
            result = SDApplyCalResults(task=self.__class__,
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

