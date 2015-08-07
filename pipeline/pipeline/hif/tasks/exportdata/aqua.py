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
    metrics are scored.

Structure
    The report contains
        A project structure section.
        A QA summary section.
        A per stage QA section.
        A per topic QA section.
 
Issues with the Original Design
    The per ASDM dimension was ignored.
    
    The multiple metrics / scores per stage and / or per ASDM
    dimension was ignored.

    For stages with single scores / metrics and multiple ASDMs the
    current implementation selects the MS with the worst metric and
    reports that value. This metric by definition corresponds to
    the lowest score.

    Tasks which have multiple scores / metrics and multiple
    ASDMs are currently dealt with on an ad hoc basis.


Future Technical Solutions
    Add a toAqua method to the base results class which returns a
    list of metrics

    Extend or modify the score objects to include a name or
    identifier and the value of the metric used to generate the
    score.

    Should metrics include a unit, e.g. be a quanta.

    Other ?

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
        #   This seems a bit inefficient as it requires  reading all
        #   the results to reconstruct the list of stages

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

        Given the current data flow it is unclear how to
        acquire the entity id of the original processing request

        The processing procedure name is known but not yet
        passed to the pipeline processing request
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
        eltree.SubElement (ps, "ReportDate").text = \
            datetime.datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S')

        exec_start = self.context.results[0].read().timestamps.start
        exec_end = self.context.results[-1].read().timestamps.end
        # remove unnecessary precision for execution duration
        dt = exec_end - exec_start
        exec_duration = datetime.timedelta(days=dt.days, seconds=dt.seconds)
        eltree.SubElement (ps, "ProcessingTime").text = str(exec_duration)

        eltree.SubElement (ps, "CasaVersion").text = casadef.casa_version
        eltree.SubElement (ps, "PipelineVersion").text = pipeline.revision

        eltree.SubElement (ps, "FinalScore").text = "Undefined"

        return ps

    def set_per_stage_qa (self):

        '''
        Add the per stage elements
            Stage number, name, and score are attributes

            Eventually we will need:
               A scores class than includes name and metric attributes
               A toAqua method on the task results class to pass pipeline
               metrics back to the AQUA report generator

        '''

        # Get the summary element.
        ppqa = eltree.SubElement(self.aquareport, "QaPerStage")

        # Loop over the stages.
        for stage in self.stagedict: 

            # Create the generic stage element
            st = eltree.SubElement(ppqa, "Stage", Number=str(stage),
                Name=self.stagedict[stage][0], Score=self.stagedict[stage][1])

            # Populate the stage elements.
            #    This must be done in a custom manner for now

            # Deterministic flagging
            #     Retrieve the result with the highest online and shadow
            #     flagging metric

            if self.stagedict[stage][0] == 'hifa_flagdata':
                self.add_online_shadow_flagging_metric(st,
                    self.context.results[stage-1])

            # Fluxcal flagging
            #    Retrieve the results with the highest flagging percentage
            #    Note: The current metric needs works, should be the percentage
            #    of the flux calibrator flagged, not the perctage of total flagged

            elif self.stagedict[stage][0] == 'hifa_fluxcalflag':
                self.add_fluxcal_flagging_metric(st,
                    self.context.results[stage-1])

            # Tsys flagging
            #     Retrieve the result with the highest flagging percentage

            elif self.stagedict[stage][0] == 'hifa_tsysflag':
                self.add_tsys_flagging_metric(st,
                    self.context.results[stage-1])

            # WVR calibration and flagging
            #     Retrieve the result with the lowest rms improvement metric

            elif self.stagedict[stage][0] == 'hifa_wvrgcalflag':
                self.add_phase_rms_ratio_metric(st,
                    self.context.results[stage-1])

            # Deviant high / low gain flagging
            #     Retrieve the result with the highest flagging percentage

            elif self.stagedict[stage][0] == 'hif_lowgainflag':
                self.add_highlow_gain_flagging_metric(st,
                    self.context.results[stage-1])

            # Applycal flagging
            #     Retrieve the result with the highest applycal flagging metric

            elif self.stagedict[stage][0] == 'hif_applycal':
                self.add_applycal_flagging_metric(st,
                    self.context.results[stage-1])

            # Generic empty result
            else:
                pass
                
        return ppqa

    def add_online_shadow_flagging_metric (self, stage_element,
        flagdata_result):

        '''
        hifa_flagdata  currently generates only a single metric, the
            percentage flagged by the online and shadow flagging agents.
        This method is amost identical to the applycal flagging
        task metric.
        '''

        # Retrieve the flagging summaries
        results = flagdata_result.read()
        if isinstance (results, collections.Iterable):
            mlist = []
            for i in range(len(results)):
                agent_stats = calcmetrics.calc_flags_per_agent(results[i].summaries)
                mlist.append (reduce(operator.add, [float(s.flagged) / s.total for s in \
                    agent_stats if s.name in ['online', 'shadow']], 0))
            metric, idx = max ((metric, idx) for (idx, metric) in enumerate(mlist)) 
            if idx is None:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
                if metric is not None:
                    metric = '%0.3f' % (100.0 * metric)
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
        hifa_tsysflag currently generates 1 metric based on the percentage of
        newly flagged data.

        '''

        # Retrieve the flagging summaries
        results = tsysflag_result.read()
        if isinstance (results, collections.Iterable):
            mlist = []
            for i in range(len(results)):
                if not results[i]:
                    continue
                agent_stats = calcmetrics.calc_flags_per_agent(results[i].summaries)
                mlist.append (reduce(operator.add, [float(s.flagged) / s.total for s in \
                    agent_stats[1:]], 0))
            metric, idx = max ((metric, idx) for (idx, metric) in enumerate(mlist)) 
            if idx is None:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
                if metric is not None:
                    metric = '%0.3f' % (100.0 * metric)
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
        hifa_fluxcalflag currently generates 1 metric based on the percentage of
        newly flagged data.

        '''

        # Retrieve the flagging summaries
        results = fluxcalflag_result.read()
        if isinstance (results, collections.Iterable):
            mlist = []
            for i in range(len(results)):
                if not results[i]:
                    continue
                agent_stats = calcmetrics.calc_flags_per_agent(results[i].summaries)
                mlist.append (reduce(operator.add, [float(s.flagged) / s.total for s in \
                    agent_stats[1:]], 0))
            metric, idx = max ((metric, idx) for (idx, metric) in enumerate(mlist)) 
            if idx is None:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
                if metric is not None:
                    metric = '%0.3f' % (100.0 * metric)
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

    def add_phase_rms_ratio_metric (self, stage_element, wvr_result):

        '''
        hifa_wvrgcalflag currently generates only a single metric, the
            ratio of phase rms (without wvr) / phase rms (with wvr).
        This metric is incorrectly labeled as a score.
        This metric is either None or a floating point value.
        '''

        # Retrieve the rms improvement metric
        results = wvr_result.read()
        if isinstance (results, collections.Iterable):

            # Find the results with the lowest metric which for this task corresponds to
            # to the lowest overall score.
            #rlist = [r.qa_wvr.overall_score for r in utils.flatten(results)]
            rlist = [r.qa_wvr.overall_score for r in results]
            metric, idx = min ((metric, idx) for (idx, metric) in enumerate(rlist)) 
            if idx is None:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
                if metric is not None:
                    metric = '%0.3f' % results[idx].qa_wvr.overall_score

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

        eltree.SubElement(stage_element, "Metric", Name="PhaseRmsRatio",
            Value=metric, Asdm=vis)

    def add_highlow_gain_flagging_metric (self, stage_element,
        gainflag_result):

        '''
        hif_lowgainflag currently generates 2 metrics, one based on the
            number of views that can be computed, and the other based on
            the percentage of newly flagged data.

        '''

        # Retrieve the flagging summaries
        results = gainflag_result.read()
        if isinstance (results, collections.Iterable):
            mlist = []
            for i in range(len(results)):
                if not results[i].view:
                    continue
                agent_stats = calcmetrics.calc_flags_per_agent(results[i].summaries)
                mlist.append (reduce(operator.add, [float(s.flagged) / s.total for s in \
                    agent_stats[1:]], 0))
            metric, idx = max ((metric, idx) for (idx, metric) in enumerate(mlist)) 
            if idx is None:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
                if metric is not None:
                    metric = '%0.3f' % (100.0 * metric)
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

    def add_applycal_flagging_metric (self, stage_element,
        applycal_result):

        '''
        hif_applycal  currently generates only a single metric, the
            percentage of data flagged by the applycal flagging agent.

        This method is amost identical to the deterministic flagging
        task metric.
        '''

        # Retrieve the flagging summaries
        results = applycal_result.read()
        if isinstance (results, collections.Iterable):
            mlist = []
            for i in range(len(results)):
                agent_stats = calcmetrics.calc_flags_per_agent(results[i].summaries)
                mlist.append (reduce(operator.add, [float(s.flagged) / s.total for s in \
                    agent_stats if s.name in ['applycal']], 0))
            metric, idx = max ((metric, idx) for (idx, metric) in enumerate(mlist)) 
            if idx is None:
                vis = 'Undefined'
                metric = 'Undefined'
            else:
                vis = os.path.splitext(os.path.basename(results[idx].inputs['vis']))[0]
                if metric is not None:
                    metric = '%0.3f' % (100.0 * metric)
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


