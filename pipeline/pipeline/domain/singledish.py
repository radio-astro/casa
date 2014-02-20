from __future__ import absolute_import
import numpy
import os
import re

from . import antenna
from . import frequencygroup
from . import measures
from . import observingrun
from . import source
from . import spectralwindow
from .datatable import DataTableImpl as DataTable

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)


def to_numeric_freq(m, unit=measures.FrequencyUnits.HERTZ):
    return float(m.convert_to(unit).value)

class ScantableList(observingrun.ObservingRun, list):
    def __init__(self):
        super(ScantableList,self).__init__()
        self.reduction_group = {}
        self.grid_position = {}
        self.datatable_instance = None
        self.datatable_name = None#os.path.join(context.name,'DataTable.tbl')

    def __getstate__(self):
        mydict = self.__dict__.copy()
        # not to pickle DataTable instance
        mydict['datatable_instance'] = None
        return mydict

    def __setstate__(self, d):
        self.__dict__ = d
        name = self.__dict__['datatable_name']
        if name is not None and self.__dict__['datatable_instance'] is None:
            datatable = DataTable(name)
            self.__dict__['datatable_instance'] = datatable
        

    @property
    def start_time(self):
        if len(self) == 0:
            return None
        qt = casatools.quanta
        s = sorted(self, 
                   key=lambda st: st.start_time['m0'],
                   cmp=lambda x,y: 1 if qt.gt(x,y) else 0 if qt.eq(x,y) else -1)
        return s[0].start_time

    @property
    def end_time(self):
        if len(self) == 0:
            return None
        qt = casatools.quanta
        s = sorted(self, 
                   key=lambda ms: ms.end_time['m0'],
                   cmp=lambda x,y: 1 if qt.gt(x,y) else 0 if qt.eq(x,y) else -1)
        return s[-1].end_time

    @property
    def st_names(self):
        return [st.basename for st in self]

    def merge_inspection(self, instance, name=None, reduction_group=None,
                         calibration_strategy=None, beam_size=None,
                         grid_position=None, observing_pattern=None):
        self.datatable_instance = instance
        if name is not None:
            self.datatable_name = name

        self.reduction_group = reduction_group
        self.grid_position = grid_position
        
        for idx in xrange(len(self)):
            self[idx].calibration_strategy = calibration_strategy[idx]
            self[idx].beam_size = beam_size[idx]
            self[idx].pattern = observing_pattern[idx]
            self[idx].work_data = self[idx].name

    def add_scantable(self, s):
        if s.basename in self.st_names:
            msg = '%s is already in the pipeline context'%(s.name)
            LOG.error(msg)
            raise Exception, msg

        self.append(s)
        if s.ms_name:
            ms = self.get_ms(name=s.ms_name)
            if hasattr(ms, 'scantables'):
                ms.scantables.append(s)
            else:
                ms.scantables = [s]

    def get_spw_for_wvr(self, name):
        st = self.get_scantable(name)
        spw = st.spectral_window
        returned_list = []
        for (k,v) in spw.items():
            if v.type == 'WVR':
                returned_list.append(k)
        return returned_list

    def get_spw_without_wvr(self, name):
        st = self.get_scantable(name)
        spw = st.spectral_window
        returned_list = []
        for (k,v) in spw.items():
            if v.type == 'WVR':
                continue
            returned_list.append(k)
        return returned_list

    def get_calmode(self, name):
        st = self.get_scantable(name)
        return st.calibration_strategy['calmode']
        
    def get_scantable(self, name):
        if isinstance(name, str):
            for entry in self:
                if entry.basename == name:
                    return entry
            return None
        else:
            # should be integer index
            return self[name]

    

class SingleDishBase(object):
    def __repr__(self):
        s = '%s:\n'%(self.__class__.__name__)
        for (k,v) in self.__dict__.items():
            if k[0] == '_':
                key = k[1:]
            else:
                key = k
            s += '\t%s=%s\n'%(key,v)
        return s

    def _init_properties(self, properties={}, kw_ignore=['self']):
        for (k,v) in properties.items():
            if k not in kw_ignore:
                setattr(self, k, v)

class ScantableRep(SingleDishBase):
    tolerance = 1.0e-3

    def __init__(self, name, ms=None, session=None):
        self.name = name.rstrip('/')
        self.ms = ms
        self.session = session

        # observation property
        self.observer = None
        self.start_time = None
        self.end_time = None

        # polarization property
        self.polarization = {}

        # spectral window property
        self.spectral_window = {}

        # calibration property
        self.tsys_transfer = None
        self.tsys_transfer_list = []

        # source property
        self.source = {}

        # antenna property
        self.antenna = None

    @property
    def basename(self):
        return os.path.basename(self.name)

    @property
    def ms_name(self):
        if self.ms is not None:
            return self.ms.basename
        else:
            return None
            
    @property
    def baselined_name(self):
        return self.name + '_work'

    def __repr__(self):
        return 'Scantable(%s)'%(self.name)

class Polarization(SingleDishBase):
    to_polid = {'XX': 0, 'YY': 1, 'XY': 2, 'YX': 3, 
                'RR': 0, 'LL': 1, 'RL': 2, 'LR': 3,
                'I' : 0,  'Q': 1, 'U' : 2, 'V' : 3} 
    to_polenum = {'XX':  9, 'YY': 12, 'XY': 10, 'YX': 11, 
                  'RR':  5, 'LL':  8, 'RL':  6, 'LR':  7,
                  'I' :  1,  'Q':  2, 'U' :  3, 'V' :  4}
    polarization_map = { 'linear': { 0: ['XX',  9],
                                     1: ['YY', 12],
                                     2: ['XY', 10],
                                     3: ['YX', 11] },
                         'circular': { 0: ['RR', 5],
                                       1: ['LL', 8],
                                       2: ['RL', 6],
                                       3: ['LR', 7] },
                         'stokes': { 0: ['I', 1],
                                     1: ['Q', 2],
                                     2: ['U', 3],
                                     3: ['V', 4] },
                         'linpol': { 0: ['Ptotal',   28],
                                     1: ['Plinear',  29],
                                     2: ['PFtotal',  30],
                                     3: ['PFlinear', 31],
                                     4: ['Pangle',   32] } }
    @staticmethod
    def from_data_desc(datadesc):
        npol = datadesc.num_polarizations
        corr_axis = datadesc.corr_axis
        corr_type = datadesc.polarizations
        if 'X' in corr_type or 'Y' in corr_type:
            poltype = 'linear'
        elif 'R' in corr_type or 'L' in corr_type:
            poltype = 'circular'
        else:
            poltype = 'stokes'
        polno = [Polarization.to_polid[x] for x in corr_axis]
        corr_enum = [Polarization.to_polenum[x] for x in corr_axis]
        entry = Polarization(type=poltype,
                             polno=polno,
                             spw_association=[datadesc.spw.id],
                             corr_string=corr_axis,
                             corr_index=corr_enum)
        return entry
        

    def __init__(self, type=None, polno=None, corr_index=None, corr_string=None, spw_association=[]):
        self._init_properties(vars())

        
class Frequencies(spectralwindow.SpectralWindow,SingleDishBase):

    frame_map = { 0: 'REST',
                  1: 'LSRK',
                  2: 'LSRD',
                  3: 'BARY',
                  4: 'GEO',
                  5: 'TOPO',
                  6: 'GALACTO',
                  7: 'LGROUP',
                  8: 'CMB' }

    def __getstate__(self):
        state_dictionary = self.__dict__.copy()
        for attribute in self.__slots__:
            state_dictionary[attribute] = getattr(self, attribute)
        return state_dictionary

    def __setstate__(self, d):
        for (k,v) in d.items():
            if not hasattr(self, k):
                setattr(self, k, v)
        self.__dict__ = d
    
    @staticmethod
    def from_spectral_window(spw):
        nchan = spw.num_channels
        spw_type = ('TP' if nchan == 1 else \
                    ('WVR' if nchan == 4 else 'SP'))
        channel0 = spw.channels[0]
        refpix = 0
        refval = to_numeric_freq(channel0.getCentreFrequency())
        if nchan == 1:
            increment = to_numeric_freq(channel0.getWidth())
        else:
            center_freq0 = channel0.getCentreFrequency()
            center_freq1 = spw.channels[1].getCentreFrequency()
            increment = to_numeric_freq(center_freq1 - center_freq0)

        entry = Frequencies(id=spw.id,
                            type=spw_type,
                            nchan=nchan,
                            bandwidth=float(to_numeric_freq(spw.bandwidth)),
                            refpix=refpix,
                            refval=refval,
                            increment=increment,
                            freq_min=to_numeric_freq(spw.min_frequency),
                            freq_max=to_numeric_freq(spw.max_frequency),
                            name=spw.name,
                            sideband=spw.sideband,
                            baseband=spw.baseband)
        return entry
        
    def __init__(self, id=None, type=None, frame=None, nchan=None, refpix=None, refval=None, increment=None, bandwidth=None, intent=None, freq_min=None, freq_max=None, pol_association=None, rest_frequencies=None,name=None,sideband=None,baseband=None):
        if increment is not None and nchan is not None:
            chan_widths = [[increment]] * nchan
        else:
            chan_widths = None
        if refpix is not None and refval is not None:
            chan_freqs = [[refval + refpix * increment * ichan] for ichan in xrange(nchan)]
        else:
            chan_freqs = None
        spectralwindow.SpectralWindow.__init__(self, id, bandwidth, freq_min, chan_widths, chan_freqs, name, sideband, baseband)
        self._init_properties(vars(),kw_ignore=['self','bandwidth'])
        intents = self.intent.split(':')
        for intent in intents:
            if self.type == 'WVR':
                self.intents.add(self.type)
            else:
                self.intents.add(intent)

        # dummy group
        self.group = frequencygroup.FrequencyGroup(0,'default')

    @property
    def frequency_range(self):
        return [self.freq_min, self.freq_max]

    @property
    def intent(self):
        return self._intent

    @intent.setter
    def intent(self, value):
        self._intent = '' if value is None else value

    @property
    def pol_association(self):
        return self._pol_association

    @pol_association.setter
    def pol_association(self, value):
        self._pol_association = [] if value is None else value

    @property
    def is_target(self):
        #return (self.type == 'SP' and self.intent.find('TARGET') != -1)
        return (self.intent.find('TARGET') != -1)

    @property
    def is_atmcal(self):
        return (self.type == 'SP' and self.intent.find('ATMOSPHERE') != -1)


class ReductionGroupMember(object):
    def __init__(self, antenna, spw, pols):
        self.antenna = antenna
        self.spw = spw
        self.pols = pols
        self.iteration = [0 for p in xrange(self.npol)]
        self.linelist = [[] for p in xrange(self.npol)]
        self.channelmap_range = [[] for p in xrange(self.npol)]

    @property
    def npol(self):
        return len(self.pols)

    def iter_countup(self, pols=None):
        for i in self.__gen_pols(pols):
            self.iteration[i] += 1

    def iter_reset(self):
        self.iteration = [0 for p in self.pols]

    def add_linelist(self, linelist, pols=None, channelmap_range=None):
        for i in self.__gen_pols(pols):
            self.linelist[i] = linelist
            if channelmap_range is not None:
                self.channelmap_range[i] = channelmap_range
            else:
                self.channelmap_range[i] = linelist

    def __gen_pols(self, pols):
        if pols is None:
            for i in xrange(self.npol):
                yield i
        elif isinstance(pols, int):
            yield pols
        else:
            for i in pols:
                yield self.pols.index(i)

    def __repr__(self):
        return 'ReductionGroupMember(antenna=%s, spw=%s, pols=%s)'%(self.antenna, self.spw, self.pols)

    def __eq__(self, other):
        return other.antenna == self.antenna and other.spw == self.spw and other.pols == self.pols 
        
class ReductionGroupDesc(list):
    def __init__(self, frequency_range=None, nchan=None):
        self.frequency_range = frequency_range
        self.nchan = nchan

    def add_member(self, antenna, spw, pols):
        new_member = ReductionGroupMember(antenna, spw, pols)
        if not new_member in self:
            self.append(new_member)

    def get_iteration(self, antenna, spw, pol=None):
        member = self[self.__search_member(antenna, spw)]
        if pol is None:
            return max(member.iteration)
        else:
            return member.iteration[pol]
            
    def iter_countup(self, antenna, spw, pols=None):
        member = self[self.__search_member(antenna, spw)]
        member.iter_countup(pols)

    def add_linelist(self, linelist, antenna, spw, pols=None, channelmap_range=None):
        member = self[self.__search_member(antenna, spw)]
        member.add_linelist(linelist, pols, channelmap_range=channelmap_range)

    def __search_member(self, antenna, spw):
        for indx in xrange(len(self)):
            member = self[indx]
            if member.antenna == antenna and member.spw == spw:
                return indx
                break

    def __repr__(self):
        return 'ReductionGroupDesc(frequency_range=%s, nchan=%s, member=%s)'%(self.frequency_range, self.nchan, self[:])
