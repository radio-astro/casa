from __future__ import absolute_import

import os
from collections import defaultdict

import numpy as np

import pipeline.domain as domain
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
from pipeline.h.tasks.common import calibrationtableaccess as caltableaccess
from pipeline.h.tasks.common import commonresultobjects
from pipeline.h.tasks.common import viewflaggers
from pipeline.h.tasks.flagging.flagdatasetter import FlagdataSetter
from .resultobjects import GainflagResults
from .resultobjects import GainflaggerDataResults
from .resultobjects import GainflaggerResults
from .resultobjects import GainflaggerViewResults
from .. import bandpass
from .. import gaincal

LOG = infrastructure.get_logger(__name__)


class GainflagInputs(vdp.StandardInputs):
    """
    GainflagInputs defines the inputs for the Gainflag pipeline task.
    """
    flag_mediandeviant = vdp.VisDependentProperty(default=False)
    flag_nrmsdeviant = vdp.VisDependentProperty(default=True)
    flag_rmsdeviant = vdp.VisDependentProperty(default=False)
    fmeddev_limit = vdp.VisDependentProperty(default=3.0)
    fnrmsdev_limit = vdp.VisDependentProperty(default=6.0)
    frmsdev_limit = vdp.VisDependentProperty(default=8.0)

    @vdp.VisDependentProperty
    def intent(self):
        # default to the intent that would be used for bandpass
        # calibration
        bp_inputs = bandpass.PhcorBandpass.Inputs(context=self.context, vis=self.vis, intent=None)
        return bp_inputs.intent

    metric_order = vdp.VisDependentProperty(default='mediandeviant, rmsdeviant, nrmsdeviant')
    niter = vdp.VisDependentProperty(default=1)

    @vdp.VisDependentProperty
    def refant(self):
        LOG.todo('What happens if self.ms is None?')
        # we cannot find the context value without the measurement set
        if not self.ms:
            return None

        # get the reference antenna for this measurement set
        ant = self.ms.reference_antenna
        if isinstance(ant, list):
            ant = ant[0]

        # return the antenna name/id if this is an Antenna domain object
        if isinstance(ant, domain.Antenna):
            return getattr(ant, 'name', ant.id)

        # otherwise return whatever we found. We assume the calling function
        # knows how to handle an object of this type.
        return ant

    @vdp.VisDependentProperty
    def spw(self):
        science_spws = self.ms.get_spectral_windows(with_channels=True, science_windows_only=True)
        return ','.join([str(spw.id) for spw in science_spws])

    def __init__(self, context, output_dir=None, vis=None, intent=None, spw=None, refant=None, niter=None,
                 flag_mediandeviant=None, fmeddev_limit=None, flag_rmsdeviant=None, frmsdev_limit=None,
                 flag_nrmsdeviant=None, fnrmsdev_limit=None, metric_order=None):
        super(GainflagInputs, self).__init__()

        # pipeline inputs
        self.context = context
        # vis must be set first, as other properties may depend on it
        self.vis = vis
        self.output_dir = output_dir

        # data selection arguments
        self.intent = intent
        self.spw = spw
        self.refant = refant

        # flagging parameters
        self.flag_mediandeviant = flag_mediandeviant
        self.fmeddev_limit = fmeddev_limit
        self.flag_rmsdeviant = flag_rmsdeviant
        self.frmsdev_limit = frmsdev_limit
        self.flag_nrmsdeviant = flag_nrmsdeviant
        self.fnrmsdev_limit = fnrmsdev_limit
        self.metric_order = metric_order
        self.niter = niter


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
            if metric == 'nrmsdeviant' and inputs.flag_nrmsdeviant:
                metrics_to_evaluate.append(metric)

        # Collect requested flagging metrics that were not specified in
        # ordered metric lists, append them to end of list, and raise a warning.
        if inputs.flag_mediandeviant and 'mediandeviant' not in metric_list:
            LOG.warning(
                'mediandeviant flagging metric requested but not specified in'
                ' the definition of the order-in-which-to-evaluate-metrics;'
                ' appending metric to the end.')
            metrics_to_evaluate.append('mediandeviant')
        if inputs.flag_rmsdeviant and 'rmsdeviant' not in metric_list:
            LOG.warning(
                'rmsdeviant flagging metric requested but not specified in'
                ' the definition of the order-in-which-to-evaluate-metrics;'
                ' appending metric to the end.')
            metrics_to_evaluate.append('rmsdeviant')
        if inputs.flag_nrmsdeviant and 'nrmsdeviant' not in metric_list:
            LOG.warning(
                'nrmsdeviant flagging metric requested but not specified in'
                ' the definition of the order-in-which-to-evaluate-metrics;'
                ' appending metric to the end.')
            metrics_to_evaluate.append('nrmsdeviant')

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
                    vis=inputs.vis, intent=inputs.intent, spw=None,
                    refant=None, niter=None, metric=metric,
                    prepend='flag {0} - '.format(metric), flag_maxabs=True,
                    fmax_limit=inputs.fmeddev_limit)
                flaggertask = Gainflagger(flaggerinputs)
            
            elif metric == 'rmsdeviant':
                flaggerinputs = GainflaggerInputs(
                    context=inputs.context, output_dir=inputs.output_dir,
                    vis=inputs.vis, intent=inputs.intent, spw=None,
                    refant=None, niter=None, metric=metric,
                    prepend='flag {0} - '.format(metric), flag_maxabs=True,
                    fmax_limit=inputs.frmsdev_limit)
                flaggertask = Gainflagger(flaggerinputs)

            elif metric == 'nrmsdeviant':
                flaggerinputs = GainflaggerInputs(
                    context=inputs.context, output_dir=inputs.output_dir,
                    vis=inputs.vis, intent=inputs.intent, spw=None,
                    refant=None, niter=None, metric=metric,
                    prepend='flag {0} - '.format(metric), flag_maxabs=True,
                    fmax_limit=inputs.fnrmsdev_limit)
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
    

class GainflaggerInputs(vdp.StandardInputs):

    flag_maxabs = vdp.VisDependentProperty(default=True)
    fmax_limit = vdp.VisDependentProperty(default=3.5)

    @vdp.VisDependentProperty
    def intent(self):
        # default to the intent that would be used for bandpass
        # calibration
        bp_inputs = bandpass.PhcorBandpass.Inputs(context=self.context, vis=self.vis, intent=None)
        return bp_inputs.intent

    metric = vdp.VisDependentProperty(default='mediandeviant')
    niter = vdp.VisDependentProperty(default=1)
    prepend = vdp.VisDependentProperty(default='')

    @vdp.VisDependentProperty
    def refant(self):
        # we cannot find the context value without the measurement set
        if not self.ms:
            return None

        # get the reference antenna for this measurement set
        ant = self.ms.reference_antenna
        if isinstance(ant, list):
            ant = ant[0]

        # return the antenna name/id if this is an Antenna domain object
        if isinstance(ant, domain.Antenna):
            return getattr(ant, 'name', ant.id)

        # otherwise return whatever we found. We assume the calling function
        # knows how to handle an object of this type.
        return ant

    @vdp.VisDependentProperty
    def spw(self):
        science_spws = self.ms.get_spectral_windows(with_channels=True, science_windows_only=True)
        return ','.join([str(spw.id) for spw in science_spws])

    def __init__(self, context, output_dir=None, vis=None, intent=None, spw=None, refant=None, niter=None,
                 flag_maxabs=None, fmax_limit=None, metric=None, prepend=None):

        # pipeline inputs
        self.context = context
        # vis must be set first, as other properties may depend on it
        self.vis = vis
        self.output_dir = output_dir

        # data selection arguments
        self.intent = intent
        self.spw = spw
        self.refant = refant

        # flagging parameters
        self.flag_maxabs = flag_maxabs
        self.fmax_limit = fmax_limit
        self.metric = metric
        self.prepend = prepend
        self.niter = niter


class Gainflagger(basetask.StandardTaskTemplate):
    Inputs = GainflaggerInputs

    def prepare(self):
        inputs = self.inputs
        
        # Initialize result and store vis in result
        result = GainflaggerResults()
        result.vis = inputs.vis

        # Construct the task that will read the data.
        datainputs = GainflaggerDataInputs(
            context=inputs.context, output_dir=inputs.output_dir,
            vis=inputs.vis, intent=inputs.intent, spw=inputs.spw,
            refant=inputs.refant)
        datatask = GainflaggerData(datainputs)
        
        # Construct the generator that will create the view of the data
        # that is the basis for flagging.
        viewtask = GainflaggerView(
            context=inputs.context, vis=inputs.vis, intent=inputs.intent,
            spw=inputs.spw, refant=inputs.refant, metric=inputs.metric)

        # Construct the task that will set any flags raised in the
        # underlying data.
        flagsetterinputs = FlagdataSetter.Inputs(
            context=inputs.context, vis=inputs.vis, table=inputs.vis,
            inpfile=[])
        flagsettertask = FlagdataSetter(flagsetterinputs)

        # Define which type of flagger to use.
        flagger = viewflaggers.MatrixFlagger

        # Translate the input flagging parameters to a more compact
        # list of rules.
        rules = flagger.make_flag_rules(
            flag_maxabs=inputs.flag_maxabs,
            fmax_limit=inputs.fmax_limit)

        # Construct the flagger task around the data view task  and the
        # flagger task. Extend any newly found flags by removing selection
        # of "FIELD" and "TIMERANGE". Extend any newly found flags in a spw
        # by flagging same in all spws of corresponding baseband.
        matrixflaggerinputs = flagger.Inputs(
            context=inputs.context, output_dir=inputs.output_dir,
            vis=inputs.vis, datatask=datatask, viewtask=viewtask,
            flagsettertask=flagsettertask, rules=rules, niter=inputs.niter,
            extendfields=['field', 'timerange'], extendbaseband=True,
            prepend=inputs.prepend, iter_datatask=True)
        flaggertask = flagger(matrixflaggerinputs)

        # Execute the flagger task.
        flaggerresult = self._executor.execute(flaggertask)
        
        # Import views, flags, and "measurement set or caltable to flag"
        # into final result
        result.importfrom(flaggerresult)
        
        # Copy flagging summaries to final result
        result.summaries = flaggerresult.summaries
        
        return result

    def analyse(self, result):
        return result


class GainflaggerDataInputs(vdp.StandardInputs):
    
    def __init__(self, context, output_dir=None, vis=None, intent=None, spw=None, refant=None):
        super(GainflaggerDataInputs, self).__init__()

        # pipeline inputs
        self.context = context
        # vis must be set first, as other properties may depend on it
        self.vis = vis
        self.output_dir = output_dir

        # data selection arguments
        self.intent = intent
        self.spw = spw
        self.refant = refant


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
            LOG.warning('No bandpass solution computed for {0}'.format(
                inputs.ms.basename))
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
            LOG.warning('No phase time solution computed for {0}'.format(
                inputs.ms.basename))
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
            LOG.warning('No amplitude time solution computed for %s '.format(
                inputs.ms.basename))
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

    def __init__(self, context, vis=None, intent=None, spw=None, refant=None,
                 metric=None):
        
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
            LOG.info('Computing flagging metrics for caltable {0}'.format(
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
                    'nrmsdeviant' calculates:
                       deviation = ( sm(ant_i) - med_sm_allants ) / sigma_sm_allants
                       where: 
                         sm(ant_i) = sigma(ant_i) / median(ant_i)
                         sigma_sm_allants = 1.4826 * mad( {sm(ant_1), sm(ant_2), ..., sm(ant_nants)} )
                         med_sm_allants = median( {sm(ant_1), sm(ant_2), ..., sm(ant_nants)} )
                    Note: mad = median absolute deviation from the median
        """        
        
        # Create the view, based on the metric
        if metric in ['mediandeviant', 'rmsdeviant', 'nrmsdeviant']:
            self.calculate_median_rms_deviant_view(table, metric)

    def calculate_median_rms_deviant_view(self, gtable, metric='mediandeviant'):
        """
        Method to calculate the "mediandeviant", "rmsdeviant", or "nrmsdeviant"
        flagging view.
        
        Input parameters:
        
        table  -- Name of gain table to be analysed.
        metric -- the name of the view metric to use:
                    'mediandeviant' calculates per antenna:
                       deviation = abs(median(antenna) - median(all antennas)) / mad(all antennas).
                    'rmsdeviant' calculates per antenna:
                       deviation = stdev(antenna) / mad(all antennas).
                    'nrmsdeviant' calculates:
                       deviation = ( sm(ant_i) - med_sm_allants ) / sigma_sm_allants
                       where: 
                         sm(ant_i) = sigma(ant_i) / median(ant_i)
                         sigma_sm_allants = 1.4826 * mad( {sm(ant_1), sm(ant_2), ..., sm(ant_nants)} )
                         med_sm_allants = median( {sm(ant_1), sm(ant_2), ..., sm(ant_nants)} )
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
                                        if antenna.id == antenna_id][0]

        # Get spectral window IDs
        spwids = [spw.id for spw in ms.get_spectral_windows(self.spw)]

        # Get range of times covered
        times = set()
        for row in gtable.rows:
            # The gain table is T, should be no pol dimension
            npols = np.shape(row.get('CPARAM'))[0]
            if npols != 1:
                raise Exception('table has polarization results')
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
                        data_per_ant[ant].append(np.abs(gain))
                        data_all_ant.append(np.abs(gain))
                        # At least one good datapoint was found for this
                        # antenna, so set flag to 0.
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
                    
                    # Calculate metric and store in flagging view
                    data[ant] = np.abs(np.median(data_per_ant[ant])
                                       - np.median(data_all_ant)) / mad_all_ant
            
            elif metric == 'rmsdeviant':
                # Calculate metric for each antenna
                for ant in data_per_ant.keys():
                    
                    # Convert to numpy arrays
                    data_per_ant[ant] = np.array(data_per_ant[ant])
                    
                    # Calculate median absolute deviation from the median
                    # mad_ant = np.median(np.abs(data_per_ant[ant] - np.median(data_per_ant[ant])))
    
                    # Calculate standard deviation of antenna
                    stdev_ant = np.std(data_per_ant[ant])
    
                    # Store "MAD(ant) / MAD(all ant)" in flagging view
                    data[ant] = stdev_ant / mad_all_ant

            # The following normalised RMS deviant metric is based on the
            # recommendation by H. Francke Henriquez documented at 
            # https://bugs.nrao.edu/browse/CAS-8831.
            elif metric == 'nrmsdeviant':
                median_per_ant = defaultdict(list)
                stdev_per_ant = defaultdict(list)
                sm_all_ant = []
                
                # Calculate required statistics for each metric:
                for ant in data_per_ant.keys():
                    
                    # Convert to numpy arrays
                    data_per_ant[ant] = np.array(data_per_ant[ant])
                    
                    # Calculate standard deviation per antenna.
                    stdev_per_ant[ant] = np.std(data_per_ant[ant])
                    
                    # Calculate median per antenna.
                    median_per_ant[ant] = np.median(data_per_ant[ant])

                    # Calculate stdev / median ratio per antenna.
                    sm_all_ant.append(stdev_per_ant[ant] / median_per_ant[ant])

                # Convert to numpy array
                sm_all_ant = np.array(sm_all_ant)

                # Calculate the median of the "sm" distribution.
                median_sm = np.median(sm_all_ant)
                
                # Assuming that "sm" are normally distributed, estimate the 
                # standard deviation as 1.4826 * MAD. 
                sigma_sm = 1.4826 * np.median(np.abs(sm_all_ant - median_sm))

                # Calculate final deviation metric for each antenna.
                for ant in data_per_ant.keys():
                    data[ant] = ((stdev_per_ant[ant]/median_per_ant[ant] - median_sm) / sigma_sm)
            
            # Create axes for view result, set time on y-axis to the first timestamp.
            axes = [
                commonresultobjects.ResultAxis(
                    name='Antenna1', units='id',
                    data=np.arange(antenna_ids[-1]+1)),
                commonresultobjects.ResultAxis(
                    name='Time', units='', data=times[0:1])
            ]

            # Convert flagging view into an ImageResult.
            viewresult = commonresultobjects.ImageResult(
                filename='%s(gtable)' % os.path.basename(gtable.vis),
                intent=self.intent, data=data, flag=flag, axes=axes,
                datatype='gain amplitude', spw=spwid)
          
            # Add the view result to the class result structure.
            self.result.addview(viewresult.description, viewresult)            
