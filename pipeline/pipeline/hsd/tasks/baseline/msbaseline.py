from __future__ import absolute_import

import os
import shutil
import glob
import numpy
import collections
import types
import itertools

#import memory_profiler

import pipeline.infrastructure.mpihelpers as mpihelpers
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
from pipeline.hsd.heuristics import MaskDeviationHeuristic
from pipeline.domain import DataTable

from .. import common
from ..common import utils
from . import maskline
from . import worker
from . import plotter
# from . import fitting
from ..common import compress

_LOG = infrastructure.get_logger(__name__)
LOG = utils.OnDemandStringParseLogger(_LOG)

class SDMSBaselineInputs(basetask.StandardInputs):
    """
    Inputs for baseline subtraction
    """
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, infiles=None, antenna=None, spw=None, pol=None, field=None,
                 linewindow=None, edge=None, broadline=None, fitorder=None,
                 fitfunc=None, clusteringalgorithm=None, deviationmask=None):
        vis = infiles
        self._init_properties(vars())
        # LOG.debug('attributes summary:')
        # for (k,v) in self.__dict__.iteritems():
        #    LOG.debug('\t{key}={value}'.format(key=k, value=v))
            
    def to_casa_args(self):
        vis = self.vis
        if type(self.vis) is types.ListType:
            self.vis = vis[0]
        args = super(SDMSBaselineInputs, self).to_casa_args()
        self.vis = vis
        
        if not args.has_key('antenna'):
            args['antenna'] = ''
        return args

    spw = basetask.property_with_default('spw', '')
    pol = basetask.property_with_default('pol', '')
    field = basetask.property_with_default('field', '')
    linewindow = basetask.property_with_default('linewindow', [])
    edge = basetask.property_with_default('edge', (0, 0))
    broadline = basetask.property_with_default('broadline', True)
    fitorder = basetask.property_with_default('fitorder', 'automatic')
    fitfunc = basetask.property_with_default('fitfunc', 'cspline')
    clusteringalgorithm = basetask.property_with_default('clusteringalgorithm', 'kmean')
    deviationmask = basetask.property_with_default('deviationmask', True)
            

class SDMSBaselineResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDMSBaselineResults, self).__init__(task, success, outcome)

    #@utils.profiler
    def merge_with_context(self, context):
        super(SDMSBaselineResults, self).merge_with_context(context)

        # increment iteration counter
        # register detected lines to reduction group member
        reduction_group = context.observing_run.ms_reduction_group
        for b in self.outcome['baselined']:
            group_id = b['group_id']
            member_list = b['members']
            lines = b['lines']
            channelmap_range = b['channelmap_range']
            group_desc = reduction_group[group_id]
            for (ms,field,ant,spw) in utils.iterate_group_member(reduction_group[group_id], member_list):#itertools.izip(vislist,fieldlist,antennalist,spwlist):
                group_desc.iter_countup(ms, ant, spw, field)
                group_desc.add_linelist(lines, ms, ant, spw, field,
                                        channelmap_range=channelmap_range)
                
        # register working data that stores spectra after baseline subtraction
        if self.outcome.has_key('work_data'):
            for (vis,work_data) in self.outcome['work_data'].iteritems():
                ms = context.observing_run.get_ms(vis)
                ms.work_data = work_data
                 
        # merge deviation_mask with context
        for ms in context.observing_run.measurement_sets:
            ms.deviation_mask = None
        if self.outcome.has_key('deviation_mask'):
            for (basename, masks) in self.outcome['deviation_mask'].iteritems():
                ms = context.observing_run.get_ms(basename)
                ms.deviation_mask = {}
                for field in ms.get_fields(intent='TARGET'):
                    for antenna in ms.antennas:
                        for spw in ms.get_spectral_windows(science_windows_only=True):
                            key = (field.id, antenna.id, spw.id)
                            if masks.has_key(key):
                                ms.deviation_mask[key] = masks[key]

    def _outcome_name(self):
        return '\n'.join(['Reduction Group {0}: member {1}'.format(b['group_id'], b['members'])
                for b in self.outcome['baselined']])
        
class SDMSBaseline(basetask.StandardTaskTemplate):
    Inputs = SDMSBaselineInputs
    
    class RGAccumulator(object):
        def __init__(self):
            self.field = []
            self.antenna = []
            self.spw = []
            self.grid_table = []
            self.channelmap_range = []
            
        def append(self, field_id, antenna_id, spw_id, grid_table, channelmap_range):
            self.field.append(field_id)
            self.antenna.append(antenna_id)
            self.spw.append(spw_id)
            self.grid_table.append(compress.CompressedObj(grid_table))
            self.channelmap_range.append(compress.CompressedObj(channelmap_range))
            
#         def extend(self, field_id_list, antenna_id_list, spw_id_list):
#             self.field.extend(field_id_list)
#             self.antenna.extend(antenna_id_list)
#             self.spw.extend(spw_id_list)
#             
        def get_field_id_list(self):
            return self.field
        
        def get_antenna_id_list(self):
            return self.antenna
        
        def get_spw_id_list(self):
            return self.spw
        
        def get_grid_table_list(self):
            return self.grid_table
        
        def get_channelmap_range_list(self):
            return self.channelmap_range
        
        def iterate_id(self):
            assert len(self.field) == len(self.antenna)
            assert len(self.field) == len(self.spw)
            for v in itertools.izip(self.field, self.antenna, self.spw):
                yield v
                
        def iterate_all(self):
            assert len(self.field) == len(self.antenna)
            assert len(self.field) == len(self.spw)
            assert len(self.field) == len(self.grid_table)
            assert len(self.field) == len(self.channelmap_range)
            for f, a, s, g, c in itertools.izip(self.field, self.antenna, self.spw, 
                                                self.grid_table, self.channelmap_range):
                _g = g.decompress()
                _c = c.decompress()
                yield f, a, s, _g, _c
                del _g
                del _c
            
        
        def get_process_list(self):
            field_id_list = self.get_field_id_list()
            antenna_id_list = self.get_antenna_id_list()
            spw_id_list = self.get_spw_id_list()
            
            assert len(field_id_list) == len(antenna_id_list)
            assert len(field_id_list) == len(spw_id_list)
            
            return field_id_list, antenna_id_list, spw_id_list
    
    def is_multi_vis_task(self):
        return True
    
#    @memory_profiler.profile
    def prepare(self):
        LOG.debug('Starting SDMDBaseline.prepare')
        inputs = self.inputs
        context = inputs.context
        datatable_name = context.observing_run.ms_datatable_name
        datatable = DataTable(datatable_name)
        reduction_group = context.observing_run.ms_reduction_group
        vis_list = inputs.vis
        ms_list = inputs.ms
        args = inputs.to_casa_args()

        window = inputs.linewindow
        edge = inputs.edge
        broadline = inputs.broadline
        fitorder = 'automatic' if inputs.fitorder is None or inputs.fitorder < 0 else inputs.fitorder
        fitfunc = inputs.fitfunc
        clusteringalgorithm = inputs.clusteringalgorithm
        deviationmask = inputs.deviationmask
        
        # mkdir stage_dir if it doesn't exist
        stage_number = context.task_counter
        stage_dir = os.path.join(context.report_dir, "stage%d" % stage_number)
        if not os.path.exists(stage_dir):
            os.makedirs(stage_dir)

        # loop over reduction group

        # This is a dictionary for deviation mask that will be merged with top-level context
        deviation_mask = collections.defaultdict(dict)
        
        # collection of field, antenna, and spw ids in reduction group per MS
        registry = collections.defaultdict(SDMSBaseline.RGAccumulator)
        
        # outcome for baseline subtraction
        baselined = []

        LOG.debug('Starting per reduction group processing: number of groups is {ngroup}', ngroup=len(reduction_group))
        for (group_id, group_desc) in reduction_group.iteritems():
            LOG.info('Processing Reduction Group {}', group_id)
            LOG.info('Group Summary:')
            for m in group_desc:
                # LOG.debug('\tAntenna %s Spw %s Pol %s'%(m.antenna, m.spw, m.pols))
                LOG.info('\tMS "{ms}" Antenna "{antenna}" (ID {antenna_id}) Spw {spw} Field "{field}" (ID {field_id})',
                          ms=m.ms.basename,
                          antenna=m.antenna_name,
                          antenna_id=m.antenna_id, spw=m.spw_id,
                          field=m.field_name,
                          field_id=m.field_id)
            # assume all members have same spw and pollist
            first_member = group_desc[0]
            iteration = first_member.iteration
            LOG.debug('iteration for group {group_id} is {iter}', group_id=group_id, iter=iteration)
 
            # skip channel averaged spw
            nchan = group_desc.nchan
            LOG.debug('nchan for group {group_id} is {nchan}', group_id=group_id, nchan=nchan)
            if nchan == 1:
                LOG.info('Skip channel averaged spw {}.', first_member.spw)
                continue
 
            LOG.debug('spw=\'{}\'', args['spw'])
            LOG.debug('vis_list={}', vis_list)
            member_list = numpy.fromiter(utils.get_valid_ms_members2(group_desc, ms_list, args['antenna'], args['field'], args['spw']),
                                         dtype=numpy.int32)
            # skip this group if valid member list is empty
            LOG.debug('member_list={}', member_list)
            if len(member_list) == 0:
                LOG.info('Skip reduction group {}', group_id)
                continue
 
            member_list.sort()
            
            LOG.debug('Members to be processed:')
            for (gms, gfield, gant, gspw) in utils.iterate_group_member(group_desc, member_list):
                LOG.debug('\tMS "{}" Field ID {} Antenna ID {} Spw ID {}', 
                          gms.basename, gfield, gant, gspw)
                 
            # Deviation Mask 
            # NOTE: deviation mask is evaluated per ms per field per spw
            if deviationmask:
                LOG.info('Apply deviation mask to baseline fitting')
                for (ms, fieldid, antennaid, spwid) in \
                    utils.iterate_group_member(group_desc, member_list):
                    if (not hasattr(ms, 'deviation_mask')) or ms.deviation_mask is None:
                        ms.deviation_mask = {}
                    if not ms.deviation_mask.has_key((fieldid,antennaid,spwid)):
                        LOG.debug('Evaluating deviation mask for {} field {} antenna {} spw {}',
                                  ms.basename, fieldid, antennaid, spwid)
                        mask_list = self.evaluate_deviation_mask(ms.name, fieldid, antennaid, spwid, 
                                                                 consider_flag=True)
                        LOG.debug('deviation mask = {}', mask_list)
                        ms.deviation_mask[(fieldid, antennaid, spwid)] = mask_list
                    deviation_mask[ms.basename][(fieldid, antennaid, spwid)] = ms.deviation_mask[(fieldid, antennaid, spwid)]
                    LOG.debug('evaluated deviation mask is {v}',
                              v=ms.deviation_mask[(fieldid, antennaid, spwid)])
            else:
                LOG.info('Deviation mask is disabled by the user')
            LOG.debug('deviation_mask={}', deviation_mask)

            # Spectral Line Detection and Validation
            # MaskLine will update DataTable.MASKLIST column
            maskline_inputs = maskline.MaskLine.Inputs(context, iteration, group_id, member_list, 
                                                       window, edge, broadline, clusteringalgorithm)
            maskline_task = maskline.MaskLine(maskline_inputs)
            job = common.ParameterContainerJob(maskline_task, datatable=datatable)
            maskline_result = self._executor.execute(job, merge=False)
            grid_table = maskline_result.outcome['grid_table']
            if grid_table is None:
                LOG.info('Skip reduction group {}', group_id)
                continue
            detected_lines = maskline_result.outcome['detected_lines']
            channelmap_range = maskline_result.outcome['channelmap_range']
            cluster_info = maskline_result.outcome['cluster_info']

            # register ids to per MS id collection
            for i in member_list:
                member = group_desc[i]
                registry[member.ms].append(member.field_id, member.antenna_id, member.spw_id, 
                                           grid_table, channelmap_range)
             
            # add entry to outcome
            baselined.append({'group_id': group_id, 'iteration': iteration,
                              'members': member_list,
                              'lines': detected_lines,
                              'channelmap_range': channelmap_range,
                              'clusters': cluster_info})
        # - end of the loop over reduction group
            
        blparam_file = lambda ms: ms.basename.rstrip('/') \
            + '_blparam_stage{stage}.txt'.format(stage=stage_number)
        work_data = {}
        plot_list = []
        plot_manager = plotter.BaselineSubtractionPlotManager(self.inputs.context, datatable)
        for ms in context.observing_run.measurement_sets:
            if ms not in registry:
                continue
            accum = registry[ms]
            vis = ms.basename
            field_id_list = accum.get_field_id_list()
            antenna_id_list = accum.get_antenna_id_list()
            spw_id_list = accum.get_spw_id_list()
            LOG.debug('subgroup member for {vis}:\n\tfield: {field}\n\tantenna: {antenna}\n\tspw: {spw}',
                      vis=ms.basename,
                      field=field_id_list,
                      antenna=antenna_id_list,
                      spw=spw_id_list)
             
            # fit order determination and subtraction
            fitter_inputs = worker.BaselineSubtractionTask.Inputs(context,
                                                                  fitfunc=fitfunc,
                                                                  vis=ms.name,
                                                                  fit_order=fitorder,
                                                                  edge=edge,
                                                                  blparam=blparam_file(ms))
            fitter_task = worker.BaselineSubtractionTask(fitter_inputs)
            job = common.ParameterContainerJob(fitter_task, datatable=datatable, 
                                               process_list=accum, 
                                               deviationmask_list=deviation_mask[vis])
            fitter_results = self._executor.execute(job, merge=False)
            LOG.debug('fitter_results: {}', fitter_results)
 
            outfile = fitter_results.outcome['outfile']
            work_data[ms.name] = outfile
             
            # plot             
            # initialize plot manager
            status = plot_manager.initialize(ms, outfile)
            for (field_id, antenna_id, spw_id, grid_table, channelmap_range) in accum.iterate_all():
                 
                if (field_id, antenna_id, spw_id) in deviation_mask[vis]:
                    deviationmask = deviation_mask[vis][(field_id, antenna_id, spw_id)]
                else:
                    deviationmask = None
                    
                 
                if status:
                    plot_list.extend(plot_manager.plot_spectra_with_fit(field_id, antenna_id, spw_id, 
                                                                        grid_table, 
                                                                        deviationmask, channelmap_range))
        plot_manager.finalize()
        
        outcome = {'baselined': baselined,
                   'work_data': work_data,
                   'edge': edge,
                   'deviation_mask': deviation_mask,
                   'plots': plot_list}
        results = SDMSBaselineResults(task=self.__class__,
                                    success=True,
                                    outcome=outcome)
                        
        return results

    def analyse(self, result):
        return result
    
    def evaluate_deviation_mask(self, vis, field_id, antenna_id, spw_id, consider_flag=False):
        """
        Create deviation mask using MaskDeviation heuristic
        """
        h = MaskDeviationHeuristic()
        mask_list = h.calculate(vis=vis, field_id=field_id, antenna_id=antenna_id, spw_id=spw_id, 
                                consider_flag=consider_flag)
        return mask_list


