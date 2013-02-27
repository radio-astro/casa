from __future__ import absolute_import
import numpy

import pipeline.infrastructure.casatools as casatools
    
class AntennaArray(object):    
    def __init__(self, name):
        self.name = name
        self.antennas = []

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
        
