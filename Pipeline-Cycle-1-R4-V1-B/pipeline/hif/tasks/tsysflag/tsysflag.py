from __future__ import absolute_import

import collections
import numpy as np 
import re

from pipeline.hif.heuristics.tsysspwmap import tsysspwmap
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
from pipeline.hif.tasks.flagging.flagdatasetter import FlagdataSetter

from .resultobjects import TsysflagResults
from ..common import commonresultobjects
from ..common import calibrationtableaccess as caltableaccess
from ..common import viewflaggers

LOG = infrastructure.get_logger(__name__)


class TsysflagInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir=None, vis=None, caltable=None, 
      intentgroups=None, metric=None, flagcmdfile=None,
      flag_nmedian=None, fnm_limit=None,
      flag_hi=None, fhi_limit=None, fhi_minsample=None,
      flag_tmf1=None, tmf1_axis=None, tmf1_limit=None,
      flag_maxabs=None, fmax_limit=None,
      niter=None):

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
    def intentgroups(self):
        if self._intentgroups is None:
            return ['*AMPLITUDE*', '*BANDPASS*', '*PHASE*+*TARGET*']
        else:
            # intentgroups is set by the user as a single string, needs
            # converting to a list of strings, i.e.
            # "['a,b,c', 'd,e,f']" becomes ['a,b,c', 'd,e,f']
            intentgroups = self._intentgroups
            intentgroups = intentgroups.replace('[', '')
            intentgroups = intentgroups.replace(']', '')
            intentgroups = intentgroups.replace(' ', '')
            intentgroups = intentgroups.replace("','", "'|'")
            intentgroups = intentgroups.replace('","', '"|"')
            intentgroups = intentgroups.split('|')
            intentgroups = [intentgroup.replace('"', '') for intentgroup in intentgroups]
            intentgroups = [intentgroup.replace("'", "") for intentgroup in intentgroups]

        return intentgroups

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

#    @property
#    def flagcmdfile(self):
#        if self._flagcmdfile is None:
#            return '%s_flagcmds.txt' % self.caltable
#        return self._flagcmdfilefile

#    @flagcmdfile.setter
#    def flagcmdfile(self, value):
#        self._flagcmdfile = value

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
    def flag_maxabs(self):
        if self._flag_maxabs is None:
            return False
        return self._flag_maxabs

    @flag_maxabs.setter
    def flag_maxabs(self, value):
        if value is None:
            value = False
        self._flag_maxabs = value

    @property
    def fmax_limit(self):
        if self._fmax_limit is None:
            return 5
        return self._fmax_limit

    @fmax_limit.setter
    def fmax_limit(self, value):
        self._fmax_limit = value

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
        flagsetterinputs = FlagdataSetter.Inputs(context=inputs.context,
          vis=inputs.vis, table=inputs.caltable, inpfile=[])
#          table=inputs.caltable, inpfile=inputs.flagcmdfile)
        flagsettertask = FlagdataSetter(flagsetterinputs)

        # Translate the input flagging parameters to a more compact
        # list of rules.
	rules = viewflaggers.MatrixFlagger.make_flag_rules (
          flag_hilo=False,
          flag_hi=inputs.flag_hi,
	  fhi_limit=inputs.fhi_limit, 
          fhi_minsample=inputs.fhi_minsample,
          flag_maxabs=inputs.flag_maxabs,
          fmax_limit=inputs.fmax_limit,
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

    def prepare(self):
        inputs = self.inputs

        # Get tsys results
        final = []

        # Loop over caltables.
        name = inputs.caltable

        # Get the tsys table name
        tsystable = caltableaccess.CalibrationTableDataFiller.getcal(name)
        self.result.vis = tsystable.vis

        # Get the MS object.
        ms = self.inputs.context.observing_run.get_ms(name=tsystable.vis)

        # Construct a callibrary entry for the results that are to be
        # merged back into the context.
        calto = callibrary.CalTo(vis=tsystable.vis)
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
            re_intent = intentgroup.replace(' ', '')
            re_intent = re_intent.replace('*', '.*')
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
            if re.search(pattern=re_intent, string='ATMOSPHERE'):
                groupids += [field.id for field in ms.fields if 'ATMOSPHERE'
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
                          from the median over all antennas/scans in the 
                          selected fields in the SpW. 
                        'fieldshape' gives an image where each pixel is a
                          measure of the departure of the shape for that 
                          antenna/scan from the median over all
                          scans for that antenna in the selected fields
                          in the SpW. 
                        'derivative' gives an image where each pixel is
                          the MAD of the channel to channel derivative
                          across the spectrum for that antenna/scan.
                        'median' gives an image where each pixel is the
                          tsys median for that antenna/scan.  
        """
        if metric == 'shape':
            self.calculate_shape_view(tsystable, spwid, intent, fieldids)
        elif metric == 'fieldshape':
            self.calculate_field_shape_view(tsystable, spwid, intent, fieldids)
        elif metric == 'median':
            self.calculate_median_view(tsystable, spwid, intent,
              fieldids)
        elif metric == 'derivative':
            self.calculate_derivative_view(tsystable, spwid, intent,
              fieldids)

    def calculate_field_shape_view(self, tsystable, spwid, intent, fieldids):
        """
        tsystable -- CalibrationTableData object giving access to the tsys
                     caltable.
        spwid     -- view will be calculated using data for this spw id.
        fieldids  -- view will be calculated using data for all field_ids in
                     this list.

        Data of the specified spwid, intent and range of fieldids are
        read from the given tsystable object. Two data 'views' will be
        created, one for each polarization. Each 'view' is a matrix with
        axes antenna_id v time. Each point in the matrix is a measure of
        the difference of the tsys spectrum there from the median of all
        the tsys spectra for that antenna/spw in the selected fields. The
        shape metric is calculated using the formula:

        metric = 100 * mean(abs(normalised tsys - median normalised tsys))

        where a 'normalised' array is defined as:

                         array
        normalised = -------------
                     median(array)

        """

        ms = self.inputs.context.observing_run.get_ms(name=self.inputs.vis)
        antenna_ids = [antenna.id for antenna in ms.antennas] 
        antenna_ids.sort()
        antenna_name = {}
        for antenna_id in antenna_ids:
            antenna_name[antenna_id] = [antenna.name for antenna in ms.antennas 
              if antenna.id==antenna_id][0]

        times = set()

        # dict of results for each pol, defaultdict ia preferred to simple {}
        # because it initialises a new (key,value) automatically if it is not
        # already present when demanded.
        tsysspectra = collections.defaultdict(TsysflagResults)

        pols = []
        for row in tsystable.rows:
            if row.get('SPECTRAL_WINDOW_ID') == spwid and \
              row.get('FIELD_ID') in fieldids:

                # The Tsys array has 2 values for each result,
                # presumably 1 number for each polarization.
                # Assume this for now and check later. Pol IDs are
                # unknown so store as '0' and '1'.
                pols = range(np.shape(row.get('FPARAM'))[0])
                for pol in pols:
          
                    tsysspectrum = commonresultobjects.SpectrumResult(
                      data=row.get('FPARAM')[pol,:,0],
                      flag=row.get('FLAG')[pol,:,0],
                      datatype='Normalised Tsys', filename=tsystable.name,
                      field_id=row.get('FIELD_ID'),
                      spw=row.get('SPECTRAL_WINDOW_ID'),
                      ant=(row.get('ANTENNA1'),
                      antenna_name[row.get('ANTENNA1')]), pol=pol,
                      time=row.get('TIME'), normalise=True)

                    tsysspectra[pol].addview(tsysspectrum.description,
                      tsysspectrum)
                    times.update([row.get('TIME')])

        for pol in pols:
            tsysmedians = TsysflagResults()

            for antenna_id in antenna_ids:
                # get median Tsys for each pol/antenna in this spw 

                spectrumstack = None
                for description in tsysspectra[pol].descriptions():
                    tsysspectrum = tsysspectra[pol].last(description)
                    if tsysspectrum.pol==pol and \
                      tsysspectrum.ant[0]==antenna_id:
                        if spectrumstack is None:
                            spectrumstack = tsysspectrum.data
                            flagstack = tsysspectrum.flag
                        else:
                            spectrumstack = np.vstack((tsysspectrum.data,
                              spectrumstack))
                            flagstack = np.vstack((tsysspectrum.flag,
                              flagstack))

                if spectrumstack is not None:
                    if np.ndim(spectrumstack) == 1:
                        # need to reshape to 2d array
                        dim = np.shape(spectrumstack)[0]
                        spectrumstack = np.reshape(spectrumstack, (1,dim))
                        flagstack = np.reshape(flagstack, (1,dim))

                    stackmedian = np.zeros(np.shape(spectrumstack)[1])
                    stackmedianflag = np.ones(np.shape(spectrumstack)[1], np.bool)

                    for j in range(np.shape(spectrumstack)[1]):
                        valid_data = spectrumstack[:,j][np.logical_not(flagstack[:,j])]
                        if len(valid_data):
                            stackmedian[j] = np.median(valid_data)
                            stackmedianflag[j] = False

                    tsysmedian = commonresultobjects.SpectrumResult(
                      data=stackmedian, 
                      datatype='Median Normalised Tsys',
                      filename=tsystable.name, spw=spwid, pol=pol,
                      ant=(antenna_id, antenna_name[antenna_id]),
                      intent=intent)

                    tsysmedians.addview(tsysmedian.description, tsysmedian)

            # build the view, get values for axes, initialise pixels
            times = np.sort(list(times))
            data = np.zeros([antenna_ids[-1]+1, len(times)])
            flag = np.ones([antenna_ids[-1]+1, len(times)], np.bool)

            # get metric for each antenna, time/scan
            for description in tsysspectra[pol].descriptions():
                # tsys spectrum
                tsysspectrum = tsysspectra[pol].last(description)
                validdata = tsysspectrum.data\
                  [np.logical_not(tsysspectrum.flag)]

                # get the 'median' for this antenna
                for description in tsysmedians.descriptions():
                    tsysmedian = tsysmedians.last(description)
                    if tsysmedian.ant[0] != tsysspectrum.ant[0]:
                        continue

                    # calculate the metric
                    validmedian = tsysmedian.data\
                      [np.logical_not(tsysspectrum.flag)]
                    if len(validdata) > 0:
                        metric = abs(validdata - validmedian)
                        metric = 100.0 * np.mean(metric)
                        metricflag = 0
                    else:
                        metric = 0.0
                        metricflag = 1

                    ant = tsysspectrum.ant
                    caltime = tsysspectrum.time
                    data[ant[0], caltime==times] = metric
                    flag[ant[0], caltime==times] = metricflag

            axes = [commonresultobjects.ResultAxis(name='Antenna1',
              units='id', data=np.arange(antenna_ids[-1]+1)),
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
        axes antenna_id v time. Each point in the matrix is a measure of the
        difference of the tsys spectrum there from the median of all the 
        tsys spectra for that spw in the selected fields. The shape metric
        is calculated using the formula:

        metric = 100 * mean(abs(normalised tsys - median normalised tsys))

        where a 'normalised' array is defined as:

                         array
        normalised = -------------
                     median(array)

        """

        ms = self.inputs.context.observing_run.get_ms(name=self.inputs.vis)
        antenna_ids = [antenna.id for antenna in ms.antennas] 
        antenna_ids.sort()
        antenna_name = {}
        for antenna_id in antenna_ids:
            antenna_name[antenna_id] = [antenna.name for antenna in ms.antennas 
              if antenna.id==antenna_id][0]

        times = set()

        # dict of results for each pol, defaultdict ia preferred to simple {}
        # because it initialises a new (key,value) automatically if it is not
        # already present when demanded.
        tsysspectra = collections.defaultdict(TsysflagResults)

        pols = []
        for row in tsystable.rows:
            if row.get('SPECTRAL_WINDOW_ID') == spwid and \
              row.get('FIELD_ID') in fieldids:

                # The Tsys array has 2 values for each result,
                # presumably 1 number for each polarization.
                # Assume this for now and check later. Pol IDs are
                # unknown so store as '0' and '1'.
                pols = range(np.shape(row.get('FPARAM'))[0])
                for pol in pols:
          
                    tsysspectrum = commonresultobjects.SpectrumResult(
                      data=row.get('FPARAM')[pol,:,0],
                      flag=row.get('FLAG')[pol,:,0],
                      datatype='Normalised Tsys', filename=tsystable.name,
                      field_id=row.get('FIELD_ID'),
                      spw=row.get('SPECTRAL_WINDOW_ID'),
                      ant=(row.get('ANTENNA1'),
                      antenna_name[row.get('ANTENNA1')]), pol=pol,
                      time=row.get('TIME'), normalise=True)

                    tsysspectra[pol].addview(tsysspectrum.description,
                      tsysspectrum)
                    times.update([row.get('TIME')])

        for pol in pols:
           # get median Tsys for each pol in this spw 
            tsysmedians = TsysflagResults()
            spectrumstack = None
            for description in tsysspectra[pol].descriptions():
                tsysspectrum = tsysspectra[pol].last(description)
                if tsysspectrum.pol==pol:
                    if spectrumstack is None:
                        spectrumstack = tsysspectrum.data
                        flagstack = tsysspectrum.flag
                    else:
                        spectrumstack = np.vstack((tsysspectrum.data,
                          spectrumstack))
                        flagstack = np.vstack((tsysspectrum.flag,
                          flagstack))

            if spectrumstack is not None:
                stackmedian = np.zeros(np.shape(spectrumstack)[1])
                stackmedianflag = np.ones(np.shape(spectrumstack)[1], np.bool)

                for j in range(np.shape(spectrumstack)[1]):
                    valid_data = spectrumstack[:,j][np.logical_not(flagstack[:,j])]
                    if len(valid_data):
                        stackmedian[j] = np.median(valid_data)
                        stackmedianflag[j] = False

                tsysmedian = commonresultobjects.SpectrumResult(
                  data=stackmedian, 
                  datatype='Median Normalised Tsys',
                  filename=tsystable.name, spw=spwid, pol=pol,
                  intent=intent)

            tsysmedians.addview(tsysmedian.description, tsysmedian)

            # build the view, get values for axes, initialise pixels
            times = np.sort(list(times))
            data = np.zeros([antenna_ids[-1]+1, len(times)])
            flag = np.ones([antenna_ids[-1]+1, len(times)], np.bool)

            # get metric for each antenna, time/scan
            for description in tsysspectra[pol].descriptions():
                tsysspectrum = tsysspectra[pol].last(description)
                validdata = tsysspectrum.data\
                  [np.logical_not(tsysspectrum.flag)]
                validmedian = stackmedian\
                  [np.logical_not(tsysspectrum.flag)]
                if len(validdata) > 0:
                    metric = abs(validdata - validmedian)
                    metric = 100.0 * np.mean(metric)
                    metricflag = 0
                else:
                    metric = 0.0
                    metricflag = 1

                ant = tsysspectrum.ant
                caltime = tsysspectrum.time
                data[ant[0], caltime==times] = metric
                flag[ant[0], caltime==times] = metricflag

            axes = [commonresultobjects.ResultAxis(name='Antenna1',
              units='id', data=np.arange(antenna_ids[-1]+1)),
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
        axes antenna_id v time. Each point in the matrix is the median
        value of the tsys spectrum for that antenna/time.
        """

        ms = self.inputs.context.observing_run.get_ms(name=self.inputs.vis)
        antenna_ids = [antenna.id for antenna in ms.antennas] 
        antenna_ids.sort()
        antenna_name = {}
        for antenna_id in antenna_ids:
            antenna_name[antenna_id] = [antenna.name for antenna in ms.antennas 
              if antenna.id==antenna_id][0]

        times = set()

        # dict of results for each pol, defaultdict ia preferred to simple {}
        # because it initialises a new (key,value) automatically if it is not
        # already present when demanded.
        tsysspectra = collections.defaultdict(TsysflagResults)

        pols = []
        for row in tsystable.rows:
            if row.get('SPECTRAL_WINDOW_ID') == spwid and \
              row.get('FIELD_ID') in fieldids:

                # The Tsys array has 2 values for each result,
                # presumably 1 number for each polarization.
                # Assume this for now and check later. Pol IDs are
                # unknown so store as '0' and '1'.
                pols = range(np.shape(row.get('FPARAM'))[0])
                for pol in pols:
                    tsysspectrum = commonresultobjects.SpectrumResult(
                      data=row.get('FPARAM')[pol,:,0],
                      flag=row.get('FLAG')[pol,:,0],
                      datatype='Tsys', filename=tsystable.name,
                      field_id=row.get('FIELD_ID'),
                      spw=row.get('SPECTRAL_WINDOW_ID'),
                      ant=(row.get('ANTENNA1'),
                      antenna_name[row.get('ANTENNA1')]), units='K',
                      pol=pol, time=row.get('TIME'), normalise=False)

                    tsysspectra[pol].addview(tsysspectrum.description,
                      tsysspectrum)
                    times.update([row.get('TIME')])

        for pol in pols:
            # get median Tsys for each pol in this spw
            tsysmedians = TsysflagResults()
            spectrumstack = None
            for description in tsysspectra[pol].descriptions():
                tsysspectrum = tsysspectra[pol].last(description)
                if tsysspectrum.pol==pol:
                    if spectrumstack is None:
                        spectrumstack = tsysspectrum.data
                        flagstack = tsysspectrum.flag
                    else:
                        spectrumstack = np.vstack((tsysspectrum.data,
                          spectrumstack))
                        flagstack = np.vstack((tsysspectrum.flag,
                          flagstack))

            if spectrumstack is not None:
                stackmedian = np.zeros(np.shape(spectrumstack)[1])
                stackmedianflag = np.ones(np.shape(spectrumstack)[1], np.bool)

                for j in range(np.shape(spectrumstack)[1]):
                    valid_data = spectrumstack[:,j][np.logical_not(flagstack[:,j])]
                    if len(valid_data):
                        stackmedian[j] = np.median(valid_data)
                        stackmedianflag[j] = False

                tsysmedian = commonresultobjects.SpectrumResult(
                  data=stackmedian, 
                  datatype='Median Normalised Tsys',
                  filename=tsystable.name, spw=spwid, pol=pol,
                  intent=intent)

            tsysmedians.addview(tsysmedian.description, tsysmedian)

            # build the view, get values for axes, initialise pixels
            times = np.sort(list(times))
            data = np.zeros([antenna_ids[-1]+1, len(times)])
            flag = np.ones([antenna_ids[-1]+1, len(times)], np.bool)

            # get metric for each antenna, time/scan
            for description in tsysspectra[pol].descriptions():
                tsysspectrum = tsysspectra[pol].last(description)
                metric = tsysspectrum.median
                metricflag = np.all(tsysspectrum.flag)

                ant = tsysspectrum.ant
                caltime = tsysspectrum.time
                data[ant[0], caltime==times] = metric
                flag[ant[0], caltime==times] = metricflag

            axes = [commonresultobjects.ResultAxis(name='Antenna1',
              units='id', data=np.arange(antenna_ids[-1]+1)),
              commonresultobjects.ResultAxis(name='Time', units='',
              data=times)]

            viewresult = commonresultobjects.ImageResult(
              filename=tsystable.name, data=data,
              flag=flag, axes=axes, datatype='Median Tsys',
              spw=spwid, intent=intent, pol=pol, cell_index=pol)
            # store the spectra contributing to this view as 'children'
            viewresult.children['tsysmedians'] = tsysmedians
            viewresult.children['tsysspectra'] = tsysspectra[pol]

            # add the view results to the class result structure
            self.result.addview(viewresult.description, viewresult)

    def calculate_derivative_view(self, tsystable, spwid, intent, fieldids):
        """
        tsystable -- CalibrationTableData object giving access to the tsys
                     caltable.
        spwid     -- view will be calculated using data for this spw id.
        fieldids  -- view will be calculated using data for all field_ids in
                     this list.

        Data of the specified spwid, intent and range of fieldids are
        read from the given tsystable object. Two data 'views' will be
        created, one for each polarization. Each 'view' is a matrix with
        axes antenna_id v time. Each point in the matrix is the median
        absolute deviation (MAD) of the channel to channel derivative
        across the Tsys spectrum for that antenna/time.
        """

        ms = self.inputs.context.observing_run.get_ms(name=self.inputs.vis)
        antenna_ids = [antenna.id for antenna in ms.antennas] 
        antenna_ids.sort()
        antenna_name = {}
        for antenna_id in antenna_ids:
            antenna_name[antenna_id] = [antenna.name for antenna in ms.antennas 
              if antenna.id==antenna_id][0]

        times = set()

        # dict of results for each pol, defaultdict ia preferred to simple {}
        # because it initialises a new (key,value) automatically if it is not
        # already present when demanded.
        tsysspectra = collections.defaultdict(TsysflagResults)

        pols = []
        for row in tsystable.rows:
            if row.get('SPECTRAL_WINDOW_ID') == spwid and \
              row.get('FIELD_ID') in fieldids:

                # The Tsys array has 2 values for each result,
                # presumably 1 number for each polarization.
                # Assume this for now and check later. Pol IDs are
                # unknown so store as '0' and '1'.
                pols = range(np.shape(row.get('FPARAM'))[0])
                for pol in pols:
                    tsysspectrum = commonresultobjects.SpectrumResult(
                      data=row.get('FPARAM')[pol,:,0],
                      flag=row.get('FLAG')[pol,:,0],
                      datatype='Tsys', filename=tsystable.name,
                      field_id=row.get('FIELD_ID'),
                      spw=row.get('SPECTRAL_WINDOW_ID'),
                      ant=(row.get('ANTENNA1'),
                      antenna_name[row.get('ANTENNA1')]), units='K',
                      pol=pol, time=row.get('TIME'), normalise=True)

                    tsysspectra[pol].addview(tsysspectrum.description,
                      tsysspectrum)
                    times.update([row.get('TIME')])

        for pol in pols:
           # get median Tsys for each pol in this spw
            tsysmedians = TsysflagResults()
            spectrumstack = None
            for description in tsysspectra[pol].descriptions():
                tsysspectrum = tsysspectra[pol].last(description)
                if tsysspectrum.pol==pol:
                    if spectrumstack is None:
                        spectrumstack = tsysspectrum.data
                        flagstack = tsysspectrum.flag
                    else:
                        spectrumstack = np.vstack((tsysspectrum.data,
                          spectrumstack))
                        flagstack = np.vstack((tsysspectrum.flag,
                          flagstack))

            if spectrumstack is not None:
                stackmedian = np.zeros(np.shape(spectrumstack)[1])
                stackmedianflag = np.ones(np.shape(spectrumstack)[1], np.bool)

                for j in range(np.shape(spectrumstack)[1]):
                    valid_data = spectrumstack[:,j][np.logical_not(flagstack[:,j])]
                    if len(valid_data):
                        stackmedian[j] = np.median(valid_data)
                        stackmedianflag[j] = False

                tsysmedian = commonresultobjects.SpectrumResult(
                  data=stackmedian, 
                  datatype='Median Normalised Tsys',
                  filename=tsystable.name, spw=spwid, pol=pol,
                  intent=intent)

            tsysmedians.addview(tsysmedian.description, tsysmedian)

            # build the view, get values for axes, initialise pixels
            times = np.sort(list(times))
            data = np.zeros([antenna_ids[-1]+1, len(times)])
            flag = np.ones([antenna_ids[-1]+1, len(times)], np.bool)

            # get MAD of channel by channel derivative of Tsys
            # for each antenna, time/scan
            for description in tsysspectra[pol].descriptions():
                tsysspectrum = tsysspectra[pol].last(description)

                tsysdata = tsysspectrum.data
                tsysflag = tsysspectrum.flag

                deriv = tsysdata[1:] - tsysdata[:-1]
                deriv_flag = np.logical_or(tsysflag[1:], tsysflag[:-1])

                valid = deriv[np.logical_not(deriv_flag)]
                if len(valid):
                    metric = np.median(np.abs(valid - np.median(valid))) * 100

                    ant = tsysspectrum.ant
                    caltime = tsysspectrum.time
                    data[ant[0], caltime==times] = metric
                    flag[ant[0], caltime==times] = 0

            axes = [commonresultobjects.ResultAxis(name='Antenna1',
              units='id', data=np.arange(antenna_ids[-1]+1)),
              commonresultobjects.ResultAxis(name='Time', units='',
              data=times)]

            viewresult = commonresultobjects.ImageResult(
              filename=tsystable.name, data=data,
              flag=flag, axes=axes, datatype='100 * MAD of normalised derivative',
              spw=spwid, intent=intent, pol=pol, cell_index=pol)
            # store the spectra contributing to this view as 'children'
            viewresult.children['tsysmedians'] = tsysmedians
            viewresult.children['tsysspectra'] = tsysspectra[pol]

            # add the view results to the class result structure
            self.result.addview(viewresult.description, viewresult)
