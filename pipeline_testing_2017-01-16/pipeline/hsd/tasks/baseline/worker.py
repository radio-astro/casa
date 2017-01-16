from __future__ import absolute_import

import os
import numpy
import collections
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.sdfilenamer as filenamer
from pipeline.hsd.heuristics import fitorder, fragmentation
from pipeline.domain import DataTable
from .. import common
from pipeline.hsd.tasks.common import utils as sdutils
from pipeline.infrastructure import casa_tasks

_LOG = infrastructure.get_logger(__name__)
LOG = sdutils.OnDemandStringParseLogger(_LOG)

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
    

#@sdutils.profiler
def write_blparam(fileobj, param):
    param_values = collections.defaultdict(str)
    for key in BLP.ORDERED_KEY:
        if param.has_key(key):
            param_values[key] = param[key]
    line = ','.join(map(str, [param_values[k] for k in BLP.ORDERED_KEY]))
    #line = ','.join((str(param[k]) if k in param.keys() else '' for k in BLP.ORDERED_KEY))
    fileobj.write(line+'\n')

def as_maskstring(masklist):
    return ';'.join(map(lambda x: '%s~%s'%(x[0],x[1]), masklist))

def generate_plot_table(ms_id, antenna_id, spw_id, polarization_ids, grid_table):
    def _filter(msid, ant, spw, pols, table):
        for row in table:
            if row[0] == spw and row[1] in pols:
                new_row_entry = row[2:6] + [numpy.array([r[3] for r in row[6] if r[-1] == msid and r[-2] == ant], dtype=int)]
                yield new_row_entry
    new_table = list(_filter(ms_id, antenna_id, spw_id, polarization_ids, grid_table))
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

    def __init__(self, inputs):
        super(BaselineFitParamConfig, self).__init__(inputs)
        self.paramdict = {}
    
    # readonly attributes    
    @property
    def ClipCycle(self):
        return 1
    
    def is_multi_vis_task(self):
        return False
    
    def prepare(self, datatable=None):
        LOG.debug('Starting BaselineFitParamConfig.prepare')
        fragmentation_heuristic = fragmentation.FragmentationHeuristics()

        # fitting order
        fit_order = self.inputs.fit_order
        if fit_order == 'automatic':
            # fit order heuristics
            LOG.info('Baseline-Fitting order was automatically determined')
            self.fitorder_heuristic = fitorder.FitOrderHeuristics()
        else:
            LOG.info('Baseline-Fitting order was fixed to be {}', fit_order)
            self.fitorder_heuristic = lambda *args, **kwargs: self.inputs.fit_order

        context = self.inputs.context
        vis = self.inputs.vis
        if datatable is None:
            LOG.info('#PNP# instantiate local datatable')
            datatable = DataTable(self.context.observing_run.ms_datatable_name)
        else:
            LOG.info('datatable is propagated from parent task')
        ms = context.observing_run.get_ms(vis)
        
        args = self.inputs.to_casa_args()

        vis = self.inputs.vis
        antenna_id = self.inputs.antenna_id
        field_id = self.inputs.field_id
        spw_id = self.inputs.spw_id
        LOG.debug('MS "{}" ant {} field {} spw {}', os.path.basename(vis),antenna_id,field_id,spw_id)
        
        nchan = ms.spectral_windows[spw_id].num_channels
        data_desc = ms.get_data_description(spw=spw_id)
        npol = data_desc.num_polarizations
        edge = common.parseEdge(self.inputs.edge)
        
        LOG.debug('nchan={nchan} edge={edge}', nchan=nchan,edge=edge)
        
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
        LOG.debug('Deviation mask for field {} antenna {} spw {}: {}',
                  field_id, antenna_id, spw_id, deviation_mask)
        if deviation_mask is not None:
            for mask_range in deviation_mask:
                mask_array[mask_range[0]:mask_range[1]] = 0
        
        base_mask_array = mask_array.copy()

        time_table = datatable.get_timetable(antenna_id, spw_id, None, os.path.basename(vis), field_id)
        member_list = time_table[timetable_index]
        
        # working with spectral data in scantable
        nrow_total = sum((len(x[0]) for x in member_list))
                
        LOG.info('Calculating Baseline Fitting Parameter...')
        LOG.info('Processing {} spectra...', nrow_total)
        
        colname = self.inputs.colname
        LOG.debug('data column name is "{}"', colname)
        
        # open blparam file (append mode)
        with open(args['blparam'], 'a') as blparamfileobj:
        
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
                    #spectra = numpy.asarray([tb.getcell(colname, row).real for row in rows])
                    spectra = numpy.zeros((len(rows), npol, nchan,), dtype=numpy.float32)
                    for (i,row) in enumerate(rows):
                        spectra[i] = tb.getcell(colname, row).real
                    #get_mask_from_flagtra: 1 valid 0 invalid
                    #arg for mask_to_masklist: 0 valid 1 invalid
                    #flaglist = [self._mask_to_masklist(-sdutils.get_mask_from_flagtra(tb.getcell('FLAGTRA', row))+1 )
                    flaglist = [self._mask_to_masklist(tb.getcell('FLAG', row).astype(int))
                                for row in rows]
    
                    #LOG.trace("Flag Mask = %s" % str(flaglist))
    
                spectra[:,:edge[0],:] = 0.0
                spectra[:,nchan-edge[1]:,:] = 0.0 
                    
                # here we assume that masklist is polarization-independent
                # (this is because that line detection/validation process accumulates 
                # polarization components together
                masklist = [datatable.tb2.getcell('MASKLIST',idx)
                            for idx in idxs]
    #                 masklist = [datatable.tb2.getcell('MASKLIST',idxs[i]) + flaglist[i]
    #                             for i in range(len(idxs))]
                LOG.debug('DONE {}', y)
                
                npol = spectra.shape[1]
                for pol in xrange(npol):
                    # fit order determination
                    polyorder = self.fitorder_heuristic(spectra[:,pol,:], [ list(masklist[i]) + flaglist[i][pol] for i in range(len(idxs))], edge)
                    #del spectra
                    if fit_order == 'automatic' and self.MaxPolynomialOrder != 'none':
                        polyorder = min(polyorder, self.MaxPolynomialOrder)
                    LOG.debug('time group {} pol {}: fitting order={}',
                              y,pol,polyorder)
                    
                    # calculate fragmentation
                    (fragment, nwindow, win_polyorder) = fragmentation_heuristic(polyorder, nchan, edge)
        
                    nrow = len(rows)
                    LOG.debug('nrow = {}', nrow)
                    LOG.debug('len(idxs) = {}', len(idxs))
                
                    for i in xrange(nrow):
                        row = rows[i]
                        idx = idxs[i]
                        LOG.trace('===== Processing at row = {} =====', row)
                        #nochange = datatable.tb2.getcell('NOCHANGE',idx)
                        #LOG.trace('row = %s, Flag = %s'%(row, nochange))
    
                        # mask lines
                        maxwidth = 1
#                       _masklist = masklist[i] 
                        _masklist = list(masklist[i]) + flaglist[i][pol]
                        for [chan0, chan1] in _masklist:
                            if chan1 - chan0 >= maxwidth:
                                maxwidth = int((chan1 - chan0 + 1) / 1.4)
                                # allowance in Process3 is 1/5:
                                #    (1 + 1/5 + 1/5)^(-1) = (5/7)^(-1)
                                #                         = 7/5 = 1.4
                        max_polyorder = int((nchan - sum(edge)) / maxwidth + 1)
                        LOG.trace('Masked Region from previous processes = {}',
                                  _masklist)
                        LOG.trace('edge parameters= {}', edge)
                        LOG.trace('Polynomial order = {}  Max Polynomial order = {}', polyorder, max_polyorder)
    
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
                        LOG.trace('Row {}: param={}', row,param)
                        write_blparam(blparamfileobj, param)
                        
                    # MS rows contain npol spectra
                    if pol == 0:
                        index_list_total.extend(idxs)

        outcome = {'blparam': args['blparam'],
                   'bloutput': args['bloutput']}
        results = BaselineSubtractionResults(success=True, outcome=outcome)
        
        return results

    def analyse(self, results):
        return results
    
    #@sdutils.profiler
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

        LOG.trace('nchan_without_edge, num_mask, diff={}, {}',
                  nchan_without_edge, num_mask)

        outdata = self._get_param(row_idx, pol, polyorder, nchan, mask, edge, nchan_without_edge, num_mask, fragment, nwindow, win_polyorder, masklist_all)

        LOG.trace('outdata={}', outdata)

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
    
    def __init__(self, inputs):
        super(CubicSplineFitParamConfig, self).__init__(inputs)
        
        # constant stuff
        self.paramdict[BLP.FUNC] = 'cspline'
        self.paramdict[BLP.CLIPNITER] = self.ClipCycle
        self.paramdict[BLP.CLIPTHRESH] = 5.0
    
    def _get_param(self, idx, pol, polyorder, nchan, mask, edge, nchan_without_edge, nchan_masked, fragment, nwindow, win_polyorder, masklist):
        num_nomask = nchan_without_edge - nchan_masked
        num_pieces = max(int(min(polyorder * num_nomask / float(nchan_without_edge) + 0.5, 0.1 * num_nomask)), 1)
        LOG.trace('Cubic Spline Fit: Number of Sections = {}', num_pieces)
        self.paramdict[BLP.ROW] = idx
        self.paramdict[BLP.POL] = pol
        self.paramdict[BLP.MASK] = masklist
        self.paramdict[BLP.NPIECE] = num_pieces
        return self.paramdict

class BaselineSubtractionWorkerInputs(BaselineSubtractionInputsBase):
    def __init__(self, context, vis=None,  
                 fit_order=None, edge=None, deviationmask_list=None, blparam=None, bloutput=None, 
                 grid_table_list=None, channelmap_range_list=None):
        self._init_properties(vars())
        
class BaselineSubtractionWorker(basetask.StandardTaskTemplate):
    Inputs = BaselineSubtractionWorkerInputs
    SubTask = None
    
    def prepare(self, datatable=None, process_list=None, deviationmask_list=None):
        context = self.inputs.context
        vis = self.inputs.vis
        ms = self.inputs.ms
        fit_order = self.inputs.fit_order
        edge = self.inputs.edge
        args = self.inputs.to_casa_args()
        blparam = args['blparam']
        bloutput = args['bloutput']
        outfile = args['outfile']
        datacolumn = args['datacolumn']
        
        assert process_list is not None
        assert deviationmask_list is not None
        field_id_list, antenna_id_list, spw_id_list = process_list.get_process_list()
        assert len(field_id_list) == len(deviationmask_list)
        
        # initialization of blparam file
        # blparam file needs to be removed before starting iteration through 
        # reduction group
        if os.path.exists(blparam):
            LOG.debug('Cleaning up blparam file for {vis}', vis=vis)
            os.remove(blparam)        
        
        for (field_id, antenna_id, spw_id, deviationmask) in \
            zip(field_id_list, antenna_id_list, spw_id_list, deviationmask_list):
            inputs = self.SubTask.Inputs(context, vis=vis, field_id=field_id,
                                         antenna_id=antenna_id, spw_id=spw_id,
                                         fit_order=fit_order, edge=edge, 
                                         deviationmask=deviationmask, blparam=blparam, 
                                         bloutput=bloutput)
            task = self.SubTask(inputs)
            job = common.ParameterContainerJob(task, datatable=datatable)
            subtask_results = self._executor.execute(job, merge=False)
            
        # execute tsdbaseline
        job = casa_tasks.sdbaseline(infile=vis, datacolumn=datacolumn, blmode='fit', dosubtract=True,
                                     blformat='table', bloutput=bloutput, 
                                     blfunc='variable', blparam=blparam,
                                     outfile=outfile, overwrite=True)
        self._executor.execute(job)
            
        outcome = {'blparam': blparam,
                   'bloutput': bloutput,
                   'outfile': outfile}
        results = BaselineSubtractionResults(success=True, outcome=outcome)
        return results
        
    def analyse(self, results):
        return results
                
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
    
