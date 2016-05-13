from __future__ import absolute_import

import os
import datetime
import xml.etree.cElementTree as eltree
from xml.dom import minidom
import collections
import operator

import casadef
import pipeline
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as calcmetrics

LOG = logging.get_logger(__name__)

'''
Prototype pipeline AQUA report generator


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

    The scores are stored with the stage results, but the metrics
    are not. For now they must be recomputed.

    Metrics have no units information. They are not CASA quanta.

Future Technical Solutions
    Add a toAqua method to the base results class which returns a
    list of metrics for export. Pass these to the QA classes
    for scoring. Or add the toAqua method to the QA classes ?
    Discuss with pipeline

    Extend or modify the score objects to include a name or
    identifier, the value of the metric used to generate the
    score, and possibly a unit.

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
    aquaReport  = AquaReport (context)

    # Construct the project structure element
    aquaReport.set_project_structure()

    # Construct the QA summary element
    aquaReport.set_qa_summary()

    # Construct the per pipeline stage elements
    aquaReport.set_per_stage_qa()

    # Construct the topics elements.
    # TBD

    LOG.info ("Writing aqua report file: %s" % (aquaFile))
    aquaReport.write(aquaFile)


class AquaReport(object):
    """
    Class for creating the AQUA pipeline report
    """

    def __init__(self, context):

        '''
        Create the AQUA document
        '''

        self.context = context

        # Construct the stage dictionary
        #   This is a bit inefficient as it requires  reading all
        #   the results to reconstruct the list of stages, but
        #   it does not take very long

        self.stagedict = collections.OrderedDict()
        for i in range(len(context.results)):
            stage_name, stage_score =  \
                get_pipeline_stage_and_score (context.results[i])
            if not stage_score:
                stage_score = 'Undefined'
            else:
                stage_score = '%0.3f' % stage_score
            self.stagedict[i+1] = (stage_name, stage_score)

        # Create the top level AQUA report element
        self.aquareport = eltree.Element("PipelineAquaReport")

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
        
    def set_qa_summary (self):

        '''
        Add the QA summary element

        The final pipeline score is not yet available
        '''

        ps = eltree.SubElement(self.aquareport, "QaSummary")

        # Generate the report date
        eltree.SubElement (ps, "ReportDate").text = \
            datetime.datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S')

        # Processing time
        exec_start = self.context.results[0].read().timestamps.start
        exec_end = self.context.results[-1].read().timestamps.end
        # remove unnecessary precision for execution duration
        dt = exec_end - exec_start
        exec_duration = datetime.timedelta(days=dt.days, seconds=dt.seconds)
        eltree.SubElement (ps, "ProcessingTime").text = str(exec_duration)

        # Software versions
        eltree.SubElement (ps, "CasaVersion").text = casadef.casa_version
        eltree.SubElement (ps, "PipelineVersion").text = pipeline.revision

        # Score for the complete pipeline run
        eltree.SubElement (ps, "FinalScore").text = "Undefined"

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
                    self.add_online_shadow_flagging_metric(st,
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

    def add_missing_intents_metric (self, stage_element, importdata_result):

        '''
        Recompute the missing intents metric for ALMA interferomery data.
        Score map was lifted from scoring code.

        Results are probably always iterable but support a non-iterable
        option just in case.
        '''

        # Score map for interferometry
        score_map = {'PHASE'     : -1.0,
                     'BANDPASS'  : -0.1,
                     'AMPLITUDE' : -0.1}
        required = set(score_map.keys())

        # Retrieve the importdata summaries
        results = importdata_result.read()

        # Loop over results.
        if isinstance (results, collections.Iterable):
            # Construct list pf metrics.
            mlist = []
            for r in results:
                # There is a list of MS objects in each result,
                # but there seems to be only one MS per result.
                for ms in r.mses:
                    # An intent is missing
                    if not required.issubset(ms.intents):
                        missing = required.difference(ms.intents)
                        mvalue = 1.0
                        for m in missing:
                            mvalue += score_map[m]
                        mlist.append(mvalue)
                    # All intents present
                    else:
                        mlist.append(1.0)

            # Determine the smallest metric
            metric, idx = min ((metric, idx) for (idx, metric) in enumerate(mlist)) 
            if idx is None:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
                if metric is not None:
                    metric = '%0.3f' % (metric)
        else:
            # By definition this is the result with the lowest metric
            if not results:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results.inputs['vis']))[0]
                for ms in results.mses:
                    if not required.issubset(ms.intents):
                        missing = required.difference(ms.intents)
                        metric = 1.0
                        for m in missing:
                            metric += score_map[m]
                    else:
                        metric = 1.0
                    metric = '%0.3f' % (metric)

        eltree.SubElement(stage_element, "Metric", Name="MissingIntentsMark",
            Value=metric, Asdm=vis)


    def add_online_shadow_flagging_metric (self, stage_element,
        flagdata_result):

        '''
        Recompute the metric defining the percentage of online and shadow
        flagged data.

        Results are probably always iterable but support a non-iterable
        option just in case.
        '''

        # Retrieve the flagging results
        results = flagdata_result.read()

        # If results is iterable loop over the results.
        if isinstance (results, collections.Iterable):

            # Construct the list of metrics.
            mlist = []
            for i in range(len(results)):
                agent_stats = calcmetrics.calc_flags_per_agent(results[i].summaries)
                mlist.append (reduce(operator.add, [float(s.flagged) / s.total for s in \
                    agent_stats if s.name in ['online', 'shadow']], 0))

            # Find the highest valued metric.
            metric, idx = max ((metric, idx) for (idx, metric) in enumerate(mlist)) 
            if idx is None:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
                if metric is not None:
                    metric = '%0.3f' % (100.0 * metric)

        # Single result.
        else:
            # By definition this is the result with the lowest metric
            if not results:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results.inputs['vis']))[0]
                agent_stats = calcmetrics.calc_flags_per_agent(results.summaries)
                metric = reduce(operator.add, [float(s.flagged) / s.total for s in \
                    agent_stats if s.name in ['online', 'shadow']], 0)
                metric = '%0.3f' % (100.0 * metric)

        eltree.SubElement(stage_element, "Metric", Name="%OnlineShadowFlags",
            Value=metric, Asdm=vis)


    def add_tsys_flagging_metric (self, stage_element, tsysflag_result):

        '''
        Recompute the metric defining the percentage of newly flagged caltable
        data.

        Results are probably always iterable but support a non-iterable
        option just in case.
        '''

        # Retrieve the flagging summaries
        results = tsysflag_result.read()

        # If results is iterable loop over the results.
        if isinstance (results, collections.Iterable):

            # Construct the list of metrics.
            mlist = []
            for i in range(len(results)):
                if not results[i]:
                    continue
                agent_stats = calcmetrics.calc_flags_per_agent(results[i].summaries)
                mlist.append (reduce(operator.add, [float(s.flagged) / s.total for s in \
                    agent_stats[1:]], 0))

            # Locate the largest metric
            metric, idx = max ((metric, idx) for (idx, metric) in enumerate(mlist)) 
            if idx is None:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
                if metric is not None:
                    metric = '%0.3f' % (100.0 * metric)

        # Single result.
        else:
            # By definition this is the result with the lowest metric
            if not results:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results.inputs['vis']))[0]
                agent_stats = calcmetrics.calc_flags_per_agent(results.summaries)
                metric =  reduce(operator.add, [float(s.flagged)/s.total for s in agent_stats[1:]], 0)
                metric = '%0.3f' % (100.0 * metric)

        eltree.SubElement(stage_element, "Metric", Name="%TsysCaltableFlags",
            Value=metric, Asdm=vis)


    def add_fluxcal_flagging_metric (self, stage_element, fluxcalflag_result):

        '''
        Recompute the metric defining the percentage of newly flagged data.

        Results are probably always iterable but support a non-iterable
        option just in case.
        '''

        # Retrieve the flagging summaries
        results = fluxcalflag_result.read()

        # If results is iterable loop over the results.
        if isinstance (results, collections.Iterable):

            # Construct the list of metrics.
            mlist = []
            for i in range(len(results)):
                if not results[i]:
                    continue
                agent_stats = calcmetrics.calc_flags_per_agent(results[i].summaries)
                mlist.append (reduce(operator.add, [float(s.flagged) / s.total for s in \
                    agent_stats[1:]], 0))

            # Locate the largest metric
            metric, idx = max ((metric, idx) for (idx, metric) in enumerate(mlist)) 
            if idx is None:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
                if metric is not None:
                    metric = '%0.3f' % (100.0 * metric)

        # Single result
        else:
            # By definition this is the result with the lowest metric
            if not results:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results.inputs['vis']))[0]
                agent_stats = calcmetrics.calc_flags_per_agent(results.summaries)
                metric =  reduce(operator.add, [float(s.flagged)/s.total for s in agent_stats[1:]], 0)
                metric = '%0.3f' % (100.0 * metric)

        eltree.SubElement(stage_element, "Metric", Name="%FluxcalFlags",
            Value=metric, Asdm=vis)

    def add_rawflagchans_flagging_metric (self, stage_element, rawflagchans_result):

        '''
        Recompute the metric defining the percentage of newly flagged data.

        Results are probably always iterable but support a non-iterable
        option just in case.
        '''

        # Retrieve the flagging summaries
        results = rawflagchans_result.read()

        # If results is iterable loop over the results.
        if isinstance (results, collections.Iterable):

            # Construct the list of metrics.
            mlist = []
            for i in range(len(results)):
                if not results[i].view:
                    continue
                agent_stats = calcmetrics.calc_flags_per_agent(results[i].summaries)
                mlist.append (reduce(operator.add, [float(s.flagged) / s.total for s in \
                    agent_stats[1:]], 0))

            # Locate the largest metric
            metric, idx = max ((metric, idx) for (idx, metric) in enumerate(mlist)) 
            if idx is None:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
                if metric is not None:
                    metric = '%0.3f' % (100.0 * metric)

        # Single result
        else:
            # By definition this is the result with the lowest metric
            if not results:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results.inputs['vis']))[0]
                if not results.view:
                    metric = 'Undefined'
                else:
                    agent_stats = calcmetrics.calc_flags_per_agent(results.summaries)
                    metric =  reduce(operator.add, [float(s.flagged)/s.total for s in agent_stats[1:]], 0)
                    metric = '%0.3f' % (100.0 * metric)

        eltree.SubElement(stage_element, "Metric", Name="%RawBadchansFlags",
            Value=metric, Asdm=vis)


    def add_phase_rms_ratio_metric (self, stage_element, wvr_result):

        '''
        Retrieve the ratio of phase rms (without wvr) / phase rms (with wvr)
        metric.  This metric is incorrectly labeled as a score in the results.
        This metric is either None or a floating point value.

        Results are probably always iterable but support a non-iterable
        option just in case.
        '''

        # Retrieve the results.
        results = wvr_result.read()

        # If results is iterable loop over the results.
        if isinstance (results, collections.Iterable):

            # Construct the list of results.
            rlist = [r.qa_wvr.overall_score for r in results]

            # Evaluate the metrics
            metric, idx = min ((metric, idx) for (idx, metric) in enumerate(rlist)) 
            if idx is None:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
                if metric is not None:
                    metric = '%0.3f' % results[idx].qa_wvr.overall_score
                else:
                    metric = 'Undefined'

        # Single result
        else:
            # By definition this is the result with the lowest metric
            if not results:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results.inputs['vis']))[0]
                if results.qa_wvr.overall_score is None:
                    metric = 'Undefined'
                else:
                    metric = '%0.3f' % results.qa_wvr.overall_score
            print 'single', 'Metric', metric, 'Vis', vis

        eltree.SubElement(stage_element, "Metric", Name="PhaseRmsRatio",
            Value=metric, Asdm=vis)


    def add_highlow_gain_flagging_metric (self, stage_element,
        gainflag_result):

        '''
        Recompute  the metric that defines the percentage of newly
        flagged data. Account for the metric which defines the
        existence of flagging views.

        Results are probably always iterable but support a non-iterable
        option just in case.
        '''

        # Retrieve the results.
        results = gainflag_result.read()

        # If results is iterable loop over the results.
        if isinstance (results, collections.Iterable):

            # Construct the list of results.
            mlist = []
            for i in range(len(results)):
                if not results[i].view:
                    continue
                agent_stats = calcmetrics.calc_flags_per_agent(results[i].summaries)
                mlist.append (reduce(operator.add, [float(s.flagged) / s.total for s in \
                    agent_stats[1:]], 0))

            # Evaluate the metrics
            metric, idx = max ((metric, idx) for (idx, metric) in enumerate(mlist)) 
            if idx is None:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
                if metric is not None:
                    metric = '%0.3f' % (100.0 * metric)

        # Single result
        else:
            # By definition this is the result with the lowest metric
            if not results:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results.inputs['vis']))[0]
                if not results.view:
                    metric = 'Undefined'
                else:
                    agent_stats = calcmetrics.calc_flags_per_agent(results.summaries)
                    metric =  reduce(operator.add, [float(s.flagged)/s.total for s in agent_stats[1:]], 0)
                    metric = '%0.3f' % (100.0 * metric)

        eltree.SubElement(stage_element, "Metric", Name="%HighLowGainFlags",
            Value=metric, Asdm=vis)

    def add_gain_flagging_metric (self, stage_element,
        gainflag_result):

        '''
        Recompute  the metric that defines the percentage of newly
        flagged data. Account for the metric which defines the
        existence of flagging views.

        Results are probably always iterable but support a non-iterable
        option just in case.
        '''

        # Retrieve the results.
        results = gainflag_result.read()

        # If results is iterable loop over the results.
        if isinstance (results, collections.Iterable):

            # Construct the list of results.
            mlist = []
            for i in range(len(results)):
                agent_stats = calcmetrics.calc_flags_per_agent(results[i].summaries)
                mlist.append (reduce(operator.add, [float(s.flagged) / s.total for s in \
                    agent_stats[1:]], 0))

            # Evaluate the metrics
            metric, idx = max ((metric, idx) for (idx, metric) in enumerate(mlist)) 
            if idx is None:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
                if metric is not None:
                    metric = '%0.3f' % (100.0 * metric)

        # Single result
        else:
            # By definition this is the result with the lowest metric
            if not results:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results.inputs['vis']))[0]
                agent_stats = calcmetrics.calc_flags_per_agent(results.summaries)
                metric =  reduce(operator.add, [float(s.flagged)/s.total for s in agent_stats[1:]], 0)
                metric = '%0.3f' % (100.0 * metric)

        eltree.SubElement(stage_element, "Metric", Name="%GainFlags",
            Value=metric, Asdm=vis)


    def add_applycal_flagging_metric (self, stage_element,
        applycal_result):

        '''
        Recompute the metric defining the percentage of newly flagged data.

        This method is amost identical to the deterministic flagging
        task metric.
        '''

        # Retrieve the results
        results = applycal_result.read()

        # If results is iterable loop over the results.
        if isinstance (results, collections.Iterable):

            # Construct the list of results.
            mlist = []
            for i in range(len(results)):
                agent_stats = calcmetrics.calc_flags_per_agent(results[i].summaries)
                mlist.append (reduce(operator.add, [float(s.flagged) / s.total for s in \
                    agent_stats if s.name in ['applycal']], 0))

            # Evaluate the metric
            metric, idx = max ((metric, idx) for (idx, metric) in enumerate(mlist)) 
            if idx is None:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
                if metric is not None:
                    metric = '%0.3f' % (100.0 * metric)

        # Single result
        else:
            # By definition this is the result with the lowest metric
            if not results:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results.inputs['vis']))[0]
                agent_stats = calcmetrics.calc_flags_per_agent(results.summaries)
                metric = reduce(operator.add, [float(s.flagged) / s.total for s in \
                    agent_stats if s.name in ['online', 'shadow']], 0)
                metric = '%0.3f' % (100.0 * metric)

        eltree.SubElement(stage_element, "Metric", Name="%ApplycalFlags",
            Value=metric, Asdm=vis)

    def write (self, filename):

        """
        Convert the document to a nicely formatted XML string
        and save it in a file
        """

        xmlstr = eltree.tostring(self.aquareport, 'utf-8')

        # Reformat it to prettyprint style
        reparsed = minidom.parseString(xmlstr)
        reparsed_xmlstr = reparsed.toprettyxml(indent="  ")

        # Save it to a file.
        with open (filename, "w") as aquafile:
            aquafile.write(reparsed_xmlstr)

def get_pipeline_task_classes (proxy):

    '''
    Get the Python task class name
        Check if result is iterable or not
        Issue if more than one class in results
        Not currently used
    '''

    result = proxy.read()
    if isinstance (result, collections.Iterable):
        classes = [r.task for r in utils.flatten(result)]
        return classes[0] if len(classes) is 1 else classes
    else:
        class_name = result.task
        return class_name

def get_pipeline_pytask_names (proxy):

    '''
    Get the Python task module and class name
        Check if result is iterable or not
        Issue if more than one module / task in results
        Not currently used
    '''

    result = proxy.read()
    if isinstance (result, collections.Iterable):
        names = ['%s.%s' % (r.task.__module__, r.task.__name__) for r in \
            utils.flatten(result)]
        return names[0] if len(names) is 1 else names
    else:
        name = result.task
        return name

def get_pipeline_stage_and_score (proxy):

    '''
    Get the CASA equivalent task name which is stored by the infrastructure
    as  <task_name> (<arg1> = <value1>, ...)
    '''

    result = proxy.read()
    casa_task_call = result.pipeline_casa_task
    first_bracket = casa_task_call.index('(')
    stage_name = casa_task_call[0:first_bracket]
    if isinstance (result, collections.Iterable):
        score = min([r.qa.representative.score for r in utils.flatten(result)])
    else:
        score = result.qa.representative.score
    return stage_name, score


