"""
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

    The scores and metrics are noew stored with the stage results.

    Metrics may have units information. They may be encoded as
    CASA quanta strings if appropriate.

Future Technical Solutions
    Suggestions
    Add a toAqua method to the base results class which returns a
    list of metrics for export. Pass these to the QA classes
    for scoring.

    Add the euivalent of a  toAqua registration method similar to what is
    done with QA handlers already
"""
from __future__ import absolute_import

import collections
import datetime
import operator
import os
import xml.etree.cElementTree as ElementTree
from xml.dom import minidom

import itertools
from casa_system import casa as casasys

import pipeline.environment as environment
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.utils as utils


LOG = logging.get_logger(__name__)

# constant for an undefined value
UNDEFINED = 'Undefined'

# this holds all QA-metric-to-XML export functions
_AQUA_REGISTRY = set()


def register_aqua_metric(fn):
    """
    Register a 'QA metric to XML' conversion function.

    This function can also be used as a decorator.

    :param fn: function to register
    :return:
    """
    _AQUA_REGISTRY.add(fn)
    return fn


class AquaXmlGenerator(object):
    """
    Class to create the XML for an AQUA pipeline report.
    """

    def get_report_xml(self, context):
        """
        Create and return the AQUA report XML for the results stored in a
        context.

        :param context: pipeline context to parse
        :return: root XML Element of AQUA report
        :rtype: xml.etree.cElementTree.Element
        """
        # read in all results in the context
        all_results = [r.read() for r in context.results]
        # the imaging tasks don't wrap themselves in a ResultsList. Until they
        # do, we have to fake that here.
        for idx, r in enumerate(all_results):
            try:
                iter(r)
            except TypeError:
                # temporary import for expedience
                # TODO make this a utility function
                import pipeline.infrastructure.renderer.htmlrenderer as htmlrenderer
                all_results[idx] = htmlrenderer.wrap_in_resultslist(r)

        # Initialize
        root = ElementTree.Element('PipelineAquaReport')

        # Construct the project structure element
        root.append(self.get_project_structure(context))

        # Construct the QA summary element
        root.append(self.get_qa_summary(context))

        # Construct the per pipeline stage elements
        root.append(self.get_per_stage_qa(context, all_results))

        # Construct the topics elements.
        root.append(self.get_topics_qa(context, all_results))

        return root

    def get_project_structure(self, context):
        """
        Get the project structure element.

        Given the current data flow it is unclear how the report generator
        generator will acquire the entity id of the original processing
        request.

        The processing procedure name is known but not yet passed to the
        pipeline processing request.

        :param context: pipeline context
        :return: XML for project structure
        :rtype: xml.etree.cElementTree.Element
        """
        root = ElementTree.Element('ProjectStructure')

        ElementTree.SubElement(root, 'ProposalCode').text = context.project_summary.proposal_code
        ElementTree.SubElement(root, 'ProcessingRequestEntityId').text = UNDEFINED
        ElementTree.SubElement(root, 'ProcessingProcedure').text = UNDEFINED

        return root

    def get_qa_summary(self, context):
        """
        Get the AQUA summary XML element.

        :param context: pipeline context
        :return: XML summarising execution
        :rtype: xml.etree.cElementTree.Element
        """
        root = ElementTree.Element('QaSummary')

        # Generate the report date
        now = datetime.datetime.utcnow()
        ElementTree.SubElement(root, 'ReportDate').text = now.strftime('%Y-%m-%d %H:%M:%S')

        # Processing time
        exec_start = context.results[0].read().timestamps.start
        exec_end = context.results[-1].read().timestamps.end
        # remove unnecessary precision for execution duration
        dt = exec_end - exec_start
        exec_duration = datetime.timedelta(days=dt.days, seconds=dt.seconds)
        ElementTree.SubElement(root, 'ProcessingTime').text = str(exec_duration)

        # Software versions
        ElementTree.SubElement(root, 'CasaVersion').text = casasys['build']['version'].strip()
        ElementTree.SubElement(root, 'PipelineVersion').text = environment.pipeline_revision

        # Score for the complete pipeline run
        # NB. the final pipeline score is not yet available.
        ElementTree.SubElement(root, 'FinalScore').text = UNDEFINED

        return root

    def get_per_stage_qa(self, context, all_results):
        """
        Get the XML for all stages.

        :param context: pipeline context
        :param all_results: all Results for this pipeline run
        :return: XML for all stages
        :rtype: xml.etree.cElementTree.Element
        """
        # Get the stage summary element.
        xml_root = ElementTree.Element('QaPerStage')

        ordered_results = sorted(all_results, key=operator.attrgetter('stage_number'))
        for stage_result in ordered_results:
            # Create the generic stage element
            stage_name, stage_score = _get_pipeline_stage_and_score(stage_result)
            stage_element = ElementTree.Element('Stage',
                                                Number=str(stage_result.stage_number),
                                                Name=stage_name,
                                                Score=str(stage_score))

            # create a list of (vis, qa score) tuples for this metric
            vis_qa_scores = [(ms_result.inputs['vis'], qa_score)
                             for ms_result in stage_result
                             for qa_score in ms_result.qa.pool]

            # calculate which items have specific renderers and which require
            # procesing by the generic renderer
            needs_specific = [(vis, qa_score) for vis, qa_score in vis_qa_scores
                              if any([fn.handles(qa_score.origin.metric_name) for fn in _AQUA_REGISTRY])]
            needs_generic = [(vis, qa_score) for vis, qa_score in vis_qa_scores
                             if not any([fn.handles(qa_score.origin.metric_name) for fn in _AQUA_REGISTRY])]

            # create a pseudo registry for the generic XML generator
            generic_registry = {GenericMetricXmlGenerator()}

            # generate XML for those with a specific renderer
            specific_elements = self._get_xml_for_qa_scores(needs_specific, _AQUA_REGISTRY)
            generic_elements = self._get_xml_for_qa_scores(needs_generic, generic_registry)

            stage_element.extend(specific_elements)
            stage_element.extend(generic_elements)

            xml_root.append(stage_element)

        return xml_root

    def _get_xml_for_qa_scores(self, items, registry):
        """
        Generate the XML elements for a list of QA scores.

        :param items: list of (vis, QAScore) tuples
        :param registry: list of XML generator functions
        :return: list of XML elements
        :rtype: list of xml.etree.ElementTree
        """
        # group scores into a {<metric name>: [<QAScore, ...>]} dict
        metric_to_scores = {}
        keyfunc = lambda (_, qa_score): qa_score.origin.metric_name
        s = sorted(list(items), key=keyfunc)
        for k, g in itertools.groupby(s, keyfunc):
            metric_to_scores[k] = list(g)

        # let each generator process the QA scores it can handle, accumulating
        # the XML as we go
        elements = []
        for metric_name, scores in metric_to_scores.iteritems():
            xml = [fn(scores) for fn in registry if fn.handles(metric_name)]
            elements.extend(utils.flatten(xml))

        return elements

    def get_topics_qa(self, context, all_results):
        """
        Get the XML for all results, divided into sections by topic.

        :param context: pipeline context
        :param all_results: all Results for this pipeline run
        :return: XML for topics
        :rtype: xml.etree.cElementTree.Element
        """
        # Set the top level topics element.
        root = ElementTree.Element('QaPerTopic')

        # Add the data topic
        topic = qaadapter.registry.get_dataset_topic()
        dataset_results = [r for r in all_results if topic.handles_result(r)]
        root.append(self.get_dataset_topic(context, dataset_results))

        # Add the flagging topic
        topic = qaadapter.registry.get_flagging_topic()
        flagging_results = [r for r in all_results if topic.handles_result(r)]
        root.append(self.get_flagging_topic(context, flagging_results))

        # Add the calibration topic
        topic = qaadapter.registry.get_calibration_topic()
        calibration_results = [r for r in all_results if topic.handles_result(r)]
        root.append(self.get_calibration_topic(context, calibration_results))

        # Add the imaging topic
        topic = qaadapter.registry.get_imaging_topic()
        imaging_results = [r for r in all_results if topic.handles_result(r)]
        root.append(self.get_imaging_topic(context, imaging_results))

        return root

    def get_calibration_topic(self, context, topic_results):
        """
        Get the XML for the calibration topic.

        :param context: pipeline context
        :param topic_results: list of Results for this topic
        :return: XML for calibration topic
        :rtype: xml.etree.cElementTree.Element
        """
        return self._xml_for_topic('Calibration', context, topic_results)

    def get_dataset_topic(self, context, topic_results):
        """
        Get the XML for the dataset topic.

        :param context: pipeline context
        :param topic_results: list of Results for this topic
        :return: XML for dataset topic
        :rtype: xml.etree.cElementTree.Element
        """
        return self._xml_for_topic('Dataset', context, topic_results)

    def get_flagging_topic(self, context, topic_results):
        """
        Get the XML for the flagging topic.

        :param context: pipeline context
        :param topic_results: list of Results for this topic
        :return: XML for flagging topic
        :rtype: xml.etree.cElementTree.Element
        """
        return self._xml_for_topic('Flagging', context, topic_results)

    def get_imaging_topic(self, context, topic_results):
        """
        Get the XML for the imaging topic.

        :param context: pipeline context
        :param topic_results: list of Results for this topic
        :return: XML for imaging topic
        :rtype: xml.etree.cElementTree.Element
        """
        return self._xml_for_topic('Imaging', context, topic_results)

    def _xml_for_topic(self, topic_name, context, topic_results):
        # the overall topic score is defined as the minimum score of all
        # representative scores for each task in  that topic, which themselves
        # are the minimum of the scores for that task
        try:
            min_score = min([r.qa.representative for r in topic_results], key=operator.attrgetter('score'))
            score = str(min_score.score)
        except ValueError:
            # empty list
            score = UNDEFINED

        xml_root = ElementTree.Element(topic_name, Score=score)
        topic_xml = self.get_per_stage_qa(context, topic_results)
        xml_root.extend(topic_xml)

        return xml_root


def export_to_disk(report, filename):
    """
    Convert an XML document to a nicely formatted XML string and save it in a
    file.
    """
    xmlstr = ElementTree.tostring(report, 'utf-8')

    # Reformat it to prettyprint style
    reparsed = minidom.parseString(xmlstr)
    reparsed_xmlstr = reparsed.toprettyxml(indent='  ')

    # Save it to a file.
    with open(filename, 'w') as aquafile:
        aquafile.write(reparsed_xmlstr)


def vis_to_asdm(vispath):
    """
    Get the expected ASDM name from the path of a measurement set.

    :param vispath: path to convert
    :return: expected name of ASDM for MS
    """
    return os.path.splitext(os.path.basename(vispath))[0]


def xml_generator_for_metric(qa_label, value_spec):
    """
    Return a function that converts a matching QAScore to XML.

    :param qa_label: QA metric label to match
    :param value_spec: string format spec for how to format metric value
    :return: function
    """
    # We don't (yet) allow % in the output XML, even when it represents a
    # percentage
    if value_spec.endswith('%}'):
        value_formatter = _create_trimmed_formatter(value_spec, 1)
    else:
        value_formatter = _create_value_formatter(value_spec)

    # return LowestScoreMetricXmlGenerator(qa_label, formatters={'Value': value_formatter})
    return MetricXmlGenerator(qa_label, formatters={'Value': value_formatter})


class MetricXmlGenerator(object):
    """
    Creates a AQUA report XML element for QA scores.
    """

    def __init__(self, metric_name, formatters=None):
        """
        The constructor accepts an optional dict of string formatters: functions
        that accept a string and return a formatted string. If this argument is
        not supplied, the default formatter keys and formatter functions applied
        will be:

            'Name': convert to string
            'Value': convert to string
            'Asdm': return basename minus extension
            'QaScore': convert to string

        :param metric_name: metric to match
        :param formatters: (optional) dict string formatters
        """
        self.metric_name = metric_name

        # set default attribute formatters before updating with user overrides
        self.attr_formatters = {
            'Name': str,
            'Value': str,
            'Asdm': vis_to_asdm,
            'QaScore': str,
        }
        if formatters:
            self.attr_formatters.update(formatters)

    def __call__(self, vis_qa_scores):
        scores_to_process = self.filter(vis_qa_scores)
        return [self.to_xml(vis, score) for vis, score in scores_to_process]

    def handles(self, metric_name):
        """
        Returns True if this class can generate XML for the given metric.

        :param metric_name: name of metric
        :return: True if metric handled by this class
        """
        return metric_name == self.metric_name

    def filter(self, qa_scores):
        """
        Reduce a list of entries to those entries that require XML to be generated.

        :param qa_scores: list of (vis, QAScore)
        :return: list of (vis, QAScore)
        """
        return qa_scores

    def to_xml(self, vis, qa_score):
        """
        Return the XML representation of a QA score and associated metric.

        :param vis: name of MS
        :param qa_score: QA score to convert
        :return: XML element
        :rtype: xml.etree.ElementTree.Element
        """
        if not qa_score:
            return None

        metric = qa_score.origin
        qa_score = str(qa_score.score)

        if isinstance(vis, list):
            asdm = ','.join([self.attr_formatters['Asdm'](v) for v in vis])
        else:
            asdm = self.attr_formatters['Asdm'](vis)

        return ElementTree.Element(
            'Metric',
            Name=self.attr_formatters['Name'](metric.metric_name),
            Value=self.attr_formatters['Value'](metric.metric_score),
            Asdm=asdm,
            QaScore=self.attr_formatters['QaScore'](qa_score)
        )


class LowestScoreMetricXmlGenerator(MetricXmlGenerator):
    """
    Metric XML Generator that only returns XML for the lowest QA score that it
    handles.
    """

    def __init__(self, metric_name, formatters=None):
        super(LowestScoreMetricXmlGenerator, self).__init__(metric_name, formatters)

    def filter(self, qa_scores):
        handled = [(vis, qa_score) for vis, qa_score in qa_scores
                   if self.handles(qa_score.origin.metric_name)]

        if not handled:
            return []

        lowest = min(handled, key=lambda (_, qa_score): operator.attrgetter('score'))
        return [lowest]


class GenericMetricXmlGenerator(MetricXmlGenerator):
    """
    Metric XML Generator that processes any score it is given, formatting the
    metric value to 3dp.
    """

    def __init__(self):
        # format all processed entries to 3dp
        formatters = {'Value': _create_value_formatter('{:0.3f}')}
        super(GenericMetricXmlGenerator, self).__init__('Generic metric', formatters)

    def handles(self, _):
        return True


def _create_trimmed_formatter(format_spec, trim=0):
    """
    Create a function that formats values as a percent.

    :param format_spec: string format specification to apply
    :param trim: number of characters to trim
    :return: function
    """
    g = _create_value_formatter(format_spec)

    def f(val):
        val = g(val)
        if val == UNDEFINED:
            return UNDEFINED
        else:
            return val[:-trim]

    return f


def _create_value_formatter(format_spec):
    """
    Create a function that applies a string format spec.

    This function return a function that accepts one argument and returns the
    string formatted according to the given string format specification. If
    the argument cannot be formatted, the default 'undefined' string will be
    returned.

    This is used internally to create a set of formatting functions that all
    exhibit the same behaviour, whereby 'Undefined' is returned on errors.

    :param format_spec: string format specification to apply
    :return: function
    """
    def f(val):
        try:
            return format_spec.format(val)
        except ValueError:
            return UNDEFINED

    return f


def _get_pipeline_stage_and_score(result):
    """
    Get the CASA equivalent task name which is stored by the infrastructure
    as  <task_name> (<arg1> = <value1>, ...)
    """
    stage_name = result.pipeline_casa_task.split('(')[0]
    if isinstance(result, collections.Iterable):
        score = min([r.qa.representative.score for r in utils.flatten(result)])
    else:
        score = result.qa.representative.score
    return stage_name, score
