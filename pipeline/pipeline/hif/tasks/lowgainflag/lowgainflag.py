from __future__ import absolute_import

import numpy as np 
import os
import types

from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure as infrastructure
from pipeline.hif.tasks.common import commoncalinputs
import pipeline.infrastructure.basetask as basetask
from pipeline.hif.tasks.flagging.flagdatasetter import FlagdataSetter

from .resultobjects import LowgainflagResults
from ..common import commonresultobjects
from ..common import calibrationtableaccess as caltableaccess
from ..common import viewflaggers
from .. import bandpass
from .. import gaincal

LOG = infrastructure.get_logger(__name__)


class LowgainflagInputs(commoncalinputs.CommonCalibrationInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, 
      intent=None, spw=None, refant=None, flag_nmedian=None,
      fnm_lo_limit=None, fnm_hi_limit=None, niter=None):

        # set the properties to the values given as input arguments
        self._init_properties(vars())

    # standard calibration getters and setters are inherited from
    # CommonCalibrationInputs

    @property
    def intent(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('intent')

        if self._intent is None:
            # default to the intent that would be used for bandpass
            # calibration
            bp_inputs = bandpass.PhcorBandpass.Inputs(
              context=self.context,
              vis=self.vis,
              intent=None)
            value = bp_inputs.intent
            return value

        return self._intent

    @intent.setter
    def intent(self, value):
        self._intent = value

    # flag 
    @property
    def flag_nmedian(self):
        return self._flag_nmedian

    @flag_nmedian.setter
    def flag_nmedian(self, value):
        if value is None:
            value = True
        self._flag_nmedian = value

    @property
    def fnm_lo_limit(self):
        return self._fnm_lo_limit

    @fnm_lo_limit.setter
    def fnm_lo_limit(self, value):
        if value is None:
            value = 0.7
        self._fnm_lo_limit = value

    @property
    def fnm_hi_limit(self):
        return self._fnm_hi_limit

    @fnm_hi_limit.setter
    def fnm_hi_limit(self, value):
        if value is None:
            value = 1.3
        self._fnm_hi_limit = value

    @property
    def niter(self):
        return self._niter

    @niter.setter
    def niter(self, value):
        if value is None:
            value = 1
        self._niter = value


class Lowgainflag(basetask.StandardTaskTemplate):
    Inputs = LowgainflagInputs

    def prepare(self):
        inputs = self.inputs

        # Construct the task that will read the data and create the
        # view of the data that is the basis for flagging.
        datainputs = LowgainflagWorkerInputs(context=inputs.context,
          output_dir=inputs.output_dir, vis=inputs.vis, intent=inputs.intent,
          spw=inputs.spw, refant=inputs.refant)
        datatask = LowgainflagWorker(datainputs)

        # Construct the task that will set any flags raised in the
        # underlying data.
        flagsetterinputs = FlagdataSetter.Inputs(context=inputs.context,
          vis=inputs.vis, table=inputs.vis, inpfile=[])
        flagsettertask = FlagdataSetter(flagsetterinputs)

        # Translate the input flagging parameters to a more compact
        # list of rules.
	rules = viewflaggers.MatrixFlagger.make_flag_rules (
          flag_nmedian=inputs.flag_nmedian,
          fnm_lo_limit=inputs.fnm_lo_limit,
          fnm_hi_limit=inputs.fnm_hi_limit)

        # Construct the flagger task around the data view task  and the
        # flagger task. When executed this will:
        #   loop:
        #     execute datatask to obtain view from underlying data
        #     examine view, raise flags
        #     execute flagsetter task to set flags in underlying data        
        #     exit loop if no flags raised or if # iterations > niter 
        matrixflaggerinputs = viewflaggers.MatrixFlaggerInputs(
          context=inputs.context, output_dir=inputs.output_dir,
          vis=inputs.vis, datatask=datatask, flagsettertask=flagsettertask,
          rules=rules, niter=inputs.niter,
          extendfields=['field', 'timerange'])
        flaggertask = viewflaggers.MatrixFlagger(matrixflaggerinputs)

        # Execute it to flag the data view
        summary_job = casa_tasks.flagdata(vis=inputs.vis, mode='summary')
        stats_before = self._executor.execute(summary_job)
        result = self._executor.execute(flaggertask)
        summary_job = casa_tasks.flagdata(vis=inputs.vis, mode='summary')
        stats_after = self._executor.execute(summary_job)
        
        result.summaries = [stats_before, stats_after]

        return result

    def analyse(self, result):
        return result


class LowgainflagWorkerInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None, vis=None, 
      intent=None, spw=None, refant=None):
        self._init_properties(vars())

        self.gacal_inputs = gaincal.GTypeGaincal.Inputs(
          context=context, vis=vis,
          intent=intent, spw=spw,
          refant=refant, calmode='a', minsnr=2.0,
          solint='inf', gaintype='T')

    @property
    def caltable(self):
        return self.gacal_inputs.caltable


class LowgainflagWorker(basetask.StandardTaskTemplate):
    Inputs = LowgainflagWorkerInputs

    def __init__(self, inputs):
        super(LowgainflagWorker, self).__init__(inputs)
        self.result = LowgainflagResults()
        self.result.vis = inputs.vis

    def prepare(self):
        inputs = self.inputs

        # Calculate a phased-up bpcal
        bpcal_inputs = bandpass.PhcorBandpass.Inputs(
          context=inputs.context, vis=inputs.vis,
          intent=inputs.intent, spw=inputs.spw, 
          refant=inputs.refant, maxchannels=0,
	  solint='inf,7.8125MHz')
        bpcal_task = bandpass.PhcorBandpass(bpcal_inputs)
        bpcal = self._executor.execute(bpcal_task, merge=True)

        # Calculate gain phases
        gpcal_inputs = gaincal.GTypeGaincal.Inputs(
          context=inputs.context, vis=inputs.vis,
          intent=inputs.intent, spw=inputs.spw,
          refant=inputs.refant,
          calmode='p', minsnr=2.0, solint='int', gaintype='G')
        gpcal_task = gaincal.GTypeGaincal(gpcal_inputs)
        gpcal = self._executor.execute(gpcal_task, merge=True)

        # Calculate gain amplitudes
        gacal_inputs = gaincal.GTypeGaincal.Inputs(
          context=inputs.context, vis=inputs.vis,
          intent=inputs.intent, spw=inputs.spw,
          refant=inputs.refant,
          calmode='a', minsnr=2.0, solint='inf', gaintype='T')
        gacal_task = gaincal.GTypeGaincal(gacal_inputs)
        gacal = self._executor.execute(gacal_task, merge=True)

        # Get the gacal table name
        gatable = gacal.final
        if len(gatable) != 1:
            raise Exception, 'not exactly one gain table produced'
        gatable = gatable[0].gaintable

        LOG.info ('Computing flagging metrics for caltable %s ' % (
          os.path.basename(gatable)))
        self.calculate_view(gatable)

        return self.result

    def analyse(self, result):
        return result

    def calculate_view(self, table):
        """
        table -- Name of gain table to be analysed.
        spwid -- view will be calculated using data for this spw id.
        """
        gtable = caltableaccess.CalibrationTableDataFiller.getcal(table)

        ms = self.inputs.context.observing_run.get_ms(name=self.inputs.vis)
        antenna_ids = [antenna.id for antenna in ms.antennas] 
        antenna_ids.sort()
        antenna_name = {}
        for antenna_id in antenna_ids:
            antenna_name[antenna_id] = [antenna.name for antenna in ms.antennas 
              if antenna.id==antenna_id][0]

        # spw arg may contain channel specification. Let MS parse input and
        # read spw ID from the SpectralWindow domain objects that are returned
        spwids = [spw.id for spw in self.inputs.ms.get_spectral_windows(self.inputs.spw)]

        # get range of times covered
        times = set()
        for row in gtable.rows:
            # The gain table is T, should be no pol dimension
            npols = np.shape(row.get('CPARAM'))[0]
            if npols != 1:
                raise Exception, 'table has polarization results'
            times.update([row.get('TIME')])
        times = np.sort(list(times))

        # times in gain table sometimes show jitter - either perhaps
        # resulting from different flagging for different antenna/spw,
        # or from out of sync timing in raw data (a problem now cured
        # I'm told, Mar-2014).
        # Ignore time differences smaller than 5sec.
        filtered_times = []
        last_time = 0.0
        for timestamp in times:
            if timestamp - last_time > 5.0:
                filtered_times.append(timestamp)
                last_time = timestamp
        times = np.array(filtered_times)

        # make gain image for each spwid
        for spwid in spwids:
            data = np.zeros([antenna_ids[-1]+1, len(times)])
            flag = np.ones([antenna_ids[-1]+1, len(times)], np.bool)

            for row in gtable.rows:
                if row.get('SPECTRAL_WINDOW_ID') == spwid:
                    gain = row.get('CPARAM')[0][0]
#                    print 'GETTING SNR'
#                    gain = row.get('SNR')[0][0]
                    ant = row.get('ANTENNA1')
                    caltime = row.get('TIME')
                    gainflag = row.get('FLAG')[0][0]
                    if not gainflag:
                        data[ant, np.abs(times-caltime) < 5] = np.abs(gain)
                        flag[ant, np.abs(times-caltime) < 5] = 0

            axes = [commonresultobjects.ResultAxis(name='Antenna1',
              units='id', data=np.arange(antenna_ids[-1]+1)),
              commonresultobjects.ResultAxis(name='Time', units='',
              data=times)]

            viewresult = commonresultobjects.ImageResult(filename=
              os.path.basename(table), intent=self.inputs.intent,
              data=data, flag=flag,
              axes=axes, datatype='gain amplitude', spw=spwid)
          
            # add the view results and their children results to the
            # class result structure
            self.result.addview(viewresult.description, viewresult)
