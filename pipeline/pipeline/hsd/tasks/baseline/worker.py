from __future__ import absolute_import

import os
import numpy
import types
import itertools

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.sdfilenamer as filenamer
from pipeline.hsd.heuristics import CubicSplineFitParamConfig
from pipeline.domain import DataTable
from .. import common
from ..common import compress
from pipeline.hsd.tasks.common import utils as sdutils
from pipeline.infrastructure import casa_tasks

_LOG = infrastructure.get_logger(__name__)
LOG = sdutils.OnDemandStringParseLogger(_LOG)


def generate_plot_table(ms_id, antenna_id, spw_id, polarization_ids, grid_table):
    def _filter(msid, ant, spw, pols, table):
        for row in table:
            if row[0] == spw and row[1] in pols:
                new_row_entry = row[2:6] + [numpy.array([r[3] for r in row[6] if r[-1] == msid and r[-2] == ant],
                                                        dtype=int)]
                yield new_row_entry
    new_table = list(_filter(ms_id, antenna_id, spw_id, polarization_ids, grid_table))
    return new_table


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


class OldBaselineSubtractionInputsBase(basetask.StandardInputs):
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
        if ('outfile' not in args or
                args['outfile'] is None or
                len(args['outfile']) == 0):
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
    plan = vdp.VisDependentProperty(default=[])
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
        

class BaselineSubtractionWorker(basetask.StandardTaskTemplate):
    Inputs = BaselineSubtractionWorkerInputs
    Heuristics = None
    
    def prepare(self):
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
        
        process_list = self.inputs.plan
        deviationmask_list = self.inputs.deviationmask
        
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
        
        datatable = DataTable(context.observing_run.ms_datatable_name)

        for (field_id, antenna_id, spw_id) in process_list.iterate_id():
            if (field_id, antenna_id, spw_id) in deviationmask_list:
                deviationmask = deviationmask_list[(field_id, antenna_id, spw_id)]
            else:
                deviationmask = None
            blparam_heuristic = self.Heuristics()
            formatted_edge = list(common.parseEdge(edge))
            out_blparam = blparam_heuristic(datatable, ms, antenna_id, field_id, spw_id, 
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
        return results
                

class CubicSplineBaselineSubtractionWorker(BaselineSubtractionWorker):
    Heuristics = CubicSplineFitParamConfig


# facade for FitParam
class BaselineSubtractionInputs(vdp.ModeInputs):
    _modes = {'spline': CubicSplineBaselineSubtractionWorker,
              'cspline': CubicSplineBaselineSubtractionWorker}

    def __init__(self, context, fitfunc, **parameters):
        super(BaselineSubtractionInputs, self).__init__(context=context, mode=fitfunc, **parameters)
    

class BaselineSubtractionTask(basetask.ModeTask):
    Inputs = BaselineSubtractionInputs
