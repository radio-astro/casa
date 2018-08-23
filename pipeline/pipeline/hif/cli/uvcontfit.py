#
# This file was generated using xslt from its XML file
#
# Copyright 2009, Associated Universities Inc., Washington DC
#
import sys
import os
from  casac import *
import string
from taskinit import casalog
from taskinit import xmlpath
#from taskmanager import tm
import task_uvcontfit
def uvcontfit(vis='', caltable='', field='', intent='', spw='', combine='', solint='int', fitorder=0, append=False):

        """Fit the continuum in the UV plane

This task estimates the continuum emission by fitting polynomials to
the real and imaginary parts of the spectral windows and channels
selected by spw and exclude spw. This fit represents a model of
the continuum in all channels. Fit orders less than 2 are strongly
recommended.
	

        """

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['caltable'] = caltable
        mytmp['field'] = field
        mytmp['intent'] = intent
        mytmp['spw'] = spw
        mytmp['combine'] = combine
        mytmp['solint'] = solint
        mytmp['fitorder'] = fitorder
        mytmp['append'] = append
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli/"
	trec = casac.utils().torecord(pathname+'uvcontfit.xml')

        casalog.origin('uvcontfit')
        if trec.has_key('uvcontfit') and casac.utils().verify(mytmp, trec['uvcontfit']) :
	    result = task_uvcontfit.uvcontfit(vis, caltable, field, intent, spw, combine, solint, fitorder, append)

	else :
	  result = False
        return result
