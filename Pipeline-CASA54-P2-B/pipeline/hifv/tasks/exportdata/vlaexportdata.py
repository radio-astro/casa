from __future__ import absolute_import

import os
import shutil

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.vdp as vdp
from pipeline.h.tasks.exportdata import exportdata
from pipeline.infrastructure import task_registry
from . import vlaifaqua

LOG = infrastructure.get_logger(__name__)


class VLAExportDataInputs(exportdata.ExportDataInputs):
    gainmap = vdp.VisDependentProperty(default=False)

    def __init__(self, context, output_dir=None, session=None, vis=None, exportmses=None, pprfile=None, calintents=None,
                 calimages=None, targetimages=None, products_dir=None, gainmap=None):
        super(VLAExportDataInputs, self).__init__(context, output_dir=output_dir, session=session, vis=vis,
                                                  exportmses=exportmses, pprfile=pprfile, calintents=calintents,
                                                  calimages=calimages, targetimages=targetimages,
                                                  products_dir=products_dir)
        self.gainmap = gainmap


@task_registry.set_equivalent_casa_task('hifv_exportdata')
class VLAExportData(exportdata.ExportData):

    # link the accompanying inputs to this task
    Inputs = VLAExportDataInputs

    def prepare(self):

        results = super(VLAExportData, self).prepare()

        # Export the AQUA report
        oussid = self.get_oussid(self.inputs.context)  # returns string of 'unknown' for VLA
        aquareport_name = 'pipeline_aquareport.xml'
        pipe_aqua_reportfile = self._export_aqua_report(self.inputs.context, oussid, aquareport_name,
                                                        self.inputs.products_dir)

        return results

    def _export_casa_restore_script(self, context, script_name, products_dir, oussid, vislist, session_list):
        """
        Save the CASA restore script.
        """

        # Generate the file list

        # Get the output file name
        ps = context.project_structure
        if ps is None:
            script_file = os.path.join(context.report_dir, script_name)
            out_script_file = os.path.join(products_dir, script_name)
        elif ps.ousstatus_entity_id == 'unknown':
            script_file = os.path.join(context.report_dir, script_name)
            out_script_file = os.path.join(products_dir, script_name)
        else:
            # ousid = ps.ousstatus_entity_id.translate(string.maketrans(':/', '__'))
            script_file = os.path.join(context.report_dir, script_name)
            out_script_file = os.path.join(products_dir, oussid + '.' + script_name)

        LOG.info('Creating casa restore script %s' % (script_file))

        # This is hardcoded.
        tmpvislist = []

        # VLA ocorr_value
        ocorr_mode = 'co'

        for vis in vislist:
            filename = os.path.basename(vis)
            if filename.endswith('.ms'):
                filename, filext = os.path.splitext(filename)
            tmpvislist.append(filename)
        task_string = "    hifv_restoredata (vis=%s, session=%s, ocorr_mode='%s', gainmap=%s)" % (
        tmpvislist, session_list, ocorr_mode, self.inputs.gainmap)

        task_string += '\n    hifv_statwt()'

        template = '''__rethrow_casa_exceptions = True
h_init()
try:
%s
finally:
    h_save()
''' % task_string

        with open(script_file, 'w') as casa_restore_file:
            casa_restore_file.write(template)

        LOG.info('Copying casa restore script %s to %s' % \
                 (script_file, out_script_file))
        if not self._executor._dry_run:
            shutil.copy(script_file, out_script_file)

        return os.path.basename(out_script_file)

    def _export_aqua_report (self, context, oussid, aquareport_name, products_dir):
        """
        Save the AQUA report.
        """
        aqua_file = os.path.join(context.output_dir, aquareport_name)

        report_generator = vlaifaqua.VLAAquaXmlGenerator()
        LOG.info('Generating pipeline AQUA report')
        try:
            report_xml = report_generator.get_report_xml(context)
            vlaifaqua.export_to_disk(report_xml, aqua_file)
        except:
            LOG.error('Error generating the pipeline AQUA report')
            return 'Undefined'

        ps = context.project_structure
        if ps is None:
            out_aqua_file = os.path.join(products_dir, aquareport_name)
        elif ps.ousstatus_entity_id == 'unknown':
            out_aqua_file = os.path.join(products_dir, aquareport_name)
        else:
            out_aqua_file = os.path.join(products_dir, oussid + '.' + aquareport_name)

        LOG.info('Copying AQUA report %s to %s' % (aqua_file, out_aqua_file))
        shutil.copy(aqua_file, out_aqua_file)
        return os.path.basename(out_aqua_file)
