from __future__ import absolute_import


class Polarization(object):
    def __init__(self, pol_id, num_corr, corr_type, corr_product):
        # prefer standard Python integers to numpy integers
        self.id = int(pol_id)
        self.num_corr = int(num_corr)
        self.corr_type = corr_type
        self.corr_product = corr_product

        # Copied from C++ casa::Stokes class
        _stokes_enum = ['Undefined', 'I', 'Q', 'U', 'V', 'RR', 'RL', 'LR', 'LL', 'XX', 'XY', 'YX', 'YY', 'RX', 'RY',
                        'LX', 'LY', 'XR', 'XL', 'YR', 'YL', 'PP', 'PQ', 'QP', 'QQ', 'RCircular', 'LCircular', 'Linear',
                        'Ptotal', 'Plinear', 'PFtotal', 'PFlinear', 'Pangle']
        self.corr_type_string = [_stokes_enum[c] for c in corr_type]

    def __str__(self):
        return 'Polarization({!r}, {!r})'.format(self.id, self.corr_type_string)

    def __repr__(self):
        return ('Polarization({!r}, {!r}, {!r}, {!r})'
                ''.format(self.id, self.num_corr, self.corr_type, self.corr_product))
