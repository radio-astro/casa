from __future__ import absolute_import

import numpy as np 
import os
import types

from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
from pipeline.hif.tasks.common import commonhelpermethods
from pipeline.hif.tasks.flagging.flagdatasetter import FlagdataSetter

from .resultobjects import FlagcorrectedResults
from ..common import commonresultobjects
from ..common import viewflaggers

LOG = infrastructure.get_logger(__name__)


class FlagcorrectedInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, spw=None,
      intent=None, metric=None,
      flag_bad_antenna=None, fba_lo_limit=None, fba_minsample=None,
      fba_frac_limit=None, fba_number_limit=None,
      niter=None):

        # set the properties to the values given as input arguments
        self._init_properties(vars())

    @property
    def spw(self):
        if self._spw is not None:
            return str(self._spw)

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('spw')

        science_spws = self.ms.get_spectral_windows(self._spw, with_channels=True)
        return ','.join([str(spw.id) for spw in science_spws])

    @spw.setter
    def spw(self, value):
        self._spw = value

    @property
    def intent(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('intent')
        return self._intent

    @intent.setter
    def intent(self, value):
        if value is None:
            value = 'BANDPASS'
        self._intent = value

    @property
    def metric(self):
        return self._metric

    @metric.setter
    def metric(self, value):
        if value is None:
            value = 'antenna'
        self._metric = value

    @property
    def flag_bad_antenna(self):
        return self._flag_bad_antenna

    @flag_bad_antenna.setter
    def flag_bad_antenna(self, value):
        if value is None:
            value = True
        self._flag_bad_antenna = value

    @property
    def fba_lo_limit(self):
        return self._fba_lo_limit

    @fba_lo_limit.setter
    def fba_lo_limit(self, value):
        if value is None:
            value = 7.0
        self._fba_lo_limit = value

    @property
    def fba_minsample(self):
        return self._fba_minsample

    @fba_minsample.setter
    def fba_minsample(self, value):
        if value is None:
            value = 5
        self._fba_minsample = value

    @property
    def fba_frac_limit(self):
        return self._fba_frac_limit

    @fba_frac_limit.setter
    def fba_frac_limit(self, value):
        if value is None:
            value = 0.05
        self._fba_frac_limit = value

    @property
    def fba_number_limit(self):
        return self._fba_number_limit

    @fba_number_limit.setter
    def fba_number_limit(self, value):
        if value is None:
            value = 10
        self._fba_number_limit = value

    @property
    def niter(self):
        return self._niter

    @niter.setter
    def niter(self, value):
        if value is None:
            value = 1
        self._niter = value


class Flagcorrected(basetask.StandardTaskTemplate):
    Inputs = FlagcorrectedInputs

    def prepare(self):
        inputs = self.inputs

        # Construct the task that will read the data and create the
        # view of the data that is the basis for flagging.
        datainputs = FlagcorrectedWorkerInputs(context=inputs.context,
          output_dir=inputs.output_dir, vis=inputs.vis, spw=inputs.spw,
          intent=inputs.intent, metric=inputs.metric)
        datatask = FlagcorrectedWorker(datainputs)

        # Construct the task that will set any flags raised in the
        # underlying data.
        flagsetterinputs = FlagdataSetter.Inputs(context=inputs.context,
          vis=inputs.vis, inpfile=[], table=inputs.vis)
        flagsettertask = FlagdataSetter(flagsetterinputs)

        # Translate the input flagging parameters to a more compact
        # list of rules.
        rules = viewflaggers.MatrixFlagger.make_flag_rules (
          flag_bad_antenna=inputs.flag_bad_antenna, 
          fba_lo_limit=inputs.fba_lo_limit,
          fba_minsample=inputs.fba_minsample,
          fba_frac_limit=inputs.fba_frac_limit,
          fba_number_limit=inputs.fba_number_limit)
        flagger = viewflaggers.MatrixFlagger
 
        # Construct the flagger task around the data view task  and the
        # flagger task. When executed this will:
        #   loop:
        #     execute datatask to obtain view from underlying data
        #     examine view, raise flags
        #     execute flagsetter task to set flags in underlying data        
        #     exit loop if no flags raised or if # iterations > niter 
        flaggerinputs = flagger.Inputs(
          context=inputs.context, output_dir=inputs.output_dir,
          vis=inputs.vis, datatask=datatask, flagsettertask=flagsettertask,
          rules=rules, niter=inputs.niter)
        flaggertask = flagger(flaggerinputs)

        # Execute it to flag the data view
        summary_job = casa_tasks.flagdata(vis=inputs.vis, mode='summary', name='before')
        stats_before = self._executor.execute(summary_job)
        result = self._executor.execute(flaggertask)
        summary_job = casa_tasks.flagdata(vis=inputs.vis, mode='summary', name='after')
        stats_after = self._executor.execute(summary_job)

        result.summaries = [stats_before, stats_after]
        return result

    def analyse(self, result):
        return result


class FlagcorrectedWorkerInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None, vis=None, spw=None,
      intent=None, metric=None):
        self._init_properties(vars())


class FlagcorrectedWorker(basetask.StandardTaskTemplate):
    Inputs = FlagcorrectedWorkerInputs

    def __init__(self, inputs):
        super(FlagcorrectedWorker, self).__init__(inputs)
        self.result = FlagcorrectedResults()

    def prepare(self):
        inputs = self.inputs

        final = []

        self.result.vis = inputs.vis

        # Get the MS object.
        self.ms = inputs.context.observing_run.get_ms(name=self.result.vis)

        # Get the spws to use
        spwids = map(int, inputs.spw.split(','))

        # Get antenna names, ids
        self.antenna_name, self.antenna_ids = \
          commonhelpermethods.get_antenna_names(self.ms)

        LOG.info ('Computing flagging metrics for vis %s ' % (self.result.vis))

        # calculate views
        if inputs.metric == 'baseline':
            self.calculate_baseline_view(spwids, inputs.intent)
        elif inputs.metric == 'antenna':
            self.calculate_antenna_view(spwids, inputs.intent)
        else:
            raise Exception, 'bad metric: %s' % inputs.metric

        return self.result

    def analyse(self, result):
        return result

    def calculate_baseline_view(self, spwids, intent):
        """
        spwids -- views will be calculated using data for each spw id
                  in this list.
        """

        # the current view will be very similar to the last, if available.
        # For now approximate as being identical which will save having to
        # recalculate
        prev_descriptions = self.result.descriptions()
        if prev_descriptions:
            for description in prev_descriptions:
                prev_result = self.result.last(description)

                self.result.addview(description, prev_result)

            # EARLY RETURN
            return

        ants = np.array(self.antenna_ids)

        # now construct the views
        for spwid in spwids:
            corrs = commonhelpermethods.get_corr_products(self.ms, spwid)

            casatools.ms.open(self.inputs.vis)
            casatools.ms.msselect({'scanintent':'*%s*' % intent,
              'spw':str(spwid)})
            rec = casatools.ms.getdata(['time'])
            times = set(rec['time'])
            times = list(times)
            times.sort()
            times = np.array(times)
            ntimes = len(times)

            ants = np.array(self.antenna_ids)
            baselines = []
            for ant1 in ants:
                for ant2 in ants: 
                    baselines.append('%s&%s' % (ant1, ant2))
            nbaselines = len(baselines)

            axes = [
              commonresultobjects.ResultAxis(name='Time',
              units='', data=times),
              commonresultobjects.ResultAxis(name='Baseline',
              units='', data=np.array(baselines), channel_width=1)]

            data = np.zeros([len(corrs), ntimes, nbaselines])
            flag = np.ones([len(corrs), ntimes, nbaselines], np.bool)

            LOG.info('calculating flagging view for spw %s' % spwid)
            casatools.ms.iterinit(maxrows=500)
            casatools.ms.iterorigin()
            iterating = True
            while iterating:
                rec = casatools.ms.getdata(['corrected_data', 'flag', 'antenna1',
                  'antenna2', 'time'])
                if 'corrected_data' not in rec.keys():
                    break

                for row in range(np.shape(rec['corrected_data'])[2]):
                    ant1 = rec['antenna1'][row]
                    ant2 = rec['antenna2'][row]
                    tim = rec['time'][row]

                    if ant1==ant2:
                        continue

                    baseline1 = ant1 * (ants[-1] + 1) + ant2
                    baseline2 = ant2 * (ants[-1] + 1) + ant1

                    for icorr,corrlist in enumerate(corrs):
                        corrected_data = rec['corrected_data'][icorr,:,row]
                        corrected_flag = rec['flag'][icorr,:,row]

                        valid_data = corrected_data[corrected_flag==False]            
                        if len(valid_data):
                            med_data_real = np.median(valid_data.real)
                            med_data_imag = np.median(valid_data.imag)
                            med_data = abs(complex(med_data_real, med_data_imag))
                            data[icorr,times==tim,baseline1] = med_data
                            flag[icorr,times==tim,baseline1] = 0
                            data[icorr,times==tim,baseline2] = med_data
                            flag[icorr,times==tim,baseline2] = 0

                iterating = casatools.ms.iternext()

            casatools.ms.close()

            # store the views
            for icorr,corrlist in enumerate(corrs):
                corr = corrlist[0]

                viewresult = commonresultobjects.ImageResult(
                  filename=self.inputs.vis, data=data[icorr],
                  flag=flag[icorr], axes=axes, datatype='Mean amplitude',
                  spw=spwid, pol=corr, intent=intent)

                # add the view results to the result structure
                self.result.addview(viewresult.description, viewresult)

    def calculate_antenna_view(self, spwids, intent):
        """
        spwids -- views will be calculated using data for each spw id
                  in this list.
        """

        ants = np.array(self.antenna_ids)

        # now construct the views
        for spwid in spwids:
            corrs = commonhelpermethods.get_corr_products(self.ms, spwid)

            casatools.ms.open(self.inputs.vis)
            casatools.ms.msselect({'scanintent':'*%s*' % intent,
              'spw':str(spwid)})
            rec = casatools.ms.getdata(['time'])
            times = set(rec['time'])
            times = list(times)
            times.sort()
            times = np.array(times)
            ntimes = len(times)
            ants = np.array(self.antenna_ids)
            nants = np.max(ants) + 1

            axes = [
              commonresultobjects.ResultAxis(name='Antenna',
              units='', data=ants, channel_width=1),
              commonresultobjects.ResultAxis(name='Time',
              units='', data=times)]

            data = np.zeros([len(corrs), nants, ntimes], np.complex)
            ndata = np.zeros([len(corrs), nants, ntimes], np.int)
            flag = np.ones([len(corrs), nants, ntimes], np.bool)

            LOG.info('calculating flagging view for spw %s' % spwid)
            casatools.ms.iterinit(maxrows=500)
            casatools.ms.iterorigin()
            iterating = True
            while iterating:
                rec = casatools.ms.getdata(['corrected_data', 'flag', 'antenna1',
                  'antenna2', 'time'])
                if 'corrected_data' not in rec.keys():
                    break

                for row in range(np.shape(rec['corrected_data'])[2]):
                    ant1 = rec['antenna1'][row]
                    ant2 = rec['antenna2'][row]
                    tim = rec['time'][row]

                    if ant1==ant2:
                        continue

                    for icorr,corrlist in enumerate(corrs):
                        corrected_data = rec['corrected_data'][icorr,:,row]
                        corrected_flag = rec['flag'][icorr,:,row]

                        valid_data = corrected_data[corrected_flag==False]            
                        if len(valid_data):
                            med_data = np.median(valid_data)
                            data[icorr,ant1,times==tim] += med_data
                            data[icorr,ant2,times==tim] += med_data
                            ndata[icorr,ant1,times==tim] += 1
                            ndata[icorr,ant2,times==tim] += 1

                iterating = casatools.ms.iternext()

            casatools.ms.close()

            old_settings = np.seterr(divide='ignore')
            data /= ndata
            data = np.abs(data)
            np.seterr(**old_settings)
            flag = ndata==0

            # store the views
            for icorr,corrlist in enumerate(corrs):
                corr = corrlist[0]

                viewresult = commonresultobjects.ImageResult(
                  filename=self.inputs.vis, data=data[icorr],
                  flag=flag[icorr], axes=axes, datatype='Mean amplitude',
                  spw=spwid, pol=corr, intent=intent)

                # add the view results to the result structure
                self.result.addview(viewresult.description, viewresult)
         
