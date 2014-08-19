from __future__ import absolute_import

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

to_polid = {'XX': 0, 'YY': 1, 'XY': 2, 'YX': 3, 
            'RR': 0, 'LL': 1, 'RL': 2, 'LR': 3,
            'I' : 0,  'Q': 1, 'U' : 2, 'V' : 3} 


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

    def get_polarization_label(self, pol_id):
        """
        Get the polarization label associated with the polarization ID. This
        converts an integer to a string, eg. 0 -> 'XX'.
        """
        corr_type = self.polarizations
        if 'X' in corr_type or 'Y' in corr_type:
            poltype = 'linear'
        elif 'R' in corr_type or 'L' in corr_type:
            poltype = 'circular'
        else:
            poltype = 'stokes'

        label, _ = polarization_map[poltype][pol_id]
        return label

    def get_polarization_id(self, pol):
        return to_polid[pol]
