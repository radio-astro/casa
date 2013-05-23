from __future__ import absolute_import

import re

class DataDescription(object):
    def __init__(self, dd_id, spw, pol_id):
        self.id = dd_id
        self.spw = spw
        self.pol_id = pol_id
        self.obs_time = None
        self.chan_freq = []
        self.corr_axis = []
        self.group_name = ''

    def __repr__(self):
        args = map(str, (self.id, self.spw.id, self.pol_id))
        return 'DataDescription({0})'.format(', '.join(args))

    @property
    def polarizations(self):
        all_corrs = ''.join(self.corr_axis)

        RL_regex = re.compile('[RL]')
        if RL_regex.match(all_corrs):
            reg = re.compile('.*[R].*')
            match = reg.match(all_corrs)
            if match:
                return ['R','L']
            else:
                return ['L','R']
        else:
            reg = re.compile('.*[X].*')
            match = reg.match(all_corrs)
            if match:
                return ['X','Y']
            else:
                return ['Y','X']

    @property
    def num_polarizations(self):
        all_corrs = ''.join(self.corr_axis)
        pols = self.polarizations
                    
        if pols == ['R','L']:
            reg2 = re.compile('.*[L].*')                        
            if reg2.match(all_corrs):
                return 2
            else:
                return 1

        if pols == ['L','R']:
            return 1

        if pols == ['X','Y']:
            reg2 = re.compile('.*[Y].*')
            if reg2.match(all_corrs):
                return 2
            else:
                return 1
        if pols == ['Y', 'X']:
            return 1
        
