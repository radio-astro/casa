from __future__ import absolute_import
import numpy as np 
import re

from pipeline.hif.heuristics.tsysspwmap import tsysspwmap as tsysspwmap
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary

from .resultobjects import TsysflagResults

from pipeline.hif.tasks.common import commonresultobjects
from pipeline.hif.tasks.common import calibrationtableaccess as caltableaccess
from pipeline.hif.tasks.common import viewflaggers

LOG = infrastructure.get_logger(__name__)



class TsysflagedgesInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir=None, vis=None, caltable=None, 
      intentgroups=None, metric=None, flag_edges=None, edge_limit=None,
      niter=None):

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
            return ['*AMPLITUDE*+*BANDPASS*+*PHASE*+*TARGET*']
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

    @property
    def flag_edges(self):
        if self._flag_edges is None:
            return True
        return self._flag_edges

    @flag_edges.setter
    def flag_edges(self, value):
        self._flag_edges = value

    @property
    def edge_limit(self):
        if self._edge_limit is None:
            return 2
        return self._edge_limit

    @edge_limit.setter
    def edge_limit(self, value):
        self._edge_limit = value

    @property
    def niter(self):
        if self._niter is None:
            return 1
        return self._niter

    @niter.setter
    def niter(self, value):
        self._niter = value


class Tsysflagedges(basetask.StandardTaskTemplate):
    Inputs = TsysflagedgesInputs

    def is_multi_vis_task(self):
        return True

    def prepare(self):
        inputs = self.inputs

        # Construct the task that will read the data and create the
        # view of the data that is the basis for flagging.
        datainputs = TsysflagedgesWorkerInputs(context=inputs.context,
          output_dir=inputs.output_dir, caltable=inputs.caltable,
          vis=inputs.vis, intentgroups=inputs.intentgroups, 
          metric=inputs.metric)
        datatask = TsysflagedgesWorker(datainputs)

        # Construct the task that will set any flags raised in the
        # underlying data.
        flagsetterinputs = caltableaccess.CalibrationTableFlagSetterInputs(
          context=inputs.context, vis=inputs.vis)
        flagsettertask = caltableaccess.CalibrationTableFlagSetter(
          flagsetterinputs)

	# Translate the input flagging parameters to a more compact
        # list of rules.
	rules = viewflaggers.VectorFlagger.make_flag_rules (
          flag_edges=inputs.flag_edges, edge_limit=inputs.edge_limit)

        # Construct the flagger task around the data view task  and the
        # flagger task. When executed this will:
        #   loop:
        #     execute datatask to obtain view from underlying data
        #     examine view, raise flags
        #     execute flagsetter task to set flags in underlying data        
        #     exit loop if no flags raised or if # iterations > niter 
        vectorflaggerinputs = viewflaggers.VectorFlaggerInputs(
          context=inputs.context, output_dir=inputs.output_dir,
          vis=inputs.vis, datatask=datatask, flagsettertask=flagsettertask,
          rules=rules, niter=inputs.niter)
        flaggertask = viewflaggers.VectorFlagger(vectorflaggerinputs)

	# Execute it to flag the data view
        result = self._executor.execute(flaggertask)

        return result

    def analyse(self, result):
        return result


class TsysflagedgesWorkerInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None, vis=None, caltable=None,
      intentgroups=None, metric=None):
        self._init_properties(vars())


class TsysflagedgesWorker(basetask.StandardTaskTemplate):
    Inputs = TsysflagedgesWorkerInputs

    def __init__(self, inputs):
        super(TsysflagedgesWorker, self).__init__(inputs)
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

                intentgroupids[intentgroup] = groupids

	    LOG.info ('Computing flagging metrics for caltable %s ' % (name))
            for tsysspwid in tsysspws:
                for intentgroup in self.inputs.intentgroups:
                    # calculate view for each group of fieldids
                    self.calculate_view(tsystable, tsysspwid, intentgroup,
                      intentgroupids[intentgroup])

        self.result.final = final[:]

        return self.result

    def analyse(self, result):
        return result

    def calculate_view(self, tsystable, spwid, intent, fieldids, metric='median'):
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
        if metric == 'median':
            self.calculate_median_tsys(tsystable, spwid, intent,
              fieldids)

    def calculate_median_tsys(self, tsystable, spwid, intent, fieldids):
        """
        tsystable -- CalibrationTableData object giving access to the tsys
                     caltable.
        spwid     -- view will be calculated using data for this spw id.
        fieldids  -- view will be calculated using data for all field_ids in
                     this list.

        Data of the specified spwid, intent and range of fieldids are
        read from the given tsystable object. From all this one data 'view' 
        is created; a 'median' Tsys spectrum where for each channel the 
        value is the median of all the tsys spectra selected.
        """

        antennas = set()
        times = set()

        # dict of results for each pol, defaultdict ia preferred to simple {}
        # because it initialises a new (key,value) automatically if it is not
        # already present when demanded.
        tsysspectra = TsysflagResults()

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
                      ant=row.get('ANTENNA1'), pol=pol,
                      time=row.get('TIME'), normalise=True)

                    tsysspectra.addview(tsysspectrum.description,
                      tsysspectrum)
                    antennas.update([row.get('ANTENNA1')])
                    times.update([row.get('TIME')])

        # get median Tsys in this spw
        spectrumstack = None
        for description in tsysspectra.descriptions():
            tsysspectrum = tsysspectra.last(description)
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

            viewresult = commonresultobjects.SpectrumResult(
              data=stackmedian, flag=stackmedianflag,
              datatype='Median Normalised Tsys',
              filename=tsystable.name, spw=spwid,
              intent=intent)

            # add the view results and their children results to the
            # class result structure
            self.result.addview(viewresult.description, viewresult)
