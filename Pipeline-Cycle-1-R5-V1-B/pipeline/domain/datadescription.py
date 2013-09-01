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

        pols = []

        # I have no doubt that this is wrong! We should revisit this when we
        # know all possible polarisation mappings and whether they should be
        # used instead of corr_axis.
        if 'R' in all_corrs:
            pols.append('R')
        if 'L' in all_corrs:
            pols.append('L')
        if 'X' in all_corrs:
            pols.append('X')
        if 'Y' in all_corrs:
            pols.append('Y')

        return pols

    @property
    def num_polarizations(self):
        return len(self.polarizations)
