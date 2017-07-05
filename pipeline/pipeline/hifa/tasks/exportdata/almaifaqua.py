from __future__ import absolute_import
import operator
import os
import xml.etree.cElementTree as ElementTree

import pipeline.domain.measures as measures
from pipeline.infrastructure import casatools
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.launcher as launcher

import pipeline.h.tasks.exportdata.aqua as aqua
from pipeline.h.tasks.exportdata.aqua import UNDEFINED, export_to_disk

LOG = logging.get_logger(__name__)


def aqua_report_from_file(context_file, aqua_file):
    """
    Create AQUA report from a context file on disk.
    """
    # Restore context from file
    LOG.info('Opening context file: {!s}'.format(context_file))
    context = launcher.Pipeline(context=context_file).context

    # Produce the AQUA report
    aqua_report_from_context(context, aqua_file)


def aqua_test_report_from_local_file(context_file, aqua_file):
    """
    Test AQUA report generation.
    The pipeline context file and web log directory must be in the same local directry
    """
    LOG.info('Opening context file: {!s} for test'.format(context_file))
    context = launcher.Pipeline(context=context_file, path_overrides={'name': os.path.splitext(context_file)[0],
                                                                      'output_dir': os.getcwd()}).context
    # Produce the AQUA report
    aqua_report_from_context(context, aqua_file)


def aqua_report_from_context(context, aqua_file):
    """
    Create AQUA report from a context object.
    """
    LOG.info('Recipe name: %s' % 'Unknown')
    LOG.info('    Number of stages: %d' % context.task_counter)

    # Initialize
    generator = AlmaAquaXmlGenerator()
    report = generator.get_report_xml(context)

    LOG.info('Writing aqua report file: %s' % aqua_file)
    export_to_disk(report, aqua_file)


class AlmaAquaXmlGenerator(aqua.AquaXmlGenerator):
    """
    Class for creating the AQUA pipeline report
    """

    def __init__(self):
        super(AlmaAquaXmlGenerator, self).__init__()

    def get_project_structure(self, context):
        # get base XML from base class
        root = super(AlmaAquaXmlGenerator, self).get_project_structure(context)

        # add our ALMA-specific elements
        ElementTree.SubElement(root, 'OusEntityId').text = context.project_structure.ous_entity_id
        ElementTree.SubElement(root, 'OusPartId').text = context.project_structure.ous_part_id
        ElementTree.SubElement(root, 'OusStatusEntityId').text = context.project_structure.ousstatus_entity_id

        return root

    def get_calibration_topic(self, context, topic_results):
        # get base XML from base class
        xml_root = super(AlmaAquaXmlGenerator, self).get_calibration_topic(context, topic_results)

        m = {
            'hifa_gfluxscale': (operator.attrgetter('measurements'), lambda r: str(r.qa.representative.score))
        }
        flux_xml = flux_xml_for_stages(context, topic_results, m)
        xml_root.extend(flux_xml)

        return xml_root

    def get_dataset_topic(self, context, topic_results):
        # get base XML from base class
        xml_root = super(AlmaAquaXmlGenerator, self).get_dataset_topic(context, topic_results)

        m = {
            'hifa_importdata': (lambda x: x.setjy_results[0].measurements, lambda _: UNDEFINED),
        }
        flux_xml = flux_xml_for_stages(context, topic_results, m)
        # omit containing flux measurement element if no measurements were found
        if len(list(flux_xml)) > 0:
            xml_root.extend(flux_xml)

        sensitivity_xml = sensitivity_xml_for_stages(context, topic_results)
        # omit containing flux measurement element if no measurements were found
        if len(list(sensitivity_xml)) > 0:
            xml_root.extend(sensitivity_xml)

        return xml_root


def flux_xml_for_stages(context, results, accessor_dict):
    """
    Get the XML for flux measurements contained in a list of results.

    This function is a higher-order function; it expects to be given a dict
    of accessor functions, which it uses to access the flux measurements and
    QA score of the appropriate results. 'Appropriate' means that the task
    name matches the dict key. This lets the function call different accessor
    functions for different types of result.

    The dict accessor dict uses task names as keys, with values as two-tuples
    comprising

        1. a function to access the flux measurements for a result
        2. a function to access the QA score for that result

    :param context: pipeline context
    :param results: results to process
    :param accessor_dict: dict of accessor functions
    :return: XML for flux measurements
    :rtype: xml.etree.cElementTree.Element
    """
    xml_root = ElementTree.Element('FluxMeasurements')

    for result in results:
        pipeline_casa_task = result.pipeline_casa_task
        for task_name, (flux_accessor, score_accessor) in accessor_dict.iteritems():
            # need parenthesis to distinguish between cases such as
            # hifa_gfluxscale and hifa_gfluxscaleflag
            if pipeline_casa_task.startswith(task_name + '('):
                flux_xml = xml_for_flux_stage(context, result, task_name, flux_accessor, score_accessor)
                xml_root.append(flux_xml)

    return xml_root


def xml_for_flux_stage(context, stage_results, origin, accessor, score_accessor):
    """
    Get the XML for all flux measurements contained in a ResultsList.

    :param context: pipeline context
    :param stage_results: ResultList containing flux results to summarise
    :param origin: text for Origin attribute value
    :param accessor: function that returns the flux measurements from the Result
    :param score_accessor: function that returns the QA score for the Result
    :rtype: xml.etree.cElementTree.Element
    """
    score = score_accessor(stage_results)
    xml_root = ElementTree.Element('FluxMeasurements', Origin=origin, Score=score)

    for result in stage_results:
        vis = os.path.basename(result.inputs['vis'])
        ms_for_result = context.observing_run.get_ms(vis)
        measurements = accessor(result)
        ms_xml = xml_for_extracted_flux_measurements(measurements, ms_for_result)
        xml_root.extend(ms_xml)

    return xml_root


def xml_for_extracted_flux_measurements(all_measurements, ms):
    """
    Get the XML for a set of flux measurements extracted from a Result.

    :param all_measurements: flux measurements dict.
    :param ms: measurement set
    :return: XML
    :rtype: xml.etree.cElementTree.Element
    """
    asdm = aqua.vis_to_asdm(ms.name)

    result = []
    for field_id, field_measurements in all_measurements.iteritems():
        field = ms.get_fields(field_id)[0]
        field_name = field.name

        if field_name.startswith('"') and field_name.endswith('"'):
            field_name = field_name[1:-1]

        for measurement in sorted(field_measurements, key=lambda m: int(m.spw_id)):
            spw = ms.get_spectral_window(measurement.spw_id)
            freq_ghz = '{:.6f}'.format(spw.centre_frequency.to_units(measures.FrequencyUnits.GIGAHERTZ))

            # I only for now ...
            for stokes in ['I']:
                try:
                    flux = getattr(measurement, stokes)
                    flux_jy = flux.to_units(measures.FluxDensityUnits.JANSKY)
                    flux_jy = '{:.3f}'.format(flux_jy)
                except:
                    continue

                try:
                    unc = getattr(measurement.uncertainty, stokes)
                    unc_jy = unc.to_units(measures.FluxDensityUnits.JANSKY)
                    if unc_jy != 0:
                        unc_jy = '{:.6f}'.format(unc_jy)
                    else:
                        unc_jy = ''
                except:
                    unc_jy = ''

                xml = ElementTree.Element('FluxMeasurement',
                                          SpwName=spw.name,
                                          MsSpwId=str(spw.id),
                                          FluxJy=flux_jy,
                                          ErrorJy=unc_jy,
                                          Asdm=asdm,
                                          Field=field_name,
                                          FrequencyGHz=freq_ghz)
                result.append(xml)

    return result


def sensitivity_xml_for_stages(context, results):
    xml_root = ElementTree.Element('ImageSensitivity')

    task_name = 'hifa_imageprecheck'

    for result in results:
        pipeline_casa_task = result.pipeline_casa_task
        if pipeline_casa_task.startswith(task_name + '('):
            stage_xml = xml_for_sensitivity_stage(context, result, task_name)
            xml_root.append(stage_xml)

    return xml_root


def xml_for_sensitivity_stage(context, stage_results, origin):
    xml_root = ElementTree.Element('SensitivityEstimates', Origin=origin, Score=UNDEFINED)

    for result in stage_results:
        for d in result.sensitivities:
            ms_xml = xml_for_sensitivity(d)
            xml_root.append(ms_xml)

    return xml_root


def xml_for_sensitivity(d):
    qa = casatools.quanta

    def value(quanta):
        return str(qa.getvalue(quanta)[0])

    bandwidth = qa.quantity(d['bandwidth'])
    bandwidth_hz = value(qa.convert(bandwidth, 'Hz'))

    major = qa.quantity(d['beam']['major'])
    major_arcsec = value(qa.convert(major, 'arcsec'))

    minor = qa.quantity(d['beam']['minor'])
    minor_arcsec = value(qa.convert(minor, 'arcsec'))

    cell_major = qa.quantity(d['cell'][0])
    cell_major_arcsec = value(qa.convert(cell_major, 'arcsec'))

    cell_minor = qa.quantity(d['cell'][1])
    cell_minor_arcsec = value(qa.convert(cell_minor, 'arcsec'))

    positionangle = qa.quantity(d['beam']['positionangle'])
    positionangle_deg = value(qa.convert(positionangle, 'deg'))

    sensitivity = qa.quantity(d['sensitivity'])
    sensitivity_jy_per_beam = value(qa.convert(sensitivity, 'Jy/beam'))

    xml = ElementTree.Element('Sensitivity',
        Array=d['array'],
        BandwidthHz=bandwidth_hz,
        BeamMajArcsec=major_arcsec,
        BeamMinArcsec=minor_arcsec,
        BeamPosAngDeg=positionangle_deg,
        BwMode=d['bwmode'],
        CellXArcsec=cell_major_arcsec,
        CellYArcsec=cell_minor_arcsec,
        Field=d['field'],
        Robust=str(d.get('robust', '')),
        UVTaper=str(d.get('uvtaper', '')),
        SensitivityJyPerBeam=sensitivity_jy_per_beam,
        MsSpwId=d['spw'],
      )

    return xml
