"""
A package for requesting ephemerides of Solar System objects from
JPL-Horizons, and, after the results arrive by email, installing
them as CASA tables in
os.getenv('CASAPATH').split()[0] + '/data/ephemerides/JPL-Horizons/'.
"""

import request, JPLephem_reader
