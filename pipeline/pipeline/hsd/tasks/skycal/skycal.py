from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.sdfilenamer as filenamer
from .. import common

LOG = infrastructure.get_logger(__name__)

class SDMSSkyCalInputs(basetask.StandardInputs):
    """
    """
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
        if args['spw'] is None:
            args['spw']= ''
            
        if args['field'] is None:
            args['field'] = ''
            
        if args['scan'] is None:
            args['scan'] = ''
            
        if args['infiles'] is None:
            args['infiles'] = ''    
        
        if args['outfile'] is None:
            args['outfile'] = ''
            
        # overwrite is always True
        args['overwrite'] = True
        
        # parameter name for input data is 'infile'
        assert args.has_key('infiles')
        args['infile'] = args.pop('infiles')
        
        # vis is not necessary
        args.pop('vis')
        
        return args
    
class SDMSSkyCalResults(common.SingleDishResults):
    """
    """
    def __init__(self, task=None, success=None, outcome=None):
        super(SDMSSkyCalResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDMSSkyCalResults, self).merge_with_context(context)
        calapp = self.outcome
        if calapp is not None:
            context.callibrary.add(calapp.calto, calapp.calfrom)
        
    def _outcome_name(self):
        return str(self.outcome)
        
class SDMSSkyCal(basetask.StandardTaskTemplate):
    Inputs = SDMSSkyCalInputs
    
    def prepare(self):
        args = self.inputs.to_casa_args()
        LOG.trace('args: %s'%(args))
        
        # retrieve ms domain object
        ms = self.inputs.ms
        
        # take calmode from calibration strategy if it is set to 'auto'
        if args['calmode'] is None or args['calmode'].lower() == 'auto':
            args['calmode'] = ms.calibration_strategy['calmode']
            
        # spw selection ---> task.prepare
        if args['spw'] is None or len(args['spw']) == 0:
            spw_list = ms.get_spectral_windows(science_windows_only=True)
            args['spw'] = ','.join(map(str, [spw.id for spw in spw_list]))
        
        # field selection ---> task.prepare
        if args['field'] is None or len(args['field']) == 0:
            field_list=ms.get_fields(intent='TARGET')
            args['field'] = ','.join([field.name for field in field_list])
            
        # scan selection 
        if args['scan'] is None:
            args['scan'] = ''
            
            
        # output file
        if args['outfile'] is None or len(args['outfile']) == 0:
            namer = filenamer.SkyCalibrationTable()
            asdm = common.asdm_name_from_ms(ms)
            namer.asdm(asdm)
            args['outfile'] = os.path.join(self.inputs.output_dir, namer.get_filename())
            
        LOG.trace('args for tsdcal: %s'%(args))

        # create job
        job = casa_tasks.tsdcal(**args)

        # execute job
        self._executor.execute(job)

        calto = callibrary.CalTo(vis=args['infile'],
                                 spw=args['spw'],
                                 field=args['field'],
                                 intent='TARGET,REFERENCE')

        # create SDCalFrom object
        calfrom = callibrary.CalFrom(gaintable=args['outfile'],
                                     gainfield=args['field'],
                                     interp='',
                                     caltype=args['calmode'])

        # make a note of the current inputs state before we start fiddling
        # with it. This origin will be attached to the final CalApplication.
        origin = callibrary.CalAppOrigin(task=SDMSSkyCal, 
                                         inputs=args)

        # create CalApplication object
        calapp = callibrary.CalApplication(calto, calfrom, origin)
        
        results = SDMSSkyCalResults(task=self.__class__,
                                    success=True,
                                    outcome=calapp)
        return results
    
    def analyse(self, result):
        return result