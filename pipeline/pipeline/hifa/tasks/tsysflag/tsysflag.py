from __future__ import absolute_import

from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
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
      flag_birdies=None, fb_sharps2_limit=None):

        # set the properties to the values given as input arguments
        self._init_properties(vars())

    @property
    def caltable(self):
        if self._caltable is None:
            caltables = self.context.callibrary.active.get_caltable(
              caltypes='tsys')

            # return just the tsys table that matches the vis being handled
            result = None
            for name in caltables:
                # Get the tsys table name
                tsystable = caltableaccess.CalibrationTableDataFiller.getcal(
                  name)
                if tsystable.vis in self.vis:
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
            value = 0.5
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
    def fb_sharps2_limit(self):
        return self._fb_sharps2_limit

    @fb_sharps2_limit.setter
    def fb_sharps2_limit(self, value):
        if value is None:
            value = 0.05
        self._fb_sharps2_limit = value


class Tsysflag(basetask.StandardTaskTemplate):
    Inputs = TsysflagInputs

    def prepare(self):
        inputs = self.inputs
        result = TsysflagResults()

        summary_job = casa_tasks.flagdata(vis=inputs.caltable, mode='summary')
        stats_before = self._executor.execute(summary_job)

        if inputs.flag_nmedian:
            LOG.info('flag nmedian')
            # Flag Tsys spectra on basis of 'median' metric.
            flaginputs = TsysflagspectraInputs(
              context=inputs.context,
              output_dir=inputs.output_dir,
              vis=inputs.vis,
              caltable=inputs.caltable,
              metric='median',
              flag_nmedian=True,
              fnm_limit = inputs.fnm_limit)

            flagtask = Tsysflagspectra(flaginputs)
            # Execute it to flag the data view
            result.add('nmedian', self._executor.execute(flagtask))

        if inputs.flag_derivative:
            LOG.info('flag derivative')
            # Flag Tsys spectra on basis of 'derivative' metric.
            flaginputs = TsysflagspectraInputs(
              context=inputs.context,
              output_dir=inputs.output_dir,
              vis=inputs.vis,
              caltable=inputs.caltable,
              metric='derivative',
              flag_nmedian=False,
              flag_maxabs=True,
              fmax_limit=inputs.fd_max_limit)

            flagtask = Tsysflagspectra(flaginputs)
            result.add('derivative', self._executor.execute(flagtask))

        if inputs.flag_edgechans:
            LOG.info('flag edgechans')
            # Flag edge channels of Tsys spectra.
            flaginputs = TsysflagchansInputs(
              context=inputs.context,
              output_dir=inputs.output_dir,
              vis=inputs.vis,
              caltable=inputs.caltable,
              intentgroups="['ATMOSPHERE,BANDPASS,AMPLITUDE']",
              flag_edges=True,
              edge_limit=inputs.fe_edge_limit,
              flag_sharps=False,
              flag_sharps2=False)

            flagtask = Tsysflagchans(flaginputs)
            result.add('edgechans', self._executor.execute(flagtask))

        if inputs.flag_fieldshape:
            LOG.info('flag fieldshape')
            # Flag Tsys spectra on basis of 'fieldshape' metric.
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
              flag_nmedian=False)

            flagtask = Tsysflagspectra(flaginputs)
            result.add('fieldshape', self._executor.execute(flagtask))

        if inputs.flag_birdies:
            LOG.info('flag birdies')
            # Flag birdies in Tsys spectra.
            flaginputs = TsysflagchansInputs(
              context=inputs.context,
              output_dir=inputs.output_dir,
              vis=inputs.vis,
              caltable=inputs.caltable,
              metric='antenna_diff',
              flag_edges=False,
              flag_sharps=False,
              flag_sharps2=True,
              sharps2_limit=inputs.fb_sharps2_limit)

            flagtask = Tsysflagchans(flaginputs)
            result.add('birdies', self._executor.execute(flagtask))

        summary_job = casa_tasks.flagdata(vis=inputs.caltable, mode='summary')
        stats_after = self._executor.execute(summary_job)
        
        result.summaries = [stats_before, stats_after]
        return result

    def analyse(self, result):
        return result
