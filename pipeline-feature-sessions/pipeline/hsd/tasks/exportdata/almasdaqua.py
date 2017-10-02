from __future__ import absolute_import

import xml.etree.cElementTree as ElementTree

import pipeline.h.tasks.exportdata.aqua as aqua


class AlmaAquaXmlGenerator(aqua.AquaXmlGenerator):
    """
    Class for creating the AQUA pipeline report
    Note __init__ and get_project_structure are copies of the counterpart in almaifaqua
    """
    def __init__(self):
        """
        Constructor
        """
        super(AlmaAquaXmlGenerator, self).__init__()

    def get_project_structure(self, context):
        # get base XML from base class
        root = super(AlmaAquaXmlGenerator, self).get_project_structure(context)

        # add our ALMA-specific elements
        ElementTree.SubElement(root, 'OusEntityId').text = context.project_structure.ous_entity_id
        ElementTree.SubElement(root, 'OusPartId').text = context.project_structure.ous_part_id
        ElementTree.SubElement(root, 'OusStatusEntityId').text = context.project_structure.ousstatus_entity_id

        return root

    def get_imaging_topic(self, context, topic_results):
        # get base XML from base class
        xml_root = super(AlmaAquaXmlGenerator, self).get_imaging_topic(context, topic_results)

        # add sensitivities
        sensitivity_xml = aqua.sensitivity_xml_for_stages(context, topic_results)
        # omit containing element if no measurements were found
        if len(list(sensitivity_xml)) > 0:
            xml_root.extend(sensitivity_xml)

        return xml_root


def _hsd_imaging_sensitivity_exporter(stage_results):
    # XML exporter expects this function to return a list of dictionaries
    sensitivities = []
    for result in stage_results:
        if result.sensitivity is not None:
            sensitivities.append(result.sensitivity)
    return sensitivities

aqua.TASK_NAME_TO_SENSITIVITY_EXPORTER['hsd_imaging'] = _hsd_imaging_sensitivity_exporter
