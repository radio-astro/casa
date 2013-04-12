from __future__ import absolute_import
import collections
import itertools
import math

import numpy
import simutil

import pipeline.infrastructure.casatools as casatools
    
Baseline = collections.namedtuple('Baseline', 'antenna1 antenna2 length')

class AntennaArray(object):    
    def __init__(self, name):
        self.name = name
        self.antennas = []

    @property
    def name(self):
        return self._name

    @name.setter
    def name(self, value):
        self._name = value
        
        me = casatools.measures
        self._position = me.observatory(self.name)

    @property
    def position(self):
        '''
        Get the array locate as a CASA measures position.
        ''' 
        return self._position

    @property
    def elevation(self):
        '''
        Get the array elevation as a CASA quantity.
        '''
        return self.position['m2']

    @property
    def latitude(self):
        '''
        Get the array latitude as a CASA quantity.
        '''
        return self.position['m1']

    @property
    def longitude(self):
        '''
        Get the array longitude as a CASA quantity.
        '''
        return self.position['m0']

    @property
    def centre(self):
        datum = self._position['refer']
        if datum == 'ITRF':
            return self._position

        qa = casatools.quanta       
        longitude = qa.convert(self.longitude, 'rad')
        latitude = qa.convert(self.latitude, 'rad')
        elevation = qa.convert(self.elevation, 'm')
        
        s = simutil.simutil()
        return s.long2xyz(qa.getvalue(longitude)[0],
                          qa.getvalue(latitude)[0],
                          qa.getvalue(elevation)[0],
                          datum)

    @property
    def min_baseline(self):
        return min(self.baselines, key=lambda b: b.length)

    @property
    def max_baseline(self):
        return max(self.baselines, key=lambda b: b.length)

    @property
    def baselines(self):
        qa = casatools.quanta
        
        def diff(ant1, ant2, attr):
            v1 = qa.getvalue(ant1.position[attr])[0]
            v2 = qa.getvalue(ant2.position[attr])[0]
            return v1-v2
        
        baselines = []
        for (ant1, ant2) in itertools.combinations(self.antennas, 2):
            length = math.sqrt(diff(ant1, ant2, 'm0')**2 + 
                               diff(ant1, ant2, 'm1')**2 + 
                               diff(ant1, ant2, 'm2')**2)
            baselines.append(Baseline(ant1, ant2, length))

        if len(baselines) == 0:
            baselines.append(Baseline(self.antennas[0], self.antennas[0], 0.0))

        return baselines

    def get_offset(self, antenna):
        '''
        Get the offset of the given antenna from the centre of the array.
        '''
        qa = casatools.quanta       
        longitude = qa.convert(self.longitude, 'rad')
        latitude = qa.convert(self.latitude, 'rad')
        elevation = qa.convert(self.elevation, 'm')
        datum = self._position['refer']

        (cx, cy, cz) = (qa.getvalue(longitude)[0],
                        qa.getvalue(latitude)[0],
                        qa.getvalue(elevation)[0])        

        s = simutil.simutil()
        if datum != 'ITRF':
            (cx, cy, cz) = s.long2xyz(cx, cy, cz, datum)
        
        ant_x = qa.getvalue(antenna.position['m0'])[0]
        ant_y = qa.getvalue(antenna.position['m1'])[0]
        ant_z = qa.getvalue(antenna.position['m2'])[0]

        xs, ys = s.itrf2loc((ant_x,), (ant_y,), (ant_z,), cx, cy, cz)
        return (xs[0], ys[0])
    
    def add_antenna(self, antenna):
        self.antennas.append(antenna)

    def get_antenna(self, id=None, name=None):
        if id is not None:
            l = [ant for ant in self.antennas if ant.id == id]
            if not l:
                raise IndexError, 'No antenna with ID {0}'.format(id)
            return l[0]

        if name is not None:
            l = [ant for ant in self.antennas if ant.name == name]
            if not l:
                raise IndexError, 'No antenna with name {0}'.format(name)  
            return l[0]

        raise Exception, 'No id or name given to get_antenna'

    @property
    def median_direction(self):
        """The median center direction for the array."""
        # construct lists of the longitude and latitude of each antenna.. 
        qt = casatools.quanta
        longs = [qt.getvalue(antenna.longitude) for antenna in self.antennas]
        lats = [qt.getvalue(antenna.latitude) for antenna in self.antennas]

        # .. and find the median of these lists 
        med_lon = numpy.median(numpy.array(longs))
        med_lat = numpy.median(numpy.array(lats))

        # Construct and return a CASA direction using these median values. As
        # antenna positions are given in radians, the units of the median
        # direction is set to radians too.  
        mt = casatools.measures
        return mt.direction(v0=qt.quantity(med_lon, 'rad'), 
                            v1=qt.quantity(med_lat, 'rad'))

    def __repr__(self):
        names = ', '.join([antenna.name for antenna in self.antennas])
        return 'AntennaArray({0})'.format(names)
        
