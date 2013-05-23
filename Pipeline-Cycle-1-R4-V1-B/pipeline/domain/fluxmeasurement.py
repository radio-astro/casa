from __future__ import absolute_import
import decimal

from . import measures
import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)


class FluxMeasurement():
    def __init__(self, spw, I, Q=measures.FluxDensity(0),
                 U=measures.FluxDensity(0), V=measures.FluxDensity(0)):
        self.spw = spw
        self.I = self._to_flux_density(I)
        self.Q = self._to_flux_density(Q)
        self.U = self._to_flux_density(U)
        self.V = self._to_flux_density(V)
        
    def _to_flux_density(self, arg):
        """
        Return arg as a new FluxDensity. If arg is a number, it is assumed to
        be the flux density in Jy.
        """
        if isinstance(arg, measures.FluxDensity):
            # create defensive copies of the flux arguments so they're not
            # shared between instances 
            return measures.FluxDensity(arg.value, arg.units)
        
        return measures.FluxDensity(arg, measures.FluxDensityUnits.JANSKY)
    
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
