from __future__ import absolute_import

import os
import types
import abc

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
import pipeline.infrastructure.sessionutils as sessionutils
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.sdfilenamer as filenamer
from pipeline.hsd.heuristics import CubicSplineFitParamConfig
from pipeline.domain import DataTable
from .. import common
from . import plotter
from pipeline.hsd.tasks.common import utils as sdutils
from pipeline.infrastructure import casa_tasks

_LOG = infrastructure.get_logger(__name__)
LOG = sdutils.OnDemandStringParseLogger(_LOG)


class BaselineSubtractionInputsBase(vdp.StandardInputs):
    DATACOLUMN = {'CORRECTED_DATA': 'corrected',
                  'DATA': 'data',
                  'FLOAT_DATA': 'float_data'}

    @vdp.VisDependentProperty
    def colname(self):
        colname = ''
        if type(self.vis) is types.StringType:
            with casatools.TableReader(self.vis) as tb:
                candidate_names = ['CORRECTED_DATA', 
                                   'DATA',
                                   'FLOAT_DATA']
                for name in candidate_names:
                    if name in tb.colnames():
                        colname = name
                        break
        return colname

    def to_casa_args(self):
        args = super(BaselineSubtractionInputsBase, self).to_casa_args()#{'vis': self.vis}
        prefix = os.path.basename(self.vis.rstrip('/'))
        
        # blparam 
        if self.blparam is None or len(self.blparam) == 0:
            args['blparam'] = prefix + '_blparam.txt'
        else:
            args['blparam'] = self.blparam
            
        # baseline caltable filename
        if self.bloutput is None or len(self.bloutput) == 0:
            namer = filenamer.BaselineSubtractedTable()
            #namer.spectral_window(self.spwid)
            #st = self.data_object()
            #asdm = common.asdm_name(st)
            asdm = prefix
            namer.asdm(asdm)
            #namer.antenna_name(st.antenna.name)
            bloutput = namer.get_filename() 
            args['bloutput'] = bloutput
        else:
            args['bloutput'] = self.bloutput
            
        # outfile
        if ('outfile' not in args or
                args['outfile'] is None or
                len(args['outfile']) == 0):
            args['outfile'] = self.vis.rstrip('/') + '_bl'
            
        args['datacolumn'] = self.DATACOLUMN[self.colname]
            
        return args
        
    
class BaselineSubtractionResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(BaselineSubtractionResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(BaselineSubtractionResults, self).merge_with_context(context)
    
    def _outcome_name(self):
        # outcome should be a name of blparam text file
        return 'blparam: "%s" bloutput: "%s"' % (self.outcome['blparam'], self.outcome['bloutput'])


class BaselineSubtractionWorkerInputs(BaselineSubtractionInputsBase):
    vis = vdp.VisDependentProperty(default='', null_input=['', None, [], ['']])
    plan = vdp.VisDependentProperty(default=None)
    fit_order = vdp.VisDependentProperty(default='automatic')
    edge = vdp.VisDependentProperty(default=(0,0))
    deviationmask = vdp.VisDependentProperty(default={})
        
    @vdp.VisDependentProperty
    def prefix(self):
        return os.path.basename(self.vis.rstrip('/'))
    
    @vdp.VisDependentProperty
    def blparam(self):
        return self.prefix + '_blparam.txt'
    
    @vdp.VisDependentProperty
    def bloutput(self):
        namer = filenamer.BaselineSubtractedTable()
        #namer.spectral_window(self.spwid)
        #st = self.data_object()
        #asdm = common.asdm_name(st)
        asdm = self.prefix
        namer.asdm(asdm)
        #namer.antenna_name(st.antenna.name)
        return namer.get_filename()
    
    @vdp.VisDependentProperty(readonly=True)
    def field(self):
        return self.plan.get_field_id_list()
    
    @vdp.VisDependentProperty(readonly=True)
    def antenna(self):
        return self.plan.get_antenna_id_list()
    
    @vdp.VisDependentProperty(readonly=True)
    def spw(self):
        return self.plan.get_spw_id_list()
    
    @vdp.VisDependentProperty(readonly=True)
    def grid_table(self):
        return self.plan.get_grid_table_list()
    
    @vdp.VisDependentProperty(readonly=True)
    def channelmap_range(self):
        return self.plan.get_channelmap_range_list()
        
    def __init__(self, context, vis=None, plan=None,
                 fit_order=None, edge=None, deviationmask=None, blparam=None, bloutput=None):
        super(BaselineSubtractionWorkerInputs, self).__init__()
        
        self.context = context
        self.vis = vis
        self.plan = plan
        self.fit_order = fit_order
        self.edge = edge
        self.deviationmask = deviationmask
        self.blparam = blparam
        self.bloutput = bloutput
        

# Base class for workers
class BaselineSubtractionWorker(basetask.StandardTaskTemplate):
    Inputs = BaselineSubtractionWorkerInputs
    
    @abc.abstractproperty
    def Heuristics(self):
        """
        A reference to the :class:`Heuristics` class.
        """
        raise NotImplementedError
        
    
    is_multi_vis_task = False
    
    def __init__(self, inputs):
        super(BaselineSubtractionWorker, self).__init__(inputs)
        
        # initialize plotter
        self.datatable = DataTable(os.path.join(self.inputs.context.observing_run.ms_datatable_name,
                                                self.inputs.ms.basename))
    
    def prepare(self):
        vis = self.inputs.vis
        ms = self.inputs.ms
        fit_order = self.inputs.fit_order
        edge = self.inputs.edge
        args = self.inputs.to_casa_args()
        blparam = args['blparam']
        bloutput = args['bloutput']
        outfile = args['outfile']
        datacolumn = args['datacolumn']
        
        process_list = self.inputs.plan
        deviationmask_list = self.inputs.deviationmask
        LOG.info('deviationmask_list={}'.format(deviationmask_list))      
        
        field_id_list = self.inputs.field
        antenna_id_list = self.inputs.antenna
        spw_id_list = self.inputs.spw
        LOG.debug('subgroup member for {vis}:\n\tfield: {field}\n\tantenna: {antenna}\n\tspw: {spw}',
                  vis=ms.basename,
                  field=field_id_list,
                  antenna=antenna_id_list,
                  spw=spw_id_list)

        # initialization of blparam file
        # blparam file needs to be removed before starting iteration through 
        # reduction group
        if os.path.exists(blparam):
            LOG.debug('Cleaning up blparam file for {vis}', vis=vis)
            os.remove(blparam)        
        
        #datatable = DataTable(context.observing_run.ms_datatable_name)

        for (field_id, antenna_id, spw_id) in process_list.iterate_id():
            if (field_id, antenna_id, spw_id) in deviationmask_list:
                deviationmask = deviationmask_list[(field_id, antenna_id, spw_id)]
            else:
                deviationmask = None
            blparam_heuristic = self.Heuristics()
            formatted_edge = list(common.parseEdge(edge))
            out_blparam = blparam_heuristic(self.datatable, ms, antenna_id, field_id, spw_id, 
                                            fit_order, formatted_edge, deviationmask, blparam)
            assert out_blparam == blparam
            
        # execute sdbaseline
        job = casa_tasks.sdbaseline(infile=vis, datacolumn=datacolumn, blmode='fit', dosubtract=True,
                                    blformat='table', bloutput=bloutput,
                                    blfunc='variable', blparam=blparam,
                                    outfile=outfile, overwrite=True)
        self._executor.execute(job)
            
        outcome = {'infile': vis,
                   'blparam': blparam,
                   'bloutput': bloutput,
                   'outfile': outfile}
        results = BaselineSubtractionResults(success=True, outcome=outcome)
        return results
        
    def analyse(self, results):
        # plot             
        # initialize plot manager
        plot_manager = plotter.BaselineSubtractionPlotManager(self.inputs.context, self.datatable) 
        outfile = results.outcome['outfile']
        ms = self.inputs.ms
        accum = self.inputs.plan
        deviationmask_list = self.inputs.deviationmask 
        LOG.info('deviationmask_list={}'.format(deviationmask_list))      
        status = plot_manager.initialize(ms, outfile)
        plot_list = []
        for (field_id, antenna_id, spw_id, grid_table, channelmap_range) in accum.iterate_all():
             
            LOG.info('field {0} antenna {1} spw {2}', field_id, antenna_id, spw_id)
            if (field_id, antenna_id, spw_id) in deviationmask_list:
                deviationmask = deviationmask_list[(field_id, antenna_id, spw_id)]
            else:
                deviationmask = None
             
            if status:
                plot_list.extend(plot_manager.plot_spectra_with_fit(field_id, antenna_id, spw_id, 
                                                                    grid_table, 
                                                                    deviationmask, channelmap_range))
        plot_manager.finalize()

        results.outcome['plot_list'] = plot_list
        return results
                

# Worker class for cubic spline fit
class CubicSplineBaselineSubtractionWorker(BaselineSubtractionWorker):
    Inputs = BaselineSubtractionWorkerInputs
    Heuristics = CubicSplineFitParamConfig


### Tier-0 Parallelization
class HpcBaselineSubtractionWorkerInputs(BaselineSubtractionWorkerInputs):
    # use common implementation for parallel inputs argument
    parallel = sessionutils.parallel_inputs_impl()
    
    def __init__(self, context, vis=None, plan=None,
                 fit_order=None, edge=None, deviationmask=None, blparam=None, bloutput=None,
                 parallel=None):
        super(HpcBaselineSubtractionWorkerInputs, self).__init__(context, vis=vis, plan=plan, 
                                                                 fit_order=fit_order, edge=edge,
                                                                 deviationmask=deviationmask, 
                                                                 blparam=blparam, bloutput=bloutput)
        self.parallel = parallel


# This is abstract class since Task is not specified yet
class HpcBaselineSubtractionWorker(sessionutils.ParallelTemplate):
    Inputs = HpcBaselineSubtractionWorkerInputs
        
    def __init__(self, inputs):
        super(HpcBaselineSubtractionWorker, self).__init__(inputs)
    
    @basetask.result_finaliser
    def get_result_for_exception(self, vis, exception):
        LOG.error('Error operating target flag for {!s}'.format(os.path.basename(vis)))
        LOG.error('{0}({1})'.format(exception.__class__.__name__, exception.message))
        import traceback
        tb = traceback.format_exc()
        if tb.startswith('None'):
            tb = '{0}({1})'.format(exception.__class__.__name__, exception.message)
        return basetask.FailedTaskResults(self, exception, tb)


class HpcCubicSplineBaselineSubtractionWorker(HpcBaselineSubtractionWorker):
    Task = CubicSplineBaselineSubtractionWorker
    
    def __init__(self, inputs):
        super(HpcCubicSplineBaselineSubtractionWorker, self).__init__(inputs)
    

# # facade for FitParam
# class BaselineSubtractionInputs(vdp.ModeInputs):
#     _modes = {'spline': CubicSplineBaselineSubtractionWorker,
#               'cspline': CubicSplineBaselineSubtractionWorker}
# 
#     def __init__(self, context, fitfunc, **parameters):
#         super(BaselineSubtractionInputs, self).__init__(context=context, mode=fitfunc, **parameters)
#     
# 
# class BaselineSubtractionTask(basetask.ModeTask):
#     Inputs = BaselineSubtractionInputs
