"""
The exportdata module provides base classes for preparing data products
on disk for upload to the archive. 

To test these classes, register some data with the pipeline using ImportData,
then execute:

import pipeline
vis = [ '<MS name>' ]

# Create a pipeline context and register some data
context = pipeline.Pipeline().context
inputs = pipeline.tasks.ImportData.Inputs(context, vis=vis)
task = pipeline.tasks.ImportData(inputs)
results = task.execute(dry_run=False)
results.accept(context)

# Run some other pipeline tasks, e.g flagging, calibration,
# and imaging in a similar manner

# Execute the export data task. The details of
# what gets exported depends on what tasks were run
# previously but may include the following
# TBD
inputs = pipeline.tasks.exportdata.Exportdata.Inputs(context,
      vis, output_dir, sessions, pprfile, products_dir)
task = pipeline.tasks.exportdata.ExportData (inputs)
  results = task.execute (dry_run = True)


"""
from __future__ import absolute_import
import os
import glob
import errno
import tarfile
import shutil
import fnmatch
import types
import StringIO
import copy
import string
import re
import collections

from casa_system import casa as casasys

import pipeline as pipeline
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.imagelibrary as imagelibrary

# the logger for this module
LOG = infrastructure.get_logger(__name__)

from .. common  import manifest

StdFileProducts = collections.namedtuple('StdFileProducts', 'ppr_file weblog_file casa_commands_file casa_pipescript casa_restore_script')


class ExportDataInputs(basetask.StandardInputs):
    """
    ExportDataInputs manages the inputs for the ExportData task.

    .. py:attribute:: context

    the (:class:`~pipeline.infrastructure.launcher.Context`) holding all
    pipeline state

    .. py:attribute:: output_dir

    the directory containing the output of the pipeline

    .. py:attribute:: session

    a string or list of strings containing the sessions(s) associated
    with each vis. Default to a single session containing all vis.
    Vis without a matching session are assigned to the last session
    in the list.

    .. py:attribute:: vis

    a string or list of strings containing the MS name(s) on which to
    operate

    .. py:attribute:: pprfile

    the pipeline processing request. 

    .. py:attribute:: calintents

    the list of calintents defining the calibrator source images to be
    saved.  Defaults to all calibrator intents.

    .. py:attribute:: calimages

    the list of calibrator source images to be saved.  Defaults to all
    calibrator images matching calintents. If defined overrides
    calintents and the calibrator images in the context.

    .. py:attribute:: targetimages

    the list of target source images to be saved.  Defaults to all
    target images. If defined overrides the list of target images in
    the context.

    .. py:attribute:: products_dir

    the directory where the data productions will be written

     """

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, session=None, vis=None, exportmses=None,
                 pprfile=None, calintents=None, calimages=None, targetimages=None,
                 products_dir=None ):

        """
        Initialise the Inputs, initialising any property values to those given
        here.

        :param context: the pipeline Context state object
        :type context: :class:`~pipeline.infrastructure.launcher.Context`
        :param output_dir: the working directory for pipeline data
        :type output_dir: string
        :param session: the  sessions for which data are to be exported
        :type session: a string or list of strings
        :param vis: the measurement set(s) for which products are to be exported
        :type vis: a string or list of strings
        :param pprfile: the pipeline processing request
        :type pprfile: a string
        :param calimages: the list of calibrator images to be saved
        :type calimages: a list
        :param targetimages: the list of target images to be saved
        :type targetimages: a list
        :param products_dir: the data products directory for pipeline data
        :type products_dir: string
        """

        # set the properties to the values given as input arguments
        self._init_properties(vars())

    # Code for handling sessions should be moved to the launcher.py,
    # basetask.py, and importdata.py modules. Session information
    # may come from the user or the pipeline processing request.

    @property
    def session(self):
        if self._session is None:
            self._session = []
        return self._session

    @session.setter
    def session (self, value):
        self._session = value

    @property
    def products_dir(self):
        if self._products_dir is None:
            if self.context.products_dir is None:
                self._products_dir = os.path.abspath('./')
            else:
                self._products_dir = self.context.products_dir
            try:
                LOG.trace('Creating products directory \'%s\'' % self._products_dir)
                os.makedirs(self.products_dir)
            except OSError as exc:
                if exc.errno == errno.EEXIST:
                    pass
                else: raise

            return self._products_dir
        return self._products_dir

    @products_dir.setter
    def products_dir(self, value):
        self._products_dir = value

    @property
    def exportmses (self):
        if self._exportmses is None:
            self._exportmses = False
        return self._exportmses

    @exportmses.setter
    def exportmses(self, value):
        self._exportmses = value

    @property
    def pprfile(self):
        if self._pprfile is None:
            self._pprfile = ''
        return self._pprfile

    @pprfile.setter
    def pprfile(self, value):
        self._pprfile = value

    @property
    def calintents(self):
        if self._calintents is None:
            self._calintents = ''
        return self._calintents

    @calintents.setter
    def calintents(self, value):
        self._calintents = value

    @property
    def calimages(self):
        if self._calimages is None:
            self._calimages = []
        return self._calimages

    @calimages.setter
    def calimages(self, value):
        self._calimages = value

    @property
    def targetimages(self):
        if self._targetimages is None:
            self._targetimages = []
        return self._targetimages

    @targetimages.setter
    def targetimages(self, value):
        self._targetimages = value

class ExportDataResults(basetask.Results):
    def __init__(self, pprequest='', sessiondict=collections.OrderedDict(),
                 visdict=collections.OrderedDict(), calimages=(), targetimages=(),
                 weblog='', pipescript='', restorescript='', commandslog='', 
                 manifest=''):
        """
        Initialise the results object with the given list of JobRequests.
        """
        super(ExportDataResults, self).__init__()
        self.pprequest = pprequest
        self.sessiondict = sessiondict
        self.visdict = visdict
        self.calimages = calimages
        self.targetimages = targetimages
        self.weblog = weblog
        self.pipescript = pipescript
        self.restorescript = restorescript
        self.commandslog = commandslog
        self.manifest = manifest

    def __repr__(self):
        s = 'ExportData results:\n'
        return s


class ExportData(basetask.StandardTaskTemplate):
    """
    ExportData is the base class for exporting data to the products
    subdirectory. It performs the following operations:

    - Saves the pipeline processing request in an XML file
    - Saves the final flags per ASDM in a compressed / tarred CASA flag
      versions file
    - Saves the final calibration apply list per ASDM in a text file
    - Saves the final set of caltables per session in a compressed /
      tarred file containing CASA tables
    - Saves the final web log in a compressed / tarred file
    - Saves the final CASA command log in a text file
    - Saves the final pipeline script in a Python file
    - Saves the final pipeline restore script in a Python file
    - Saves the images in FITS cubes one per target and spectral window
    """

    # link the accompanying inputs to this task
    Inputs = ExportDataInputs

    # Override the default behavior for multi-vis tasks
    def is_multi_vis_task(self):
        return True

    def prepare(self):
        """
        Prepare and execute an export data job appropriate to the
        task inputs.
        """

        # Create a local alias for inputs, so we're not saying
        # 'self.inputs' everywhere
        inputs = self.inputs

        # Initialize the standard ous is string.
        oussid = self.get_oussid(inputs.context)

        # Define the results object
        result = ExportDataResults()

        # Make the standard vislist and the sessions lists. 
        session_list, session_names, session_vislists, vislist = self._make_lists(inputs.context,
            inputs.session, inputs.vis)

        # Export the standard per OUS file products
        #    The pipeline processing request
        #    A compressed tarfile of the weblog
        #    The pipeline processing script
        #    The pipeline restore script (if exportmses = False)
        #    The CASA commands log
        recipe_name = self.get_recipename(inputs.context)
        if not recipe_name:
            prefix = oussid
        else:
            prefix = oussid + '.' + recipe_name
        stdfproducts = self._do_standard_ous_products(inputs.context, inputs.exportmses,
            prefix, inputs.pprfile, session_list, vislist, inputs.output_dir, inputs.products_dir)
        if stdfproducts.ppr_file:
            result.pprequest = os.path.basename(stdfproducts.ppr_file) 
        result.weblog = os.path.basename(stdfproducts.weblog_file)
        result.pipescript = os.path.basename(stdfproducts.casa_pipescript)
        if inputs.exportmses:
            result.restorescript = 'Undefined'
        else:
            result.restorescript = os.path.basename(stdfproducts.casa_restore_script)
        result.commandslog = os.path.basename(stdfproducts.casa_commands_file)

        # Make the standard ms dictionary and export per ms products
        #    Currently these are compressed tar files of per MS flagging tables and per MS text files of calibration apply instructions
        if inputs.exportmses:
            visdict = self._do_ms_products (inputs.context, vislist, inputs.products_dir)
        else:
            visdict = self._do_standard_ms_products (inputs.context, vislist, inputs.products_dir)
        result.visdict=visdict

        # Make the standard sessions dictionary and export per session products
        #    Currently these are compressed tar files of per session calibration tables
        sessiondict = self._do_standard_session_products (inputs.context, oussid, session_names, session_vislists,
            inputs.products_dir)
        result.sessiondict=sessiondict

        # Export calibrator images to FITS
        calimages_list, calimages_fitslist = self._export_images ( \
            inputs.context, True, inputs.calintents, inputs.calimages, \
            inputs.products_dir)
        result.calimages=(calimages_list, calimages_fitslist)

        # Export science target images to FITS
        targetimages_list, targetimages_fitslist = self._export_images ( \
            inputs.context, False, 'TARGET', inputs.targetimages, \
            inputs.products_dir)
        result.targetimages=(targetimages_list, targetimages_fitslist)

        # Export the pipeline manifest file
        #    TBD Remove support for auxiliary data products to the individual pipelines
        pipemanifest = self._make_pipe_manifest (inputs.context, oussid, stdfproducts, sessiondict, visdict,
            inputs.exportmses,
            [os.path.basename(image) for image in calimages_fitslist], 
            [os.path.basename(image) for image in targetimages_fitslist])
        casa_pipe_manifest = self._export_pipe_manifest(inputs.context, oussid,
            'pipeline_manifest.xml', inputs.products_dir, pipemanifest)
        result.manifest=os.path.basename(casa_pipe_manifest)

        # Return the results object, which will be used for the weblog
        return result


    def analyse(self, results):
        """
        Analyse the results of the export data operation.

        This method does not perform any analysis, so the results object is
        returned exactly as-is, with no data massaging or results items
        added.

        :rtype: :class:~`ExportDataResults`
        """
        return results

    def get_oussid (self, context):

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

    def get_recipename (self, context):

        """
        Get the recipe name
        """

        # Get the parent ous ousstatus name. This is the sanitized ous
        # status uid
        ps = context.project_structure
        if ps is None:
            recipe_name = ''
        elif ps.recipe_name == 'Undefined':
            recipe_name  = ''
        else:
            recipe_name = ps.recipe_name

        return recipe_name


    def _make_lists (self, context, session, vis, imaging=False):

        '''
        Create the vis and sessions lists
        '''

        # Force inputs.vis to be a list.
        vislist = vis
        if type(vislist) is types.StringType:
            vislist = [vislist,]
        if imaging:
            vislist = [vis for vis in vislist if context.observing_run.get_ms(name=vis).is_imaging_ms]
        else:
            vislist = [vis for vis in vislist if not context.observing_run.get_ms(name=vis).is_imaging_ms]

        # Get the session list and the visibility files associated with
        # each session.
        session_list, session_names, session_vislists= self._get_sessions ( \
            context, session, vislist)

        return session_list, session_names, session_vislists, vislist

    def _do_standard_ous_products(self, context, exportmses, oussid, pprfile, session_list, vislist, output_dir, products_dir):

        '''
        Generate the per ous standard products
        '''

        # Locate and copy the pipeline processing request.
        #     There should normally be at most one pipeline processing request.
        #     In interactive mode there is no PPR.
        ppr_files = self._export_pprfile (context, output_dir, products_dir, oussid, pprfile)
        if (ppr_files != []):
            ppr_file = os.path.basename(ppr_files[0])
        else:
            ppr_file = None

        # Export a tar file of the web log
        weblog_file = self._export_weblog (context, products_dir, oussid)

        # Export the processing log independently of the web log
        casa_commands_file = self._export_casa_commands_log (context,
            context.logs['casa_commands'], products_dir, oussid)

        # Export the processing script independently of the web log
        casa_pipescript = self._export_casa_script (context,
            context.logs['pipeline_script'], products_dir, oussid)

        # Export the restore script independently of the web log
        if exportmses:
            casa_restore_script = 'Undefined'
        else:
            casa_restore_script = self._export_casa_restore_script (context,
                context.logs['pipeline_restore_script'], products_dir,
                oussid, vislist, session_list)

        return StdFileProducts (ppr_file,
            weblog_file,
            casa_commands_file,
            casa_pipescript,
            casa_restore_script)

    def _do_ms_products(self, context, vislist, products_dir):
        '''
        Tar up the final calibrated mses and put them in the products
        directory.
        Used for reprocessing applications
        '''

        # Loop over the measurements sets in the working directory and tar
        # them up.
        mslist = []
        for visfile in vislist:
            ms_file = self._export_final_ms ( context, visfile, products_dir)
            mslist.append(ms_file)

        # Create the ordered vis dictionary
        #    The keys are the base vis names
        #    The values are the ms files
        visdict = collections.OrderedDict()
        for i in range(len(vislist)):
            visdict[os.path.basename(vislist[i])] = \
                 os.path.basename(mslist[i])

        return visdict

    def _do_standard_ms_products (self, context, vislist, products_dir):

        '''
        Generate the per ms standard products
        '''

        # Loop over the measurements sets in the working directory and
        # save the final flags using the flag manager.
        flag_version_name = 'Pipeline_Final'
        for visfile in vislist:
            self._save_final_flagversion (visfile, flag_version_name)

        # Copy the final flag versions to the data products directory
        # and tar them up.
        flag_version_list = []
        for visfile in vislist:
            flag_version_file = self._export_final_flagversion ( \
                context, visfile, flag_version_name, \
                products_dir)
            flag_version_list.append(flag_version_file)

        # Loop over the measurements sets in the working directory, and
        # create the calibration apply file(s) in the products directory.
        apply_file_list = []
        for visfile in vislist:
            apply_file =  self._export_final_applylist (context, \
                visfile, products_dir)
            apply_file_list.append (apply_file)

        # Create the ordered vis dictionary
        #    The keys are the base vis names
        #    The values are a tuple containing the flags and applycal files
        visdict = collections.OrderedDict()
        for i in range(len(vislist)):
            visdict[os.path.basename(vislist[i])] = \
                (os.path.basename(flag_version_list[i]), \
                 os.path.basename(apply_file_list[i]))

        return visdict

    def _do_standard_session_products (self, context, oussid, session_names, session_vislists, products_dir, imaging=False):

        '''
        Generate the per ms standard products
        '''

        # Export tar files of the calibration tables one per session
        caltable_file_list = []
        for i in range(len(session_names)):
            caltable_file = self._export_final_calfiles (context, oussid,
                session_names[i], session_vislists[i], products_dir, imaging=imaging)
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

    def _make_pipe_manifest (self, context, oussid, stdfproducts, sessiondict,
        visdict, exportmses, calimages, targetimages):

        '''
        Generate the manifest file
        '''

        # Initialize the manifest document and the top level ous status.
        pipemanifest = self._init_pipemanifest(oussid)
        ouss = pipemanifest.set_ous(oussid)
        pipemanifest.add_casa_version(ouss, casasys['build']['version'].strip())
        pipemanifest.add_pipeline_version(ouss, pipeline.revision)
        pipemanifest.add_procedure_name(ouss, context.project_structure.recipe_name)

        if stdfproducts.ppr_file:
            pipemanifest.add_pprfile (ouss, os.path.basename(stdfproducts.ppr_file))

        # Add the flagging and calibration products
        for session_name in sessiondict:
            session = pipemanifest.set_session(ouss, session_name)
            pipemanifest.add_caltables(session, sessiondict[session_name][1])
            for vis_name in sessiondict[session_name][0]:
                if exportmses:
                    pipemanifest.add_ms (session, vis_name, visdict[vis_name])
                else:
                    pipemanifest.add_asdm (session, vis_name, visdict[vis_name][0],
                        visdict[vis_name][1])

        # Add a tar file of the web log
        pipemanifest.add_weblog (ouss, os.path.basename(stdfproducts.weblog_file))

        # Add the processing log independently of the web log
        pipemanifest.add_casa_cmdlog (ouss,
            os.path.basename(stdfproducts.casa_commands_file))

        # Add the processing script independently of the web log
        pipemanifest.add_pipescript (ouss, os.path.basename(stdfproducts.casa_pipescript))

        # Add the restore script independently of the web log
        if exportmses:
            pipemanifest.add_restorescript (ouss, '')
        else:
            pipemanifest.add_restorescript (ouss, os.path.basename(stdfproducts.casa_restore_script))


        # Add the calibrator images
        pipemanifest.add_images (ouss, calimages, 'calibrator')

        # Add the target images
        pipemanifest.add_images (ouss, targetimages, 'target')

        return pipemanifest

    def _init_pipemanifest (self, oussid):
        '''
        Initialize the pipeline manifest
        '''

        pipemanifest = manifest.PipelineManifest(oussid)
        return pipemanifest

    def _export_pprfile (self, context, output_dir, products_dir, oussid, pprfile):

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
                if fnmatch.fnmatch (file, pprtemplate):
                    LOG.debug('Located pipeline processing request %s' % (file))
                    pprmatches.append (os.path.join(output_dir, file))

        # Copy the pipeline processing request files.
        pprmatchesout = []
        for file in pprmatches:
            if oussid:
                outfile = os.path.join (products_dir, oussid + '.pprequest.xml')
            else:
                outfile = file
            pprmatchesout.append(outfile)
            LOG.info('Copying pipeline processing file %s to %s' % \
                     (os.path.basename(file), os.path.basename(outfile)))
            if not self._executor._dry_run:
                shutil.copy (file, outfile)

        return pprmatchesout

    def _export_final_ms (self, context, vis, products_dir):

        """
        Save the ms to a compressed tarfile in products.
        """

        # Save the current working directory and move to the pipeline
        # working directory. This is required for tarfile IO
        cwd = os.getcwd()
        try:
            os.chdir (context.output_dir)

            # Define the name of the output tarfile
            visname = os.path.basename(vis)
            tarfilename = visname + '.tgz'
            LOG.info('Storing final ms %s in %s' % (visname, tarfilename))

            # Create the tar file
            if self._executor._dry_run:
                return tarfilename

            tar = tarfile.open (os.path.join(products_dir, tarfilename), "w:gz")
            tar.add (visname)
            tar.close()

        finally:
            # Restore the original current working directory
             os.chdir(cwd)

        return tarfilename


    def _save_final_flagversion (self, vis, flag_version_name):

        """
        Save the final flags to a final flag version.
        """

        LOG.info('Saving final flags for %s in flag version %s' % \
                 (os.path.basename(vis), flag_version_name))
        if not self._executor._dry_run:
            task = casa_tasks.flagmanager (vis=vis,
                                           mode='save', versionname=flag_version_name)
            self._executor.execute (task)

    def _export_final_flagversion (self, context, vis, flag_version_name,
                                   products_dir):

        """
        Save the final flags version to a compressed tarfile in products.
        """

        # Save the current working directory and move to the pipeline
        # working directory. This is required for tarfile IO
        cwd = os.getcwd()
        os.chdir (context.output_dir)

        # Define the name of the output tarfile
        visname = os.path.basename(vis)
        tarfilename = visname + '.flagversions.tgz'
        LOG.info('Storing final flags for %s in %s' % (visname, tarfilename))

        # Define the directory to be saved
        flagsname = os.path.join (visname + '.flagversions',
                                  'flags.' + flag_version_name)
        LOG.info('Saving flag version %s' % (flag_version_name))

        # Define the versions list file to be saved
        flag_version_list = os.path.join (visname + '.flagversions',
                                          'FLAG_VERSION_LIST')
        ti = tarfile.TarInfo(flag_version_list)
        #line = "Pipeline_Final : Final pipeline flags\n"
        line = "%s : Final pipeline flags\n" % flag_version_name
        ti.size = len (line)
        LOG.info('Saving flag version list')

        # Create the tar file
        if not self._executor._dry_run:
            tar = tarfile.open (os.path.join(products_dir, tarfilename), "w:gz")
            tar.add (flagsname)
            tar.addfile (ti, StringIO.StringIO(line))
            tar.close()

        # Restore the original current working directory
        os.chdir(cwd)

        return tarfilename

    def _export_final_applylist (self, context, vis, products_dir, imaging=False):
        """
        Save the final calibration list to a file. For now this is
        a text file. Eventually it will be the CASA callibrary file.
        """

        if imaging:
            applyfile_name = os.path.basename(vis) + '.auxcalapply.txt'
        else:
            applyfile_name = os.path.basename(vis) + '.calapply.txt'
        LOG.info('Storing calibration apply list for %s in  %s',
                 os.path.basename(vis), applyfile_name)

        if self._executor._dry_run:
            return applyfile_name

        try:
            calto = callibrary.CalTo(vis=vis)
            applied_calstate = context.callibrary.applied.trimmed(context, calto)

            # Log the list in human readable form. Better way to do this ?
            for calto, calfrom in applied_calstate.merged().iteritems():
                LOG.info('Apply to:  Field: %s  Spw: %s  Antenna: %s',
                         calto.field, calto.spw, calto.antenna)
                for item in calfrom:
                    LOG.info('    Gaintable: %s  Caltype: %s  Gainfield: %s  Spwmap: %s  Interp: %s',
                              os.path.basename(item.gaintable),
                              item.caltype,
                              item.gainfield,
                              item.spwmap,
                              item.interp)

            # Open the file.
            with open(os.path.join(products_dir, applyfile_name), "w") as applyfile:
                applyfile.write('# Apply file for %s\n' % (os.path.basename(vis)))
                applyfile.write(applied_calstate.as_applycal())
        except:
            applyfile_name = 'Undefined'
            LOG.info('No calibrations for MS %s' % os.path.basename(vis)) 

        return applyfile_name

    def _get_sessions (self, context, sessions, vis):

        """
    Return a list of sessions where each element of the list contains
    the  vis files associated with that session. In future this routine
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
        session_seqno = 0; session_dict = {}
        for i in range(len(wksessions)):
            if wksessions[i] not in session_dict:
                session_dict[wksessions[i]] = session_seqno
                session_seqno = session_seqno + 1

        # Initialize the output session names and visibility file lists
        session_names = []
        session_vis_list = []
        for key, value in sorted(session_dict.iteritems(), \
                                 key=lambda(k,v): (v,k)):
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
                session_vis_list[len(session_names)-1].append(vis[j])

        # Log the sessions
        for i in range(len(session_vis_list)):
            LOG.info('Visibility list for session %s is %s' % \
                     (session_names[i], session_vis_list[i]))

        return wksessions, session_names, session_vis_list

    def _export_final_calfiles(self, context, oussid, session, vislist, products_dir, imaging=False):
        """
        Save the final calibration tables in a tarfile one file
        per session.
        """

        # Save the current working directory and move to the pipeline
        # working directory. This is required for tarfile IO
        cwd = os.getcwd()
        try:
            os.chdir(context.output_dir)

            # Define the name of the output tarfile
            if imaging:
                tarfilename = '{}.{}.auxcaltables.tgz'.format(oussid, session)
            else:
                tarfilename = '{}.{}.caltables.tgz'.format(oussid, session)
            LOG.info('Saving final caltables for %s in %s', session, tarfilename)

            # Create the tar file
            if self._executor._dry_run:
                return tarfilename

            caltables = set()

            for visfile in vislist:
                LOG.info('Collecting final caltables for %s in %s',
                         os.path.basename(visfile), tarfilename)

                # Create the list of applied caltables for that vis
                try:
                    calto = callibrary.CalTo(vis=visfile)
                    calstate = context.callibrary.applied.trimmed(context, calto)
                    caltables.update(calstate.get_caltable())
                except:
                    LOG.info('No caltables for MS %s' % os.path.basename(visfile))

            if not caltables:
                return 'Undefined'

            with tarfile.open(os.path.join(products_dir, tarfilename), 'w:gz') as tar:
                # Tar the session list.
                for table in caltables:
                    tar.add(table, arcname=os.path.basename(table))

            return tarfilename

        finally:
            # Restore the original current working directory
            os.chdir(cwd)

    def _export_weblog (self, context, products_dir, oussid):

        """
        Save the processing web log to a tarfile
        """

        # Save the current working directory and move to the pipeline
        # working directory. This is required for tarfile IO
        cwd = os.getcwd()
        os.chdir (context.output_dir)

        # Define the name of the output tarfile
        ps = context.project_structure
        if ps is None:
            tarfilename = 'weblog.tgz'
        elif ps.ousstatus_entity_id == 'unknown':
            tarfilename = 'weblog.tgz'
        else:
            tarfilename = oussid + '.weblog.tgz'

        LOG.info('Saving final weblog in %s' % (tarfilename))

        # Create the tar file
        if not self._executor._dry_run:
            tar = tarfile.open (os.path.join(products_dir, tarfilename), "w:gz")
            tar.add (os.path.join(os.path.basename(os.path.dirname(context.report_dir)), 'html'))
            tar.close()

        # Restore the original current working directory
        os.chdir(cwd)

        return tarfilename

    def _export_casa_commands_log (self, context, casalog_name, products_dir, oussid):

        """
        Save the CASA commands file.
        """

        ps = context.project_structure
        if ps is None:
            casalog_file = os.path.join (context.report_dir, casalog_name)
            out_casalog_file = os.path.join (products_dir, casalog_name)
        elif ps.ousstatus_entity_id == 'unknown':
            casalog_file = os.path.join (context.report_dir, casalog_name)
            out_casalog_file = os.path.join (products_dir, casalog_name)
        else:
            casalog_file = os.path.join (context.report_dir, casalog_name)
            out_casalog_file = os.path.join (products_dir, oussid + '.' + casalog_name)

        LOG.info('Copying casa commands log %s to %s' % \
                 (casalog_file, out_casalog_file))
        if not self._executor._dry_run:
            shutil.copy (casalog_file, out_casalog_file)

        return os.path.basename(out_casalog_file)


    def _export_casa_restore_script (self, context, script_name, products_dir, oussid, vislist, session_list):

        """
        Save the CASA restore scropt.
        """

        # Generate the file list

        # Get the output file name
        ps = context.project_structure
        if ps is None:
            script_file = os.path.join (context.report_dir, script_name)
            out_script_file = os.path.join (products_dir, script_name)
        elif ps.ousstatus_entity_id == 'unknown':
            script_file = os.path.join (context.report_dir, script_name)
            out_script_file = os.path.join (products_dir, script_name)
        else:
            script_file = os.path.join (context.report_dir, script_name)
            out_script_file = os.path.join (products_dir, oussid + '.' + script_name)

        LOG.info('Creating casa restore script %s' %  (script_file))

        # This is hardcoded.
        tmpvislist=[]

        #ALMA default
        ocorr_mode = 'ca'

        for vis in vislist:
            filename = os.path.basename(vis)
            if filename.endswith('.ms'):
                filename, filext = os.path.splitext(filename)
            tmpvislist.append(filename)
        task_string = "    hif_restoredata (vis=%s, session=%s, ocorr_mode='%s')" % (tmpvislist, session_list, ocorr_mode)


        template = '''__rethrow_casa_exceptions = True
h_init()
try:
%s
finally:
    h_save()
''' % task_string

        with open (script_file, 'w') as casa_restore_file:
            casa_restore_file.write(template)

        LOG.info('Copying casa restore script %s to %s' % \
                 (script_file, out_script_file))
        if not self._executor._dry_run:
            shutil.copy (script_file, out_script_file)

        return os.path.basename (out_script_file)

    def _export_casa_script (self, context, casascript_name, products_dir, oussid):

        """
        Save the CASA script.
        """

        ps = context.project_structure
        if ps is None:
            casascript_file = os.path.join (context.report_dir, casascript_name)
            out_casascript_file = os.path.join (products_dir, casascript_name)
        elif ps.ousstatus_entity_id == 'unknown':
            casascript_file = os.path.join (context.report_dir, casascript_name)
            out_casascript_file = os.path.join (products_dir, casascript_name)
        else:
            #ousid = ps.ousstatus_entity_id.translate(string.maketrans(':/', '__'))
            casascript_file = os.path.join (context.report_dir, casascript_name)
            out_casascript_file = os.path.join (products_dir, oussid + '.' + casascript_name)

        LOG.info('Copying casa script file %s to %s' % \
                 (casascript_file, out_casascript_file))
        if not self._executor._dry_run:
            shutil.copy (casascript_file, out_casascript_file)

        return os.path.basename(out_casascript_file)

    def _export_pipe_manifest(self, context, oussid, manifest_name, products_dir, pipemanifest):

        """
        Save the manifest file.
        """

        out_manifest_file = os.path.join (products_dir, oussid + '.' + manifest_name)
        LOG.info('Creating manifest file %s' % (out_manifest_file))
        if not self._executor._dry_run:
            pipemanifest.write(out_manifest_file)
            
        return out_manifest_file

    def _fitsfile(self, products_dir, imagename):

        """Strip off stage and iter information to generate
           FITS file name."""

        # Need to remove stage / iter information
        #fitsname = re.sub('\.s\d+.*\.iter.*\.', '.', imagename)
        fitsname = re.sub('\.s\d+[_]\d+\.', '.', imagename)
        fitsname = re.sub('\.iter\d+\.image', '', fitsname)
        fitsname = re.sub('\.iter\d+\.image.pbcor', '.pbcor', fitsname)
        fitsname = re.sub('\.iter\d+\.mask', '.mask', fitsname)
        fitsname = re.sub('\.iter\d+\.alpha', '.alpha', fitsname)
        # .pb must be tried after .pbcor.image !
        fitsname = re.sub('\.iter\d+\.pb', '.pb', fitsname)
        fitsfile = os.path.join (products_dir,
                                 os.path.basename(fitsname) + '.fits')

        return fitsfile

    def _export_images (self, context, calimages, calintents, images,
                        products_dir):

        """
        Expora the images to FITS files.
        """


        # Create the image list
        images_list = []
        if len(images) == 0:
            # Get the image library
            if calimages:
                LOG.info ('Exporting calibrator source images')
                if calintents == '':
                    intents = ['PHASE', 'BANDPASS', 'CHECK', 'AMPLITUDE']
                else:
                    intents = calintents.split(',')
                cleanlist = context.calimlist.get_imlist()
            else:
                LOG.info ('Exporting target source images')
                intents = ['TARGET']
                cleanlist = context.sciimlist.get_imlist()
            for image_number, image in enumerate(cleanlist):
                # We need to store the image
                cleanlist[image_number]['fitsfiles'] = []
                cleanlist[image_number]['auxfitsfiles'] = []
                # Image name probably includes path
                if image['sourcetype'] in intents:
                    if (image['multiterm']):
                        for nt in xrange(image['multiterm']):
                            imagename = image['imagename'].replace('.image', '.image.tt%d' % (nt))
                            images_list.append(imagename)
                            cleanlist[image_number]['fitsfiles'].append(self._fitsfile(products_dir, imagename))
                        if (image['imagename'].find('.pbcor') != -1):
                            imagename = image['imagename'].replace('.image.pbcor', '.alpha')
                            images_list.append(imagename)
                            cleanlist[image_number]['fitsfiles'].append(self._fitsfile(products_dir, imagename))
                            imagename = '%s.error' % (image['imagename'].replace('.image.pbcor', '.alpha'))
                            images_list.append(imagename)
                            cleanlist[image_number]['fitsfiles'].append(self._fitsfile(products_dir, imagename))
                        else:
                            imagename = image['imagename'].replace('.image', '.alpha')
                            images_list.append(imagename)
                            cleanlist[image_number]['fitsfiles'].append(self._fitsfile(products_dir, imagename))
                            imagename = '%s.error' % (image['imagename'].replace('.image', '.alpha'))
                            images_list.append(imagename)
                            cleanlist[image_number]['fitsfiles'].append(self._fitsfile(products_dir, imagename))
                    else:
                        imagename = image['imagename']
                        images_list.append(imagename)
                        cleanlist[image_number]['fitsfiles'].append(self._fitsfile(products_dir, imagename))

                    # Add PBs for interferometry
                    if (image['imagename'].find('.image') != -1):
                        if (image['imagename'].find('.pbcor') != -1):
                            if (image['multiterm']):
                                imagename = image['imagename'].replace('.image.pbcor', '.pb.tt0')
                                images_list.append(imagename)
                                cleanlist[image_number]['auxfitsfiles'].append(self._fitsfile(products_dir, imagename))
                            else:
                                imagename = image['imagename'].replace('.image.pbcor', '.pb')
                                images_list.append(imagename)
                                cleanlist[image_number]['auxfitsfiles'].append(self._fitsfile(products_dir, imagename))
                        else:
                            if (image['multiterm']):
                                imagename = image['imagename'].replace('.image', '.pb.tt0')
                                images_list.append(imagename)
                                cleanlist[image_number]['auxfitsfiles'].append(self._fitsfile(products_dir, imagename))
                            else:
                                imagename = image['imagename'].replace('.image', '.pb')
                                images_list.append(imagename)
                                cleanlist[image_number]['auxfitsfiles'].append(self._fitsfile(products_dir, imagename))

                    # Add auto-boxing masks for interferometry
                    if (image['imagename'].find('.image') != -1):
                        if (image['imagename'].find('.pbcor') != -1):
                            if (image['multiterm']):
                                imagename = image['imagename'].replace('.image.pbcor', '.mask.tt0')
                                images_list.append(imagename)
                                cleanlist[image_number]['auxfitsfiles'].append(self._fitsfile(products_dir, imagename))
                            else:
                                imagename = image['imagename'].replace('.image.pbcor', '.mask')
                                images_list.append(imagename)
                                cleanlist[image_number]['auxfitsfiles'].append(self._fitsfile(products_dir, imagename))
                        else:
                            if (image['multiterm']):
                                imagename = image['imagename'].replace('.image', '.mask.tt0')
                                images_list.append(imagename)
                                cleanlist[image_number]['auxfitsfiles'].append(self._fitsfile(products_dir, imagename))
                            else:
                                imagename = image['imagename'].replace('.image', '.mask')
                                images_list.append(imagename)
                                cleanlist[image_number]['auxfitsfiles'].append(self._fitsfile(products_dir, imagename))
        else:
            # Assume only the root image name was given.
            cleanlib = imagelibrary.ImageLibrary()
            for image in images:
                if calimages:
                    imageitem = imagelibrary.ImageItem(imagename=image,
                                                       sourcename='UNKNOWN',
                                                       spwlist='UNKNOWN',
                                                       sourcetype='CALIBRATOR')
                else:
                    imageitem = imagelibrary.ImageItem(imagename=image,
                                                       sourcename='UNKNOWN',
                                                       spwlist='UNKNOWN',
                                                       sourcetype='TARGET')
                cleanlib.add_item(imageitem)
                if os.path.basename(image) == '':
                    images_list.append(os.path.join(context.output_dir, image))
                else:
                    images_list.append(image)
            cleanlist = cleanlib.get_imlist()
            # Need to add the FITS names
            for i in xrange(len(cleanlist)):
                cleanlist[i]['fitsfiles'] = [self._fitsfile(products_dir, images_list[i])]
                cleanlist[i]['auxfitsfiles'] = []

        # Convert to FITS.
        fits_list = []
        for image in images_list:
            print 'Working on', image
            fitsfile = self._fitsfile(products_dir, image)
            LOG.info('Saving final image %s to FITS file %s' % \
                     (os.path.basename(image), os.path.basename(fitsfile)))
            if not self._executor._dry_run:
                task = casa_tasks.exportfits (imagename=image,
                                              fitsimage=fitsfile,  velocity=False, optical=False,
                                              bitpix=-32, minpix=0, maxpix=-1, overwrite=True,
                                              dropstokes=False, stokeslast=True)
                self._executor.execute (task)
                fits_list.append(fitsfile)


        new_cleanlist = copy.deepcopy(cleanlist)

        return new_cleanlist, fits_list

