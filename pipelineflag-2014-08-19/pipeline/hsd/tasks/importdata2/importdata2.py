from __future__ import absolute_import
import os
import re
import contextlib
import tarfile
import string
import shutil

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.basetask as basetask
import pipeline.domain as domain
from pipeline.infrastructure import tablereader
from ... import heuristics
from .. import common
from ..common import utils

import pipeline.hif.tasks.importdata.importdata as importdata

LOG = infrastructure.get_logger(__name__)


class SDImportData2Inputs(importdata.ImportDataInputs):
    pass

#class SDImportData2Results(importdata.ImportDataResults):
#    pass
class SDImportData2Results(basetask.Results):
    '''
    ImportDataResults holds the results of the ImportData task. It contains
    the resulting MeasurementSet domain objects and optionally the additional 
    SetJy results generated from flux entries in Source.xml.
    '''
    
    def __init__(self, mses=None, setjy_results=None):
        super(SDImportData2Results, self).__init__()
        self.mses = [] if mses is None else mses
        self.setjy_results = setjy_results
        self.origin = {}
        
    def merge_with_context(self, context):
        target = context.observing_run
        for ms in self.mses:
            LOG.info('Adding {0} to context'.format(ms.name))
            target.add_measurement_set(ms)

        if self.setjy_results:
            for result in self.setjy_results:
                result.merge_with_context(context)
            
    def __repr__(self):
        return 'SDImportData2Results:\n\t{0}'.format(
                '\n\t'.join([ms.name for ms in self.mses]))

class SDImportData2(importdata.ImportData):
    Inputs = SDImportData2Inputs

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

        results = SDImportData2Results()
        to_import = set()
        to_convert = set()
        to_clearcal = set()

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
                    to_clearcal.add(dst)
                    continue

                if not self._executor._dry_run:
                    LOG.info('Copying %s to %s' % (src, inputs.output_dir))
                    shutil.copytree(f, dst)

        # clear the calibration of any stale file that exists in the working
        # directory
        for old_file in to_clearcal:
            self._do_clearcal(old_file)
            
        # launch an import job for each ASDM we need to convert 
        for asdm in to_convert:
            self._do_importasdm(asdm)

        # calculate the filenames of the resultant measurement sets
        asdms = [os.path.join(inputs.output_dir, f) for f in to_convert]

        # Now everything is in MS format, create a list of the MSes to import
        converted_asdms = [self._asdm_to_vis_filename(asdm) for asdm in asdms]
        to_import.extend(converted_asdms)

        # get the path to the MS for the converted ASDMs, which we'll later
        # compare to ms.name in order to calculate the origin of each MS   
        converted_asdm_abspaths = [os.path.abspath(f) for f in converted_asdms]

        LOG.info('Creating pipeline objects for measurement set(s) {0}'
                  ''.format(', '.join(to_import)))
        if self._executor._dry_run:
            return SDImportData2Results()

        ms_reader = tablereader.ObservingRunReader
        
        to_import = [os.path.abspath(f) for f in to_import]
        observing_run = ms_reader.get_observing_run(to_import)
        for ms in observing_run.measurement_sets:
            LOG.debug('Setting session to %s for %s' % (inputs.session,
                                                        ms.basename))
            ms.session = inputs.session
            
            ms_origin = 'ASDM' if ms.name in converted_asdm_abspaths else 'MS'
            results.origin[ms.basename] = ms_origin

        # get the flux measurements from Source.xml for each MS
        xml_results = importdata.get_setjy_results(observing_run.measurement_sets)
        # write/append them to flux.csv

        # Cycle 1 hack for exporting the field intents to the CSV file: 
        # export_flux_from_result queries the context, so we pseudo-register
        # the mses with the context by replacing the original observing run
        orig_observing_run = inputs.context.observing_run
        inputs.context.observing_run = observing_run
        try:            
            importdata.export_flux_from_result(xml_results, inputs.context)
        finally:
            inputs.context.observing_run = orig_observing_run
            
        # re-read from flux.csv, which will include any user-coded values
        combined_results = importdata.import_flux(inputs.context.output_dir, observing_run)

        results.mses.extend(observing_run.measurement_sets)
        results.setjy_results = combined_results
        
        return results
