from __future__ import absolute_import

import collections
import fnmatch
import os
import shutil
import string
import tarfile
import types

from casa_system import casa as casasys

import pipeline as pipeline
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
from pipeline.h.tasks.common import manifest
from pipeline.h.tasks.exportdata import exportdata
from pipeline.infrastructure import casa_tasks
from pipeline.infrastructure import task_registry

LOG = infrastructure.get_logger(__name__)

StdFileProducts = collections.namedtuple('StdFileProducts', 'ppr_file weblog_file casa_commands_file casa_pipescript')


class ExportvlassdataResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[]):
        super(ExportvlassdataResults, self).__init__()
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

    def __repr__(self):
        return 'ExportvlassdataResults:'


class ExportvlassdataInputs(exportdata.ExportDataInputs):
    gainmap = vdp.VisDependentProperty(default=False)

    def __init__(self, context, output_dir=None, session=None, vis=None, exportmses=None, pprfile=None, calintents=None,
                 calimages=None, targetimages=None, products_dir=None, gainmap=None):
        super(ExportvlassdataInputs, self).__init__(context, output_dir=output_dir, session=session, vis=vis,
                                                    exportmses=exportmses, pprfile=pprfile, calintents=calintents,
                                                    calimages=calimages, targetimages=targetimages,
                                                    products_dir=products_dir)
        self.gainmap = gainmap


@task_registry.set_equivalent_casa_task('hifv_exportvlassdata')
class Exportvlassdata(basetask.StandardTaskTemplate):

    Inputs = ExportvlassdataInputs

    def prepare(self):

        LOG.info("This Exportvlassdata class is running.")

        # Create a local alias for inputs, so we're not saying
        # 'self.inputs' everywhere
        inputs = self.inputs

        # Initialize the standard ous is string.
        oussid = self.get_oussid(inputs.context)

        # Define the results object
        result = ExportvlassdataResults()

        # Make the standard vislist and the sessions lists.
        session_list, session_names, session_vislists, vislist = self._make_lists(inputs.context, inputs.session,
                                                                                  inputs.vis)

        # Export the standard per OUS file products
        #    The pipeline processing request
        #    A compressed tarfile of the weblog
        #    The pipeline processing script
        #    The CASA commands log
        recipe_name = self.get_recipename(inputs.context)
        if not recipe_name:
            prefix = oussid
        else:
            prefix = oussid + '.' + recipe_name
        stdfproducts = self._do_standard_ous_products(inputs.context, prefix, inputs.pprfile, inputs.output_dir,
                                                      inputs.products_dir)
        if stdfproducts.ppr_file:
            result.pprequest = os.path.basename(stdfproducts.ppr_file)
        result.weblog = os.path.basename(stdfproducts.weblog_file)
        result.pipescript = os.path.basename(stdfproducts.casa_pipescript)
        result.commandslog = os.path.basename(stdfproducts.casa_commands_file)

        imlist = self.inputs.context.subimlist.get_imlist()

        dname = inputs.products_dir + '/'
        for imageitem in imlist:

            if imageitem['multiterm']:
                pbcor_image_name = imageitem['imagename'].replace('subim', 'pbcor.tt0.subim')
                rms_image_name = imageitem['imagename'].replace('subim', 'pbcor.tt0.rms.subim')
            else:
                pbcor_image_name = imageitem['imagename'].replace('subim', 'pbcor.subim')
                rms_image_name = imageitem['imagename'].replace('subim', 'pbcor.rms.subim')

            fits_pbcor_image = dname + pbcor_image_name + '.fits'
            task = casa_tasks.exportfits(imagename=pbcor_image_name, fitsimage=fits_pbcor_image)
            self._executor.execute(task)
            LOG.info('Wrote {ff}'.format(ff=fits_pbcor_image))

            fits_rms_image = dname + rms_image_name + '.fits'
            task = casa_tasks.exportfits(imagename=rms_image_name, fitsimage=fits_rms_image)
            self._executor.execute(task)
            LOG.info('Wrote {ff}'.format(ff=fits_rms_image))

        # Export the pipeline manifest file
        #    TBD Remove support for auxiliary data products to the individual pipelines
        pipemanifest = self._make_pipe_manifest(inputs.context, oussid, stdfproducts, {}, {}, [],
                                                [os.path.basename(fits_pbcor_image), os.path.basename(fits_rms_image)])
        casa_pipe_manifest = self._export_pipe_manifest('pipeline_manifest.xml', inputs.products_dir, pipemanifest)
        result.manifest = os.path.basename(casa_pipe_manifest)

        # Return the results object, which will be used for the weblog
        return result

    def analyse(self, results):
        return results

    def get_oussid(self, context):

        """
        Determine the ous prefix
        """

        # Get the parent ous ousstatus name. This is the sanitized ous
        # status uid
        ps = context.project_structure
        if ps is None:
            oussid = 'unknown'
        elif ps.ousstatus_entity_id == 'unknown':
            oussid = 'unknown'
        else:
            oussid = ps.ousstatus_entity_id.translate(string.maketrans(':/', '__'))

        return oussid

    def get_recipename(self, context):
        """
        Get the recipe name
        """

        # Get the parent ous ousstatus name. This is the sanitized ous
        # status uid
        ps = context.project_structure
        if ps is None:
            recipe_name = ''
        elif ps.recipe_name == 'Undefined':
            recipe_name = ''
        else:
            recipe_name = ps.recipe_name

        return recipe_name

    def _make_lists(self, context, session, vis, imaging=False):
        """
        Create the vis and sessions lists
        """

        # Force inputs.vis to be a list.
        vislist = vis
        if type(vislist) is types.StringType:
            vislist = [vislist, ]
        if imaging:
            vislist = [vis for vis in vislist if context.observing_run.get_ms(name=vis).is_imaging_ms]
        else:
            vislist = [vis for vis in vislist if not context.observing_run.get_ms(name=vis).is_imaging_ms]

        # Get the session list and the visibility files associated with
        # each session.
        session_list, session_names, session_vislists = self._get_sessions(context, session, vislist)

        return session_list, session_names, session_vislists, vislist

    def _do_standard_ous_products(self, context, oussid, pprfile, output_dir, products_dir):
        """
        Generate the per ous standard products
        """

        # Locate and copy the pipeline processing request.
        #     There should normally be at most one pipeline processing request.
        #     In interactive mode there is no PPR.
        ppr_files = self._export_pprfile(context, output_dir, products_dir, oussid, pprfile)
        if ppr_files != []:
            ppr_file = os.path.basename(ppr_files[0])
        else:
            ppr_file = None

        # Export a tar file of the web log
        weblog_file = self._export_weblog(context, products_dir, oussid)

        # Export the processing log independently of the web log
        casa_commands_file = self._export_casa_commands_log(context,
                                                            context.logs['casa_commands'], products_dir, oussid)

        # Export the processing script independently of the web log
        casa_pipescript = self._export_casa_script(context,
                                                   context.logs['pipeline_script'], products_dir, oussid)

        return StdFileProducts(ppr_file,
                               weblog_file,
                               casa_commands_file,
                               casa_pipescript)

    def _make_pipe_manifest(self, context, oussid, stdfproducts, sessiondict, visdict, calimages, targetimages):
        """
        Generate the manifest file
        """

        # Initialize the manifest document and the top level ous status.
        pipemanifest = self._init_pipemanifest(oussid)
        ouss = pipemanifest.set_ous(oussid)
        pipemanifest.add_casa_version(ouss, casasys['build']['version'].strip())
        pipemanifest.add_pipeline_version(ouss, pipeline.revision)
        pipemanifest.add_procedure_name(ouss, context.project_structure.recipe_name)

        if stdfproducts.ppr_file:
            pipemanifest.add_pprfile(ouss, os.path.basename(stdfproducts.ppr_file))

        # Add the flagging and calibration products
        for session_name in sessiondict:
            session = pipemanifest.set_session(ouss, session_name)
            pipemanifest.add_caltables(session, sessiondict[session_name][1])
            for vis_name in sessiondict[session_name][0]:
                pipemanifest.add_asdm(session, vis_name, visdict[vis_name][0],
                                      visdict[vis_name][1])

        # Add a tar file of the web log
        pipemanifest.add_weblog(ouss, os.path.basename(stdfproducts.weblog_file))

        # Add the processing log independently of the web log
        pipemanifest.add_casa_cmdlog(ouss,
                                     os.path.basename(stdfproducts.casa_commands_file))

        # Add the processing script independently of the web log
        pipemanifest.add_pipescript(ouss, os.path.basename(stdfproducts.casa_pipescript))

        # Add the calibrator images
        pipemanifest.add_images(ouss, calimages, 'calibrator')

        # Add the target images
        pipemanifest.add_images(ouss, targetimages, 'target')

        return pipemanifest

    def _init_pipemanifest(self, oussid):
        """
        Initialize the pipeline manifest
        """

        pipemanifest = manifest.PipelineManifest(oussid)
        return pipemanifest

    def _export_pprfile(self, context, output_dir, products_dir, oussid, pprfile):

        # Prepare the search template for the pipeline processing request file.
        #    Was a template in the past
        #    Forced to one file now but keep the template structure for the moment
        if pprfile == '':
            ps = context.project_structure
            if ps is None:
                pprtemplate = None
            elif ps.ppr_file == '':
                pprtemplate = None
            else:
                pprtemplate = os.path.basename(ps.ppr_file)
        else:
            pprtemplate = os.path.basename(pprfile)

        # Locate the pipeline processing request(s) and  generate a list
        # to be copied to the data products directory. Normally there
        # should be only one match but if there are more copy them all.
        pprmatches = []
        if pprtemplate is not None:
            for file in os.listdir(output_dir):
                if fnmatch.fnmatch(file, pprtemplate):
                    LOG.debug('Located pipeline processing request %s' % (file))
                    pprmatches.append(os.path.join(output_dir, file))

        # Copy the pipeline processing request files.
        pprmatchesout = []
        for file in pprmatches:
            if oussid:
                outfile = os.path.join(products_dir, oussid + '.pprequest.xml')
            else:
                outfile = file
            pprmatchesout.append(outfile)
            LOG.info('Copying pipeline processing file %s to %s' % \
                     (os.path.basename(file), os.path.basename(outfile)))
            if not self._executor._dry_run:
                shutil.copy(file, outfile)

        return pprmatchesout

    def _get_sessions(self, context, sessions, vis):
        """
        Return a list of sessions where each element of the list contains
        the vis files associated with that session. In future this routine
        will be driven by the context but for now use the user defined sessions
        """

        # If the input session list is empty put all the visibility files
        # in the same session.
        if len(sessions) == 0:
            wksessions = []
            for visname in vis:
                session = context.observing_run.get_ms(name=visname).session
                wksessions.append(session)
        else:
            wksessions = sessions

        # Determine the number of unique sessions.
        session_seqno = 0;
        session_dict = {}
        for i in range(len(wksessions)):
            if wksessions[i] not in session_dict:
                session_dict[wksessions[i]] = session_seqno
                session_seqno = session_seqno + 1

        # Initialize the output session names and visibility file lists
        session_names = []
        session_vis_list = []
        for key, value in sorted(session_dict.iteritems(), key=lambda (k, v): (v, k)):
            session_names.append(key)
            session_vis_list.append([])

        # Assign the visibility files to the correct session
        for j in range(len(vis)):
            # Match the session names if possible
            if j < len(wksessions):
                for i in range(len(session_names)):
                    if wksessions[j] == session_names[i]:
                        session_vis_list[i].append(vis[j])
            # Assign to the last session
            else:
                session_vis_list[len(session_names) - 1].append(vis[j])

        # Log the sessions
        for i in range(len(session_vis_list)):
            LOG.info('Visibility list for session %s is %s' % \
                     (session_names[i], session_vis_list[i]))

        return wksessions, session_names, session_vis_list

    def _export_weblog(self, context, products_dir, oussid):
        """
        Save the processing web log to a tarfile
        """

        # Save the current working directory and move to the pipeline
        # working directory. This is required for tarfile IO
        cwd = os.getcwd()
        os.chdir(context.output_dir)

        # Define the name of the output tarfile
        ps = context.project_structure
        if ps is None:
            tarfilename = 'weblog.tgz'
        elif ps.ousstatus_entity_id == 'unknown':
            tarfilename = 'weblog.tgz'
        else:
            tarfilename = oussid + '.weblog.tgz'

        LOG.info('Saving final weblog in %s' % tarfilename)

        # Create the tar file
        if not self._executor._dry_run:
            tar = tarfile.open(os.path.join(products_dir, tarfilename), "w:gz")
            tar.add(os.path.join(os.path.basename(os.path.dirname(context.report_dir)), 'html'))
            tar.close()

        # Restore the original current working directory
        os.chdir(cwd)

        return tarfilename

    def _export_casa_commands_log(self, context, casalog_name, products_dir, oussid):
        """
        Save the CASA commands file.
        """

        ps = context.project_structure
        if ps is None:
            casalog_file = os.path.join(context.report_dir, casalog_name)
            out_casalog_file = os.path.join(products_dir, casalog_name)
        elif ps.ousstatus_entity_id == 'unknown':
            casalog_file = os.path.join(context.report_dir, casalog_name)
            out_casalog_file = os.path.join(products_dir, casalog_name)
        else:
            casalog_file = os.path.join(context.report_dir, casalog_name)
            out_casalog_file = os.path.join(products_dir, oussid + '.' + casalog_name)

        LOG.info('Copying casa commands log %s to %s' % \
                 (casalog_file, out_casalog_file))
        if not self._executor._dry_run:
            shutil.copy(casalog_file, out_casalog_file)

        return os.path.basename(out_casalog_file)

    def _export_casa_script(self, context, casascript_name, products_dir, oussid):
        """
        Save the CASA script.
        """

        ps = context.project_structure
        if ps is None:
            casascript_file = os.path.join(context.report_dir, casascript_name)
            out_casascript_file = os.path.join(products_dir, casascript_name)
        elif ps.ousstatus_entity_id == 'unknown':
            casascript_file = os.path.join(context.report_dir, casascript_name)
            out_casascript_file = os.path.join(products_dir, casascript_name)
        else:
            # ousid = ps.ousstatus_entity_id.translate(string.maketrans(':/', '__'))
            casascript_file = os.path.join(context.report_dir, casascript_name)
            out_casascript_file = os.path.join(products_dir, oussid + '.' + casascript_name)

        LOG.info('Copying casa script file %s to %s' % \
                 (casascript_file, out_casascript_file))
        if not self._executor._dry_run:
            shutil.copy(casascript_file, out_casascript_file)

        return os.path.basename(out_casascript_file)

    def _export_pipe_manifest(self, manifest_name, products_dir, pipemanifest):
        """
        Save the manifest file.
        """

        out_manifest_file = os.path.join(products_dir, manifest_name)
        LOG.info('Creating manifest file %s' % out_manifest_file)
        if not self._executor._dry_run:
            pipemanifest.write(out_manifest_file)

        return out_manifest_file
