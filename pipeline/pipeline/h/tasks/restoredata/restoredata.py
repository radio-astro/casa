"""
The restore data module provides a class for reimporting, reflagging, and
recalibrating a subset of the ASDMs belonging to a member OUS, using pipeline
flagging and calibration data products.

The basic restore data module assumes that the ASDMs, flagging, and calibration
data products are on disk in the rawdata directory in the format produced by
the ExportData class.

This class assumes that the required data products have been
    o downloaded from the archive along with the ASDMs (not yet possible)
    o are sitting on disk in a form which is compatible with what is
      produced by ExportData

To test these classes, register some data with the pipeline using ImportData,
then execute:

import pipeline
vis = [ '<ASDM name>' ]

# Create a pipeline context and register some data
context = pipeline.Pipeline().context
inputs = pipeline.tasks.RestoreData.Inputs(context, vis=vis)
task = pipeline.tasks.RestoreData(inputs)
results = task.execute(dry_run=False)
results.accept(context)
"""
from __future__ import absolute_import
import glob
import os
import re
import shutil
import string
import tarfile
import tempfile
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from .. import applycal
from .. import importdata

# Note:
#    manifest.py should probably be moved to the common
#    subdirectory once the workflow is proofed
#from pipeline.h.tasks.exportdata import manifest
from .. common import manifest

from pipeline.infrastructure import casa_tasks

# the logger for this module
LOG = infrastructure.get_logger(__name__)


class RestoreDataInputs(basetask.StandardInputs):
    """
    RestoreDataInputs manages the inputs for the RestoreData task.

    .. py:attribute:: context

        the (:class:`~pipeline.infrastructure.launcher.Context`) holding all
        pipeline state

    .. py:attribute:: products_dir

        the directory containing the archived pipeline flagging and calibration
        data products. Data products will be unpacked from this directory
        into rawdata_dir. Support for this parameter is not yet implemented.

    .. py:attribute:: rawdata_dir

        the directory containing the raw data ASDM(s) and the pipeline
        flagging and calibration data products.

    .. py:attribute:: output_dir

        the working directory where the restored data will be written

    .. py:attribute:: session

        a string or list of strings containing the sessions(s) one for
        each vis.

    .. py:attribute:: vis

        a string or list of strings containing the ASDM(s) to be restored.
     """

    def __init__(self, context, copytoraw=None, products_dir=None, rawdata_dir=None,
        output_dir=None, session=None, vis=None, bdfflags=None, lazy=None, asis=None,
        ocorr_mode=None):

        """
        Initialise the Inputs, initialising any property values to those given
        here.

        :param context: the pipeline Context state object
        :type context: :class:`~pipeline.infrastructure.launcher.Context`
        :param copytoraw: copy the required data products from products_dir to
         rawdata_dir
        :param products_dir: the directory of archived pipeline products
        :type products_dir: string
        :param rawdata_dir: the raw data directory for ASDM(s) and products
        :type products_dir: string
        :param output_dir: the working directory for the restored data
        :type output_dir: string
        :param session: the  parent session of each vis
        :type session: a string or list of strings
        :param vis: the ASDMs(s) for which data is to be restored
        :type vis: a string or list of strings
        :param bdfflags: set the BDF flags
        :type bdfflags: boolean True or False
        :param lazy: use the lazy filler to restore data
        :type lazy: boolean True or False
        :param asis: list of ASDM tables to import as is
        :type asis: comma delimated list of tables
        """

        # set the properties to the values given as input arguments
        self._init_properties(vars())

    # Session information  may come from the user or the pipeline processing
    # request.

    @property
    def copytoraw(self):
        if self._copytoraw is None:
            self._copytoraw = True
            return self._copytoraw
        return self._copytoraw

    @copytoraw.setter
    def copytoraw(self, value):
        self._copytoraw = value

    @property
    def products_dir(self):
        if self._products_dir is None:
            if self.context.products_dir is None:
                self._products_dir = os.path.abspath('../products')
            else:
                self._products_dir = self.context.products_dir
            return self._products_dir
        return self._products_dir

    @products_dir.setter
    def products_dir(self, value):
        self._products_dir = value

    @property
    def rawdata_dir(self):
        if self._rawdata_dir is None:
            self._rawdata_dir = os.path.abspath('../rawdata')
        elif not self._rawdata_dir:
            self._rawdata_dir = os.path.abspath('../rawdata')
        return self._rawdata_dir

    @rawdata_dir.setter
    def rawdata_dir(self, value):
        self._rawdata_dir = value

    @property
    def session(self):
        if self._session is None:
            self._session = []
        return self._session

    @session.setter
    def session(self, value):
        self._session = value

    # MandatoryPipelineInputs raises an exception if vis has not been
    # registered with the context. For an import task, the vis is never
    # registered; to avoid the exception, we override the vis getter and
    # setter.
    @property
    def vis(self):
        return self._vis

    @vis.setter
    def vis(self, value):
        if type(value) is types.ListType:
            self._my_vislist = value
        self._vis = value

    @property
    def bdfflags(self):
        if self._bdfflags is None:
            self._bdfflags = True
        return self._bdfflags

    @bdfflags.setter
    def bdfflags(self, value):
        self._bdfflags = value

    @property
    def lazy(self):
        if self._lazy is None:
            self._lazy = False
        return self._lazy

    @lazy.setter
    def lazy(self, value):
        self._lazy = value

    @property
    def ocorr_mode(self):
        if self._ocorr_mode is None:
            self._ocorr_mode = 'ca'
        return self._ocorr_mode

    @ocorr_mode.setter
    def ocorr_mode(self, value):
        self._ocorr_mode = value

    @property
    def asis(self):
        if self._asis is None:
            self._asis = ''
        return self._asis

    @asis.setter
    def asis(self, value):
        self._asis = value


class RestoreDataResults(basetask.Results):
    def __init__(self, importdata_results=None, applycal_results=None):
        """
        Initialise the results objects.
        """
        super(RestoreDataResults, self).__init__()
        self.importdata_results = importdata_results
        self.applycal_results = applycal_results
        self.mses = []

    def merge_with_context(self, context):
        if self.importdata_results:
            for result in self.importdata_results:
                result.merge_with_context(context)
        for ms in context.observing_run.measurement_sets:
            self.mses.append(ms)
        if self.applycal_results:
            if type(self.applycal_results) is types.ListType:
                for result in self.applycal_results:
                    result.merge_with_context(context)
            else:
                self.applycal_results.merge_with_context(context)

    def __repr__(self):
        return 'RestoreDataResults:\n\t{0}'.format(
                '\n\t'.join([ms.name for ms in self.mses]))


class RestoreData(basetask.StandardTaskTemplate):
    """
    RestoreData is the base class for restoring flagged and calibrated
    data produced during a previous pipeline run and archived on disk.

    - Imports the selected ASDMs from rawdata
    - Imports the flagversions for the selected ASDMs from ../rawdata
    - Imports the calibration data for the selected ASDMs from ../rawdata
    - Restores the final set of pipeline flags
    - Restores the final calibration state
    - Applies the calibrations
    """

    # link the accompanying inputs to this task
    Inputs = RestoreDataInputs

    # Override the default behavior for multi-vis tasks
    # Does this interfere with multi-vis behavior of
    # called tasks.
    is_multi_vis_task = True

    def prepare(self):
        """
        Prepare and execute an export data job appropriate to the
        task inputs.
        """
        # Create a local alias for inputs, so we're not saying
        # 'self.inputs' everywhere
        inputs = self.inputs

        # Force inputs.vis and inputs.session to be a list.
        sessionlist = inputs.session
        if type(sessionlist) is types.StringType:
            sessionlist = [sessionlist, ]
        tmpvislist = inputs.vis
        if type(tmpvislist) is types.StringType:
            tmpvislist = [tmpvislist, ]
        vislist = []
        for vis in tmpvislist:
            if os.path.dirname(vis) == '':
                vislist.append(os.path.join(inputs.rawdata_dir, vis))
            else:
                vislist.append(vis)

        # Download ASDMs from the archive or products_dir to rawdata_dir.
        #   TBD: Currently assumed done somehow

        # Copy the required calibration products from "someplace" on disk
        #   (default ../products) to ../rawdata. The pipeline manifest file
        #   if present is used to determine which files to copy. Otherwise   
        #   a file naming scheme is used. The latter is deprecated as it
        #   requires the exportdata / restoredata tasks to be synchronized
        #   but it is maintained for testing purposes.
        if inputs.copytoraw:
            self._do_copy_manifest_toraw ('*pipeline_manifest.xml')
            pipemanifest = self._do_get_manifest ('*pipeline_manifest.xml') 
            self._do_copytoraw(pipemanifest)
        else:
            pipemanifest = self._do_get_manifest ('*pipeline_manifest.xml') 

        # Convert ASDMS assumed to be on disk in rawdata_dir. After this step
        # has been completed the MS and MS.flagversions directories will exist
        # and MS,flagversions will contain a copy of the original MS flags,
        # Flags.Original.
        #    TBD: Add error handling
        import_results = self._do_importasdm(sessionlist=sessionlist, vislist=vislist)

        # Restore final MS.flagversions and flags
        flag_version_name = 'Pipeline_Final'
        flag_version_list = self._do_restore_flags(pipemanifest,
            flag_version_name=flag_version_name)

        # Get the session list and the visibility files associated with
        # each session.
        session_names, session_vislists = self._get_sessions()

        # Restore calibration tables
        self._do_restore_caltables(pipemanifest, session_names=session_names,
            session_vislists=session_vislists)

        # Import calibration apply lists
        self._do_restore_calstate(pipemanifest)

        # Apply the calibrations.
        apply_results = self._do_applycal()

        # Return the results object, which will be used for the weblog
        return RestoreDataResults(import_results, apply_results)

    def analyse(self, results):
        """
        Analyse the results of the export data operation.

        This method does not perform any analysis, so the results object is
        returned exactly as-is, with no data massaging or results items
        added.

        :rtype: :class:~`ExportDataResults`
        """
        return results

    def _do_copy_manifest_toraw (self, template):

        """
        Get the pipeline manifest
        """

        inputs = self.inputs

        # Download the pipeline manifest file from the archive or
        #     products_dir to rawdata_dir
        manifestfiles = glob.glob(os.path.join(inputs.products_dir, template))
        for manifestfile in manifestfiles:
            LOG.info('Copying %s to %s' % (manifestfile, inputs.rawdata_dir))
            shutil.copy(manifestfile, os.path.join(inputs.rawdata_dir,
                os.path.basename(manifestfile)))

    def _do_get_manifest (self, template):

        """
        Get the pipeline manifest object
        """

        inputs = self.inputs

        # Get the list of files in the rawdata directory
        manifestfiles = glob.glob(os.path.join(inputs.rawdata_dir, template))

        # Parse manifest file if it exists.
        if len(manifestfiles) > 0:
            # Parse manifest file
            #    There should be only one of these so pick one
            pipemanifest = manifest.PipelineManifest('')
            pipemanifest.import_xml(manifestfiles[0])
        else:
            pipemanifest = None

        return pipemanifest

    def _do_copytoraw (self, pipemanifest):

        inputs = self.inputs

        ouss = pipemanifest.get_ous()

        # Download flag versions
        #   Download from the archive or products_dir to rawdata_dir.
        if pipemanifest is not None:
            inflagfiles = [os.path.join(inputs.products_dir, flagfile) for flagkey, \
                flagfile in pipemanifest.get_final_flagversions(ouss).iteritems()]
        else:
            inflagfiles = glob.glob(os.path.join(inputs.products_dir, \
                '*.flagversions.tgz'))
        for flagfile in inflagfiles:
            LOG.info('Copying %s to %s' % (flagfile, inputs.rawdata_dir))
            shutil.copy(flagfile, os.path.join(inputs.rawdata_dir,
                os.path.basename(flagfile)))

        # Download calibration tables
        #   Download calibration files from the archive or products_dir to
        if pipemanifest is not None:
            incaltables = [os.path.join(inputs.products_dir, caltable) for caltablekey, \
                caltable in pipemanifest.get_caltables(ouss).iteritems()]
        else:
            incaltables = glob.glob(os.path.join(inputs.products_dir, \
                '*.caltables.tgz'))
        for caltable in incaltables:
            LOG.info('Copying %s to %s' % (caltable, inputs.rawdata_dir))
            shutil.copy(caltable, os.path.join(inputs.rawdata_dir,
                os.path.basename(caltable)))

        # Download calibration apply lists
        #   Download from the archive or products_dir to rawdata_dir.
        #   TBD: Currently assumed done somehow
        if pipemanifest is not None:
            inapplycals = [os.path.join(inputs.products_dir, applycals) for applycalskey, \
                applycals in pipemanifest.get_applycals(ouss).iteritems()]
        else:
            inapplycals = glob.glob(os.path.join(inputs.products_dir, \
                '*.calapply.txt'))
        for applycal in inapplycals:
            LOG.info('Copying %s to %s' % (applycal, inputs.rawdata_dir))
            shutil.copy(applycal, os.path.join(inputs.rawdata_dir,
                os.path.basename(applycal)))

    def _do_importasdm(self, sessionlist, vislist):
        inputs = self.inputs
        # The asis is temporary until we get the EVLA / ALMA factoring
        # figured out.
        importdata_inputs = importdata.ImportData.Inputs(inputs.context,
            vis=vislist, session=sessionlist, save_flagonline=False,
            lazy=inputs.lazy, bdfflags=inputs.bdfflags, dbservice=False,
            asis=inputs.asis, ocorr_mode=inputs.ocorr_mode)
        importdata_task = importdata.ImportData(importdata_inputs)
        return self._executor.execute(importdata_task, merge=True)

    def  _do_restore_flags(self, pipemanifest, flag_version_name='Pipeline_Final'):
        inputs = self.inputs
        flagversionlist = []
        if pipemanifest is not None:
            ouss = pipemanifest.get_ous()
        else:
            ouss = None

        # Loop over MS list in working directory
        for ms in inputs.context.observing_run.measurement_sets:

            # Remove imported MS.flagversions from working directory
            flagversion = ms.basename + '.flagversions'
            flagversionpath = os.path.join(inputs.output_dir, flagversion)
            if os.path.exists(flagversionpath):
                LOG.info('Removing default flagversion for %s' % (ms.basename))
                if not self._executor._dry_run:
                    shutil.rmtree(flagversionpath)

            # Untar MS.flagversions file in rawdata_dir to output_dir
            if ouss is not None:
                tarfilename = os.path.join(inputs.rawdata_dir,
                    pipemanifest.get_final_flagversions(ouss)[ms.basename])
            else:
                tarfilename = os.path.join(inputs.rawdata_dir,
                    ms.basename + '.flagversions.tgz')
            LOG.info('Extracting %s' % flagversion)
            LOG.info('    From %s' % tarfilename)
            LOG.info('    Into %s' % inputs.output_dir)
            with tarfile.open(tarfilename, 'r:gz') as tar:
                if not self._executor._dry_run:
                    tar.extractall(path=inputs.output_dir)

            # Restore final flags version using flagmanager
            LOG.info('Restoring final flags for %s from flag version %s' % \
                     (ms.basename, flag_version_name))
            if not self._executor._dry_run:
                task = casa_tasks.flagmanager(vis=ms.name,
                                              mode='restore',
                                              versionname=flag_version_name)
                self._executor.execute(task)

            flagversionlist.append(flagversionpath)

        return flagversionlist

    def _do_restore_calstate(self, pipemanifest):
        inputs = self.inputs
        if pipemanifest is not None:
            ouss = pipemanifest.get_ous()
        else:
            ouss = None

        # Loop over MS list in working directory
        append = False
        for ms in inputs.context.observing_run.measurement_sets:
            if ouss is not None:
                applyfile_name = os.path.join(inputs.rawdata_dir,
                    pipemanifest.get_applycals(ouss)[ms.basename])
            else:
                applyfile_name = os.path.join(inputs.rawdata_dir,
                    ms.basename + '.calapply.txt')
            LOG.info('Restoring calibration state for %s from %s'
                     '' % (ms.basename, applyfile_name))

            if not self._executor._dry_run:
                # Write converted calstate to a temporary file and use this
                # for the import. the temporary file will automatically be
                # deleted once out of scope
                with tempfile.NamedTemporaryFile() as tmpfile:
                    LOG.trace('Writing converted calstate to %s'
                              '' % tmpfile.name)
                    converted = self._convert_calstate_paths(applyfile_name)
                    tmpfile.write(converted)
                    tmpfile.flush()

                    inputs.context.callibrary.import_state(tmpfile.name,
                                                           append=append)
            append = True

    def _convert_calstate_paths(self, applyfile):
        """
        Convert paths in the exported calstate to point to the new output
        directory.

        Returns the converted commands as a list of strings
        """

        # regex to match unix paths
        unix_path = re.compile('((?:\\/[\\w\\.\\-]+)+)',
                               re.IGNORECASE | re.DOTALL)

        # define a function that replaces directory names with our new output
        # directory
        def repfn(matchobj):
            basename = os.path.basename(matchobj.group(0))
            return os.path.join(self.inputs.output_dir, basename)

        # search-and-replace directory names in the exported calstate file
        with open(applyfile, 'r') as f:
            return unix_path.sub(repfn, f.read())

    def _do_restore_caltables(self, pipemanifest, session_names=None, session_vislists=None):
        inputs = self.inputs
        if pipemanifest is not None:
            ouss = pipemanifest.get_ous()
        else:
            ouss = None

        # Determine the OUS uid
        ps = inputs.context.project_structure
        if ps is None:
            ousid = ''
        elif ps.ousstatus_entity_id == 'unknown':
            ousid = ''
        else:
            ousid = ps.ousstatus_entity_id.translate(\
                string.maketrans(':/', '__')) + '.'

        # Loop over sessions
        for index, session in enumerate(session_names):

            # Get the visibility list for that session.
            vislist = session_vislists[index]

            # Open the tarfile and get the names
            if ouss is not None:
                tarfilename = os.path.join(inputs.rawdata_dir,
                    pipemanifest.get_caltables(ouss)[session])
            elif ousid == '':
                tarfilename = glob.glob(os.path.join(inputs.rawdata_dir, '*' + session +
                    '.caltables.tgz'))[0]
            else:
                tarfilename = os.path.join(inputs.rawdata_dir,
                    ousid + session + '.caltables.tgz')

            with tarfile.open(tarfilename, 'r:gz') as tar:
                tarmembers = tar.getmembers()

                # Loop over the visibilities associated with that session
                for vis in vislist:
                    LOG.info('Restoring caltables for %s from %s'
                             '' % (os.path.basename(vis), tarfilename))
                    extractlist = []
                    for member in tarmembers:
                        if member.name.startswith(os.path.basename(vis)):
                            extractlist.append(member)
                            # it is uncertain whether or not slash (/) exists at the end
                            if member.name.endswith('.tbl/') or member.name.endswith('.tbl'):
                                LOG.info('    Extracting caltable %s' % member.name)
                    if not self._executor._dry_run:
                        if len(extractlist) == len(tarmembers):
                            tar.extractall(path=inputs.output_dir)
                        else:
                            tar.extractall(path=inputs.output_dir,
                                           members=extractlist)

    def _do_applycal(self):
        inputs = self.inputs
        applycal_inputs = applycal.Applycal.Inputs(inputs.context)
        applycal_task = applycal.Applycal(applycal_inputs)
        return self._executor.execute(applycal_task, merge=True)

    def _get_sessions(self, sessions=[], vis=[]):

        """
        Return a list of sessions where each element of the list contains
        the  vis files associated with that session. If sessions is
        undefined the context is searched for session information
        """

        inputs = self.inputs

        # Get the MS list from the context by default.
        if len(vis) == 0:
            wkvis = []
            for ms in inputs.context.observing_run.measurement_sets:
                wkvis.append(ms.name)
        else:
            wkvis = vis

        # If the input session list is empty determine the sessions from
        # the context.
        if len(sessions) == 0:
            wksessions = []
            for visname in wkvis:
                session = inputs.context.observing_run.get_ms(name=visname).session
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
        for key, _ in sorted(session_dict.iteritems(),
                             key=lambda(k, v): (v, k)):
            session_names.append(key)
            session_vis_list.append([])

        # Assign the visibility files to the correct session
        for j in range(len(wkvis)):
            # Match the session names if possible
            if j < len(wksessions):
                for i in range(len(session_names)):
                    if wksessions[j] == session_names[i]:
                        session_vis_list[i].append(wkvis[j])
            # Assign to the last session
            else:
                session_vis_list[len(session_names) - 1].append(wkvis[j])

        # Log the sessions
        for i in range(len(session_vis_list)):
            LOG.info('Visibility list for session %s is %s' % \
            (session_names[i], session_vis_list[i]))

        return session_names, session_vis_list

