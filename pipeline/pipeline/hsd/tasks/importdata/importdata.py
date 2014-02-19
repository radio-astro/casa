from __future__ import absolute_import
import contextlib
import os
import shutil
import string
import tarfile
import types
import re

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.domain as domain
from pipeline.infrastructure import casa_tasks, sdtablereader
from ... import heuristics
from ..common import utils

import asap as sd

LOG = infrastructure.get_logger(__name__)


class SDImportDataInputs(basetask.StandardInputs):
    def __init__(self, context=None, infiles=None, output_dir=None, 
                 session=None, overwrite=None):
        self._init_properties(vars())

    @property
    def overwrite(self):
        return self._overwrite

    @overwrite.setter
    def overwrite(self, value):
        if value is None:
            value = False
        else:
            value = utils.to_bool(value)
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

    @property
    def infiles(self):
        return self._vis

    @infiles.setter
    def infiles(self, value):
        self.vis = value

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

    def as_dict(self):
        properties = super(SDImportDataInputs,self).as_dict()
        properties['infiles'] = properties.pop('vis')
        return properties


class SDImportDataResults(basetask.Results):
    def __init__(self, mses=[], scantables=[]):
        super(SDImportDataResults, self).__init__()
        self.mses = mses
        self.scantables = scantables
        self.origin = {}
        
    def merge_with_context(self, context):
        if not isinstance(context.observing_run, domain.ScantableList):
            context.observing_run = domain.ScantableList()
            context.callibrary = callibrary.SDCalLibrary(context)
        target = context.observing_run
        for ms in self.mses:
            LOG.info('Adding {0} to context'.format(ms.name))
            target.add_measurement_set(ms)
        for st in self.scantables:
            LOG.info('Adding {0} to context'.format(st.name))            
            target.add_scantable(st)
            
    def __repr__(self):
        return 'SDImportDataResults:\n\t{0}\n\t{1}'.format(
            '\n\t'.join([ms.name for ms in self.mses]),
            '\n\t'.join([st.name for st in self.scantables]))


class SDImportData(basetask.StandardTaskTemplate):
    Inputs = SDImportDataInputs

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
        context = inputs.context
        
        if vis is None:
            msg = 'Empty input data set list'
            LOG.warning(msg)
            raise ValueError, msg

        vis = os.path.abspath(os.path.expanduser(os.path.expandvars(vis)))
        if not os.path.exists(vis):
            msg = 'Input data set \'{0}\' not found'.format(vis)
            LOG.error(msg)
            raise IOError, msg

        # to_import: MS to be imported
        # to_convert: ASDM to be converted to MS and then to be imported
        # to_import_sd: Scantable to be imported 
        # to_convert_sd: non-Scantable, non-MS single dish data to
        #                be converted to Scantable and then to be imported
        to_import = set()
        to_convert = set()
        to_import_sd = set()
        to_convert_sd = set()

        # if this is a tar, get the names of the files and directories inside
        # the tar and calculate which can be directly imported (filenames with
        # a measurement set fingerprint) and which must be converted (files
        # with an ASDM fingerprint).
        if os.path.isfile(vis) and tarfile.is_tarfile(vis):
            with contextlib.closing(tarfile.open(vis)) as tar:
                filenames = tar.getnames()                

                filelists = self._analyse_filenames(filenames, 
                                                    vis)

                to_convert = [os.path.join(inputs.output_dir, asdm)
                              for asdm in filelists[0]]
                to_import = [os.path.join(inputs.output_dir, ms)
                              for ms in filelists[1]]
                to_import_sd = [os.path.join(inputs.output_dir, st)
                                for st in filelists[2]]
                to_convert_sd = [os.path.join(inputs.output_dir, st)
                                 for st in filelists[3]]

                if not self._executor._dry_run:
                    LOG.info('Extracting %s to %s' % (vis, inputs.output_dir))
                    tar.extractall(path=inputs.output_dir)

        # Assume that if vis is not a tar, it's a directory ready to be
        # imported, or in the case of an ASDM, converted then imported.
        else:
            # get a list of all the files in the given directory
            h = heuristics.DataTypeHeuristics()
            if not isinstance(vis, list):
                vis = [vis]
            for v in vis:
                data_type = h(v).upper()
                if data_type == 'ASDM':
                    to_convert.add(v)
                elif data_type == 'MS2':
                    to_import.add(v)
                elif data_type == 'ASAP':
                    to_import_sd.add(v)
                elif data_type in ['FITS','NRO']:
                    to_convert_sd.add(v)

            if not to_import and not to_convert \
                   and not to_import_sd and not to_convert_sd:
                raise TypeError, '%s is in unsupported data format' % vis

            # convert all paths to absolute paths for the next sequence
            to_import = map(os.path.abspath, to_import)
            to_import_sd = map(os.path.abspath, to_import_sd)

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
        converted_asdm_abspaths = [self._asdm_to_vis_filename(asdm) for asdm in asdms]

        # Now everything is in MS format, create a list of the MSes to import 
        to_import.extend(converted_asdm_abspaths)
        
        LOG.info('Creating pipeline objects for measurement set(s) {0}'
                  ''.format(', '.join(to_import)))
        if self._executor._dry_run:
            return SDImportDataResults()

        ms_reader = sdtablereader.ObservingRunReader
        
        to_import = [os.path.abspath(f) for f in to_import]

        # convert MS to Scantable
        scantable_list = []
        st_ms_map = []
        idx = 0
        for ms in to_import:
            LOG.info('Import %s as Scantable'%(ms))
            prefix = ms.rstrip('/')
            if re.match('.*\.(ms|MS)$',prefix):
                prefix = prefix[:-3]
            prefix = os.path.join(context.output_dir, prefix)
            LOG.debug('prefix: %s'%(prefix))
            scantables = sd.splitant(filename=ms,#ms.name,
                                     outprefix=prefix,
                                     overwrite=True)
            scantable_list.extend(scantables)
            st_ms_map.extend([idx]*len(scantables))
            
            idx += 1
        
        # launch an import job for each non-Scantable, non-MS single-dish data
        for any_data in to_convert_sd:
            LOG.info('Importing %s as Scantable'%(any_data))
            self._import_to_scantable(any_data)
            
        # calculate the filenames of the resultant Scantable
        to_import_sd.extend([self._any_data_to_scantable_name(name) for name in to_convert_sd])

        # Now everything is in Scnatable format, import them
        scantable_list.extend(to_import_sd)
        if len(st_ms_map) > 0:
            st_ms_map.extend([-1]*len(to_import_sd))

        LOG.debug('scantable_list=%s'%(scantable_list))
        LOG.debug('to_import=%s'%(to_import))
        LOG.debug('st_ms_map=%s'%(st_ms_map))
        observing_run_sd = ms_reader.get_observing_run_for_sd(scantable_list,
                                                              to_import,
                                                              st_ms_map)
    
        for st in observing_run_sd:
            LOG.debug('Setting session to %s for %s' % (inputs.session, 
                                                        st.basename))
            st.session = inputs.session
            
        results = SDImportDataResults(observing_run_sd.measurement_sets,
                                      observing_run_sd)#scantable_list)

        for ms in observing_run_sd.measurement_sets:
            LOG.debug('Setting session to %s for %s' % (inputs.session,
                                                        ms.basename))
            ms.session = inputs.session
            ms_origin = 'ASDM' if ms.name in converted_asdm_abspaths else 'MS'
            results.origin[ms.basename] = ms_origin

        return results
    
    def analyse(self, results):
        return results

    def _analyse_filenames(self, filenames, vis):
        to_import = set()
        to_convert = set()
        to_import_sd = set()
        to_convert_sd = set()
        
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
                                     # we need CalAtmosphere table to
                                     # access opacity value
                                     asis='CalAtmosphere',
                                     overwrite=self.inputs.overwrite)        
        
        self._executor.execute(task)

    def _import_to_scantable(self, data):
        outfile = self._any_data_to_scantable_name(data)
        s = sd.scantable(data, avearge=False)
        s.save(outfile, format='ASAP', overwrite=self.inputs.overwrite)
        

    def _any_data_to_scantable_name(self, name):
        return '{0}.asap'.format(os.path.join(self.inputs.output_dir,
                                              os.path.basename(name.rstrip('/'))))
