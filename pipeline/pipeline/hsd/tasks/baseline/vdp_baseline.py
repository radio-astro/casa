from __future__ import absolute_import

import collections
import itertools
# import memory_profiler
import os

import numpy

# import pipeline.infrastructure.mpihelpers as mpihelpers
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
import pipeline.infrastructure.utils as utils
from pipeline.h.heuristics import fieldnames
from pipeline.hsd.heuristics import MaskDeviationHeuristic
from pipeline.domain import DataTable

from .. import common
from ..common import utils as sdutils
from . import maskline
from . import worker
from . import plotter
# from . import fitting
from ..common import compress

_LOG = infrastructure.get_logger(__name__)
LOG = sdutils.OnDemandStringParseLogger(_LOG)


class SDBaselineInputs(vdp.StandardInputs):
    """
    Inputs for baseline subtraction
    """
    linewindow = vdp.VisDependentProperty(default=[])
    edge = vdp.VisDependentProperty(default=(0, 0))
    broadline = vdp.VisDependentProperty(default=True)
    fitorder = vdp.VisDependentProperty(default='automatic')
    fitfunc = vdp.VisDependentProperty(default='cspline')
    clusteringalgorithm = vdp.VisDependentProperty(default='kmean')
    deviationmask = vdp.VisDependentProperty(default=True)

    @vdp.VisDependentProperty
    def infiles(self):
        return self.vis

    @infiles.convert
    def infiles(self, value):
        self.vis = value
        return value

    antenna = vdp.VisDependentProperty(default='')

    @antenna.convert
    def antenna(self, value):
        antennas = self.ms.get_antenna(value)
        # if all antennas are selected, return ''
        if len(antennas) == len(self.ms.antennas):
            return ''
        return utils.find_ranges([a.id for a in antennas])
#         return ','.join([str(a.id) for a in antennas])

    @vdp.VisDependentProperty
    def field(self):
        # this will give something like '0542+3243,0343+242'
        field_finder = fieldnames.IntentFieldnames()
        intent_fields = field_finder.calculate(self.ms, 'TARGET')

        # run the answer through a set, just in case there are duplicates
        fields = set()
        fields.update(utils.safe_split(intent_fields))

        return ','.join(fields)

    @vdp.VisDependentProperty
    def spw(self):
        science_spws = self.ms.get_spectral_windows(with_channels=True)
        return ','.join([str(spw.id) for spw in science_spws])

    @vdp.VisDependentProperty
    def pol(self):
        # filters polarization by self.spw
        selected_spwids = [int(spwobj.id) for spwobj in self.ms.get_spectral_windows(self.spw, with_channels=True)]
        pols = set()
        for idx in selected_spwids:
            pols.update(self.ms.get_data_description(spw=idx).corr_axis)
        
        return ','.join(pols)

    def __init__(self, context, infiles=None, antenna=None, spw=None, pol=None, field=None,
                 linewindow=None, edge=None, broadline=None, fitorder=None,
                 fitfunc=None, clusteringalgorithm=None, deviationmask=None):
        super(SDBaselineInputs, self).__init__()

        # context and vis/infiles must be set first so that properties that require
        # domain objects can be function
        self.context = context
        self.infiles = infiles
        # self.output_dir = output_dir

        # task specific parameters
        self.antenna = antenna
        self.spw = spw
        self.pol = pol
        self.field = field
        self.linewindow = linewindow
        self.edge = edge
        self.broadline = broadline
        self.fitorder = fitorder
        self.fitfunc = fitfunc
        self.clusteringalgorithm = clusteringalgorithm
        self.deviationmask = deviationmask
            
    def to_casa_args(self):
        vis = self.vis
        if isinstance(self.vis, list):
            self.vis = vis[0]
        args = super(SDBaselineInputs, self).to_casa_args()
        self.vis = vis
        
        if 'antenna' not in args:
            args['antenna'] = ''
        return args
            

class SDBaselineResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDBaselineResults, self).__init__(task, success, outcome)

    # @sdutils.profiler
    def merge_with_context(self, context):
        super(SDBaselineResults, self).merge_with_context(context)

        # increment iteration counter
        # register detected lines to reduction group member
        reduction_group = context.observing_run.ms_reduction_group
        for b in self.outcome['baselined']:
            group_id = b['group_id']
            member_list = b['members']
            lines = b['lines']
            channelmap_range = b['channelmap_range']
            group_desc = reduction_group[group_id]
            for (ms, field, ant, spw) in sdutils.iterate_group_member(reduction_group[group_id], member_list):  # itertools.izip(vislist,fieldlist,antennalist,spwlist):
                group_desc.iter_countup(ms, ant, spw, field)
                group_desc.add_linelist(lines, ms, ant, spw, field,
                                        channelmap_range=channelmap_range)
                
        # register working data that stores spectra after baseline subtraction
        if 'work_data' in self.outcome:
            for (vis, work_data) in self.outcome['work_data'].iteritems():
                ms = context.observing_run.get_ms(vis)
                ms.work_data = work_data
                 
        # merge deviation_mask with context
        for ms in context.observing_run.measurement_sets:
            ms.deviation_mask = None
        if 'deviation_mask' in self.outcome:
            for (basename, masks) in self.outcome['deviation_mask'].iteritems():
                ms = context.observing_run.get_ms(basename)
                ms.deviation_mask = {}
                for field in ms.get_fields(intent='TARGET'):
                    for antenna in ms.antennas:
                        for spw in ms.get_spectral_windows(science_windows_only=True):
                            key = (field.id, antenna.id, spw.id)
                            if key in masks:
                                ms.deviation_mask[key] = masks[key]

    def _outcome_name(self):
        return '\n'.join(['Reduction Group {0}: member {1}'.format(b['group_id'], b['members'])
                          for b in self.outcome['baselined']])


class SDBaseline(basetask.StandardTaskTemplate):
    Inputs = SDBaselineInputs
    
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
            if isinstance(grid_table, compress.CompressedObj):
                self.grid_table.append(grid_table)
            else:
                self.grid_table.append(compress.CompressedObj(grid_table))
            self.channelmap_range.append(channelmap_range)
            
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
                yield f, a, s, _g, c
                del _g

        def get_process_list(self):
            field_id_list = self.get_field_id_list()
            antenna_id_list = self.get_antenna_id_list()
            spw_id_list = self.get_spw_id_list()
            
            assert len(field_id_list) == len(antenna_id_list)
            assert len(field_id_list) == len(spw_id_list)
            
            return field_id_list, antenna_id_list, spw_id_list
    
    is_multi_vis_task = True
    
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
        registry = collections.defaultdict(SDBaseline.RGAccumulator)
        
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
            member_list = numpy.fromiter(sdutils.get_valid_ms_members2(group_desc, ms_list, args['antenna'],
                                                                       args['field'], args['spw']), dtype=numpy.int32)
            # skip this group if valid member list is empty
            LOG.debug('member_list={}', member_list)
            if len(member_list) == 0:
                LOG.info('Skip reduction group {}', group_id)
                continue
 
            member_list.sort()
            
            LOG.debug('Members to be processed:')
            for (gms, gfield, gant, gspw) in sdutils.iterate_group_member(group_desc, member_list):
                LOG.debug('\tMS "{}" Field ID {} Antenna ID {} Spw ID {}', 
                          gms.basename, gfield, gant, gspw)
                 
            # Deviation Mask 
            # NOTE: deviation mask is evaluated per ms per field per spw
            if deviationmask:
                LOG.info('Apply deviation mask to baseline fitting')
                for (ms, fieldid, antennaid, spwid) in sdutils.iterate_group_member(group_desc, member_list):
                    if (not hasattr(ms, 'deviation_mask')) or ms.deviation_mask is None:
                        ms.deviation_mask = {}
                    if (fieldid, antennaid, spwid) not in ms.deviation_mask:
                        LOG.debug('Evaluating deviation mask for {} field {} antenna {} spw {}',
                                  ms.basename, fieldid, antennaid, spwid)
                        mask_list = self.evaluate_deviation_mask(ms.name, fieldid, antennaid, spwid, 
                                                                 consider_flag=True)
                        LOG.debug('deviation mask = {}', mask_list)
                        ms.deviation_mask[(fieldid, antennaid, spwid)] = mask_list
                    deviation_mask[ms.basename][(fieldid, antennaid, spwid)] = ms.deviation_mask[(fieldid, antennaid,
                                                                                                  spwid)]
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
            compressed_table = compress.CompressedObj(grid_table)
            del grid_table

            detected_lines = maskline_result.outcome['detected_lines']
            channelmap_range = maskline_result.outcome['channelmap_range']
            cluster_info = maskline_result.outcome['cluster_info']
            
            # register ids to per MS id collection
            for i in member_list:
                member = group_desc[i]
                registry[member.ms].append(member.field_id, member.antenna_id, member.spw_id, 
                                           compressed_table, channelmap_range)
             
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
                    
                del grid_table
                
        plot_manager.finalize()
        
        outcome = {'baselined': baselined,
                   'work_data': work_data,
                   'edge': edge,
                   'deviation_mask': deviation_mask,
                   'plots': plot_list}
        results = SDBaselineResults(task=self.__class__,
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
