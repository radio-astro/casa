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
import task_hif_findcont
def hif_findcont(vis=[''], target_list={}, parallel='automatic', pipelinemode='automatic', dryrun=False, acceptresults=True):

        """Find continuum frequency ranges

Compute continuum ranges for all sources and spectral windows.

Keyword arguments:

--- pipeline parameter arguments which can be set in any pipeline mode

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
       determines the values of all context defined pipeline inputs
       automatically.  In interactive mode the user can set the pipeline
       context defined parameters manually.  In 'getinputs' mode the user
       can check the settings of all pipeline parameters without running
       the task.
       default: 'automatic'.


---- pipeline context defined parameter arguments which can be set only in
'interactive mode'

vis -- The list of input MeasurementSets. Defaults to the list of MeasurementSets
    specified in the h_init or hif_importdata sets.
    example: vis='ngc5921.ms'
             vis=['ngc5921a.ms', ngc5921b.ms', 'ngc5921c.ms']
    default: use all MeasurementSets in the context 

parallel -- use multiple CPU nodes to compute dirty images
    default: \'automatic\'

--- pipeline task execution modes
dryrun -- Run the commands (True) or generate the commands to be run but
   do not execute (False).
   default: False

acceptresults -- Add the results of the task to the pipeline context (True) or
   reject them (False).
   default: True

Output:

results -- If pipeline mode is 'getinputs' then None is returned. Otherwise
    the results object for the pipeline task is returned.


Examples:


        """
        if type(vis)==str: vis=[vis]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['target_list'] = target_list
        mytmp['parallel'] = parallel
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli/"
	trec = casac.utils().torecord(pathname+'hif_findcont.xml')

        casalog.origin('hif_findcont')
        if trec.has_key('hif_findcont') and casac.utils().verify(mytmp, trec['hif_findcont']) :
	    result = task_hif_findcont.hif_findcont(vis, target_list, parallel, pipelinemode, dryrun, acceptresults)

	else :
	  result = False
        return result
