from __future__ import absolute_import

import numpy as np

class Polarization(object):
    def __init__(self, pol_id, num_corr, corr_type, corr_product, flag_row):
        # Copied from C++ casa::Stokes class
        stokes_enum = np.array(['Undefined',
                       'I',
                       'Q',
                       'U',
                       'V',
                       'RR',
                       'RL',
                       'LR',
                       'LL',
                       'XX',
                       'XY',
                       'YX',
                       'YY',
                       'RX',
                       'RY',
                       'LX',
                       'LY',
                       'XR',
                       'XL',
                       'YR',
                       'YL',
                       'PP',
                       'PQ',
                       'QP',
                       'QQ',
                       'RCircular',
                       'LCircular',
                       'Linear',
                       'Ptotal',
                       'Plinear',
                       'PFtotal',
                       'PFlinear',
                       'Pangle'])
        self.id = pol_id
        self.num_corr = num_corr
        self.corr_type = corr_type
        self.corr_type_string = stokes_enum[corr_type]
        self.corr_product = corr_product 
        self.flag_row = flag_row

    def __repr__(self):
        args = map(str, (self.id, self.corr_type_string))
        return 'Polarization({0})'.format(', '.join(args))
