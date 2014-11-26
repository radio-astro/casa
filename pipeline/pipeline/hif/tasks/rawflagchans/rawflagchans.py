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

from .resultobjects import RawflagchansResults
from ..common import commonresultobjects
from ..common import viewflaggers

LOG = infrastructure.get_logger(__name__)


class RawflagchansInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir=None, vis=None, spw=None,
      intent=None,
      metric=None, flag_hilo=None, fhl_limit=None, fhl_minsample=None,
      flag_tmf=None, tmf_axis=None, tmf_excess_limit=None,
      flag_bad_quadrant=None, fbq_hilo_limit=None,
      fbq_antenna_frac_limit=None, fbq_baseline_frac_limit=None,
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

        if self._intent is not None:
            return self._intent
        
        # if the spw was set, look to see which intents were observed in that
        # spectral window and return the intent based on our order of
        # preference: BANDPASS, PHASE, AMPLITUDE
        preferred_intents = ('BANDPASS', 'PHASE', 'AMPLITUDE')
        if self.spw:
            for spw in self.ms.get_spectral_windows(self.spw):
                for intent in preferred_intents:
                    if intent in spw.intents:
                        if intent != preferred_intents[0]:
                            LOG.warning('%s spw %s: %s not present, %s used instead' %
                              (os.path.basename(self.vis), spw.id, 
                              preferred_intents[0], intent))
                        return intent
            
        # spw was not set, so look through the spectral windows
        for intent in preferred_intents:
            intentok = True
            for spw in self.ms.spectral_windows:
                if intent not in spw.intents:
                    intentok = False
                    break
                if not intentok:
                    break
            if intentok:
                if intent != preferred_intents[0]:
                    LOG.warning('%s %s: %s not present, %s used instead' %
                      (os.path.basename(self.vis), spw.id, preferred_intents[0],
                      intent))
                return intent
                
        # current fallback - return an empty intent
        return ''

    @intent.setter
    def intent(self, value):
        self._intent = value

    @property
    def metric(self):
        return self._metric

    @metric.setter
    def metric(self, value):
        if value is None:
            value = 'combined'
        self._metric = value

    @property
    def flag_hilo(self):
        return self._flag_hilo

    @flag_hilo.setter
    def flag_hilo(self, value):
        if value is None:
            value = True
        self._flag_hilo = value

    @property
    def fhl_limit(self):
        return self._fhl_limit

    @fhl_limit.setter
    def fhl_limit(self, value):
        if value is None:
            value = 10.0
        self._fhl_limit = value

    @property
    def fhl_minsample(self):
        return self._fhl_minsample

    @fhl_minsample.setter
    def fhl_minsample(self, value):
        if value is None:
            value = 5
        self._fhl_minsample = value

    @property
    def flag_tmf(self):
        if self._flag_tmf is None:
            return True
        return self._flag_tmf

    @flag_tmf.setter
    def flag_tmf(self, value):
        self._flag_tmf = value

    @property
    def tmf_axis(self):
        if self._tmf_axis is None:
            return 'Baseline'
        return self._tmf_axis

    @tmf_axis.setter
    def tmf_axis(self, value):
        self._tmf_axis = value

    @property
    def tmf_excess_limit(self):
        if self._tmf_excess_limit is None:
            return 2
        return self._tmf_excess_limit

    @tmf_excess_limit.setter
    def tmf_excess_limit(self, value):
        self._tmf_excess_limit = value

    @property
    def flag_bad_quadrant(self):
        return self._flag_bad_quadrant

    @flag_bad_quadrant.setter
    def flag_bad_quadrant(self, value):
        if value is None:
            value = True
        self._flag_bad_quadrant = value

    @property
    def fbq_hilo_limit(self):
        return self._fbq_hilo_limit

    @fbq_hilo_limit.setter
    def fbq_hilo_limit(self, value):
        if value is None:
            value = 10.0
        self._fbq_hilo_limit = value

    @property
    def fbq_antenna_frac_limit(self):
        return self._fbq_antenna_frac_limit

    @fbq_antenna_frac_limit.setter
    def fbq_antenna_frac_limit(self, value):
        if value is None:
            value = 0.2
        self._fbq_antenna_frac_limit = value

    @property
    def fbq_baseline_frac_limit(self):
        return self._fbq_baseline_frac_limit

    @fbq_baseline_frac_limit.setter
    def fbq_baseline_frac_limit(self, value):
        if value is None:
            value = 0.2
        self._fbq_baseline_frac_limit = value

    @property
    def niter(self):
        if self._niter is None:
            return 2
        return self._niter

    @niter.setter
    def niter(self, value):
        self._niter = value


class Rawflagchans(basetask.StandardTaskTemplate):
    Inputs = RawflagchansInputs

    def prepare(self):
        inputs = self.inputs

        # Construct the task that will read the data and create the
        # view of the data that is the basis for flagging.
        datainputs = RawflagchansWorkerInputs(context=inputs.context,
          output_dir=inputs.output_dir, vis=inputs.vis, spw=inputs.spw,
          intent=inputs.intent, metric=inputs.metric)
        datatask = RawflagchansWorker(datainputs)

        # Construct the task that will set any flags raised in the
        # underlying data.
        flagsetterinputs = FlagdataSetter.Inputs(context=inputs.context,
          vis=inputs.vis, inpfile=[], table=inputs.vis)
        flagsettertask = FlagdataSetter(flagsetterinputs)

        # Translate the input flagging parameters to a more compact
        # list of rules.
        rules = viewflaggers.MatrixFlagger.make_flag_rules (
          flag_hilo=inputs.flag_hilo, fhl_limit=inputs.fhl_limit,
          fhl_minsample=inputs.fhl_minsample,
#          flag_tmf1=inputs.flag_tmf, tmf1_axis='Antenna2',
          flag_tmf1=inputs.flag_tmf, tmf1_axis=inputs.tmf_axis,
          tmf1_limit=1.0, tmf1_excess_limit=inputs.tmf_excess_limit,
          flag_bad_quadrant=inputs.flag_bad_quadrant,
          fbq_hilo_limit=inputs.fbq_hilo_limit,
          fbq_antenna_frac_limit=inputs.fbq_antenna_frac_limit,
          fbq_baseline_frac_limit=inputs.fbq_baseline_frac_limit)
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


class RawflagchansWorkerInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None, vis=None, spw=None,
      intent=None, metric=None):
        self._init_properties(vars())


class RawflagchansWorker(basetask.StandardTaskTemplate):
    Inputs = RawflagchansWorkerInputs

    def __init__(self, inputs):
        super(RawflagchansWorker, self).__init__(inputs)
        self.result = RawflagchansResults()

    def _readfile(self, file):

        with open(file, 'r') as f:
            lines = f.readlines()
            for line in lines:
                if line[0] == '#':
                    # ignore comments
                    continue

                try:
                    words = line.split()
                    chan = int(words[0])
                    data = float(words[1])
                    spwid = int(words[11])
                    corr = words[12]
                    ant1 = int(words[5])
                    ant2 = int(words[6])

                    self._plotdata[(spwid,corr,ant1,ant2,chan)] = data
                except:
                    print 'fail:', line

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
        if inputs.metric == 'separate':
            self.calculate_separate_view(spwids, inputs.intent)
        elif inputs.metric == 'combined':
            self.calculate_combined_view(spwids, inputs.intent)
        else:
            raise Exception, 'bad metric: %s' % inputs.metric

        return self.result

    def analyse(self, result):
        return result

#    def calculate_separate_view(self, spwids, intent):
#        """
#        spwid     -- view will be calculated using data for this spw id.
#        """

        # the current view will be very similar to the last, if available.
        # For now approximate as being identical which will save having to
        # recalculate
#        prev_descriptions = self.result.descriptions()
#        if prev_descriptions:
#            for description in prev_descriptions:
#                prev_result = self.result.last(description)
#                self.result.addview(description, prev_result)

            # EARLY RETURN
#            return

        # plotms uses field and not intent
#        fieldids = [field.id for field in self.ms.get_fields(intent=intent)]
#        if not fieldids:
#            LOG.error('no data for intent: %s' % intent)
#            raise Exception, 'no data for intent: %s' % intent

#        fieldids = ','.join([str(fieldid) for fieldid in fieldids])
#        plotfile='%s_temp.txt' % os.path.basename(self.inputs.vis)
#        print 'plotfile', plotfile

        # plotms export seems unreliable except for small bits of dataset
        # so this is inefficient but might work better in future
#        self._plotdata = {}
#        ants = np.array(self.antenna_ids)
#        for spwid in spwids:
#            first = True
#            for ant in ants:
#                print 'plotting', ant
#                if first:
#                    casa.plotms(vis=self.inputs.vis, xaxis='channel',
#                      yaxis='amp', field=fieldids, spw=str(spwid),
#                      antenna=str(ant),
#                      averagedata=True, avgtime='3600',
#                      plotfile=plotfile, expformat='txt', overwrite=True,
#                      showgui=False)
##                    first = False

#                print 'reading'
#                self._readfile(plotfile)

        # now construct the views
#        for spwid in spwids:
#            corrs = commonhelpermethods.get_corr_products(self.ms, spwid)
#            spw = self.ms.get_spectral_window(spwid)
#            nchans = spw.num_channels

#            axes = [
#              commonresultobjects.ResultAxis(name='channels',
#              units='', data=np.arange(nchans)),
#              commonresultobjects.ResultAxis(name='Antenna2',
#              units='id', data=np.arange(self.antenna_ids[-1]+1))]

#            for corrlist in corrs:
#                corr = corrlist[0]
#                data = np.zeros([self.antenna_ids[-1]+1, nchans, 
#                  self.antenna_ids[-1]+1])
#                flag = np.ones([self.antenna_ids[-1]+1, nchans,
#                  self.antenna_ids[-1]+1], np.bool)
#                ants = np.array(self.antenna_ids)

#                for ant1 in ants:
#                    slice = np.zeros([nchans])
#                    slice_flag = np.ones([nchans])

#                    for ant2 in ants[ants > ant1]: 
#                        for chan in range(nchans): 
#                            try:
#                                slice[chan] = \
#                                  self._plotdata[(spwid,corr,ant1,ant2,chan)]
#                                slice_flag[chan] = 0
#                            except:
#                                pass

#                        slice -= np.median(slice)
#                        data[ant1,:,ant2] = slice
#                        data[ant2,:,ant1] = slice
#                        flag[ant1,:,ant2] = slice_flag
#                        flag[ant2,:,ant1] = slice_flag

#                for ant1 in ants:
#                    viewresult = commonresultobjects.ImageResult(
#                      filename=self.inputs.vis, data=data[ant1],
#                      flag=flag[ant1], axes=axes, datatype='Mean amplitude',
#                      spw=spwid, pol=corr, ant=self.antenna_name[ant1])

                    # add the view results and their children results to the
                    # class result structure
#                    self.result.addview(viewresult.description, viewresult)

        # tidy up
#        os.system('rm -fr %s' % plotfile)

    def calculate_separate_view(self, spwids, intent):
        """
        spwids -- views will be calculated using data for each spw id
                  in this list.
        """

#        # the current view will be very similar to the last, if available.
#        # For now approximate as being identical which will save having to
#        # recalculate
#        prev_descriptions = self.result.descriptions()
#        if prev_descriptions:
#            for description in prev_descriptions:
#                prev_result = self.result.last(description)
#                self.result.addview(description, prev_result)

#            # EARLY RETURN
#            return

        ants = np.array(self.antenna_ids)

        # now construct the views
        for spwid in spwids:
            corrs = commonhelpermethods.get_corr_products(self.ms, spwid)
            spw = self.ms.get_spectral_window(spwid)
            nchans = spw.num_channels

            ants = np.array(self.antenna_ids)
            baselines = []
            for ant1 in ants:
                for ant2 in ants: 
                    baselines.append('%s&%s' % (ant1, ant2))

            axes = [
              commonresultobjects.ResultAxis(name='channels',
              units='', data=np.arange(nchans)),
              commonresultobjects.ResultAxis(name='Antenna2',
              units='id', data=np.arange(self.antenna_ids[-1]+1))]

            data = np.zeros([len(corrs), self.antenna_ids[-1]+1, nchans, 
              self.antenna_ids[-1]+1], np.complex)
            flag = np.ones([len(corrs), self.antenna_ids[-1]+1, nchans,
              self.antenna_ids[-1]+1], np.bool)
            ndata = np.zeros([len(corrs), self.antenna_ids[-1]+1, nchans,
              self.antenna_ids[-1]+1], np.int)

            LOG.info('calculating flagging view for spw %s' % spwid)
            casatools.ms.open(self.inputs.vis)
            casatools.ms.msselect({'scanintent':'*BANDPASS*','spw':str(spwid)})
#            ifrdata = casatools.ms.getdata(['data', 'flag', 'antenna1',
#              'antenna2'], ifraxis=True, average=True)
            casatools.ms.iterinit(maxrows=500)
            casatools.ms.iterorigin()
            iterating = True
            while iterating:
                rec = casatools.ms.getdata(['data', 'flag', 'antenna1',
                  'antenna2'])
                if 'data' not in rec.keys():
                    break

                for row in range(np.shape(rec['data'])[2]):
                    ant1 = rec['antenna1'][row]
                    ant2 = rec['antenna2'][row]

                    if ant1==ant2:
                        continue

                    for icorr,corrlist in enumerate(corrs):
                       data[icorr,ant1,:,ant2][
                         rec['flag'][icorr,:,row]==False]\
                         += rec['data'][icorr,:,row][
                         rec['flag'][icorr,:,row]==False]
                       ndata[icorr,ant1,:,ant2][
                         rec['flag'][icorr,:,row]==False] += 1
                       data[icorr,ant2,:,ant1][
                         rec['flag'][icorr,:,row]==False]\
                         += rec['data'][icorr,:,row][
                         rec['flag'][icorr,:,row]==False]
                       ndata[icorr,ant2,:,ant1][
                         rec['flag'][icorr,:,row]==False] += 1

                iterating = casatools.ms.iternext()

            casatools.ms.close()

            # calculate the average values - ignore divide by 0
            old_settings = np.seterr(divide='ignore')
            data /= ndata
            np.seterr(**old_settings)
            data = np.abs(data)
            flag = ndata==0

            # store the views
            for icorr,corrlist in enumerate(corrs):
                corr = corrlist[0]

                for ant1 in ants:
                    # refine the view
                    for ant2 in ants:
                        valid = data[icorr,ant1,:,ant2][flag[icorr,ant1,:,ant2]==False]
                        if len(valid):
                            data[icorr,ant1,:,ant2] -= np.median(valid)

                    for chan in range(nchans):
                        valid = data[icorr,ant1,chan,:][flag[icorr,ant1,chan,:]==False]
                        if len(valid):
                            data[icorr,ant1,chan,:] -= np.median(valid)

                    # store the view result
                    viewresult = commonresultobjects.ImageResult(
                      filename=self.inputs.vis, data=data[icorr,ant1],
                      flag=flag[icorr,ant1], axes=axes,
                      datatype='Mean amplitude',
                      spw=spwid, pol=corr, ant=(ant1,self.antenna_name[ant1]))

                    # add the view results to the class result structure
                    self.result.addview(viewresult.description, viewresult)

    def calculate_combined_view(self, spwids, intent):
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

                # re-refine the data to take into account possible
                # new flagging
                data = prev_result.data
                flag = prev_result.flag

                self.refine_view(data, flag)
         
                prev_result.data = data
                prev_result.flag = flag

                self.result.addview(description, prev_result)

            # EARLY RETURN
            return

        ants = np.array(self.antenna_ids)

        # now construct the views
        for spwid in spwids:
            corrs = commonhelpermethods.get_corr_products(self.ms, spwid)
            spw = self.ms.get_spectral_window(spwid)
            nchans = spw.num_channels

            ants = np.array(self.antenna_ids)
            baselines = []
            for ant1 in ants:
                for ant2 in ants: 
                    baselines.append('%s&%s' % (ant1, ant2))

            axes = [
              commonresultobjects.ResultAxis(name='channels',
              units='', data=np.arange(nchans)),
              commonresultobjects.ResultAxis(name='Baseline',
              units='', data=np.array(baselines), channel_width=1)]

            data = np.zeros([len(corrs), nchans, 
              (self.antenna_ids[-1]+1) * (self.antenna_ids[-1]+1)], np.complex)
            flag = np.ones([len(corrs), nchans, 
              (self.antenna_ids[-1]+1) * (self.antenna_ids[-1]+1)], np.bool)
            ndata = np.zeros([len(corrs), nchans, 
              (self.antenna_ids[-1]+1) * (self.antenna_ids[-1]+1)], np.int)

            LOG.info('calculating flagging view for spw %s' % spwid)
            casatools.ms.open(self.inputs.vis)
            casatools.ms.msselect({'scanintent':'*BANDPASS*','spw':str(spwid)})
#            ifrdata = casatools.ms.getdata(['data', 'flag', 'antenna1',
#              'antenna2'], ifraxis=True, average=True)
            casatools.ms.iterinit(maxrows=500)
            casatools.ms.iterorigin()
            iterating = True
            while iterating:
                rec = casatools.ms.getdata(['data', 'flag', 'antenna1',
                  'antenna2'])
                if 'data' not in rec.keys():
                    break

                for row in range(np.shape(rec['data'])[2]):
                    ant1 = rec['antenna1'][row]
                    ant2 = rec['antenna2'][row]

                    if ant1==ant2:
                        continue

                    baseline1 = ant1 * (ants[-1] + 1) + ant2
                    baseline2 = ant2 * (ants[-1] + 1) + ant1

                    for icorr,corrlist in enumerate(corrs):
                       data[icorr,:,baseline1][
                         rec['flag'][icorr,:,row]==False]\
                         += rec['data'][icorr,:,row][
                         rec['flag'][icorr,:,row]==False]
                       ndata[icorr,:,baseline1][
                         rec['flag'][icorr,:,row]==False] += 1
                       data[icorr,:,baseline2][
                         rec['flag'][icorr,:,row]==False]\
                         += rec['data'][icorr,:,row][
                         rec['flag'][icorr,:,row]==False]
                       ndata[icorr,:,baseline2][
                         rec['flag'][icorr,:,row]==False] += 1

                iterating = casatools.ms.iternext()

            casatools.ms.close()

            # calculate the average values - ignore divide by 0
            old_settings = np.seterr(divide='ignore')
            data /= ndata
            np.seterr(**old_settings)
            data = np.abs(data)
            flag = ndata==0

            # refine and store the views
            for icorr,corrlist in enumerate(corrs):
                corr = corrlist[0]

                self.refine_view(data[icorr], flag[icorr])

#                for baseline in range(np.shape(data)[2]):
#                    valid = data[icorr,:,baseline][flag[icorr,:,baseline]==False]
#                    if len(valid):
#                        data[icorr,:,baseline] -= np.median(valid)

#                for chan in range(nchans):
#                    valid = data[icorr,chan,:][flag[icorr,chan,:]==False]
#                    if len(valid):
#                        data[icorr,chan,:] -= np.median(valid)

                viewresult = commonresultobjects.ImageResult(
                  filename=self.inputs.vis, data=data[icorr],
                  flag=flag[icorr], axes=axes, datatype='Mean amplitude',
                  spw=spwid, pol=corr, intent=intent)

                # add the view results to the result structure
                self.result.addview(viewresult.description, viewresult)

    def refine_view(self, data, flag):
        """Refine the data view by subtracting the median from each
        baseline row, then subtracting the median from each channel
        column.
        """

        for baseline in range(np.shape(data)[1]):
            valid = data[:,baseline][flag[:,baseline]==False]
            if len(valid):
                data[:,baseline] -= np.median(valid)

        for chan in range(np.shape(data)[0]):
            valid = data[chan,:][flag[chan,:]==False]
            if len(valid):
                data[chan,:] -= np.median(valid)
         
