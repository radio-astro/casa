from __future__ import absolute_import
import xml.etree.cElementTree as eltree
from xml.dom import minidom

import pipeline.hif.tasks.exportdata.manifest as manifest

class SingleDishPipelineManifest(manifest.PipelineManifest):
    def __init__(self, ouss_id):
        super(SingleDishPipelineManifest, self).__init__(ouss_id)
        
    def add_jyperk(self, ous, jyperkfile):
        """
         Add the Jy/K factors file to the OUS element
        """
        eltree.SubElement (ous, "jyperk", name=jyperkfile)
        