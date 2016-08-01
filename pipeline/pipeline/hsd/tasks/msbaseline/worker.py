from __future__ import absolute_import

import os
import time
import re
import numpy
import collections
import itertools
import types
import shutil

import asap as sd

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.sdfilenamer as filenamer
from pipeline.hsd.heuristics import fitorder, fragmentation
from pipeline.domain import DataTable
from .. import common
from pipeline.hsd.tasks.common import utils as sdutils
from pipeline.infrastructure import casa_tasks
from . import plotter
import pipeline.infrastructure.renderer.logger as logger

from pipeline.infrastructure.displays.singledish.utils import sd_polmap

LOG = infrastructure.get_logger(__name__)

class BaselineParamKeys(object):
    ROW = 'row'
    POL = 'pol'
    MASK = 'mask'
    CLIPNITER = 'clipniter'
    CLIPTHRESH = 'clipthresh'
    USELF = 'use_linefinder'
    LFTHRESH = 'thresh'
    LEDGE = 'Ledge'
    REDGE = 'Redge'
    AVG_LIMIT = 'avg_limit'
    FUNC = 'blfunc'
    ORDER = 'order'
    NPIECE = 'npiece'
    NWAVE = 'nwave'
    ORDERED_KEY = [ROW, POL, MASK, CLIPNITER, CLIPTHRESH, USELF, LFTHRESH, 
                   LEDGE, REDGE, AVG_LIMIT, FUNC, ORDER, NPIECE, NWAVE]
BLP = BaselineParamKeys
    

def write_blparam(blparam_file, param):
    param_values = collections.defaultdict(str)
    for key in BLP.ORDERED_KEY:
        if param.has_key(key):
            param_values[key] = param[key]
    line = ','.join(map(str, [param_values[k] for k in BLP.ORDERED_KEY]))
    with open(blparam_file, 'a') as f:
        f.write(line+'\n')

def as_maskstring(masklist):
    return ';'.join(map(lambda x: '%s~%s'%(x[0],x[1]), masklist))

def generate_plot_table(ms_id, antenna_id, spw_id, polarization_ids, grid_table):
    def filter(msid, ant, spw, pols, table):
        for row in table:
            if row[0] == spw and row[1] in pols:
                new_row_entry = row[2:6] + [numpy.array([r[3] for r in row[6] if r[-1] == msid and r[-2] == ant], dtype=int)]
                yield new_row_entry
    new_table = list(filter(ms_id, antenna_id, spw_id, polarization_ids, grid_table))
    return new_table

class BaselineSubtractionInputsBase(basetask.StandardInputs):
    DATACOLUMN = {'CORRECTED_DATA': 'corrected',
                  'DATA': 'data',
                  'FLOAT_DATA': 'float_data'}
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
        if (not args.has_key('outfile')) or args['outfile'] is None or len(args['outfile']) == 0:
            args['outfile'] = self.vis.rstrip('/') + '_bl'
            
        args['datacolumn'] = self.DATACOLUMN[self.colname]
            
        return args
    
    @property
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
    
class BaselineSubtractionResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(BaselineSubtractionResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(BaselineSubtractionResults, self).merge_with_context(context)
    
    def _outcome_name(self):
        # outcome should be a name of blparam text file
        return 'blparam: "%s" bloutput: "%s"'%(self.outcome['blparam'], self.outcome['bloutput'])

class BaselineFitParamConfig(basetask.StandardTaskTemplate):
    ApplicableDuration = 'raster' # 'raster' | 'subscan'
    MaxPolynomialOrder = 'none' # 'none', 0, 1, 2,...
    PolynomialOrder = 'automatic' # 'automatic', 0, 1, 2, ...
    ClipCycle = 1

    def is_multi_vis_task(self):
        return False
    
    def prepare(self):
        LOG.debug('Starting BaselineFitParamConfig.prepare')
        fragmentation_heuristic = fragmentation.FragmentationHeuristics()

        # fitting order
        fit_order = self.inputs.fit_order
        if fit_order == 'automatic':
            # fit order heuristics
            LOG.info('Baseline-Fitting order was automatically determined')
            self.fitorder_heuristic = fitorder.FitOrderHeuristics()
        else:
            LOG.info('Baseline-Fitting order was fixed to be %d'%(fit_order))
            self.fitorder_heuristic = lambda *args, **kwargs: self.inputs.fit_order

        context = self.inputs.context
        vis = self.inputs.vis
        datatable = DataTable(context.observing_run.ms_datatable_name)
        ms = context.observing_run.get_ms(vis)
        
        args = self.inputs.to_casa_args()

        vis = self.inputs.vis
        antenna_id = self.inputs.antenna_id
        field_id = self.inputs.field_id
        spw_id = self.inputs.spw_id
        LOG.debug('MS "%s" ant %s field %s spw %s'%(os.path.basename(vis),antenna_id,field_id,spw_id))
        
        nchan = ms.spectral_windows[spw_id].num_channels
        edge = common.parseEdge(self.inputs.edge)
        
        LOG.debug('nchan={nchan} edge={edge}'.format(nchan=nchan,edge=edge))
        
        if self.ApplicableDuration == 'subscan':
            timetable_index = 1
        else:
            timetable_index = 0
            
        index_list_total = []

        # prepare mask arrays
        mask_array = numpy.ones(nchan, dtype=int)
        mask_array[:edge[0]] = 0
        mask_array[nchan-edge[1]:] = 0
        
        # deviation mask
        deviation_mask = self.inputs.deviationmask
        LOG.debug('Deviation mask for field %s antenna %s spw %s: %s'%(field_id, antenna_id, spw_id, deviation_mask))
        if deviation_mask is not None:
            for mask_range in deviation_mask:
                mask_array[mask_range[0]:mask_range[1]] = 0
        
        base_mask_array = mask_array.copy()

        time_table = datatable.get_timetable(antenna_id, spw_id, None, os.path.basename(vis), field_id)
        member_list = time_table[timetable_index]
        
        # working with spectral data in scantable
        nrow_total = sum((len(x[0]) for x in member_list))
                
        LOG.info('Calculating Baseline Fitting Parameter...')
        LOG.info('Processing %d spectra...'%(nrow_total))
        
        colname = self.inputs.colname
        LOG.debug('data column name is "%s"'%(colname))
        
        for y in xrange(len(member_list)):
            rows = member_list[y][0]
            idxs = member_list[y][1]
        
            with casatools.TableReader(vis) as tb:
                #spectra = numpy.fromiter((tb.getcell(colname,row)
                #                          for row in rows), 
                #                         dtype=numpy.float64)
                #tsel = tb.query('ROWNUMBER() IN [%s]'%((','.join(map(str, rows)))), style='python')
                #spectra = tsel.getcol()
                #tsel.close()
                spectra = numpy.asarray([tb.getcell(colname, row) for row in rows])
                #get_mask_from_flagtra: 1 valid 0 invalid
                #arg for mask_to_masklist: 0 valid 1 invalid
                #flaglist = [self._mask_to_masklist(-sdutils.get_mask_from_flagtra(tb.getcell('FLAGTRA', row))+1 )
                flaglist = [self._mask_to_masklist(tb.getcell('FLAG', row).astype(int))
                            for row in rows]

                LOG.debug("Flag Mask = %s" % str(flaglist))

            spectra[:,:edge[0],:] = 0.0
            spectra[:,nchan-edge[1]:,:] = 0.0 
                
            # here we assume that masklist is polarization-independent
            # (this is because that line detection/validation process accumulates 
            # polarization components together
            masklist = [datatable.tb2.getcell('MASKLIST',idx)
                        for idx in idxs]
#                 masklist = [datatable.tb2.getcell('MASKLIST',idxs[i]) + flaglist[i]
#                             for i in range(len(idxs))]
            LOG.debug('DONE %s'%(y))
            
            npol = spectra.shape[1]
            for pol in xrange(npol):
                # fit order determination
                polyorder = self.fitorder_heuristic(spectra[:,pol,:], [ list(masklist[i]) + flaglist[i][pol] for i in range(len(idxs))], edge)
                #del spectra
                if fit_order == 'automatic' and self.MaxPolynomialOrder != 'none':
                    polyorder = min(polyorder, self.MaxPolynomialOrder)
                LOG.debug('time group %d pol %d: fitting order=%s'%(y,pol,polyorder))
                
                # calculate fragmentation
                (fragment, nwindow, win_polyorder) = fragmentation_heuristic(polyorder, nchan, edge)
    
                nrow = len(rows)
                LOG.debug('nrow = %s'%(nrow))
                LOG.debug('len(idxs) = %s'%(len(idxs)))
            
                for i in xrange(nrow):
                    row = rows[i]
                    idx = idxs[i]
                    LOG.trace('===== Processing at row = %s ====='%(row))
                    nochange = datatable.tb2.getcell('NOCHANGE',idx)
                    LOG.trace('row = %s, Flag = %s'%(row, nochange))

                    # mask lines
                    maxwidth = 1
#                     _masklist = masklist[i] 
                    _masklist = list(masklist[i]) + flaglist[i][pol]
                    for [chan0, chan1] in _masklist:
                        if chan1 - chan0 >= maxwidth:
                            maxwidth = int((chan1 - chan0 + 1) / 1.4)
                            # allowance in Process3 is 1/5:
                            #    (1 + 1/5 + 1/5)^(-1) = (5/7)^(-1)
                            #                         = 7/5 = 1.4
                    max_polyorder = int((nchan - sum(edge)) / maxwidth + 1)
                    LOG.trace('Masked Region from previous processes = %s'%(_masklist))
                    LOG.trace('edge parameters= (%s,%s)'%(edge))
                    LOG.trace('Polynomial order = %d  Max Polynomial order = %d'%(polyorder, max_polyorder))

                    # fitting
                    polyorder = min(polyorder, max_polyorder)
                    mask_array[:] = base_mask_array
                    #irow = len(row_list_total)+len(row_list)
                    #irow = len(index_list_total) + i
                    irow = row
                    param = self._calc_baseline_param(irow, pol, polyorder, nchan, 0, edge, _masklist, win_polyorder, fragment, nwindow, mask_array)
                    # defintion of masklist differs in pipeline and ASAP (masklist = [a, b+1] in pipeline masks a channel range a ~ b-1)
                    param[BLP.MASK] = [ [start, end-1] for [start, end] in param[BLP.MASK] ]
                    param[BLP.MASK] = as_maskstring(param[BLP.MASK])
                    LOG.trace('Row %s: param=%s'%(row,param))
                    write_blparam(args['blparam'], param)
                    
                # MS rows contain npol spectra
                if pol == 0:
                    index_list_total.extend(idxs)

        outcome = {'blparam': args['blparam'],
                   'bloutput': args['bloutput']}
        results = BaselineSubtractionResults(success=True, outcome=outcome)
        
        return results

    def analyse(self, results):
        return results
    
    def _calc_baseline_param(self, row_idx, pol, polyorder, nchan, modification, edge, masklist, win_polyorder, fragment, nwindow, mask):
        # Create mask for line protection
        nchan_without_edge = nchan - sum(edge)
        if type(masklist) == list or type(masklist) == numpy.ndarray:
            for [m0, m1] in masklist:
                mask[m0:m1] = 0
        else:
            LOG.critical('Invalid masklist')
        num_mask = int(nchan_without_edge - numpy.sum(mask[edge[0]:nchan-edge[1]] * 1.0))
        masklist_all = self.__mask_to_masklist(mask)

        LOG.trace('nchan_without_edge, num_mask, diff=%s, %s'%(nchan_without_edge, num_mask))

        outdata = self._get_param(row_idx, pol, polyorder, nchan, mask, edge, nchan_without_edge, num_mask, fragment, nwindow, win_polyorder, masklist_all)

        LOG.trace('outdata=%s'%(outdata))

        return outdata

    def _mask_to_masklist(self, mask):
        return [self.__mask_to_masklist(m) for m in mask]
    
    def __mask_to_masklist(self, mask):
        """
        Converts mask array to masklist
        
        Argument
            mask : an array of channel mask in values 0 (rejected) or 1 (adopted)
        """
        # get indices of clump boundaries
        idx = (mask[1:] ^ mask[:-1]).nonzero()
        idx = (idx[0] + 1)
        # idx now contains pairs of start-end indices, edges need handling
        # depending on first and last mask value
        if mask[0]:
            if len(idx) == 0:
                return [[0, len(mask)]]
            r = [[0, idx[0]]]
            if len(idx) % 2 == 1:
                r.extend(idx[1:].reshape(-1, 2).tolist())
            else:
                r.extend(idx[1:-1].reshape(-1, 2).tolist())
        else:
            if len(idx) == 0:
                return []
            if len(idx) % 2 == 1:
                r = (idx[:-1].reshape(-1, 2).tolist())
            else:
                r = (idx.reshape(-1, 2).tolist())
        if mask[-1]:
            r.append([idx[-1], len(mask)])
        return r

class CubicSplineFitParamConfigInputs(BaselineSubtractionInputsBase):
    def __init__(self, context, vis=None, field_id=None, antenna_id=None, spw_id=None, 
                 fit_order=None, edge=None, deviationmask=None, blparam=None, bloutput=None):
        self._init_properties(vars())
                
    fit_order = basetask.property_with_default('fit_order', 'automatic')
    edge = basetask.property_with_default('edge', (0,0))
    
class CubicSplineFitParamConfig(BaselineFitParamConfig):
    Inputs = CubicSplineFitParamConfigInputs
    def _get_param(self, idx, pol, polyorder, nchan, mask, edge, nchan_without_edge, nchan_masked, fragment, nwindow, win_polyorder, masklist):
        num_nomask = nchan_without_edge - nchan_masked
        num_pieces = max(int(min(polyorder * num_nomask / float(nchan_without_edge) + 0.5, 0.1 * num_nomask)), 1)
        LOG.trace('Cubic Spline Fit: Number of Sections = %d' % num_pieces)
        return {BLP.ROW: idx, BLP.POL: pol, BLP.MASK: masklist, BLP.NPIECE: num_pieces, BLP.FUNC: 'cspline', BLP.CLIPTHRESH: 5.0, BLP.CLIPNITER: self.ClipCycle}
    

class BaselineSubtractionWorkerInputs(BaselineSubtractionInputsBase):
    def __init__(self, context, vis=None, field_id_list=None, antenna_id_list=None, spw_id_list=None, 
                 fit_order=None, edge=None, deviationmask_list=None, blparam=None, bloutput=None, 
                 grid_table_list=None, channelmap_range_list=None):
        self._init_properties(vars())
        
class BaselineSubtractionWorker(basetask.StandardTaskTemplate):
    Inputs = BaselineSubtractionWorkerInputs
    SubTask = None
    
    def prepare(self):
        context = self.inputs.context
        vis = self.inputs.vis
        ms = self.inputs.ms
        field_id_list = self.inputs.field_id_list
        antenna_id_list = self.inputs.antenna_id_list
        spw_id_list = self.inputs.spw_id_list
        fit_order = self.inputs.fit_order
        edge = self.inputs.edge
        deviationmask_list = self.inputs.deviationmask_list
        args = self.inputs.to_casa_args()
        blparam = args['blparam']
        bloutput = args['bloutput']
        outfile = args['outfile']
        datacolumn = args['datacolumn']
        
        assert len(field_id_list) == len(antenna_id_list)
        assert len(antenna_id_list) == len(spw_id_list)
        assert len(field_id_list) == len(deviationmask_list)
        
        # initialization of blparam file
        # blparam file needs to be removed before starting iteration through 
        # reduction group
        if os.path.exists(blparam):
            LOG.debug('Cleaning up blparam file for {vis}'.format(vis=vis))
            os.remove(blparam)        
        
        for (field_id, antenna_id, spw_id, deviationmask) in \
            zip(field_id_list, antenna_id_list, spw_id_list, deviationmask_list):
            inputs = self.SubTask.Inputs(context, vis=vis, field_id=field_id,
                                         antenna_id=antenna_id, spw_id=spw_id,
                                         fit_order=fit_order, edge=edge, 
                                         deviationmask=deviationmask, blparam=blparam, 
                                         bloutput=bloutput)
            task = self.SubTask(inputs)
            subtask_results = self._executor.execute(task, merge=True)
            
        # execute tsdbaseline
        job = casa_tasks.tsdbaseline(infile=vis, datacolumn=datacolumn, blmode='fit', dosubtract=True,
                                     blformat='table', bloutput=bloutput, 
                                     blfunc='variable', blparam=blparam,
                                     outfile=outfile, overwrite=True)
        self._executor.execute(job)
#         # copy infile to outfile for testing
#         if os.path.exists(outfile):
#             shutil.rmtree(outfile)
#         shutil.copytree(vis, outfile)
            
        outcome = {'blparam': blparam,
                   'bloutput': bloutput,
                   'outfile': outfile}
        results = BaselineSubtractionResults(success=True, outcome=outcome)
        return results
        
    def analyse(self, results):
        # plotting
        #plot_table = self.inputs.plot_table
        #channelmap_range = self.inputs.channelmap_range
        plot_list = []
        ms = self.inputs.ms
        ms_id = self.inputs.context.observing_run.measurement_sets.index(ms)
        vis = self.inputs.vis
        field_id_list = self.inputs.field_id_list
        antenna_id_list = self.inputs.antenna_id_list
        spw_id_list = self.inputs.spw_id_list
        grid_table_list = self.inputs.grid_table_list
        channelmap_range_list = self.inputs.channelmap_range_list
        deviationmask_list = self.inputs.deviationmask_list
        args = self.inputs.to_casa_args()
        outfile = args['outfile']
        ###if grid_table is not None:
        
        # mkdir stage_dir if it doesn't exist
        #stage_dir = self.inputs.stage_dir
        #if stage_dir is None:
        stage_number = self.inputs.context.task_counter
        stage_dir = os.path.join(self.inputs.context.report_dir,"stage%d" % stage_number)
        if not os.path.exists(stage_dir):
            os.makedirs(stage_dir)
            
        # get row map between original and baseline-subtracted MS
        rowmap = self.get_rowmap_for_baseline(ms, outfile)
            
        #st = self.inputs.context.observing_run[antennaid]
        
        for (field_id, antenna_id, spw_id, grid_table, deviation_mask, channelmap_range) in \
                zip(field_id_list, antenna_id_list, spw_id_list, grid_table_list, deviationmask_list, channelmap_range_list):
            if grid_table is not None:
                data_desc = ms.get_data_description(spw=spw_id)
                num_pol = data_desc.num_polarizations
                pol_list = numpy.arange(num_pol, dtype=int)
                source_name = ms.fields[field_id].source.name.replace(' ', '_').replace('/','_')
                plot_table = generate_plot_table(ms_id, antenna_id, spw_id, pol_list, grid_table)
                plot_list.extend(list(self.plot_spectra_with_fit(source_name, antenna_id, spw_id, 
                                                                 pol_list, plot_table, 
                                                                 vis, outfile,
                                                                 stage_dir, deviation_mask,
                                                                 channelmap_range,
                                                                 rowmap=rowmap)))
            
            
#             # TODO: use proper source name when we can handle multiple source 
#             source_name = ''
#             for (source_id,source) in st.source.items():
#                 if 'TARGET' in source.intents:
#                     source_name = source.name.replace(' ', '_').replace('/','_')
# #             prefix = 'spectral_plot_before_subtraction_%s_%s_ant%s_spw%s'%('.'.join(st.basename.split('.')[:-1]),source_name,antennaid,spwid)
# #             plot_list.extend(self.plot_spectra(source_name, antennaid, spwid, pollist, self.inputs.grid_table, 
# #                                                filename_in, stage_dir, prefix, channelmap_range))
# #             prefix = prefix.replace('before', 'after')
# #             plot_list.extend(self.plot_spectra(source_name, antennaid, spwid, pollist, grid_table, filename_out, stage_dir, prefix, channelmap_range))
#         
#             plot_list.extend(list(self.plot_spectra_with_fit(source_name, antennaid, spwid, pollist, grid_table, filename_in, filename_out, stage_dir, channelmap_range)))
        
#         outcome = {'bltable': bltable_name,
#                    'index_list': index_list_total,
#                    'outtable': filename_out,
#                    'plot_list': plot_list}
#         result = FittingResults(task=self.__class__,
#                                 success=True,
#                                 outcome=outcome)
   
        results.outcome.update({'plot_list': plot_list})
        
        return results
    
    def get_rowmap_for_baseline(self, ms, postfit_data):
        rowmap = sdutils.make_row_map(ms, postfit_data)
        return rowmap

    
    def plot_spectra_with_fit(self, source, ant, spwid, pols, grid_table, prefit_data, postfit_data, outdir, deviation_mask, channelmap_range, rowmap=None):
        #st = self.inputs.context.observing_run[ant]
        ms= self.inputs.ms
        line_range = [[r[0] - 0.5 * r[1], r[0] + 0.5 * r[1]] for r in channelmap_range if r[2] is True]
        if len(line_range) == 0:
            line_range = None
        for pol in pols:
            LOG.debug('Generating plots for source %s ant %s spw %s pol %s'%(source, ant, spwid, pol))
            outfile_template = lambda x: 'spectral_plot_%s_subtraction_%s_%s_ant%s_spw%s_pol%s.png'%(x,'.'.join(ms.basename.split('.')[:-1]),source,ant,spwid,pol)
            prefit_outfile = os.path.join(outdir, outfile_template('before'))
            postfit_outfile = os.path.join(outdir, outfile_template('after'))
            LOG.debug('prefit_outfile=\'%s\''%(os.path.basename(prefit_outfile)))
            LOG.debug('postfit_outfile=\'%s\''%(os.path.basename(postfit_outfile)))
            status = plotter.plot_profile_map_with_fit(self.inputs.context, ms, ant, spwid, pol, grid_table, prefit_data, postfit_data, 
                                                       prefit_outfile, postfit_outfile, deviation_mask, line_range, rowmap=rowmap)
            if os.path.exists(prefit_outfile):
                parameters = {'intent': 'TARGET',
                              'spw': spwid,
                              'pol': sd_polmap[pol],
                              'ant': ms.antennas[ant].name,
                              'vis': ms.basename,
                              'type': 'sd_sparse_map_before_subtraction',
                              'file': prefit_data}
                plot = logger.Plot(prefit_outfile,
                                   x_axis='Frequency',
                                   y_axis='Intensity',
                                   field=source,
                                   parameters=parameters)
                yield plot
            if os.path.exists(postfit_outfile):
                parameters = {'intent': 'TARGET',
                              'spw': spwid,
                              'pol': sd_polmap[pol],
                              'ant': ms.antennas[ant].name,
                              'vis': ms.basename,
                              'type': 'sd_sparse_map_after_subtraction',
                              'file': postfit_data}
                plot = logger.Plot(postfit_outfile,
                                   x_axis='Frequency',
                                   y_axis='Intensity',
                                   field=source,
                                   parameters=parameters)
                yield plot
                
class CubicSplineBaselineSubtractionWorker(BaselineSubtractionWorker):
    SubTask = CubicSplineFitParamConfig

# facade for FitParam
class BaselineSubtractionInputs(basetask.ModeInputs):
    _modes = {'spline': CubicSplineBaselineSubtractionWorker, 
              'cspline': CubicSplineBaselineSubtractionWorker}

    def __init__(self, context, fitfunc, **parameters):
        super(BaselineSubtractionInputs, self).__init__(context=context, mode=fitfunc, **parameters)
    
class BaselineSubtractionTask(basetask.ModeTask):
    Inputs = BaselineSubtractionInputs
    
