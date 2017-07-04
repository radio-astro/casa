"""
The exportdata for SD module provides base classes for preparing data products
on disk for upload to the archive. 

To test these classes, register some data with the pipeline using ImportData,
then execute:

import pipeline
# Create a pipeline context and register some data
context = pipeline.Pipeline().context
output_dir = "."
products_dir = "./products"
inputs = pipeline.tasks.singledish.SDExportData.Inputs(context,output_dir,products_dir)
task = pipeline.tasks.singledish.SDExportData (inputs)
results = task.execute (dry_run = True)
results = task.execute (dry_run = False)

"""
from __future__ import absolute_import
import os

import tarfile
import types
import string
import collections
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.sdfilenamer as filenamer
#import pipeline.hif.tasks.exportdata.exportdata as hif_exportdata
import pipeline.h.tasks.exportdata.exportdata as h_exportdata
import pipeline.hifa.tasks.exportdata.almaexportdata as almaexportdata
#from . import manifest

# the logger for this module
LOG = infrastructure.get_logger(__name__)


# Inputs class must be separated per task class even if it's effectively the same
class SDMSExportDataInputs(h_exportdata.ExportDataInputs):
    pass


class SDMSExportData(almaexportdata.ALMAExportData):
    """
    SDMSExportData is the base class for exporting data to the products
    subdirectory. It performs the following operations:
    
    - Saves the pipeline processing request in an XML file
    - Saves the images in FITS cubes one per target and spectral window
    - Saves the final flags and bl coefficient per ASDM in a compressed / tarred CASA flag
      versions file
    - Saves the final web log in a compressed / tarred file
    - Saves the text formatted list of contents of products directory
    """
    Inputs = SDMSExportDataInputs
    
#     def _export_final_calfiles(self, context, oussid, session, vislist, products_dir, imaging=None):
#         """
#         Save the final calibration tables in a tarfile one file
#         per session.
#         
#         This method is an exact copy of same method in superclass except 
#         for handling baseline caltables.
#         """
# 
#         # Save the current working directory and move to the pipeline
#         # working directory. This is required for tarfile IO
#         cwd = os.getcwd()
#         try:
#             os.chdir(context.output_dir)
# 
#             # Define the name of the output tarfile
#             tarfilename = '{}.{}.caltables.tgz'.format(oussid, session)
#             LOG.info('Saving final caltables for %s in %s', session, tarfilename)
# 
#             # Create the tar file
#             if self._executor._dry_run:
#                 return tarfilename
# 
#             caltables = set()
#             
#             bl_caltables = set()
# 
#             for visfile in vislist:
#                 LOG.info('Collecting final caltables for %s in %s',
#                          os.path.basename(visfile), tarfilename)
# 
#                 # Create the list of applied caltables for that vis
#                 try:
#                     calto = callibrary.CalTo(vis=visfile)
#                     calstate = context.callibrary.applied.trimmed(context, calto)
#                     caltables.update(calstate.get_caltable())
#                 except:
#                     LOG.info('No caltables for MS %s' % os.path.basename(visfile))
#                     
#                 # Create the list of baseline caltable for that vis
#                 namer = filenamer.CalibrationTable()
#                 namer.asdm(os.path.basename(visfile))
#                 namer.bl_cal()
#                 name = namer.get_filename()
#                 LOG.debug('bl cal table for %s is %s'%(visfile, name))
#                 if os.path.exists(name):
#                     bl_caltables.add(name)
# 
#             if not caltables:
#                 return 'Undefined'
# 
#             with tarfile.open(os.path.join(products_dir, tarfilename), 'w:gz') as tar:
#                 # Tar the session list.
#                 for table in caltables:
#                     tar.add(table, arcname=os.path.basename(table))
#                     
#                 for table in bl_caltables:
#                     tar.add(table, arcname=os.path.basename(table))
# 
#             return tarfilename
# 
#         finally:
#             # Restore the original current working directory
#             os.chdir(cwd)

    # This is almost equivalent to ALMAExportData.prepare() 
    # only difference is to use self._make_lists instead of ExportData._make_lists
    def prepare(self):

        results = h_exportdata.ExportData.prepare(self)

        oussid = self.get_oussid(self.inputs.context)

        # Make the imaging vislist and the sessions lists.
        session_list, session_names, session_vislists, vislist = self._make_lists(self.inputs.context,
            self.inputs.session, self.inputs.vis, imaging=True)
        
        LOG.info('vislist={}'.format(vislist))

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

    def _make_lists (self, context, session, vis, imaging=False):

        '''
        Create the vis and sessions lists
        '''
        LOG.info('Single dish specific _make_lists')
        # Force inputs.vis to be a list.
        vislist = vis
        if type(vislist) is types.StringType:
            vislist = [vislist,]
        # in single dish pipeline, all mses are non-imaging ones but they need to be 
        # returned even when imaging is False so no filtering is done

        # Get the session list and the visibility files associated with
        # each session.
        session_list, session_names, session_vislists= self._get_sessions ( \
            context, session, vislist)

        return session_list, session_names, session_vislists, vislist

    def _do_aux_session_products (self, context, oussid, session_names, session_vislists, output_dir, products_dir):

        # Make the standard sessions dictionary and export per session products
        #    Currently these are compressed tar files of per session calibration tables
        # Export tar files of the calibration tables one per session
        LOG.info('_do_aux_session_products')
        caltable_file_list = []
        for i in range(len(session_names)):
            caltable_file = self._export_final_baseline_calfiles (context, oussid,
                session_names[i], session_vislists[i], products_dir)
            caltable_file_list.append (caltable_file)

        # Create the ordered session dictionary
        #    The keys are the session names
        #    The values are a tuple containing the vislist and the caltables
        sessiondict = collections.OrderedDict()
        for i in range(len(session_names)):
            sessiondict[session_names[i]] = \
                ([os.path.basename(visfile) for visfile in session_vislists[i]], \
                 os.path.basename(caltable_file_list[i]))

        return sessiondict

    def _export_final_baseline_calfiles(self, context, oussid, session, vislist, products_dir):
        """
        Save the final calibration tables in a tarfile one file
        per session.
        
        This method is an exact copy of same method in superclass except 
        for handling baseline caltables.
        """

        # Save the current working directory and move to the pipeline
        # working directory. This is required for tarfile IO
        cwd = os.getcwd()
        try:
            os.chdir(context.output_dir)

            # Define the name of the output tarfile
            tarfilename = '{}.{}.auxcaltables.tgz'.format(oussid, session)
            #tarfilename = '{}.{}.caltables.tgz'.format(oussid, session)
            LOG.info('Saving final caltables for %s in %s', session, tarfilename)

            # Create the tar file
            if self._executor._dry_run:
                return tarfilename

#             caltables = set()
            
            bl_caltables = set()

            for visfile in vislist:
                LOG.info('Collecting final caltables for %s in %s',
                         os.path.basename(visfile), tarfilename)

                # Create the list of applied caltables for that vis
#                 try:
#                     calto = callibrary.CalTo(vis=visfile)
#                     calstate = context.callibrary.applied.trimmed(context, calto)
#                     caltables.update(calstate.get_caltable())
#                 except:
#                     LOG.info('No caltables for MS %s' % os.path.basename(visfile))
                    
                # Create the list of baseline caltable for that vis
                namer = filenamer.CalibrationTable()
                namer.asdm(os.path.basename(visfile))
                namer.bl_cal()
                name = namer.get_filename()
                LOG.debug('bl cal table for %s is %s'%(visfile, name))
                if os.path.exists(name):
                    bl_caltables.add(name)

#             if not caltables:
#                 return 'Undefined'

            with tarfile.open(os.path.join(products_dir, tarfilename), 'w:gz') as tar:
                # Tar the session list.
#                 for table in caltables:
#                     tar.add(table, arcname=os.path.basename(table))
                    
                for table in bl_caltables:
                    tar.add(table, arcname=os.path.basename(table))

            return tarfilename

        finally:
            # Restore the original current working directory
            os.chdir(cwd)

    def _do_aux_ms_products (self, context, vislist, products_dir):
   
        # Loop over the measurements sets in the working directory, and
        # create the calibration apply file(s) in the products directory.
        apply_file_list = []
        for visfile in vislist:
            apply_file =  self._export_final_baseline_applylist (context, visfile, products_dir)
            apply_file_list.append (apply_file)

        # Create the ordered vis dictionary
        #    The keys are the base vis names
        #    The values are a tuple containing the flags and applycal files
        visdict = collections.OrderedDict()
        for i in range(len(vislist)):
            visdict[os.path.basename(vislist[i])] = \
                os.path.basename(apply_file_list[i])

        return visdict

    def _export_final_baseline_applylist (self, context, vis, products_dir):
        """
        Save the final calibration list to a file. For now this is
        a text file. Eventually it will be the CASA callibrary file.
        """

        applyfile_name = os.path.basename(vis) + '.auxcalapply.txt'
        LOG.info('Storing calibration apply list for %s in  %s',
                 os.path.basename(vis), applyfile_name)

        if self._executor._dry_run:
            return applyfile_name

        try:
            # Log the list in human readable form. Better way to do this ?
            cmd = string.Template("sdbaseline(infile='${infile}', datacolumn='corrected', blmode='apply', bltable='${bltable}', blfunc='poly', outfile='${outfile}', overwrite=True)")

            # Create the list of baseline caltable for that vis
            namer = filenamer.CalibrationTable()
            namer.asdm(os.path.basename(vis))
            namer.bl_cal()
            name = namer.get_filename()
            LOG.debug('bl cal table for %s is %s'%(vis, name))
            if os.path.exists(name):
                applied_calstate = cmd.safe_substitute(infile=vis,
                                                       bltable=name,
                                                       outfile=vis.rstrip('/') + '_bl')

                # Open the file.
                with open(os.path.join(products_dir, applyfile_name), "w") as applyfile:
                    applyfile.write('# Apply file for %s\n' % (os.path.basename(vis)))
                    applyfile.write(applied_calstate)
        except:
            applyfile_name = 'Undefined'
            LOG.info('No calibrations for MS %s' % os.path.basename(vis)) 

        return applyfile_name

    def _detect_jyperk(self, context):
        reffile_list = set(self.__get_reffile(context.results))
        
        if len(reffile_list) == 0:
            # if no reffile is found, return None
            LOG.debug('No K2Jy factor file found.')
            return None
        if len(reffile_list) > 1:
            raise RuntimeError("K2Jy conversion file must be only one. %s found."%(len(reffile_list)))
        
        jyperk = reffile_list.pop()
        
        if not os.path.exists(jyperk):
            # if reffile doesn't exist, return None
            LOG.debug('K2Jy file \'%s\' not found'%(jyperk))
            return None
        
        LOG.info('Exporting {0} as a product'.format(jyperk))
        return os.path.abspath(jyperk)
    
    def __get_reffile(self, results):
        for proxy in results:
            result = proxy.read()
            if not isinstance(result, basetask.ResultsList):
                result = [result]
            for r in result:
                if str(r).find('SDK2JyCalResults') != -1:
                    if hasattr(r, 'reffile'):
                        reffile = r.reffile
                        if reffile is not None and os.path.exists(reffile):
                            yield reffile
                            
    def _do_auxiliary_products(self, context, oussid, output_dir, products_dir):
        cwd = os.getcwd()
        tarfilename = 'Undefined'
        jyperk = self._detect_jyperk(context)
        if jyperk is None:
            return None

        try:
            os.chdir(output_dir)

            # Define the name of the output tarfile
            tarfilename = '{}.auxproducts.tgz'.format(oussid)
            LOG.info('Saving auxliary dat products in %s', tarfilename)

            # Open tarfile
            with tarfile.open(os.path.join(products_dir, tarfilename), 'w:gz') as tar:

                # Save flux file
                if os.path.exists(jyperk):
                    tar.add(jyperk, arcname=os.path.basename(jyperk))
                    LOG.info('Saving auxiliary data product %s in %s', os.path.basename(jyperk), tarfilename)
                else:
                    LOG.info('Auxiliary data product %s does not exist', os.path.basename(jyperk))

                tar.close()
        finally:
            # Restore the original current working directory
            os.chdir(cwd)

        return tarfilename
    
#     def _add_jyperk_to_manifest(self, manifest_file, jyperk):
#         pipemanifest = manifest.SingleDishPipelineManifest('')
#         pipemanifest.import_xml(manifest_file)
#         ous = pipemanifest.get_ous()
#         pipemanifest.add_jyperk(ous, os.path.basename(jyperk))
#         pipemanifest.write(manifest_file)
        
#     def _save_final_flagversion (self, vis, flag_version_name):
# 
#         """
#         Save the final flags to a final flag version.
#         Save flags for baseline-subtracted MS as well as calibrated (original) MS.
#         """
#         super(SDMSExportData, self)._save_final_flagversion(vis, flag_version_name)
#         ms = self.inputs.context.observing_run.get_ms(vis)
#         work_data = ms.work_data
#         if (work_data != vis) and (os.path.exists(work_data)):
#             super(SDMSExportData, self)._save_final_flagversion(work_data, flag_version_name)
# 
#     def _export_final_flagversion (self, context, vis, flag_version_name,
#                                    products_dir):
# 
#         """
#         Save the final flags version to a compressed tarfile in products.
#         Include flags for baseline-subtracted MS as well as calibrated (original) MS.
#         """
# 
#         # Save the current working directory and move to the pipeline
#         # working directory. This is required for tarfile IO
#         cwd = os.getcwd()
#         os.chdir (context.output_dir)
# 
#         ms = self.inputs.context.observing_run.get_ms(vis)
#         work_data = ms.work_data
#         if (work_data != vis) and (os.path.exists(work_data)):
#             vislist = [vis, work_data]
#         else:
#             vislist = [vis]
#             
# 
#         flagsname_list = []
#         ti_list = []
#         tarfilename = os.path.basename(vis) + '.flagversions.tgz'
#         for _vis in vislist:
#             # Define the name of the output tarfile
#             visname = os.path.basename(_vis)
#             LOG.info('Storing final flags for %s in %s' % (visname, tarfilename))
#     
#             # Define the directory to be saved
#             flagsname = os.path.join (visname + '.flagversions',
#                                       'flags.' + flag_version_name)
#             LOG.info('Saving flag version %s' % (flag_version_name))
#     
#             # Define the versions list file to be saved
#             flag_version_list = os.path.join (visname + '.flagversions',
#                                               'FLAG_VERSION_LIST')
#             ti = tarfile.TarInfo(flag_version_list)
#             #line = "Pipeline_Final : Final pipeline flags\n"
#             line = "%s : Final pipeline flags\n" % flag_version_name
#             ti.size = len (line)
#             LOG.info('Saving flag version list')
#             
#             flagsname_list.append(flagsname)
#             ti_list.append(ti)
# 
#         # Create the tar file
#         if not self._executor._dry_run:
#             tar = tarfile.open (os.path.join(products_dir, tarfilename), "w:gz")
#             for flagsname in flagsname_list:
#                 tar.add (flagsname)
#             for ti in ti_list:
#                 tar.addfile (ti, StringIO.StringIO(line))
#             tar.close()
# 
#         # Restore the original current working directory
#         os.chdir(cwd)
# 
#         return tarfilename
        