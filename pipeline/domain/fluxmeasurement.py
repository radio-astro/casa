from __future__ import absolute_import
import decimal

from . import measures
import pipeline.infrastructure.logging as logging

LOG = logging.get_logger(__name__)


class FluxMeasurement():
    def __init__(self, spw, I, Q=measures.FluxDensity(0),
                 U=measures.FluxDensity(0), V=measures.FluxDensity(0)):
        self.spw = spw
        # create defensive copies of the flux arguments so they're not shared
        # between instances 
        self.I = measures.FluxDensity(I.value, I.units)
        self.Q = measures.FluxDensity(Q.value, Q.units)
        self.U = measures.FluxDensity(U.value, U.units)
        self.V = measures.FluxDensity(V.value, V.units)
        
    @property
    def casa_flux_density(self):
        iquv = [self.I.to_units(measures.FluxDensityUnits.JANSKY),
                self.Q.to_units(measures.FluxDensityUnits.JANSKY),
                self.U.to_units(measures.FluxDensityUnits.JANSKY),
                self.V.to_units(measures.FluxDensityUnits.JANSKY) ]
        return map(float, iquv)

    @property
    def frequency(self):
        return self.spw.ref_frequency.to_units(measures.FrequencyUnits.HERTZ)

    def __repr__(self):
        return '<FluxMeasurement(Spw #{spw}, IQUV=({iquv})>'.format(
            spw=self.spw.id, 
            iquv=','.join(map(str, (self.I, self.Q, self.U, self.V))))

    def __add__(self, other):
        if not isinstance(other, self.__class__):
            raise TypeError, "unsupported operand type(s) for +: '%s' and '%s'" % (self.__class__.__name__, 
                                                                                   other.__class__.__name__)

        if self.frequency != other.frequency:
            LOG.warn('Adding flux values with different frequencies: '
                     '{0} vs {1}'.format(self.frequency, other.frequency))


        spw = self.spw       
        I = self.I + other.I
        Q = self.Q + other.Q
        U = self.U + other.U
        V = self.V + other.V
        
        return self.__class__(spw, I, Q, U, V)

    def __div__(self, other):
        if not isinstance(other, (int, float, long, decimal.Decimal)):
            raise TypeError, "unsupported operand type(s) for /: '%s' and '%s'" % (self.__class__.__name__, 
                                                                                   other.__class__.__name__)

        spw = self.spw
        I = self.I / other
        Q = self.Q / other
        U = self.U / other
        V = self.V / other
        
        return self.__class__(spw, I, Q, U, V)

    def __mul__(self, other):
        if not isinstance(other, (int, float, long, decimal.Decimal)):
            raise TypeError, "unsupported operand type(s) for *: '%s' and '%s'" % (self.__class__.__name__, other.__class__.__name__)

        spw = self.spw
        I = self.I * other
        Q = self.Q * other
        U = self.U * other
        V = self.V * other
        
        return self.__class__(spw, I, Q, U, V)

    def __rmul__(self, other):
        if not isinstance(other, (int, float, long, decimal.Decimal)):
            raise TypeError, "unsupported operand type(s) for *: '%s' and '%s'" % (self.__class__.__name__, other.__class__.__name__)

        spw = self.spw
        I = self.I * other
        Q = self.Q * other
        U = self.U * other
        V = self.V * other
        
        return self.__class__(spw, I, Q, U, V)
