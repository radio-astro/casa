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
import task_h_import_calstate
def h_import_calstate(filename=''):

        """Import a calibration state from disk
Import a calibration state to disk.
		
Keyword arguments:

filename -- Name of the saved calibration state.

Description

h_import calstate clears and then recreates the pipeline calibration state
based on the set of applycal calls given in the named file. The applycal
statements are interpreted in additive fashion; for identically specified
data selection targets, caltables specified in later statements will be added
to the state created by earlier calls.

Issues

Example

1. Import a calibration state from disk.

    h_import_calstate(filename='aftergaincal.calstate')


        """

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['filename'] = filename
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/h/cli/"
	trec = casac.utils().torecord(pathname+'h_import_calstate.xml')

        casalog.origin('h_import_calstate')
        if trec.has_key('h_import_calstate') and casac.utils().verify(mytmp, trec['h_import_calstate']) :
	    result = task_h_import_calstate.h_import_calstate(filename)

	else :
	  result = False
        return result
