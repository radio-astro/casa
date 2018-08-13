from __future__ import absolute_import
import xml.etree.cElementTree as eltree
from xml.dom import minidom

#import pipeline.hif.tasks.exportdata.manifest as manifest
#import pipeline.h.tasks.exportdata.manifest as manifest
import pipeline.h.tasks.common.manifest as manifest

class SingleDishPipelineManifest(manifest.PipelineManifest):
    def __init__(self, ouss_id):
        super(SingleDishPipelineManifest, self).__init__(ouss_id)
        
    def import_xml(self, xmlfile):
        with open(xmlfile, 'r') as f:
            lines = map(lambda x: x.replace('\n', '').strip(), f.readlines())
            self.piperesults = eltree.fromstring(''.join(lines))
            
    def get_ous(self):
        return self.piperesults.getchildren()[0]
        
    def add_jyperk(self, ous, jyperkfile):
        """
         Add the Jy/K factors file to the OUS element
        """
        eltree.SubElement (ous, "jyperk", name=jyperkfile)
        
