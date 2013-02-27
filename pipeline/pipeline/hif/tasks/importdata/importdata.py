from __future__ import absolute_import
import contextlib
import os
import shutil
import string
import tarfile
import types

import pipeline.infrastructure.api as api
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure.jobrequest import casa_tasks
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.tablereader as tablereader

LOG = logging.get_logger(__name__)


class ImportDataInputs(basetask.StandardInputs):
    def __init__(self, context=None, vis=None, output_dir=None, 
                 session=None, overwrite=None):
        self._init_properties(vars())

    @property
    def overwrite(self):
        return self._overwrite

    @overwrite.setter
    def overwrite(self, value):
        if value is None:
            value = False
        self._overwrite = value

    @property
    def session(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('session')
        
        if not isinstance(self.vis, list) and isinstance(self._session, list):
            idx = self._my_vislist.index(self.vis)
            return self._session[idx]

        if type(self.vis) is types.StringType and type(self._session) is types.StringType:
            return self._session
        
        # current default - return all intents
        return 'Session_default'
    
    @session.setter
    def session(self, value):
        self._session = value

    def to_casa_args(self):
        raise NotImplementedError

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


class ImportDataResults(api.Results):
    def __init__(self, mses=[]):
        self.mses = mses
        
    def merge_with_context(self, context):
        target = context.observing_run
        for ms in self.mses:
            LOG.info('Adding {0} to context'.format(ms.name))
            target.add_measurement_set(ms)
            
    def __repr__(self):
        return 'ImportDataResults:\n\t{0}'.format(
                '\n\t'.join([ms.name for ms in self.mses]))


class ImportData(basetask.StandardTaskTemplate):
    Inputs = ImportDataInputs

    def _ms_directories(self, names):
        """
        Inspect a list of file entries, finding the root directory of any
        measurement sets present via a set of identifying files and
        directories.
        """
        identifiers = ('SOURCE', 'FIELD', 'ANTENNA', 'DATA_DESCRIPTION')
        
        matching = [os.path.dirname(n) for n in names 
                    if os.path.basename(n) in identifiers]

        return set([m for m in matching 
                    if matching.count(m) == len(identifiers)])

    def _asdm_directories(self, members):
        """
        Inspect a list of file entries, finding the root directory of any
        ASDMs present via a set of identifying files and directories .
        """
        identifiers = ('ASDMBinary', 'Main.xml', 'ASDM.xml', 'Antenna.xml')
        
        matching = [os.path.dirname(m) for m in members 
                    if os.path.basename(m) in identifiers]
            
        return set([m for m in matching 
                    if matching.count(m) == len(identifiers)])

    def prepare(self, **parameters):
        inputs = self.inputs
        vis = inputs.vis
        
        if vis is None:
            msg = 'Empty input data set list'
            LOG.warning(msg)
            raise ValueError, msg

        if not os.path.exists(vis):
            msg = 'Input data set \'{0}\' not found'.format(vis)
            LOG.error(msg)
            raise IOError, msg
        
        to_import = set()
        to_convert = set()

        # if this is a tar, get the names of the files and directories inside
        # the tar and calculate which can be directly imported (filenames with
        # a measurement set fingerprint) and which must be converted (files
        # with an ASDM fingerprint).
        if os.path.isfile(vis) and tarfile.is_tarfile(vis):
            with contextlib.closing(tarfile.open(vis)) as tar:
                filenames = tar.getnames()                

                (to_import, to_convert) = self._analyse_filenames(filenames, 
                                                                  vis)

                to_convert = [os.path.join(inputs.output_dir, asdm)
                              for asdm in to_convert]
                to_import = [os.path.join(inputs.output_dir, ms)
                              for ms in to_import]

                if not self._executor._dry_run:
                    LOG.info('Extracting %s to %s' % (vis, inputs.output_dir))
                    tar.extractall(path=inputs.output_dir)

        # Assume that if vis is not a tar, it's a directory ready to be
        # imported, or in the case of an ASDM, converted then imported.
        else:
            # get a list of all the files in the given directory
            filenames = [os.path.join(vis, f) for f in os.listdir(vis)]

            (to_import, to_convert) = self._analyse_filenames(filenames, 
                                                              vis)

            if not to_import and not to_convert:
                raise TypeError, '%s is neither a MS nor an ASDM' % vis

            # convert all paths to absolute paths for the next sequence
            to_import = map(os.path.abspath, to_import)

            # if the file is not in the working directory, copy it across,
            # replacing the filename with the relocated filename
            to_copy = set([f for f in to_import 
                           if string.find(f, inputs.output_dir) != 0])                
            for src in to_copy:
                dst = os.path.join(os.path.abspath(inputs.output_dir), 
                                   os.path.basename(src))
                to_import.remove(src)
                to_import.append(dst)
                
                if os.path.exists(dst):
                    LOG.warning('%s already in %s. Will import existing data.'
                        '' % (os.path.basename(src), inputs.output_dir))
                    continue

                if not self._executor._dry_run:
                    LOG.info('Copying %s to %s' % (src, inputs.output_dir))
                    shutil.copytree(f, dst)

        # launch an import job for each ASDM we need to convert 
        for asdm in to_convert:
            self._do_importasdm(asdm)
        # calculate the filenames of the resultant measurement sets
        asdms = [os.path.join(inputs.output_dir, f) for f in to_convert]

        # Now everything is in MS format, create a list of the MSes to import 
        to_import.extend([self._asdm_to_vis_filename(asdm) for asdm in asdms])
        
        LOG.info('Creating pipeline objects for measurement set(s) {0}'
                  ''.format(', '.join(to_import)))
        if self._executor._dry_run:
            return ImportDataResults()

        ms_reader = tablereader.ObservingRunReader
        
        to_import = [os.path.abspath(f) for f in to_import]
        observing_run = ms_reader.get_observing_run(to_import)
        for ms in observing_run.measurement_sets:
            LOG.debug('Setting session to %s for %s' % (inputs.session, 
                                                        ms.basename))
            ms.session = inputs.session
    
        results = ImportDataResults(observing_run.measurement_sets)
        return results
    
    def analyse(self, results):
        return results

    def _analyse_filenames(self, filenames, vis):
        to_import = set()
        to_convert = set()
        
        ms_dirs = self._ms_directories(filenames)
        if ms_dirs:
            LOG.debug('Adding measurement set(s) {0} from {1} to import queue'
                ''.format(', '.join([os.path.basename(f) for f in ms_dirs]), 
                          vis))
            cleaned_paths = map(os.path.normpath, ms_dirs)
            to_import.update(cleaned_paths)
                
        asdm_dirs = self._asdm_directories(filenames)
        if asdm_dirs:
            LOG.debug('Adding ASDMs {0} from {1} to conversion queue'
                      ''.format(', '.join(asdm_dirs), vis))
            to_convert.update(asdm_dirs)

        return (to_import, to_convert)

    def _asdm_to_vis_filename(self, asdm):
        return '{0}.ms'.format(os.path.join(self.inputs.output_dir, 
                                            os.path.basename(asdm)))
    
    def _do_importasdm(self, asdm):
        vis = self._asdm_to_vis_filename(asdm)
        outfile = os.path.join(self.inputs.output_dir,
                               os.path.basename(asdm)+"_flagcmds.txt")

        task = casa_tasks.importasdm(asdm=asdm, 
                                     vis=vis, 
                                     savecmds=True,
                                     outfile=outfile,
                                     process_caldevice=False,
                                     overwrite=self.inputs.overwrite)        
        
        self._executor.execute(task)
