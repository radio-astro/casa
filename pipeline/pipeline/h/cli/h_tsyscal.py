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
import task_h_tsyscal
def h_tsyscal(vis=[''], caltable=[''], chantol=1, pipelinemode='automatic', dryrun=False, acceptresults=True):

        """Derive a Tsys calibration table

Derive the Tsys calibration for list of ALMA MeasurementSets.

Keyword arguments:

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context defined pipeline inputs
   automatically.  In interactive mode the user can set the pipeline
   context defined parameters manually.  In 'getinputs' mode the user
   can check the settings of all pipeline parameters without running
   the task.
   default: 'automatic'.

chantol -- The tolerance in channels for mapping atmospheric calibration
   windows (TDM) to science windows (FDM or TDM)
   default: 1
   example: 5

---- pipeline parameter arguments which can be set in any pipeline mode

---- pipeline context defined parameter arguments which can be set only in
'interactive mode'

vis -- List of input visibility files
    default: none; example: vis='ngc5921.ms'

caltable -- Name of output gain calibration tables
    default: none; example: caltable='ngc5921.gcal'

-- Pipeline task execution modes

dryrun -- Run the commands (True) or generate the commands to be run but
   do not execute (False).
   default: True

acceptresults -- Add the results of the task to the pipeline context (True) or
   reject them (False).
   default: True

Output:

results -- If pipeline mode is 'getinputs' then None is returned. Otherwise
    the results object for the pipeline task is returned.

Description

Derive the Tsys calibration for list of ALMA MeasurementSets.

Issues

Example


        """
        if type(vis)==str: vis=[vis]
        if type(caltable)==str: caltable=[caltable]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['caltable'] = caltable
        mytmp['chantol'] = chantol
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/h/cli/"
	trec = casac.utils().torecord(pathname+'h_tsyscal.xml')

        casalog.origin('h_tsyscal')
        if trec.has_key('h_tsyscal') and casac.utils().verify(mytmp, trec['h_tsyscal']) :
	    result = task_h_tsyscal.h_tsyscal(vis, caltable, chantol, pipelinemode, dryrun, acceptresults)

	else :
	  result = False
        return result
