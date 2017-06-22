from __future__ import absolute_import

import os
import shutil
import collections
import glob
import tarfile

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

        # Export the auxiliary file products into a single tar file
        #    These are optional for reprocessing but informative to the user
        #    The calibrator source fluxes file
        #    The antenna positions file
        #    The continuum regions file
        #    The target flagging file
        auxfproducts =  self._do_auxiliary_products(self.inputs.context, oussid, self.inputs.output_dir, self.inputs.products_dir)

        # Export the AQUA report
        aquareport_name = 'pipeline_aquareport.xml'
        pipe_aqua_reportfile = self._export_aqua_report(self.inputs.context, oussid, aquareport_name, almaifaqua,
                                                        self.inputs.products_dir)

        # Update the manifest
        manifest = os.path.join(self.inputs.context.products_dir, results.manifest)
        if auxfproducts is not None or pipe_aqua_reportfile is not None:
            manifest = os.path.join(self.inputs.context.products_dir, results.manifest)
            self._add_to_manifest(manifest, auxfproducts, pipe_aqua_reportfile)

        return results

    def _do_auxiliary_products(self, context, oussid, output_dir, products_dir):
        """
        Generate the auxliliary products
        """

        fluxfile_name = 'flux.csv'
        antposfile_name = 'antennapos.csv'
        contfile_name = 'cont.dat'
        empty = True

        # Get the flux, antenna position, and continuum subtraction
        # files and test to see if at least one of them exists
        flux_file = os.path.join (output_dir, fluxfile_name)
        antpos_file = os.path.join (output_dir, antposfile_name)
        cont_file = os.path.join (output_dir, contfile_name)
        if os.path.exists(flux_file) or os.path.exists(antpos_file) or os.path_exists(cont_file):
            empty = False

        # Export the target source template flagging files
        #    Whether or not these should be exported to the archive depends on
        #    the final place of the target flagging step in the work flow and
        #    how flags will or will not be stored back into the ASDM

        targetflags_filelist = []
        for file_name in glob.glob('*_flagtargetstemplate.txt'):
            flags_file = os.path.join (output_dir, file_name)
            if os.path.exists(flags_file):
                empty = False
                targetflags_filelist.append(flags_file)
            else:
                targetflags_filelist.append('Undefined')

        if empty:
            return None

        # Create the tarfile
        cwd = os.getcwd()
        tarfilename = 'Undefined'
        try:
            os.chdir(output_dir)

            # Define the name of the output tarfile
            tarfilename = '{}.auxproducts.tgz'.format(oussid)
            LOG.info('Saving auxliary dat products in %s', tarfilename)

            # Open tarfile
            with tarfile.open(os.path.join(products_dir, tarfilename), 'w:gz') as tar:

                # Save flux file
                if os.path.exists(flux_file):
                    tar.add(flux_file, arcname=os.path.basename(flux_file))
                    LOG.info('Saving auxiliary data product %s in %s', os.path.basename(flux_file), tarfilename)
                else:
                    LOG.info('Auxiliary data product %s does not exist', os.path.basename(flux_file))
                    flux_file = 'Undefined'

                # Save antenna positions file
                if os.path.exists(antpos_file):
                    tar.add(antpos_file, arcname=os.path.basename(antpos_file))
                    LOG.info('Saving auxiliary data product %s in %s', os.path.basename(antpos_file), tarfilename)
                else:
                    LOG.info('Auxiliary data product %s does not exist', os.path.basename(antpos_file))
                    antpos_file = 'Undefined'

                # Save continuum regions file
                if os.path.exists(cont_file):
                    tar.add(cont_file, arcname=os.path.basename(cont_file))
                    LOG.info('Saving auxiliary data product %s in %s', os.path.basename(cont_file), tarfilename)
                else:
                    LOG.info('Auxiliary data product %s does not exist', os.path.basename(cont_file))
                    cont_file = 'Undefined'

                # Save target flag files
                for flags_file in targetflags_filelist:
                    if os.path.exists(cont_file):
                        tar.add(flags_file, arcname=os.path.basename(flags_file))
                        LOG.info('Saving auxiliary data product %s in %s', os.path.basename(flags_file), tarfilename)
                    else:
                        LOG.info('Auxiliary data product %s does not exist', os.path.basename(flags_file))

                tar.close()
        finally:
            # Restore the original current working directory
            os.chdir(cwd)

        return tarfilename

    def _export_aqua_report(self, context, oussid, aquareport_name, aqua, products_dir):
        """
        Save the AQUA report.
        """
        aqua_report_generator = aqua.AlmaAquaXmlGenerator()

        LOG.info('Generating pipeline AQUA report')
        try:
            aqua_xml = aqua_report_generator.get_report_xml(context)
            aqua.export_to_disk(aqua_xml, aquareport_name)
        except:
            LOG.error('Error generating the pipeline AQUA report')
        finally:
            aqua_file = os.path.join(context.output_dir, aquareport_name)

            ps = context.project_structure
            if ps is None:
                out_aqua_file = os.path.join(products_dir, aquareport_name)
            elif ps.ousstatus_entity_id == 'unknown':
                out_aqua_file = os.path.join(products_dir, aquareport_name)
            else:
                out_aqua_file = os.path.join(products_dir, oussid + '.' + aquareport_name)

            if os.path.exists(aqua_file):
                LOG.info('Copying AQUA report %s to %s' % (aqua_file, out_aqua_file))
                shutil.copy(aqua_file, out_aqua_file)
                return os.path.basename(out_aqua_file)
            else:
                return 'Undefined'

    def _add_to_manifest(self, manifest_file, auxfproducts, aqua_report):

        pipemanifest = manifest.ALMAIfPipelineManifest('')
        pipemanifest.import_xml(manifest_file)
        ouss = pipemanifest.get_ous()

        if aqua_report:
            pipemanifest.add_aqua_report(ouss, os.path.basename(aqua_report))

        if auxfproducts:
            # Add auxliary data products file
            pipemanifest.add_aux_products_file(ouss, os.path.basename(auxfproducts))

        pipemanifest.write(manifest_file)
