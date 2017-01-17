from __future__ import absolute_import

import os
import shutil

from . import almaifaqua
from . import manifest

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

import pipeline.h.tasks.exportdata.exportdata as exportdata

LOG = infrastructure.get_logger(__name__)

class ALMAExportDataInputs(exportdata.ExportDataInputs):

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, session=None, vis=None,
                 pprfile=None, calintents=None, calimages=None, targetimages=None,
                 products_dir=None ):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

class ALMAExportData(exportdata.ExportData):

    # link the accompanying inputs to this task
    Inputs = ALMAExportDataInputs
   
    def prepare(self):

        results = super(ALMAExportData, self).prepare()
        oussid = self.get_oussid(self.inputs.context)
        aquareport_name = 'pipeline_aquareport.xml'
        pipe_aqua_reportfile = self._export_aqua_report (self.inputs.context, oussid, aquareport_name,
            almaifaqua, self.inputs.products_dir)
        manifest = os.path.join(self.inputs.context.products_dir, results.manifest)
        if pipe_aqua_reportfile is not None:
            manifest = os.path.join(self.inputs.context.products_dir, results.manifest)
            self._add_aquareport_to_manifest(manifest, pipe_aqua_reportfile)

        return results

    def _export_aqua_report (self, context, oussid, aquareport_name, aqua, products_dir):

        """
        Save the AQUA report.
        """

        LOG.info ('Generating pipeline AQUA report')
        try:
            aqua.aquaReportFromContext (context, aquareport_name)
        except:
            LOG.error ('Error generating the pipeline AQUA report')
        finally:
            ps = context.project_structure
            if ps is None:
                aqua_file = os.path.join (context.output_dir, aquareport_name)
                out_aqua_file = os.path.join (products_dir, aquareport_name)
            elif ps.ousstatus_entity_id == 'unknown':
                aqua_file = os.path.join (context.output_dir, aquareport_name)
                out_aqua_file = os.path.join (products_dir, aquareport_name)
            else:
                aqua_file = os.path.join (context.output_dir, aquareport_name)
                out_aqua_file = os.path.join (products_dir, oussid + '.' + aquareport_name)
            if os.path.exists(aqua_file):
                LOG.info('Copying AQUA report %s to %s' % (aqua_file, out_aqua_file))
                shutil.copy (aqua_file, out_aqua_file)
                return os.path.basename(out_aqua_file)
            else:
                return 'Undefined'

    def _add_aquareport_to_manifest(self, manifest_file, aqua_report):
        pipemanifest = manifest.ALMAIfPipelineManifest('')
        pipemanifest.import_xml(manifest_file)
        ous = pipemanifest.get_ous()
        pipemanifest.add_aqua_report(ous, os.path.basename(aqua_report))
        pipemanifest.write(manifest_file)
