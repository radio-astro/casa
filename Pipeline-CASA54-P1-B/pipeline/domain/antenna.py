"""The antenna module defines the Antenna class.
"""
from __future__ import absolute_import
import pprint

import pipeline.infrastructure.casatools as casatools

_pprinter = pprint.PrettyPrinter()

    
class Antenna(object):
    """
    Antenna is a logical representation of an antenna. 
    
    An Antenna has the following properties:

    .. py:attribute:: id
    
        the numerical identifier of this antenna within the ANTENNA subtable
        of the measurement set
        
    .. py:attribute:: name
    
        the (potentially empty) name of this antenna
        
    .. py:attribute:: longitude
    
        the longitude of this antenna

    .. py:attribute:: latitude
    
        the latitude of this antenna

    .. py:attribute:: height
    
        the radial distance of this antenna from the Earth's centre

    .. py:attribute:: diameter
    
        the physical diameter of this antenna

    .. py:attribute:: direction
    
        the J2000 position on the sky to which this antenna points
    """
    def __init__(self, antenna_id, name, station, position, offset, diameter):
        self.id = antenna_id

        # work around NumPy bug with empty strings
        # http://projects.scipy.org/numpy/ticket/1239
        self.name = str(name)
        self.station = str(station)

        self.diameter = diameter
        self.position = position
        self.offset = offset

        # The longitude, latitude and height of a CASA position are given in
        # canonical units, so we don't need to perform any further conversion
        self.longitude = position['m0']
        self.latitude = position['m1']
        self.height = position['m2']
        
        mt = casatools.measures
        self.direction = mt.direction(v0=self.longitude, v1=self.latitude)

    def __repr__(self):
        return '{0}({1}, {2!r}, {3!r}, {4}, {5}, {6})'.format(
            self.__class__.__name__,
            self.id,
            self.name,
            self.station,
            _pprinter.pformat(self.position),
            _pprinter.pformat(self.offset),
            self.diameter)

    def __str__(self):
        qt = casatools.quanta
        lon = qt.tos(self.longitude) 
        lat = qt.tos(self.latitude) 
        return '<Antenna {id} (lon={lon}, lat={lat})>'.format(
            id=self.identifier, lon=lon, lat=lat)

    @property
    def identifier(self):
        '''
        A human-readable identifier for this Antenna.
        '''
        return self.name if self.name else '#{0}'.format(self.id)

