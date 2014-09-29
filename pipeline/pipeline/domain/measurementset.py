from __future__ import absolute_import
import collections
import contextlib
import itertools
import os
import string
import types

import numpy

from . import spectralwindow
from . import measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)


class MeasurementSet(object):    
    def __init__(self, name, session=None):
        self.name = name
        self.antenna_array = None
        self.data_descriptions = []
        self.fields = []
        self.frequency_groups = []
        self.states = []
        self.reference_antenna = None
        self.reference_spwmap= None
        self.phaseup_spwmap= None
        self.flagcmds = []
        self.session = session
        self.filesize = self._calc_filesize() 

    def _calc_filesize(self):
        '''
        Calculate the disk usage of this measurement set.
        '''
        total_bytes = 0
        for dirpath, _, filenames in os.walk(self.name):
            for f in filenames:
                fp = os.path.join(dirpath, f)
                total_bytes += os.path.getsize(fp)
    
        return measures.FileSize(total_bytes, 
                                 measures.FileSizeUnits.BYTES)
    
    def __repr__(self):
        return 'MeasurementSet({0})'.format(self.name)
        
    @property
    def spectral_windows(self):
        spws = []
        for group in self.frequency_groups.values():
            spws.extend(group.windows)
        return spws

    @property
    def intents(self):
        intents = set()
        # we look to field rather than state as VLA datasets don't have state
        # entries
        for field in self.fields:
            intents.update(field.intents)
        return intents
    
    @property
    def antennas(self):
        # return a copy rather than the underlying list
        return list(self.antenna_array.antennas)

    @property
    def basename(self):
        return os.path.basename(self.name)
    
    def get_antenna(self, search_term=''):
        if search_term == '':
            return self.antennas

        return [a for a in self.antennas
                if a.id in utils.ant_arg_to_id(self.name, search_term)]

    def get_state(self, state_id=None):
        match = [state for state in self.states if state.id == state_id]
        if match:
            return match[0]
        else:
            return None

    def get_scans(self, scan_id=None, scan_intent=None):
        pool = self.scans

        if scan_id is not None:
            # encase raw numbers in a tuple
            if not isinstance(scan_id, collections.Sequence):
                scan_id = (scan_id,)
            pool = [s for s in pool if s.id in scan_id]

        if scan_intent is not None:
            if type(scan_intent) in types.StringTypes:
                if scan_intent in ('', '*'):
                    # empty string equals all intents for CASA
                    scan_intent = ','.join(self.intents)
                scan_intent = scan_intent.split(',')
            scan_intent = set(scan_intent) 
            pool = [s for s in pool if not s.intents.isdisjoint(scan_intent)]

        return pool

    def get_data_description(self, spw=None, id=None):
        match = None
        if spw is not None:
            if isinstance(spw, spectralwindow.SpectralWindow):
                match = [dd for dd in self.data_descriptions
                         if dd.spw is spw]
            elif type(spw) is types.IntType:
                match = [dd for dd in self.data_descriptions
                         if dd.spw.id is spw]        
        if id is not None:
            match = [dd for dd in self.data_descriptions if dd.id == id]
            
        if match:
            return match[0]
        else:
            return None
    
    def get_fields(self, task_arg=None, field_id=None, name=None, intent=None):
        """
        Get Fields from this MeasurementSet matching the given criteria. If no
        criteria are given, all Fields in the MeasurementSet will be returned.

        Arguments can be given as either single items of the expected type,
        sequences of the expected type, or in the case of name or intent, as
        comma separated strings. For instance, name could be 'HOIX', 
        'HOIX,0841+708' or ('HOIX','0841+708').
        
        :param field_id: field ID(s) to match
        :param name: field name(s) to match
        :param intent: observing intent(s) to match
        :rtype: a (potentially empty) list of :class:`~pipeline.domain.field.Field` \
             objects
        """
        pool = self.fields
        #print pool

        if task_arg not in (None, ''):
            pool = [f for f in pool
                    if f.id in utils.field_arg_to_id(self.name, task_arg)]
        
        if field_id is not None:
            # encase raw numbers in a tuple
            if not isinstance(field_id, collections.Sequence):
                field_id = (field_id,)
            pool = [f for f in pool if f.id in field_id]

        if name is not None:
            if type(name) in types.StringTypes:
                name = string.split(name, ',')
            name = set(name) 
            pool = [f for f in pool if f.name in name]
        
        if intent is not None:
            if type(intent) in types.StringTypes:
                if intent in ('', '*'):
                    # empty string equals all intents for CASA
                    intent = ','.join(self.intents)
                intent = string.split(intent, ',')
            intent = set(intent) 
            pool = [f for f in pool if not f.intents.isdisjoint(intent)]

        return pool

    def get_spectral_window(self, spw_id):
        if spw_id is not None:
            spw_id = int(spw_id)
            match = [spw for spw in self.spectral_windows 
                     if spw.id is spw_id]
            if match:
                return match[0]
            else:
                raise KeyError('No spectral window with ID \'{0}\' found in '
                               '{1}'.format(spw_id, self.basename))

    def get_spectral_windows(self, task_arg='', with_channels=False,
                             num_channels=(), science_windows_only=True):
        """
        Return the spectral windows corresponding to the given CASA-style spw
        argument, filtering out windows that may not be science spectral 
        windows (WVR windows, channel average windows etc.).
        """
        spws = self.get_all_spectral_windows(task_arg, with_channels)

        # if requested, filter spws by number of channels
        if num_channels:
            spws = [w for w in spws if w.num_channels in num_channels] 
            
        if not science_windows_only:
            return spws
        
        if self.antenna_array.name == 'ALMA':
            science_intents = set(['TARGET','PHASE','BANDPASS','AMPLITUDE', 'POLARIZATION', 'POLANGLE', 'POLLEAKAGE', 'CHECK'])
            return [w for w in spws if w.num_channels not in (1,4)
                    and not science_intents.isdisjoint(w.intents)]

        return spws

    def get_all_spectral_windows(self, task_arg='', with_channels=False):
        """Return the spectral windows corresponding to the given CASA-style
        spw argument.
        """
        # we may have more spectral windows in our MeasurementSet than have
        # data in the measurement set on disk. Ask for all 
        if task_arg in (None, ''):
            task_arg = '*'

        # expand spw tuples into a range per spw, eg. spw9 : 1,2,3,4,5
        selected = collections.defaultdict(set)
        for (spw, start, end, step) in utils.spw_arg_to_id(self.name, task_arg):
            selected[spw].update(set(range(start, end+1, step))) 
            
        if not with_channels:
            return [spw for spw in self.spectral_windows if spw.id in selected]

        spws = []
        for spw_id, channels in selected.items():
            spw_obj = self.get_spectral_window(spw_id)
            proxy = spectralwindow.SpectralWindowWithChannelSelection(spw_obj, 
                                                                      channels)
            spws.append(proxy)
        return spws

    def get_frequency_groups_for_intent(self, intent):
        # get the valid spectral windows for fields observed with this intent
        valid_spws = [field.valid_spws for field in self.fields 
                      if intent in field.intents]
        # .. and remove any duplicates..
        valid_spws = set(*valid_spws)
    
        if not valid_spws:
            raise LookupError(('No valid spectral windows '
                               'with intent {0}'.format(intent)))
    
        # .. now get the frequency group for each of these valid windows
        freq_groups = [spw.group for spw in self.spectral_windows 
                       if spw in valid_spws]
        # .. removing any duplicates
        freq_groups = set(freq_groups)
        
        return freq_groups
    
    def get_original_intent(self, intent=None):
        """
        Get the original obs_modes that correspond to the given pipeline
        observing intents.
        """
        obs_modes = [state.get_obs_mode_for_intent(intent)
                     for state in self.states]
        return set(itertools.chain(*obs_modes))
    
    @property
    def start_time(self):
        qt = casatools.quanta
        s = sorted(self.scans, 
                   key=lambda scan: scan.start_time['m0'],
                   cmp=lambda x,y: 1 if qt.gt(x,y) else 0 if qt.eq(x,y) else -1)
        return s[0].start_time

    @property
    def end_time(self):
        qt = casatools.quanta
        s = sorted(self.scans, 
                   key=lambda scan: scan.end_time['m0'],
                   cmp=lambda x,y: 1 if qt.gt(x,y) else 0 if qt.eq(x,y) else -1)
        return s[-1].start_time
        
    
    def get_vla_max_integration_time(self):
        """Get the integration time used by the original VLA scripts
       
           Returns -- The max integration time used
        """
        
        vis = self.name
        
        with utils.open_table(vis + '/FIELD') as table:
            #casatools.table.open(vis+'/FIELD')
            numFields = table.nrows()
            field_positions = table.getcol('PHASE_DIR')
            field_ids = range(numFields)
            field_names = table.getcol('NAME')
            #casatools.table.close()
        
        with utils.open_table(vis) as table:
            #casatools.table.open(vis)
            scanNums = sorted(numpy.unique(table.getcol('SCAN_NUMBER')))
            field_scans = []
            for ii in range(0,numFields):
                subtable = table.query('FIELD_ID==%s'%ii)
                field_scans.append(list(numpy.unique(subtable.getcol('SCAN_NUMBER'))))
            #casatools.table.close()
        
        ## field_scans is now a list of lists containing the scans for each field.
        ## so, to access all the scans for the fields, you'd:
        #
        #for ii in range(0,len(field_scans)):
        #   for jj in range(0,len(field_scans[ii]))
        #
        ## the jj'th scan of the ii'th field is in field_scans[ii][jj]
        
        # Identify intents
        
        with utils.open_table(vis + '/STATE') as table:
            #casatools.table.open(vis+'/STATE')
            intents = table.getcol('OBS_MODE')
            #casatools.table.close()
        
        """Figure out integration time used"""
        
        casatools.ms.open(vis)
        scan_summary = casatools.ms.getscansummary()
        ms_summary = casatools.ms.summary()
        casatools.ms.close()
        startdate=float(ms_summary['BeginTime'])
    
        integ_scan_list = []
        for scan in scan_summary:
            integ_scan_list.append(int(scan))
        sorted_scan_list = sorted(integ_scan_list)
        
        # find max and median integration times
        #
        integration_times = []
        for ii in sorted_scan_list:
            integration_times.append(scan_summary[str(ii)]['0']['IntegrationTime'])
            
        maximum_integration_time = max(integration_times)
        median_integration_time = numpy.median(integration_times)
        
        int_time = maximum_integration_time
        
        return int_time
       
       
       
    
    def get_median_integration_time(self, intent=None):
        """Get the median integration time used to get data for the given
        intent.
    
        Keyword arguments:
        intent  -- The intent of the data of interest.
      
        Returns -- The median integration time used.
        """
        LOG.debug('inefficiency - MSFlagger reading file to get integration '
                  'time')
    
        # get the field IDs and state IDs for fields in the measurement set,
        # filtering by intent if necessary
        if intent:    
            field_ids = [field.id for field in self.fields 
                         if intent in field.intents]
            state_ids = [state.id for state in self.states
                         if intent in state.intents]
#        if intent:
#            re_intent = intent.replace('*', '.*')
#            re_intent = re.compile(re_intent)
#            field_ids = [field.id for field in self.fields 
#                         if re_intent.match(str(field.intents))]
#            state_ids = [state.id for state in self.states
#                         if re_intent.match(str(state.intents))]
        else:
            field_ids = [field.id for field in self.fields]
            state_ids = [state.id for state in self.states]
            
        # VLA datasets have an empty STATE table; in the main table such rows
        # have a state ID of -1.
        if not state_ids:
            state_ids = [-1] 
        
        with utils.open_table(self.name) as table:
            taql = '(STATE_ID IN %s AND FIELD_ID IN %s)' % (state_ids, field_ids)
            with contextlib.closing(table.query(taql)) as subtable:
                integration = subtable.getcol('INTERVAL')          
            return numpy.median(integration)
    
    
    def get_median_science_integration_time(self, intent=None, spw=None):
        """Get the median integration time for science targets used to get data for the given
        intent.
    
        Keyword arguments:
        intent  -- The intent of the data of interest.
        spw     -- spw string list - '1,7,11,18'
      
        Returns -- The median integration time used.
        """
        LOG.debug('inefficiency - MSFlagger reading file to get median integration '
                  'time for science targets')
        
        if (spw == None):
            spws = self.spectral_windows
        else: 
            
            try:
                #Put csv string of spws into a list
                spw_string_list = spw.split(',')
                
                #Get all spw objects
                all_spws = self.spectral_windows
        
                #Filter out the science spw objects
                spws = [ispw for ispw in all_spws if str(ispw.id) in spw_string_list]
            except:
                LOG.error("Incorrect spw string format.")
        
            
        
        # now get the science spws, those used for scientific intent
        science_spws = [ispw for ispw in spws if 
          ispw.num_channels not in [1,4] and not ispw.intents.isdisjoint(
          ['BANDPASS', 'AMPLITUDE', 'PHASE', 'TARGET'])]
        LOG.debug('science spws are: %s' % [ispw.id for ispw in science_spws])
    
        # and the science fields/states
        science_field_ids = [field.id for field in self.fields
          if not set(field.intents).isdisjoint(
            ['BANDPASS', 'AMPLITUDE', 'PHASE', 'TARGET'])]
        science_state_ids = [state.id for state in self.states
          if not set(state.intents).isdisjoint(
            ['BANDPASS', 'AMPLITUDE', 'PHASE', 'TARGET'])]
            
        
        science_spw_dd_ids = [self.get_data_description(spw).id for spw in science_spws]
        
        with utils.open_table(self.name) as table:
            taql = '(STATE_ID IN %s AND FIELD_ID IN %s AND DATA_DESC_ID in %s)' % (science_state_ids, science_field_ids, science_spw_dd_ids)
            with contextlib.closing(table.query(taql)) as subtable:
                integration = subtable.getcol('INTERVAL')          
            return numpy.median(integration)
    
    
    
    
    
    
    
    @property
    def session(self):
        return self._session
    
    @session.setter
    def session(self, value):
        if value is None:
            value = 'session_1'
        self._session = value
