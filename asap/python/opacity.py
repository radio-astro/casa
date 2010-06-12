import os
import math
from asap import scantable
from asap import merge
from asap import fitter
from asap import selector
from asap import rcParams
from asap._asap import atmosphere


class model(object):
    def _to_pascals(self, val):
        if val > 2000:
            return val
        return val*100

    def __init__(self, temperature=288, pressure=101325., humidity=0.5,
                 elevation=700.):
        """
        This class implements opacity/atmospheric brightness temperature model
        equivalent to the model available in MIRIAD. The actual math is a
        convertion of the Fortran code written by Bob Sault for MIRIAD.
        It implements a simple model of the atmosphere and Liebe's model (1985)
        of the complex refractive index of air.

        The model of the atmosphere is one with an exponential fall-off in
        the water vapour content (scale height of 1540 m) and a temperature
        lapse rate of 6.5 mK/m. Otherwise the atmosphere obeys the ideal gas
        equation and hydrostatic equilibrium.

        Note, the model includes atmospheric lines up to 800 GHz, but was not
        rigorously tested above 100 GHz and for instruments located at
        a significant elevation. For high-elevation sites it may be necessary to
        adjust scale height and lapse rate.

        Parameters:
            temperature:    air temperature at the observatory (K)
            pressure:       air pressure at the sea level if the observatory
                            elevation is set to non-zero value (note, by
                            default is set to 700m) or at the observatory
                            ground level if the elevation is set to 0. (The
                            value is in Pascals or hPa, default 101325 Pa
            humidity:       air humidity at the observatory (fractional),
                            default is 0.5
            elevation:     observatory elevation about sea level (in meters)
        """
        self._atm = atmosphere(temp, self._to_pascals(pressure), humidity)
        self.set_observatory_elevation(elevation)

    def get_opacities(self, freq, elevation=None):
        """Get the opacity value(s) for the fiven frequency(ies).
        If no elevation is given the opacities for the zenith are returned.
        If an elevation is specified refraction is also taken into account.
        Parameters:
            freq:       a frequency value in Hz, or a list of frequency values.
                        One opacity value per frequency is returned as a scalar
                        or list.
            elevation:  the elevation at which to compute the opacity. If `None`
                        is given (default) the zenith opacity is returned.


        """
        func = None
        if isinstance(freq, (list, tuple)):
            if elevation is None:
                return self._atm.zenith_opacities(freq)
            else:
                elevation *= math.pi/180.
                return self._atm.opacities(freq, elevation)
        else:
            if elevation is None:
                return self._atm.zenith_opacity(freq)
            else:
                elevation *= math.pi/180.
                return self._atm.opacity(freq, elevation)

    def set_weather(self, temperature, pressure, humidity):
        """Update the model using the given environmental parameters.
        Parameters:
            temperature:    air temperature at the observatory (K)
            pressure:       air pressure at the sea level if the observatory
                            elevation is set to non-zero value (note, by
                            default is set to 700m) or at the observatory
                            ground level if the elevation is set to 0. (The
                            value is in Pascals or hPa, default 101325 Pa
            humidity:       air humidity at the observatory (fractional),
                            default is 0.5
        """
        pressure = self._to_pascals(pressure)
        self._atm.set_weather(temperature, pressure, humidity)

    def set_observatory_elevation(self, elevation):
        """Update the model using the given the observatory elevation
        Parameters:
            elevation:  the elevation at which to compute the opacity. If `None`
                        is given (default) the zenith opacity is returned.
        """
        self._atm.set_observatory_elevation(el)


def _import_data(data):
    if not isinstance(data, (list,tuple)):
        if isinstance(data, scantable):
            return data
        elif isinstance(data, str):
            return scantable(data)
    tables = []
    for d in data:
        if isinstance(d, scantable):
            tables.append(d)
        elif isinstance(d, str):
            if os.path.exists(d):
                tables.append(scantable(d))
            else:
                raise IOError("Data file doesn't exists")
        else:
            raise TypeError("data is not a scantable or valid file")
    return merge(tables)

def skydip(data, averagepol=True, tsky=300., plot=False,
           temperature=288, pressure=101325., humidity=0.5):
    """Determine the opacity from a set of 'skydip' obervations.
    This can be any set of observations over a range of elevations,
    but will ususally be a dedicated (set of) scan(s).
    Return a list of 'n' opacities for 'n' IFs. In case of averagepol
    being 'False' a list of 'n*m' elements where 'm' is the number of
    polarisations, e.g.
    nIF = 3, nPol = 2 => [if0pol0, if0pol1, if1pol0, if1pol1, if2pol0, if2pol1]

    The opacity is determined by fitting a first order polynomial to:


        Tsys(airmass) = p0 + airmass*p1

    where

        airmass = 1/sin(elevation)

        tau =  p1/Tsky

    Parameters:
        data:       a list of file names or scantables or a single
                    file name or scantable.
        averagepol: Return the average of the opacities for the polarisations
                    This might be useful to set to 'False' if one polarisation
                    is corrupted (Mopra). If set to 'False', an opacity value
                    per polarisation is returned.
        tksy:       The sky temperature (default 300.0K). This might
                    be read from the data in the future.
        plot:       Plot each fit (airmass vs. Tsys). Default is 'False'
    """
    rcsave = rcParams['verbose']
    rcParams['verbose'] = False
    if plot:
        from matplotlib import pylab
    scan = _import_data(data)
    f = fitter()
    f.set_function(poly=1)
    sel = selector()
    basesel = scan.get_selection()
    inos = scan.getifnos()
    pnos = scan.getpolnos()
    opacities = []
    om = opacitymodel(temperature, pressure, humidity)
    for ino in inos:
        sel.set_ifs(ino)
        opacity = []
        fits = []
        airms = []
        tsyss = []
        if plot:
            pylab.cla()
            pylab.ioff()
            pylab.clf()
            pylab.xlabel("Airmass")
            pylab.ylabel(r"$T_{sys}$")
        for pno in pnos:
            sel.set_polarisations(pno)
            scan.set_selection(basesel+sel)
            freq = scan.get_coordinate(0).get_reference_value()/1e9
            freqstr = "%0.4f GHz" % freq
            tsys = scan.get_tsys()
            elev = scan.get_elevation()
            airmass = [ 1./math.sin(i) for i in elev ]
            airms.append(airmass)
            tsyss.append(tsys)
            f.set_data(airmass, tsys)
            f.fit()
            fits.append(f.get_fit())
            params = f.get_parameters()["params"]
            opacity.append(params[1]/tsky)
        if averagepol:
            opacities.append(sum(opacity)/len(opacity))
        else:
            opacities += opacity
        if plot:
            colors = ['b','g','k']
            n = len(airms)
            for i in range(n):
                pylab.plot(airms[i], tsyss[i], 'o', color=colors[i])
                pylab.plot(airms[i], fits[i], '-', color=colors[i])
                pylab.figtext(0.7,0.3-(i/30.0),
                                  r"$\tau_{fit}=%0.2f$" % opacity[i],
                                  color=colors[i])
            if averagepol:
                pylab.figtext(0.7,0.3-(n/30.0),
                                  r"$\tau_{avg}=%0.2f$" % opacities[-1],
                                  color='r')
                n +=1
            pylab.figtext(0.7,0.3-(n/30.0),
                          r"$\tau_{model}=%0.2f$" % om.get_opacities(freq*1e9),
                          color='grey')

            pylab.title("IF%d : %s" % (ino, freqstr))

            pylab.ion()
            pylab.draw()
            raw_input("Hit <return> for next fit...")
        sel.reset()

    scan.set_selection(basesel)
    rcParams['verbose'] = rcsave
    if plot:
        pylab.close()
    return opacities
