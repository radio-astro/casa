from __future__ import absolute_import
import collections
import contextlib
import itertools
import os
import string
import re
import types

import numpy

from . import spectralwindow
from . import measures
import pipeline.extern.pyparsing as pyparsing
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)


def _parse_spw(task_arg, all_spw_ids=[]):
    """
    Convert the CASA-style spw argument to a list of spw IDs.
        
    Channel limits are also parsed in this function but are not currently
    used. The channel limits may be found as the channels property of an
    atom.
    
    Parsing the input '0:0~6^2,2:6~38^4 (0, 1, 4, 5, 6, 7)' results in the
    following results data structure;

          <result>
            <atom>
              <spws>
                <ITEM>0</ITEM>
              </spws>
              <channels>
                <ITEM>0</ITEM>
                <ITEM>2</ITEM>
                <ITEM>4</ITEM>
                <ITEM>6</ITEM>
              </channels>
            </atom>
            <atom>
              <spws>
                <ITEM>2</ITEM>
              </spws>
              <channels>
                <ITEM>6</ITEM>
                <ITEM>10</ITEM>
                <ITEM>14</ITEM>
                <ITEM>18</ITEM>
                <ITEM>22</ITEM>
                <ITEM>26</ITEM>
                <ITEM>30</ITEM>
                <ITEM>34</ITEM>
                <ITEM>38</ITEM>
              </channels>
            </atom>
          </result>
    """
    if task_arg in (None, ''):
        return all_spw_ids
    
    # recognise but suppress the mode-switching tokens
    TILDE, LESSTHAN, CARET, COLON, ASTERISK = map(pyparsing.Suppress, '~<^:*')

    # recognise '123' as a number, converting to an integer
    number = pyparsing.Word(pyparsing.nums).setParseAction(lambda tokens : int(tokens[0]))

    # convert '1~10' to a range
    rangeExpr = number('start') + TILDE + number('end')
    rangeExpr.setParseAction(lambda tokens : range(tokens.start, tokens.end+1))

    # convert '1~10^2' to a range with the given step size
    rangeWithStepExpr = number('start') + TILDE + number('end') + CARET + number('step')
    rangeWithStepExpr.setParseAction(lambda tokens : range(tokens.start, tokens.end+1, tokens.step))

    # convert <10 to a range
    ltExpr = LESSTHAN + number('max')
    ltExpr.setParseAction(lambda tokens : range(0, tokens.max))

    # convert * to all spws
    allExpr = ASTERISK.setParseAction(lambda tokens : all_spw_ids)
    
    # spw and channel components can be any of the above patterns    
    numExpr = rangeWithStepExpr | rangeExpr | ltExpr | allExpr | number
    
    # recognise and group multiple channel definitions separated by semi-colons
    channelsExpr = pyparsing.Group(pyparsing.delimitedList(numExpr, delim=';'))

    # group the number so it converted to a node, spw in this case
    spwsExpr = pyparsing.Group(numExpr)

    # the complete expression is either spw or spw:chan
    atomExpr = pyparsing.Group(spwsExpr('spws') + COLON + channelsExpr('channels') | spwsExpr('spws'))

    # and we can have multiple items separated by commas
    finalExpr = pyparsing.delimitedList(atomExpr('atom'), delim=',')('result')

    parse_result = finalExpr.parseString(str(task_arg))

    results = {}
    for atom in parse_result.result:
        for spw in atom.spws:
            if spw not in results:
                results[spw] = set(atom.channels)
            else:
                results[spw].update(atom.channels)

    Atom = collections.namedtuple('Atom', ['spw', 'channels'])
    return [Atom(spw=k, channels=v) for k,v in results.items()]


def _parse_field(task_arg, fields=[]):
    if task_arg in (None, ''):
        return [f.id for f in fields]
    
    # recognise but suppress the mode-switching tokens
    TILDE = pyparsing.Suppress('~')

    # recognise '123' as a number, converting to an integer
    number = pyparsing.Word(pyparsing.nums).setParseAction(lambda tokens : int(tokens[0]))

    # convert '1~10' to a range
    rangeExpr = number('start') + TILDE + number('end')
    rangeExpr.setParseAction(lambda tokens : range(tokens.start, tokens.end+1))

    boundary = [c for c in pyparsing.printables if c not in (' ', ',')]
    field_id = pyparsing.WordStart(boundary) + (rangeExpr | number) + pyparsing.WordEnd(boundary)
    
    casa_chars = ''.join([c for c in string.printable 
                          if c not in string.whitespace])
    field_name = pyparsing.Word(casa_chars + ' ')

    def get_ids_for_matching(tokens):
        search_term = tokens[0]
        if '*' in search_term:
            regex = search_term.replace('*','.*') + '$'
            return [f.id for f in fields if re.match(regex, f.name)]    
        return [f.id for f in fields if f.name == search_term]
        
    field_name.setParseAction(get_ids_for_matching)        
    
    results = set()
    for atom in pyparsing.commaSeparatedList.parseString(str(task_arg)):
        for parser in [field_name('fields'), field_id('fields')]:
            for match in parser.searchString(atom):
                results.update(match.asList())
    
    return sorted(list(results))


def _parse_antenna(task_arg, antennas=[]):
    if task_arg in (None, ''):
        return [a.id for a in antennas]
    
    # recognise but suppress the mode-switching tokens
    TILDE = pyparsing.Suppress('~')

    # recognise '123' as a number, converting to an integer
    number = pyparsing.Word(pyparsing.nums).setParseAction(lambda tokens : int(tokens[0]))

    # convert '1~10' to a range
    rangeExpr = number('start') + TILDE + number('end')
    rangeExpr.setParseAction(lambda tokens : range(tokens.start, tokens.end+1))

    # antenna-oriented 'by ID' expressions can be any of the above patterns    
    boundary = [c for c in pyparsing.printables if c not in (' ', ',')]
    numExpr = pyparsing.WordStart(boundary) + (rangeExpr | number) + pyparsing.WordEnd(boundary)
    
    # group the number so it converted to a node, fields in this case
    antenna_id_expr = pyparsing.Group(numExpr)

    casa_chars = ''.join([c for c in string.printable 
                          if c not in ',;"/' + string.whitespace]) 
    antenna_name = pyparsing.Word(casa_chars)

    def get_antenna(tokens):
        search_term = tokens[0]
        if '*' in search_term:
            regex = search_term.replace('*','.*') + '$'
            return [a.id for a in antennas if re.match(regex, a.name)]    
        return [a.id for a in antennas if a.name == search_term]

    antenna_name.setParseAction(get_antenna)        
    
    antenna_name_expr = pyparsing.Group(antenna_name)

    # the complete expression
    atomExpr = pyparsing.Group(antenna_id_expr('antennas') | antenna_name_expr('antennas'))

    # and we can have multiple items separated by commas
    finalExpr = pyparsing.delimitedList(atomExpr('atom'), delim=',')('result')

    parse_result = finalExpr.parseString(str(task_arg))
    
    results = set()
    for atom in parse_result.result:
        map(results.add, atom.antennas)

    return sorted(list(results))
    

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
        requested_ids = set(_parse_antenna(search_term, self.antennas))
        present_ids = set([a.id for a in self.antennas])
        
        if not requested_ids <= present_ids:
            missing_ids = requested_ids - present_ids
            msg = ('Antenna IDs %s not found in %s (search term=\'%s\')'
                   % (','.join([str(i) for i in missing_ids]), 
                      self.basename, search_term))
            LOG.error(msg)
            raise ValueError(msg)
        return [a for a in self.antennas if a.id in requested_ids]

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

        if task_arg is not None:
            requested_ids = set(_parse_field(task_arg, self.fields))
            present_ids = set([f.id for f in self.fields])
            
            if not requested_ids <= present_ids:
                missing_ids = requested_ids - present_ids
                
                # purely numerical field names (eg. field.name='123') will 
                # return two matches, one for the ID and one for the name. 
                # In this situation we expect fewer matched IDs than requested
                # IDs. Remove names from the missing IDs before complaining:
                missing_ids = set([str(i) for i in missing_ids])
                names = set([f.name for f in self.fields])

                still_missing = missing_ids - names

                if still_missing:
                    msg = ('Field IDs %s not found in %s (search term=\'%s\')'
                           % (','.join(still_missing), 
                              self.basename, task_arg))
                    LOG.error(msg)
                    raise ValueError()
            pool = [f for f in pool if f.id in requested_ids]
        
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
            science_intents = set(['TARGET','PHASE','BANDPASS','AMPLITUDE'])
            return [w for w in spws if w.num_channels not in (1,4)
                    and not science_intents.isdisjoint(w.intents)]

        return spws

    def get_all_spectral_windows(self, task_arg='', with_channels=False):
        """Return the spectral windows corresponding to the given CASA-style
        spw argument.
        """
        if task_arg in (None, ''):
            return list(self.spectral_windows)

        all_spw_ids = [spw.id for spw in self.spectral_windows]
        atoms = _parse_spw(task_arg, all_spw_ids)
        
        if not with_channels:
            spw_ids = [atom.spw for atom in atoms]
            return [self.get_spectral_window(i) for i in spw_ids]
        
        spws = []
        for atom in atoms:
            spw_obj = self.get_spectral_window(atom.spw)
            proxy = spectralwindow.SpectralWindowWithChannelSelection(spw_obj, 
                                                                      atom.channels)
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
        
    @property
    def session(self):
        return self._session
    
    @session.setter
    def session(self, value):
        if value is None:
            value = 'session_1'
        self._session = value
