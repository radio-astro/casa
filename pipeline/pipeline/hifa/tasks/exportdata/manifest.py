from __future__ import absolute_import
import xml.etree.cElementTree as eltree
from xml.dom import minidom

import pipeline.h.tasks.exportdata.manifest as manifest

class ALMAIfPipelineManifest(manifest.PipelineManifest):
    def __init__(self, ouss_id):
        super(ALMAIfPipelineManifest, self).__init__(ouss_id)

    def import_xml(self, xmlfile):
        with open(xmlfile, 'r') as f:
            lines = map(lambda x: x.replace('\n', '').strip(), f.readlines())
            self.piperesults = eltree.fromstring(''.join(lines))

    def get_ous(self):
        return self.piperesults.getchildren()[0]

    def add_aqua_report(self, ous, aqua_report):
        """
         Add the AQUA report to the OUS element
        """
        eltree.SubElement (ous, "aqua_report", name=aqua_report)

