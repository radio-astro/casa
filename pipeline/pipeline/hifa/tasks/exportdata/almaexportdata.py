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

        # Make the imaging vislist and the sessions lists.
        session_list, session_names, session_vislists, vislist = super(ALMAExportData, self)._make_lists(self.inputs.context,
            self.inputs.session, self.inputs.vis, imaging=True)

        if vislist:
            # Export the auxiliary caltables if any
            #    These are currently the uvcontinuum fit tables.
            auxcaltables = self._do_aux_session_products(self.inputs.context, oussid, session_names, session_vislists,
                self.inputs.output_dir, self.inputs.products_dir)

            # Export the auxiliary cal apply files if any
            #    These are currently the uvcontinuum fit tables.
            auxcalapplys = self._do_aux_ms_products(self.inputs.context, vislist, self.inputs.products_dir)
        else:
            auxcaltables = None
            auxcalapplys = None

        # Export the auxiliary file products into a single tar file
        #    These are optional for reprocessing but informative to the user
        #    The calibrator source fluxes file
        #    The antenna positions file
        #    The continuum regions file
        #    The target flagging file
        auxfproducts =  self._do_auxiliary_products(self.inputs.context, oussid, self.inputs.output_dir, self.inputs.products_dir)

        # Export the AQUA report
        aquareport_name = 'pipeline_aquareport.xml'
        pipe_aqua_reportfile = self._export_aqua_report(self.inputs.context, oussid, aquareport_name,
                                                        self.inputs.products_dir)

        # Update the manifest
        manifest = os.path.join(self.inputs.context.products_dir, results.manifest)
        if auxfproducts is not None or pipe_aqua_reportfile is not None:
            manifest = os.path.join(self.inputs.context.products_dir, results.manifest)
            self._add_to_manifest(manifest, auxfproducts, auxcaltables, auxcalapplys, pipe_aqua_reportfile)

        return results

    def _do_aux_session_products (self, context, oussid, session_names, session_vislists, output_dir, products_dir):

        # Make the standard sessions dictionary and export per session products
        #    Currently these are compressed tar files of per session calibration tables
        sessiondict = super(ALMAExportData, self)._do_standard_session_products (context, oussid, session_names,
            session_vislists, products_dir, imaging=True)

        return sessiondict

    def _do_aux_ms_products (self, context, vislist, products_dir):
   
        # Loop over the measurements sets in the working directory, and
        # create the calibration apply file(s) in the products directory.
        apply_file_list = []
        for visfile in vislist:
            apply_file =  super(ALMAExportData, self)._export_final_applylist (context, \
                visfile, products_dir, imaging=True)
            apply_file_list.append (apply_file)

        # Create the ordered vis dictionary
        #    The keys are the base vis names
        #    The values are a tuple containing the flags and applycal files
        visdict = collections.OrderedDict()
        for i in range(len(vislist)):
            visdict[os.path.basename(vislist[i])] = \
                os.path.basename(apply_file_list[i])

        return visdict

    def _do_auxiliary_products(self, context, oussid, output_dir, products_dir):

        '''
        Generate the auxliliary products
        '''

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

        # Export the general and target source template flagging files
        #    The general template flagging files are nnot required for the restore but are
        #    informative to the user
        #    Whether or not the target template files  should be exported to the archive depends
        #    on the final place of the target flagging step in the work flow and
        #    how flags will or will not be stored back into the ASDM

        targetflags_filelist = []
        for file_name in glob.glob('*_flag*template.txt'):
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
                    if os.path.exists(flags_file):
                        tar.add(flags_file, arcname=os.path.basename(flags_file))
                        LOG.info('Saving auxiliary data product %s in %s', os.path.basename(flags_file), tarfilename)
                    else:
                        LOG.info('Auxiliary data product %s does not exist', os.path.basename(flags_file))

                tar.close()
        finally:
            # Restore the original current working directory
            os.chdir(cwd)

        return tarfilename

    def _export_aqua_report(self, context, oussid, aquareport_name, products_dir):
        """
        Save the AQUA report.
        """
        aqua_file = os.path.join(context.output_dir, aquareport_name)

        report_generator = almaifaqua.AlmaAquaXmlGenerator()
        LOG.info('Generating pipeline AQUA report')
        try:
            report_xml = report_generator.get_report_xml(context)
            almaifaqua.export_to_disk(report_xml, aqua_file)
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

    def _add_to_manifest(self, manifest_file, aux_fproducts, aux_caltablesdict, aux_calapplysdict, aqua_report):

        pipemanifest = manifest.ALMAIfPipelineManifest('')
        pipemanifest.import_xml(manifest_file)
        ouss = pipemanifest.get_ous()

        if aqua_report:
            pipemanifest.add_aqua_report(ouss, os.path.basename(aqua_report))

        if aux_fproducts:
            # Add auxliary data products file
            pipemanifest.add_aux_products_file (ouss, os.path.basename(aux_fproducts))

        # Add the auxiliary caltables
        if aux_caltablesdict:
            for session_name in aux_caltablesdict:
                session = pipemanifest.get_session(ouss, session_name)
                pipemanifest.add_auxcaltables(session, aux_caltablesdict[session_name][1])
                for vis_name in aux_caltablesdict[session_name][0]:
                    pipemanifest.add_auxasdm (session, vis_name, aux_calapplysdict[vis_name])

        pipemanifest.write(manifest_file)
