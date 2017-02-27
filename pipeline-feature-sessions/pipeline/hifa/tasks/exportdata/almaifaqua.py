from __future__ import absolute_import

import os
import xml.etree.cElementTree as eltree

import pipeline
import pipeline.domain.measures as measures
import pipeline.infrastructure.logging as logging

LOG = logging.get_logger(__name__)

import pipeline.h.tasks.exportdata.aqua as aqua

'''
Prototype pipeline ALMA Interferometry pipeline AQUA report generator


Definitions
    Metrics are physical quantities, e.g. phase rms improvement resulting from
    WVR calibration., % data flagged, etc

    Scores are numbers between 0.0 and 1.0 derived from metrics.  Not all
    metrics currently derived in the pipeline are scored.

Structure
    The report contains
        A project structure section.
        A QA summary section.
        A per stage QA section.
        A per topic QA section.
 
Issues with the Original Schema / Current Pipeline Design
    The per ASDM dimension was ignored.
    
    The multiple metrics / scores per stage and / or per ASDM
    dimension was ignored.

    For stages with single scores / metrics and multiple ASDMs the
    current report generator selects the MS with the worst metric and
    reports that value. This metric by definition corresponds to
    the lowest score.

    Stages which generate multiple scores / metrics and multiple
    ASDMs are currently dealt with on an ad hoc basis.

    Metrics may have units information. If so they may be encoded
    as CASA quanta in string format

Future Technical Solutions
    Add a toAqua method to the base results class which returns a
    list of metrics for export. Pass these to the QA classes
    for scoring.

    Or add a register to AQUA method to the QA handlers ?
'''

def aquaReportFromFile (contextFile, aquaFile):

    '''
    Create AQUA report from a context file on disk.
    '''

    # Restore context from file
    context = pipeline.Pipeline (context=contextFile).context 
    LOG.info ("Opening context file: %s" % (contextFile))

    # Produce the AQUA report
    aquaReportFromContext (context, aquaFile)

def aquaTestReportFromLocalFile (contextFile, aquaFile):
    '''
    Test AQUA report generation.
    The pipeline context file and web log directory must be in the same local directry
    '''

    context = pipeline.Pipeline (context=contextFile,
        path_overrides={'name':os.path.splitext(contextFile)[0], 'output_dir':os.getcwd()}).context 
    LOG.info ("Opening context file: %s for test" % (contextFile))

    # Produce the AQUA report
    aquaReportFromContext (context, aquaFile)

def aquaReportFromContext (context, aquaFile):

    '''
    Create AQUA report from a context object.
    '''

    LOG.info ("Recipe name: %s" % ("Unknown"))
    LOG.info ("    Number of stages: %d" % (context.task_counter))

    # Initialize
    aquaReport  = AlmaIfAquaReport (context)

    # Construct the project structure element
    aquaReport.set_project_structure()

    # Construct the QA summary element
    aquaReport.set_qa_summary()

    # Construct the per pipeline stage elements
    aquaReport.set_per_stage_qa()

    # Construct the topics elements.
    # TBD
    aquaReport.set_topics_qa()

    LOG.info ("Writing aqua report file: %s" % (aquaFile))
    aquaReport.write(aquaFile)


class AlmaIfAquaReport(aqua.AquaReport):
    """
    Class for creating the AQUA pipeline report
    """

    def __init__(self, context):

        '''
        Create the AQUA document
        Attributes
               context The context
             stagedict The stage dictionary
             fluxstage The stage holding the flux results
            aquareport The aquareport
        '''

        # Initialize super class
        super(AlmaIfAquaReport, self).__init__(context)

        # Add the flux stage information which is specific
        # to the ALMA interferometry pipeline
        self.fluxstage = None
        for i in range(len(self.context.results)):
            (stage_name, stage_score) = self.stagedict[i+1]
            if stage_name == 'hifa_gfluxscale':
                self.fluxstage = i
                break

    def set_project_structure (self):

        '''
        Add the project structure element

        Given the current data flow it is unclear how the report
        generator will acquire the entity id of the original
        processing request

        The processing procedure name is known but not yet
        passed to the pipeline processing request.
        '''

        ps = eltree.SubElement(self.aquareport, "ProjectStructure")
        eltree.SubElement (ps, "ProposalCode").text = \
            self.context.project_summary.proposal_code
        eltree.SubElement (ps, "OusEntityId").text = \
            self.context.project_structure.ous_entity_id
        eltree.SubElement (ps, "OusPartId").text = \
            self.context.project_structure.ous_part_id
        eltree.SubElement (ps, "OusStatusEntityId").text = \
            self.context.project_structure.ousstatus_entity_id
        eltree.SubElement (ps, "ProcessingRequestEntityId").text = \
            "Undefined"
        eltree.SubElement (ps, "ProcessingProcedure").text = \
            "Undefined"

        return ps
        
    def set_per_stage_qa (self):

        '''
        Add the per stage elements
            Stage number, name, and score are element attributes

            Eventually we will need:
               A scores class than includes name and metric attributes
               A toAqua method on the task results class to pass pipeline
               metrics back to the AQUA report generator

        '''

        # Get the stage summary element.
        ppqa = eltree.SubElement(self.aquareport, "QaPerStage")

        # Loop over the stages.
        #    Stage must for now be identified by name.

        for stage in self.stagedict: 

            # Create the generic stage element
            st = eltree.SubElement(ppqa, "Stage", Number=str(stage),
                Name=self.stagedict[stage][0], Score=self.stagedict[stage][1])

            # Populate the stage elements.
            #    This must be done in an ad hoc manner for now

            # Data import
            #    Recompute the missing intents metric which for this case is
            #    identical to the score.
            #    This stage actually implements multiple scores. Most of these
            #    are not appropriate for AQUA.

            if self.stagedict[stage][0] == 'hifa_importdata':
                try:
                    self.add_missing_intents_metric(st,
                        self.context.results[stage-1])
                except:
                    LOG.warn ("Error handling hifa_importdata result")

            # Deterministic flagging
            #     Retrieve the result with the highest online and shadow
            #     flagging metric.
            #     TBD: Modify metric in QA class to include BDF flags.

            elif self.stagedict[stage][0] == 'hifa_flagdata':
                try:
                    self.add_online_shadow_template_flagging_metric(st,
                        self.context.results[stage-1])
                except:
                    LOG.warn ("Error handling hifa_flagdata result")

            # Flux calibrator flagging
            #    Retrieve the result with the highest percentage of new
            #    flags.
            #    TBD: The current metric needs work. It should measure the
            #    percentage of new flux calibrator flags, not the percentage
            #    of new flags for the whole data set.

            elif self.stagedict[stage][0] == 'hifa_fluxcalflag':
                try:
                    self.add_fluxcal_flagging_metric(st,
                        self.context.results[stage-1])
                except:
                    LOG.warn ("Error handling hifa_fluxcalflag result")

            # Raw data bad channel flagging
            #    Retrieve the results with the highest percentage of new
            #    flags.

            elif self.stagedict[stage][0] == 'hif_rawflagchans':
                try:
                    self.add_rawflagchans_flagging_metric(st,
                        self.context.results[stage-1])
                except:
                    LOG.warn ("Error handling hif_rawflagchans result")

            # Tsys flagging
            #     Retrieve the result with the highest percentage of new
            #     flags.

            elif self.stagedict[stage][0] == 'hifa_tsysflag':
                try:
                    self.add_tsys_flagging_metric(st,
                        self.context.results[stage-1])
                except:
                    LOG.warn ("Error handling hifa_tsysflag result")

            # WVR calibration and flagging
            #     Retrieve the result with the poorest rms improvement.

            elif self.stagedict[stage][0] == 'hifa_wvrgcalflag':
                try:
                    self.add_phase_rms_ratio_metric(st,
                        self.context.results[stage-1])
                except:
                    LOG.warn ("Error handling hifa_wvrgcalflag result")

            # Deviant (high / low) gain flagging
            #     Retrieve the result with the highest percentage of new
            #     flags

            elif self.stagedict[stage][0] == 'hif_lowgainflag':
                try:
                    self.add_highlow_gain_flagging_metric(st,
                        self.context.results[stage-1])
                except:
                    LOG.warn ("Error handling hif_lowgainflag result")

            # Deviant gain flagging
            #     Retrieve the result with the highest percentage of new
            #     flags

            elif self.stagedict[stage][0] == 'hif_gainflag':
                try:
                    self.add_gain_flagging_metric(st,
                        self.context.results[stage-1])
                except:
                    LOG.warn ("Error handling hif_gainflag result")

            # Applycal flagging
            #     Retrieve the result with the highest percentage of
            #     newly flagged data

            elif self.stagedict[stage][0] == 'hif_applycal':
                try:
                    self.add_applycal_flagging_metric(st,
                        self.context.results[stage-1])
                except:
                    LOG.warn ("Error handling hif_applycal result")

            # Generic empty result for stages with no supported metrics
            # Currently these include the calibration and imaging tasks
            else:
                pass
                
        return ppqa

    def add_calibration_topic (self, topicqa):

        # Check if there are any calibration results
        #    Return if not. Registry seems linked to web log. How to detect this ?
        # TBD

        # Set the calibration topics element element
        calqa = eltree.SubElement(topicqa, "Calibration", Score="Undefined")

        # Create the flux summary record
        # Test for the existence of a flux scaling stage
        if not self.fluxstage:
            return calqa
        self.add_fluxes_summary (calqa)

        return calqa

    def add_fluxes_summary (self, calqa):

        # This is the global flux score for the flux scaling stage
        fluxscore = self.stagedict[self.fluxstage+1][1]
        fluxqa = eltree.SubElement(calqa, "FluxMeasurements", Score=fluxscore)

        # Locate the flux scaling results in the stage dict
        flux_results = self.context.results[self.fluxstage].read()

        # Loop over the flux results registering
        # Results are always iterable remove else branch
        for fr in flux_results:
            ms_for_result = self.context.observing_run.get_ms(fr.vis)
            vis = os.path.basename(fr.vis)
            for field_arg, measurements in fr.measurements.items():
                fieldname = ms_for_result.get_fields(field_arg)[0].name
                if fieldname.startswith('"') and fieldname.endswith('"'):
                    fieldname = fieldname[1:-1]
                for measurement in sorted(measurements, key=lambda m: int(m.spw_id)):
                    spw = ms_for_result.get_spectral_window(measurement.spw_id)
                    frequency = '%0.6f' % (spw.centre_frequency.to_units(measures.FrequencyUnits.GIGAHERTZ))
                    # I only for now ...
                    for stokes in ['I']:
                        try:
                            flux = getattr(measurement, stokes)
                            flux_jy = flux.to_units(measures.FluxDensityUnits.JANSKY)
                            flux_jy = '%0.3f'% float(flux_jy)

                            unc = getattr(measurement.uncertainty, stokes)
                            unc_jy = unc.to_units(measures.FluxDensityUnits.JANSKY)
                            if unc_jy != 0:
                                unc_jy = '%0.6f'% float(unc_jy)
                            else:
                                unc_jy = ''

                            fvtp = eltree.SubElement(fluxqa, "FluxMeasurement", Name="FluxMeasurement", FluxJy=flux_jy, ErrorJy=unc_jy,
                                Asdm=vis, Field=fieldname, FrequencyGHz=frequency)
                        except:
                            pass
        return fluxqa
    
    def add_missing_intents_metric (self, stage_element, importdata_result):

        '''
        Recompute the missing intents metric for ALMA interferomery data.
        Score map was lifted from scoring code.

        Results are probably always iterable but support a non-iterable
        option just in case.
        '''

        # Retrieve the importdata summaries
        results = importdata_result.read()

        vis = 'Undefined'
        metric_value = 'Undefined'
        metric_name = 'Undefined'

        if not results:
            eltree.SubElement(stage_element, "Metric", Name=metric_name,
                Value=metric_value, Asdm=vis)
            return

        # Construct a list of score values of the appropriate type
        scores = []; scorevalues = []
        for r in results:
            for qascore in r.qa.pool:
                if qascore.origin.metric_name != 'MissingIntentsMark':
                    continue
                scores.append(qascore)
                scorevalues.append(qascore.score)

        # Locate the largest metric
        qavalue, idx = min ((qavalue, idx) for (idx, qavalue) in enumerate(scorevalues))

        if idx is not None:
            vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
            if qavalue is not None:
                metric_name = scores[idx].origin.metric_name
                metric_value = scores[idx].origin.metric_score
                if metric_value != 'N/A':
                    metric_value = '%0.3f' % (metric_value)
                else:
                    metric_value = 'Undefined'

        eltree.SubElement(stage_element, "Metric", Name=metric_name,
            Value=metric_value, Asdm=vis)


    def add_online_shadow_template_flagging_metric (self, stage_element,
        flagdata_result):

        '''
        Results are probably always iterable but support a non-iterable
        option just in case.
        '''

        # Retrieve the flagging results
        results = flagdata_result.read()

        vis = 'Undefined'
        metric_value = 'Undefined'
        metric_name = 'Undefined'

        if not results:
            eltree.SubElement(stage_element, "Metric", Name=metric_name,
                Value=metric_value, Asdm=vis)
            return

        # Construct a list of score values of the appropriate type
        scores = []; scorevalues = []
        for r in results:
            for qascore in r.qa.pool:
                if qascore.origin.metric_name != '%OnlineShadowTemplateFlags':
                    continue
                scores.append(qascore)
                scorevalues.append(qascore.score)

        # Find the highest valued metric.
        qavalue, idx = min ((qavalue, idx) for (idx, qavalue) in enumerate(scorevalues)) 
        if idx is not None:
            vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
            if qavalue is not None:
                metric_name = scores[idx].origin.metric_name
                metric_value = scores[idx].origin.metric_score
                if metric_value != 'N/A':
                    metric_value = '%0.3f' % (100.0 * metric_value)
                else:
                    metric_value = 'Undefined'

        eltree.SubElement(stage_element, "Metric", Name=metric_name,
            Value=metric_value, Asdm=vis)


    def add_tsys_flagging_metric (self, stage_element, tsysflag_result):

        '''
        Results are probably always iterable but support a non-iterable
        option just in case.
        '''

        # Retrieve the flagging summaries
        results = tsysflag_result.read()

        vis = 'Undefined'
        metric_value = 'Undefined'
        metric_name = 'Undefined'

        if not results:
            eltree.SubElement(stage_element, "Metric", Name=metric_name,
                Value=metric_value, Asdm=vis)
            return

        # Construct a list of score values of the appropriate type
        scores = []; scorevalues = []
        for r in results:
            for qascore in r.qa.pool:
                if qascore.origin.metric_name != '%TsysCaltableFlags':
                    continue
                scores.append(qascore)
                scorevalues.append(qascore.score)

        # Locate the largest metric
        qavalue, idx = min ((qavalue, idx) for (idx, qavalue) in enumerate(scorevalues))

        if idx is not None:
            vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
            if qavalue is not None:
                metric_name = scores[idx].origin.metric_name
                metric_value = scores[idx].origin.metric_score
                if metric_value != 'N/A':
                    metric_value = '%0.3f' % (100.0 * metric_value)
                else:
                    metric_value = 'Undefined'

        eltree.SubElement(stage_element, "Metric", Name=metric_name,
            Value=metric_value, Asdm=vis)


    def add_fluxcal_flagging_metric (self, stage_element, fluxcalflag_result):

        '''
        Results are probably always iterable but support a non-iterable
        option just in case.

        Note that this tasks implements 2 score, a percentage flagging score, and a reference
        spw map warning score.
        '''

        # Retrieve the flagging summaries
        results = fluxcalflag_result.read()

        vis = 'Undefined'
        metric_value = 'Undefined'
        metric_name = 'Undefined'

        if not results:
            eltree.SubElement(stage_element, "Metric", Name=metric_name,
                Value=metric_value, Asdm=vis)
            return

        # Construct a list of score values of the appropriate type
        scores = []; scorevalues = []
        for r in results:
            for qascore in r.qa.pool:
                if qascore.origin.metric_name != '%FluxcalFlags':
                    continue
                scores.append(qascore)
                scorevalues.append(qascore.score)

        # Locate the largest metric
        qavalue, idx = min ((qavalue, idx) for (idx, qavalue) in enumerate(scorevalues)) 

        if idx is not None:
            vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
            if qavalue is not None:
                metric_name = scores[idx].origin.metric_name
                metric_value = scores[idx].origin.metric_score
                if metric_value != 'N/A':
                    metric_value = '%0.3f' % (100.0 * metric_value)
                else:
                    metric_value = 'Undefined'

        eltree.SubElement(stage_element, "Metric", Name=metric_name,
            Value=metric_value, Asdm=vis)

    def add_rawflagchans_flagging_metric (self, stage_element, rawflagchans_result):

        '''
        Results are probably always iterable but support a non-iterable
        option just in case.
        '''

        # Retrieve the flagging summaries
        results = rawflagchans_result.read()


        vis = 'Undefined'
        metric_value = 'Undefined'
        metric_name = 'Undefined'

        if not results:
            eltree.SubElement(stage_element, "Metric", Name=metric_name,
                Value=metric_value, Asdm=vis)
            return

        # Construct a list of score values of the appropriate type
        scores = []; scorevalues = []
        for r in results:
            for qascore in r.qa.pool:
                if qascore.origin.metric_name != '%RawBadchansFlags':
                    continue
                scores.append(qascore)
                scorevalues.append(qascore.score)

        # Locate the largest metric
        qavalue, idx = min ((qavalue, idx) for (idx, qavalue) in enumerate(scorevalues))

        if idx is not None:
            vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
            if qavalue is not None:
                metric_name = scores[idx].origin.metric_name
                metric_value = scores[idx].origin.metric_score
                if metric_value != 'N/A':
                    metric_value = '%0.3f' % (100.0 * metric_value)
                else:
                    metric_value = 'Undefined'

        eltree.SubElement(stage_element, "Metric", Name=metric_name,
            Value=metric_value, Asdm=vis)


    def add_phase_rms_ratio_metric (self, stage_element, wvr_result):

        '''
        Retrieve the ratio of phase rms (without wvr) / phase rms (with wvr)
        metric.  

        Results are probably always iterable but support a non-iterable
        option just in case.
        '''

        # Retrieve the results.
        results = wvr_result.read()

        # Initialize
        vis = 'Undefined'
        metric_value = 'Undefined'
        metric_name = 'Undefined'

        if not results:
            eltree.SubElement(stage_element, "Metric", Name=metric_name,
                Value=metric_value, Asdm=vis)
            return

        # Construct a list of score values of the appropriate type
        scores = []; scorevalues = []
        for r in results:
            for qascore in r.qa.pool:
                if qascore.origin.metric_name != 'PhaseRmsRatio':
                    continue
                scores.append(qascore)
                scorevalues.append(qascore.score)

        # Evaluate the metrics
        qavalue, idx = min ((qavalue, idx) for (idx, qavalue) in enumerate(scorevalues)) 
        if idx is not None:
            vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
            if qavalue is not None:
                metric_name = scores[idx].origin.metric_name
                metric_value = scores[idx].origin.metric_score
                if metric_value != 'N/A':
                    metric_value = '%0.3f' % metric_value
                else:
                    metric_value = 'Undefined'

        eltree.SubElement(stage_element, "Metric", Name=metric_name,
            Value=metric_value, Asdm=vis)


    def add_highlow_gain_flagging_metric (self, stage_element,
        gainflag_result):

        '''
        Results are probably always iterable but support a non-iterable
        option just in case.
        '''

        # Retrieve the results.
        results = gainflag_result.read()

        vis = 'Undefined'
        metric_value = 'Undefined'
        metric_name = 'Undefined'

        if not results:
            eltree.SubElement(stage_element, "Metric", Name=metric_name,
                Value=metric_value, Asdm=vis)
            return

        # Construct a list of score values of the appropriate type
        scores = []; scorevalues = []
        for r in results:
            for qascore in r.qa.pool:
                if qascore.origin.metric_name != '%HighLowGainFlags':
                    continue
                scores.append(qascore)
                scorevalues.append(qascore.score)

        # Locate the largest metric
        qavalue, idx = min ((qavalue, idx) for (idx, qavalue) in enumerate(scorevalues))

        if idx is not None:
            vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
            if qavalue is not None:
                metric_name = scores[idx].origin.metric_name
                metric_value = scores[idx].origin.metric_score
                if metric_value != 'N/A':
                    metric_value = '%0.3f' % (100.0 * metric_value)
                else:
                    metric_value = 'Undefined'

        eltree.SubElement(stage_element, "Metric", Name=metric_name,
            Value=metric_value, Asdm=vis)

    def add_gain_flagging_metric (self, stage_element,
        gainflag_result):

        '''
        Results are probably always iterable but support a non-iterable
        option just in case.
        '''

        # Retrieve the results.
        results = gainflag_result.read()

        vis = 'Undefined'
        metric_value = 'Undefined'
        metric_name = 'Undefined'

        if not results:
            eltree.SubElement(stage_element, "Metric", Name=metric_name,
                Value=metric_value, Asdm=vis)
            return

        # Construct a list of score values of the appropriate type
        scores = []; scorevalues = []
        for r in results:
            for qascore in r.qa.pool:
                if qascore.origin.metric_name != '%GainFlags':
                    continue
                scores.append(qascore)
                scorevalues.append(qascore.score)

        # Locate the largest metric
        qavalue, idx = min ((qavalue, idx) for (idx, qavalue) in enumerate(scorevalues))

        if idx is not None:
            vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
            if qavalue is not None:
                metric_name = scores[idx].origin.metric_name
                metric_value = scores[idx].origin.metric_score
                if metric_value != 'N/A':
                    metric_value = '%0.3f' % (100.0 * metric_value)
                else:
                    metric_value = 'Undefined'

        eltree.SubElement(stage_element, "Metric", Name=metric_name,
            Value=metric_value, Asdm=vis)


    def add_applycal_flagging_metric (self, stage_element,
        applycal_result):

        '''
        Recompute the metric defining the percentage of newly flagged data.

        This method is amost identical to the deterministic flagging
        task metric.
        '''

        # Retrieve the results
        results = applycal_result.read()

        vis = 'Undefined'
        metric_value = 'Undefined'
        metric_name = 'Undefined'

        if not results:
            eltree.SubElement(stage_element, "Metric", Name=metric_name,
                Value=metric_value, Asdm=vis)
            return

        # Construct a list of score values of the appropriate type
        scores = []; scorevalues = []
        for r in results:
            for qascore in r.qa.pool:
                if qascore.origin.metric_name != '%ApplycalFlags':
                    continue
                scores.append(qascore)
                scorevalues.append(qascore.score)

        # Locate the largest metric
        qavalue, idx = min ((qavalue, idx) for (idx, qavalue) in enumerate(scorevalues))

        if idx is not None:
            vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
            if qavalue is not None:
                metric_name = scores[idx].origin.metric_name
                metric_value = scores[idx].origin.metric_score
                if metric_value != 'N/A':
                    metric_value = '%0.3f' % (100.0 * metric_value)
                else:
                    metric_value = 'Undefined'

        eltree.SubElement(stage_element, "Metric", Name=metric_name,
            Value=metric_value, Asdm=vis)

