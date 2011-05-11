from asap._asap import coordinate as _coordinate

class coordinate(_coordinate):
    """Representation of the spectral coordinate of the data (frequency axis).
    """
    def to_frequency(self, pixel, unit='Hz'):
        """
        Convert a channel/pixel value to a frequency.
        """
        umap = {'GHz': 1.e9, 'MHz' : 1.e6, 'kHz': 1.e3, 'Hz': 1. }
        return _coordinate.to_frequency(self, pixel)/umap[unit]


    def to_velocity(self, pixel, unit='km/s'):
        """
        Convert a channel/pixel value to a velocity.
        """
        umap = {'km/s': 1, 'm/s': 1e-3}
        return _coordinate.to_velocity(self, pixel)/umap[unit]
