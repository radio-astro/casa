from __future__ import absolute_import

import inspect

import numpy as np

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.vdp as vdp
from pipeline.h.tasks.common import commonhelpermethods
from pipeline.h.tasks.common import viewflaggers
from pipeline.hifa.tasks import bandpass
from pipeline.infrastructure import task_registry
from . import wvrgcalflagsetter
from .resultobjects import WvrgcalflagResults, WvrgcalflagViewResults
from ..wvrgcal import wvrgcal

__all__ = [
    'Wvrgcalflag',
    'WvrgcalflagInputs'
]

LOG = infrastructure.get_logger(__name__)


class WvrgcalflagInputs(wvrgcal.WvrgcalInputs):
    """
    WvrgcalflagInputs defines the inputs for the Wvrgcalflag pipeline task.
    """
    ants_with_wvr_nr_thresh = vdp.VisDependentProperty(default=3)
    ants_with_wvr_thresh = vdp.VisDependentProperty(default=0.2)
    flag_hi = vdp.VisDependentProperty(default=True)

    @vdp.VisDependentProperty
    def flag_intent(self):
        bp_inputs = bandpass.ALMAPhcorBandpass.Inputs(context=self.context, vis=self.vis, intent=None)
        return bp_inputs.intent

    fhi_limit = vdp.VisDependentProperty(default=10)
    fhi_minsample = vdp.VisDependentProperty(default=5)

    @vdp.VisDependentProperty
    def qa_intent(self):
        # Ensure that qa_intent includes flag_intent otherwise the results for
        # flag_intent will not be calculated.
        value_set = {'PHASE'}
        value_set.update(self.flag_intent.split(','))
        return ','.join(value_set)

    @qa_intent.convert
    def qa_intent(self, value):
        # Ensure that qa_intent includes flag_intent otherwise the results for
        # flag_intent will not be calculated.
        value_set = set(value.split(','))
        value_set.update(self.flag_intent.split(','))
        return ','.join(value_set)

    def __init__(self, context, output_dir=None, vis=None, caltable=None, offsetstable=None, hm_toffset=None,
                 toffset=None, segsource=None, hm_tie=None, tie=None, sourceflag=None, nsol=None, disperse=None,
                 wvrflag=None, hm_smooth=None, smooth=None, scale=None, maxdistm=None, minnumants=None,
                 mingoodfrac=None, refant=None, flag_intent=None, qa_intent=None, qa_bandpass_intent=None,
                 accept_threshold=None, flag_hi=None, fhi_limit=None, fhi_minsample=None, ants_with_wvr_thresh=None,
                 ants_with_wvr_nr_thresh=None):

        super(WvrgcalflagInputs, self).__init__(
            context, output_dir=output_dir, vis=vis, caltable=caltable, offsetstable=offsetstable,
            hm_toffset=hm_toffset, toffset=toffset, segsource=segsource, hm_tie=hm_tie, tie=tie, sourceflag=sourceflag,
            nsol=nsol, disperse=disperse, wvrflag=wvrflag, hm_smooth=hm_smooth, smooth=smooth, scale=scale,
            maxdistm=maxdistm, minnumants=minnumants, mingoodfrac=mingoodfrac, refant=refant, qa_intent=qa_intent,
            qa_bandpass_intent=qa_bandpass_intent, accept_threshold=accept_threshold)

        # solution parameters
        self.ants_with_wvr_nr_thresh = ants_with_wvr_nr_thresh
        self.ants_with_wvr_thresh = ants_with_wvr_thresh
        self.flag_intent = flag_intent

        # flagging parameters
        self.flag_hi = flag_hi
        self.fhi_limit = fhi_limit
        self.fhi_minsample = fhi_minsample


@task_registry.set_equivalent_casa_task('hifa_wvrgcalflag')
@task_registry.set_casa_commands_comment(
    'Water vapour radiometer corrections are calculated for each antenna. The quality of the correction is assessed by '
    'comparing a phase gain solution calculated with and without the WVR correction. This requires calculation of a '
    'temporary phase gain on the bandpass calibrator, a temporary bandpass using that temporary gain, followed by phase'
    ' gains with the temporary bandpass, with and without the WVR correction. After that, some antennas are wvrflagged '
    '(so that their WVR corrections are interpolated), and then the quality of the correction recalculated.'
)
class Wvrgcalflag(basetask.StandardTaskTemplate):
    Inputs = WvrgcalflagInputs
    
    def prepare(self):
        inputs = self.inputs

        # Identify number and fraction of antennas with WVR.
        nr_ants_wvr, frac_ants_wvr = self._identify_ants_with_wvr()

        # Evaluate whether there are enough antennas with WVR by number and
        # fraction to be worth proceeding with calibration and flagging.
        if (nr_ants_wvr < inputs.ants_with_wvr_nr_thresh or
                frac_ants_wvr < self.inputs.ants_with_wvr_thresh):
            # If there are too few antennas, return an empty result, with
            # flag set to indicate that there were too few WVR antennas.
            flaggerresult = viewflaggers.MatrixFlaggerResults(vis=inputs.vis)
            result = WvrgcalflagResults(vis=inputs.vis,
                                        flaggerresult=flaggerresult,
                                        too_few_wvr=True)
            # Log info message.
            if nr_ants_wvr < inputs.ants_with_wvr_nr_thresh:
                LOG.info("Number of antennas with WVR ({}) below the"
                         " threshold ({}), skipping WVR calibration."
                         "".format(nr_ants_wvr, inputs.ants_with_wvr_nr_thresh))
            elif frac_ants_wvr < self.inputs.ants_with_wvr_thresh:
                LOG.info("Fraction of antennas with WVR ({:.2f}) below the"
                         " threshold of {}, skipping WVR calibration."
                         "".format(frac_ants_wvr, self.inputs.ants_with_wvr_thresh))
        else:
            # If there are enough WVR antennas initially, run calibration
            # and flagging.
            flaggerresult = self._run_flagger()

            # Attach flagging result to final result.
            result = WvrgcalflagResults(vis=inputs.vis,
                                        flaggerresult=flaggerresult)

        return result

    def analyse(self, result):
        inputs = self.inputs

        # If flagging views are available, then evaluate the flagger result to
        # check whether the WVR correction should still be applied.
        if result.flaggerresult.viewresult and result.flaggerresult.viewresult.descriptions():
            # Identify number and fraction of unflagged antennas with WVR.
            nr_ants_wvr, frac_ants_wvr = self._identify_unflagged_ants_with_wvr(result)

            # If too few unflagged antennas with WVR remain (by number or
            # fraction), then remove the calfile from the result so that it
            # cannot be accepted into the context, and set flag in result
            # to indicate too few WVR antennas remained.
            if (nr_ants_wvr < inputs.ants_with_wvr_nr_thresh or
                    frac_ants_wvr < inputs.ants_with_wvr_thresh):
                result.flaggerresult.dataresult.final = []
                result.too_few_wvr_post_flagging = True

                # Log warning.
                if nr_ants_wvr < inputs.ants_with_wvr_nr_thresh:
                    LOG.warning(
                        "After flagging, the number of unflagged antennas"
                        " with WVR ({}) is below the threshold of {}, skipping"
                        " WVR calibration.".format(nr_ants_wvr, inputs.ants_with_wvr_nr_thresh))
                elif frac_ants_wvr < inputs.ants_with_wvr_thresh:
                    LOG.warning(
                        "After flagging, the fraction of unflagged antennas"
                        " with WVR ({:.2f}) is below the threshold of {};"
                        " wvrgcal file will not be applied."
                        "".format(frac_ants_wvr, inputs.ants_with_wvr_thresh))

            # If the associated qa score indicates that applying the WVR
            # calibration will make things worse then remove the calfile from
            # the result so that it cannot be accepted into the context.
            elif result.flaggerresult.dataresult.qa_wvr.overall_score is not None \
                    and result.flaggerresult.dataresult.qa_wvr.overall_score < inputs.accept_threshold:
                LOG.warning('wvrgcal file has qa score ({0}) below'
                            ' accept_threshold ({1}) and will not be'
                            ' applied'.format(result.flaggerresult.dataresult.qa_wvr.overall_score,
                                              inputs.accept_threshold))
                result.flaggerresult.dataresult.final = []

        return result

    def _identify_ants_with_wvr(self):
        """
        Identify antennas with WVR, return as number and as fraction of all
        antennas.
        """
        # Get the MS domain object.
        ms = self.inputs.context.observing_run.get_ms(name=self.inputs.vis)

        # Identify antennas with WVR as those whose name does not start with 'CM'.
        ants_with_wvr = [antenna.name
                         for antenna in ms.antennas
                         if not antenna.name.startswith('CM')]
        nr_ants_wvr = len(ants_with_wvr)
        frac_ants_wvr = nr_ants_wvr / float(len(ms.antennas))

        return nr_ants_wvr, frac_ants_wvr

    def _identify_unflagged_ants_with_wvr(self, result):
        """
        Evaluate flagger result to identify remaining unflagged antennas with
        WVR, return as a number and as a fraction of all antennas.

        :param result: WvrgcalflagResults object
        :return: (int, float): number of antennas, fraction of antennas.
        """
        # Get the MS domain object.
        ms = self.inputs.context.observing_run.get_ms(name=self.inputs.vis)

        # Only expect 1 flagging view for Wvrgcalflag.
        view = result.flaggerresult.last(result.flaggerresult.descriptions()[0])

        # Identify antennas fully flagged for all timestamps, mapping the
        # array indices to the original antenna IDs using the flagging view
        # x-axis data.
        antids_fully_flagged = view.axes[0].data[
            np.where(np.all(view.flag, axis=1))[0]]

        # Identify antennas with WVR that also remain unflagged.
        ants_with_wvr_and_unflagged = [
            antenna.name
            for antenna in ms.antennas
            if not antenna.name.startswith('CM')
            and antenna.id not in antids_fully_flagged
        ]

        nr_ants_wvr = len(ants_with_wvr_and_unflagged)
        frac_ants_wvr = nr_ants_wvr / float(len(ms.antennas))

        return nr_ants_wvr, frac_ants_wvr

    def _run_flagger(self):
        inputs = self.inputs

        # Construct the task that will prepare/read the data necessary to
        # create a flagging view.
        datainputs = WvrgcalflagData.Inputs(
            context=inputs.context, output_dir=inputs.output_dir,
            vis=inputs.vis, caltable=inputs.caltable,
            offsetstable=inputs.offsetstable,
            hm_toffset=inputs.hm_toffset, toffset=inputs.toffset,
            segsource=inputs.segsource, hm_tie=inputs.hm_tie,
            tie=inputs.tie, sourceflag=inputs.sourceflag, nsol=inputs.nsol,
            disperse=inputs.disperse, wvrflag=inputs.wvrflag,
            hm_smooth=inputs.hm_smooth, smooth=inputs.smooth,
            scale=inputs.scale, maxdistm=inputs.maxdistm,
            minnumants=inputs.minnumants, mingoodfrac=inputs.mingoodfrac,
            refant=inputs.refant, qa_intent=inputs.qa_intent,
            qa_bandpass_intent=inputs.qa_bandpass_intent)
        datatask = WvrgcalflagData(datainputs)

        # Construct the generator that will create the views of the data
        # that is the basis for flagging.
        viewtask = WvrgcalflagView(context=inputs.context,
                                   flag_intent=inputs.flag_intent)

        # Construct the "flagsetter" task: Wvrgcalflag has its own dedicated
        # flagsetter that does not really set flags in underlying data, but
        # rather updates the "pre-existing flags" parameter (wvrflag) of the
        # datatask, such that a subsequent run of the datatask takes the newly
        # found flags into account.
        flagsetterinputs = wvrgcalflagsetter.WvrgcalFlagSetterInputs(
            context=inputs.context, table=inputs.vis, vis=inputs.vis,
            datatask=datatask)
        flagsettertask = wvrgcalflagsetter.WvrgcalFlagSetter(flagsetterinputs)

        # Define which type of flagger to use.
        flagger = viewflaggers.MatrixFlagger

        # Translate the input flagging parameters to a more compact
        # list of rules.
        rules = flagger.make_flag_rules(
            flag_hi=inputs.flag_hi,
            fhi_limit=inputs.fhi_limit,
            fhi_minsample=inputs.fhi_minsample)

        # Construct the flagger task around the data, view, and flagsetter
        # tasks. Wvrgcalflag passes its own Results class to be used by the
        # flagger, and the flagger produces the final result.
        flaggerinputs = flagger.Inputs(
            context=inputs.context, output_dir=inputs.output_dir,
            vis=inputs.vis, datatask=datatask, viewtask=viewtask,
            flagsettertask=flagsettertask, rules=rules, niter=1,
            iter_datatask=True)
        flaggertask = flagger(flaggerinputs)

        # Execute the flagger task, and wrap this child task in a
        # SuspendCapturingLogger so that warnings emitted by the child task
        # do not make it to the web log page; see CAS-7795.
        with logging.SuspendCapturingLogger():
            flaggerresult = self._executor.execute(flaggertask)

        return flaggerresult


class WvrgcalflagDataInputs(vdp.StandardInputs):

    def __init__(self, context, output_dir=None, vis=None, caltable=None, offsetstable=None, hm_toffset=None,
                 toffset=None, segsource=None, hm_tie=None, tie=None, sourceflag=None, nsol=None, disperse=None,
                 wvrflag=None, hm_smooth=None, smooth=None, scale=None, maxdistm=None, minnumants=None,
                 mingoodfrac=None, refant=None, qa_intent=None, qa_bandpass_intent=None):

        # pipeline inputs
        self.context = context
        # vis must be set first, as other properties may depend on it
        self.vis = vis
        self.output_dir = output_dir

        # data selection arguments
        self.caltable = caltable
        self.offsetstable = offsetstable

        # solution parameters
        self.hm_toffset = hm_toffset
        self.toffset = toffset
        self.segsource = segsource
        self.hm_tie = hm_tie
        self.tie = tie
        self.sourceflag = sourceflag
        self.nsol = nsol
        self.disperse = disperse
        self.wvrflag = wvrflag
        self.hm_smooth = hm_smooth
        self.smooth = smooth
        self.scale = scale
        self.maxdistm = maxdistm
        self.minnumants = minnumants
        self.mingoodfrac = mingoodfrac
        self.refant = refant
        self.qa_intent = qa_intent
        self.qa_bandpass_intent = qa_bandpass_intent

    # WvrgcalflagDataInputs is a worker task for Wvrgcalflag and does not need
    # to define its own defaults for input parameters. However, as a result, its
    # input parameters will not show up in the "inputs as a dictionary" object.
    # The weblog renderer for Wvrgcalflag creates plots based on the result
    # from WvrgcalflagData, and the renderer needs to access attributes from
    # its inputs (expecting a dictionary).
    # To ensure all parameters from WvrgcalflagData show up in its final inputs
    # as a dictionary, we override "as_dict" here.

    # TODO: could this override be removed after initializing all parameters
    # TODO: as vdp parameters with default = None?
    def as_dict(self):
        skip = ['context', 'ms']
        return {dd_name: dd
                for dd_name, dd in inspect.getmembers(self, lambda a: not(inspect.isroutine(a)))
                if not (dd_name.startswith('_') or dd_name in skip)}


class WvrgcalflagData(basetask.StandardTaskTemplate):
    Inputs = WvrgcalflagDataInputs

    def __init__(self, inputs):
        super(WvrgcalflagData, self).__init__(inputs)

        # Initialize parameters that should persist across multiple flagging
        # iterations. These will be populated by the output from Wvrgcal on
        # the first iteration, and are then passed on to Wvrgcal on subsequent
        # iterations.
        self.bandpass_result = None
        self.nowvr_result = None
        self.qa_spw = ''

    def prepare(self):
        inputs = self.inputs

        # Calculate the wvrgcal:
        #  * set accept_threshold to zero to ensure that the Wvrgcal result
        #    is always accepted into the local copy of the context, to force
        #    trigger the QA scoring determination.
        #  * pass along locally cached values for "bandpass_result",
        #    "nowvr_result": if these are not empty, then they were
        #    populated by a previous flagging iteration, and Wvrgcal will
        #    avoid running an exact repeat of the same task
        #  * pass along locally cached value for "qa_spw" to ensure that
        #    Wvrgcal will use the same spw list on subsequent iterations.
        wvrgcalinputs = wvrgcal.Wvrgcal.Inputs(
            context=inputs.context, output_dir=inputs.output_dir,
            vis=inputs.vis, offsetstable=inputs.offsetstable,
            hm_toffset=inputs.hm_toffset, toffset=inputs.toffset,
            segsource=inputs.segsource, hm_tie=inputs.hm_tie, tie=inputs.tie,
            sourceflag=inputs.sourceflag, nsol=inputs.nsol,
            disperse=inputs.disperse, wvrflag=inputs.wvrflag,
            hm_smooth=inputs.hm_smooth, smooth=inputs.smooth,
            scale=inputs.scale, maxdistm=inputs.maxdistm,
            minnumants=inputs.minnumants, mingoodfrac=inputs.mingoodfrac,
            refant=inputs.refant, qa_intent=inputs.qa_intent,
            qa_bandpass_intent=inputs.qa_bandpass_intent,
            accept_threshold=0.0, bandpass_result=self.bandpass_result,
            nowvr_result=self.nowvr_result, qa_spw=self.qa_spw)
        wvrgcaltask = wvrgcal.Wvrgcal(wvrgcalinputs)
        result = self._executor.execute(wvrgcaltask, merge=True)

        # Cache bandpass result, nowvr result, and qa_spw for next call to
        # wvrgcal.
        self.bandpass_result = result.qa_wvr.bandpass_result
        self.nowvr_result = result.qa_wvr.nowvr_result
        self.qa_spw = result.qa_wvr.qa_spw

        # Add a top-level reference to the "No WVR" result, expected by WVR
        # renderer.
        result.nowvr_result = result.qa_wvr.nowvr_result

        return result

    def analyse(self, result):
        return result


class WvrgcalflagView(object):
    def __init__(self, context, flag_intent=None):
        """
        Creates an WvrgcalflagView instance.
        """
        self.context = context
        self.flag_intent = flag_intent

    def __call__(self, dataresult):
        """
        When called, the WvrgcalflagView object calculates flagging views
        based on the provided dataresult (from Wvrgcal). For the views matching
        the intent specified by "inputs.flag_intent", it retrieves the view,
        updates the flags in this view to reflect the flagging found by
        Wvrgcal, and then stores the resulting view in WvrgcalflagViewResults.

        dataresult  -- WvrgcalResult object.

        Returns:
        WvrgcalflagViewResults object containing the flagging view.
        """

        # Initialize result structure.
        result = WvrgcalflagViewResults(vis=dataresult.vis)

        # As part of the data task, Wvrgcal will have created flagging views,
        # Wvrgcalflag will start from a copy of the views stored in dataresult.
        LOG.info('Retrieving flagging views for vis {0}'.format(
            dataresult.vis))

        # Get a list of available intents from the MS.
        ms = self.context.observing_run.get_ms(name=dataresult.vis)
        ms_intent_list = ms.intents

        # Test if the flagging intents are present in the MS.
        intent_available = []
        flag_intent_list = self.flag_intent.split(',')
        for flag_intent in flag_intent_list:
            intent_available = [intent for intent in ms_intent_list if flag_intent in intent]
            if intent_available:
                LOG.info('flagging views will use %s data' % flag_intent)
            else:
                LOG.warning('no data for intent %s' % flag_intent)
        if not intent_available:
            LOG.warning('no data fits flag_intent {0}, no flagging will be'
                        ' done'.format(self.flag_intent))

        # From the QA section of the dataresult (from Wvrgcal) copy those views
        # for which the intent matches the flag_intent.
        for description in dataresult.qa_wvr.descriptions():
            for flag_intent in flag_intent_list:
                if 'Intent:%s' % flag_intent in description:
                    # Translate list of WVR flagged antenna names back to list
                    # of antenna IDs.
                    ant_names, ant_ids = commonhelpermethods.get_antenna_names(ms)
                    wvrflagids = []
                    for ant_name in dataresult.wvrflag:
                        ant_id = [idx for idx in ant_names if
                                  ant_names[idx] == ant_name]
                        wvrflagids += ant_id

                    # Set antennas specified by the wvrflag parameter in the
                    # flagging image to show that these data are already 'flagged'
                    # (i.e. interpolated).
                    image = dataresult.qa_wvr.last(description)
                    image.setflags(axisname='Antenna', indices=wvrflagids)

                    # Add updated view to result.
                    result.addview(description, image)

        return result
