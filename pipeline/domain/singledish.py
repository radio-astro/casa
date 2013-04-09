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
        self.basename = os.path.basename(name.rstrip('/'))
        self.name = name
        self.ms = ms
        self.ms_name = ms.basename if ms is not None else None
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
        

class Polarization(SingleDishBase):
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
        polno = numpy.arange(npol, dtype=int)
        corrs = datadesc.polarizations
        if 'X' in corrs:
            ptype = 'linear'
        elif 'R' in corrs:
            ptype = 'circular'
        else:
            ptype = 'stokes'
        corrstr = ['%s%s'%(corrs[i],corrs[i]) for i in xrange(min(2,npol))]
        if npol > 2:
            corrstr = corrstr + ['%s%s'%(corrs[i],corrs[j]) for (i,j) in [(0,1),(1,0)]]
        polmap = Polarization.polarization_map[ptype]
        entry = Polarization(type=ptype,
                             polno=polno,
                             spw_association=[datadesc.spw.id],
                             corr_string=corrstr,
                             corr_index=[polmap[i][0] for i in xrange(npol)])
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
    
    @staticmethod
    def from_spectral_window(spw):
        nchan = spw.num_channels
        spw_type = ('TP' if nchan == 1 else \
                    ('WVR' if nchan == 4 else 'SP'))
        channel0 = spw.channels[0]
        refpix = 0
        refval = to_numeric_freq(channel0.getCentreFrequency())
        increment = to_numeric_freq(channel0.getWidth())
        entry = Frequencies(id=spw.id,
                            type=spw_type,
                            nchan=nchan,
                            bandwidth=float(to_numeric_freq(spw.bandwidth)),
                            refpix=refpix,
                            refval=refval,
                            increment=increment,
                            freq_min=to_numeric_freq(spw.min_frequency),
                            freq_max=to_numeric_freq(spw.max_frequency))
        return entry
        
    def __init__(self, id=None, type=None, frame=None, nchan=None, refpix=None, refval=None, increment=None, bandwidth=None, intent=None, freq_min=None, freq_max=None, pol_association=None):
        if increment is not None and nchan is not None:
            chan_widths = [[increment]] * nchan
        else:
            chan_widths = None
        if refpix is not None and refval is not None:
            chan_freqs = [[refval + refpix * increment * ichan] for ichan in xrange(nchan)]
        else:
            chan_freqs = None
        super(Frequencies,self).__init__(id, bandwidth, freq_min, chan_widths, chan_freqs)
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
