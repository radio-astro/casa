from __future__ import absolute_import

import os
import datetime
import xml.etree.cElementTree as eltree
from xml.dom import minidom
import collections
import operator

import casadef
import pipeline
import pipeline.domain.measures as measures
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils

from casa_system import casa as casasys

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
    aquaReport.set_topics_qa()

    LOG.info ("Writing aqua report file: %s" % (aquaFile))
    aquaReport.write(aquaFile)


class AquaReport(object):
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

        # Store the context
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
        # and store it.
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
        eltree.SubElement (ps, "CasaVersion").text = casasys['build']['version'].strip()
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


        return ppqa

    def set_topics_qa (self):

        '''
        Add the per topics elements
        '''

        # Set the top level topics element.
        topicqa = eltree.SubElement(self.aquareport, "QaPerTopic")

        # Add the data topic
        # TBD

        # Add the flagging topic
        # TBD

        # Add the calibration topic
        self.add_calibration_topic(topicqa)
        
        # Add the imaging topic
        # TBD

        return topicqa

    def add_calibration_topic (self, topicqa):

        # Check if there are any calibration results
        #    Return if not. Registry seems linked to web log. How to detect this ?
        # TBD

        # Set the calibration topics element element
        calqa = eltree.SubElement(topicqa, "Calibration", Score="Undefined")

        return calqa

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

