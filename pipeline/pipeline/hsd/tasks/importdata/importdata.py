from __future__ import absolute_import
import os
import re
import contextlib
import tarfile
import string
import shutil

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.domain as domain
import pipeline.domain.measures as measures
import pipeline.domain.singledish as singledish
from ... import heuristics

import pipeline.hif.tasks.importdata.importdata as importdata
import pipeline.hifa.tasks.importdata.almaimportdata as almaimportdata

LOG = infrastructure.get_logger(__name__)

class SDImportDataInputs(almaimportdata.ALMAImportDataInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context=None, vis=None, output_dir=None,
                 asis=None, process_caldevice=None, session=None, overwrite=None, 
                 bdfflags=None, save_flagonline=None, lazy=None, dbservice=None,
                 with_pointing_correction=None, createmms=None):
        self._init_properties(vars())

    asis = basetask.property_with_default('asis', 'Antenna Station Receiver CalAtmosphere CalWVR')
    with_pointing_correction = basetask.property_with_default('with_pointing_correction', True)

class SDImportDataResults(basetask.Results):
    '''
    SDImportDataResults is an equivalent class with ImportDataResults. 
    Purpose of SDImportDataResults is to replace QA scoring associated 
    with ImportDataResults with single dish specific QA scoring, which 
    is associated with this class.
    
    ImportDataResults holds the results of the ImportData task. It contains
    the resulting MeasurementSet domain objects and optionally the additional 
    SetJy results generated from flux entries in Source.xml.
    '''
    
    def __init__(self, mses=None, reduction_group=None, setjy_results=None):
        super(SDImportDataResults, self).__init__()
        self.mses = [] if mses is None else mses
        self.setjy_results = setjy_results
        self.reduction_group = reduction_group
        self.origin = {}
        self.results = importdata.ImportDataResults(mses=mses, setjy_results=setjy_results)
        
    def merge_with_context(self, context):
        if not isinstance(context.observing_run, domain.ScantableList):
            context.observing_run = domain.ScantableList()
        self.results.merge_with_context(context)
        context.observing_run.ms_reduction_group = self.reduction_group
           
    def __repr__(self):
        return 'SDImportDataResults:\n\t{0}'.format(
                '\n\t'.join([ms.name for ms in self.mses]))

class SDImportData(importdata.ImportData):
    Inputs = SDImportDataInputs 
    
    def prepare(self, **parameters):
        # get results object by running super.prepare()
        results = super(SDImportData, self).prepare()
        
        # per ms inspection: beam size and calibration strategy
        for ms in results.mses:
            self._inspect_beam_size(ms)
            self._inspect_calibration_strategy(ms)
            
        # inspection: reduction group
        reduction_group = self._inspect_reduction_group(results.mses)
                
        # create results object
        myresults = SDImportDataResults(mses=results.mses, reduction_group=reduction_group, 
                                        setjy_results=results.setjy_results)
        myresults.origin = results.origin
        return myresults
    
    def _inspect_calibration_strategy(self, ms):
        tsys_transfer = []
        calibration_type_heuristic = heuristics.CalibrationTypeHeuristics()
        spwmap_heuristic = heuristics.TsysSpwMapHeuristics()
        calibration_type = calibration_type_heuristic(ms.name)
        science_windows = ms.get_spectral_windows(science_windows_only=True)
        tsys_windows = [spw for spw in ms.spectral_windows \
                        if 'ATMOSPHERE' in spw.intents and spw.name.find('CH_AVG') == -1]
        TOL = singledish.ScantableRep.tolerance
        for spwa in tsys_windows:
            fmina = float(spwa._min_frequency.value)
            fmaxa = float(spwa._max_frequency.value)
            for spwt in science_windows:
                if spwa == spwt:
                    # identical spw, skip (not necessary to transfer Tsys)
                    continue
                elif spwa.baseband != spwt.baseband:
                    # different baseband, skip
                    continue
                else:
                    fmint = float(spwt._min_frequency.value)
                    fmaxt = float(spwt._max_frequency.value)
                    dfmin = (fmint - fmina) / fmina
                    dfmax = (fmaxt - fmaxa) / fmaxa
                    LOG.trace('(fmina,fmaxa) = (%s, %s)'%(fmina, fmaxa))
                    LOG.trace('(fmint,fmaxt) = (%s, %s)'%(fmint, fmaxt))
                    LOG.trace('dfmin = %s, dfmax=%s, TOL = %s'%(dfmin, dfmax, TOL))
                    if dfmin >= -TOL and dfmax <= TOL:
                        tsys_transfer.append([spwa.id, spwt.id])
        do_tsys_transfer = len(tsys_transfer) > 0
        spwmap = spwmap_heuristic(ms, tsys_transfer)
        calibration_strategy = {'tsys': do_tsys_transfer,
                                'tsys_strategy': spwmap,
                                'calmode': calibration_type}
        ms.calibration_strategy = calibration_strategy
        
        
    def _inspect_beam_size(self, ms):
        beam_size_heuristic = heuristics.SingleDishBeamSize()
        beam_sizes = {}
        for antenna in ms.antennas:
            diameter = antenna.diameter 
            antenna_id = antenna.id
            beam_size_for_antenna = {}
            for spw in ms.spectral_windows:
                spw_id = spw.id
                center_frequency = float(spw.centre_frequency.convert_to(measures.FrequencyUnits.GIGAHERTZ).value)
                beam_size = beam_size_heuristic(diameter=diameter, frequency=center_frequency)
                beam_size_quantity = casatools.quanta.quantity(beam_size, 'arcsec')
                beam_size_for_antenna[spw_id] = beam_size_quantity
            beam_sizes[antenna_id] = beam_size_for_antenna
        ms.beam_sizes = beam_sizes
        
    def _inspect_reduction_group(self, mslist):
        reduction_group = {}
        group_spw_names = {}
        antenna_index= 0
        for (index, ms) in enumerate(mslist):
            science_windows = ms.get_spectral_windows(science_windows_only=True)
            for spw in science_windows:
                data_description = ms.get_data_description(spw=spw) # returns only one data description
                name = spw.name
                nchan = spw.num_channels
                min_frequency = float(spw._min_frequency.value)
                max_frequency = float(spw._max_frequency.value)
                polarization_id = data_description.pol_id
                corr_type_string = ms.polarizations[polarization_id].corr_type_string.squeeze()
                polarizations = map(lambda x: singledish.Polarization.to_polid[x], corr_type_string)
                if len(name) > 0:
                    # grouping by name
                    match = self.__find_match_by_name(name, group_spw_names)
                else:
                    # grouping by frequency range
                    match = self.__find_match_by_coverage(nchan, min_frequency, max_frequency, 
                                                          reduction_group, fraction=0.99)
                if match == False:
                    # add new group
                    key = len(reduction_group)
                    group_spw_names[key] = name
                    newgroup = singledish.ReductionGroupDesc(frequency_range=[min_frequency, max_frequency], 
                                                             nchan=nchan)
                    reduction_group[key] = newgroup
                else:
                    key = match
                for antenna in ms.antennas:
                    reduction_group[key].add_member(antenna_index + antenna.id, spw.id, polarizations)
            antenna_index += len(ms.antennas)
        
        return reduction_group
    
    def __find_match_by_name(self, spw_name, spw_names):
        match = False
        for (group_key,group_spw_name) in spw_names.items():
            if (group_spw_name==''): 
                raise RuntimeError, "Got empty group spectral window name"
            elif spw_name == group_spw_name:
                match = group_key
                break
        return match

    def __find_match_by_coverage(self, nchan, min_frequency, max_frequency, reduction_group, fraction=0.99):
        if fraction<=0 or fraction>1.0:
            raise ValueError, "overlap fraction should be between 0.0 and 1.0"
        LOG.warn("Creating reduction group by frequency overlap. This may be not proper if observation dates extend over long period.")
        match = False
        for (group_key,group_desc) in reduction_group.items():
            group_range = group_desc.frequency_range
            group_nchan = group_desc.nchan
            overlap = max( 0.0, min(group_range[1], max_frequency)
                           - max(group_range[0], min_frequency))
            width = max(group_range[1], max_frequency) - min(group_range[0], min_frequency)
            coverage = overlap/width
            if nchan == group_nchan and coverage >= fraction:
                match = group_key
                break
        return match