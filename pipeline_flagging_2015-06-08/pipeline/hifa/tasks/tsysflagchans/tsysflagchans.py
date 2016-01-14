from __future__ import absolute_import

import collections
import numpy as np 
import re

from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.utils as utils
from pipeline.hif.tasks.flagging.flagdatasetter import FlagdataSetter

from pipeline.hif.tasks.common import commonresultobjects
from pipeline.hif.tasks.common import calibrationtableaccess as caltableaccess
from pipeline.hif.tasks.common import commonhelpermethods
from pipeline.hif.tasks.common import viewflaggers

from ..tsysflagspectra import TsysflagspectraResults

LOG = infrastructure.get_logger(__name__)

def intent_ids(intent, ms):
    """Get the fieldids associated with the given intents.
    """  

    # translate '*PHASE*+*TARGET*' or '*PHASE*,*TARGET*' to
    # regexp form '.*PHASE.*|.*TARGET.*'
    re_intent = intent.replace(' ', '')
    re_intent = re_intent.replace('*', '.*')
    re_intent = re_intent.replace('+', '|')
    re_intent = re_intent.replace(',', '|')

    # translate intents to fieldids - have to be careful that
    # PHASE ids have PHASE intent and no other
    ids = []
    if re.search(pattern=re_intent, string='AMPLITUDE'):
        ids += [field.id for field in ms.fields if 'AMPLITUDE'
          in field.intents]
    if re.search(pattern=re_intent, string='BANDPASS'):
        ids += [field.id for field in ms.fields if 'BANDPASS'
          in field.intents]
    if re.search(pattern=re_intent, string='PHASE'):
        ids += [field.id for field in ms.fields if 
          'PHASE' in field.intents and
          'BANDPASS' not in field.intents and
          'AMPLITUDE' not in field.intents]
    if re.search(pattern=re_intent, string='TARGET'):
        ids += [field.id for field in ms.fields if 'TARGET'
          in field.intents]
    if re.search(pattern=re_intent, string='ATMOSPHERE'):
        ids += [field.id for field in ms.fields if 'ATMOSPHERE'
          in field.intents]
        
    return ids


class TsysflagchansInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, caltable=None, 
      intentgroups=None, metric=None,
      flag_edges=None, edge_limit=None, 
      flag_sharps=None, sharps_limit=None, niter=None, prepend=''):

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
    def intentgroups(self):
        if self._intentgroups is None:
            return ['ATMOSPHERE']
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
            return 3.0
        return self._edge_limit

    @edge_limit.setter
    def edge_limit(self, value):
        self._edge_limit = value

    @property
    def flag_sharps(self):
        if self._flag_sharps is None:
            return False
        return self._flag_sharps

    @flag_sharps.setter
    def flag_sharps(self, value):
        self._flag_sharps = value

    @property
    def sharps_limit(self):
        if self._sharps_limit is None:
            return 0.05
        return self._sharps_limit

    @sharps_limit.setter
    def sharps_limit(self, value):
        self._sharps_limit = value

    @property
    def niter(self):
        if self._niter is None:
            return 1
        return self._niter

    @niter.setter
    def niter(self, value):
        self._niter = value


class Tsysflagchans(basetask.StandardTaskTemplate):
    Inputs = TsysflagchansInputs

    def prepare(self):
        inputs = self.inputs

        # Construct the task that will read the data and create the
        # view of the data that is the basis for flagging.
        datainputs = TsysflagchansWorkerInputs(context=inputs.context,
          output_dir=inputs.output_dir, caltable=inputs.caltable,
          vis=inputs.vis, intentgroups=inputs.intentgroups, 
          metric=inputs.metric)
        datatask = TsysflagchansWorker(datainputs)

        # Construct the task that will set any flags raised in the
        # underlying data.
        flagsetterinputs = FlagdataSetter.Inputs(context=inputs.context,
          vis=inputs.vis, table=inputs.caltable, inpfile=[])
        flagsettertask = FlagdataSetter(flagsetterinputs)

        # Depending on the flagging metric, select the appropriate type of 
        # flagger (Vector vs. Matrix), and translate the appropriate input
        # flagging parameters to a more compact list of rules.
        if inputs.metric != 'channel_time':
            rules = viewflaggers.VectorFlagger.make_flag_rules(
              flag_edges=inputs.flag_edges, 
              edge_limit=inputs.edge_limit,
              flag_sharps=inputs.flag_sharps, 
              sharps_limit=inputs.sharps_limit)
            flagger = viewflaggers.VectorFlagger
        else:
            rules = viewflaggers.MatrixFlagger.make_flag_rules()
            flagger = viewflaggers.MatrixFlagger

        # Construct the flagger task around the data view task and the
        # flagsetter task.
        flaggerinputs = flagger.Inputs(
          context=inputs.context, output_dir=inputs.output_dir,
          vis=inputs.vis, datatask=datatask, flagsettertask=flagsettertask,
          rules=rules, niter=inputs.niter, prepend=inputs.prepend)
        flaggertask = flagger(flaggerinputs)

        # Create a "before" summary of the flagging state
        summary_job = casa_tasks.flagdata(vis=inputs.caltable, mode='summary')
        stats_before = self._executor.execute(summary_job)

        # Execute the flagger task
        result = self._executor.execute(flaggertask)
        
        # Create an "after" summary of the flagging state
        summary_job = casa_tasks.flagdata(vis=inputs.caltable, mode='summary')
        stats_after = self._executor.execute(summary_job)

        # Copy flagging summaries to final result        
        result.summaries = [stats_before, stats_after]

        return result

    def analyse(self, result):
        return result


class TsysflagchansWorkerInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None, vis=None, caltable=None,
      intentgroups=None, metric=None):
        self._init_properties(vars())


class TsysflagchansWorker(basetask.StandardTaskTemplate):

    # This task assumes that 'tsyscal' has been called and its Tsys table
    # has been accepted into the context 
    
    Inputs = TsysflagchansWorkerInputs

    def __init__(self, inputs):
        super(TsysflagchansWorker, self).__init__(inputs)
        self.result = TsysflagspectraResults()

    def prepare(self):
       
        inputs = self.inputs

        # Get the tsys table name
        name = inputs.caltable

        # Load the tsystable from file into memory, store vis in result
        tsystable = caltableaccess.CalibrationTableDataFiller.getcal(name)
        self.result.vis = tsystable.vis

        # Get the Tsys spw map by retrieving it from the first tsys CalFrom 
        # that is present in the callibrary. 
        spwmap = utils.get_calfroms(inputs, 'tsys')[0].spwmap

        # Construct a callibrary entry, and store in result. This entry was already
        # created and merged into the context by tsyscal, but we recreate it here
        # since it is needed by the weblog renderer to create a Tsys summary chart.
        calto = callibrary.CalTo(vis=tsystable.vis)
        calfrom = callibrary.CalFrom(name, caltype='tsys', spwmap=spwmap)
        calapp = callibrary.CalApplication(calto, calfrom)
        self.result.final = [calapp]

        # Get the MS object from the context
        ms = self.inputs.context.observing_run.get_ms(name=tsystable.vis)

        # Get the spws from the tsystable.
        tsysspws = set()
        for row in tsystable.rows:
            tsysspws.update([row.get('SPECTRAL_WINDOW_ID')])

        # Get ids of fields for intent groups of interest
        intentgroupids = {}
        for intentgroup in self.inputs.intentgroups:
            groupids = intent_ids(intentgroup, ms)
            intentgroupids[intentgroup] = groupids

        # Compute the flagging view for every spw and every intentgroup
        LOG.info ('Computing flagging metrics for caltable %s ' % (name))
        for tsysspwid in tsysspws:
            for intentgroup in self.inputs.intentgroups:
                self.calculate_view(tsystable, tsysspwid, intentgroup,
                  intentgroupids[intentgroup], metric=self.inputs.metric)

        return self.result

    def analyse(self, result):
        return result

    def calculate_view(self, tsystable, spwid, intent, fieldids, 
      metric='median'):
        """
        tsystable -- CalibrationTableData object giving access to the tsys
                     caltable.
        spwid     -- view will be calculated using data for this spw id.
        fieldids  -- view will be calculated using data for all field_ids in
                     this list.
        metric    -- the name of the view metric:
                        'median' gives median Tsys spectra for each spw.
                        'antenna_median' gives median Tsys spectra for each
                        spw/antenna.
                        'antenna_diff' gives difference spectra between
                        the antenna median spectra and spw median.
        """
        if metric == 'median':
            self.calculate_median_tsys(tsystable, spwid, intent,
              fieldids)
        elif metric == 'antenna_median':
            self.calculate_median_antenna_tsys(tsystable, spwid, intent,
              fieldids)
        elif metric == 'channel_time':
            self.calculate_channel_time_image(tsystable, spwid, intent,
              fieldids)
        elif metric == 'antenna_diff':
            self.calculate_antenna_diff_tsys(tsystable, spwid, intent,
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

        # Get the MS object from the context
        ms = self.inputs.context.observing_run.get_ms(name=self.inputs.vis)

        # Get antenna names
        antenna_names, _ = commonhelpermethods.get_antenna_names(ms)

        # Get names of polarisations, and create polarisation index 
        corr_type = commonhelpermethods.get_corr_axis(ms, spwid)
        pols = range(len(corr_type))

        # Select Tsysspectra and corresponding times for specified spwid and fieldids
        tsysspectra, _, _ = self.get_tsystable_data(tsystable, spwid, fieldids, 
                                                                antenna_names, pols,
                                                                normalise=True)

        # Initialize a stack of all Tsys spectra
        spectrumstack = None

        # Create a stack of all Tsys spectra    
        for pol in pols:
            for description in tsysspectra[pol].descriptions():
                tsysspectrum = tsysspectra[pol].last(description)
                if spectrumstack is None:
                    spectrumstack = tsysspectrum.data
                    flagstack = tsysspectrum.flag
                else:
                    spectrumstack = np.vstack((tsysspectrum.data,
                      spectrumstack))
                    flagstack = np.vstack((tsysspectrum.flag,
                      flagstack))

        # From the stack of Tsys spectra, create a median Tsys 
        # spectrum and corresponding flagging state, convert to a
        # SpectrumResult, and store this as a view in the 
        # final class result structure:
        if spectrumstack is not None:

            # Initialize median spectrum and corresponding flag list
            stackmedian = np.zeros(np.shape(spectrumstack)[1])
            stackmedianflag = np.ones(np.shape(spectrumstack)[1], np.bool)
            
            # Calculate median Tsys spectrum from spectrum stack
            for j in range(np.shape(spectrumstack)[1]):
                valid_data = spectrumstack[:,j][np.logical_not(flagstack[:,j])]
                if len(valid_data):
                    stackmedian[j] = np.median(valid_data)
                    stackmedianflag[j] = False

            # Create a view result from the median Tsys spectrum,
            # and store it in the final result.
            viewresult = commonresultobjects.SpectrumResult(
              data=stackmedian, flag=stackmedianflag,
              datatype='Median Normalised Tsys',
              filename=tsystable.name, spw=spwid,
              intent=intent)
            self.result.addview(viewresult.description, viewresult)

    def calculate_median_antenna_tsys(self, tsystable, spwid, intent, fieldids):
        """
        tsystable -- CalibrationTableData object giving access to the tsys
                     caltable.
        spwid     -- view will be calculated using data for this spw id.
        fieldids  -- view will be calculated using data for all field_ids in
                     this list.

        Data of the specified spwid, intent and range of fieldids are
        read from the given tsystable object. From all this a series
        of 'views' are created, one for each antenna. Each 'view'
        is the 'median' Tsys spectrum, where for each channel the 
        value is the median of the tsys spectra selected.
        """

        # Get the MS object from the context
        ms = self.inputs.context.observing_run.get_ms(name=self.inputs.vis)

        # Get antenna names, ids
        antenna_names, antenna_ids = commonhelpermethods.get_antenna_names(ms)

        # Get names of polarisations, and create polarisation index 
        corr_type = commonhelpermethods.get_corr_axis(ms, spwid)
        pols = range(len(corr_type))

        # Select Tsysspectra and corresponding times for specified spwid and fieldids
        tsysspectra, _, _ = self.get_tsystable_data(tsystable, spwid, fieldids, 
                                                                antenna_names, pols,
                                                                normalise=True)

        # Initialize dictionary of spectrum stacks for antennas, and 
        # a dictionary for corresponding flagging states
        ant_spectrumstack = {}
        ant_flagstack = {}

        # Create a separate stack of all Tsys spectra for each antenna
        for pol in pols:
            for description in tsysspectra[pol].descriptions():
                tsysspectrum = tsysspectra[pol].last(description)
    
                for antenna_id in antenna_ids:
                    if tsysspectrum.ant[0] != antenna_id:
                        continue
    
                    if antenna_id not in ant_spectrumstack.keys():
                        ant_spectrumstack[antenna_id] = tsysspectrum.data
                        ant_flagstack[antenna_id] = tsysspectrum.flag
                    else:
                        ant_spectrumstack[antenna_id] = np.vstack((tsysspectrum.data,
                          ant_spectrumstack[antenna_id]))
                        ant_flagstack[antenna_id] = np.vstack((tsysspectrum.flag,
                          ant_flagstack[antenna_id]))

        # Create separate flagging views for each antenna
        for antenna_id in antenna_ids:
            
            if ant_spectrumstack[antenna_id] is not None:

                # Initialize the data and flagging state for the flagging view
                stackmedian = np.zeros(np.shape(ant_spectrumstack[antenna_id])[1])
                stackmedianflag = np.ones(np.shape(ant_spectrumstack[antenna_id])[1], np.bool)
                
                # Populate the flagging view based on the flagging metric
                for j in range(np.shape(ant_spectrumstack[antenna_id])[1]):
                    valid_data = ant_spectrumstack[antenna_id][:,j][np.logical_not(ant_flagstack[antenna_id][:,j])]
                    if len(valid_data):
                        stackmedian[j] = np.median(valid_data)
                        stackmedianflag[j] = False
            
                # Convert flagging view into an SpectrumResult
                viewresult = commonresultobjects.SpectrumResult(
                  data=stackmedian, flag=stackmedianflag,
                  datatype='Median Normalised Tsys',
                  filename=tsystable.name, spw=spwid,
                  intent=intent,
                  ant=(antenna_id, antenna_names[antenna_id]))

                # Add the view result to the class result structure
                self.result.addview(viewresult.description, viewresult)

    def calculate_antenna_diff_tsys(self, tsystable, spwid, intent, fieldids):
        """
        tsystable -- CalibrationTableData object giving access to the tsys
                     caltable.
        spwid     -- view will be calculated using data for this spw id.
        fieldids  -- view will be calculated using data for all field_ids in
                     this list.

        Data of the specified spwid, intent and range of fieldids are
        read from the given tsystable object. From all this a series
        of 'views' are created, one for each antenna. Each 'view'
        is the difference spectrum resulting from the subtraction
        of the 'spw median' Tsys spectrum from the 'antenna/spw median'
        spectrum.
        """

        # Get the MS object from the context
        ms = self.inputs.context.observing_run.get_ms(name=self.inputs.vis)

        # Get antenna names, ids
        antenna_names, antenna_ids = commonhelpermethods.get_antenna_names(ms)

        # Get names of polarisations, and create polarisation index 
        corr_type = commonhelpermethods.get_corr_axis(ms, spwid)
        pols = range(len(corr_type))

        # Select Tsysspectra and corresponding times for specified spwid and fieldids
        tsysspectra, _, _ = self.get_tsystable_data(tsystable,
          spwid, fieldids, antenna_names, pols, normalise=True)

        # Initialize spectrum stacks for antennas and for spw
        spw_spectrumstack = None
        ant_spectrumstack = {}
        ant_flagstack = {}

        # Create a stack of all Tsys spectra for specified spw,
        # as well as separate stacks for each antenna
        for pol in pols:
            for description in tsysspectra[pol].descriptions():
                tsysspectrum = tsysspectra[pol].last(description)
                
                # Update full stack of all Tsys spectra
                if spw_spectrumstack is None:
                    spw_spectrumstack = tsysspectrum.data
                    spw_flagstack = tsysspectrum.flag
                else:
                    spw_spectrumstack = np.vstack((tsysspectrum.data,
                      spw_spectrumstack))
                    spw_flagstack = np.vstack((tsysspectrum.flag,
                      spw_flagstack))
    
                # Update antenna-specific stacks
                for antenna_id in antenna_ids:
                    if tsysspectrum.ant[0] != antenna_id:
                        continue
    
                    if antenna_id not in ant_spectrumstack.keys():
                        ant_spectrumstack[antenna_id] = tsysspectrum.data
                        ant_flagstack[antenna_id] = tsysspectrum.flag
                    else:
                        ant_spectrumstack[antenna_id] = np.vstack((tsysspectrum.data,
                          ant_spectrumstack[antenna_id]))
                        ant_flagstack[antenna_id] = np.vstack((tsysspectrum.flag,
                          ant_flagstack[antenna_id]))

        # From the stack of all Tsys spectra for the specified spw, 
        # create a median Tsys spectrum and corresponding flagging state
        if spw_spectrumstack is not None:

            # Initialize median spectrum and corresponding flag list
            # and populate with valid data
            spw_median = np.zeros(np.shape(spw_spectrumstack)[1])
            spw_medianflag = np.ones(np.shape(spw_spectrumstack)[1], np.bool)
            for j in range(np.shape(spw_spectrumstack)[1]):
                valid_data = spw_spectrumstack[:,j][np.logical_not(spw_flagstack[:,j])]
                if len(valid_data):
                    spw_median[j] = np.median(valid_data)
                    spw_medianflag[j] = False

        # Create separate flagging views for each antenna
        for antenna_id in antenna_ids:

            if ant_spectrumstack[antenna_id] is not None:

                # Initialize the data and flagging state for the flagging view
                ant_median = np.zeros(np.shape(ant_spectrumstack[antenna_id])[1])
                ant_medianflag = np.ones(np.shape(ant_spectrumstack[antenna_id])[1], np.bool)
                
                # Populate the flagging view based on the flagging metric:
                # Calculate diff between median for spw/antenna and median for spw
                for j in range(np.shape(ant_spectrumstack[antenna_id])[1]):
                    valid_data = ant_spectrumstack[antenna_id][:,j][np.logical_not(ant_flagstack[antenna_id][:,j])]
                    if len(valid_data):
                        ant_median[j] = np.median(valid_data)
                        ant_medianflag[j] = False
                ant_median -= spw_median
                ant_medianflag = (ant_medianflag | spw_medianflag)
            
                # Convert flagging view into an SpectrumResult
                viewresult = commonresultobjects.SpectrumResult(
                  data=ant_median, flag=ant_medianflag,
                  datatype='Normalised Tsys Difference',
                  filename=tsystable.name, spw=spwid,
                  intent=intent,
                  ant=(antenna_id, antenna_names[antenna_id]))

                # Add the view result to the class result structure
                self.result.addview(viewresult.description, viewresult)

    def calculate_channel_time_image(self, tsystable, spwid, intent, fieldids):
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

        # Get the MS object from the context
        ms = self.inputs.context.observing_run.get_ms(name=self.inputs.vis)

        # Get antenna names, ids
        antenna_names, antenna_ids = commonhelpermethods.get_antenna_names(ms)

        # Get names of polarisations, and create polarisation index 
        corr_type = commonhelpermethods.get_corr_axis(ms, spwid)
        pols = range(len(corr_type))

        # Select Tsysspectra and corresponding times for specified spwid and fieldids
        tsysspectra, times, nchannels = self.get_tsystable_data(tsystable,
          spwid, fieldids, antenna_names, pols, normalise=True)

        # Create separate flagging views for each antenna and each polarisation
        for antenna_id in antenna_ids:
            
            for pol in pols:

                # Initialize the data and flagging state for the flagging view,
                # and get values for the 'times' axis.
                times = np.sort(list(times))
                data = np.zeros([nchannels, len(times)])
                flag = np.ones([nchannels, len(times)], np.bool)

                # Populate the flagging view based on the flagging metric,
                # for each antenna, time/scan
                for description in tsysspectra[pol].descriptions():
                    tsysspectrum = tsysspectra[pol].last(description)

                    if antenna_id==tsysspectrum.ant[0]:
                        caltime = tsysspectrum.time
                        data[:, caltime==times] = tsysspectrum.data.reshape([nchannels,1])
                        flag[:, caltime==times] = tsysspectrum.flag.reshape([nchannels,1])

                # Create axes for flagging view
                axes = [commonresultobjects.ResultAxis(name='Channel',
                  units='', data=np.arange(nchannels)),
                  commonresultobjects.ResultAxis(name='Time', units='',
                  data=times)]

                # Convert flagging view into an ImageResult
                viewresult = commonresultobjects.ImageResult(
                  filename=tsystable.name, data=data,
                  flag=flag, axes=axes, datatype='Tsys',
                  ant=(antenna_id, antenna_names[antenna_id]),
                  spw=spwid, intent=intent, pol=pol, cell_index=pol)

                # Add the view result to the class result structure
                self.result.addview(viewresult.description, viewresult)
    
    def get_tsystable_data(self, tsystable, spwid, fieldids, antenna_names, pols, normalise=None):

        # Initialize a dictionary of Tsys spectra results and corresponding times
        tsysspectra = collections.defaultdict(TsysflagspectraResults)
        times = set()

        if normalise:
            datatype = 'Normalised Tsys'
        else:
            datatype = 'Tsys'
            
        # pol=corr_type[pol][0]

        # Select rows from tsystable that match the specified spw and fields,
        # store a Tsys spectrum for each polarisation in the tsysspectra results
        # and store the corresponding time.
        for row in tsystable.rows:
            if row.get('SPECTRAL_WINDOW_ID') == spwid and \
              row.get('FIELD_ID') in fieldids:

                for pol in pols:
                    tsysspectrum = commonresultobjects.SpectrumResult(
                      data=row.get('FPARAM')[pol,:,0],
                      flag=row.get('FLAG')[pol,:,0],
                      datatype=datatype, filename=tsystable.name,
                      field_id=row.get('FIELD_ID'),
                      spw=row.get('SPECTRAL_WINDOW_ID'),
                      ant=(row.get('ANTENNA1'),
                      antenna_names[row.get('ANTENNA1')]), units='K',
                      pol=pol,
                      time=row.get('TIME'), normalise=normalise)

                    tsysspectra[pol].addview(tsysspectrum.description,
                      tsysspectrum)
                    times.update([row.get('TIME')])

                # Store the number of channels
                nchannels = np.shape(row.get('FPARAM'))[1]

        return tsysspectra, times, nchannels