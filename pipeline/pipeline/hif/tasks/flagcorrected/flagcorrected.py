from __future__ import absolute_import

import types

import numpy as np 

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools

from pipeline.hif.tasks.common import commonresultobjects
from pipeline.hif.tasks.common import commonhelpermethods
from pipeline.hif.tasks.common import viewflaggers

from pipeline.hif.tasks.flagging.flagdatasetter import FlagdataSetter

from .resultobjects import FlagcorrectedResults, FlagcorrectedDataResults, FlagcorrectedViewResults

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

        # Initialize result and store vis in result
        result = FlagcorrectedResults()
        result.vis = inputs.vis

        # Construct the task that will read the data.
        datainputs = FlagcorrectedDataInputs(context=inputs.context,
          vis=inputs.vis, spw=inputs.spw, intent=inputs.intent, 
          metric=inputs.metric)
        datatask = FlagcorrectedData(datainputs)
        
        # Construct the generator that will create the view of the data
        # that is the basis for flagging.
        viewtask = FlagcorrectedView(metric=inputs.metric)

        # Construct the task that will set any flags raised in the
        # underlying data.
        flagsetterinputs = FlagdataSetter.Inputs(context=inputs.context,
          vis=inputs.vis, table=inputs.vis, inpfile=[])
        flagsettertask = FlagdataSetter(flagsetterinputs)

        # Define which type of flagger to use.
        flagger = viewflaggers.NewMatrixFlagger

        # Translate the input flagging parameters to a more compact
        # list of rules.
        # FIXME: if metric='baseline', these flagging rules are possibly
        # not appropriate; need to add a switch for generating rules
        # depending on metric.
        rules = flagger.make_flag_rules (
          flag_bad_antenna=inputs.flag_bad_antenna, 
          fba_lo_limit=inputs.fba_lo_limit,
          fba_minsample=inputs.fba_minsample,
          fba_frac_limit=inputs.fba_frac_limit,
          fba_number_limit=inputs.fba_number_limit)
 
        # Construct the flagger task around the data view task and the
        # flagger task. 
        flaggerinputs = flagger.Inputs(
          context=inputs.context, output_dir=inputs.output_dir,
          vis=inputs.vis, datatask=datatask, viewtask=viewtask, 
          flagsettertask=flagsettertask, rules=rules, niter=inputs.niter,
          iter_datatask=True)
        flaggertask = flagger(flaggerinputs)

        # Execute the flagger task.
        flaggerresult = self._executor.execute(flaggertask)

        # Import views, flags, and "measurement set or caltable to flag"
        # into final result.
        result.importfrom(flaggerresult)

        # Copy flagging summaries to final result, and update
        # names to match expectations by renderer.
        result.summaries = flaggerresult.summaries
        result.summaries[0]['name'] = 'before'
        result.summaries[-1]['name'] = 'after'

        # Store flagging metric in result (for renderer).
        result.metric = inputs.metric

        return result

    def analyse(self, result):
        return result


class FlagcorrectedDataInputs(basetask.StandardInputs):
    
    def __init__(self, context, vis=None, spw=None,
      intent=None, metric=None):
        """
        Keyword arguments:
        spw    -- views are created for these spws.
        intent -- views are created for this intent.
        metric -- 'antenna' : create an "antenna" flagging view.
                  'baseline' : create a "baseline" flagging view.
        """
        
        self._init_properties(vars())


class FlagcorrectedData(basetask.StandardTaskTemplate):
    Inputs = FlagcorrectedDataInputs

    def __init__(self, inputs):
        super(FlagcorrectedData, self).__init__(inputs)
        
    def prepare(self):

        # Initialize result structure.
        result = FlagcorrectedDataResults()
        result.data = {}
        result.intent = self.inputs.intent
        result.new = True

        # Get metric from inputs.
        metric = self.inputs.metric

        # Take vis from inputs, and add to result.
        result.table = self.inputs.vis

        LOG.info ('Reading flagging view data for vis %s ' % (self.inputs.vis))
        
        # Get the spws to use.
        spwids = map(int, self.inputs.spw.split(','))

        # Get the MS object.
        ms = self.inputs.context.observing_run.get_ms(name=self.inputs.vis)

        # Get antenna names, ids
        self.antenna_name, self.antenna_ids = \
          commonhelpermethods.get_antenna_names(ms)
        ants = np.array(self.antenna_ids)
        nants = len(ants)

        # Create a list of antenna baselines.
        baselines = []
        for ant1 in ants:
            for ant2 in ants: 
                baselines.append('%s&%s' % (ant1, ant2))
        nbaselines = len(baselines)

        # Read in data for separately for each spw.
        for spwid in spwids:

            LOG.info('Reading flagging view data for spw %s' % spwid)

            # Get the correlation products.
            corrs = commonhelpermethods.get_corr_products(ms, spwid)
        
            # Open MS and read in required data.       
            with casatools.MSReader(ms.name) as openms:
                try:
                    openms.msselect({'scanintent':'*%s*' % self.inputs.intent, 'spw':str(spwid)})
                except:
                    LOG.warning('Unable to compute flagging view for spw %s' % spwid)
                    openms.close()
                    # Continue to next spw.
                    continue

                # Extract the unique time stamps present in the current selection of the MS.
                rec = openms.getdata(['time'])
                times = np.sort(np.unique(rec['time']))
                ntimes = len(times)
    
                # Initialize output arrays based on metric.
                if metric == 'baseline':
                    # Initialize the data with zeroes.
                    data = np.zeros([len(corrs), ntimes, nbaselines])
                    # Initialize the flagging state as ones, i.e. data=flagged.
                    flag = np.ones([len(corrs), ntimes, nbaselines], np.bool)
                elif metric == 'antenna':
                    # Initialize the data, and number of data points
                    # used, for the flagging view.
                    data = np.zeros([len(corrs), nants, ntimes], np.complex)
                    ndata = np.zeros([len(corrs), nants, ntimes], np.int)
                else:
                    raise Exception, 'bad metric: %s' % metric
    
                # Read in the MS data in chunks of limited number of rows at a time.
                openms.iterinit(maxrows=500)
                openms.iterorigin()
                iterating = True
                while iterating:
                    # Get first chunk of records; stop if no "corrected_data" are
                    # present.
                    rec = openms.getdata(['corrected_data', 'flag', 'antenna1',
                      'antenna2', 'time'])
                    if 'corrected_data' not in rec.keys():
                        break
    
                    for row in range(np.shape(rec['corrected_data'])[2]):
                        # Extract antennas and time stamp from current row.
                        ant1 = rec['antenna1'][row]
                        ant2 = rec['antenna2'][row]
                        tim = rec['time'][row]
    
                        # Skip auto-correlation data.                        
                        if ant1==ant2:
                            continue
    
                        # Treat data for each polarisation independently.
                        for icorr in range(len(corrs)):
                            # Get corrected data, flags, and derive unflagged data.
                            corrected_data = rec['corrected_data'][icorr,:,row]
                            corrected_flag = rec['flag'][icorr,:,row]
                            valid_data = corrected_data[corrected_flag==False]

                            # If unflagged data is found...
                            if len(valid_data):
                                # Store data in the correct output arrays, depending on 
                                # metric.
                                if metric == 'baseline':
                                    # Determine which baselines these data belong to.
                                    baseline1 = ant1 * (ants[-1] + 1) + ant2
                                    baseline2 = ant2 * (ants[-1] + 1) + ant1
                                    
                                    # Create complex measurement from real and imag
                                    # data.
                                    med_data_real = np.median(valid_data.real)
                                    med_data_imag = np.median(valid_data.imag)
                                    med_data = abs(complex(med_data_real, med_data_imag))
                                    
                                    # Store data for each baseline, and set corresponding
                                    # flag to unflagged.
                                    data[icorr, times==tim, baseline1] = med_data
                                    flag[icorr, times==tim, baseline1] = 0
                                    data[icorr, times==tim, baseline2] = med_data
                                    flag[icorr, times==tim, baseline2] = 0
                                elif metric == 'antenna':
                                    # Take median of unflagged data.
                                    med_data = np.median(valid_data)
                                    
                                    # Store data for each antenna, and set corresponding
                                    # flag to unflagged.
                                    data[icorr, ant1, times==tim] += med_data
                                    data[icorr, ant2, times==tim] += med_data
                                    ndata[icorr, ant1, times==tim] += 1
                                    ndata[icorr, ant2, times==tim] += 1
    
                    iterating = openms.iternext()
    
            # Process the read in data, if needed, and store in result.
            if metric == 'baseline':
                # Store data and related information for this spwid.
                result.data[spwid] = {
                  'data': data,
                  'flag': flag,
                  'times': times,
                  'baselines': baselines,
                  'corrs': corrs}
                
            elif metric == 'antenna':
                # Calculate the average data value.
                # Suppress any divide by 0 error messages.
                old_settings = np.seterr(divide='ignore')
                data /= ndata
                np.seterr(**old_settings)

                # Take the absolute value of the data points.
                data = np.abs(data)
                
                # Set flagging state to "True" wherever no data points were available.
                flag = ndata==0

                # Store data and related information for this spwid.
                result.data[spwid] = {
                  'data': data,
                  'flag': flag,
                  'times': times,
                  'ants': ants,
                  'corrs': corrs}

        return result

    def analyse(self, result):
        return result


class FlagcorrectedView(object):

    def __init__(self, metric=None):
        """
        Creates an FlagcorrectedView instance.

        Keyword arguments:
        metric    -- the name of the view metric:
                        'antenna' creates an antenna flagging view.
                        'baseline' creates a baseline flagging view.
        """
        
        # Store input parameters.
        self.metric = metric


    def __call__(self, dataresult):
        """
        When called, the FlagcorrectedView object calculates flagging views
        for the vis / table provided by FlagcorrectedDataResults.

        dataresult  -- FlagcorrectedDataResults object.
                    
        Returns:
        FlagcorrectedViewResults object containing the flagging view.
        """
        
        # Initialize result structure.
        self.result = FlagcorrectedViewResults()

        # Take vis from data task results, and add to result.
        self.result.vis = dataresult.table

        LOG.info ('Computing flagging metrics for vis %s ' % (self.result.vis))

        # Check if dataresult is new for current iteration, or created during 
        # a previous iteration.
        
        if dataresult.new:
            # If the dataresult is from a newly run datatask, then 
            # create the flagging view.

            # Get intent from dataresult
            intent = dataresult.intent

            # The dataresult should have stored separate results
            # for each spwid it could find data for in the MS.
            # Create a separate flagging view for each spw.
            for spwid, spwdata in dataresult.data.items():
   
                LOG.info('Calculating flagging view for spw %s' % spwid)

                # Create axes for flagging view.
                if self.metric == 'baseline':
                    axes = [
                      commonresultobjects.ResultAxis(name='Time',
                      units='', data=spwdata['times']),
                      commonresultobjects.ResultAxis(name='Baseline',
                      units='', data=np.array(spwdata['baselines']), channel_width=1)]
                elif self.metric == 'antenna':
                    axes = [
                      commonresultobjects.ResultAxis(name='Antenna',
                      units='', data=spwdata['ants'], channel_width=1),
                      commonresultobjects.ResultAxis(name='Time',
                      units='', data=spwdata['times'])]
    
                # From the data array, create a view for each polarisation. 
                for icorr,corrlist in enumerate(spwdata['corrs']):
                    corr = corrlist[0]
    
                    viewresult = commonresultobjects.ImageResult(
                      filename=self.result.vis, data=spwdata['data'][icorr],
                      flag=spwdata['flag'][icorr], axes=axes, datatype='Mean amplitude',
                      spw=spwid, pol=corr, intent=intent)
    
                    # add the view results to the result structure
                    self.result.addview(viewresult.description, viewresult)
        else:
            # If the dataresult is from a previously run datatask, then
            # the datatask is not being iterated, and instead the 
            # flagging view will be taken from an earlier flagging view.
            
            # Check in the result structure stored in this instance of RawflagchansView
            # for the presence of previous result descriptions.
            prev_descriptions = self.result.descriptions()
            if prev_descriptions:
                # If result descriptions are present, then this instance was called 
                # before, and views were already calculated previously. 
                # Since the current view will be very similar to the last, it is
                # approximated to be identical, which saves having to recalculate.
                for description in prev_descriptions:
                    
                    # Get a deep copy to the last result belonging to the description.
                    prev_result = self.result.last(description)
    
                    # Store this view as the new view in the result structure.
                    self.result.addview(description, prev_result)
            else:
                LOG.error('Not iterating datatask, but no previous flagging '
                          ' views available to create a refined view from. '
                          'Cannot create flagging views.')

        return self.result

