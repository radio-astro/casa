from __future__ import absolute_import
import os
import shutil

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

from pipeline.h.tasks.exportdata import exportdata

LOG = infrastructure.get_logger(__name__)

class VLAExportDataInputs(exportdata.ExportDataInputs):

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, session=None, vis=None, exportmses=None,
                 pprfile=None, calintents=None, calimages=None, targetimages=None,
                 products_dir=None, gainmap=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

class VLAExportData(exportdata.ExportData):

    # link the accompanying inputs to this task
    Inputs = VLAExportDataInputs


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

