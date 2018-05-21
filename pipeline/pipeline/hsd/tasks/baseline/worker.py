from __future__ import absolute_import

import os
import numpy
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.sdfilenamer as filenamer
from pipeline.hsd.heuristics import CubicSplineFitParamConfig
from pipeline.domain import DataTable
from .. import common
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
    

class BaselineSubtractionResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(BaselineSubtractionResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(BaselineSubtractionResults, self).merge_with_context(context)
    
    def _outcome_name(self):
        # outcome should be a name of blparam text file
        return 'blparam: "%s" bloutput: "%s"' % (self.outcome['blparam'], self.outcome['bloutput'])


class BaselineSubtractionWorkerInputs(BaselineSubtractionInputsBase):
    def __init__(self, context, vis=None,  
                 fit_order=None, edge=None, deviationmask_list=None, blparam=None, bloutput=None, 
                 grid_table_list=None, channelmap_range_list=None):
        self._init_properties(vars())
        

class BaselineSubtractionWorker(basetask.StandardTaskTemplate):
    Inputs = BaselineSubtractionWorkerInputs
    Heuristics = None
    
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
        
        # initialization of blparam file
        # blparam file needs to be removed before starting iteration through 
        # reduction group
        if os.path.exists(blparam):
            LOG.debug('Cleaning up blparam file for {vis}', vis=vis)
            os.remove(blparam)        
        
        if datatable is None:
            LOG.info('#PNP# instantiate local datatable')
            datatable = DataTable(context.observing_run.ms_datatable_name)
        else:
            LOG.info('datatable is propagated from parent task')

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
    Heuristics = CubicSplineFitParamConfig


# facade for FitParam
class BaselineSubtractionInputs(basetask.ModeInputs):
    _modes = {'spline': CubicSplineBaselineSubtractionWorker,
              'cspline': CubicSplineBaselineSubtractionWorker}

    def __init__(self, context, fitfunc, **parameters):
        super(BaselineSubtractionInputs, self).__init__(context=context, mode=fitfunc, **parameters)
    

class BaselineSubtractionTask(basetask.ModeTask):
    Inputs = BaselineSubtractionInputs
