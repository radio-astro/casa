from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.sdfilenamer as filenamer
from .. import common

LOG = infrastructure.get_logger(__name__)


class SDSkyCalInputs(basetask.StandardInputs):
    def __init__(self, context, calmode=None, fraction=None, noff=None,
                 width=None, elongated=None, output_dir=None,
                 infiles=None, outfile=None, field=None,
                 spw=None, scan=None):
        self._init_properties(vars())
        #self.vis = self.infiles
        
    @property
    def infiles(self):
        return self.vis
    
    @infiles.setter
    def infiles(self, value):
        self.vis = value
        
    def to_casa_args(self):
        args = self._get_task_args()
        
        # if value is None, replace it with ''
        for s in ('spw', 'field', 'scan', 'infiles', 'outfile'):
            if args[s] is None:
                args[s] = ''

        # overwrite is always True
        args['overwrite'] = True
        
        # parameter name for input data is 'infile'
        args['infile'] = args.pop('infiles')
        
        # vis is not necessary
        del args['vis']
        
        return args
    
class SDSkyCalResults(common.SingleDishResults):
    """
    """
    def __init__(self, task=None, success=None, outcome=None):
        super(SDSkyCalResults, self).__init__(task, success, outcome)
        self.final = self.outcome

    def merge_with_context(self, context):
        super(SDSkyCalResults, self).merge_with_context(context)

        if self.outcome is None:
            return

        for calapp in self.outcome:
            context.callibrary.add(calapp.calto, calapp.calfrom)

    def _outcome_name(self):
        return str(self.outcome)


class SDSkyCal(basetask.StandardTaskTemplate):
    Inputs = SDSkyCalInputs

    def prepare(self):
        args = self.inputs.to_casa_args()
        LOG.trace('args: %s' % args)
        
        # retrieve ms domain object
        ms = self.inputs.ms
        calibration_strategy = ms.calibration_strategy
        default_field_strategy = calibration_strategy['field_strategy']
        
        # take calmode from calibration strategy if it is set to 'auto'
        if args['calmode'] is None or args['calmode'].lower() == 'auto':
            args['calmode'] = calibration_strategy['calmode']
            
        # spw selection ---> task.prepare
        if args['spw'] is None or len(args['spw']) == 0:
            spw_list = ms.get_spectral_windows(science_windows_only=True)
            args['spw'] = ','.join(map(str, [spw.id for spw in spw_list]))
        
        # field selection ---> task.prepare
        if args['field'] is None or len(args['field']) == 0:
            field_strategy = default_field_strategy
        else:
            field_strategy = {}
            field_ids = casatools.ms.msseltoindex(vis=ms.name, field=args['field'])
            for field_id in field_ids:
                for target_id, reference_id in default_field_strategy.iteritems():
                    if field_id == target_id:
                        field_strategy[field_id] = default_field_strategy[field_id]
                        continue
                    elif field_id == reference_id:
                        field_strategy[target_id] = field_id
                        continue
                        
        # scan selection 
        if args['scan'] is None:
            args['scan'] = ''
            
        calapps = []
        for (target_id, reference_id) in field_strategy.iteritems():
            myargs = args.copy()
            
            # output file
            reference_field_name = ms.get_fields(reference_id)[0].clean_name
            if myargs['outfile'] is None or len(myargs['outfile']) == 0:
                namer = filenamer.SkyCalibrationTable()
                # caltable name should be <ASDM uid>.ms.<FIELD>.skycal.tbl
                #asdm = common.asdm_name_from_ms(ms)
                asdm = ms.basename
                namer.asdm(asdm)
                namer.field(reference_field_name)
                myargs['outfile'] = os.path.join(self.inputs.output_dir, namer.get_filename())
            else:
                myargs['outfile'] = myargs['outfile'] + '.%s'%(reference_field_name)
                
            # field
            myargs['field'] = str(reference_id)
                
            LOG.debug('args for tsdcal: %s'%(myargs))
    
            # create job
            job = casa_tasks.sdcal(**myargs)
    
            # execute job
            self._executor.execute(job)
    
            # make a note of the current inputs state before we start fiddling
            # with it. This origin will be attached to the final CalApplication.
            origin = callibrary.CalAppOrigin(task=SDSkyCal,
                                             inputs=args)
            
            calto = callibrary.CalTo(vis=myargs['infile'],
                                     spw=myargs['spw'],
                                     field=str(target_id),
                                     intent='TARGET')
    
            # create SDCalFrom object
            calfrom = callibrary.CalFrom(gaintable=myargs['outfile'],
                                         gainfield=str(reference_id),
                                         interp='linear,linear',
                                         caltype=myargs['calmode'])

            # create CalApplication object
            calapp = callibrary.CalApplication(calto, calfrom, origin)
            calapps.append(calapp)
        
        results = SDSkyCalResults(task=self.__class__,
                                    success=True,
                                    outcome=calapps)
        return results
    
    def analyse(self, result):
        return result
