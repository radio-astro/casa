from __future__ import absolute_import

import os

import numpy as np

import pipeline.domain as domain
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
import pipeline.infrastructure.utils as utils
from pipeline.h.tasks.common import calibrationtableaccess as caltableaccess
from pipeline.h.tasks.common import commonresultobjects
from pipeline.h.tasks.common import viewflaggers
from pipeline.h.tasks.flagging.flagdatasetter import FlagdataSetter
from .resultobjects import LowgainflagDataResults
from .resultobjects import LowgainflagResults
from .resultobjects import LowgainflagViewResults
from .. import bandpass
from .. import gaincal

__all__ = [
    'LowgainflagInputsInputs',
    'Lowgainflag'
]

LOG = infrastructure.get_logger(__name__)


class LowgainflagInputs(vdp.StandardInputs):
    """
    LowgainflagInputs defines the inputs for the Lowgainflag pipeline task.
    """
    flag_nmedian = vdp.VisDependentProperty(default=True)
    fnm_hi_limit = vdp.VisDependentProperty(default=1.3)
    fnm_lo_limit = vdp.VisDependentProperty(default=0.7)

    @vdp.VisDependentProperty
    def intent(self):
        # default to the intent that would be used for bandpass
        # calibration
        bp_inputs = bandpass.PhcorBandpass.Inputs(context=self.context, vis=self.vis, intent=None)
        return bp_inputs.intent

    niter = vdp.VisDependentProperty(default=1)

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

    def __init__(self, context, output_dir=None, vis=None, intent=None, spw=None, refant=None, flag_nmedian=None,
                 fnm_lo_limit=None, fnm_hi_limit=None, niter=None):
        super(LowgainflagInputs, self).__init__()

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
        self.flag_nmedian = flag_nmedian
        self.fnm_hi_limit = fnm_hi_limit
        self.fnm_lo_limit = fnm_lo_limit
        self.niter = niter


class Lowgainflag(basetask.StandardTaskTemplate):
    Inputs = LowgainflagInputs

    def prepare(self):
        inputs = self.inputs
        
        # Initialize result and store vis in result
        result = LowgainflagResults(vis=inputs.vis)

        # Construct the task that will read the data.
        datainputs = LowgainflagDataInputs(
            context=inputs.context, output_dir=inputs.output_dir,
            vis=inputs.vis, intent=inputs.intent, spw=inputs.spw,
            refant=inputs.refant)
        datatask = LowgainflagData(datainputs)
        
        # Construct the generator that will create the view of the data
        # that is the basis for flagging.
        viewtask = LowgainflagView(
            context=inputs.context, vis=inputs.vis, intent=inputs.intent,
            spw=inputs.spw, refant=inputs.refant)

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
            flag_nmedian=inputs.flag_nmedian,
            fnm_lo_limit=inputs.fnm_lo_limit,
            fnm_hi_limit=inputs.fnm_hi_limit)

        # Construct the flagger task around the data view task and the
        # flagsetter task. 
        matrixflaggerinputs = flagger.Inputs(
            context=inputs.context, output_dir=inputs.output_dir,
            vis=inputs.vis, datatask=datatask, viewtask=viewtask,
            flagsettertask=flagsettertask, rules=rules, niter=inputs.niter,
            extendfields=['field', 'timerange'], iter_datatask=True)
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
        """
        Analyses the Lowgainflag result:

        Identifies, for each view (spw), which antennas have been flagged, and
        marks these to move to the end of the reference antenna list.
        """
        # Initialize set of antennas-to-demote: antennas that are flagged in
        # any of the spws.
        ants_flagged_in_any_spw = set()

        # Create a summary of the flagging state by going through
        # each view product for the specified metric.
        for description in result.descriptions():

            # Get final view.
            view = result.last(description)

            # Update set of antennas-to-demote with IDs of antennas fully
            # flagged in current view.
            ants_flagged_in_any_spw.update(np.where(np.all(view.flag, axis=1))[0])

        # If any antennas-to-demote were found, then proceed...
        if ants_flagged_in_any_spw:

            # Get the MS object
            ms = self.inputs.context.observing_run.get_ms(name=self.inputs.vis)

            # If a list of reference antennas was registered with the MS..
            if (hasattr(ms, 'reference_antenna') and
                    isinstance(ms.reference_antenna, str)):

                # Create list of current refants
                refant = ms.reference_antenna.split(',')

                # Create translation dictionary, reject empty antenna name strings.
                antenna_id_to_name = {ant.id: ant.name for ant in ms.antennas if ant.name.strip()}

                # Translate IDs of antennas-to-demote to antenna names.
                ants_to_demote_as_refant = {
                    antenna_id_to_name[ant_id]
                    for ant_id in ants_flagged_in_any_spw
                }

                # Compute intersection between refants and ants to demote as
                # refant.
                refants_to_demote = {
                    ant for ant in refant
                    if ant in ants_to_demote_as_refant
                }

                # Create string for log message.
                ant_msg = utils.commafy(refants_to_demote, quotes=False)

                # Check if the list of refants-to-demote comprises all
                # refants, in which case the re-ordering of refants is
                # skipped.
                if refants_to_demote == set(refant):

                    # Log warning that refant list should have been updated, but
                    # will not be updated so as to avoid an empty refant list.
                    LOG.warning(
                        '{} - the following antennas are fully flagged '
                        'for one or more spws, but since these comprise all '
                        'refants, the refant list is *NOT* updated to '
                        're-order these to the end of the refant list: '
                        '{}'.format(ms.basename, ant_msg))
                else:
                    # Log a warning if any antennas are to be demoted from
                    # the refant list.
                    LOG.warning(
                        '{} - the following antennas are moved to the end '
                        'of the refant list because they are fully '
                        'flagged for one or more spws: '
                        '{}'.format(ms.basename, ant_msg))

                    # Update result to set the refants to demote:
                    result.refants_to_demote = refants_to_demote

            # If no list of reference antennas was registered with the MS,
            # raise a warning.
            else:
                LOG.warning(
                    '{} - no reference antennas found in MS, cannot update '
                    'the reference antenna list.'.format(ms.basename))

        return result


class LowgainflagDataInputs(vdp.StandardInputs):
    def __init__(self, context, output_dir=None, vis=None, intent=None,
                 spw=None, refant=None):
        super(LowgainflagDataInputs, self).__init__()

        # pipeline inputs
        self.context = context
        # vis must be set first, as other properties may depend on it
        self.vis = vis
        self.output_dir = output_dir

        # data selection arguments
        self.intent = intent
        self.spw = spw
        self.refant = refant


class LowgainflagData(basetask.StandardTaskTemplate):
    Inputs = LowgainflagDataInputs

    def __init__(self, inputs):
        super(LowgainflagData, self).__init__(inputs)

    def prepare(self):
        inputs = self.inputs

        # Initialize result structure
        result = LowgainflagDataResults()
        result.vis = inputs.vis

        # Calculate a phased-up bpcal
        bpcal_inputs = bandpass.PhcorBandpass.Inputs(
            context=inputs.context, vis=inputs.vis, intent=inputs.intent,
            spw=inputs.spw, refant=inputs.refant, solint='inf,7.8125MHz')
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
            calmode='a', minsnr=2.0, solint='inf', gaintype='T')
        gacal_task = gaincal.GTypeGaincal(gacal_inputs)
        gacal = self._executor.execute(gacal_task, merge=False)
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

 
class LowgainflagView(object):

    def __init__(self, context, vis=None, intent=None, spw=None, refant=None):
        
        self.context = context
        self.vis = vis
        self.intent = intent
        self.spw = spw
        self.refant = refant

    def __call__(self, data):
        
        # Initialize result structure
        self.result = LowgainflagViewResults()

        if data.table_available:

            # Calculate the view
            gatable = data.table
            LOG.info('Computing flagging metrics for caltable {0}'.format(
                os.path.basename(gatable)))
            self.calculate_view(gatable)

        # Add visibility name to result
        self.result.vis = self.vis

        return self.result

    def calculate_view(self, table):
        """
        table -- Name of gain table to be analysed.
        spwid -- view will be calculated using data for this spw id.
        """

        gtable = caltableaccess.CalibrationTableDataFiller.getcal(table)

        ms = self.context.observing_run.get_ms(name=self.vis)
        antenna_ids = [antenna.id for antenna in ms.antennas] 
        antenna_ids.sort()
        antenna_name = {}
        for antenna_id in antenna_ids:
            antenna_name[antenna_id] = [antenna.name for antenna in ms.antennas
                                        if antenna.id == antenna_id][0]

        # spw arg may contain channel specification. Let MS parse input and
        # read spw ID from the SpectralWindow domain objects that are returned
        spwids = [spw.id for spw in ms.get_spectral_windows(self.spw)]

        # get range of times covered
        times = set()
        for row in gtable.rows:
            # The gain table is T, should be no pol dimension
            npols = np.shape(row.get('CPARAM'))[0]
            if npols != 1:
                raise Exception('table has polarization results')
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

            axes = [
                commonresultobjects.ResultAxis(
                    name='Antenna1', units='id',
                    data=np.arange(antenna_ids[-1]+1)),
                commonresultobjects.ResultAxis(
                    name='Time', units='', data=times)
            ]

            # associate the result with a generic filename - using
            # specific names gives confusing duplicates on the weblog
            # display
            viewresult = commonresultobjects.ImageResult(
                filename='%s(gtable)' % os.path.basename(gtable.vis),
                intent=self.intent, data=data, flag=flag, axes=axes,
                datatype='gain amplitude', spw=spwid)
          
            # add the view results and their children results to the
            # class result structure
            self.result.addview(viewresult.description, viewresult)
