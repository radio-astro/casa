from __future__ import absolute_import

from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.hif.tasks.common import calibrationtableaccess as caltableaccess

from ..tsysflagspectra import TsysflagspectraInputs, Tsysflagspectra
from ..tsysflagchans import TsysflagchansInputs, Tsysflagchans
from .resultobjects import TsysflagResults

LOG = infrastructure.get_logger(__name__)


class TsysflagInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, caltable=None, 
      flag_nmedian=None, fnm_limit=None,
      flag_derivative=None, fd_max_limit=None,
      flag_edgechans=None, fe_edge_limit=None,
      flag_fieldshape=None, ff_refintent=None, ff_max_limit=None,
      ff_tmf1_limit=None, 
      flag_birdies=None, fb_sharps_limit=None):

        # set the properties to the values given as input arguments
        self._init_properties(vars())

        # Default order in which flagging metrics are recommended
        # to be evaluated.
        #default_metric_order = 'nmedian, derivative, edgechans, fieldshape, birdies'
        default_metric_order = 'nmedian, derivative, fieldshape, edgechans, birdies'
        
        # Initialize order in which flagging metrics will be evaluated,
        # set to default order if not provided.
        # TODO: "metric_order" could become an argument available to users.
        metric_order = None
        if metric_order is None:
            metric_order = default_metric_order

        # Convert metric string to list
        metric_list = [metric.strip() for metric in default_metric_order.split(',')]
        
        # Initialize ordered list of metrics to evaluate
        self.metrics_to_evaluate = []
        
        # Convert requested flagging metrics to ordered list
        # FIXME: if a requested flagging metric is missing from 
        # the "metric order", it is skipped; raise warning in such case?
        for metric in metric_list:
            if metric == 'nmedian' and self.flag_nmedian:
                self.metrics_to_evaluate.append(metric)
            if metric == 'derivative' and self.flag_derivative:
                self.metrics_to_evaluate.append(metric)
            if metric == 'edgechans' and self.flag_edgechans:
                self.metrics_to_evaluate.append(metric)
            if metric == 'fieldshape' and self.flag_fieldshape:
                self.metrics_to_evaluate.append(metric)
            if metric == 'birdies' and self.flag_birdies:
                self.metrics_to_evaluate.append(metric)

    @property
    def caltable(self):
        if self._caltable is None:
            caltables = self.context.callibrary.active.get_caltable(
              caltypes='tsys')

            # return just the tsys table that matches the vis being handled
            result = None
            for name in caltables:
                # Get the tsys table name
                tsystable_vis = \
                  caltableaccess.CalibrationTableDataFiller._readvis(name)
                if tsystable_vis in self.vis:
                    result = name
                    break

            return result

        return self._caltable

    @caltable.setter
    def caltable(self, value):
        self._caltable = value

    @property
    def flag_nmedian(self):
        return self._flag_nmedian

    @flag_nmedian.setter
    def flag_nmedian(self, value):
        if value is None:
            value = True
        self._flag_nmedian = value
 
    @property
    def fnm_limit(self):
        return self._fnm_limit

    @fnm_limit.setter
    def fnm_limit(self, value):
        if value is None:
            value = 2.0
        self._fnm_limit = value

    @property
    def flag_derivative(self):
        return self._flag_derivative

    @flag_derivative.setter
    def flag_derivative(self, value):
        if value is None:
            value = True
        self._flag_derivative = value

    @property
    def fd_max_limit(self):
        return self._fd_max_limit

    @fd_max_limit.setter
    def fd_max_limit(self, value):
        if value is None:
            value = 5
        self._fd_max_limit = value

    @property
    def flag_edgechans(self):
        return self._flag_edgechans

    @flag_edgechans.setter
    def flag_edgechans(self, value):
        if value is None:
            value = True
        self._flag_edgechans = value

    @property
    def fe_edge_limit(self):
        return self._fe_edge_limit

    @fe_edge_limit.setter
    def fe_edge_limit(self, value):
        if value is None:
            value = 3.0
        self._fe_edge_limit = value

    @property
    def flag_fieldshape(self):
        return self._flag_fieldshape

    @flag_fieldshape.setter
    def flag_fieldshape(self, value):
        if value is None:
            value = True
        self._flag_fieldshape = value

    @property
    def ff_refintent(self):
        return self._ff_refintent

    @ff_refintent.setter
    def ff_refintent(self, value):
        if value is None:
            value = 'BANDPASS'
        self._ff_refintent = value

    @property
    def ff_max_limit(self):
        return self._ff_max_limit

    @ff_max_limit.setter
    def ff_max_limit(self, value):
        if value is None:
            value = 5
        self._ff_max_limit = value

    @property
    def ff_tmf1_limit(self):
        return self._ff_tmf1_limit

    @ff_tmf1_limit.setter
    def ff_tmf1_limit(self, value):
        if value is None:
            value = 0.666
        self._ff_tmf1_limit = value

    @property
    def flag_birdies(self):
        return self._flag_birdies

    @flag_birdies.setter
    def flag_birdies(self, value):
        if value is None:
            value = True
        self._flag_birdies = value

    @property
    def fb_sharps_limit(self):
        return self._fb_sharps_limit

    @fb_sharps_limit.setter
    def fb_sharps_limit(self, value):
        if value is None:
            value = 0.05
        self._fb_sharps_limit = value


class Tsysflag(basetask.StandardTaskTemplate):
    Inputs = TsysflagInputs

    def prepare(self):
        
        inputs = self.inputs
        
        # Initialize the final result
        result = TsysflagResults()

        # Create a pre-flagging summary of the flags
        summary_job = casa_tasks.flagdata(vis=inputs.caltable, mode='summary')
        stats_before = self._executor.execute(summary_job)

        # Run flagger for each metric
        for metric in inputs.metrics_to_evaluate:
            result.add(metric, self.run_flagger(metric))

        # Create a post-flagging summary of the flags
        summary_job = casa_tasks.flagdata(vis=inputs.caltable, mode='summary')
        stats_after = self._executor.execute(summary_job)
        
        # Add the "before" and "after" flagging summaries to the final result
        result.summaries = [stats_before, stats_after]

        # Store order of metrics in result
        result.metric_order = inputs.metrics_to_evaluate

        return result

    def analyse(self, result):
        return result
    
    def run_flagger(self, metric):

        inputs = self.inputs
        LOG.info('flag '+metric)

        # Flag Tsys spectra on basis of 'median' metric.
        if metric == 'nmedian':
            flaginputs = TsysflagspectraInputs(
              context=inputs.context,
              output_dir=inputs.output_dir,
              vis=inputs.vis,
              caltable=inputs.caltable,
              metric='median',
              flag_nmedian=True,
              fnm_limit = inputs.fnm_limit,
              flag_hi=False,
              flag_maxabs=False,
              flag_tmf1=False,
              prepend='flag nmedian - ')
            flagtask = Tsysflagspectra(flaginputs)
            
        # Flag Tsys spectra on basis of 'derivative' metric.
        if metric == 'derivative':
            flaginputs = TsysflagspectraInputs(
              context=inputs.context,
              output_dir=inputs.output_dir,
              vis=inputs.vis,
              caltable=inputs.caltable,
              metric='derivative',
              flag_maxabs=True,
              fmax_limit=inputs.fd_max_limit,
              flag_nmedian=False,
              flag_hi=False,
              flag_tmf1=False,
              prepend='flag derivative - ')
            flagtask = Tsysflagspectra(flaginputs)

        # Flag edge channels of Tsys spectra.
        if metric == 'edgechans':
            flaginputs = TsysflagchansInputs(
              context=inputs.context,
              output_dir=inputs.output_dir,
              vis=inputs.vis,
              caltable=inputs.caltable,
              intentgroups="['ATMOSPHERE','BANDPASS','AMPLITUDE']",
              flag_edges=True,
              edge_limit=inputs.fe_edge_limit,
              flag_sharps=False,
              prepend='flag edgechans - ')
            flagtask = Tsysflagchans(flaginputs)

        # Flag Tsys spectra on basis of 'fieldshape' metric.
        if metric == 'fieldshape':
            flaginputs = TsysflagspectraInputs(
              context=inputs.context,
              output_dir=inputs.output_dir,
              vis=inputs.vis,
              caltable=inputs.caltable,
              metric='fieldshape',
              refintent=inputs.ff_refintent,
              flag_maxabs=True,
              fmax_limit=inputs.ff_max_limit,
              flag_tmf1=True,
              tmf1_axis='Antenna1',
              tmf1_limit=inputs.ff_tmf1_limit,
              flag_nmedian=False,
              flag_hi=False,
              prepend='flag fieldshape - ')
            flagtask = Tsysflagspectra(flaginputs)

        # Flag birdies in Tsys spectra.
        if metric == 'birdies':
            flaginputs = TsysflagchansInputs(
              context=inputs.context,
              output_dir=inputs.output_dir,
              vis=inputs.vis,
              caltable=inputs.caltable,
              metric='antenna_diff',
              flag_edges=False,
              flag_sharps=True,
              sharps_limit=inputs.fb_sharps_limit,
              prepend='flag birdies - ')
            flagtask = Tsysflagchans(flaginputs)
        
        # Return the result from the executed flagger task
        return self._executor.execute(flagtask)
