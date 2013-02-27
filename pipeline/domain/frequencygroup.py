from __future__ import absolute_import

class FrequencyGroup(object):
    '''FrequencyGroup is a container for SpectralWindows. It has logic to 
    determine which SpectralWindows should be classed as narrow-bandwidth line
    windows and which should be large-bandwidth continuum windows.'''     

    def __init__(self, group_id, name):
        '''Create a new frequency group with the given ID and name.'''
        self._windows = []
        self.id = group_id        
        self.name = name
        # the measurement set containing this frequency group
        self.measurement_set = None
    
    @property
    def windows(self):
        return self._windows
    
    def add_spw(self, spw):
        '''Add a spectral window to this frequency group.'''        
        # tag the window with details of its containing frequency group
        spw.group = self

        self._windows.append(spw)

    def get_continuum_windows(self):
        '''Get the continuum spectral windows contained in this frequency
        group, sorted by bandwidth'''
        # get the maximum bandwidth of all spectral windows except WVR
        # channels, identified by the number of channels
        max_bandwidth = max([spw.bandwidth for spw in self._windows
                             if spw.num_channels != 4])

        # we consider a continuum window to have 1 channel with a large 
        # (greater than 90% of the maximum) bandwidth
        continuum_spws = [spw for spw in self._windows 
                          if spw.num_channels is 1 
                          and spw.bandwidth > 0.9 * max_bandwidth]

        # return the windows sorted by bandpass
        return sorted(continuum_spws, key=lambda window: window.bandwidth)

    def get_line_windows(self):
        '''Get the line spectral windows contained in this frequency group,
        sorted by bandwidth'''
        # by definition, anything that's not a continuum or WVR window is a
        # line window
        line_windows = [spw for spw in self._windows 
                        if spw not in self.get_continuum_windows()
                        and spw.num_channels != 4]        

        # return the line windows sorted by bandpass
        return sorted(line_windows, key=lambda window: window.bandwidth)

    def get_spectral_window(self, spw_id):
        '''Get the spectral window with the given ID.'''
        if spw_id is not None:
            spw_id = int(spw_id)
            match = [spw for spw in self.windows 
                     if spw.id is spw_id]
            if match:
                return match[0]
            else:
                raise ValueError('No spectral window known with the '
                                 'ID {0}'.format(spw_id))

    def __repr__(self):
        return 'FrequencyGroup(' + str(self._windows) + ')'
        
