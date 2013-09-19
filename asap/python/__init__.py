"""
This is the ATNF Single Dish Analysis package.

"""
import os
import sys

# first import!
from asap.env import *
# second import!
from asap.parameters import *
# third import
from asap.logging import *
from asap.utils import *
# explicitly import 'hidden' functions
from asap.utils import _n_bools, _is_sequence_or_number, _to_list

if is_ipython():
    from ipysupport import *

# Only use rcParams['verbose'] in standard asap cli mode
# not in scripts or casapy
if not is_asap_cli():
    rcParams['verbose'] = False

setup_env()

# anything which uses matplotlib has to be imported after this!!!
if rcParams['useplotter']:
    try:
        gui = os.environ.has_key('DISPLAY') and rcParams['plotter.gui']
        if gui:
            import matplotlib
            if 'matplotlib.backends' not in matplotlib.sys.modules:
                matplotlib.use("TkAgg")
        from asapplotter import asapplotter
        from matplotlib import pylab
        xyplotter = pylab
        plotter = asapplotter(gui)
    except ImportError:
        asaplog.push( "Matplotlib not installed. No plotting available")
        asaplog.post('WARN')

from selector import selector
from asapmath import *
from scantable import scantable
from linecatalog import linecatalog
from asaplinefind import linefinder
from simplelinefinder import simplelinefinder
from interactivemask import interactivemask
from asapfitter import fitter
from opacity import skydip
from opacity import model as opacity_model
from asapgrid import asapgrid, asapgrid2
from edgemarker import edgemarker
if is_casapy():
    from plotter2 import plotter2
from sbseparator import sbseparator
from _asap import srctype

__date__ = '$Date: 2013-07-31 01:37:40 -0600 (Wed, 31 Jul 2013) $'.split()[1]
__version__  = '4.2.0a'
__revision__ = get_revision()

def welcome():
    return """Welcome to ASAP v%s (%s) - the ATNF Spectral Analysis Package

Please report any bugs via:
http://svn.atnf.csiro.au/trac/asap/simpleticket

[IMPORTANT: ASAP is 0-based]
Type commands() to get a list of all available ASAP commands.""" % (__version__,
                                                                    __date__)
