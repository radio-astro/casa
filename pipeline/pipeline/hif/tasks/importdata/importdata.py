from __future__ import absolute_import
import collections
import contextlib
import csv
import itertools
import operator
import os
import shutil
import string
import tarfile
import types
import xml.etree.ElementTree as ElementTree

import pipeline.domain as domain
import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.tablereader as tablereader
from ..common import commonfluxresults

LOG = infrastructure.get_logger(__name__)


class ImportDataInputs(basetask.StandardInputs):
    def __init__(self, context=None, vis=None, output_dir=None,
                 asis=None, session=None, overwrite=None, save_flagonline=None):
        self._init_properties(vars())

    overwrite = basetask.property_with_default('overwrite', False)
    save_flagonline = basetask.property_with_default('save_flagonline', True)
    asis = basetask.property_with_default('asis', 'Receiver CalAtmosphere')

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


class ImportDataResults(basetask.Results):
    def __init__(self, mses=[], setjy_results=None):
        super(ImportDataResults, self).__init__()
        self.mses = mses
        self.setjy_results = setjy_results
        
    def merge_with_context(self, context):
        target = context.observing_run
        for ms in self.mses:
            LOG.info('Adding {0} to context'.format(ms.name))
            target.add_measurement_set(ms)

        if self.setjy_results:
            for result in self.setjy_results:
                result.merge_with_context(context)
            
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

        setjy_results = get_setjy_results(observing_run.measurement_sets)

        results = ImportDataResults(observing_run.measurement_sets, 
                                    setjy_results)
    
        # write flux results to a CSV. I'm not sure what these values will be
        # used for now as we can import all flux values into the context with
        # no ill effects.
        export_flux_from_result(setjy_results, inputs.context)

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
    
    def _do_clearcal(self, vis):
        task = casa_tasks.clearcal(vis=vis, addmodel=False)
        self._executor.execute(task)
        
    def _do_importasdm(self, asdm):
        inputs = self.inputs        
        vis = self._asdm_to_vis_filename(asdm)
        outfile = os.path.join(inputs.output_dir,
                               os.path.basename(asdm) + "_flagonline.txt")
        
        task = casa_tasks.importasdm(asdm=asdm,
                                     vis=vis,
                                     savecmds=inputs.save_flagonline,
                                     outfile=outfile,
                                     process_caldevice=False,
				     asis=inputs.asis,
                                     overwrite=inputs.overwrite)
        
        self._executor.execute(task)

        asdm_source = os.path.join(asdm, 'Source.xml')
        if os.path.exists(asdm_source):
            vis_source = os.path.join(vis, 'Source.xml')            
            LOG.info('Copying Source.xml from ASDM to measurement set')
            LOG.trace('Copying Source.xml: %s to %s' % (asdm_source,
                                                        vis_source))
            shutil.copy(asdm_source, vis_source)


def get_setjy_results(mses):
    results = []
    for ms in mses:
        result = commonfluxresults.FluxCalibrationResults(ms.name)
        science_spws = ms.get_spectral_windows(science_windows_only=True)
        
        for source, measurements in read_fluxes(ms).items():
            m = [m for m in measurements if m.spw in science_spws]
            
            # import flux values for all fields and intents so that we can 
            # compare them to the fluxscale-derived values later in the run
#            for field in [f for f in source.fields if 'AMPLITUDE' in f.intents]:
            for field in source.fields:
                result.measurements[field.id].extend(m)
        
        results.append(result)
        
    return results

def read_fluxes(ms):
    result = collections.defaultdict(list)
        
    source_table = os.path.join(ms.name, 'Source.xml')
    if not os.path.exists(source_table):
        LOG.info('No Source XML found at %s. No flux import performed.'
                 % source_table)
        return result

    source_element = ElementTree.parse(source_table)
    if not source_element:
        LOG.info('Could not parse Source XML at %s. No flux import performed' 
                 % source_table)
        return result

    for row in source_element.findall('row'):
        flux_text = row.findtext('flux')
        frequency_text = row.findtext('frequency')
        source_id = row.findtext('sourceId')
        spw_id = row.findtext('spectralWindowId')

        # all elements must contain data to proceed
        if None in (flux_text, frequency_text, source_id, spw_id):
            continue
        
        # spws can overlap, so rather than looking up spw by frequency,
        # extract the spw id from the element text. I assume the format uses
        # underscores, eg. 'SpectralWindow_13'
        spw_id = string.split(spw_id, '_')[1]
        spw = ms.get_spectral_window(spw_id)

        source = ms.sources[int(source_id)]

        # we are mapping to spw rather than frequency, so should only take 
        # one flux density. 
        iquv = to_jansky(flux_text)[0]
        m = domain.FluxMeasurement(spw, *iquv)
        result[source].append(m)

    return result

def get_flux_density(frequency_text, flux_text):
    frequencies = to_hertz(frequency_text)
    fluxes = to_jansky(flux_text)

    for freq, flux in zip(frequencies, fluxes):
        yield (freq, flux)

def to_jansky(flux_text):
    '''
    Convert a string extracted from an ASDM XML element to FluxDensity domain 
    objects.
    '''
    flux_fn = lambda f : measures.FluxDensity(float(f), 
                                              measures.FluxDensityUnits.JANSKY)
    return get_atoms(flux_text, flux_fn)

def to_hertz(flux_text):
    '''
    Convert a string extracted from an ASDM XML element to Frequency domain 
    objects.
    '''
    freq_fn = lambda f : measures.Frequency(float(f), 
                                            measures.FrequencyUnits.HERTZ)
    return get_atoms(flux_text, freq_fn)

def get_atoms(text, conv_fn=lambda x: x):
    '''
    Get the individual measurements from an ASDM element.
    
    This function converts a CASA record from a linear space-separated string
    into a multidimensional list, using the dimension headers given at the
    start of the CASA record to determine the number and size of each
    dimension.
    
    text - text from an ASDM element, with space-separated values
    fn - optional function converting a string to a user-defined type
    '''
    values = string.split(text)
    # syntax is <num dimensions> <size dimension 1> <size dimension 2> etc.
    num_dimensions = int(values[0])
    dimension_sizes = map(int, values[1:num_dimensions+1])

    # find how may values are needed to form one complete 'entity'
    step_size = reduce(operator.mul, dimension_sizes)
    # idx holds the index of the first value for each entity    
    idx = len(dimension_sizes)+1

    results = []
    while idx < len(values):
        # get our complete entity as a linear list of strings, ready to be 
        # parcelled up into dimensions
        data = values[idx:idx+step_size]
        # convert the values using the given function, eg. from string to Jy
        data = map(conv_fn, data)
        # group the values into dimensions using the sizes in the header
        for s in dimension_sizes[-1:0:-1]:
            data = list(grouper(s, data))        
        results.extend(data)
        idx = idx + step_size

    return results

def grouper(n, iterable, fillvalue=None):
    '''
    grouper(3, 'ABCDEFG', 'x') --> ABC DEF Gxx
    '''
    args = [iter(iterable)] * n
    return itertools.izip_longest(fillvalue=fillvalue, *args)

def export_flux_from_context(context, filename=None):
    '''
    Export flux densities stored in the given context to a CSV file.
    '''
    if not filename:
        filename = os.path.join(context.output_dir, 'flux.csv')
        
    with open(filename, 'wt') as f:
        writer = csv.writer(f)
        writer.writerow(('ms', 'field', 'spw', 'I', 'Q', 'U', 'V'))
    
        counter = 0
        for ms in context.observing_run.measurement_sets:
            for field in ms.fields:
                for flux in field.flux_densities:
                    (I, Q, U, V) = flux.casa_flux_density
                    writer.writerow((ms.basename, field.id, flux.spw.id, 
                                     I, Q, U, V))
                    counter += 1

        LOG.info('Exported %s flux measurements to %s' % (counter, filename))

def export_flux_from_result(results, context, filename='flux.csv'):
    '''
    Export flux densities from a set of results to a CSV file.
    '''
    if type(results) is not types.ListType:
        results = [results,]        
    abspath = os.path.join(context.output_dir, filename)
        
    with open(abspath, 'wt') as f:
        writer = csv.writer(f)
        writer.writerow(('ms', 'field', 'spw', 'I', 'Q', 'U', 'V'))
    
        counter = 0
        for setjy_result in results:
            ms_name = setjy_result.vis
            ms_basename = os.path.basename(ms_name)
            for field_id, measurements in setjy_result.measurements.items():
                for m in measurements:
                    (I, Q, U, V) = m.casa_flux_density
                    writer.writerow((ms_basename, field_id, m.spw.id, 
                                     I, Q, U, V))
                    counter += 1

        LOG.info('Exported %s flux measurements to %s' % (counter, abspath))


def import_flux(context, filename=None):
    '''
    Read flux densities from a CSV file and import them into the context.
    ''' 
    if not filename:
        filename = os.path.join(context.output_dir, 'flux.csv')

    with open(filename, 'rt') as f:
        reader = csv.reader(f)

        # first row is header row
        reader.next()

        counter = 0
        for row in reader:
            (ms_name, field_id, spw_id, I, Q, U, V) = row
            spw_id = int(spw_id)
            ms = context.observing_run.get_ms(ms_name)
            fields = ms.get_fields(field_id)
            spw = ms.get_spectral_window(spw_id)
            measurement = domain.FluxMeasurement(spw, I, Q, U, V)

            # A single field identifier could map to multiple field objects,
            # but the flux should be the same for all, so we iterate..
            for field in fields:
                # .. removing any existing measurements in these spws from
                # these fields..
                map(field.flux_densities.remove,
                    [m for m in field.flux_densities if m.spw.id is spw_id])    
                 
                # .. and then updating with our new values
                LOG.trace('Adding %s to %s' % (measurement, spw))
                field.flux_densities.add(measurement)
                counter += 1
                
        LOG.info('Imported %s flux measurements from %s' % (counter, filename))
