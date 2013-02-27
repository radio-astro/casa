from __future__ import absolute_import

import collections
import numpy as np 
import re
import types

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.logging as logging

from pipeline.hif.tasks.tsysflag.resultobjects import TsysflagResults
from pipeline.hif.tasks.common import commonresultobjects as commonresultobjects
from pipeline.hif.tasks.common import calibrationtableaccess as caltableaccess
from pipeline.hif.tasks.common import viewflaggers

from pipeline.hif.heuristics.tsysspwmap import tsysspwmap as tsysspwmap

LOG = logging.get_logger(__name__)


class TsysflagInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir=None, vis=None, caltable=None, 
      intentgroups=None, metric=None, flag_nmedian=None, fnm_limit=None,
      flag_hi=None, fhi_limit=None, fhi_minsample=None,
      flag_tmf1=None, tmf1_axis=None, tmf1_limit=None, niter=None):

        # set the properties to the values given as input arguments
        self._init_properties(vars())

    @property
    def caltable(self):
        if self._caltable is None:
            return self.context.callibrary.active.get_caltable(
              caltypes='tsys')
        return self._caltable

    @caltable.setter
    def caltable(self, value):
        self._caltable = value

    @property
    def intentgroups(self):
        if self._intentgroups is None:
            return ['*AMPLITUDE*', '*BANDPASS*', '*PHASE* + *TARGET*']
        return self._intentgroups

    @intentgroups.setter
    def intentgroups(self, value):
        self._intentgroups = value

    @property
    def metric(self):
        if self._metric is None:
            return 'median'
        return self._metric

    @metric.setter
    def metric(self, value):
        self._metric = value

    # flag n * median
    @property
    def flag_nmedian(self):
        if self._flag_nmedian is None:
            return True
        return self._flag_nmedian

    @flag_nmedian.setter
    def flag_nmedian(self, value):
        self._flag_nmedian = value

    @property
    def fnm_limit(self):
        if self._fnm_limit is None:
            return 2.0
        return self._fnm_limit

    @fnm_limit.setter
    def fnm_limit(self, value):
        self._fnm_limit = value

    # flag high outlier
    @property
    def flag_hi(self):
        if self._flag_hi is None:
            return False
        return self._flag_hi

    @flag_hi.setter
    def flag_hi(self, value):
        if value is None:
            value = False
        self._flag_hi = value

    @property
    def fhi_limit(self):
        if self._fhi_limit is None:
            return 5
        return self._fhi_limit

    @fhi_limit.setter
    def fhi_limit(self, value):
        if value is None:
            value = 5
        self._fhi_limit = value

    @property
    def fhi_minsample(self):
        if self._fhi_minsample is None:
            return 5
        return self._fhi_minsample

    @fhi_minsample.setter
    def fhi_minsample(self, value):
        if value is None:
            value = 5
        self._fhi_minsample = value

    @property
    def flag_tmf1(self):
        if self._flag_tmf1 is None:
            return False
        return self._flag_tmf1

    @flag_tmf1.setter
    def flag_tmf1(self, value):
        if value is None:
            value = False
        self._flag_tmf1 = value

    @property
    def tmf1_limit(self):
        if self._tmf1_limit is None:
            return 0.5
        return self._tmf1_limit

    @tmf1_limit.setter
    def tmf1_limit(self, value):
        if value is None:
            value = 0.5
        self._tmf1_limit = value

    @property
    def tmf1_axis(self):
        if self._tmf1_axis is None:
            return 'Time'
        return self._tmf1_axis

    @tmf1_axis.setter
    def tmf1_axis(self, value):
        if value is None:
            value = 'Time'
        self._tmf1_axis = value

    @property
    def niter(self):
        if self._niter is None:
            return 1
        return self._niter

    @niter.setter
    def niter(self, value):
        self._niter = value


class Tsysflag(basetask.StandardTaskTemplate):
    Inputs = TsysflagInputs

    def is_multi_vis_task(self):
        return True

    def prepare(self):
        inputs = self.inputs

        # Construct the task that will read the data and create the
        # view of the data that is the basis for flagging.
        datainputs = TsysflagWorkerInputs(context=inputs.context,
          output_dir=inputs.output_dir, caltable=inputs.caltable,
          vis=inputs.vis, intentgroups=inputs.intentgroups, 
          metric=inputs.metric)
        datatask = TsysflagWorker(datainputs)

        # Construct the task that will set any flags raised in the
        # underlying data.
        flagsetterinputs = caltableaccess.CalibrationTableFlagSetterInputs(
          context=inputs.context, vis=inputs.vis)
        flagsettertask = caltableaccess.CalibrationTableFlagSetter(
          flagsetterinputs)

	# Translate the input flagging parameters to a more compact
        # list of rules.
	rules = viewflaggers.MatrixFlagger.make_flag_rules (
          flag_hilo=False,
          flag_hi=inputs.flag_hi,
	  fhi_limit=inputs.fhi_limit, 
          fhi_minsample=inputs.fhi_minsample,
	  flag_lo=False,
          flag_tmf1=inputs.flag_tmf1,
	  tmf1_axis=inputs.tmf1_axis,
          tmf1_limit=inputs.tmf1_limit,
	  flag_tmf2=False,
          flag_nmedian = inputs.flag_nmedian,
          fnm_limit = inputs.fnm_limit)

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
          rules=rules, niter=inputs.niter)
        flaggertask = viewflaggers.MatrixFlagger(matrixflaggerinputs)

	# Execute it to flag the data view
        result = self._executor.execute(flaggertask)

        return result

    def analyse(self, result):
        return result


class TsysflagWorkerInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None, vis=None, caltable=None,
      intentgroups=None, metric=None):
        self._init_properties(vars())


class TsysflagWorker(basetask.StandardTaskTemplate):
    Inputs = TsysflagWorkerInputs

    def __init__(self, inputs):
        super(TsysflagWorker, self).__init__(inputs)
        self.result = TsysflagResults()

    def is_multi_vis_task(self):
        # This task does not work on the vis files but instead on tsys
        # cal tables that have already been created from them. In the
        # automatic 'automatic multi ms handling' framework this would
        # mean that the same flagging would be done repeatedly - once
        # for each measurement set in vis. So, return True here to prevent
        # that happening.
        return True

    def prepare(self):
        inputs = self.inputs

        # Get tsys results
        final = []

	# Loop over caltables.
        for name in inputs.caltable:

	    # Get the tsys table name
            tsystable = caltableaccess.CalibrationTableDataFiller.getcal(name)
            self.result.vis = tsystable.vis

	    # Get the MS object.
            ms = self.inputs.context.observing_run.get_ms(name=tsystable.vis)

            # Construct a callibrary entry for the results that are to be
            # merged back into the context.
            calto = callibrary.CalTo(vis=tsystable.vis)
            #spwmap = tsysmap.tsysspwmap(vis=tsystable.vis, tsystable=name)
            spwmap = tsysspwmap(vis=tsystable.vis, tsystable=name)
            calfrom = callibrary.CalFrom(name, caltype='tsys', spwmap=spwmap)
            calapp = callibrary.CalApplication(calto, calfrom)
            final.append(calapp)

            # Get the spws from the tsystable.
            tsysspws = set()
            for row in tsystable.rows:
                tsysspws.update([row.get('SPECTRAL_WINDOW_ID')])

            # Get ids of fields for intent groups of interest
            intentgroupids = {}
            for intentgroup in self.inputs.intentgroups:
                # translate '*PHASE*+*TARGET*' or '*PHASE*,*TARGET*' to
                # regexp form '.*PHASE.*|.*TARGET.*'
                re_intent = intentgroup.replace('*', '.*')
                re_intent = re_intent.replace('+', '|')
                re_intent = re_intent.replace(',', '|')

                # translate intents to fieldids - have to be careful that
                # PHASE ids have PHASE intent and no other
                groupids = []
                if re.search(pattern=re_intent, string='AMPLITUDE'):
                    groupids += [field.id for field in ms.fields if 'AMPLITUDE'
                      in field.intents]
                if re.search(pattern=re_intent, string='BANDPASS'):
                    groupids += [field.id for field in ms.fields if 'BANDPASS'
                      in field.intents]
                if re.search(pattern=re_intent, string='PHASE'):
                    groupids += [field.id for field in ms.fields if 
                      'PHASE' in field.intents and
                      'BANDPASS' not in field.intents and
                      'AMPLITUDE' not in field.intents]
                if re.search(pattern=re_intent, string='TARGET'):
                    groupids += [field.id for field in ms.fields if 'TARGET'
                      in field.intents]

                intentgroupids[intentgroup] = groupids

	    LOG.info ('Computing flagging metrics for caltable %s ' % (name))
            for tsysspwid in tsysspws:
                for intentgroup in self.inputs.intentgroups:
                    # calculate view for each group of fieldids
                    self.calculate_view(tsystable, tsysspwid, intentgroup,
                      intentgroupids[intentgroup], inputs.metric)

        self.result.final = final[:]

        return self.result

    def analyse(self, result):
        return result

    def calculate_view(self, tsystable, spwid, intent, fieldids, metric):
        """
        tsystable -- CalibrationTableData object giving access to the tsys
                     caltable.
        spwid     -- view will be calculated using data for this spw id.
        fieldids  -- view will be calculated using data for all field_ids in
                     this list.
        metric    -- the name of the view metric:
                        'shape' gives an image where each pixel is a measure 
                        of the departure of the shape for that antenna/scan 
                        from the median. 
                        'median' gives an image where each pixel is the
                        tsys median for that antenna/scan.  
        """
        if metric == 'shape':
            self.calculate_shape_view(tsystable, spwid, intent, fieldids)
        elif metric == 'median':
            self.calculate_median_view(tsystable, spwid, intent,
              fieldids)

    def calculate_shape_view(self, tsystable, spwid, intent, fieldids):
        """
        tsystable -- CalibrationTableData object giving access to the tsys
                     caltable.
        spwid     -- view will be calculated using data for this spw id.
        fieldids  -- view will be calculated using data for all field_ids in
                     this list.

        Data of the specified spwid, intent and range of fieldids are
        read from the given tsystable object. Two data 'views' will be
        created, one for each polarization. Each 'view' is a matrix with
        axes antenna_id and time (or equivalently scan). Each point in the 
        matrix is a measure of the difference of the tsys spectrum there 
        from the median of all the tsys spectra selected. The shape metric
        is calculated using the formula:

        metric = 100 * mean(abs(normalised tsys - median normalised tsys))

        where a 'normalised' array is defined as:

                         array
        normalised = -------------
                     median(array)

        The first 10 and last 10 channels in each Tsys spectrum are assumed
        to be noisy and are not included in the calculation of the metric. 
        """

        antennas = set()
        times = set()

        # dict of results for each pol, defaultdict ia preferred to simple {}
        # because it initialises a new (key,value) automatically if it is not
        # already present when demanded.
        tsysspectra = collections.defaultdict(TsysflagResults)

        pols = []
        for row in tsystable.rows:
            if row.get('SPECTRAL_WINDOW_ID') == spwid and \
              row.get('FIELD_ID') in fieldids:

                # Designate a small number of edge channels in each spw
                # as 'noisy'. These are not flagged but will be ignored
                # in scaling of plots if displays/slice.py is used to do
                # the plotting.
                noisychannels = np.zeros(np.shape(row.get('FPARAM')[0,:,0]),
                  np.bool)
                noisychannels[:10] = True
                noisychannels[-10:] = True

                # The Tsys array has 2 values for each result,
                # presumably 1 number for each polarization.
                # Assume this for now and check later. Pol IDs are
                # unknown so store as '0' and '1'.
                pols = range(np.shape(row.get('FPARAM'))[0])
                for pol in pols:
                    tsysspectrum = commonresultobjects.SpectrumResult(
                      data=row.get('FPARAM')[pol,:,0],
                      flag=row.get('FLAG')[pol,:,0],
                      noisychannels=noisychannels,
                      datatype='Normalised Tsys', filename=tsystable.name,
                      field_id=row.get('FIELD_ID'),
                      spw=row.get('SPECTRAL_WINDOW_ID'),
                      ant=row.get('ANTENNA1'), pol=pol,
                      time=row.get('TIME'), normalise=True)

                    tsysspectra[pol].addview(tsysspectrum.description,
                      tsysspectrum)
                    antennas.update([row.get('ANTENNA1')])
                    times.update([row.get('TIME')])

        # get median Tsys for each pol in this spw - ignores flags for now
        for pol in pols:
            tsysmedians = TsysflagResults()
            spectrumstack = None
            for description in tsysspectra[pol].descriptions():
                tsysspectrum = tsysspectra[pol].last(description)
                if tsysspectrum.pol==pol:
                    if spectrumstack is None:
                        spectrumstack = tsysspectrum.data
                    else:
                        spectrumstack = np.vstack((tsysspectrum.data,
                          spectrumstack))

            if spectrumstack is not None:
                stackmedian = np.median(spectrumstack, axis=0)
                tsysmedian = commonresultobjects.SpectrumResult(
                  data=stackmedian, noisychannels=noisychannels,
                  datatype='Median Normalised Tsys',
                  filename=tsystable.name, spw=spwid, pol=pol,
                  intent=intent)

            tsysmedians.addview(tsysmedian.description, tsysmedian)

            # build the view, get values for axes, initialise pixels
            antennas = np.sort(list(antennas))
            times = np.sort(list(times))
            data = np.zeros([antennas[-1]+1, len(times)])
            flag = np.ones([antennas[-1]+1, len(times)], np.bool)

            # get metric for each antenna, time/scan
            for description in tsysspectra[pol].descriptions():
                tsysspectrum = tsysspectra[pol].last(description)
                validdata = tsysspectrum.data[noisychannels==False]\
                  [np.logical_not(tsysspectrum.flag[noisychannels==False])]
                validmedian = stackmedian[noisychannels==False]\
                  [np.logical_not(tsysspectrum.flag[noisychannels==False])]
                if len(validdata) > 0:
                    metric = abs(validdata - validmedian)
                    metric = 100.0 * np.mean(metric)
                    metricflag = 0
                else:
                    metric = 0.0
                    metricflag = 1

                ant = tsysspectrum.ant
                caltime = tsysspectrum.time
                data[ant, caltime==times] = metric
                flag[ant, caltime==times] = metricflag

            axes = [commonresultobjects.ResultAxis(name='Antenna1',
              units='id', data=np.arange(antennas[-1]+1)),
              commonresultobjects.ResultAxis(name='Time', units='',
              data=times)]

            viewresult = commonresultobjects.ImageResult(
              filename=tsystable.name, data=data,
              flag=flag, axes=axes, datatype='Shape Metric * 100',
              spw=spwid, intent=intent, pol=pol, cell_index=pol)
            # store the spectra contributing to this view as 'children'
            viewresult.children['tsysmedians'] = tsysmedians
            viewresult.children['tsysspectra'] = tsysspectra[pol]

            # add the view results and their children results to the
            # class result structure
            self.result.addview(viewresult.description, viewresult)

    def calculate_median_view(self, tsystable, spwid, intent, fieldids):
        """
        tsystable -- CalibrationTableData object giving access to the tsys
                     caltable.
        spwid     -- view will be calculated using data for this spw id.
        fieldids  -- view will be calculated using data for all field_ids in
                     this list.

        Data of the specified spwid, intent and range of fieldids are
        read from the given tsystable object. Two data 'views' will be
        created, one for each polarization. Each 'view' is a matrix with
        axes antenna_id and time (or equivalently scan). Each point in the 
        matrix is the median value of the tsys spectrum for that 
        antenna/time.
        """

        antennas = set()
        times = set()

        # dict of results for each pol, defaultdict ia preferred to simple {}
        # because it initialises a new (key,value) automatically if it is not
        # already present when demanded.
        tsysspectra = collections.defaultdict(TsysflagResults)

        pols = []
        for row in tsystable.rows:
            if row.get('SPECTRAL_WINDOW_ID') == spwid and \
              row.get('FIELD_ID') in fieldids:

                # Designate a small number of edge channels in each spw
                # as 'noisy'. These are not flagged but will be ignored
                # in scaling of plots if displays/slice.py is used to do
                # the plotting.
                noisychannels = np.zeros(np.shape(row.get('FPARAM')[0,:,0]),
                  np.bool)
                noisychannels[:10] = True
                noisychannels[-10:] = True

                # The Tsys array has 2 values for each result,
                # presumably 1 number for each polarization.
                # Assume this for now and check later. Pol IDs are
                # unknown so store as '0' and '1'.
                pols = range(np.shape(row.get('FPARAM'))[0])
                for pol in pols:
                    tsysspectrum = commonresultobjects.SpectrumResult(
                      data=row.get('FPARAM')[pol,:,0],
                      flag=row.get('FLAG')[pol,:,0],
                      noisychannels=noisychannels,
                      datatype='Tsys', filename=tsystable.name,
                      field_id=row.get('FIELD_ID'),
                      spw=row.get('SPECTRAL_WINDOW_ID'),
                      ant=row.get('ANTENNA1'), units='K', pol=pol,
                      time=row.get('TIME'), normalise=False)

                    tsysspectra[pol].addview(tsysspectrum.description,
                      tsysspectrum)
                    antennas.update([row.get('ANTENNA1')])
                    times.update([row.get('TIME')])

        # get median Tsys for each pol in this spw - ignores flags for now
        for pol in pols:
            tsysmedians = TsysflagResults()
            spectrumstack = None
            for description in tsysspectra[pol].descriptions():
                tsysspectrum = tsysspectra[pol].last(description)
                if tsysspectrum.pol==pol:
                    if spectrumstack is None:
                        spectrumstack = tsysspectrum.data
                    else:
                        spectrumstack = np.vstack((tsysspectrum.data,
                          spectrumstack))

            if spectrumstack is not None:
                stackmedian = np.median(spectrumstack, axis=0)
                tsysmedian = commonresultobjects.SpectrumResult(
                  data=stackmedian,
                  noisychannels=noisychannels, datatype='Median Tsys',
                  filename=tsystable.name, spw=spwid, pol=pol,
                  intent=intent, units='K')

            tsysmedians.addview(tsysmedian.description, tsysmedian)

            # build the view, get values for axes, initialise pixels
            antennas = np.sort(list(antennas))
            times = np.sort(list(times))
            data = np.zeros([antennas[-1]+1, len(times)])
            flag = np.ones([antennas[-1]+1, len(times)], np.bool)

            # get metric for each antenna, time/scan
            for description in tsysspectra[pol].descriptions():
                tsysspectrum = tsysspectra[pol].last(description)
                metric = tsysspectrum.median
                metricflag = np.all(tsysspectrum.flag)

                ant = tsysspectrum.ant
                caltime = tsysspectrum.time
                data[ant, caltime==times] = metric
                flag[ant, caltime==times] = metricflag

            axes = [commonresultobjects.ResultAxis(name='Antenna1',
              units='id', data=np.arange(antennas[-1]+1)),
              commonresultobjects.ResultAxis(name='Time', units='',
              data=times)]

            viewresult = commonresultobjects.ImageResult(
              filename=tsystable.name, data=data,
              flag=flag, axes=axes, datatype='Median',
              spw=spwid, intent=intent, pol=pol, cell_index=pol)
            # store the spectra contributing to this view as 'children'
            viewresult.children['tsysmedians'] = tsysmedians
            viewresult.children['tsysspectra'] = tsysspectra[pol]

            # add the view results to the class result structure
            self.result.addview(viewresult.description, viewresult)
