from __future__ import absolute_import

import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)

class MSReductionGroupMember(object):
    def __init__(self, ms, antenna_id, spw_id, field_id=None):
        self.ms = ms
        self.antenna_id = antenna_id
        self.spw_id = spw_id
        self.field_id = -1 if field_id is None else field_id
        self.iteration = 0
        self.linelist = []
        self.channelmap_range = []

    @property
    def npol(self):
        return 1
    
    @property
    def spw(self):
        return self.ms.spectral_windows[self.spw_id]
        
    @property
    def antenna(self):
        return self.ms.antennas[self.antenna_id]
    
    @property
    def antenna_name(self):
        return self.antenna.name
    
    @property
    def field(self):
        return self.ms.fields[self.field_id]
    
    @property
    def field_name(self):
        return self.field.name

    def iter_countup(self):
        self.iteration += 1

    def iter_reset(self):
        self.iteration = 0

    def add_linelist(self, linelist, pols=None, channelmap_range=None):
        self.linelist = linelist
        if channelmap_range is not None:
            self.channelmap_range = channelmap_range
        else:
            self.channelmap_range = linelist

    def __repr__(self):
        return 'MSReductionGroupMember(ms=\'%s\', antenna=%s, spw=%s, field_id=%s)' % (self.ms.basename, self.antenna_id, self.spw_id, self.field_id)

    def __eq__(self, other):
        #LOG.debug('MSReductionGroupMember.__eq__')
        return other.ms.name == self.ms.name and other.antenna_id == self.antenna_id and other.spw_id == self.spw_id and other.field_id == self.field_id

    def __ne__(self, other):
        return other.ms.name != self.ms.name or other.antenna_id != self.antenna_id or other.spw_id != self.spw_id or other.field_id != self.field_id
     
class MSReductionGroupDesc(list):
    def __init__(self, spw_name=None, min_frequency=None, max_frequency=None, nchan=None, field=None):
        self.spw_name = spw_name
        self.max_frequency = max_frequency
        self.min_frequency = min_frequency
        self.nchan = nchan
        self.field = field
        
    @property
    def frequency_range(self):
        return [self.min_frequency, self.max_frequency]
    
    @property
    def field_name(self):
        return self.field.name.strip('"')    

    def merge(self, other):
        assert self == other
        for member in other:
            LOG.trace('ms.name=\"%s\" antenna=%s spw=%s, field_id=%s'%(member.ms.name, member.antenna_id, member.spw_id, member.field_id))
            if not member in self:
                LOG.debug('Adding (%s, %s, %s, %s)'%(member.ms.name,member.antenna_id,member.spw_id,member.field_id))
                self.append(member)

    def add_member(self, ms, antenna_id, spw_id, field_id=None):
        new_member = MSReductionGroupMember(ms, antenna_id, spw_id, field_id)
        if not new_member in self:
            self.append(new_member)

    def get_iteration(self, ms, antenna_id, spw_id, field_id=None):
        member = self[self.__search_member(ms, antenna_id, spw_id, field_id)]
        return member.iteration
            
    def iter_countup(self, ms, antenna_id, spw_id, field_id=None):
        member = self[self.__search_member(ms, antenna_id, spw_id, field_id)]
        member.iter_countup()

    def add_linelist(self, linelist, ms, antenna_id, spw_id, field_id=None, channelmap_range=None):
        member = self[self.__search_member(ms, antenna_id, spw_id, field_id)]
        member.add_linelist(linelist, channelmap_range=channelmap_range)

    def __search_member(self, ms, antenna_id, spw_id, field_id=None):
        for indx in xrange(len(self)):
            member = self[indx]
            if member.ms.name == ms.name and member.antenna_id == antenna_id and member.spw_id == spw_id and member.field_id == field_id:
                return indx
                break
            
    def __eq__(self, other):
        #LOG.debug('MSReductionGroupDesc.__eq__')
        if (not isinstance(self.spw_name, str)) or len(self.spw_name) == 0:
            return self.max_frequency == other.max_frequency \
                and self.min_frequency == other.min_frequency \
                and self.nchan == other.nchan \
                and self.field_name == other.field_name
        else:
            return self.spw_name == other.spw_name \
                and self.field_name == other.field_name
            
    def __ne__(self, other):
        if (not isinstance(self.spw_name, str)) or len(self.spw_name) == 0:
            return self.max_frequency != other.max_frequency \
                or self.min_frequency != other.min_frequency \
                or self.nchan != other.nchan \
                or self.field_name != other.field_name
        else:
            return self.spw_name != other.spw_name \
                or self.field_name != other.field_name

    def __repr__(self):
        if (not isinstance(self.spw_name, str)) or len(self.spw_name) == 0:
            return 'MSReductionGroupDesc(frequency_range=%s, nchan=%s, field=\'%s\', member=%s)' % (self.frequency_range, self.nchan, self.field_name, self[:])
        else:
            return 'MSReductionGroupDesc(spw_name=%s, frequency_range=%s, nchan=%s, field=\'%s\', member=%s)' % (self.spw_name, self.frequency_range, self.nchan, self.field_name, self[:])
  