"""Module with class to instantiate the casapy tools."""

# History:
# 16-Jul-2007 jfl First version.
# 16-Aug-2007 jfl Remove import of hut.
#  6-Nov-2007 jfl Best bandpass release.
#  8-Feb-2008 jfl Mosaic release
# 20-Mar-2008 jfl BookKeeper release.
# 10-Sep-2008 jfl msCalibrater release.
# 26-Sep-2008 jfl mosaic release.

# package modules

from numpy import *

# alma modules

import casac


class BaseTools:
    """Class to instantiate the casapy tools."""

    def __init__(self):
        """Constructor.
        """

#        print 'BaseTools constructor called'

# get tools

        self._calibraterTool = casac.homefinder.find_home_by_name(
         'calibraterHome')
        self._calibrater = self._calibraterTool.create()
        self._deconvolverTool = casac.homefinder.find_home_by_name(
         'deconvolverHome')
        self._deconvolver = self._deconvolverTool.create()
        self._flaggerTool = casac.homefinder.find_home_by_name('flaggerHome')
        self._flagger = self._flaggerTool.create()
        self._imageTool = casac.homefinder.find_home_by_name('imageHome')
        self._image = self._imageTool.create()
        self._imagerTool = casac.homefinder.find_home_by_name('imagerHome')
        self._imager = self._imagerTool.create()
        self._measuresTool = casac.homefinder.find_home_by_name('measuresHome')
        self._measures = self._measuresTool.create()
        self._msTool = casac.homefinder.find_home_by_name('msHome')
        self._ms = self._msTool.create()
        self._quantaTool = casac.homefinder.find_home_by_name('quantaHome')
        self._quanta = self._quantaTool.create()
        self._regionmanagerTool = casac.homefinder.find_home_by_name(
         'regionmanagerHome')
        self._regionmanager = self._regionmanagerTool.create()
        self._tableTool = casac.homefinder.find_home_by_name('tableHome')
        self._table = self._tableTool.create()


    def copy(self, otherSelf):
        """Copy tools to otherSelf.
        """
        otherSelf._calibrater = self._calibrater
        otherSelf._deconvolver = self._deconvolver
        otherSelf._flagger = self._flagger
        otherSelf._image = self._image
        otherSelf._imager = self._imager
        otherSelf._measures = self._measures
        otherSelf._ms = self._ms
        otherSelf._quanta = self._quanta
        otherSelf._regionmanager = self._regionmanager
        otherSelf._table = self._table

