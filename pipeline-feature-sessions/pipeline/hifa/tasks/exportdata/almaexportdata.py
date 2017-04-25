from __future__ import absolute_import

import os
import shutil
import collections
import glob

from . import almaifaqua
from . import manifest

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

import pipeline.h.tasks.exportdata.exportdata as exportdata

LOG = infrastructure.get_logger(__name__)

AuxFileProducts = collections.namedtuple('AuxFileProducts', 'flux_file antenna_file cont_file flagtargets_list')

class ALMAExportDataInputs(exportdata.ExportDataInputs):

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, session=None, vis=None, exportmses=None,
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

        # Export the auxiliary file products
        #    TBD Move this routine to the ALMA interferometry pipeline
        #    These are optional for reprocessing but informative to the user
        #    The calibrator source fluxes file
        #    The antenna positions file
        #    The continuum regions file
        #    The target flagging file
        auxfproducts =  self._do_auxiliary_products(self.inputs.context, oussid, self.inputs.output_dir, self.inputs.products_dir)

        aquareport_name = 'pipeline_aquareport.xml'
        pipe_aqua_reportfile = self._export_aqua_report (self.inputs.context, oussid, aquareport_name,
            almaifaqua, self.inputs.products_dir)

        manifest = os.path.join(self.inputs.context.products_dir, results.manifest)
        if auxfproducts is not None or pipe_aqua_reportfile is not None:
            manifest = os.path.join(self.inputs.context.products_dir, results.manifest)
            self._add_to_manifest(manifest, auxfproducts, pipe_aqua_reportfile)

        return results

    def _do_auxiliary_products(self, context, oussid, output_dir, products_dir):

        '''
        Generate the auxliliary products
        '''

        # Export the flux.csv file
        #    Does not need to be exported to the archive because the information
        #    is encapsulated in the calibration tables
        #    Relies on file name convention
        flux_file = self._export_flux_file (context, oussid, 'flux.csv', products_dir)

        # Export the antennapos.csv file.
        #    Does not need to be exported to the archive because the information
        #    is encapsulated in the calibration tables
        #    Relies on file name convention
        antenna_file = self._export_antpos_file (context, oussid, 'antennapos.csv',
            products_dir)

        # Export the cont.dat file.
        #    May need to be exported to the archive. This is TBD
        #    Relies on file name convention
        cont_file = self._export_cont_file (context, oussid, 'cont.dat',
            products_dir)

        # Export the target source template flagging files
        #    Whether or not these should be exported to the archive depends on
        #    the final place of the target flagging step in the work flow
        targetflags_list = self._export_targetflags_files (context, oussid,
            '*_flagtargetstemplate.txt', products_dir)

        return AuxFileProducts (flux_file,
            antenna_file,
            cont_file,
            targetflags_list)

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

    def _export_flux_file (self, context, oussid, fluxfile_name, products_dir):

        """
        Save the flux file
        """

        ps = context.project_structure
        if ps is None:
            flux_file = os.path.join (context.output_dir, fluxfile_name)
            out_flux_file = os.path.join (products_dir, fluxfile_name)
        elif ps.ousstatus_entity_id == 'unknown':
            flux_file = os.path.join (context.output_dir, fluxfile_name)
            out_flux_file = os.path.join (products_dir, fluxfile_name)
        else:
            flux_file = os.path.join (context.output_dir, fluxfile_name)
            out_flux_file = os.path.join (products_dir, oussid + '.' + fluxfile_name)

        if os.path.exists(flux_file):
            LOG.info('Copying flux file %s to %s' % \
                     (flux_file, out_flux_file))
            shutil.copy (flux_file, out_flux_file)
            return os.path.basename(out_flux_file)
        else:
            return 'Undefined'

    def _export_antpos_file (self, context, oussid, antposfile_name, products_dir):

        """
        Save the antenna positions file
        """

        ps = context.project_structure
        if ps is None:
            antpos_file = os.path.join (context.output_dir, antposfile_name)
            out_antpos_file = os.path.join (products_dir, antposfile_name)
        elif ps.ousstatus_entity_id == 'unknown':
            antpos_file = os.path.join (context.output_dir, antposfile_name)
            out_antpos_file = os.path.join (products_dir, antposfile_name)
        else:
            antpos_file = os.path.join (context.output_dir, antposfile_name)
            out_antpos_file = os.path.join (products_dir, oussid + '.' + antposfile_name)

        if os.path.exists(antpos_file):
            LOG.info('Copying antenna postions file %s to %s' % \
                     (antpos_file, out_antpos_file))
            shutil.copy (antpos_file, out_antpos_file)
            return os.path.basename(out_antpos_file)
        else:
            return 'Undefined'

    def _export_cont_file (self, context, oussid, contfile_name, products_dir):

        """
        Save the continuum regions file
        """

        ps = context.project_structure
        if ps is None:
            cont_file = os.path.join (context.output_dir, contfile_name)
            out_cont_file = os.path.join (products_dir, contfile_name)
        elif ps.ousstatus_entity_id == 'unknown':
            cont_file = os.path.join (context.output_dir, contfile_name)
            out_cont_file = os.path.join (products_dir, contfile_name)
        else:
            cont_file = os.path.join (context.output_dir, contfile_name)
            out_cont_file = os.path.join (products_dir, oussid + '.' + contfile_name)

        if os.path.exists(cont_file):
            LOG.info('Copying continuum frequency ranges file %s to %s' % \
                     (cont_file, out_cont_file))
            shutil.copy (cont_file, out_cont_file)
            return os.path.basename(out_cont_file)
        else:
            return 'Undefined'

    def _export_targetflags_files (self, context, oussid, pattern, products_dir):

        """
        Export the target flags files
        Remove file name dependency on oussid but leave argument in place for now
        """

        output_filelist = []
        ps = context.project_structure
        for file_name in glob.glob(pattern):
            flags_file = os.path.join (context.output_dir, file_name)
            out_flags_file = os.path.join (products_dir, file_name)
            if os.path.exists(flags_file):
                LOG.info('Copying science target flags file %s to %s' % \
                     (flags_file, out_flags_file))
                shutil.copy (flags_file, out_flags_file)
                output_filelist.append(os.path.basename(out_flags_file))
            else:
                output_filelist.append('Undefined')

        return output_filelist

    def _add_to_manifest(self, manifest_file, auxfproducts, aqua_report):

        pipemanifest = manifest.ALMAIfPipelineManifest('')
        pipemanifest.import_xml(manifest_file)
        ouss = pipemanifest.get_ous()

        if aqua_report:
            pipemanifest.add_aqua_report(ouss, os.path.basename(aqua_report))

        if auxfproducts:
            # Add the flux.csv file
            pipemanifest.add_flux_file (ouss, os.path.basename(auxfproducts.flux_file))

            # Add the antennapos.csv file.
            pipemanifest.add_antennapos_file (ouss, os.path.basename(auxfproducts.antenna_file))

            # Add the cont.dat file.
            #    May need to be exported to the archive. This is TBD
            #    Relies on file name convention
            pipemanifest.add_cont_file (ouss, os.path.basename(auxfproducts.cont_file))


        pipemanifest.write(manifest_file)
