from __future__ import absolute_import

import pipeline.infrastructure.api as api
#import pipeline.heuristics as heuristics
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.logging as logging
from pipeline.infrastructure.jobrequest import casa_tasks
#from . import common
from pipeline.hsd.tasks.common import common

LOG = logging.get_logger(__name__)

import os
import asap as sd

class SDCalibrationInputs(common.SingleDishInputs):
    """
    Inputs for single dish calibraton
    """
    def __init__(self, context, output_dir=None,
                 infiles=None, outfile=None, calmode=None, iflist=None,
                 scanlist=None, pollist=None):
        self._init_properties(vars())            

    def to_casa_args(self):
        args = super(SDCalibrationInputs,self).to_casa_args()

        # take iflist from observing_run (shouldbe ScantableList object)
        if len(args['iflist']) == 0:
            # filter out WVR
            args['iflist'] = self.context.observing_run.get_spw_without_wvr(args['infile'])

        # take calmode
        if args['calmode'] is None or args['calmode'].lower() == 'auto':
            args['calmode'] = self.context.observing_run.get_calmode(args['infile'])
        
        # always overwrite existing data
        args['overwrite'] = True

        # output file
        if args['outfile'] is None or len(args['outfile']) == 0:
            suffix = '_cal'
            args['outfile'] = args['infile'].rstrip('/') + suffix

        return args


class SDCalibrationResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDCalibrationResults,self).__init__(task, success, outcome)
    
class SDCalibration(common.SingleDishTaskTemplate):
    Inputs = SDCalibrationInputs

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
            return result

        # In the following, inputs.infiles should be a string,
        # not a list of string
        args = inputs.to_casa_args()
                
        # input file
        args['infile'] = os.path.join(inputs.output_dir, args['infile'])
        
        # output file
        args['outfile'] = os.path.join(inputs.output_dir, args['outfile'])

        # print calmode
        LOG.info('calibration type is \'%s\' (type=%s)'%(args['calmode'],type(args['calmode'])))
        
        # create job
        job = casa_tasks.sdcal(**args)

        # execute job
        self._executor.execute(job)

        # create result object
        result = SDCalibrationResults(success=True, outcome=args['outfile'])
        result.task = self.__class__
        if inputs.context.subtask_counter is 0: 
            result.stage_number = inputs.context.task_counter - 1
        else:
            result.stage_number = inputs.context.task_counter               

        return result

    def analyse(self, result):
        return result

