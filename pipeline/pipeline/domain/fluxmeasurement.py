from __future__ import absolute_import
import decimal

from . import measures
import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)


class FluxMeasurement():
    def __init__(self, spw_id, I, Q=measures.FluxDensity(0),
                 U=measures.FluxDensity(0), V=measures.FluxDensity(0),
                 spix=decimal.Decimal('0.0'), origin=None):
        self.spw_id = spw_id
        self.I = self._to_flux_density(I)
        self.Q = self._to_flux_density(Q)
        self.U = self._to_flux_density(U)
        self.V = self._to_flux_density(V)
        self.spix = self._to_decimal(spix)
        if origin is None:
            self.origin = ('', '', 'N/A')  # Origin, Date, ageOfNearestMonitorPoint
        else:
            self.origin = origin
        
    def _to_flux_density(self, arg):
        """
        Return arg as a new FluxDensity. If arg is a number, it is assumed to
        be the flux density in Jy.
        """
        if isinstance(arg, measures.FluxDensity):
            # create defensive copies of the flux arguments so they're not
            # shared between instances 
            return measures.FluxDensity(arg.value, arg.units)
        
        try:
            return measures.FluxDensity(arg, measures.FluxDensityUnits.JANSKY)
        except:
            LOG.error("Could not convert %s to fluxdensity" % arg)
            raise

    def _to_decimal(self, arg):
        if isinstance(arg, decimal.Decimal):
            return arg
        elif isinstance(arg, (int, float, long)):
            return decimal.Decimal(map(str, arg))
        else:
            LOG.error("Could not convert %s to decimal" % arg)
            raise
    
    @property
    def casa_flux_density(self):
        iquv = [self.I.to_units(measures.FluxDensityUnits.JANSKY),
                self.Q.to_units(measures.FluxDensityUnits.JANSKY),
                self.U.to_units(measures.FluxDensityUnits.JANSKY),
                self.V.to_units(measures.FluxDensityUnits.JANSKY) ]
        return map(float, iquv)

    def __repr__(self):
        return '<FluxMeasurement(Spw #{spw}, IQUV=({iquv}), spix={spix}>'.format(
            spw=self.spw_id,
            iquv=','.join(map(str, (self.I, self.Q, self.U, self.V))),
            spix=float(self.spix))

    def __add__(self, other):
        if not isinstance(other, self.__class__):
            raise TypeError, "unsupported operand type(s) for +: '%s' and '%s'" % (self.__class__.__name__, 
                                                                                   other.__class__.__name__)

        spw_id = self.spw_id       
        I = self.I + other.I
        Q = self.Q + other.Q
        U = self.U + other.U
        V = self.V + other.V
        spix = self.spix
        
        return self.__class__(spw_id, I, Q, U, V, spix)

    def __div__(self, other):
        if not isinstance(other, (int, float, long, decimal.Decimal)):
            raise TypeError, "unsupported operand type(s) for /: '%s' and '%s'" % (self.__class__.__name__, 
                                                                                   other.__class__.__name__)

        spw_id = self.spw_id
        I = self.I / other
        Q = self.Q / other
        U = self.U / other
        V = self.V / other
        spix = self.spix
        
        return self.__class__(spw_id, I, Q, U, V, spix)

    def __mul__(self, other):
        if not isinstance(other, (int, float, long, decimal.Decimal)):
            raise TypeError, "unsupported operand type(s) for *: '%s' and '%s'" % (self.__class__.__name__, other.__class__.__name__)

        spw_id = self.spw_id
        I = self.I * other
        Q = self.Q * other
        U = self.U * other
        V = self.V * other
        spix = self.spix
        
        return self.__class__(spw_id, I, Q, U, V, spix)

    def __rmul__(self, other):
        if not isinstance(other, (int, float, long, decimal.Decimal)):
            raise TypeError, "unsupported operand type(s) for *: '%s' and '%s'" % (self.__class__.__name__, other.__class__.__name__)

        spw_id = self.spw_id
        I = self.I * other
        Q = self.Q * other
        U = self.U * other
        V = self.V * other
        spix = self.spix
        
        return self.__class__(spw_id, I, Q, U, V, spix)
