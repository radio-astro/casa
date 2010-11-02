"""
A package for requesting ephemerides of Solar System objects from
JPL-Horizons, and, after the results arrive by email, installing
them as CASA tables in
os.getenv('CASAPATH').split()[0] + '/data/ephemerides/JPL-Horizons/'.

There are two modules:
  request - Requests one or more ephemerides from JPL-Horizons by batch email.
            It also contains some lists of asteroids, moons, and planets, and
            functions (list_asteroids() and list_moons()) to pretty print them.

  JPLephem_reader - Converts ASCII ephemerides from JPL-Horizons into CASA
                    tables and installs them where casapy can find them.
"""

import request, JPLephem_reader
