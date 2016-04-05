from __future__ import absolute_import

import numpy as np 
import os
import types
from collections import defaultdict

import pipeline.infrastructure as infrastructure
from pipeline.hif.tasks.common import commoncalinputs
import pipeline.infrastructure.basetask as basetask
from pipeline.hif.tasks.flagging.flagdatasetter import FlagdataSetter

from .resultobjects import GainflagResults
from .resultobjects import GainflaggerResults
from .resultobjects import GainflaggerDataResults
from .resultobjects import GainflaggerViewResults
from ..common import commonresultobjects
from ..common import calibrationtableaccess as caltableaccess
from ..common import viewflaggers
from .. import bandpass
from .. import gaincal

LOG = infrastructure.get_logger(__name__)


class GainflagInputs(commoncalinputs.CommonCalibrationInputs):
    
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, 
      intent=None, spw=None, refant=None, niter=None,
      flag_mediandeviant=None, fmeddev_limit=None,
      flag_rmsdeviant=None, frmsdev_limit=None,
      metric_order=None):

        # set the properties to the values given as input arguments
        self._init_properties(vars())
        
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
    
    @property
    def flag_mediandeviant(self):
        return self._flag_mediandeviant

    @flag_mediandeviant.setter
    def flag_mediandeviant(self, value):
        if value is None:
            value = False
        self._flag_mediandeviant = value

    @property
    def flag_rmsdeviant(self):
        return self._flag_rmsdeviant

    @flag_rmsdeviant.setter
    def flag_rmsdeviant(self, value):
        if value is None:
            value = True
        self._flag_rmsdeviant = value

    @property
    def fmeddev_limit(self):
        return self._fmeddev_limit

    @fmeddev_limit.setter
    def fmeddev_limit(self, value):
        if value is None:
            value = 3.0
        self._fmeddev_limit = value

    @property
    def frmsdev_limit(self):
        return self._frmsdev_limit

    @frmsdev_limit.setter
    def frmsdev_limit(self, value):
        if value is None:
            value = 3.5
        self._frmsdev_limit = value

    @property
    def metric_order(self):
        return self._metric_order

    @metric_order.setter
    def metric_order(self, value):
        if value is None:
            value = 'mediandeviant, rmsdeviant'
        self._metric_order = value
    
    @property
    def niter(self):
        return self._niter

    @niter.setter
    def niter(self, value):
        if value is None:
            value = 1
        self._niter = value


class Gainflag(basetask.StandardTaskTemplate):
    Inputs = GainflagInputs

    def prepare(self):
        inputs = self.inputs
        
        # Initialize ordered list of metrics to evaluate
        metrics_to_evaluate = []
        
        # Convert metric order string to list
        metric_list = [metric.strip() for metric in inputs.metric_order.split(',')]
        
        # Convert requested flagging metrics to ordered list
        for metric in metric_list:
            if metric == 'mediandeviant' and inputs.flag_mediandeviant:
                metrics_to_evaluate.append(metric)
            if metric == 'rmsdeviant' and inputs.flag_rmsdeviant:
                metrics_to_evaluate.append(metric)

        # Collect requested flagging metrics that were not specified in
        # ordered metric lists, append them to end of list, and raise a warning.
        if inputs.flag_mediandeviant and 'mediandeviant' not in metric_list:
            LOG.warning('mediandeviant flagging metric requested but not specified in the definition of the order-in-which-to-evaluate-metrics; appending metric to the end.')
            metrics_to_evaluate.append('mediandeviant')
        if inputs.flag_rmsdeviant and 'rmsdeviant' not in metric_list:
            LOG.warning('rmsdeviant flagging metric requested but not specified in the definition of the order-in-which-to-evaluate-metrics; appending metric to the end.')
            metrics_to_evaluate.append('rmsdeviant')

        # Initialize result and store vis and order of metrics in result
        result = GainflagResults()
        result.vis = inputs.vis
        result.metric_order = metrics_to_evaluate
        
        # Run flagger for each metric
        for metric in metrics_to_evaluate:            
            LOG.info('Running Gainflagger for metric: {0}'.format(metric))
            
            if metric == 'mediandeviant':
                flaggerinputs = GainflaggerInputs(
                  context=inputs.context, output_dir=inputs.output_dir,
                  vis=inputs.vis, intent=None, spw=None, refant=None, niter=None,
                  metric=metric, prepend='flag {0} - '.format(metric),
                  flag_maxabs=True, fmax_limit=inputs.fmeddev_limit)
                flaggertask = Gainflagger(flaggerinputs)
            
            elif metric == 'rmsdeviant':
                flaggerinputs = GainflaggerInputs(
                  context=inputs.context, output_dir=inputs.output_dir,
                  vis=inputs.vis, intent=None, spw=None, refant=None, niter=None,
                  metric=metric, prepend='flag {0} - '.format(metric),
                  flag_maxabs=True, fmax_limit=inputs.frmsdev_limit)
                flaggertask = Gainflagger(flaggerinputs)
            
            result.add(metric, self._executor.execute(flaggertask))

        # Extract before and after flagging summaries from individual results:
        stats_before = result.components[metrics_to_evaluate[0]].summaries[0]
        stats_after = result.components[metrics_to_evaluate[-1]].summaries[-1]
        
        # Add the "before" and "after" flagging summaries to the final result
        result.summaries = [stats_before, stats_after]

        return result
    
    
    def analyse(self, result):
        return result
    

class GainflaggerInputs(commoncalinputs.CommonCalibrationInputs):
    
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, 
      intent=None, spw=None, refant=None, niter=None, 
      flag_maxabs=None, fmax_limit=None,
      metric=None, prepend=''):

        # set the properties to the values given as input arguments
        self._init_properties(vars())

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

    @property
    def metric(self):
        if self._metric is None:
            return 'mediandeviant'
        return self._metric

    @metric.setter
    def metric(self, value):
        self._metric = value

    @property
    def flag_maxabs(self):
        if self._flag_maxabs is None:
            return True
        return self._flag_maxabs

    @flag_maxabs.setter
    def flag_maxabs(self, value):
        self._flag_maxabs = value
        
    @property
    def fmax_limit(self):
        if self._fmax_limit is None:
            return 3.5
        return self._fmax_limit

    @fmax_limit.setter
    def fmax_limit(self, value):
        self._fmax_limit = value
        
    @property
    def niter(self):
        return self._niter

    @niter.setter
    def niter(self, value):
        if value is None:
            value = 1
        self._niter = value


class Gainflagger(basetask.StandardTaskTemplate):
    Inputs = GainflaggerInputs

    def prepare(self):
        inputs = self.inputs
        
        # Initialize result and store vis in result
        result = GainflaggerResults()
        result.vis = inputs.vis

        # Construct the task that will read the data.
        datainputs = GainflaggerDataInputs(context=inputs.context,
          output_dir=inputs.output_dir, vis=inputs.vis, intent=inputs.intent,
          spw=inputs.spw, refant=inputs.refant)
        datatask = GainflaggerData(datainputs)
        
        # Construct the generator that will create the view of the data
        # that is the basis for flagging.
        viewtask = GainflaggerView(context=inputs.context,
          output_dir=inputs.output_dir, vis=inputs.vis, intent=inputs.intent,
          spw=inputs.spw, refant=inputs.refant, metric=inputs.metric)

        # Construct the task that will set any flags raised in the
        # underlying data.
        flagsetterinputs = FlagdataSetter.Inputs(context=inputs.context,
          vis=inputs.vis, table=inputs.vis, inpfile=[])
        flagsettertask = FlagdataSetter(flagsetterinputs)

        # Translate the input flagging parameters to a more compact
        # list of rules.
        rules = viewflaggers.NewMatrixFlagger.make_flag_rules(
          flag_maxabs=inputs.flag_maxabs,
          fmax_limit=inputs.fmax_limit)

        # Construct the flagger task around the data view task  and the
        # flagger task. Extend any newly found flags by removing selection
        # of "FIELD" and "TIMERANGE". Extend any newly found flags in a spw
        # by flagging same in all spws of corresponding baseband.
        matrixflaggerinputs = viewflaggers.NewMatrixFlaggerInputs(
          context=inputs.context, output_dir=inputs.output_dir,
          vis=inputs.vis, datatask=datatask, viewtask=viewtask, 
          flagsettertask=flagsettertask, rules=rules, niter=inputs.niter,
          extendfields=['field', 'timerange'], extendbaseband=True,
          prepend=inputs.prepend, iter_datatask=True)
        flaggertask = viewflaggers.NewMatrixFlagger(matrixflaggerinputs)

        # Execute it to flag the data view
        flaggerresult = self._executor.execute(flaggertask)
        
        # Import views, flags, and "measurement set or caltable to flag"
        # into final result
        result.importfrom(flaggerresult)
        
        # Copy flagging summaries to final result
        result.summaries = flaggerresult.summaries
        
        return result
    
    
    def analyse(self, result):
        return result


class GainflaggerDataInputs(basetask.StandardInputs):
    
    def __init__(self, context, output_dir=None, vis=None, 
      intent=None, spw=None, refant=None):
        self._init_properties(vars())


class GainflaggerData(basetask.StandardTaskTemplate):
    Inputs = GainflaggerDataInputs

    def __init__(self, inputs):
        super(GainflaggerData, self).__init__(inputs)
    
    
    def prepare(self):
        inputs = self.inputs

        # Initialize result structure
        result = GainflaggerDataResults()
        result.vis = inputs.vis

        # Calculate a phased-up bpcal
        bpcal_inputs = bandpass.PhcorBandpass.Inputs(
          context=inputs.context, vis=inputs.vis,
          intent=inputs.intent, spw=inputs.spw, 
          refant=inputs.refant, solint='inf,7.8125MHz')
        bpcal_task = bandpass.PhcorBandpass(bpcal_inputs)
        bpcal = self._executor.execute(bpcal_task, merge=False)
        if not bpcal.final:
            LOG.warning('No bandpass solution computed for %s ' % (inputs.ms.basename))
        else:
            bpcal.accept(inputs.context)

        # Calculate gain phases
        gpcal_inputs = gaincal.GTypeGaincal.Inputs(
          context=inputs.context, vis=inputs.vis,
          intent=inputs.intent, spw=inputs.spw,
          refant=inputs.refant,
          calmode='p', minsnr=2.0, solint='int', gaintype='G')
        gpcal_task = gaincal.GTypeGaincal(gpcal_inputs)
        gpcal = self._executor.execute(gpcal_task, merge=False)
        if not gpcal.final:
            LOG.warning('No phase time solution computed for %s ' % (inputs.ms.basename))
        else:
            gpcal.accept(inputs.context)

        # Calculate gain amplitudes
        gacal_inputs = gaincal.GTypeGaincal.Inputs(
          context=inputs.context, vis=inputs.vis,
          intent=inputs.intent, spw=inputs.spw,
          refant=inputs.refant,
          calmode='a', minsnr=2.0, solint='int', gaintype='T')
        gacal_task = gaincal.GTypeGaincal(gacal_inputs)
        gacal = self._executor.execute(gacal_task, merge=False)
        
        # Store final gain table in result
        if not gacal.final:
            gatable = list(gacal.error)
            gatable = gatable[0].gaintable
            LOG.warning('No amplitude time solution computed for %s ' % (inputs.ms.basename))
            result.table = gatable
            result.table_available = False
        else:
            gacal.accept(inputs.context)
            gatable = gacal.final
            gatable = gatable[0].gaintable
            result.table = gatable
            result.table_available = True
            
        return result

    def analyse(self, result):
        return result

    
class GainflaggerView(object):

    def __init__(self, context, output_dir=None, vis=None, 
      intent=None, spw=None, refant=None, metric=None):
        
        self.context = context
        self.vis = vis
        self.intent = intent
        self.spw = spw
        self.refant = refant
        self.metric = metric
    
    
    def __call__(self, data):
        
        # Initialize result structure
        self.result = GainflaggerViewResults()

        if data.table_available:
            
            # Calculate the view
            gatable = data.table
            LOG.info ('Computing flagging metrics for caltable %s ' % (
                os.path.basename(gatable)))
            
            self.calculate_view(gatable, metric=self.metric)

        # Add visibility name to result
        self.result.vis = self.vis

        return self.result
    
    
    def calculate_view(self, table, metric='mediandeviant'):
        """
        Method to calculate the flagging view.
        
        Input parameters:
        
        table  -- Name of gain table to be analysed.
        metric -- the name of the view metric to use:
                    'mediandeviant' calculates per antenna:
                       abs(median(antenna) - median(all antennas)) / mad(all antennas).
                    'rmsdeviant' calculates per antenna:
                       stdev(antenna) / mad(all antennas).
                    Note: mad = median absolute deviation from the median
        """        
        
        # Create the view, based on the metric
        if metric in ['mediandeviant', 'rmsdeviant']:
            self.calculate_median_rms_deviant_view(table, metric)

        
    def calculate_median_rms_deviant_view(self, gtable, metric='mediantdeviant'):
        """
        Method to calculate either the "mediandeviant" or "rmsdeviant" flagging view.
        
        Input parameters:
        
        table  -- Name of gain table to be analysed.
        metric -- the name of the view metric to use:
                    'mediandeviant' calculates per antenna:
                       abs(median(antenna) - median(all antennas)) / mad(all antennas).
                    'rmsdeviant' calculates per antenna:
                       stdev(antenna) / mad(all antennas).
                    Note: mad = median absolute deviation from the median
        """        
        
        # Open caltable
        gtable = caltableaccess.CalibrationTableDataFiller.getcal(gtable)

        # Get antenna IDs
        ms = self.context.observing_run.get_ms(name=self.vis)
        antenna_ids = [antenna.id for antenna in ms.antennas] 
        antenna_ids.sort()
        antenna_name = {}
        for antenna_id in antenna_ids:
            antenna_name[antenna_id] = [antenna.name for antenna in ms.antennas 
              if antenna.id==antenna_id][0]

        # Get spectral window IDs
        spwids = [spw.id for spw in ms.get_spectral_windows(self.spw)]

        # Get range of times covered
        times = set()
        for row in gtable.rows:
            # The gain table is T, should be no pol dimension
            npols = np.shape(row.get('CPARAM'))[0]
            if npols != 1:
                raise Exception, 'table has polarization results'
            times.update([row.get('TIME')])
        times = np.sort(list(times))
        
        # make gain image for each spwid
        for spwid in spwids:
                        
            # Create lists to store flagging view in
            # (initialize flags as one, will be set to 0 later if valid data is found)           
            data = np.zeros([len(antenna_ids), 1])
            flag = np.ones([len(antenna_ids), 1], np.bool)

            # Read in data
            data_per_ant = defaultdict(list)
            data_all_ant = []
            for row in gtable.rows:
                # Only read in data from current spwid
                if row.get('SPECTRAL_WINDOW_ID') == spwid:
                    gain = row.get('CPARAM')[0][0]
                    ant = row.get('ANTENNA1')
                    gainflag = row.get('FLAG')[0][0]
                    # Only store non-flagged data
                    if not gainflag:
                        data_per_ant[ant].append(gain)
                        data_all_ant.append(gain)
                        # At least one good datapoint was found for this antenna, so set 
                        # flag to 0.
                        flag[ant] = 0
            
            # Convert "all antenna" data to numpy array
            data_all_ant = np.array(data_all_ant)

            # Calculate MAD for all antennas combined
            mad_all_ant = np.median(np.abs(data_all_ant - np.median(data_all_ant)))
            
            if metric == 'mediandeviant':
                # Calculate MAD for each antenna
                for ant in data_per_ant.keys():
                    
                    # Convert to numpy arrays
                    data_per_ant[ant] = np.array(data_per_ant[ant])
                    
                    # Calculate "abs(median(ant) - median(all_ant)) / MAD(all_ant)" and store in flagging view
                    data[ant] = np.abs(np.median(data_per_ant[ant]) - np.median(data_all_ant)) / mad_all_ant
            
            elif metric == 'rmsdeviant':
                # Calculate metric for each antenna
                for ant in data_per_ant.keys():
                    
                    # Convert to numpy arrays
                    data_per_ant[ant] = np.array(data_per_ant[ant])
                    
                    ## Calculate median absolute deviation from the median
                    #mad_ant = np.median(np.abs(data_per_ant[ant] - np.median(data_per_ant[ant])))
    
                    # Calculate standard deviation of antenna
                    stdev_ant = np.std(data_per_ant[ant])
    
                    # Store "MAD(ant) / MAD(all ant)" in flagging view
                    data[ant] = stdev_ant / mad_all_ant
            
            # Create axes for view result, set time on y-axis to the first timestamp
            axes = [commonresultobjects.ResultAxis(name='Antenna1',
              units='id', data=np.arange(antenna_ids[-1]+1)),
              commonresultobjects.ResultAxis(name='Time', units='',
              data=times[0:1])]

            # associate the result with a generic filename - using
            # specific names gives confusing duplicates on the weblog
            # display
            viewresult = commonresultobjects.ImageResult(filename=
              '%s(gtable)' % os.path.basename(gtable.vis),
              intent=self.intent,
              data=data, flag=flag,
              axes=axes, datatype='gain amplitude', spw=spwid)
          
            # add the view results and their children results to the
            # class result structure
            self.result.addview(viewresult.description, viewresult)            
