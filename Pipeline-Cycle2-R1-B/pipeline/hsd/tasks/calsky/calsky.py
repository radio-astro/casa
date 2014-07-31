from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.sdfilenamer as filenamer
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
from .. import common

LOG = infrastructure.get_logger(__name__)

class SDCalSkyInputs(common.SingleDishInputs):
    """
    Inputs for single dish calibraton
    """
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, calmode=None, fraction=None, noff=None,
                 width=None, elongated=None, output_dir=None,
                 infiles=None, outfile=None, field=None,
                 spw=None, scan=None, pol=None):
        self._init_properties(vars())
        self._to_list(['infiles'])
        for key in ['spw', 'scan', 'pol']:
            val = getattr(self, key)
            if val is None or (val[0] == '[' and val[-1] == ']'):
                self._to_list([key])
        self._to_bool(['elongated'])
        if isinstance(self.fraction, str) and self.fraction.endswith('%'):
            self._to_numeric(['noff', 'width'])
        else:
            self._to_numeric(['fraction', 'noff', 'width'])
        if isinstance(self.noff, float):
            self.noff = int(self.noff)

    def to_casa_args(self):
        args = super(SDCalSkyInputs,self).to_casa_args()

        index = self.context.observing_run.st_names.index(os.path.basename(args['infile']))

        # take spw from observing_run (shouldbe ScantableList object)
        args['spw'] = self._to_casa_arg(args['spw'], index)
        if len(args['spw']) == 0:
            # only science spws
            science_spw = self.context.observing_run.get_spw_for_science(args['infile'])
            args['spw'] = ','.join(map(str,science_spw))
        
        # scan
        args['scan'] = self._to_casa_arg(args['scan'], index)
            
        # pol
        args['pol'] = self._to_casa_arg(args['pol'], index)

        # take calmode
        if args['calmode'] is None or args['calmode'].lower() == 'auto':
            args['calmode'] = self.context.observing_run.get_calmode(args['infile'])
        
        # always overwrite existing data
        args['overwrite'] = True

        # output file
        if args['outfile'] is None or len(args['outfile']) == 0:
            namer = filenamer.SkyCalibrationTable()
            st = self.context.observing_run.get_scantable(args['infile'])
            asdm = common.asdm_name(st)
            namer.asdm(asdm)
            namer.antenna_name(st.antenna.name)
            args['outfile'] = namer.get_filename()

        return args


class SDCalSkyResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDCalSkyResults,self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDCalSkyResults,self).merge_with_context(context)
        calapp = self.outcome
        if calapp is not None:
            context.callibrary.add(calapp.calto, calapp.calfrom)
        
    def _outcome_name(self):
        # usually, outcome is a name of the file
        return self.outcome.__str__()
    
class SDCalSky(common.SingleDishTaskTemplate):
    Inputs = SDCalSkyInputs

    @common.datatable_setter
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
            LOG.todo('logrecords for SDCalSKyResults must be handled properly')
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
        
        if args['calmode'] == 'none':
            # Return empty Results object if calmode='none'
            LOG.info('Calibration is already done for scantable %s'%(args['infile'])) 
            result = SDCalSkyResults(task=self.__class__,
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

            # create CalTo object
            # CalTo object is created using associating MS name
            basename = os.path.basename(args['infile'].rstrip('/'))
            scantable = inputs.context.observing_run.get_scantable(basename)
            #spw = callibrary.SDCalApplication.iflist_to_spw(args['iflist'])
            spw = args['spw']
            calto = callibrary.CalTo(vis=scantable.ms_name,
                                     spw=spw,
                                     antenna=scantable.antenna.name,
                                     intent='TARGET,REFERENCE')

            # create SDCalFrom object
            calfrom = callibrary.SDCalFrom(gaintable=args['outfile'],
                                           interp='',
                                           caltype=args['calmode'])

            # create SDCalApplication object
            calapp = callibrary.SDCalApplication(calto, calfrom)

            # create result object
            result = SDCalSkyResults(task=self.__class__,
                                     success=True,
                                     outcome=calapp)
        result.task = self.__class__

        result.stage_number = inputs.context.task_counter               

        return result

    def analyse(self, result):
        return result

