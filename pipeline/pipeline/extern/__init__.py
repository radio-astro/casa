from __future__ import absolute_import
import pkg_resources
import sys

sys.path.append(pkg_resources.resource_filename(__name__, 'Mako-1.0.0-py2.7.egg'))
import mako

sys.path.append(pkg_resources.resource_filename(__name__, 'cachetools-1.0.1-py2.7.egg'))
import cachetools

sys.path.append(pkg_resources.resource_filename(__name__, 'sortedcontainers-1.4.4-py2.7.egg'))
import sortedcontainers

sys.path.append(pkg_resources.resource_filename(__name__, 'intervaltree-2.1.0-py2.7.egg'))
import intervaltree

sys.path.append(pkg_resources.resource_filename(__name__, 'pyparsing-2.1.7-py2.7.egg'))
import pyparsing

from . import logutils
from . import XmlObjectifier
